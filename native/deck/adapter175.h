#pragma once
#include "device192.h"
// Compiled only into the isolated Deck development adapter by adapt_runtime175.
// The maintained Windows adapter source and normal Workshop build are unchanged.
#include "mailbox175.h"
#include "menu178.h"
#include "glyphs181.h"
#include "rts175.h"
#include "config174.h"
#include "steam_input174.h"
#include "proton_paths174.h"
#include "native_guard175.h"
#include "orders175.h"
#include "defaults175_generated.h"
#include "ownership177.h"
#include "cursor178.h"
#include "input_trace181.h"
#include "native_modifiers188.h"
#include "selection189.h"
#include "qol193.h"
#include "battle_ready181.h"
#include <fstream>

#include "look201.h"
#include "look202.h"
namespace deck175 {
static SRWLOCK queue_lock=SRWLOCK_INIT;
static deck181::Mailbox mailbox;
static Mailbox rts_mailbox;
static deck174::SteamBackend backend;
static Rts rts;
static deck174::Config config;
static volatile LONG running{},posted{},stopping{},cleanup_done{},errors{},notified{};
static UINT message_id;
static uint64_t next_attempt{},last_callback{};
static bool configured{},checked{};
static HWND owned_window;
static std::wstring manifest_path;
static WPARAM owned_mouse_flags;
static deck177::Lease input_lease177;
static deck178::Cursor cursor178;
static deck193::DoubleClick selection193;
static deck188::Modifiers modifiers188;
static SHORT WINAPI native_async188(int key){SHORT physical=GetAsyncKeyState(key);return modifiers188.merge(key,physical,GetTickCount64(),GetForegroundWindow());}

static uint64_t clock_us(){LARGE_INTEGER t{},f{};QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return uint64_t(t.QuadPart/f.QuadPart)*1000000+uint64_t(t.QuadPart%f.QuadPart)*1000000/uint64_t(f.QuadPart);}
static bool native_context(ContextProof& proof,bool cleanup=false){
    proof={};
    __try{
        DWORD pid{},thread=GetWindowThreadProcessId(input_window,&pid);
        proof.owner_thread=thread&&thread==GetCurrentThreadId()&&pid==GetCurrentProcessId();
        if(!proof.owner_thread||!state||(!cleanup&&(!enabled||!state->ready||state->stop))||!input_window||!native_window_proc)return false;
        proof.focused=GetForegroundWindow()==input_window;
        auto root=field<uintptr_t>(base,0x1bc8180),env=root?field<uintptr_t>(root,0x110):0;
        if(!root||!env||field<uintptr_t>(root,0)!=base+0x170d284||field<uintptr_t>(root,0x18)!=base+0x170d290||field<uintptr_t>(root,0x10c)!=base+0x170d304)return false;
        auto common=field<uintptr_t>(env,8),phase=common?field<uintptr_t>(common,0xb4):0;
        if(!phase)return false;
        auto manager=field<uintptr_t>(env,0x281fc),camera=manager?field<uintptr_t>(manager,0x8e0):0;
        if(!manager||!camera||field<uintptr_t>(manager,0)!=base+0x15d26fc)return false;
        proof.identity={root,env,common,phase,manager,camera};
        proof.possessed=state->mode==POSSESSED||dead_view();
        proof.rts=deck181::battle_ready(base,[](uintptr_t at){return field<uintptr_t>(at,0);})&&camera==field<uintptr_t>(manager,0x8cc)&&field<uintptr_t>(camera,0)==base+0x15d1a74;
        // These are the same native menu/movie blockers used by10fb980.
        using Blocked=bool(__thiscall*)(void*);
        proof.blocked=field<unsigned char>(env,0x281e8)!=0||reinterpret_cast<Blocked>(base+0x2e5810)(reinterpret_cast<void*>(root+0x10c));
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
struct NativeHost:Host {
    Identity identity;WPARAM modifier_flags{};
    void modifiers(bool shift,bool control)override{modifier_flags=(shift?MK_SHIFT:0)|(control?MK_CONTROL:0);}
    explicit NativeHost(Identity value):identity(value){}
    bool current(const Identity& expected)override{ContextProof now;return native_context(now)&&now.permits_rts()&&now.identity==expected;}
    bool point(RECT& rect,POINT& pixel){
        if(!current(identity)||!GetClientRect(input_window,&rect)||rect.right<200||rect.bottom<200||rect.right>32767||rect.bottom>32767||
           !GetCursorPos(&pixel)||!ScreenToClient(input_window,&pixel))return false;
        pixel.x=std::clamp<LONG>(pixel.x,0,rect.right-1);pixel.y=std::clamp<LONG>(pixel.y,0,rect.bottom-1);return true;
    }
    bool event(UINT message,WPARAM flags){
        RECT rect{};POINT pixel{};if(!point(rect,pixel))return false;
        if(message==WM_MOUSEWHEEL&&!ClientToScreen(input_window,&pixel))return false;
        deck189::SelectionModifiers modifiers(base,message==WM_MOUSEWHEEL?WPARAM(LOWORD(flags)):flags);
        CallWindowProcW(native_window_proc,input_window,message,flags,MAKELPARAM(pixel.x,pixel.y));return true;
    }
    void cancel_owned(const Identity& expected)override{
        // Permit cancellation after focus/menu changes only while this exact
        // native UI identity still exists. Never touch a new battle or camera.
        selection193.reset();wheel=0;owned_mouse_flags=0;
        ContextProof now;if(!native_context(now,true)||!(now.identity==expected))return;
        __try{
            RECT client{};if(!GetClientRect(input_window,&client))return;
            LPARAM center=MAKELPARAM(client.right/2,client.bottom/2);
            auto decode=reinterpret_cast<NativeMouseDecode100>(base+0xbd4a10);
            decode(input_window,WM_LBUTTONUP,0,center,client.right,client.bottom,0);
            decode(input_window,WM_RBUTTONUP,0,center,client.right,client.bottom,0);
            unsigned release=4;
            reinterpret_cast<NativeCursorEvent100>(base+0x2f7350)(reinterpret_cast<void*>(expected.root+0x10c),&release,0,0,1);
            CallWindowProcW(native_window_proc,input_window,WM_CANCELMODE,0,0);
        }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&errors);}
    }
    bool pointer(Vec2 delta)override{
        RECT rect{};POINT pixel{};if(!point(rect,pixel)||!deck174::finite(delta))return false;
        int x=pixel.x,y=pixel.y;
        if(!cursor178.move(x,y,rect.right,rect.bottom,double(delta.x)*rect.right*.6*config.cursor_sensitivity,-double(delta.y)*rect.bottom*.6*config.cursor_sensitivity))return false;
        pixel.x=x;pixel.y=y;
        selection193.move(x,y,std::max(1,GetSystemMetrics(SM_CXDOUBLECLK)/2),std::max(1,GetSystemMetrics(SM_CYDOUBLECLK)/2));
        if(!ClientToScreen(input_window,&pixel))return false;
        // This moves the game's focused native cursor, not a system input
        // injection or a keyboard/mouse controller preset. The game's own move
        // message performs hit-testing and draws its ordinary cursor.
        if(!SetCursorPos(pixel.x,pixel.y))return false;
        // Direct native gestures do not alter OS button state. Supply the
        // owned drag bits on native moves; the queued OS move may have none.
        return event(WM_MOUSEMOVE,owned_mouse_flags);
    }
    void reset_pointer()override{cursor178.reset();}
    void reset_clicks()override{selection193.reset();}
    bool scroll(float delta)override{
        if(!current(identity)||!std::isfinite(delta)||std::fabs(delta)>2)return false;
        wheel+=delta*4.f;int notches=int(wheel);
        if(notches){wheel-=notches;return event(WM_MOUSEWHEEL,MAKEWPARAM(0,short(std::clamp(notches,-8,8)*WHEEL_DELTA)));}
        return true;
    }
    bool select(bool down,bool modifier)override{
        if(!current(identity))return false;
        const WPARAM flags=(down?MK_LBUTTON:0)|(modifier?MK_CONTROL:0)|modifier_flags;
        RECT rect{};POINT pixel{};if(!point(rect,pixel)){selection193.reset();return false;}
        const bool twice=selection193.button(down,(flags&(MK_CONTROL|MK_SHIFT))!=0,pixel.x,pixel.y,GetTickCount64(),GetDoubleClickTime(),std::max(1,GetSystemMetrics(SM_CXDOUBLECLK)/2),std::max(1,GetSystemMetrics(SM_CYDOUBLECLK)/2));
        if(!event(down?(twice?WM_LBUTTONDBLCLK:WM_LBUTTONDOWN):WM_LBUTTONUP,flags)){selection193.reset();return false;}
        owned_mouse_flags=down?flags:0;return true;
    }
    bool order(unsigned phase,bool modifier)override{
        if(phase>3||!current(identity))return false;
        selection193.reset();
        __try{command_refresh103(identity.root);}
        __except(EXCEPTION_EXECUTE_HANDLER){return false;}
        if(!event(phase==3?WM_RBUTTONDBLCLK:phase==0?WM_RBUTTONDOWN:phase==1?WM_MOUSEMOVE:WM_RBUTTONUP,(phase==2?0:MK_RBUTTON)|modifier_flags|(modifier?MK_SHIFT:0)))return false;
        owned_mouse_flags=phase!=2?(MK_RBUTTON|modifier_flags|(modifier?MK_SHIFT:0)):0;return true;
    }
    bool halt()override{
        if(!current(identity))return false;
        __try{
            return native_halt(base,identity.root,identity.environment,
                [](uintptr_t address){return field<uintptr_t>(address,0);},
                [&](){return current(identity);},
                [&](uintptr_t root,unsigned action,float value){
                    using NativeAction=void(__thiscall*)(void*,unsigned,float);
                    reinterpret_cast<NativeAction>(base+0x10dfb80)(reinterpret_cast<void*>(root),action,value);
                });
        }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    bool camera(Vec2 pan,float rotate,float zoom,float pitch,float dt)override{
        if(!current(identity)||!deck174::finite(pan)||!std::isfinite(rotate)||!std::isfinite(zoom)||!std::isfinite(pitch)||!std::isfinite(dt)||dt<0||dt>.05f)return false;
        __try{
            using Step=void(__thiscall*)(void*,float);
            auto table=field<uintptr_t>(identity.camera,0);
            for(unsigned i=0;i<8;++i)if(field<uintptr_t>(table,binding175::camera_slots[i])!=base+binding175::camera_functions[i])return false;
            auto apply=[&](unsigned slot,float value){reinterpret_cast<Step>(field<uintptr_t>(table,slot))(reinterpret_cast<void*>(identity.camera),value);};
            // 325c50 stores the native camera's tick factor; 2f7770 and324950
            // already multiply their configured rates by that factor. The old
            // extra dt made motion especially slow at high frame rates.
            float native_tick=field<unsigned char>(identity.camera,0x144)?field<float>(identity.camera,0x148):field<float>(identity.camera,0x140);
            float ticks{};if(!deck178::camera_ticks(dt,native_tick,ticks))return false;
            const float pace=config.camera_pan_sensitivity*ticks;
            if(pan.y)apply(pan.y>0?0x94:0x98,std::fabs(pan.y)*pace);
            if(pan.x)apply(pan.x>0?0x9c:0xa0,std::fabs(pan.x)*pace);
            if(rotate)apply(0x12c,rotate*config.camera_turn_sensitivity*ticks);
            if(pitch)apply(0x130,pitch*config.camera_turn_sensitivity*ticks);
            // Original cam_down/cam_up vtable routes (2fdcf0): continuous
            // distance movement, with native rate/height handling in2f7a50.
            // Like pan/turn, the native routine already applies its tick factor.
            if(zoom)apply(zoom>0?0xa8:0xa4,std::fabs(zoom)*config.look_sensitivity*ticks);
            return true;
        }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
    }
    static inline float wheel{};
};
#include "battle181.h"
static void prepare_files(HMODULE module){
    wchar_t path[32768]{};DWORD n=GetModuleFileNameW(module,path,32768);if(!n||n>=32768)throw std::runtime_error("Deck module path unavailable");
    auto directory=std::wstring(path)+L".deck175";deck174::paths::plain(directory);
    if(!CreateDirectoryW(directory.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("Deck config directory unavailable");
    auto write=[&](const wchar_t* name,const char* bytes,size_t size){
        auto target=directory+L"\\"+name;deck174::paths::plain(target);
        HANDLE file=CreateFileW(target.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE){
            if(GetLastError()!=ERROR_FILE_EXISTS)throw std::runtime_error("Deck config write failed");
            std::ifstream stream(target,std::ios::binary);std::string existing(size+1,'\0');stream.read(existing.data(),std::streamsize(existing.size()));existing.resize(size_t(stream.gcount()));
            if(existing!=std::string(bytes,size))throw std::runtime_error("Foreign Deck config preserved");return;
        }
        DWORD written{};bool ok=WriteFile(file,bytes,DWORD(size),&written,nullptr)&&written==size&&FlushFileBuffers(file);CloseHandle(file);
        if(!ok)throw std::runtime_error("Deck config incomplete");
    };
    write(L"deck-actions174.vdf",defaults175::manifest,sizeof(defaults175::manifest)-1);
    write(L"deck-native174.vdf",defaults175::bindings,sizeof(defaults175::bindings)-1);
    auto settings=directory+L"\\controller174.ini";deck174::paths::plain(settings);
    if(GetFileAttributesW(settings.c_str())==INVALID_FILE_ATTRIBUTES)write(L"controller174.ini",defaults175::settings,sizeof(defaults175::settings)-1);
    std::ifstream in(settings,std::ios::binary);std::string text(16385,'\0');in.read(text.data(),std::streamsize(text.size()));
    if(in.bad()||size_t(in.gcount())>16384)throw std::runtime_error("Deck settings exceed bound");text.resize(size_t(in.gcount()));
    std::string error;if(!deck174::parse_config(text,config,error))throw std::runtime_error(error);
    rts.configure(config);manifest_path=directory+L"\\deck-actions174.vdf";
}
static bool eligible181(){
 if(!state||!state->ready||state->stop||!enabled||!input_window||!native_window_proc||!IsWindow(input_window))return false;
 DWORD pid{};if(!GetWindowThreadProcessId(input_window,&pid)||pid!=GetCurrentProcessId())return false;
 __try{
  if(!deck181::battle_ready(base,[](uintptr_t at){return field<uintptr_t>(at,0);}))return false;
  auto root=field<uintptr_t>(base,0x1bc8180),env=root?field<uintptr_t>(root,0x110):0;
  auto manager=env?field<uintptr_t>(env,0x281fc):0,camera=manager?field<uintptr_t>(manager,0x8e0):0;
  return root&&field<uintptr_t>(root,0)==base+0x170d284&&manager&&field<uintptr_t>(manager,0)==base+0x15d26fc&&camera;
 }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static void yield181(){
 // Restore the keyboard poll before relinquishing the battle lease. The
 // campaign bridge may then reacquire its own disabled original trampoline.
 // If the site changed unexpectedly, retain ownership and never overwrite it.
 if(!keyboard_owner196.release())return;
 modifiers188.clear();
 if(running){if(!backend.close())return;InterlockedExchange(&running,0);controller181::clear_input();}
 AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
 InterlockedExchange(&posted,0);InterlockedExchange(&controller181::menu_open,0);input_lease177.release();
}
static void poll(HMODULE module){
    if(stopping||!device192::enabled())return;
    static deck181::PendingEntry pending_entry;
    if(state&&pending_entry.expired(state->mode==ARMED&&!possessed&&!entry_attempted,GetTickCount64())){
        // No borrowed object is touched on this worker: the existing failure
        // path posts any required camera restoration to its owning window.
        InterlockedExchange(&entry_attempted,1);InterlockedExchange(&entry_pending,0);entry_failed(181);
    }
    const bool eligible=eligible181();deck177::battle_request().set(eligible);
    if(!eligible){yield181();deck181::trace_input(0,unsigned(backend.state()),0,input_window,unsigned(posted),state?state->mode:0,state?unsigned(state->aiming):0);return;}
    if(owned_window!=input_window){
        owned_window=input_window;InterlockedExchange(&posted,0);controller181::clear_input();
        AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
    }
    // The frontend owns Steam Input on the UI thread until its native menu
    // leaves. Do not initialize/shutdown a second reader during that handoff.
    if(deck177::busy(deck177::Owner::Frontend)){
        if(running){
            if(!backend.close())return;
            InterlockedExchange(&running,0);
            AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
        }
        input_lease177.release();return;
    }
    if(!checked){checked=true;if(!native_guards(base)){InterlockedExchange(&errors,1);}}
    if(errors){
        if(input_window&&GetForegroundWindow()==input_window&&!InterlockedCompareExchange(&notified,1,0)){
            const wchar_t* why=errors==1?L"The native RTS function checks did not match this process.":
                errors==2?L"The private controller configuration could not be prepared.":L"The native controller bridge could not initialize.";
            std::wstring text=std::wstring(L"Steam Deck RTS test175 did not start.\n\n")+why+
                L"\n\nThe normal possession mod remains available. Please send a photo of this message. No recorder or debugger was started.";
            MessageBoxW(input_window,text.c_str(),L"Unit Possession - private Deck test",MB_OK|MB_ICONWARNING);
        }return;
    }
    if(!configured){try{prepare_files(module);configured=true;}catch(...){InterlockedExchange(&errors,2);return;}}
    // The saved Deck trace proved the Async import was unused for selection.
    // Refresh the actual native UI modifier consumer at the event boundary.
    if(!message_id){std::wstring name=L"S2UP.Deck175.Frame."+std::to_wstring(GetCurrentProcessId());message_id=RegisterWindowMessageW(name.c_str());if(!message_id){InterlockedExchange(&errors,3);return;}}
    backend.select_context(controller181::context());
    if(!running&&GetTickCount64()>=next_attempt){
        if(!deck177::claim_battle(input_lease177))return;
        // A frontend can start between the first check and lease creation.
        // Its poll waits for this lease, so release before entering Steam.
        if(deck177::busy(deck177::Owner::Frontend)){input_lease177.release();return;}
        if(!keyboard_owner196.acquire()){input_lease177.release();return;}
        int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,manifest_path.data(),int(manifest_path.size()),nullptr,0,nullptr,nullptr);
        if(n<=0){input_lease177.release();InterlockedIncrement(&errors);return;}std::string utf(size_t(n),'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,manifest_path.data(),int(manifest_path.size()),utf.data(),n,nullptr,nullptr);
        if(backend.open_existing(utf.c_str(),true))InterlockedExchange(&running,1);
        else {input_lease177.release();next_attempt=GetTickCount64()+2000;
            auto status=backend.state();if(status!=deck174::BackendState::WaitingSteam&&status!=deck174::BackendState::WaitingDevice)InterlockedExchange(&errors,4);}
    }
    if(!running||!input_window||!native_window_proc)return;
    DWORD pid{};GetWindowThreadProcessId(input_window,&pid);if(pid!=GetCurrentProcessId())return;
    deck174::Frame frame;bool ok=backend.sample(clock_us(),GetForegroundWindow()==input_window,frame);
    auto output=ok?controller181::sample(frame):deck181::Output{};
    const bool modifier_owner188=ok&&output.accepted&&!output.help_blocked&&output.context==deck174::Context::RTS&&frame.focused&&GetForegroundWindow()==input_window;
    modifiers188.publish(modifier_owner188&&output.held(deck181::Control),modifier_owner188&&output.held(deck181::Shift),modifier_owner188?input_window:nullptr,GetTickCount64());
    // Controls help is rendered by our own bounded presentation channel.
    deck181::trace_input(2,unsigned(backend.state()),unsigned(output.context),input_window,unsigned(posted),state?state->mode:0,state?unsigned(state->aiming):0,ok?&frame:nullptr);
    if(!ok)controller181::clear_input();
    bool queued=true;AcquireSRWLockExclusive(&queue_lock);
    if(ok){
        if(output.show_controls){mailbox.invalidate();rts_mailbox.invalidate();}
        // The former RTS delivery path remains independent of possession intent.
        if(output.context==deck174::Context::RTS&&!output.help_blocked)rts_mailbox.put(frame);else rts_mailbox.invalidate();
        queued=mailbox.put(frame,output,state?state->lease_generation:0,state?state->actor_id:0,state?state->unit_id:0);
    }else{mailbox.invalidate();rts_mailbox.invalidate();}
    ReleaseSRWLockExclusive(&queue_lock);if(!queued)controller181::clear_input();
    owned_window=input_window;
    if(!InterlockedCompareExchange(&posted,1,0))if(!PostMessageW(owned_window,message_id,175,0))InterlockedExchange(&posted,0);
    // Deliver timestamped camera input before any optional image IO/decoding.
    controller181::present();
}
static bool window(HWND window,UINT message,WPARAM w,LPARAM l){
    if(!stopping&&window==input_window&&
       (w=='K'||(message==WM_CHAR&&w=='k'))&&
       (message==WM_KEYDOWN||message==WM_KEYUP||message==WM_CHAR)&&hudcycle198::window_owned(keyboard_fps84())){
        ContextProof proof;native_context(proof);
        return controller181::keyboard_hud198(message,w,l,proof);
    }
    if(message==WM_NCDESTROY&&window==owned_window){
        controller181::clear_input();InterlockedExchange(&posted,0);owned_window=nullptr;
        AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
        return false;
    }
    if(window==owned_window&&message==WM_MOUSEMOVE){
        ContextProof proof;
        if(native_context(proof)&&proof.permits_rts()){
            deck181::Sample modifier_sample188;WPARAM held188=0;if(deck181::read_sample(GetCurrentProcessId(),modifier_sample188)&&modifier_sample188.context==unsigned(deck174::Context::RTS)){if(modifier_sample188.keys[VK_CONTROL/32]&(1u<<(VK_CONTROL%32)))held188|=MK_CONTROL;if(modifier_sample188.keys[VK_SHIFT/32]&(1u<<(VK_SHIFT%32)))held188|=MK_SHIFT;}
            CallWindowProcW(native_window_proc,window,message,w|owned_mouse_flags|held188,l);return true;
        }
    }
    if(!message_id||message!=message_id||w!=175||window!=owned_window)return false;
    InterlockedExchange(&posted,0);
    std::array<deck181::Queued,deck181::Mailbox::capacity> frames;bool reset=false;
    std::array<deck174::Frame,Mailbox::capacity> rts_frames;bool rts_reset=false;
    AcquireSRWLockExclusive(&queue_lock);unsigned count=mailbox.take(frames,reset),rts_count=rts_mailbox.take(rts_frames,rts_reset);ReleaseSRWLockExclusive(&queue_lock);
    ContextProof proof;bool context=native_context(proof);NativeHost host(proof.identity);
    InterlockedExchange(&controller181::menu_open,context&&proof.blocked?1:0);
    const auto now=clock_us();if(rts_reset||stopping||!context)rts.reset(host);if(reset||stopping||!context)controller181::cancel_window();
    if(!stopping&&context&&proof.permits_rts()){
        rts.begin(proof,host,now);
        for(unsigned i=0;i<rts_count;++i)rts.sample(rts_frames[i],proof,host,now);
        rts.finish(host,now);
    }else rts.reset(host);
    if(stopping)InterlockedExchange(&cleanup_done,1);
    if(!stopping&&context)for(unsigned i=0;i<count;++i)controller181::dispatch(frames[i],proof);
    last_callback=now;return true;
}
static void stop(){
    if(InterlockedExchange(&stopping,1))return;
    deck177::battle_request().set(false);
    controller181::clear_input();
    AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
    if(owned_window&&message_id){
        InterlockedExchange(&posted,1);
        if(PostMessageW(owned_window,message_id,175,0)){
            auto deadline=GetTickCount64()+250;while(!cleanup_done&&GetTickCount64()<deadline)Sleep(1);
        }
    }
    if(running){if(!backend.close())return;InterlockedExchange(&running,0);}
    modifiers188.remove(&native_async188);input_lease177.release();
}
struct Lifetime {~Lifetime(){stop();}};
static void focus_lost(HWND window){
    if(window!=owned_window)return;modifiers188.clear();
    controller181::clear_input();controller181::cancel_window();
    AcquireSRWLockExclusive(&queue_lock);mailbox.invalidate();rts_mailbox.invalidate();ReleaseSRWLockExclusive(&queue_lock);
    ContextProof proof;native_context(proof,true);NativeHost host(proof.identity);rts.reset(host);
}
} // deck175
static bool deck175_window(HWND window,UINT message,WPARAM w,LPARAM l){return deck175::window(window,message,w,l);}
static void deck175_focus_lost(HWND window){deck175::focus_lost(window);}

extern "C" __declspec(dllexport) const deck181::InputTrace* __cdecl S2UPDeckInputProbe181(){return &deck181::input_trace;}

extern "C" __declspec(dllexport) const deck188::Modifiers::Status* __cdecl S2UPDeckModifiersProbe188(){return &deck175::modifiers188.status;}
