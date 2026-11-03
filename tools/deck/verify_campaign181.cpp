// Production owned-pack reader boundaries only; no engine entry or game hooks.
#include "ui_compat174_generated.h"
#include <cassert>
#include <iostream>
int wmain(int argc,wchar_t** argv){
 assert(argc==3);std::filesystem::path game=argv[1],fixtures=argv[2];unsigned passed=0;
 constexpr char root[]="ui/campaign ui/layout.root.luac";
 constexpr char sha[]="d14bea292474e62bb6e1e7a5f4938cdfa121b5bcca3003d693c9a31d5d703b50";
 const std::string identity(64,'a');
 auto select=[&](const std::wstring& mods){return workshop168::selected_root(game,L"add_working_directory \""+fixtures.wstring()+L"\";\n"+mods,identity,root,60729,sha);};
 auto original=select(L"");assert(original.size()==60729);++passed;
 auto frontend=workshop168::selected_root(game,L"");assert(frontend.size()==13739&&assets159::sha(frontend.data(),frontend.size())==workshop165::root_sha);++passed;
 assert(select(L"mod \"database.pack\";")==original);++passed;
 auto alternate=select(L"mod \"campaign-addon.pack\";");assert(std::string(alternate.begin(),alternate.end())=="owned test addon A");++passed;
 assert(select(L"mod \"self.pack\";")==original);++passed;
 bool conflict=false;try{select(L"mod \"campaign-addon.pack\";mod \"campaign-other.pack\";");}catch(const std::exception&){conflict=true;}assert(conflict);++passed;
 bool oversized=false;try{select(L"mod \"oversized.pack\";");}catch(const std::exception&){oversized=true;}assert(oversized);++passed;
 auto path=fixtures/L"owned-transient.luac";HANDLE file=workshop165::transient_root168(path,original);assert(file!=INVALID_HANDLE_VALUE);
 bool occupied=false;try{auto other=workshop165::transient_root168(path,original);CloseHandle(other);}catch(const std::exception&){occupied=true;}assert(occupied);CloseHandle(file);assert(!std::filesystem::exists(path));++passed;
 std::cout<<"PASS "<<passed<<" campaign181 owned-reader/coexistence/lifetime checks; no game or Steam execution\n";
}
