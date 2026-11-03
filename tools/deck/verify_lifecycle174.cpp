#define S2UP165_LUA_FIXTURE
#include "entry174.cpp"
static DWORD WINAPI owned_fault174(void*){RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);return 0;}

int wmain(int argc,wchar_t** argv){try{
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    require(argc==2,"Use one new fixture directory beside bin");
    wchar_t executable[32768]{};GetModuleFileNameW(nullptr,executable,32768);
    const auto out=std::filesystem::absolute(argv[1]).lexically_normal();
    require(out.parent_path()==std::filesystem::path(executable).parent_path().parent_path()&&
        !std::filesystem::exists(out),"Fixture output must be new and confined to build output");
    workshop165::plain(out);std::filesystem::create_directory(out);module174=GetModuleHandleW(nullptr);
    auto config=out/L"native.vdf";cache174(202,config);cache174(202,config);
    require(read174(config,65536)==resource174(202),"Native config differs after readback");
    {std::ofstream changed(config,std::ios::binary);changed<<"foreign configuration";}
    bool refused=false;try{cache174(202,config);}catch(...){refused=true;}
    require(refused&&read174(config,65536)=="foreign configuration","Foreign config overwritten");
    auto ini=out/L"controller174.ini";cache174(203,ini);deck174::Config c;std::string why;
    require(deck174::parse_config(read174(ini,16384),c,why),"Default input config invalid");
    {std::ofstream changed(ini,std::ios::binary);changed<<"left_deadzone=0.2\n";}
    require(deck174::parse_config(read174(ini,16384),c,why)&&c.left_deadzone==.2f,"User config is not readable");
    auto prefix=(out/L"owned.dll").wstring();deck174::Proof proof;
    for(unsigned i=0;i<6;++i)report174(prefix,proof,0,0,-2,true,0,"fixture");
    for(unsigned i=0;i<4;++i)require(std::filesystem::file_size(prefix+L".deck174-report"+std::to_wstring(i)+L".json")<=16384,"Report exceeds bound");
    require(!std::filesystem::exists(prefix+L".deck174-report4.json"),"Report count grew beyond limit");
    auto foreign=(out/L"foreign.dll").wstring();
    {std::ofstream file(foreign+L".deck174-report0.json",std::ios::binary);file<<"foreign report";}
    for(unsigned i=0;i<6;++i)report174(foreign,proof,0,0,-2,true,0,"fixture");
    require(read174(foreign+L".deck174-report0.json",16384)=="foreign report","Foreign report replaced");
    require(!std::filesystem::exists(foreign+L".deck174-report4.json"),"Foreign report caused unbounded growth");
    DWORD fault{};require(guarded174(owned_fault174,nullptr,&fault)==ERROR_INVALID_DATA&&fault==EXCEPTION_ACCESS_VIOLATION,
        "Owned diagnostic fault was not contained");
    require(filter174(0xe1740001)==EXCEPTION_CONTINUE_SEARCH,"Unrelated exceptions were consumed");
    std::cout<<"PASS 9 production resource/report/scoped-fault checks; no hardware worker or game launched\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
