#pragma once
#include "../../native/deck/proton_paths174.h"
// Owned-game input only. Offsets are uint64_t: Shogun's Lua numbers are floats.
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>
#include "../../native/common/local_assets159.h"
namespace workshop165 {
inline constexpr char root_sha[]="255b305cefb24a68cf6d2fc79d07efad7c0252acfd98ecd3ad10fa55c959cae8";
inline void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
inline void plain(const std::filesystem::path& path){deck174::paths::plain(path);}
inline std::vector<unsigned char> ui_root168(const std::filesystem::path& pack,bool stock,std::string* metadata=nullptr,const char* target_root="ui/frontend ui/layout_scripts/root.luac",unsigned expected_size=13739,const char* expected_sha=root_sha){
    if(metadata)metadata->clear();
    deck174::paths::read_file(pack);std::ifstream in(pack,std::ios::binary);check(bool(in),"Owned data.pack is missing");
    unsigned char header[32]{};check(bool(in.read((char*)header,32)),"Truncated pack header");
    uint32_t flags=assets159::u32(header+4),count=assets159::u32(header+16),index_size=assets159::u32(header+20);
    uint64_t total=std::filesystem::file_size(pack);
    uint32_t dependencies=assets159::u32(header+8),dependency_size=assets159::u32(header+12);
    const bool pfh3=!memcmp(header,"PFH3",4),pfh2=!memcmp(header,"PFH2",4);
    const bool timestamps=(flags&0x40)!=0;
    const bool format=stock?(pfh3&&flags<=4):((pfh2||pfh3)&&!(flags&~0x4fu)&&(flags&15)<=4);
    check(format&&dependencies<=4096&&dependency_size<=1048576&&
          (!stock||(!dependencies&&!dependency_size))&&count<=100000&&index_size<=16*1024*1024&&
          32ull+dependency_size+index_size<=total,"Unsupported pack index");
    std::vector<unsigned char> dependency_index(dependency_size);
    check(dependency_index.empty()||bool(in.read((char*)dependency_index.data(),dependency_index.size())),"Truncated pack dependencies");
    check(size_t(std::count(dependency_index.begin(),dependency_index.end(),0))==dependencies&&
          (dependency_index.empty()||dependency_index.back()==0),"Pack dependency index differs");
    std::vector<unsigned char> index(index_size);check(bool(in.read((char*)index.data(),index.size())),"Truncated pack index");
    size_t cursor=0;uint64_t offset=32ull+dependency_size+index_size,root_offset=0,metadata_offset=0;unsigned root_size=0,metadata_size=0;std::set<std::string> seen;
    for(unsigned i=0;i<count;++i){
        const size_t fixed=timestamps?12:4;
        check(cursor+fixed<=index.size(),"Truncated pack entry");unsigned size=assets159::u32(index.data()+cursor);cursor+=fixed;
        auto begin=cursor;while(cursor<index.size()&&index[cursor])++cursor;check(cursor<index.size(),"Unterminated pack name");
        std::string name((char*)index.data()+begin,cursor-begin);++cursor;std::replace(name.begin(),name.end(),'\\','/');
        std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(tolower(c));});
        check(!name.empty()&&name.front()!='/'&&name.find(':')==name.npos&&name.find("//")==name.npos&&name.find("../")==name.npos&&
              name.find("/..") == name.npos&&name.find("./")==name.npos&&seen.insert(name).second&&offset<=total&&size<=total-offset,"Unsafe/duplicate/out-of-range pack entry");
        if(name==target_root){root_offset=offset;root_size=size;}
        if(metadata&&name=="possession/workshop165.json"){metadata_offset=offset;metadata_size=size;}
        offset+=size;
    }
    check(cursor==index.size(),"Pack index has trailing bytes");
    if(!root_size&&!stock)return {};
    // The full possession bootstrap carries its own native payload and is
    // larger than a menu script. Selection discards it after self-identification;
    // only a <=256 KiB original/menu addon can enter the Lua handoff.
    check(root_size>0&&root_size<=8*1024*1024&&(!stock||root_size==expected_size),"UI root script is missing or too large");
    if(metadata&&metadata_size){
        check(metadata_size<=16384,"Possession metadata exceeds bound");in.seekg(std::streamoff(metadata_offset));
        metadata->resize(metadata_size);check(bool(in.read(&(*metadata)[0],metadata_size)),"Truncated possession metadata");
    }
    in.seekg(std::streamoff(root_offset));std::vector<unsigned char> bytes(root_size);
    check(bool(in.read((char*)bytes.data(),bytes.size())),"Truncated owned root script");
    check(!stock||assets159::sha(bytes.data(),bytes.size())==expected_sha,"Owned root script fingerprint differs");return bytes;
}
inline std::vector<unsigned char> original_root(const std::filesystem::path& pack){return ui_root168(pack,true);}
// Transient handoff to Lua. The reader uses rbD so both handles share deletion.
// Existing files are always preserved; Windows cleans our file on forced exit.
inline HANDLE transient_root168(const std::filesystem::path& path,const std::vector<unsigned char>& bytes){
    plain(path);check(!bytes.empty()&&bytes.size()<=262144,"UI handoff exceeds bound");
    HANDLE file=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE|DELETE,
        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,CREATE_NEW,
        FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,nullptr);
    check(file!=INVALID_HANDLE_VALUE,"UI handoff is occupied; existing file preserved");
    DWORD written=0;
    if(!WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr)||written!=bytes.size()||!FlushFileBuffers(file)){
        CloseHandle(file);check(false,"Cannot write temporary UI handoff");
    }
    return file;
}
inline void create_exact(const std::filesystem::path& path,const std::vector<unsigned char>& bytes){
    plain(path);
    auto matches=[&](){
        std::ifstream in(path,std::ios::binary);check(bool(in)&&std::filesystem::file_size(path)==bytes.size(),"Existing cache differs; preserved");
        std::vector<unsigned char> actual(bytes.size());check(bool(in.read((char*)actual.data(),actual.size()))&&actual==bytes,"Existing cache differs; preserved");
    };
    if(std::filesystem::exists(path)){matches();return;}
    auto staging=path.wstring()+L".pending166";plain(staging);
    if(std::filesystem::exists(staging)){
        std::ifstream in(staging,std::ios::binary);std::vector<unsigned char> previous(bytes.size()+1);
        in.read((char*)previous.data(),previous.size());previous.resize(size_t(in.gcount()));in.close();
        check(previous==bytes,"Unknown/incomplete cache staging preserved; no further copies created");
        if(!MoveFileExW(staging.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)){matches();check(DeleteFileW(staging.c_str())!=0,"Cannot retire exact owned staging duplicate");}
        return;
    }
    HANDLE file=CreateFileW(staging.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    check(file!=INVALID_HANDLE_VALUE,"Cannot create exclusive cache staging");
    DWORD written=0;bool ok=bytes.size()<=MAXDWORD&&WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr)&&written==bytes.size()&&FlushFileBuffers(file);
    CloseHandle(file);
    if(!ok){DeleteFileW(staging.c_str());check(false,"Cannot finish owned cache staging");}
    if(!MoveFileExW(staging.c_str(),path.c_str(),MOVEFILE_WRITE_THROUGH)){
        try{matches();}catch(...){DeleteFileW(staging.c_str());throw;}
        check(DeleteFileW(staging.c_str())!=0,"Cannot retire exact owned staging duplicate");
    }
}
// A small live-check receipt exists only for this process. Windows removes it
// even after a crash/forced exit. Readers must use FILE_SHARE_DELETE as well as
// FILE_SHARE_READ; the receipt never gates gameplay initialization.
class LiveReceipt166 {
    HANDLE file=INVALID_HANDLE_VALUE;
public:
    LiveReceipt166()=default;
    LiveReceipt166(const LiveReceipt166&)=delete;
    ~LiveReceipt166(){if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);}
    bool write(const std::filesystem::path& path,const std::vector<unsigned char>& bytes){
        if(file!=INVALID_HANDLE_VALUE)return true;
        if(bytes.empty()||bytes.size()>4096)return false;
        plain(path);
        file=CreateFileW(path.c_str(),GENERIC_WRITE|DELETE,FILE_SHARE_READ|FILE_SHARE_DELETE,nullptr,CREATE_NEW,
            FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,nullptr);
        if(file==INVALID_HANDLE_VALUE)return false;
        DWORD written=0;
        if(!WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr)||written!=bytes.size()||!FlushFileBuffers(file)){
            CloseHandle(file);file=INVALID_HANDLE_VALUE;return false;
        }
        return true;
    }
};
}
