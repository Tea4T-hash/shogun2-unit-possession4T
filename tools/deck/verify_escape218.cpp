// Exercise production Escape queue; window/focus queries are explicit providers.
// Never creates a window, steals focus, sends input or runs a game process.
#include <windows.h>
#include <cassert>
#include <iostream>
#include <map>
#include "../../native/deck/battle_speech226.h"
static HWND focused218=reinterpret_cast<HWND>(0x12000);static DWORD owner218=7;
static BOOL fixture_window218(HWND w){return w==reinterpret_cast<HWND>(0x12000);}
static DWORD fixture_thread218(HWND,LPDWORD pid){*pid=owner218;return 10;}
static HWND fixture_focus218(){return focused218;}
static DWORD fixture_pid218(){return 7;}
#define IsWindow fixture_window218
#define GetWindowThreadProcessId fixture_thread218
#define GetForegroundWindow fixture_focus218
#define GetCurrentProcessId fixture_pid218
#include "../../native/deck/escape218.h"
#undef IsWindow
#undef GetWindowThreadProcessId
#undef GetForegroundWindow
#undef GetCurrentProcessId
int main(){unsigned cases=0;auto check=[&](bool good){assert(good);++cases;};deck218::Escape q;unsigned char keys[256]{};auto own=reinterpret_cast<HWND>(0x12000),other=reinterpret_cast<HWND>(0x13000);
 check(!q.request(nullptr,1));check(!q.request(other,1));owner218=8;check(!q.request(own,1));owner218=7;focused218=other;check(!q.request(own,1));focused218=own;
 check(q.request(own,1000));keys[0x11]=0x80;check(q.apply(keys,true,own,1001)&&keys[1]==0x80&&keys[0x11]==0x80);keys[1]=0;check(!q.apply(keys,true,own,1002)&&keys[1]==0);
 check(q.request(own,2000));check(!q.apply(keys,false,own,2010));check(q.apply(keys,true,own,2011));keys[1]=0;
 check(q.request(own,3000));check(!q.apply(keys,true,own,3251));check(!q.apply(keys,true,own,3252));
 check(q.request(own,4000));check(!q.apply(keys,true,other,4001));check(!q.apply(keys,true,own,4002));
 check(q.request(own,5000));q.clear();check(!q.apply(keys,true,own,5001));
 check(q.request(own,6000));check(!q.apply(keys,true,own,5999));check(!q.apply(keys,true,own,6001));
 // A held physical Escape stays physical; this queue only ORs its one pulse.
 keys[1]=0x80;check(q.request(own,7000));check(q.apply(keys,true,own,7001)&&keys[1]==0x80);check(!q.apply(keys,true,own,7002)&&keys[1]==0x80);
 keys[1]=0;
 constexpr uintptr_t image=0x10000000,root=0x14000,environment=0x20000,common=0x51000,phase=0x52000,speech=0x53000;
 std::map<uintptr_t,uintptr_t> memory={
    {image+0x1bc8180,root},{root,image+0x170d284},{root+0x110,environment},
    {environment+8,common},{common+0xb4,phase},{phase+0x14,2},
    {environment+0x2827c,speech},{speech,image+0x15d3c38},{speech+0x1c,environment},{speech+0xa4,1},
 };
 auto active=[&](){return deck226::active_speech(image,[&](uintptr_t at){return memory[at];});};
 check(active()==speech);
 check(q.request(own,8000,active()));
 check(q.apply(keys,true,own,8001,active())&&keys[1]==0x80);
 keys[1]=0;
 check(!q.apply(keys,true,own,8002,active())&&keys[1]==0);
 check(q.request(own,9000,active()));
 memory[phase+0x14]=3;
 check(!active()&&!q.apply(keys,true,own,9001,active())&&keys[1]==0);
 memory[phase+0x14]=2;
 check(!q.apply(keys,true,own,9002,active()));
 check(q.request(own,10000,active()));
 check(!q.apply(keys,true,own,10001,speech+0x200)&&keys[1]==0);
 memory[speech+0xa4]=0;memory[speech+0xa3]=1;
 check(!active());
 memory[speech+0xa4]=1;memory[speech+0x1c]=environment+4;
 check(!active());
 memory[speech+0x1c]=environment;memory[speech]=image+0x15d3c3c;
 check(!active());
 memory[speech]=image+0x15d3c38;memory[environment+0x2827c]=0;
 check(!active());
 memory[environment+0x2827c]=speech;memory[root]=0;
 check(!active());
 std::cout<<"PASS escape218: "<<cases<<" native-key pulse/window/focus/expiry/handoff cases; window APIs are providers; no live input\n";
}
