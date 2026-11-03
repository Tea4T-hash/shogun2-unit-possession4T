#pragma once
#include "device192.h"
// Included only after the existing Workshop Lua arity/host verifier. Every
// entry returns its existing true argument or no results; no Lua stack writes,
// retained lua_State, engine pointer calls, external helper or OS key injection.
#include "steam_input174.h"
#include "config174.h"
#include "proton_paths174.h"
#include "defaults175_generated.h"
#include "ownership177.h"
#include "cursor178.h"
#include <gdiplus.h>
#include <fstream>

namespace frontend177 {
static deck174::SteamBackend backend;
static deck174::Normalizer normalizer;
static deck174::Config config;
static DWORD owner;
static deck177::Lease lease;
static bool prepared,failed,glyphs_ready,focused,glyph_error;
static unsigned glyph_attempts;
static uint64_t next_glyph;
static uint64_t next_attempt,last_poll;
static bool confirm,cancel,up,down,left,right,pointer;
static bool pad_seed;
static deck174::Vec2 previous_pad;
static deck178::Cursor cursor178;
static std::wstring directory,manifest;
static void clear_edges(){confirm=cancel=up=down=left=right=pointer=false;}
static bool stop(){
    if(owner&&owner!=GetCurrentThreadId())return false;
    clear_edges();normalizer.reset();pad_seed=false;focused=false;cursor178.reset();
    if(!backend.close())return false;
    lease.release();
    prepared=false;return true;
}
static std::wstring utf16(const std::string& value){
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),int(value.size()),nullptr,0);
    if(n<=0)throw std::runtime_error("Controller path is not UTF8");std::wstring result(size_t(n),L'\0');
    MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),int(value.size()),result.data(),n);return result;
}
static std::string utf8(const std::wstring& value){
    int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),int(value.size()),nullptr,0,nullptr,nullptr);
    if(n<=0)throw std::runtime_error("Controller path conversion failed");std::string result(size_t(n),'\0');
    WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,value.data(),int(value.size()),result.data(),n,nullptr,nullptr);return result;
}
static void exact_file(const std::wstring& path,const void* data,size_t size){
    deck174::paths::plain(path);
    HANDLE file=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file==INVALID_HANDLE_VALUE){
        if(GetLastError()!=ERROR_FILE_EXISTS)throw std::runtime_error("Controller cache unavailable");
        std::ifstream stream(path,std::ios::binary);std::vector<char> bytes(size+1);
        stream.read(bytes.data(),std::streamsize(bytes.size()));
        if(size_t(stream.gcount())!=size||memcmp(bytes.data(),data,size))throw std::runtime_error("Existing controller cache differs; preserved");
        return;
    }
    DWORD written{};bool ok=WriteFile(file,data,DWORD(size),&written,nullptr)&&written==size&&FlushFileBuffers(file);CloseHandle(file);
    if(!ok)throw std::runtime_error("Controller cache write incomplete");
}
static bool prepare(bool deferred_lease=false){
    if(!device192::enabled())return false;
    if(prepared)return owner==GetCurrentThreadId();owner=GetCurrentThreadId();failed=false;
    directory=own165()+L".deck177";deck174::paths::plain(directory);
    if(!CreateDirectoryW(directory.c_str(),nullptr)&&GetLastError()!=ERROR_ALREADY_EXISTS)throw std::runtime_error("Controller directory unavailable");
    exact_file(directory+L"\\deck-actions174.vdf",defaults175::manifest,sizeof(defaults175::manifest)-1);
    exact_file(directory+L"\\deck-native174.vdf",defaults175::bindings,sizeof(defaults175::bindings)-1);
    manifest=directory+L"\\deck-actions174.vdf";
    if(!deferred_lease&&!lease.acquire(deck177::Owner::Frontend))throw std::runtime_error("Another native frontend controller is active");
    prepared=true;glyphs_ready=glyph_error=false;glyph_attempts=0;next_glyph=0;last_poll=next_attempt=0;normalizer.reset();return true;
}
static void glyph(int origin,const wchar_t* name){
    const char* raw=backend.glyph_file(origin);if(!raw||!raw[0]||strnlen_s(raw,32769)>32768)throw std::runtime_error("Steam controller glyph unavailable");
    auto source=utf16(raw);
    // Proton normally exposes Linux absolute paths through its existing Z:
    // mapping. Read only a resolved existing disk file; never create a mapping.
    if(!source.empty()&&source[0]==L'/')source=L"Z:"+source;
    for(auto& c:source)if(c==L'/')c=L'\\';deck174::paths::read_file(source);
    if(std::filesystem::file_size(source)>4*1024*1024)throw std::runtime_error("Steam glyph exceeds bound");
    static ULONG_PTR token{};if(!token){Gdiplus::GdiplusStartupInput input;
        if(Gdiplus::GdiplusStartup(&token,&input,nullptr)!=Gdiplus::Ok)throw std::runtime_error("Controller image decoder unavailable");}
    Gdiplus::Bitmap image(source.c_str());unsigned w=image.GetWidth(),h=image.GetHeight();
    if(image.GetLastStatus()!=Gdiplus::Ok||!w||!h||w>256||h>256)throw std::runtime_error("Steam glyph dimensions invalid");
    std::vector<unsigned char> bytes(18+size_t(w)*h*4);bytes[2]=2;bytes[12]=static_cast<unsigned char>(w&255);bytes[13]=static_cast<unsigned char>(w>>8);
    bytes[14]=static_cast<unsigned char>(h&255);bytes[15]=static_cast<unsigned char>(h>>8);bytes[16]=32;bytes[17]=0x28;
    Gdiplus::Rect rect(0,0,INT(w),INT(h));Gdiplus::BitmapData pixels{};
    if(image.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&pixels)!=Gdiplus::Ok)throw std::runtime_error("Steam glyph decode failed");
    for(unsigned y=0;y<h;++y)memcpy(bytes.data()+18+size_t(y)*w*4,static_cast<unsigned char*>(pixels.Scan0)+ptrdiff_t(y)*pixels.Stride,w*4);
    image.UnlockBits(&pixels);exact_file(directory+L"\\"+name,bytes.data(),bytes.size());
}
static void prepare_glyphs(){
    if(glyphs_ready||backend.state()!=deck174::BackendState::Ready)return;
    glyph(deck174::steam006::digital[unsigned(deck174::Button::A)].origin,L"a.tga");
    glyph(deck174::steam006::digital[unsigned(deck174::Button::B)].origin,L"b.tga");
    glyph(deck174::steam006::analog_origin[0],L"stick.tga");glyph(deck174::steam006::analog_origin[3],L"pad.tga");
    glyph(deck174::steam006::analog_origin[4],L"l2.tga");glyph(deck174::steam006::analog_origin[5],L"r2.tga");glyphs_ready=true;
}
static bool poll(){
    if(!prepared||owner!=GetCurrentThreadId())return false;clear_edges();
    auto engine=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll"));
    if(engine&&battle_ready(engine)){stop();return false;}
    // On return from battle, let its owning worker shut down before this UI
    // thread initializes the shared Steam Input interface again.
    if(deck177::busy(deck177::Owner::Battle))return false;
    auto now=GetTickCount64();if(last_poll&&now-last_poll<8)return false;
    float dt=last_poll?std::clamp(float(now-last_poll)*.001f,0.f,.05f):0.f;last_poll=now;
    if(!backend.initialized()){
        if(now<next_attempt)return false;next_attempt=now+2000;
        if(!backend.open_existing(utf8(manifest).c_str(),true)){
            auto state=backend.state();failed=state!=deck174::BackendState::WaitingSteam&&state!=deck174::BackendState::WaitingDevice;return false;
        }
        failed=false;
    }
    HWND window=GetForegroundWindow();DWORD pid{};GetWindowThreadProcessId(window,&pid);
    focused=window&&pid==GetCurrentProcessId();deck174::Frame frame;deck174::Edges edges;
    if(!backend.sample(now*1000,focused,frame)||!normalizer.accept(frame,now*1000,config,edges)){pad_seed=false;cursor178.reset();return false;}
    using deck174::Button;auto pressed=[&](Button b){return (edges.down&deck174::bit(b))!=0;};auto held=[&](Button b){return (frame.buttons&deck174::bit(b))!=0;};
    confirm=pressed(Button::A)||pressed(Button::RightPadClick)||edges.left_fire;cancel=pressed(Button::B);
    // Cursor navigation uses the game's own hit-test/hover path. Never choose
    // a component by geometric guesswork or force focus behind a modal dialog.
    auto stick=deck174::deadzone(frame.left_stick,config.left_deadzone);
    float dx=stick.x*dt*.55f,dy=stick.y*dt*.55f;
    dx+=(held(Button::Right)-held(Button::Left))*dt*.3f;
    dy+=(held(Button::Up)-held(Button::Down))*dt*.3f;
    if(held(Button::RightPadTouch)){
        if(pad_seed){dx+=(frame.right_pad.x-previous_pad.x)*.6f;dy+=(frame.right_pad.y-previous_pad.y)*.6f;}
        previous_pad=frame.right_pad;pad_seed=true;
    }else {pad_seed=false;if(!dx&&!dy)cursor178.reset();}
    RECT rect{};POINT pixel{};
    if((dx||dy)&&GetClientRect(window,&rect)&&rect.right>200&&rect.bottom>200&&GetCursorPos(&pixel)&&ScreenToClient(window,&pixel)){
        int x=pixel.x,y=pixel.y;
        if(!cursor178.move(x,y,rect.right,rect.bottom,dx*rect.right,-dy*rect.bottom))return false;
        pixel.x=x;pixel.y=y;
        if(ClientToScreen(window,&pixel))pointer=SetCursorPos(pixel.x,pixel.y)!=FALSE;
    }
    // Steam may prepare a glyph lazily. Bound retries so an unavailable image
    // cannot decode/throw every UI frame or create accumulating cache files.
    if(!glyphs_ready&&glyph_attempts<3&&now>=next_glyph){
        ++glyph_attempts;next_glyph=now+1500;
        try{prepare_glyphs();glyph_error=false;}catch(...){glyph_error=true;}
    }
    return true;
}
}
#define S2UP_MENU177_EXPORT(name,expression) extern "C" __declspec(dllexport) int __cdecl s2up_deck_##name##177(void* lua){try{if(!caller165(lua))return 0;return (expression)?1:0;}catch(...){frontend177::failed=true;return 0;}}
S2UP_MENU177_EXPORT(prepare,frontend177::prepare())
S2UP_MENU177_EXPORT(supported,device192::enabled())
S2UP_MENU177_EXPORT(poll,frontend177::poll())
S2UP_MENU177_EXPORT(stop,frontend177::stop())
S2UP_MENU177_EXPORT(active,frontend177::prepared)
S2UP_MENU177_EXPORT(focused,frontend177::focused)
S2UP_MENU177_EXPORT(ready,frontend177::backend.initialized()&&frontend177::backend.state()==deck174::BackendState::Ready)
S2UP_MENU177_EXPORT(bindings,frontend177::backend.state()==deck174::BackendState::WaitingBindings)
S2UP_MENU177_EXPORT(failed,frontend177::failed)
S2UP_MENU177_EXPORT(glyphs,frontend177::glyphs_ready)
S2UP_MENU177_EXPORT(glyph_failed,frontend177::glyph_error&&frontend177::glyph_attempts>=3)
S2UP_MENU177_EXPORT(confirm,frontend177::confirm)
S2UP_MENU177_EXPORT(cancel,frontend177::cancel)
S2UP_MENU177_EXPORT(up,frontend177::up)
S2UP_MENU177_EXPORT(down,frontend177::down)
S2UP_MENU177_EXPORT(left,frontend177::left)
S2UP_MENU177_EXPORT(right,frontend177::right)
S2UP_MENU177_EXPORT(pointer,frontend177::pointer)
#undef S2UP_MENU177_EXPORT
