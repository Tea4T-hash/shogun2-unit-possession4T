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
    unsigned action{};
    bool fire_mode(bool skirmish){
        return deck175::native_fire_mode(base,root,env,skirmish,
            [&](uintptr_t at){auto i=memory.find(at);return i==memory.end()?uintptr_t(0):i->second;},
            [&](uintptr_t at,unsigned char& value){auto i=memory.find(at);if(i==memory.end())return false;value=static_cast<unsigned char>(i->second);return true;},
            [&](uintptr_t unit,unsigned char& eligible){auto i=memory.find(unit+0xf1c);if(i==memory.end())return false;eligible=static_cast<unsigned char>(i->second);return true;},
            [&](){return alive;},
            [&](uintptr_t object,unsigned requested,float value){assert(object==root&&value==0);action=requested;++calls;memory[queue+4+0x5000]+=37;});
    }
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
    unsigned fire_cases=0;
    auto fire_check=[&](bool ok){assert(ok);++fire_cases;};
    constexpr uintptr_t unit=0x26000000,group=0x27000000,entries=0x25000000;
    for(unsigned active:{0u,1u,2u}){
        Fixture f;f.memory[entries]=unit;f.memory[unit+0xf1c]=1;f.memory[unit+0xeb0]=active;
        fire_check(active<2?f.fire_mode(false)&&f.calls==1&&f.action==(active?3u:2u):
            !f.fire_mode(false)&&!f.calls);
    }
    for(unsigned other_eligible:{0u,1u}){
        Fixture f;const auto other=unit+0x1000;
        f.memory[f.owner+0xc4]=2;f.memory[entries]=unit;f.memory[entries+4]=other;
        f.memory[unit+0xf1c]=1;f.memory[unit+0xeb0]=1;
        f.memory[other+0xf1c]=other_eligible;f.memory[other+0xeb0]=0;
        fire_check(f.fire_mode(false)&&f.calls==1&&f.action==(other_eligible?2u:3u));
    }
    {Fixture f;f.memory[entries]=unit;f.memory[unit+0xf1c]=0;
        fire_check(!f.fire_mode(false)&&!f.calls);}
    for(unsigned active:{0u,1u,2u}){
        Fixture f;f.memory[entries]=unit;f.memory[unit+0x179]=1;f.memory[unit+0xeb2]=active;
        fire_check(active<2?f.fire_mode(true)&&f.calls==1&&f.action==(active?0x55u:0x54u):
            !f.fire_mode(true)&&!f.calls);
    }
    {Fixture f;f.memory[entries]=unit;f.memory[unit+0x179]=0;f.memory[unit+0xeb2]=0;
        fire_check(!f.fire_mode(true)&&!f.calls);}
    {Fixture f;f.memory[entries]=unit;f.memory[unit+0x179]=1;
        fire_check(!f.fire_mode(true)&&!f.calls);}
    for(unsigned invalid:{0u,1u,2u}){
        Fixture f;f.memory[f.owner+0xc4]=0;f.memory[f.owner+0xb4]=1;f.memory[f.owner+0xb8]=1;
        f.memory[f.owner+0xbc]=entries;f.memory[entries]=group;
        f.memory[group+0x20]=invalid==2?513:1;f.memory[group+0x24]=entries+4;f.memory[entries+4]=unit;
        f.memory[group+0x60]=1;f.memory[group+0x64]=entries+8;f.memory[entries+8]=unit+0x1000;
        f.memory[unit+0x179]=1;f.memory[unit+0xeb2]=1;
        f.memory[unit+0x1179]=invalid==1?0:1;f.memory[unit+0x1eb2]=0;
        fire_check(invalid?!f.fire_mode(true)&&!f.calls:f.fire_mode(true)&&f.action==0x54);
    }
    {Fixture f;f.memory[f.queue+4+0x5000]=0x5000;fire_check(!f.fire_mode(false)&&!f.calls);}
    std::cout<<"PASS fire mode57: "<<fire_cases<<" guarded native fire/skirmish order cases\n";
    std::cout<<"PASS "<<passed<<" RTS175 native Halt boundary checks; shared production preflight and action4 dispatch, no game code executed\n";
}
