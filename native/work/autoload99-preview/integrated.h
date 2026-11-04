#pragma once
#include "../../outputs/shogun2-possession-cleanup-v46/src/identity.h"
#include "../../outputs/shogun2-possession-cleanup-v46/src/protocol.h"
#include "embedded_hashes.h"
#include "../../common/startup_view160.h"
#include "game_audio162.h"
#include "../../common/startup_error167.h"
#include "../../common/startup_gate168.h"
#include <shlobj.h>
#include <regex>
#include <filesystem>
#ifdef S2FP_LOCAL_ASSETS
#include "../../common/local_assets159.h"
#endif
static volatile LONG autoload_stage,autoload_error;
static std::wstring runtime_folder;
static std::wstring read_text(const std::wstring& path){
    std::ifstream in(path,std::ios::binary); if(!in)return L"";
    std::string data(1048577,'\0');in.read(&data[0],std::streamsize(data.size()));
    require(!in.bad()&&in.gcount()<=1048576,"Mod selection file too large or unreadable");data.resize(size_t(in.gcount()));
    std::wstring text;size_t start=0;
    bool le=data.size()>1&&static_cast<unsigned char>(data[0])==255&&static_cast<unsigned char>(data[1])==254;
    bool be=data.size()>1&&static_cast<unsigned char>(data[0])==254&&static_cast<unsigned char>(data[1])==255;
    if(le||be||(data.size()>1&&data[1]==0)){
        start=le||be?2:0;require((data.size()-start)%2==0,"Truncated UTF16 mod selection");for(size_t i=start;i+1<data.size();i+=2){unsigned a=static_cast<unsigned char>(data[i]),b=static_cast<unsigned char>(data[i+1]);text+=wchar_t(be?(a<<8)|b:(b<<8)|a);}return text;
    }
    if(data.size()>2&&data.substr(0,3)=="\xef\xbb\xbf")start=3;
    int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,data.data()+start,int(data.size()-start),nullptr,0);
    UINT cp=n?CP_UTF8:CP_ACP;if(!n)n=MultiByteToWideChar(cp,0,data.data()+start,int(data.size()-start),nullptr,0);
    text.resize(n);if(n)MultiByteToWideChar(cp,0,data.data()+start,int(data.size()-start),&text[0],n);return text;
}
static bool selected_text(std::wstring text){
    text=std::regex_replace(text,std::wregex(L"/\\*[\\s\\S]*?\\*/|//[^\\r\\n]*|#[^\\r\\n]*"),L"");
    std::wregex pattern(L"^\\s*mod\\s+(?:\"shogun2_possession_checkpoint45\\.pack\"|shogun2_possession_checkpoint45\\.pack)\\s*;",std::regex_constants::icase);
    std::wistringstream lines(text);std::wstring line;while(std::getline(lines,line))if(std::regex_search(line,pattern))return true;return false;
}
static std::wstring known_folder(int id){wchar_t path[MAX_PATH];require(SUCCEEDED(SHGetFolderPathW(nullptr,id|CSIDL_FLAG_CREATE,nullptr,SHGFP_TYPE_CURRENT,path)),"User folder unavailable");return path;}
static bool selected(){
#ifdef POSSESSION_WORKSHOP_BUILD
    // This worker is reachable only through the enabled pack's Lua entry point.
    // Enabling a pack via Workshop need not produce the installer's fixed directive.
    return true;
#else
    return selected_text(read_text(known_folder(CSIDL_APPDATA)+L"\\The Creative Assembly\\Shogun2\\scripts\\user.script.txt"));
#endif
}
static void record(LONG stage,const std::string& detail){
    InterlockedExchange(&autoload_stage,stage);
    (void)detail; // Stage is observable in memory; routine startup creates no log.
}
static std::wstring extract(HMODULE module,int id,const wchar_t* name,const char* expected){
    std::wstring path=runtime_folder+L"\\"+name;
    if(GetFileAttributesW(path.c_str())!=INVALID_FILE_ATTRIBUTES){require(hash_file(path)==expected,"Cached mod file differs; preserved without overwrite");return path;}
    HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(id),MAKEINTRESOURCEW(10));require(resource!=nullptr,"Embedded mod component missing");
    HGLOBAL loaded=LoadResource(module,resource);const void* bytes=LockResource(loaded);DWORD size=SizeofResource(module,resource);require(bytes&&size,"Embedded mod component empty");
    Handle file(CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr));require(file.h!=INVALID_HANDLE_VALUE,"Cannot extract mod component");
    DWORD written=0;require(WriteFile(file.h,bytes,size,&written,nullptr)&&written==size,"Cannot write mod component");
    CloseHandle(file.h);file.h=nullptr;require(hash_file(path)==expected,"Extracted mod component failed integrity check");return path;
}
#ifdef S2FP_LOCAL_ASSETS
static void cache_local159(const std::wstring& source,const std::wstring& target,unsigned kind,bool share_bytes=false){
    assets159::Store input;require(input.load(source,kind),"Local game resources missing or corrupt; run the local-assets setup again");
    if(GetFileAttributesW(target.c_str())==INVALID_FILE_ATTRIBUTES){
        auto stage=target+L".local159.pending166";
        bool owned=false;
        try{
            if(GetFileAttributesW(stage.c_str())==INVALID_FILE_ATTRIBUTES){
                // On the same volume, updates share the immutable bytes.
                if(!share_bytes||!CreateHardLinkW(stage.c_str(),source.c_str(),nullptr))
                    require(CopyFileW(source.c_str(),stage.c_str(),TRUE)!=0,"Cannot stage local resource cache");
                owned=true;
            }
            require(hash_file(stage)==hash_file(source),"Staged local resources differ");
            require(MoveFileExW(stage.c_str(),target.c_str(),MOVEFILE_WRITE_THROUGH)!=0,"Cannot commit local resource cache; existing files preserved");
        }catch(...){if(owned)DeleteFileW(stage.c_str());throw;}
    }
    require(hash_file(source)==hash_file(target),"Local resource cache differs; preserved without overwrite");
    assets159::Store output;require(output.load(target,kind),"Local resource cache failed integrity check");
}
#endif
static uint32_t local_u32(uintptr_t address){uint32_t value=0;SIZE_T read=0;require(address>=65536&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&value,4,&read)&&read==4,"Battle not ready");return value;}
static bool battle_ready(uintptr_t base){try{auto root=local_u32(base+0x1bc8180);auto env=local_u32(root+0x110);auto common=local_u32(env+8);auto phase=local_u32(common+0xb4);auto id=local_u32(phase+0x14);return id>=5&&id<=8;}catch(...){return false;}}
static bool other_adapter(){Handle snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,GetCurrentProcessId()));require(snap.h!=INVALID_HANDLE_VALUE,"Module inventory failed");MODULEENTRY32W entry{};entry.dwSize=sizeof entry;
    require(Module32FirstW(snap.h,&entry)!=0,"Module inventory unavailable");do{std::wstring name=entry.szModule;std::transform(name.begin(),name.end(),name.begin(),towlower);if(name.find(L"possession-v")==0)return true;}while(Module32NextW(snap.h,&entry));return false;}
// A failed later component must not leave an interactive partial installation.
// Only call for a component this worker loaded, before publishing readiness.
static void stop_component168(bool owned,bool qte){
    if(!owned)return;
    wchar_t name[80]{};
    if(qte)swprintf_s(name,L"Local\\Shogun2QteObserver79-%lu",GetCurrentProcessId());
    else mapping_name(name,GetCurrentProcessId());
    struct Prefix {uint32_t magic,version,pid,size;LONG stop;};
    Handle map(OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,name));
    if(!map.h)return;
    auto p=static_cast<Prefix*>(MapViewOfFile(map.h,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(Prefix)));
    if(!p)return;
    if(p->magic==(qte?0x31545153u:POC_MAGIC)&&p->version==(qte?79u:46u)&&
       p->pid==GetCurrentProcessId()&&(qte?(p->size>=32&&p->size<4096):p->size==sizeof(Status)))InterlockedExchange(&p->stop,1);
    UnmapViewOfFile(p);
}
static std::string component_error168(HMODULE module,const char* fallback){
    using Read=unsigned(__cdecl*)(char*,unsigned);
    auto read=reinterpret_cast<Read>(GetProcAddress(module,"S2UPInitializationError168"));
    if(!read)read=reinterpret_cast<Read>(GetProcAddress(module,"_S2UPInitializationError168"));
    char text[1025]{};unsigned n=read?read(text,sizeof text):0;
    return n&&n<sizeof text?std::string(text,n):std::string(fallback);
}
#if defined(POSSESSION_WORKSHOP_BUILD) && defined(S2FP_LOCAL_ASSETS)
static void workshop_assets165(HMODULE module,const std::wstring& game);
#endif
// Kit v10: every build unpacked into its own cache folder and old folders were never removed.
// Keep only this build's folder. Only plain files directly inside "workshop-*"/"integrated-*"
// sibling folders are deleted; a file still in use by another running game is left alone.
static void retire_old_runtimes173(){
    try{
        const std::filesystem::path own(runtime_folder);const auto parent=own.parent_path();const auto mine=own.filename().wstring();
        WIN32_FIND_DATAW d{};HANDLE f=FindFirstFileW((parent/L"*").c_str(),&d);if(f==INVALID_HANDLE_VALUE)return;
        do{
            const std::wstring n=d.cFileName;
            if(!(d.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)||(d.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT))continue;
            if(n==mine||(n.rfind(L"workshop-",0)!=0&&n.rfind(L"integrated-",0)!=0))continue;
            const auto dir=parent/n;WIN32_FIND_DATAW e{};HANDLE g=FindFirstFileW((dir/L"*").c_str(),&e);
            if(g!=INVALID_HANDLE_VALUE){
                do{if(!(e.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)))DeleteFileW((dir/e.cFileName).c_str());}while(FindNextFileW(g,&e));
                FindClose(g);
            }
            RemoveDirectoryW(dir.c_str());
        }while(FindNextFileW(f,&d));
        FindClose(f);
    }catch(...){}
}
static DWORD WINAPI integrated_worker(void* argument){bool owned_adapter168=false,owned_qte168=false,owned_gate168=false;try{
    wchar_t exe[MAX_PATH];require(GetModuleFileNameW(nullptr,exe,MAX_PATH)>0,"Game path unavailable");
    const wchar_t* name=wcsrchr(exe,L'\\');name=name?name+1:exe;
    if(_wcsicmp(name,L"shogun2.exe")&&_wcsicmp(name,L"shogun2.retail.exe")){InterlockedExchange(&autoload_stage,1);return 0;}
    runtime_folder=known_folder(CSIDL_LOCAL_APPDATA)+L"\\Shogun2PossessionCheckpoint45\\"
#ifdef POSSESSION_WORKSHOP_BUILD
        L"workshop-"
#else
        L"integrated-"
#endif
        +EMBEDDED_BUILD;
    require(SHCreateDirectoryExW(nullptr,runtime_folder.c_str(),nullptr)==ERROR_SUCCESS||GetFileAttributesW(runtime_folder.c_str())!=INVALID_FILE_ATTRIBUTES,"Mod cache unavailable");
    retire_old_runtimes173();
    record(2,"Integrated mod startup; no external installation or PowerShell required");
    if(!selected()){record(3,"Mod unchecked; no gameplay or HUD loaded");return 0;}
    std::wstring game=exe;game=game.substr(0,game.find_last_of(L'\\'));
#ifndef POSSESSION_WORKSHOP_BUILD
    require(hash_file(game+L"\\data\\shogun2_possession_checkpoint45.pack")==EMBEDDED_PACK_SHA,"Selected mod pack differs");
#endif
#ifdef S2FP_LOCAL_ASSETS
#ifdef POSSESSION_WORKSHOP_BUILD
    workshop_assets165(static_cast<HMODULE>(argument),game);
#else
    // Prepare outside the game process during setup; validate before any native hook loads.
    for(unsigned kind:{1u,2u}){
        const wchar_t* file=kind==1?L"hud-assets159.s2fp":L"qte-assets159.s2fp";
        std::wstring source=game+L"\\data\\shogun2_possession_local_assets159\\"+file;
        cache_local159(source,runtime_folder+L"\\"+file,kind);
    }
#endif
#endif
    auto adapter=extract(static_cast<HMODULE>(argument),101,L"possession-v46.dll",EMBEDDED_ADAPTER_SHA);
    auto hud=extract(static_cast<HMODULE>(argument),102,L"possession-hud.exe",EMBEDDED_HUD_SHA);
    auto qte=extract(static_cast<HMODULE>(argument),103,L"qte-playable-v79.dll",EMBEDDED_QTE_SHA);
    // Choice is inherited from the Steam prelaunch helper. Never show a dialog
    // from a game worker: the fullscreen window already exists at this point.
    bool starting_third160=startup160::third_person();
    record(4,starting_third160?"Third-person default selected; waiting for a battle":"First-person default selected; waiting for a battle");
    uintptr_t base=0;for(;;){if(!selected()){record(3,"Mod unchecked while waiting");return 0;}
        HMODULE engine=GetModuleHandleW(L"Empire.Retail.dll");if(engine){base=reinterpret_cast<uintptr_t>(engine);if(battle_ready(base))break;}Sleep(1000);}
    wchar_t engine_path[MAX_PATH];require(GetModuleFileNameW(reinterpret_cast<HMODULE>(base),engine_path,MAX_PATH)>0,"Engine path unavailable");
    (void)engine172::admit(engine_path,hash_file(engine_path));
    wchar_t coordinator_name168[80]{};swprintf_s(coordinator_name168,L"Local\\S2UP-Startup168-%lu",GetCurrentProcessId());
    Handle coordinator168(CreateMutexW(nullptr,FALSE,coordinator_name168));
    require(coordinator168.h&&GetLastError()!=ERROR_ALREADY_EXISTS,"Another possession startup is already in progress");
    require(!other_adapter(),"Another possession adapter is already loaded; restart to use the installed mod");
    require(GetModuleHandleW(L"qte-playable-v79.dll")==nullptr,"Another QTE79 is already resident; restart first");
    // Also detect an older/renamed runtime before trusting a status channel or
    // requesting cleanup. Loading our DLL does not confer ownership of an
    // already existing mapping with the same protocol name.
    wchar_t status_name168[80]{};mapping_name(status_name168,GetCurrentProcessId());
    for(unsigned kind168=0;kind168<2;++kind168){
        if(kind168)swprintf_s(status_name168,L"Local\\Shogun2QteObserver79-%lu",GetCurrentProcessId());
        SetLastError(ERROR_SUCCESS);Handle previous168(OpenFileMappingW(FILE_MAP_READ,FALSE,status_name168));
        require(!previous168.h&&GetLastError()==ERROR_FILE_NOT_FOUND,"A possession/QTE control channel already exists; restart before using this build");
    }
    require(startup168::pending(),"Cannot coordinate component startup");
    owned_gate168=true;
    auto adapter_module168=LoadLibraryW(adapter.c_str());require(adapter_module168!=nullptr,"Embedded possession engine failed to load");record(5,"Possession engine loaded; waiting for initialization");
    owned_adapter168=true;
    wchar_t map_name[80];mapping_name(map_name,GetCurrentProcessId());bool ready=false;
    for(int i=0;i<200;++i){Handle map(OpenFileMappingW(FILE_MAP_READ,FALSE,map_name));if(map.h){auto state=static_cast<const Status*>(MapViewOfFile(map.h,FILE_MAP_READ,0,0,sizeof(Status)));if(state){bool valid=state->magic==POC_MAGIC&&state->version==46&&state->pid==GetCurrentProcessId()&&state->size==sizeof(Status);bool error=valid&&state->error!=0;ready=valid&&state->ready&&!state->stop;UnmapViewOfFile(state);if(error)throw std::runtime_error(component_error168(adapter_module168,"Possession engine rejected initialization"));if(ready)break;}}Sleep(50);}
    require(ready,"Possession engine did not become ready");
    require(GetModuleHandleW(L"qte-playable-v79.dll")==nullptr,"Another QTE79 is already resident; restart first");
    auto qte_module168=LoadLibraryW(qte.c_str());require(qte_module168!=nullptr,"Embedded QTE failed to load");
    owned_qte168=true;
    struct QtePrefix {uint32_t magic,version,pid,size;LONG stop,ready,mode,error;};
    swprintf_s(map_name,L"Local\\Shogun2QteObserver79-%lu",GetCurrentProcessId());bool qte_ready=false;
    for(int i=0;i<200;++i){Handle map(OpenFileMappingW(FILE_MAP_READ,FALSE,map_name));if(map.h){
        auto q=static_cast<const QtePrefix*>(MapViewOfFile(map.h,FILE_MAP_READ,0,0,sizeof(QtePrefix)));
        if(q){bool valid=q->magic==0x31545153&&q->version==79&&q->pid==GetCurrentProcessId()&&q->size>=sizeof(QtePrefix)&&q->size<4096;
            bool error=valid&&q->error!=0;qte_ready=valid&&q->ready&&!q->stop;UnmapViewOfFile(q);
            if(error)throw std::runtime_error(component_error168(qte_module168,"QTE rejected initialization"));if(qte_ready)break;}}
        Sleep(50);}
    require(qte_ready,"Embedded QTE did not become ready");
    STARTUPINFOW startup{};startup.cb=sizeof startup;startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=SW_HIDE;PROCESS_INFORMATION child{};std::wstring command=L"\""+hud+L"\"";
    require(CreateProcessW(hud.c_str(),&command[0],nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,runtime_folder.c_str(),&startup,&child)!=0,"Embedded HUD failed to start");
    CloseHandle(child.hThread);DWORD result=WaitForSingleObject(child.hProcess,1500);if(result==WAIT_OBJECT_0){DWORD code=0;GetExitCodeProcess(child.hProcess,&code);CloseHandle(child.hProcess);require(code==0,"Embedded HUD exited with an error");}else CloseHandle(child.hProcess);
    require(startup168::ready(),"Cannot publish complete component startup");
    record(6,"Ready: equipment81, authentic HUD81 and restyled QTE79 started automatically");
#ifdef S2FP_LOCAL_ASSETS
    // This existing loader worker owns the audio stream for the game process.
    // Keep only the verified PCM alive until the voice handle has closed.
    assets159::Store audio_store;
    if(!audio_store.load(runtime_folder+L"\\hud-assets159.s2fp",1)){record(6,"Friendly-fire audio unavailable: local resources rejected");return 0;}
    auto blob=audio_store.get(301);
    if(!blob.bytes||!blob.size){record(6,"Ready without optional English advisor voice");return 0;}
    std::vector<unsigned char> voice(blob.bytes,blob.bytes+blob.size);
    audio_store.clear(); // retain only the 134 KiB voice, not artwork/fonts in a 32-bit game
    gameaudio162::Feedback feedback;
    wchar_t owner_name[80];mapping_name(owner_name,GetCurrentProcessId());
    Handle owner_map(OpenFileMappingW(FILE_MAP_READ,FALSE,owner_name));
    auto owner=owner_map.h?static_cast<const Status*>(MapViewOfFile(owner_map.h,FILE_MAP_READ,0,0,sizeof(Status))):nullptr;
    if(!owner){record(6,"Friendly-fire audio unavailable: possession status missing");return 0;}
    record(6,"Friendly-fire voice uses the game-process Windows audio stream");
    for(;;){Status status{};bool stable=gameaudio162::snapshot(owner,status);if(stable&&status.stop)break;
        feedback.poll(status,stable,voice.data(),voice.size());Sleep(20);}
    UnmapViewOfFile(owner);
#endif
    return 0;
}catch(const std::exception& error){
    if(autoload_stage<6&&owned_gate168){startup168::failed();stop_component168(owned_qte168,true);stop_component168(owned_adapter168,false);}
    LONG stage=autoload_stage;DWORD code=GetLastError();InterlockedExchange(&autoload_error,LONG(code?code:ERROR_INVALID_DATA));
    record(7,error.what());startup167::remember(error.what());
#ifdef POSSESSION_WORKSHOP_BUILD
    if(stage<6)startup167::notify();
#endif
    return 1;}}

