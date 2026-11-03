#pragma once
#include "frontend177.h" // retain verified file/glyph helpers, not old UI polling
#include "menu179.h"
#include <cstring>

namespace menu180 {
static SRWLOCK lock=SRWLOCK_INIT;
static HANDLE stop_event,input_thread;
static volatile LONG running{},stopping{},ui_frames{},back_pending{},notified{};
static HWND game_window;
struct Snapshot {bool ready{},bindings{},failed{},focused{},glyphs{},glyph_failed{};};
static Snapshot snapshot;
static char error_text[512]{};
static bool ui_back;
static volatile LONG nav_pending{},nav_waiting{},target_pending{};
static int ui_direction,target_x,target_y;
static Snapshot read(){AcquireSRWLockShared(&lock);auto s=snapshot;ReleaseSRWLockShared(&lock);return s;}
static void remember(const char* data,size_t length){
    AcquireSRWLockExclusive(&lock);
    if(!error_text[0]&&data&&length){length=std::min(length,sizeof(error_text)-1);memcpy(error_text,data,length);error_text[length]=0;}
    ReleaseSRWLockExclusive(&lock);
}
static void notify(){
    if(InterlockedCompareExchange(&notified,1,0))return;
    char detail[512]{};AcquireSRWLockShared(&lock);memcpy(detail,error_text,sizeof(detail));ReleaseSRWLockShared(&lock);
    wchar_t message[1100]{};wcscpy_s(message,L"Private Steam Deck test180 - menu status\n\n");
    wchar_t decoded[600]{};MultiByteToWideChar(CP_UTF8,0,detail,-1,decoded,600);wcscat_s(message,decoded);
    wcscat_s(message,L"\n\nPlease photograph this message. This is a one-time failure report, not a recording or timed control test.");
    MessageBoxW(game_window,message,L"Unit Possession - private Deck test180",MB_OK|MB_ICONWARNING);
}
static BOOL CALLBACK find_window(HWND window,LPARAM value){
    DWORD pid{};GetWindowThreadProcessId(window,&pid);RECT rect{};
    if(pid==GetCurrentProcessId()&&IsWindowVisible(window)&&GetClientRect(window,&rect)&&rect.right>=640&&rect.bottom>=400){
        *reinterpret_cast<HWND*>(value)=window;return FALSE;
    }return TRUE;
}
struct Host {
    HWND window{};WPARAM buttons{};deck178::Cursor cursor;float wheel{};
    void navigate(int direction){InterlockedExchange(&nav_waiting,1);InterlockedCompareExchange(&nav_pending,direction,0);}
    bool navigation_pending()const{return nav_waiting!=0;}
    void clear_navigation(){InterlockedExchange(&nav_pending,0);InterlockedExchange(&nav_waiting,0);InterlockedExchange(&target_pending,0);}
    bool target(int x,int y){
        RECT rect{};POINT point{};if(!position(rect,point)||x<0||y<0||x>=rect.right||y>=rect.bottom)return false;
        POINT screen{x,y};if(!ClientToScreen(window,&screen)||!SetCursorPos(screen.x,screen.y))return false;
        cursor.reset();return PostMessageW(window,WM_MOUSEMOVE,buttons,MAKELPARAM(x,y))!=FALSE;
    }
    bool focused()const{DWORD pid{};return IsWindow(window)&&GetWindowThreadProcessId(window,&pid)&&pid==GetCurrentProcessId()&&GetForegroundWindow()==window;}
    bool position(RECT& rect,POINT& point){
        if(!focused()||!GetClientRect(window,&rect)||rect.right<200||rect.bottom<200||rect.right>32767||rect.bottom>32767||
           !GetCursorPos(&point)||!ScreenToClient(window,&point))return false;
        point.x=std::clamp<LONG>(point.x,0,rect.right-1);point.y=std::clamp<LONG>(point.y,0,rect.bottom-1);return true;
    }
    bool pointer(float dx,float dy){
        clear_navigation();RECT rect{};POINT point{};if(!position(rect,point))return false;int x=point.x,y=point.y;
        if(!cursor.move(x,y,rect.right,rect.bottom,double(dx)*rect.right,-double(dy)*rect.bottom))return false;
        POINT screen{x,y};if(!ClientToScreen(window,&screen)||!SetCursorPos(screen.x,screen.y))return false;
        return PostMessageW(window,WM_MOUSEMOVE,buttons,MAKELPARAM(x,y))!=FALSE;
    }
    bool button(bool left,bool down){
        RECT rect{};POINT point{};if(!position(rect,point))return false;
        WPARAM flag=left?MK_LBUTTON:MK_RBUTTON;if(down)buttons|=flag;else buttons&=~flag;
        UINT message=left?(down?WM_LBUTTONDOWN:WM_LBUTTONUP):(down?WM_RBUTTONDOWN:WM_RBUTTONUP);
        return PostMessageW(window,message,buttons,MAKELPARAM(point.x,point.y))!=FALSE;
    }
    bool scroll(float delta){
        if(!std::isfinite(delta)||std::fabs(delta)>2)return false;RECT rect{};POINT point{};if(!position(rect,point))return false;
        wheel+=delta*4.f;int notches=int(wheel);if(!notches)return true;wheel-=notches;
        if(!ClientToScreen(window,&point))return false;
        return PostMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(WORD(buttons),short(std::clamp(notches,-8,8)*WHEEL_DELTA)),MAKELPARAM(point.x,point.y))!=FALSE;
    }
    void reset_cursor(){cursor.reset();wheel=0;}
    void release_owned(bool left,bool right){
        if(IsWindow(window)&&(buttons||left||right)){
            PostMessageW(window,WM_CANCELMODE,0,0);
            // Release outside the client so focus loss cannot click a menu.
            if(left||(buttons&MK_LBUTTON))PostMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(-1,-1));
            if(right||(buttons&MK_RBUTTON))PostMessageW(window,WM_RBUTTONUP,0,MAKELPARAM(-1,-1));
        }buttons=0;reset_cursor();
    }
};
static DWORD WINAPI worker(void*){
    frontend177::owner=GetCurrentThreadId();deck179::Menu179 policy;Host host;
    uint64_t next_attempt{},next_glyph{};unsigned glyph_attempts{};
    try{for(;;){
        if(WaitForSingleObject(stop_event,8)!=WAIT_TIMEOUT)break;
        auto engine=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll"));
        if(engine&&battle_ready(engine))break;
        // Intro/menu transitions may replace the native window. Never keep
        // sending input to a destroyed trailer window.
        if(host.window&&!IsWindow(host.window)){policy.release(host);host.window=nullptr;game_window=nullptr;}
        HWND candidate=GetForegroundWindow();DWORD candidate_pid{};RECT candidate_rect{};
        if(candidate&&candidate!=host.window&&GetWindowThreadProcessId(candidate,&candidate_pid)&&candidate_pid==GetCurrentProcessId()&&
           IsWindowVisible(candidate)&&GetClientRect(candidate,&candidate_rect)&&candidate_rect.right>=640&&candidate_rect.bottom>=400){
            policy.release(host);host.window=candidate;game_window=candidate;
        }
        if(!host.window){EnumWindows(find_window,reinterpret_cast<LPARAM>(&host.window));game_window=host.window;}
        if(deck177::busy(deck177::Owner::Battle))continue;
        auto now=GetTickCount64();auto& backend=frontend177::backend;
        if(!backend.initialized()&&now>=next_attempt){next_attempt=now+2000;backend.open_existing(frontend177::utf8(frontend177::manifest).c_str(),true);}
        if(InterlockedExchange(&target_pending,0)){
            int x{},y{};AcquireSRWLockShared(&lock);x=target_x;y=target_y;ReleaseSRWLockShared(&lock);
            if(x>=0)host.target(x,y);InterlockedExchange(&nav_waiting,0);
        }
        deck174::Frame frame;bool good=backend.sample(now*1000,host.focused(),frame);bool back=false;
        if(good){policy.sample(frame,now*1000,frontend177::config,host,back);if(back)InterlockedExchange(&back_pending,1);}
        else policy.release(host);
        if(good&&!frontend177::glyphs_ready&&glyph_attempts<3&&now>=next_glyph){
            ++glyph_attempts;next_glyph=now+1500;try{frontend177::prepare_glyphs();}catch(const std::exception& e){if(glyph_attempts==3)remember(e.what(),strlen(e.what()));}
        }
        Snapshot state;auto status=backend.state();state.ready=status==deck174::BackendState::Ready;state.bindings=status==deck174::BackendState::WaitingBindings;
        state.failed=status!=deck174::BackendState::Ready&&status!=deck174::BackendState::WaitingBindings&&status!=deck174::BackendState::WaitingSteam&&status!=deck174::BackendState::WaitingDevice&&status!=deck174::BackendState::Idle;
        state.focused=host.focused();state.glyphs=frontend177::glyphs_ready;state.glyph_failed=glyph_attempts>=3&&!state.glyphs;
        AcquireSRWLockExclusive(&lock);snapshot=state;bool error=error_text[0]!=0;ReleaseSRWLockExclusive(&lock);
        // Root initialization can precede a long intro. Absence of an early
        // frame alone is not an initialization error; report real errors only.
        if(error&&!notified&&host.focused()){policy.release(host);notify();policy.normalizer.reset();}
    }}catch(const std::exception& e){remember(e.what(),strlen(e.what()));if(host.focused())notify();}
    policy.release(host);frontend177::stop();
    AcquireSRWLockExclusive(&lock);if(stop_event){CloseHandle(stop_event);stop_event=nullptr;}snapshot={};ReleaseSRWLockExclusive(&lock);
    InterlockedExchange(&running,0);return 0;
}
static bool prepare(){
    if(!deck174::paths::proton())return false;
    if(running&&stopping&&input_thread)WaitForSingleObject(input_thread,250);
    if(InterlockedCompareExchange(&running,1,0))return !stopping;
    try{
        if(input_thread){CloseHandle(input_thread);input_thread=nullptr;}
        if(!frontend177::prepare())throw std::runtime_error("Native menu resource preparation failed");
        InterlockedExchange(&stopping,0);InterlockedExchange(&ui_frames,0);InterlockedExchange(&back_pending,0);
        AcquireSRWLockExclusive(&lock);error_text[0]=0;snapshot={};ReleaseSRWLockExclusive(&lock);
        stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!stop_event)throw std::runtime_error("Native menu stop event unavailable");
        input_thread=CreateThread(nullptr,0,worker,nullptr,0,nullptr);
        if(!input_thread)throw std::runtime_error("Native menu input worker could not start");return true;
    }catch(const std::exception& e){remember(e.what(),strlen(e.what()));if(stop_event){CloseHandle(stop_event);stop_event=nullptr;}frontend177::stop();InterlockedExchange(&running,0);
#ifndef S2UP165_LUA_FIXTURE
        notify(); // bounded startup failure only; normal startup is silent
#endif
        return false;}
}
static bool stop(){
    InterlockedExchange(&stopping,1);AcquireSRWLockShared(&lock);if(stop_event)SetEvent(stop_event);ReleaseSRWLockShared(&lock);return true;
}
static bool poll(){ui_back=InterlockedExchange(&back_pending,0)!=0;ui_direction=InterlockedExchange(&nav_pending,0);return running&&!stopping;}
static std::string argument(void* lua){
#ifndef S2UP165_LUA_FIXTURE
    // Read-only Lua5.1 C API inside the admitted immutable engine. No retained
    // lua_State and no number-to-string coercion that could mutate the stack.
    auto base=reinterpret_cast<unsigned char*>(GetModuleHandleW(L"Empire.Retail.dll"));
    const unsigned char expected_type[]={0xff,0x74,0x24,0x08,0xff,0x74,0x24,0x08,0xe8,0xa3,0xed,0xff,0xff,0x83,0xc4,0x08};
    const unsigned char expected_string[]={0x56,0xff,0x74,0x24,0x0c,0x8b,0x74,0x24,0x0c,0x56,0xe8,0x01,0xef,0xff,0xff,0x8b};
    unsigned char actual[16]{};SIZE_T read{};
    auto matches=[&](unsigned offset,const unsigned char* wanted){return ReadProcessMemory(GetCurrentProcess(),base+offset,actual,16,&read)&&read==16&&!memcmp(actual,wanted,16);};
    if(!base||!matches(0xea3a90,expected_type)||!matches(0xea3930,expected_string))return {};
    using Type=int(__cdecl*)(void*,int);using String=const char*(__cdecl*)(void*,int,size_t*);
    if(reinterpret_cast<Type>(base+0xea3a90)(lua,1)!=4)return {};
    size_t size{};const char* text=reinterpret_cast<String>(base+0xea3930)(lua,1,&size);return text?std::string(text,std::min<size_t>(size,511)):std::string{};
#else
    (void)lua;return {}; // fixture never calls game APIs or opens a dialog
#endif
}
static void report(void* lua){auto text=argument(lua);remember(text.data(),text.size());}
static bool focus_text(const std::string& text){
    int x=-1,y=-1;
    if(!text.empty()){
        if(text.size()>12)return false;unsigned a=0,b=0;bool comma=false,first=false,second=false;
        for(char c:text){if(c==','&&!comma){comma=true;continue;}if(c<'0'||c>'9')return false;
            auto& value=comma?b:a;value=value*10+unsigned(c-'0');if(value>32767)return false;if(comma)second=true;else first=true;}
        if(!comma||!first||!second)return false;x=int(a);y=int(b);
    }
    AcquireSRWLockExclusive(&lock);target_x=x;target_y=y;ReleaseSRWLockExclusive(&lock);
    InterlockedExchange(&target_pending,1);return true;
}
}
#define MENU180(name,value) extern "C" __declspec(dllexport) int __cdecl s2up_menu_##name##180(void* lua){try{if(!caller165(lua))return 0;return (value)?1:0;}catch(const std::exception& e){menu180::remember(e.what(),strlen(e.what()));return 0;}}
MENU180(prepare,menu180::prepare())
MENU180(supported,deck174::paths::proton())
MENU180(stop,menu180::stop())
MENU180(poll,menu180::poll())
MENU180(active,menu180::running&&!menu180::stopping)
MENU180(ready,menu180::read().ready)
MENU180(bindings,menu180::read().bindings)
MENU180(failed,menu180::read().failed)
MENU180(focused,menu180::read().focused)
MENU180(glyphs,menu180::read().glyphs)
MENU180(glyph_failed,menu180::read().glyph_failed)
MENU180(cancel,menu180::ui_back)
MENU180(heartbeat,(InterlockedIncrement(&menu180::ui_frames),true))
MENU180(up,menu180::ui_direction==1)
MENU180(down,menu180::ui_direction==2)
MENU180(left,menu180::ui_direction==3)
MENU180(right,menu180::ui_direction==4)
MENU180(focus,menu180::focus_text(menu180::argument(lua)))
MENU180(error,(menu180::report(lua),false))
#undef MENU180
