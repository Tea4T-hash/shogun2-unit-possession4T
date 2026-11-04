// Lua bridge runs on the UI's Lua thread; gameplay startup remains in its worker.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include "../../native/work/autoload99-preview/integrated.h"
#include "pack165.h"
#include "ui_compat168.h"
#include "../../native/work/autoload99-preview/workshop_assets165.h"
static HMODULE module165;
static INIT_ONCE host_once165=INIT_ONCE_STATIC_INIT,start_once165=INIT_ONCE_STATIC_INIT;
static std::wstring game165;
static HANDLE root_guard165=INVALID_HANDLE_VALUE;
static bool prepared165=false;
static SRWLOCK state_lock165=SRWLOCK_INIT;
struct StateGuard165 {StateGuard165(){AcquireSRWLockExclusive(&state_lock165);}~StateGuard165(){ReleaseSRWLockExclusive(&state_lock165);}};
using LuaGetTop165=int(__cdecl*)(void*);
static LuaGetTop165 get_top165=nullptr;
static workshop165::LiveReceipt166 live_receipt166;
static std::wstring own165(){wchar_t path[32768]{};DWORD n=GetModuleFileNameW(module165,path,32768);require(n&&n<32768,"Native cache path unavailable");return path;}
static std::wstring process_stamp165(){FILETIME created{},exited{},kernel{},user{};require(GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel,&user)!=0,"Process identity unavailable");return std::to_wstring((uint64_t(created.dwHighDateTime)<<32)|created.dwLowDateTime);}
// Kit v10: each build left its own S2UP-Workshop165-<fingerprint>.dll in LOCALAPPDATA.
// Delete the other builds' files; this build's DLL (and its own temporary files) stay.
// A file still loaded by another running game cannot be deleted and is left alone.
static void retire_old_bridges173(){
    try{
        const std::filesystem::path own(own165());const auto folder=own.parent_path();const auto mine=own.filename().wstring();
        WIN32_FIND_DATAW d{};HANDLE f=FindFirstFileW((folder/L"S2UP-Workshop165-*").c_str(),&d);if(f==INVALID_HANDLE_VALUE)return;
        do{
            if(d.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT))continue;
            const std::wstring n=d.cFileName;if(n.rfind(mine,0)==0)continue;
            DeleteFileW((folder/n).c_str());
        }while(FindNextFileW(f,&d));
        FindClose(f);
    }catch(...){}
}
static void failure165(const char* why){DWORD error=GetLastError();startup167::remember(why);InterlockedExchange(&autoload_error,error?LONG(error):ERROR_INVALID_DATA);}
// Zero Lua results: no engine API or Lua stack access, safe at any arity.
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_error167(void*){startup167::notify();return 0;}
#ifdef S2UP165_LUA_FIXTURE
static int __cdecl fixture_top165(void* value){auto state=(unsigned char*)value;uintptr_t top=0,base=0;memcpy(&top,state+8,4);memcpy(&base,state+12,4);return int((top-base)/8);}
#endif
static BOOL CALLBACK bind165(PINIT_ONCE,void*,void**){try{
#ifdef S2UP165_LUA_FIXTURE
    wchar_t input[32768]{};DWORD n=GetEnvironmentVariableW(L"S2UP165_FIXTURE_GAME",input,32768);require(n&&n<32768,"Fixture game path missing");game165=input;get_top165=fixture_top165;
#else
    wchar_t exe[32768]{},engine_path[32768]{};DWORD n=GetModuleFileNameW(nullptr,exe,32768);require(n&&n<32768,"Game path unavailable");
    std::filesystem::path host(exe);auto name=host.filename().wstring();require(!_wcsicmp(name.c_str(),L"shogun2.exe")||!_wcsicmp(name.c_str(),L"shogun2.retail.exe"),"Not a supported game host");
    game165=host.parent_path().wstring();HMODULE engine=GetModuleHandleW(L"Empire.Retail.dll");require(engine!=nullptr,"Engine not loaded");
    require(GetModuleFileNameW(engine,engine_path,32768)>0,"Engine path unavailable");
    const auto loaded_engine_sha170=hash_file(engine_path);
    const auto binding172=engine172::admit(engine_path,loaded_engine_sha170);
    engine172::live_hooks(reinterpret_cast<uintptr_t>(engine),3,binding172);
    // Pinned lua_gettop: cdecl int(lua_State*), read-only, 14 original bytes.
    const unsigned char expected[]={0x8b,0x4c,0x24,0x04,0x8b,0x41,0x08,0x2b,0x41,0x0c,0xc1,0xf8,0x03,0xc3};
    auto entry=(unsigned char*)engine+0xea2d60;unsigned char actual[sizeof expected]{};SIZE_T read=0;
    require(ReadProcessMemory(GetCurrentProcess(),entry,actual,sizeof actual,&read)&&read==sizeof actual&&!memcmp(actual,expected,sizeof actual),"Lua API bytes differ");
    get_top165=reinterpret_cast<LuaGetTop165>(entry);
#endif
    const auto folder_engine_sha170=hash_file(game165+L"\\Empire.Retail.dll");
    (void)engine172::admit(game165+L"\\Empire.Retail.dll",folder_engine_sha170);
#ifndef S2UP165_LUA_FIXTURE
    require(folder_engine_sha170==loaded_engine_sha170,"Loaded engine and game-folder engine differ");
#endif
#ifndef S2UP165_LUA_FIXTURE
    HMODULE pinned{};require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&module165),&pinned)!=0,"Cannot retain Workshop bridge for this process");
#endif
}catch(const std::exception& error){get_top165=nullptr;failure165(error.what());}return TRUE;}
static bool caller165(void* state){InitOnceExecuteOnce(&host_once165,bind165,nullptr,nullptr);return get_top165&&get_top165(state)==1;}
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* state){StateGuard165 guard;try{
    if(!caller165(state))return 0;
    prepared165=false;if(root_guard165!=INVALID_HANDLE_VALUE){CloseHandle(root_guard165);root_guard165=INVALID_HANDLE_VALUE;}
    auto selection_path168=game165+L"\\used_mods.txt";
    std::wstring selection_text169;
#ifdef S2UP165_LUA_FIXTURE
    wchar_t selection_fixture168[32768]{};DWORD count168=GetEnvironmentVariableW(L"S2UP168_FIXTURE_SELECTION",selection_fixture168,32768);
    if(count168&&count168<32768)selection_text169=read_text(selection_fixture168);
#else
    selection_text169=read_text(selection_path168);
#endif
    auto bytes=workshop168::selected_root(game165,selection_text169,hash_file(own165()));
    auto path=own165()+L".root.luac";
    root_guard165=workshop165::transient_root168(path,bytes);
    require(root_guard165!=INVALID_HANDLE_VALUE,"Cannot protect original root while Lua loads it");
    std::vector<unsigned char> actual(bytes.size());DWORD read=0;LARGE_INTEGER size{},zero{};
    require(SetFilePointerEx(root_guard165,zero,nullptr,FILE_BEGIN)!=0,"Cannot rewind UI handoff");
    require(GetFileSizeEx(root_guard165,&size)&&size.QuadPart==bytes.size()&&ReadFile(root_guard165,actual.data(),DWORD(actual.size()),&read,nullptr)&&read==actual.size()&&actual==bytes,"Protected root cache differs");
    prepared165=true;
    // Sole Lua caller supplies true. Return that existing argument as one result;
    // lua_gettop checked the arity, no Lua internals/stack writes are performed.
    return 1;
}catch(const std::exception& error){if(root_guard165!=INVALID_HANDLE_VALUE){CloseHandle(root_guard165);root_guard165=INVALID_HANDLE_VALUE;}failure165(error.what());return 0;}}
static BOOL CALLBACK begin165(PINIT_ONCE,void*,void**){
#ifdef S2UP165_LUA_FIXTURE
    return TRUE; // Fixture must never load gameplay, hooks or a HUD.
#else
    if(!startup160::publish(true))return FALSE; // Third-person by default; P switches. No chooser.
    HANDLE thread=CreateThread(nullptr,0,integrated_worker,module165,0,nullptr);
    if(!thread)return FALSE;CloseHandle(thread);return TRUE;
#endif
}
extern "C" __declspec(dllexport) int __cdecl s2up_workshop_start165(void* state){StateGuard165 guard;try{
    if(!caller165(state)||!prepared165)return 0;
    if(root_guard165!=INVALID_HANDLE_VALUE){CloseHandle(root_guard165);root_guard165=INVALID_HANDLE_VALUE;}
    if(!InitOnceExecuteOnce(&start_once165,begin165,nullptr,nullptr))return 0;
    auto stamp=process_stamp165();auto path=own165()+L".pid"+std::to_wstring(GetCurrentProcessId())+L".start"+stamp+L".json";
    std::string text="{\"schema\":1,\"test\":"+std::to_string(EMBEDDED_WORKSHOP_TEST)+",\"pid\":"+std::to_string(GetCurrentProcessId())+",\"process_start_filetime\":\""+std::string(stamp.begin(),stamp.end())+"\",\"native_entry\":true,\"local_original_script\":true,\"chooser\":false,"
#ifdef S2UP165_LUA_FIXTURE
        "\"fixture\":true,\"worker_requested\":false}\n";
#else
        "\"fixture\":false,\"worker_requested\":true}\n";
#endif
    std::vector<unsigned char> bytes(text.begin(),text.end());live_receipt166.write(path,bytes);retire_old_bridges173();return 1;
}catch(const std::exception& error){failure165(error.what());return 0;}}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,void*){if(reason==DLL_PROCESS_ATTACH)module165=module;return TRUE;}
