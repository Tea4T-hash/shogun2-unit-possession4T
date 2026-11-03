#define S2UP_DECK_PATH_FIXTURE
#include "../../native/outputs/shogun2-possession-cleanup-v46/src/identity.h"
#include "pack174_generated.h"
#include "ui_compat174_generated.h"
#include <winioctl.h>

// Create an NTFS directory junction only within this fresh owned build fixture.
// No source/game file is moved, changed or deleted; the link is retained.
static void junction(const std::filesystem::path& link,const std::filesystem::path& target){
    require(!std::filesystem::exists(link)&&std::filesystem::is_directory(target),"Junction fixture inputs differ");
    std::filesystem::create_directory(link);
    std::wstring substitute=L"\\??\\"+target.wstring(),print=target.wstring();
    size_t path_bytes=(substitute.size()+print.size()+2)*sizeof(wchar_t);
    require(path_bytes+16<16384,"Junction fixture exceeds bound");
    std::vector<unsigned char> bytes(16+path_bytes);
    DWORD tag=IO_REPARSE_TAG_MOUNT_POINT;WORD length=WORD(8+path_bytes);
    WORD substitute_length=WORD(substitute.size()*2),print_offset=WORD(substitute_length+2),print_length=WORD(print.size()*2);
    memcpy(bytes.data(),&tag,4);memcpy(bytes.data()+4,&length,2);
    memcpy(bytes.data()+10,&substitute_length,2);memcpy(bytes.data()+12,&print_offset,2);memcpy(bytes.data()+14,&print_length,2);
    memcpy(bytes.data()+16,substitute.c_str(),substitute_length+2);
    memcpy(bytes.data()+16+print_offset,print.c_str(),print_length+2);
    Handle file(CreateFileW(link.c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,nullptr));
    DWORD n{};require(file.h!=INVALID_HANDLE_VALUE&&DeviceIoControl(file.h,FSCTL_SET_REPARSE_POINT,
        bytes.data(),DWORD(bytes.size()),nullptr,0,&n,nullptr),"Cannot create owned junction fixture");
}
template<class F> static bool refused(F body){try{body();return false;}catch(const std::exception&){return true;}}

int wmain(int argc,wchar_t** argv){try{
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::cout.setf(std::ios::unitbuf);
    require(argc==3,"Use new build-local fixture and owned stock game");
    wchar_t executable[32768]{};GetModuleFileNameW(nullptr,executable,32768);
    auto out=std::filesystem::absolute(argv[1]).lexically_normal();
    auto game=std::filesystem::absolute(argv[2]).lexically_normal();
    require(out.parent_path()==std::filesystem::path(executable).parent_path().parent_path()&&
        !std::filesystem::exists(out),"Path fixture must be new and confined to build output");
    workshop165::plain(out);std::filesystem::create_directory(out);
    auto target=out/L"real";std::filesystem::create_directory(target);
    {std::ofstream f(target/L"owned.txt");f<<"owned fixture";}
    auto alias=out/L"linked-parent";junction(alias,target);
    using namespace deck174::paths;
    require(GetFileAttributesW(alias.c_str())&FILE_ATTRIBUTE_REPARSE_POINT,"Junction is not a reparse fixture");
    require(refused([&]{plain(alias/L"new.txt");}),"Windows ancestor reparse protection weakened");
    fixture_proton=true;
    plain(alias/L"new.txt");read_file(alias/L"owned.txt");read_directory(alias);
    require(followed_links.load()>=3,"Followed handle checks were not exercised");
    require(refused([&]{plain(alias);}),"Linked output leaf accepted");
    require(refused([&]{read_file(alias);}),"Directory accepted as a linked input file");
    for(auto bad:{L"\\\\server\\share\\file",L"\\\\.\\pipe\\test",L"C:\\file:stream"})
        require(refused([&]{plain(bad);}),"Non-local/alternate stream path accepted");
    require(permitted(true,true,false,Use::ReadFile)&&!permitted(true,true,false,Use::WriteDestination)&&
        !permitted(false,false,true,Use::ReadFile),"Link role policy differs");
    auto game_alias=out/L"owned-game-alias";junction(game_alias,game);
    auto original=workshop165::original_root(game/L"data/data.pack");
    auto linked=workshop165::original_root(game_alias/L"data/data.pack");
    require(original==linked&&original.size()==13739,"Real pack reader differs through linked game directory");
    auto root=out/L"temporary.root.luac";
    HANDLE handoff=workshop165::transient_root168(root,linked);
    require(handoff!=INVALID_HANDLE_VALUE,"Temporary root fixture failed");CloseHandle(handoff);
    require(!std::filesystem::exists(root),"Transient handoff leaked");
    // Actual file-symlink coverage is optional on Windows without developer
    // privileges. The role predicate is always checked; report a real skip.
    auto file_alias=out/L"read-only-file-link.txt";
    bool file_link=CreateSymbolicLinkW(file_alias.c_str(),(target/L"owned.txt").c_str(),2)!=0;
    if(file_link){read_file(file_alias);require(refused([&]{plain(file_alias);}),"File-link output was accepted");}
    else std::cout<<"SKIP actual Windows file-symlink creation (Win32="<<GetLastError()<<"); role policy tested\n";
    auto dangling_target=out/L"vanishing-target";std::filesystem::create_directory(dangling_target);
    auto dangling=out/L"dangling-alias";junction(dangling,dangling_target);
    require(RemoveDirectoryW(dangling_target.c_str())!=0,"Cannot retire exact empty fixture target");
    require(refused([&]{read_directory(dangling);}),"Broken link accepted");
    std::cout<<"PASS diagnostic Proton path policy: real junctions, exact owned-root reader, output refusal, Windows strictness and broken links; actual Proton pending\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
