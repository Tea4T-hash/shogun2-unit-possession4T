#pragma once
// Diagnostic-only path policy. The regular Windows Workshop build is unchanged.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <filesystem>
#include <atomic>
#include <stdexcept>
#include <string>
#include <vector>

namespace deck174 { namespace paths {
enum class Use {WriteDestination,ReadFile,ReadDirectory};
inline std::atomic<unsigned> followed_links{0};
#ifdef S2UP_DECK_PATH_FIXTURE
inline bool fixture_proton=false;
#endif
inline bool proton(){
#ifdef S2UP_DECK_PATH_FIXTURE
    if(fixture_proton)return true;
#endif
    return GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"wine_get_version")!=nullptr;
}
inline void need(bool ok,const char* label){
    if(!ok)throw std::runtime_error(std::string("Deck path: ")+label+
        " (Win32="+std::to_string(GetLastError())+")");
}
inline bool local_name(const std::wstring& value){
    if(value.size()<3||value.size()>32760||value[1]!=L':'||value[2]!=L'\\'||
       !((value[0]>=L'A'&&value[0]<=L'Z')||(value[0]>=L'a'&&value[0]<=L'z')))return false;
    for(size_t i=2;i<value.size();++i)if(value[i]<32||value[i]==L':')return false;
    return true;
}
// Read aliases may name a resolved disk file/directory. Output leaves may never
// be links. Directory aliases in ancestors are expected in Proton prefixes and
// Steam libraries; verify the followed handle instead of treating them as NTFS
// cache redirection. This is not a claim of protection from same-user Unix races.
inline bool permitted(bool is_proton,bool leaf,bool directory,Use use){
    if(!is_proton)return false;
    if(!leaf)return directory;
    if(use==Use::ReadDirectory)return directory;
    if(use==Use::ReadFile)return !directory;
    return false;
}
inline void followed(const std::filesystem::path& path,bool directory){
    HANDLE file=CreateFileW(path.c_str(),0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
        nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,nullptr);
    need(file!=INVALID_HANDLE_VALUE,"linked input cannot be opened");
    try{
        BY_HANDLE_FILE_INFORMATION info{};
        need(GetFileType(file)==FILE_TYPE_DISK&&GetFileInformationByHandle(file,&info),
            "linked input is not a disk object");
        need(bool(info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==directory&&
             !(info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT),"linked input did not resolve to the required object");
        std::vector<wchar_t> buffer(32768);
        DWORD n=GetFinalPathNameByHandleW(file,buffer.data(),DWORD(buffer.size()),FILE_NAME_NORMALIZED|VOLUME_NAME_DOS);
        need(n&&n<buffer.size(),"linked input final name is unavailable");
        std::wstring final(buffer.data(),n);
        if(final.compare(0,4,L"\\\\?\\")==0)final.erase(0,4);
        need(local_name(final),"linked input resolved to a non-local path");
        auto root=std::filesystem::path(final).root_path().wstring();
        UINT type=GetDriveTypeW(root.c_str());
        need(type!=DRIVE_REMOTE&&type!=DRIVE_UNKNOWN&&type!=DRIVE_NO_ROOT_DIR,
            "linked input resolved to a remote or unavailable volume");
        ++followed_links;
    }catch(...){CloseHandle(file);throw;}
    CloseHandle(file);
}
inline void validate(const std::filesystem::path& path,Use use){
    auto absolute=std::filesystem::absolute(path).lexically_normal();
    need(local_name(absolute.wstring()),"use a local absolute drive path");
    std::filesystem::path current;
    const bool host_proton=proton();
    for(auto& part:absolute){
        current/=part;
        // Wait for C:\ rather than inspecting the unrelated drive-relative C:.
        if(current==absolute.root_name())continue;
        DWORD attributes=GetFileAttributesW(current.c_str());
        if(attributes==INVALID_FILE_ATTRIBUTES){
            DWORD error=GetLastError();
            need(error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND,"cannot inspect local input");
            continue;
        }
        if(attributes&FILE_ATTRIBUTE_REPARSE_POINT){
            bool directory=(attributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
            need(permitted(host_proton,current==absolute,directory,use),
                "linked output leaf or unsupported Windows reparse path refused");
            followed(current,directory);
        }
    }
}
inline void plain(const std::filesystem::path& path){validate(path,Use::WriteDestination);}
inline void read_file(const std::filesystem::path& path){validate(path,Use::ReadFile);}
inline void read_directory(const std::filesystem::path& path){validate(path,Use::ReadDirectory);}
}} // namespace deck174::paths
