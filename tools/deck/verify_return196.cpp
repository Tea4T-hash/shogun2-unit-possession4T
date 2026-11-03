#define NOMINMAX
#include "../../native/deck/native_input185.h"
#include "../../native/deck/campaign_keys185.h"
#include <cassert>
#include <iostream>
static deck185::NativeInput frontend;
static deck185::CampaignKeys queue;
static uintptr_t image;static uint64_t tick=1000;static LONG epoch=1;static unsigned polls;
static bool allowed=true;
static bool __cdecl campaign(){
 ++polls;bool valid=frontend.original();
 queue.apply(reinterpret_cast<unsigned char*>(image+0x1b30f58),valid,allowed,tick,epoch);return valid;
}
int wmain(int argc,wchar_t**argv){
 assert(argc==2);
 auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x1d88000,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));assert(memory);
 image=reinterpret_cast<uintptr_t>(memory);auto at=memory+0xbd75f0;auto current=memory+0x1b30f58;auto previous=memory+0x1b31058;
 // Owned mock original input, same copy-then-poll ABI as the existing fixture.
 unsigned char stub[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe,0,0,0,0,0xbf,0,0,0,0,0xf3,0xa5,
  0xb9,0x40,0,0,0,0x8d,0xbe,0,0xff,0xff,0xff,0x31,0xc0,0xf3,0xab,
  0xc6,0x05,0,0,0,0,0x80,0x5f,0x5e,0xb0,1,0xc3};
 uintptr_t cur=reinterpret_cast<uintptr_t>(current),prev=reinterpret_cast<uintptr_t>(previous),physical=cur+0x11;
 memcpy(stub+8,&cur,4);memcpy(stub+13,&prev,4);memcpy(stub+36,&physical,4);memcpy(at,stub,sizeof stub);FlushInstructionCache(GetCurrentProcess(),at,sizeof stub);
 assert(frontend.create(image,&campaign)&&frontend.enable());
 using Poll=bool(__cdecl*)();auto native=reinterpret_cast<Poll>(at);
 unsigned commands=0;
 auto check_campaign=[&](){
  for(auto item:{std::pair<unsigned,unsigned>{deck183::Back,1},{deck183::EndTurn,0x1c},{deck183::Finance,0x31},{deck183::Diplomacy,0x25}}){
   tick+=600;assert(queue.put(item.first,tick,epoch));unsigned n=polls;assert(native()&&polls==n+1);
   assert(current[item.second]==0x80&&current[0x11]==0x80);tick+=10;assert(native()&&current[item.second]==0&&previous[item.second]==0x80);++commands;
  }
 };
 check_campaign();assert(frontend.disable()&&!memcmp(at,stub,sizeof stub));
 auto dll=LoadLibraryExW(argv[1],nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);assert(dll);
 auto create=reinterpret_cast<bool(__cdecl*)(void*)>(GetProcAddress(dll,"owner_create"));
 auto acquire=reinterpret_cast<Poll>(GetProcAddress(dll,"owner_acquire"));
 auto release=reinterpret_cast<Poll>(GetProcAddress(dll,"owner_release"));
 auto destroy=reinterpret_cast<Poll>(GetProcAddress(dll,"owner_destroy"));
 auto count=reinterpret_cast<unsigned(__cdecl*)()>(GetProcAddress(dll,"owner_calls"));
 assert(create&&acquire&&release&&destroy&&count&&create(at));
 // Reproduce the old bug: after battle the retained adapter hook prevents the
 // disabled campaign hook from enabling; B/Enter have no consumer.
 assert(!frontend.enable());auto n=polls;queue.put(deck183::Back,tick,epoch);assert(native()&&polls==n&&current[1]==0);queue.clear();
 unsigned rounds=0;
 for(unsigned i=0;i<30;++i){
  auto owned=count();n=polls;assert(native()&&count()==owned+1&&polls==n);
  assert(release()&&!memcmp(at,stub,sizeof stub));assert(release());
  assert(frontend.enable());auto start=count();check_campaign();assert(count()==start);
  assert(!acquire()); // Never overwrite the frontend owner's live hook.
  assert(frontend.disable()&&!memcmp(at,stub,sizeof stub));assert(acquire());++rounds;
 }
 // An unknown patch must survive both acquisition and release attempts.
 auto saved=at[0];at[0]=0xc3;assert(!release()&&at[0]==0xc3);at[0]=saved;
 assert(release());at[0]=0xc3;assert(!acquire()&&at[0]==0xc3);at[0]=stub[0];
 assert(acquire()&&release()&&frontend.enable());
 // Focus/root loss still discards pending campaign commands after handoff.
 allowed=false;queue.put(deck183::EndTurn,tick,epoch);assert(native()&&current[0x1c]==0);allowed=true;
 queue.put(deck183::Back,tick,epoch);++epoch;assert(native()&&current[1]==0);
 assert(frontend.disable()&&destroy());assert(MH_RemoveHook(at)==MH_OK);assert(MH_Uninitialize()==MH_OK);
 assert(!memcmp(at,stub,sizeof stub));FreeLibrary(dll);assert(VirtualFree(memory,0,MEM_RELEASE));
 std::cout<<"PASS return196: "<<rounds<<" two-DLL campaign/battle roundtrips, "<<commands<<" Back/Enter/panel pulses; old retained-hook failure reproduced, foreign hooks preserved\n";
}
