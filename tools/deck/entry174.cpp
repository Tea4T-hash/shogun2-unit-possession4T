// Input-only diagnostic entry. Reuses the established owned-root handoff,
// engine contract and exact lua_gettop guard, but NEVER includes/starts gameplay.
#include "../../native/outputs/shogun2-possession-cleanup-v46/src/identity.h"
#include "pack174_generated.h"
#include "ui_compat174_generated.h"
#include "../../native/common/startup_error167.h"
#include "../../native/deck/steam_input174.h"
#include "../../native/deck/proof174.h"
#include "../../native/deck/config174.h"
#include "../../native/deck/diagnostic174.h"
#include <filesystem>
#include <sstream>

static HMODULE module174;
static SRWLOCK gate174=SRWLOCK_INIT;
static INIT_ONCE bind_once174=INIT_ONCE_STATIC_INIT,start_once174=INIT_ONCE_STATIC_INIT;
static HANDLE root174=INVALID_HANDLE_VALUE;
static bool prepared174{};
static std::wstring game174;
using GetTop174=int(__cdecl*)(void*);
static GetTop174 top174;
struct Lock174 {Lock174(){AcquireSRWLockExclusive(&gate174);}~Lock174(){ReleaseSRWLockExclusive(&gate174);}};
static std::wstring own174(){wchar_t p[32768]{};DWORD n=GetModuleFileNameW(module174,p,32768);require(n&&n<32768,"Probe path unavailable");return p;}
static std::wstring selection174(const std::wstring& path){
    std::ifstream in(path,std::ios::binary);if(!in)return L"";
    std::string data(1048577,'\0');in.read(data.data(),std::streamsize(data.size()));
    require(!in.bad()&&in.gcount()<=1048576,"Selection exceeds bound");data.resize(size_t(in.gcount()));
    bool le=data.size()>1&&(unsigned char)data[0]==255&&(unsigned char)data[1]==254;
    bool be=data.size()>1&&(unsigned char)data[0]==254&&(unsigned char)data[1]==255;
    std::wstring text;size_t start=0;
    if(le||be||(data.size()>1&&data[1]==0)){
        start=le||be?2:0;require((data.size()-start)%2==0,"Truncated UTF16 selection");
        for(size_t i=start;i+1<data.size();i+=2){unsigned a=(unsigned char)data[i],b=(unsigned char)data[i+1];text+=wchar_t(be?(a<<8)|b:(b<<8)|a);}return text;
    }
    if(data.size()>2&&data.substr(0,3)=="\xef\xbb\xbf")start=3;
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,data.data()+start,int(data.size()-start),nullptr,0);
    UINT cp=n?CP_UTF8:CP_ACP;if(!n)n=MultiByteToWideChar(cp,0,data.data()+start,int(data.size()-start),nullptr,0);
    text.resize(n);if(n)MultiByteToWideChar(cp,0,data.data()+start,int(data.size()-start),text.data(),n);return text;
}
static std::string resource174(unsigned id){
    HRSRC r=FindResourceW(module174,MAKEINTRESOURCEW(id),MAKEINTRESOURCEW(10));require(r!=nullptr,"Probe resource missing");
    DWORD n=SizeofResource(module174,r);require(n&&n<=65536,"Probe resource exceeds bound");
    auto data=static_cast<const char*>(LockResource(LoadResource(module174,r)));require(data!=nullptr,"Probe resource unreadable");return {data,n};
}
static std::string read174(const std::wstring& path,size_t limit){
    std::ifstream in(path,std::ios::binary);require(bool(in),"Probe file unavailable");
    std::string s(limit+1,'\0');in.read(s.data(),std::streamsize(s.size()));require(!in.bad()&&size_t(in.gcount())<=limit,"Probe file exceeds bound");s.resize(size_t(in.gcount()));return s;
}
static void cache174(unsigned resource,const std::wstring& path){
    workshop165::plain(path);auto data=resource174(resource);
    Handle f(CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
    if(f.h==INVALID_HANDLE_VALUE){require(GetLastError()==ERROR_FILE_EXISTS,"Cannot create probe resource");require(read174(path,65536)==data,"Foreign probe resource preserved");return;}
    DWORD written{};require(WriteFile(f.h,data.data(),DWORD(data.size()),&written,nullptr)&&written==data.size()&&FlushFileBuffers(f.h),"Probe resource write failed");
}
static uint64_t microseconds174(){LARGE_INTEGER q{},f{};QueryPerformanceCounter(&q);QueryPerformanceFrequency(&f);return uint64_t(q.QuadPart/f.QuadPart)*1000000+uint64_t(q.QuadPart%f.QuadPart)*1000000/uint64_t(f.QuadPart);}
static std::string utf8174(const std::wstring& text){int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),int(text.size()),nullptr,0,nullptr,nullptr);require(n>0,"Probe path encoding failed");std::string s(n,'\0');WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text.data(),int(text.size()),s.data(),n,nullptr,nullptr);return s;}
static void report174(const std::wstring& prefix,const deck174::Proof& proof,unsigned status,
    unsigned keyboard_edges,int gamepad_index,bool shutdown_ok,uint64_t max_poll_us,const char* error,
    deck174::BackendInfo info={},DWORD fault_code=0,uint64_t elapsed_us=0,bool measurement_started=false,bool user_cancelled=false){
    std::ostringstream s;s<<"{\"schema\":174,\"diagnostic_only\":true,\"gameplay_loaded\":false,\"engine_hooks_installed\":false,"
        "\"hardware_validation_pending\":true,\"GamingMode_validation_pending\":true,\"host_sample_timestamps_not_hardware_latency\":true,"
        "\"Steam_client_manifest_override_reset_verified\":false,"
        "\"diagnostic_revision\":"<<deck174::diagnostic_revision<<",\"measurement_started\":"<<(measurement_started?"true":"false")<<
        ",\"user_cancelled\":"<<(user_cancelled?"true":"false")<<",\"measured_elapsed_ms\":"<<elapsed_us/1000<<","
        "\"proton_path_policy_revision\":1,\"followed_local_links\":"<<deck174::paths::followed_links.load()<<","
        "\"touchscreen_implemented\":false,\"pad_pressure_available\":false,\"backend_state\":"<<status<<
        ",\"frames\":"<<proof.frames<<",\"rejected_frames\":"<<proof.rejected<<",\"device_epochs\":"<<proof.connections<<
        ",\"focused_frames\":"<<proof.focused_frames<<",\"unfocused_frames\":"<<proof.unfocused_frames<<
        ",\"native_inputs_observed\":"<<(proof.required_inputs_observed()?"true":"false")<<
        ",\"legacy_key_mouse_edges_observed\":"<<keyboard_edges<<",\"emulated_gamepad_index\":"<<gamepad_index<<
        ",\"shutdown_completed\":"<<(shutdown_ok?"true":"false")<<",\"maximum_poll_us\":"<<max_poll_us<<
        ",\"worker_fault_code\":"<<fault_code<<",\"error\":\"";
    // Error strings are fixed first-party labels, not paths or unbounded exception text.
    for(unsigned i=0;error&&error[i]&&i<96;++i){char c=error[i];if(c>=' '&&c!='"'&&c!='\\')s<<c;}
    s<<"\",\"manifest_accepted\":"<<(info.manifest_accepted?"true":"false")<<
        ",\"native_handles_present\":"<<(info.handles_present?"true":"false")<<
        ",\"connected_controllers\":"<<info.connected_count<<
        ",\"active_physical_button_mask\":"<<info.active_physical_buttons<<
        ",\"mismatched_expected_origin\":"<<info.bad_origin_expected<<
        ",\"mismatched_actual_origin\":"<<info.bad_origin_observed<<",\"analog_modes\":[";
    for(unsigned i=0;i<6;++i){if(i)s<<',';s<<info.analog_modes[i];}
    s<<"],\"buttons\":{";
    for(unsigned i=0;i<deck174::button_count;++i){if(i)s<<',';s<<'"'<<deck174::steam006::digital[i].name<<"\":"<<proof.presses[i];}
    s<<"},\"independent_grips\":"<<proof.independent_grips<<",\"axes\":[";
    for(unsigned i=0;i<proof.axes.size();++i){if(i)s<<',';auto a=proof.axes[i];s<<"{\"samples\":"<<a.samples;
        if(a.samples)s<<",\"min\":"<<a.minimum<<",\"max\":"<<a.maximum<<",\"intermediate\":"<<a.intermediate;s<<'}';}
    s<<"],\"gyro\":[";
    for(unsigned i=0;i<3;++i){if(i)s<<',';auto a=proof.gyro[i];s<<"{\"samples\":"<<a.samples;if(a.samples)s<<",\"min\":"<<a.minimum<<",\"max\":"<<a.maximum;s<<'}';}s<<"],\"accelerometer\":[";
    for(unsigned i=0;i<3;++i){if(i)s<<',';auto a=proof.accelerometer[i];s<<"{\"samples\":"<<a.samples;if(a.samples)s<<",\"min\":"<<a.minimum<<",\"max\":"<<a.maximum;s<<'}';}s<<"]}\n";
    auto bytes=s.str();require(bytes.size()<=16384,"Diagnostic report exceeds bound");
    // At most four reports per content-addressed development build; preserve all
    // previous/foreign files. No per-frame log, overwrite or cleanup watcher.
    for(unsigned i=0;i<4;++i){auto path=prefix+L".deck174-report"+std::to_wstring(i)+L".json";workshop165::plain(path);
        Handle f(CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));
        if(f.h==INVALID_HANDLE_VALUE){if(GetLastError()==ERROR_FILE_EXISTS)continue;return;}
        DWORD written{};WriteFile(f.h,bytes.data(),DWORD(bytes.size()),&written,nullptr);FlushFileBuffers(f.h);return;}
}
// Contain only this diagnostic worker's narrow memory/API faults. Never install
// a process-wide exception handler or consume a real gameplay-thread crash.
static int filter174(DWORD code){return code==EXCEPTION_ACCESS_VIOLATION||
    code==EXCEPTION_ILLEGAL_INSTRUCTION||code==EXCEPTION_DATATYPE_MISALIGNMENT?
        EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH;}
static DWORD guarded174(LPTHREAD_START_ROUTINE routine,void* parameter,DWORD* fault){
    __try{return routine(parameter);}
    __except(filter174(GetExceptionCode())){*fault=GetExceptionCode();return ERROR_INVALID_DATA;}
}
static BOOL CALLBACK find_owner174(HWND window,LPARAM value){
    DWORD pid{};GetWindowThreadProcessId(window,&pid);
    if(pid==GetCurrentProcessId()&&IsWindowVisible(window)&&!GetWindow(window,GW_OWNER)){
        *reinterpret_cast<HWND*>(value)=window;return FALSE;
    }return TRUE;
}
static HWND dialog_owner174(){HWND owner{};EnumWindows(find_owner174,reinterpret_cast<LPARAM>(&owner));return owner;}
static bool start_prompt174(HWND owner){
    return MessageBoxW(owner,
        L"Native Steam Deck input is ready.\n\n"
        L"Touch OK to START. The full 90 seconds begin after this message closes.\n\n"
        L"- Press L4, L5, R4 and R5 separately, several times.\n"
        L"- Move both sticks through full circles.\n"
        L"- Press BOTH sticks straight down until they click (L3 and R3).\n"
        L"- Swipe, touch and click both trackpads through all edges.\n"
        L"- Slowly squeeze L2 and R2 all the way and release.\n"
        L"- Press A/B/X/Y, L1/R1, D-pad, Menu and View.\n"
        L"- Hold the Deck still briefly, then tilt it in all directions.\n\n"
        L"Stay in the main menu. A RESULTS message will appear automatically.\n"
        L"Photograph that message; you do not need to find a file.\n\n"
        L"This is an input test only. Possession/gameplay controls are disabled.\n"
        L"After testing or cancelling, disable this probe and restart the Deck.",
        L"Steam Deck input test - ready",MB_OKCANCEL|MB_ICONINFORMATION|MB_DEFBUTTON2|MB_SETFOREGROUND)==IDOK;
}
static void results_prompt174(HWND owner,const deck174::Proof& proof,deck174::BackendState status,
    deck174::BackendInfo info,const deck174::Measurement& timer,uint64_t elapsed,unsigned keyboard_edges,
    int gamepad,bool shutdown,const char* error){
    auto text=deck174::result_text(proof,status,info,timer,elapsed,keyboard_edges,gamepad,shutdown);
    if(error&&*error){std::wstring label;for(unsigned i=0;error[i]&&i<96;++i)label+=wchar_t((unsigned char)error[i]);text+=L"\n\n"+label;}
    MessageBoxW(owner,text.c_str(),L"Steam Deck input test - RESULTS",MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND);
}
static DWORD WINAPI worker_body174(void*){
    deck174::SteamBackend backend;deck174::Proof proof;unsigned keyboard_edges=0;int gamepad_index=-2;
    uint64_t max_poll_us=0,elapsed_us=0;bool shutdown=true;const char* error="";std::wstring own;
    deck174::Measurement measurement;HWND owner=dialog_owner174();
    try{
        own=own174();
        auto manifest=own+L".deck-actions174.vdf",binding=own+L".deck-native174.vdf";
        // The action manifest uses a relative configuration path. Give both files
        // fixed relative names inside an exclusively owned content-addressed dir.
        auto directory=own+L".deck174";workshop165::plain(directory);
        if(!CreateDirectoryW(directory.c_str(),nullptr))require(GetLastError()==ERROR_ALREADY_EXISTS,"Cannot create diagnostic directory");
        manifest=directory+L"\\deck-actions174.vdf";binding=directory+L"\\deck-native174.vdf";
        cache174(201,manifest);cache174(202,binding);
        auto settings=directory+L"\\controller174.ini";
        if(GetFileAttributesW(settings.c_str())==INVALID_FILE_ATTRIBUTES)cache174(203,settings);
        workshop165::plain(settings);deck174::Config config;std::string reason;
        require(deck174::parse_config(read174(settings,16384),config,reason),reason);
        const auto utf=utf8174(manifest);uint64_t start=GetTickCount64();
        while(!backend.open_existing(utf.c_str())&&GetTickCount64()-start<15000){
            if(backend.state()!=deck174::BackendState::WaitingSteam)break;Sleep(100);}
        if(backend.state()!=deck174::BackendState::WaitingDevice){error="Native Steam Input initialization unavailable";}
        else {
            // A broken binding fails visibly BEFORE asking the user to perform
            // a timed test. These warmup reads never count as exercised inputs.
            deck174::Frame warmup;bool ready=false;start=GetTickCount64();
            while(GetTickCount64()-start<15000){
                if(backend.sample(microseconds174(),true,warmup)){ready=true;break;}Sleep(8);
            }
            if(!ready)error="Native bindings were not ready; the 90-second test did not start";
            else {
                constexpr unsigned keys[]={'W','A','S','D',VK_F9,VK_F10,VK_LBUTTON,VK_RBUTTON};bool previous[8]{};
                uint64_t previous_poll=0;
                deck174::measure(measurement,proof,elapsed_us,microseconds174,[&](){
                    if(!start_prompt174(owner))return false;
                    for(unsigned i=0;i<8;++i)previous[i]=(GetAsyncKeyState(int(keys[i]))&0x8000)!=0;
                    return true;
                },[&](uint64_t now,deck174::Frame& frame){
                    if(previous_poll)max_poll_us=std::max(max_poll_us,now-previous_poll);previous_poll=now;
                    DWORD foreground{};GetWindowThreadProcessId(GetForegroundWindow(),&foreground);
                    bool accepted=backend.sample(now,foreground==GetCurrentProcessId(),frame);
                    gamepad_index=backend.emulated_gamepad_index();
                    for(unsigned i=0;i<8;++i){bool down=(GetAsyncKeyState(int(keys[i]))&0x8000)!=0;if(down&&!previous[i])++keyboard_edges;previous[i]=down;}
                    return accepted;
                },[](){Sleep(8);});
            }
        }
    }catch(...){error="Diagnostic resource or API initialization failed";}
    if(measurement.started&&!elapsed_us)elapsed_us=measurement.elapsed(microseconds174());
    const unsigned status=unsigned(backend.state());auto info=backend.info();shutdown=backend.close();
    try{report174(own,proof,status,keyboard_edges,gamepad_index,shutdown,max_poll_us,error,info,0,
        elapsed_us,measurement.started,measurement.cancelled);}catch(...){}
    if(!measurement.cancelled)try{results_prompt174(owner,proof,deck174::BackendState(status),info,
        measurement,elapsed_us,keyboard_edges,gamepad_index,shutdown,error);}catch(...){}
    return 0;
}
static DWORD WINAPI worker174(void* parameter){
    DWORD fault{};DWORD result=guarded174(worker_body174,parameter,&fault);
    if(fault){try{deck174::Proof empty;report174(own174(),empty,unsigned(deck174::BackendState::Failed),
        0,-2,false,0,"Diagnostic worker memory fault; origin not established",{},fault);}catch(...){} }
    return result;
}
static BOOL CALLBACK bind174(PINIT_ONCE,void*,void**){try{
#ifdef S2UP165_LUA_FIXTURE
    wchar_t path[32768]{};DWORD n=GetEnvironmentVariableW(L"S2UP165_FIXTURE_GAME",path,32768);require(n&&n<32768,"Fixture path missing");game174=path;
    top174=[](void* p)->int{uintptr_t a{},b{};memcpy(&a,(char*)p+8,4);memcpy(&b,(char*)p+12,4);return int((a-b)/8);};
#else
    wchar_t host[32768]{},path[32768]{};DWORD n=GetModuleFileNameW(nullptr,host,32768);require(n&&n<32768,"Host path unavailable");
    std::filesystem::path exe(host);require(!_wcsicmp(exe.filename().c_str(),L"shogun2.exe")||!_wcsicmp(exe.filename().c_str(),L"shogun2.retail.exe"),"Wrong diagnostic host");game174=exe.parent_path().wstring();
    HMODULE engine=GetModuleHandleW(L"Empire.Retail.dll");require(engine&&GetModuleFileNameW(engine,path,32768),"Engine unavailable");
    auto sha=hash_file(path);auto binding=engine172::admit(path,sha);engine172::live_hooks(uintptr_t(engine),3,binding);
    require(sha==hash_file(game174+L"\\Empire.Retail.dll"),"Loaded engine differs from folder");
    const unsigned char expected[]={0x8b,0x4c,0x24,0x04,0x8b,0x41,0x08,0x2b,0x41,0x0c,0xc1,0xf8,0x03,0xc3};
    auto entry=(unsigned char*)engine+0xea2d60;unsigned char actual[sizeof expected]{};SIZE_T bytes{};
    require(ReadProcessMemory(GetCurrentProcess(),entry,actual,sizeof actual,&bytes)&&bytes==sizeof actual&&!memcmp(actual,expected,sizeof actual),"Lua gettop guard differs");top174=reinterpret_cast<GetTop174>(entry);
    HMODULE pin{};require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&module174),&pin)!=0,"Cannot retain diagnostic module");
#endif
}catch(const std::exception& e){top174=nullptr;startup167::remember(e.what());}return TRUE;}
static bool caller174(void* p){InitOnceExecuteOnce(&bind_once174,bind174,nullptr,nullptr);return top174&&top174(p)==1;}
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_error167(void*){
#ifndef S2UP165_LUA_FIXTURE
    startup167::notify();
#endif
    return 0;
}
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* p){Lock174 guard;try{
    if(!caller174(p))return 0;prepared174=false;if(root174!=INVALID_HANDLE_VALUE){CloseHandle(root174);root174=INVALID_HANDLE_VALUE;}
    auto bytes=workshop168::selected_root(game174,selection174(game174+L"\\used_mods.txt"),hash_file(own174()));
    root174=workshop165::transient_root168(own174()+L".root.luac",bytes);require(root174!=INVALID_HANDLE_VALUE,"Cannot prepare owned root");prepared174=true;return 1;
}catch(const std::exception& e){if(root174!=INVALID_HANDLE_VALUE){CloseHandle(root174);root174=INVALID_HANDLE_VALUE;}startup167::remember(e.what());return 0;}}
static BOOL CALLBACK start174(PINIT_ONCE,void*,void**){
#ifdef S2UP165_LUA_FIXTURE
    return TRUE;
#else
    HANDLE thread=CreateThread(nullptr,0,worker174,nullptr,0,nullptr);if(!thread)return FALSE;CloseHandle(thread);return TRUE;
#endif
}
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_start165(void* p){Lock174 guard;try{
    if(!caller174(p)||!prepared174)return 0;
    if(root174!=INVALID_HANDLE_VALUE){CloseHandle(root174);root174=INVALID_HANDLE_VALUE;}
    return InitOnceExecuteOnce(&start_once174,start174,nullptr,nullptr)?1:0;
}catch(const std::exception& e){startup167::remember(e.what());return 0;}}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,void*){if(reason==DLL_PROCESS_ATTACH)module174=module;return TRUE;}
