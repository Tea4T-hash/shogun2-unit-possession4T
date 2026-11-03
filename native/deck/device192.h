#pragma once
// One immutable device decision per game process. Valve SteamUtils010 is
// obtained from the game's existing Steam client; no Steam DLL is replaced,
// loaded from disk or initialized/shut down here. Proton/controller presence
// alone never selects the Deck profile.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <cstdio>
namespace device192 {
enum class Mode : unsigned {Desktop=1,Deck=2};
enum class Reason : unsigned {NotDeck=1,VerifiedDeck,NoSteam,NoExports,NoClient,NoInterface,InvalidMethod,QueryFault};
struct Decision {Mode mode{Mode::Desktop};Reason reason{Reason::NoSteam};};
// Valve Proton pinned SDK162: ISteamUtils has 39 methods, including the two
// STEAM_PRIVATE_API slots. IsSteamRunningOnSteamDeck is slot34, bool(this).
struct Utils010 {
 virtual void unused0()=0;
 virtual void unused1()=0;
 virtual void unused2()=0;
 virtual void unused3()=0;
 virtual void unused4()=0;
 virtual void unused5()=0;
 virtual void unused6()=0;
 virtual void unused7()=0;
 virtual void unused8()=0;
 virtual void unused9()=0;
 virtual void unused10()=0;
 virtual void unused11()=0;
 virtual void unused12()=0;
 virtual void unused13()=0;
 virtual void unused14()=0;
 virtual void unused15()=0;
 virtual void unused16()=0;
 virtual void unused17()=0;
 virtual void unused18()=0;
 virtual void unused19()=0;
 virtual void unused20()=0;
 virtual void unused21()=0;
 virtual void unused22()=0;
 virtual void unused23()=0;
 virtual void unused24()=0;
 virtual void unused25()=0;
 virtual void unused26()=0;
 virtual void unused27()=0;
 virtual void unused28()=0;
 virtual void unused29()=0;
 virtual void unused30()=0;
 virtual void unused31()=0;
 virtual void unused32()=0;
 virtual void unused33()=0;
 virtual bool IsSteamRunningOnSteamDeck()=0;
};
inline bool executable(const void* p){MEMORY_BASIC_INFORMATION m{};return p&&VirtualQuery(p,&m,sizeof m)&&m.State==MEM_COMMIT&&!(m.Protect&(PAGE_GUARD|PAGE_NOACCESS))&&(m.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY));}
inline Decision query(HMODULE steam){
 if(!steam)return {};
 using Pipe=int(__cdecl*)();using Client=void*(__cdecl*)();using GetUtils=void*(__cdecl*)(void*,int,const char*);
 auto pipe=reinterpret_cast<Pipe>(GetProcAddress(steam,"SteamAPI_GetHSteamPipe"));
 auto client=reinterpret_cast<Client>(GetProcAddress(steam,"SteamClient"));
 auto get=reinterpret_cast<GetUtils>(GetProcAddress(steam,"SteamAPI_ISteamClient_GetISteamUtils"));
 if(!pipe||!client||!get)return {Mode::Desktop,Reason::NoExports};
 __try{
  auto c=client();int p=pipe();if(!c||!p)return {Mode::Desktop,Reason::NoClient};
  auto u=static_cast<Utils010*>(get(c,p,"SteamUtils010"));if(!u)return {Mode::Desktop,Reason::NoInterface};
  void** table{};void* method{};SIZE_T got{};
  if(!ReadProcessMemory(GetCurrentProcess(),u,&table,sizeof table,&got)||got!=sizeof table||!table||
     !ReadProcessMemory(GetCurrentProcess(),table+34,&method,sizeof method,&got)||got!=sizeof method||!executable(method))return {Mode::Desktop,Reason::InvalidMethod};
  return u->IsSteamRunningOnSteamDeck()?Decision{Mode::Deck,Reason::VerifiedDeck}:Decision{Mode::Desktop,Reason::NotDeck};
 }__except(EXCEPTION_EXECUTE_HANDLER){return {Mode::Desktop,Reason::QueryFault};}
}
inline uint64_t stamp(HANDLE process){FILETIME created{},exited{},kernel{},user{};if(!GetProcessTimes(process,&created,&exited,&kernel,&user))return 0;return (uint64_t(created.dwHighDateTime)<<32)|created.dwLowDateTime;}
struct Record {unsigned magic{},version{},bytes{},pid{};uint64_t created{};unsigned mode{},reason{};};
constexpr unsigned magic=0x32395044;
inline void name(wchar_t(&value)[100],DWORD pid){swprintf_s(value,L"Local\\S2UP-Device192-%lu",pid);}
inline bool valid(const Record& r,DWORD pid,uint64_t created){return r.magic==magic&&r.version==192&&r.bytes==sizeof(Record)&&r.pid==pid&&created&&r.created==created&&(r.mode==unsigned(Mode::Desktop)||r.mode==unsigned(Mode::Deck))&&r.reason>=unsigned(Reason::NotDeck)&&r.reason<=unsigned(Reason::QueryFault)&&(r.mode!=unsigned(Mode::Deck)||r.reason==unsigned(Reason::VerifiedDeck));}
class Publisher {
 HANDLE handle{};
public:
 ~Publisher(){if(handle)CloseHandle(handle);}
 bool publish(Decision decision){
  if(handle)return false;
  const DWORD pid=GetCurrentProcessId();auto created=stamp(GetCurrentProcess());if(!created)return false;
  wchar_t path[100];name(path,pid);HANDLE h=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Record),path);
  DWORD error=GetLastError();if(!h)return false;
  // A second coordinator must not overwrite an existing process decision.
  if(error==ERROR_ALREADY_EXISTS){CloseHandle(h);return false;}
  auto data=static_cast<Record*>(MapViewOfFile(h,FILE_MAP_WRITE,0,0,sizeof(Record)));if(!data){CloseHandle(h);return false;}
  *data={0,192,sizeof(Record),pid,created,unsigned(decision.mode),unsigned(decision.reason)};MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&data->magic),LONG(magic));
  UnmapViewOfFile(data);handle=h;return true;
 }
};
class Reader {
 HANDLE handle{};const Record* data{};DWORD pid{};uint64_t created{},retry{};
public:
 ~Reader(){close();}
 void close(){if(data)UnmapViewOfFile(data);if(handle)CloseHandle(handle);data=nullptr;handle=nullptr;pid=0;created=retry=0;}
 bool deck(DWORD process){
  if(pid&&pid!=process)close();
  if(!data){if(GetTickCount64()<retry)return false;retry=GetTickCount64()+1000;
   HANDLE target=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,process);if(!target)return false;auto time=stamp(target);CloseHandle(target);if(!time)return false;
   wchar_t path[100];name(path,process);HANDLE h=OpenFileMappingW(FILE_MAP_READ,FALSE,path);if(!h)return false;
   auto view=static_cast<const Record*>(MapViewOfFile(h,FILE_MAP_READ,0,0,sizeof(Record)));if(!view){CloseHandle(h);return false;}
   Record copy=*view;if(!valid(copy,process,time)){UnmapViewOfFile(view);CloseHandle(h);return false;}handle=h;data=view;pid=process;created=time;
  }
  MemoryBarrier();Record copy=*data;return valid(copy,process,created)&&copy.mode==unsigned(Mode::Deck);
 }
};
inline bool enabled(DWORD pid=GetCurrentProcessId()){static thread_local Reader reader;return reader.deck(pid);}
inline INIT_ONCE once=INIT_ONCE_STATIC_INIT;inline Publisher publisher;inline bool published{};
inline BOOL CALLBACK choose(PINIT_ONCE,void*,void**){published=publisher.publish(query(GetModuleHandleW(L"steam_api.dll")));return TRUE;}
inline bool select(){return InitOnceExecuteOnce(&once,choose,nullptr,nullptr)&&published;}
} // device192
