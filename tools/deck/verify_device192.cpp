// Production query/IPC and layout policies. Fake Steam is explicitly provided
// by this fixture; no game's Steam client or running process is examined.
#include "../../native/deck/device192.h"
#include "../../native/deck/channel181.h"
#include "../../native/outputs/shogun2-possession-cleanup-v46/src/hud_layout108.h"
#include <cassert>
#include <iostream>
#include <string>
int wmain(int argc,wchar_t** argv){
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);_set_error_mode(_OUT_TO_STDERR);
 if(argc==4&&!wcscmp(argv[1],L"--reader")){device192::Reader reader;assert(reader.deck(DWORD(wcstoul(argv[2],nullptr,10)))==(argv[3][0]==L'1'));return 0;}
 assert(argc==3);unsigned cases=0;
 HMODULE fake=LoadLibraryW(argv[1]);assert(fake);
 auto set=reinterpret_cast<void(__cdecl*)(int)>(GetProcAddress(fake,"FixtureMode192"));
 auto calls=reinterpret_cast<unsigned(__cdecl*)()>(GetProcAddress(fake,"FixtureCalls192"));assert(set&&calls);
 using device192::Mode;using device192::Reason;
 assert(device192::query(nullptr).mode==Mode::Desktop);++cases;
 assert(device192::query(GetModuleHandleW(L"kernel32.dll")).reason==Reason::NoExports);++cases;
 for(int mode=0;mode<=5;++mode){set(mode);auto decision=device192::query(fake);assert(decision.mode==(mode==1?Mode::Deck:Mode::Desktop));assert(calls()==unsigned(mode==0||mode==1||mode==5));++cases;}
 const bool deck=argv[2][0]==L'1';set(deck?1:0);assert(device192::select());assert(calls()==1);++cases;
 set(deck?0:1);assert(device192::select());assert(calls()==0);assert(device192::enabled()==deck);++cases;
 device192::Publisher duplicate;assert(!duplicate.publish({Mode::Deck,Reason::VerifiedDeck}));++cases;
 device192::Reader reader;assert(reader.deck(GetCurrentProcessId())==deck&&!reader.deck(0));++cases;
 wchar_t exe[32768]{};assert(GetModuleFileNameW(nullptr,exe,32768));
 std::wstring cmd=std::wstring(L"\"")+exe+L"\" --reader "+std::to_wstring(GetCurrentProcessId())+(deck?L" 1":L" 0");
 STARTUPINFOW startup{sizeof startup};PROCESS_INFORMATION child{};assert(CreateProcessW(exe,cmd.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&child));assert(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0);DWORD code=1;assert(GetExitCodeProcess(child.hProcess,&code)&&code==0);CloseHandle(child.hThread);CloseHandle(child.hProcess);++cases;
 device192::Record record{device192::magic,192,sizeof(device192::Record),GetCurrentProcessId(),device192::stamp(GetCurrentProcess()),unsigned(Mode::Deck),unsigned(Reason::VerifiedDeck)};
 assert(device192::valid(record,record.pid,record.created));assert(!device192::valid(record,record.pid,record.created+1));record.reason=unsigned(Reason::NoSteam);assert(!device192::valid(record,record.pid,record.created));record.reason=unsigned(Reason::VerifiedDeck);record.mode=7;assert(!device192::valid(record,record.pid,record.created));++cases;
 deck181::Channel channel;assert(channel.open(GetCurrentProcessId(),true));deck181::Sample sample;sample.context=unsigned(deck174::Context::Possession);sample.focused=sample.lease=1;sample.tick=GetTickCount64();sample.keys['W'/32]|=1u<<('W'%32);assert(channel.publish(sample));deck181::Sample read;assert(deck181::read_sample(GetCurrentProcessId(),read)==deck);++cases;
 for(auto wh:{std::pair<float,float>{1280,800},{1920,1080},{2560,1440},{3840,2160}}){
  float base=std::min(wh.first/1920.f,wh.second/1080.f);if(wh.first>=1280&&wh.second>=720)base=std::max(.85f,base);base*=.85f;
  hudlayout108::deck_ui192=device192::enabled();assert(std::fabs(hudlayout108::scale(wh.first,wh.second)-base*(deck?1.15f:1.f))<.00001f);assert(std::fabs(hudlayout108::qte_anchor_y(wh.second)-wh.second*(deck?.25f:.19f))<.001f);++cases;
 }
 std::cout<<"PASS device192: "<<cases<<" production query/profile/layout checks; "<<(deck?"Deck":"desktop")<<"; child HUD record read; fake Steam only\n";return 0;
}
