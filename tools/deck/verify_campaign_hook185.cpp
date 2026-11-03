#define NOMINMAX
#include "../../native/deck/native_input185.h"
#include "../../native/deck/campaign_keys185.h"
#include <cassert>
#include <iostream>
static deck185::NativeInput input185;static deck185::CampaignKeys keys185;static uintptr_t image185;static bool allowed185=true;static uint64_t tick185=1000;static LONG epoch185=1;
static bool __cdecl poll185(){bool valid=input185.original();keys185.apply(reinterpret_cast<unsigned char*>(image185+0x1b30f58),valid,allowed185,tick185,epoch185);return valid;}
int main(){
 auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x1d88000,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));assert(memory);image185=reinterpret_cast<uintptr_t>(memory);auto at=memory+0xbd75f0;auto current=memory+0x1b30f58;auto previous=memory+0x1b31058;
 // Owned mock device implements the VERIFIED copy-before-poll ABI. No byte of
 // this stub is shipped or executed in a real game; game admission is separate.
 unsigned char stub[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe,0,0,0,0,0xbf,0,0,0,0,0xf3,0xa5,
  0xb9,0x40,0,0,0,0x8d,0xbe,0,0xff,0xff,0xff,0x31,0xc0,0xf3,0xab,
  0xc6,0x05,0,0,0,0,0x80,0x5f,0x5e,0xb0,1,0xc3};
 uintptr_t cur=reinterpret_cast<uintptr_t>(current),prev=reinterpret_cast<uintptr_t>(previous),physical=cur+0x11;
 memcpy(stub+8,&cur,4);memcpy(stub+13,&prev,4);memcpy(stub+36,&physical,4);memcpy(at,stub,sizeof stub);FlushInstructionCache(GetCurrentProcess(),at,sizeof stub);
 assert(input185.create(image185,&poll185)&&input185.enable());using Poll=bool(__cdecl*)();auto native=reinterpret_cast<Poll>(at);unsigned checks=0;
 for(unsigned i=0;i<100;++i){tick185+=500;assert(keys185.put(deck183::Finance,tick185,epoch185)&&native());assert(current[0x31]==0x80&&current[0x11]==0x80);tick185+=10;assert(native());assert(current[0x31]==0&&previous[0x31]==0x80&&current[0x11]==0x80);++checks;}
 allowed185=false;assert(keys185.put(deck183::EndTurn,tick185,epoch185)&&native()&&current[0x1c]==0&&!keys185.count);++checks;allowed185=true;
 for(unsigned i=0;i<20;++i){keys185.clear();assert(input185.disable()&&!memcmp(at,stub,19));assert(native()&&current[0x11]==0x80);assert(input185.enable());++checks;}
 assert(input185.disable());at[0]=0xc3;assert(!input185.enable());at[0]=stub[0];assert(input185.enable());++checks;
 keys185.put(deck183::Clan,tick185,epoch185);++epoch185;assert(native()&&current[0x26]==0);++checks;
 keys185.clear();assert(input185.disable()&&!memcmp(at,stub,sizeof stub));assert(MH_RemoveHook(at)==MH_OK);assert(VirtualFree(memory,0,MEM_RELEASE));
 std::cout<<"PASS "<<checks<<" campaign185 actual owned poll-hook press/release/physical-key/focus/epoch/scene-handoff/foreign-prefix checks; no game or global input\n";
}
