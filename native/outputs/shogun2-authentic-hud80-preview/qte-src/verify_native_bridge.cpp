#include "qte_native_bridge.h"
#include <iostream>
#include <stdexcept>
#include <vector>
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct World {
    static constexpr uint32_t start=0x10000,engine=start,witness=0x1b00000;
    static constexpr uint32_t environment=witness+0x1000,common=witness+0x2000,context=witness+0x3000;
    static constexpr uint32_t player=witness+0x4000,enemy=witness+0x5000,unit1=witness+0x6000,unit2=witness+0x8000;
    std::vector<unsigned char> bytes=std::vector<unsigned char>(0x1b00000);
    qte::NativeBridge* bridge{};bool attack=true,selection_fault=false;
    unsigned sorts{},rolls{},prepares{},updates{};uint32_t outcome=5;
    qte::NativeVerdict committed{};
    template<class T>void put(uint32_t address,T value){check(address>=start&&address-start+sizeof value<=bytes.size(),"Fixture bound");memcpy(bytes.data()+address-start,&value,sizeof value);}
    static bool read(void* p,uint32_t address,void* out,size_t size){auto& w=*static_cast<World*>(p);
        if(address<start||size>w.bytes.size()||address-start>w.bytes.size()-size)return false;
        memcpy(out,w.bytes.data()+address-start,size);return true;}
    template<class T>T get(uint32_t address){T value{};check(read(this,address,&value,sizeof value),"Fixture read");return value;}
    qte::Reader reader(){return {read,this};}
    void setup(bool attacks,uint32_t result){
        attack=attacks;outcome=result;selection_fault=false;sorts=rolls=prepares=updates=0;committed={};
        put(witness+4,environment);put(environment+8,common);put(common+0x9c,context);
        put(witness+0xc,uint8_t(0));put(witness+0xd0,uint8_t(0));put(witness+0xee,uint8_t(0));
        put(witness+0x24,2u);put(witness+0x10,player);put(witness+0x14,enemy);
        for(unsigned axis=0;axis<3;++axis){put(engine+0x1a4dc0c+axis*4,0.f);put(player+0x48+axis*4,float(axis));put(enemy+0x48+axis*4,float(axis+2));}
        for(unsigned i=0;i<2;++i){uint32_t actor=i?enemy:player,unit=i?unit2:unit1;
            put(actor,engine+0x15dfec8);put(actor+0xf0,uint8_t(1));put(actor+0x688,witness);
            put(actor+0x214,unit);put(actor+0x1c8,i);put(actor+0x44,i);put(actor+0x21c,0u);
            put(unit+4,i+1);put(unit+0xd3c,uint8_t(0));put(unit+0xd34,2u);put(unit+0xce8,2u);
            put(unit+0xcbc,160u);put(unit+0x2bc,160u);
            uint32_t army=witness+0xa000+i*0x2000,members=army+0x1000;
            put(unit+0x2a4,army);put(army+0x114,1u);put(army+0x118,members);put(members,unit);
            put(army+0x1ec,uint8_t(1));put(unit+0xcf8,unit+0xd00);put(unit+0xcfc,unit+0xd00);
        }
    }
    qte::Owner owner(){return {1,9,environment,{player,0,unit1,1,0,1.f},true};}
    static bool sort(void* p,uint32_t begin,uint32_t end,uint32_t count,const float*){
        auto& w=*static_cast<World*>(p);check(begin==witness+0x10&&end==begin+8&&count==2,"Sort ABI arguments");++w.sorts;return true;}
    static bool select(void* p,uint32_t ctx,uint32_t begin,uint32_t end,uint32_t flags,uint32_t flags2,qte::NativeVerdict& out){
        auto& w=*static_cast<World*>(p);check(ctx==context&&begin==witness+0x10&&end==begin+8&&flags==0&&flags2==0,"Select ABI arguments");
        ++w.rolls;out={w.outcome,w.attack?player:enemy,w.attack?enemy:player};return !w.selection_fault;}
    static uint32_t prepare(void* p,uint32_t address){auto& w=*static_cast<World*>(p);++w.prepares;
        if(address!=witness)return 0xa5a5007f; // preserve full native EAX, not only AL
        // Fixture original prepare calls its selector through the bridge hook.
        qte::NativeVerdict selected;
        check(w.bridge->selector(address,0x3e5f7c,context,address+0x10,address+0x18,0,0,selected),"Original prepare selector failed");
        w.committed=selected;w.put(witness+0xc,uint8_t(1));return 0;}
    static uint32_t update(void* p,uint32_t){++static_cast<World*>(p)->updates;return 0xbeef006f;}
    static bool routing(void*,uint32_t,bool& value){value=false;return true;}
    qte::SelectionCalls selection(){return {this,sort,select};}
    qte::BridgeCalls calls(){return {this,prepare,update,routing};}
};
int main(){try{using namespace qte;const OutcomeBindings fixture{0,4,true}; /* synthetic codes only */ World world;unsigned cases=0;
    for(unsigned outcome=0;outcome<=5;++outcome)for(bool attack:{false,true})for(bool success:{false,true}){
        world.setup(attack,outcome);NativeBridge bridge(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&bridge;
        auto owner=world.owner();
        check(bridge.prepare(World::witness,1,owner)==0&&bridge.snapshot().active,"Native pair not acquired");
        check(world.rolls==1&&world.prepares==0&&bridge.update(World::witness,1)==0&&world.updates==0,"Initial native selection/update hold incorrect");
        check(bridge.prepare(World::witness+0x100,2,owner)==0xa5a5007f&&bridge.update(World::witness+0x100,2)==0xbeef006f,"Other combat return values changed");
        const auto snapshot=bridge.snapshot();
        check(snapshot.action==(attack?Action::Attack:Action::Defend),"Native selected roles not used");
        bridge.frame(owner,5,true,true,snapshot.serial,true,true);
        check(bridge.snapshot().elapsed==0&&bridge.snapshot().result==Result::Pending,"Paused input/time accepted");
        bridge.frame(owner,0,false,true,snapshot.serial,false,false);
        bridge.frame(owner,success?snapshot.ideal:0,false,true,snapshot.serial,attack,!attack);
        check(bridge.snapshot().result==(success?Result::Success:Result::Early),"Timed bridge result incorrect");
        check(bridge.prepare(World::witness,1,owner)==0,"Original native prepare result changed");
        check(world.rolls==1&&world.prepares==2&&world.committed.outcome==(attack?(success?4u:0u):(success?0u:4u)),"Cached native verdict/RNG contract failed");
        check(world.committed.attacker_address==(attack?World::player:World::enemy)&&world.committed.defender_address==(attack?World::enemy:World::player),"Native role pointers changed");
        check(!bridge.snapshot().active&&bridge.replay_errors()==0&&bridge.update(World::witness,1)==0xbeef006f,"Completed prompt retained or native update restricted");++cases;
    }
    for(unsigned cancel=0;cancel<4;++cancel){world.setup(true,5);NativeBridge bridge(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&bridge;
        auto owner=world.owner();bridge.prepare(World::witness,1,owner);
        if(cancel==0)bridge.release();
        if(cancel==1){owner.lease++;bridge.frame(owner,.1,false,true,bridge.snapshot().serial,false,false);}
        if(cancel==2){owner.verified_melee=false;bridge.frame(owner,.1,false,true,bridge.snapshot().serial,false,false);}
        if(cancel==3){world.put(World::unit1+0xce8,99u);bridge.frame(owner,.1,false,true,bridge.snapshot().serial,false,false);}
        check(bridge.prepare(World::witness,1,owner)==0&&world.committed.outcome==5&&world.rolls==1&&!bridge.snapshot().active,"Release/context loss did not restore stock selected combat");
    }
    for(unsigned invalid=0;invalid<7;++invalid){world.setup(true,5);NativeBridge bridge(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&bridge;
        bridge.prepare(World::witness,1,world.owner());
        if(invalid==0)world.put(World::enemy+0xf0,uint8_t(0));
        if(invalid==1)world.put(World::enemy+0x1c8,99u);
        if(invalid==2)world.put(World::enemy+0x688,World::witness+0x100u);
        if(invalid==3)world.put(World::witness+0x24,3u);
        if(invalid==4)bridge.teardown();
        if(invalid==5){auto changed=world.owner();changed.battle++;bridge.frame(changed,.1,false,true,bridge.snapshot().serial,false,false);}
        if(invalid==6)world.put(World::witness+0x14,World::player);
        check(bridge.update(World::witness,1)==1&&!bridge.snapshot().active&&world.prepares==0&&world.rolls==1,"Dead/recycled pair replayed or stuck");
    }
    world.setup(true,5);NativeBridge missing(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&missing;
    world.put(World::unit2+0x2a4,0u);
    check(missing.prepare(World::witness,1,world.owner())==0&&!missing.snapshot().active&&world.rolls==1&&world.committed.outcome==5,"Missing native context caused reroll or fake prompt");
    world.setup(true,5);NativeBridge denied(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&denied;
    auto owner=world.owner();owner.verified_melee=false;
    check(denied.prepare(World::witness,1,owner)==0&&world.rolls==1&&world.sorts==0&&!denied.snapshot().active,"Ranged/unowned actor acquired QTE");
    // Recorded native initial prepare: both participants have backlink0,
    // active0/phase0. Candidate50/51 incorrectly rejected this state.
    {world.setup(true,5);world.put(World::player+0x688,0u);world.put(World::enemy+0x688,0u);
     check(player_encounter(world.reader(),World::player,World::witness,World::engine,false),"Captured initial participant filtered");
     check(!player_encounter(world.reader(),World::player,World::witness,World::engine,true),"Unowned update admitted");
     NativeActor identity;
     check(!actor_identity(world.reader(),World::player,World::witness,World::engine,identity),"Strict registered lookup relaxed");
     check(actor_identity(world.reader(),World::player,World::witness,World::engine,identity,true),"Initial identity rejected");
     NativeBridge initial(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&initial;
     auto owned=world.owner();check(initial.prepare(World::witness,1,owned)==0&&initial.occupied()&&initial.snapshot().active&&world.rolls==1&&world.prepares==0,"Initial native pair not held");
     initial.frame(owned,.1,false,true,initial.snapshot().serial,false,false);
     check(initial.update(World::witness,1)==0&&initial.snapshot().active,"Held unregistered pair invalidated");
     initial.release();check(initial.prepare(World::witness,1,owned)==0&&!initial.occupied()&&world.rolls==1&&world.committed.outcome==5,"Initial pair stock replay failed");}
    for(unsigned invalid=0;invalid<5;++invalid){world.setup(true,5);world.put(World::player+0x688,0u);
        if(invalid==0)world.put(World::witness+0xc,uint8_t(1));
        if(invalid==1)world.put(World::witness+0xd0,uint8_t(1));
        if(invalid==2)world.put(World::witness+0x10,World::enemy);
        if(invalid==3)world.put(World::player+0x688,World::witness+0x100u);
        if(invalid==4)world.put(World::player+0xf0,uint8_t(0));
        check(!player_encounter(world.reader(),World::player,World::witness,World::engine,false),"Unregistered foreign/prepared/dead pair admitted");
        NativeActor identity;check(!actor_identity(world.reader(),World::player,World::witness,World::engine,identity,true),"Initial identity admitted invalid pair");
    }
    world.setup(true,5);NativeBridge fault(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&fault;world.selection_fault=true;
    check(fault.prepare(World::witness,1,world.owner())==1&&world.rolls==1&&world.prepares==0&&fault.replay_errors()==1,"Failed selector attempted a second RNG call");
    world.setup(true,5);NativeBridge timestamped(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&timestamped;
    owner=world.owner();timestamped.prepare(World::witness,1,owner);auto prompt=timestamped.snapshot();
    timestamped.input_sample(prompt.ideal,prompt.serial-1,true,false,false,true);
    check(timestamped.snapshot().elapsed==0,"Stale queued click advanced current prompt");
    // Later facts must not retroactively alter the window seen by this click.
    world.put(World::unit1+0xd34,5u);world.put(World::unit1+0xce8,7u);
    timestamped.input_sample(prompt.ideal,prompt.serial,true,false,false,true);
    check(timestamped.snapshot().result==Result::Success,"Delayed on-time click missed");
    timestamped.frame(owner,.2,false,true,prompt.serial,true,false);
    check(timestamped.snapshot().result==Result::Success,"Later tick rewrote click verdict");
    check(timestamped.prepare(World::witness,1,owner)==0&&world.committed.outcome==4,"Timestamped verdict failed native replay");
    world.setup(true,4);NativeBridge charge_hold(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&charge_hold;
    charge_hold.frame(world.owner(),0,false,true,0,true,false);
    charge_hold.prepare(World::witness,1,world.owner());prompt=charge_hold.snapshot();
    charge_hold.input_sample(prompt.ideal,prompt.serial,true,false,false,true);
    check(charge_hold.snapshot().result==Result::Pending,"Charge-held attack answered a new QTE");
    charge_hold.input_sample(0,prompt.serial,false,false,false,true);
    charge_hold.input_sample(0,prompt.serial,true,false,false,true);
    check(charge_hold.snapshot().result==Result::Success,"New QTE rejected fresh attack after release");
    world.setup(false,5);NativeBridge held(world.reader(),world.selection(),world.calls(),World::engine,fixture);world.bridge=&held;
    held.prepare(World::witness,1,world.owner());prompt=held.snapshot();
    held.input_sample(10,prompt.serial,false,true,true,true);
    check(held.snapshot().elapsed==0&&held.snapshot().result==Result::Pending,"Paused timestamped press accepted");
    held.input_sample(prompt.ideal,prompt.serial,false,true,false,true);
    check(held.snapshot().result==Result::Pending,"Button held through pause answered QTE");
    held.input_sample(0,prompt.serial,false,false,false,true);held.input_sample(0,prompt.serial,false,true,false,true);
    check(held.snapshot().result==Result::Success,"Released/repressed button failed");
    world.setup(true,5);NativeBridge unverified(world.reader(),world.selection(),world.calls(),World::engine);world.bridge=&unverified;
    check(unverified.prepare(World::witness,1,world.owner())==0&&!unverified.snapshot().active&&world.rolls==1&&world.sorts==0&&world.committed.outcome==5,"Unknown hit/block bindings changed stock combat");
    std::cout<<"PASS: "<<cases<<" combined native-bridge timing/role/outcome scenarios plus stock cancellation, foreign forwarding, zero serial, ranged exclusion, missing context, death/reuse and selector-fault guards. All engine callbacks are fixtures; native hook ABI and live behavior remain unverified.\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
