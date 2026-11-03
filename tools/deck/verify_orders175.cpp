#include "../../native/deck/orders175.h"
#include <cassert>
#include <iostream>
#include <unordered_map>
struct Fixture {
    uintptr_t base=0x10000000,root=0x20000000,env=0x21000000,selected=0x22000000,owner=0x23000000,queue=0x24000000;
    std::unordered_map<uintptr_t,uintptr_t> memory;
    unsigned calls{},current_calls{};bool alive=true,retire_before_invoke=false,append=true;
    Fixture(){memory[root+0x110]=env;memory[root+0x5b8]=env;memory[env+0x2826c]=selected;
        memory[selected+0x4a8]=owner;memory[root+0x5b4]=owner;memory[env+0x28048]=queue;
        memory[owner+0xb0]=base+0x15c4da0;memory[owner+0xc0]=512;
        memory[owner+0xc4]=1;memory[owner+0xc8]=0x25000000;}
    bool run(){return deck175::native_halt(base,root,env,
        [&](uintptr_t address){auto i=memory.find(address);return i==memory.end()?uintptr_t(0):i->second;},
        [&](){return alive&&(!retire_before_invoke||++current_calls<2);},
        [&](uintptr_t object,unsigned action,float value){assert(object==root&&action==4&&value==0);++calls;if(append)memory[queue+4+0x5000]+=37;});}
};
int main(){
    unsigned passed=0;
    auto check=[&](bool ok){assert(ok);++passed;};
    {Fixture f;check(f.run()&&f.calls==1);}
    {Fixture f;f.memory[f.root+0x110]=1;check(!f.run()&&!f.calls);}
    {Fixture f;f.memory[f.root+0x5b8]=1;check(!f.run()&&!f.calls);}
    {Fixture f;f.memory[f.root+0x5b4]=1;check(!f.run()&&!f.calls);}
    {Fixture f;f.memory[f.owner+0xb0]=1;check(!f.run()&&!f.calls);}
    {bool ok=true;for(auto offset:{8,0x20,0x2c,0x34,0x3c}){Fixture f;f.memory[f.owner+0xb0+offset]=1;ok&=!f.run()&&!f.calls;}check(ok);}
    {Fixture f;f.memory[f.owner+0xc4]=0;check(!f.run()&&!f.calls);}
    {Fixture f;f.memory[f.owner+0xc4]=513;check(!f.run()&&!f.calls);}
    {Fixture f;f.memory[f.owner+0xc4]=512;f.memory[f.queue+4+0x5000]=0x5000-(128+8*512);check(f.run()&&f.calls==1);}
    {bool ok=true;for(auto used:{uintptr_t(0x5000),uintptr_t(0xffffffff),uintptr_t(0x5000-135)}){Fixture f;f.memory[f.queue+4+0x5000]=used;ok&=!f.run()&&!f.calls;}check(ok);}
    {Fixture f;f.retire_before_invoke=true;check(!f.run()&&!f.calls);}
    {Fixture f;f.append=false;check(!f.run()&&f.calls==1);}
    std::cout<<"PASS "<<passed<<" RTS175 native Halt boundary checks; shared production preflight and action4 dispatch, no game code executed\n";
}
