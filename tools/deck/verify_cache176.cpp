// Real owned resource bundles/NTFS links; emulate the documented same-inode
// sharing check at rename only. This fixture is not a Proton hardware test.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <string>
#include <iostream>
static std::wstring source176;
static unsigned alias_checks176,blocked176;
static DWORD last_block176;
static BOOL WINAPI rename176(LPCWSTR from,LPCWSTR to,DWORD flags){
    HANDLE a=CreateFileW(source176.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    HANDLE b=CreateFileW(from,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
    BY_HANDLE_FILE_INFORMATION x{},y{};
    bool same=a!=INVALID_HANDLE_VALUE&&b!=INVALID_HANDLE_VALUE&&GetFileInformationByHandle(a,&x)&&GetFileInformationByHandle(b,&y)&&
        x.dwVolumeSerialNumber==y.dwVolumeSerialNumber&&x.nFileIndexHigh==y.nFileIndexHigh&&x.nFileIndexLow==y.nFileIndexLow;
    if(a!=INVALID_HANDLE_VALUE)CloseHandle(a);if(b!=INVALID_HANDLE_VALUE)CloseHandle(b);
    if(same){
        ++alias_checks176;
        // A DELETE open against the original name observes the real validator
        // handle's sharing mode on Windows. Apply it to the hard-link rename,
        // matching Proton's inode-wide sharing policy.
        HANDLE probe=CreateFileW(source176.c_str(),DELETE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,OPEN_EXISTING,0,nullptr);
        if(probe==INVALID_HANDLE_VALUE){last_block176=GetLastError();++blocked176;SetLastError(last_block176);return FALSE;}
        CloseHandle(probe);
    }
    return MoveFileExW(from,to,flags);
}
#define MoveFileExW rename176
#include "../../native/work/autoload99-preview/integrated.h"
#undef MoveFileExW
// The fixture calls cache_local159 only; never start the integrated worker,
// extractor or any gameplay component from this executable.
static void workshop_assets165(HMODULE,const std::wstring&){throw std::runtime_error("Cache-only fixture must not start Workshop gameplay");}

static bool attempt176(const std::wstring& target,unsigned kind,bool link){
    try{cache_local159(source176,target,kind,link);return true;}
    catch(const std::exception& error){std::cout<<"Observed: "<<error.what()<<'\n';return false;}
}
static bool same176(const std::wstring& a,const std::wstring& b){
    Handle fa(CreateFileW(a.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr));
    Handle fb(CreateFileW(b.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr));
    BY_HANDLE_FILE_INFORMATION x{},y{};
    return fa.h!=INVALID_HANDLE_VALUE&&fb.h!=INVALID_HANDLE_VALUE&&GetFileInformationByHandle(fa.h,&x)&&GetFileInformationByHandle(fb.h,&y)&&
        x.dwVolumeSerialNumber==y.dwVolumeSerialNumber&&x.nFileIndexHigh==y.nFileIndexHigh&&x.nFileIndexLow==y.nFileIndexLow;
}
int wmain(int argc,wchar_t** argv){try{
    require(argc==4,"Use fixture <owned HUD bundle> <owned QTE bundle> <new output>");
    std::filesystem::path out=argv[3];require(!std::filesystem::exists(out),"Existing fixture output preserved");std::filesystem::create_directories(out);
    unsigned passed=0;
    for(unsigned kind:{1u,2u}){
        source176=argv[kind];assets159::Store validate;require(validate.load(source176,kind),"Owned fixture bundle invalid");validate.clear();
        const auto before=hash_file(source176);
        auto fresh=(out/(kind==1?L"hud.s2fp":L"qte.s2fp")).wstring();
        auto blocked_before=blocked176;
        bool ok=attempt176(fresh,kind,true);
#ifdef S2UP_CACHE_EXPECT_OLD
        require(!ok&&blocked176==blocked_before+1&&last_block176==ERROR_SHARING_VIOLATION&&!std::filesystem::exists(fresh),"Old lifetime did not reproduce its own alias sharing conflict");
#else
        require(ok&&blocked176==blocked_before&&same176(source176,fresh)&&hash_file(fresh)==before,"Fixed hard-link commit failed or stopped sharing bytes");
        require(!std::filesystem::exists(fresh+L".local159.pending166"),"Successful commit left staging");
#endif
        require(hash_file(source176)==before,"Source bundle changed");++passed;
    }
    source176=argv[2];const auto source_hash=hash_file(source176);
    auto pending=(out/L"recovered.s2fp").wstring(),stage=pending+L".local159.pending166";
    require(CreateHardLinkW(stage.c_str(),source176.c_str(),nullptr)!=0,"Cannot prepare interrupted hard-link fixture");
    bool recovered=attempt176(pending,2,true);
#ifdef S2UP_CACHE_EXPECT_OLD
    require(!recovered&&last_block176==ERROR_SHARING_VIOLATION&&std::filesystem::exists(stage),"Old pending alias did not reproduce conflict");
#else
    require(recovered&&!std::filesystem::exists(stage)&&same176(source176,pending),"Existing exact pending alias not recovered");
#endif
    ++passed;
    auto copied=(out/L"copied.s2fp").wstring();require(attempt176(copied,2,false)&&!same176(source176,copied)&&hash_file(copied)==source_hash,"Copy fallback changed bytes");++passed;
    size_t files=std::distance(std::filesystem::directory_iterator(out),std::filesystem::directory_iterator{});
    for(unsigned i=0;i<25;++i)require(attempt176(copied,2,true),"Existing exact target rejected");
    require(files==size_t(std::distance(std::filesystem::directory_iterator(out),std::filesystem::directory_iterator{})),"Repeat commit accumulated files");++passed;
    auto foreign=(out/L"foreign.s2fp").wstring();{std::ofstream file(foreign);file<<"foreign target";}
    require(!attempt176(foreign,2,true)&&std::filesystem::file_size(foreign)==14,"Foreign target changed");++passed;
    auto foreign_pending=(out/L"foreign-pending.s2fp").wstring();{std::ofstream file(foreign_pending+L".local159.pending166");file<<"foreign stage";}
    require(!attempt176(foreign_pending,2,true)&&!std::filesystem::exists(foreign_pending)&&std::filesystem::file_size(foreign_pending+L".local159.pending166")==13,"Foreign pending stage changed");++passed;
    auto locked=(out/L"externally-locked.s2fp").wstring();
    {Handle reader(CreateFileW(source176.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr));
        require(reader.h!=INVALID_HANDLE_VALUE,"Cannot prepare independent reader");
        require(!attempt176(locked,2,true)&&last_block176==ERROR_SHARING_VIOLATION&&!std::filesystem::exists(locked),"Independent reader lock was bypassed");}
    require(hash_file(source176)==source_hash,"Independent reader/source changed");++passed;
#ifdef S2UP_CACHE_EXPECT_OLD
    std::cout<<"PASS "<<passed<<" cache176 controlled OLD-path checks: self-held validator produces error32 for fresh HUD/QTE and pending aliases; not Proton execution\n";
#else
    std::cout<<"PASS "<<passed<<" cache176 corrected-path checks: temporary handle released, exact bytes/links, recovery, copy fallback, foreign files and independent locks preserved; not Proton execution\n";
#endif
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
