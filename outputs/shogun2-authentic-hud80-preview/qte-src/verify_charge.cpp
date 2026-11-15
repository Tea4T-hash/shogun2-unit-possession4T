// Pose-only synthetic actors are ordinary melee here. Committed native
// charges are handled by the production dispatcher (verify_charge140), with
// no QTE. This fixture must not demand the deleted charge LMB prompt or
// rewrite a selected counterattacker into the player.
#define PREPARED_FIXTURE_ENTRY legacy_prepared_fixture_main
#include "verify_prepared_bridge.cpp"
#undef PREPARED_FIXTURE_ENTRY
#include "strict_melee_gate.h"
struct ChargeWorld:PreparedWorld {
    static constexpr uint32_t rider=witness+0xf000,links=witness+0xef00,extra=witness+0xe000;
    unsigned retained{},released{};bool horse{},group{},countercharger_selected{};
    static bool sort(void* p,uint32_t begin,uint32_t end,uint32_t count,const float*){
        auto& w=*static_cast<ChargeWorld*>(p);check(begin==witness+0x10&&end==begin+4*count&&(count==2||count==3),"Original group not supplied to native sort");++w.sorts;return true;
    }
    static bool select(void* p,uint32_t context_,uint32_t begin,uint32_t end,uint32_t flags,uint32_t flags2,qte::NativeVerdict& out){
        auto& w=*static_cast<ChargeWorld*>(p);check(context_==context&&begin==witness+0x10&&end==begin+4*(w.group?3:2)&&flags==0&&flags2==0,"Native original target selection range changed");
        ++w.rolls;out={5,w.countercharger_selected?enemy:player,w.countercharger_selected?player:enemy};return true;
    }
    static bool write(void* p,uint32_t address,const void* value,size_t n){auto& w=*static_cast<ChargeWorld*>(p);
        if(address<start||address-start+n>w.bytes.size())return false;memcpy(w.bytes.data()+address-start,value,n);return true;}
    static void release_extra(void* p,uint32_t witness_,uint32_t actor){
        auto& w=*static_cast<ChargeWorld*>(p);check(witness_==witness&&actor==extra,"Selected actor or wrong witness released");++w.released;
    }
    static bool retain(void* p,uint32_t witness_,const qte::NativeActor& player_,const qte::NativeActor& enemy_){
        auto& w=*static_cast<ChargeWorld*>(p);++w.retained;
        return qte::strictmelee::retain_selected(w.reader(),write,p,engine,witness_,player_,enemy_,release_extra);
    }
    static uint32_t resume(void* p,uint32_t address,const qte::NativeVerdict& result,const qte::NativePlan* plans){
        auto& w=*static_cast<ChargeWorld*>(p);check(plans[0].kind==(w.horse?qte::Horse:qte::Human),"Native combat class not retained in plan");
        return PreparedWorld::resume(p,address,result,plans);
    }
    void init(bool mounted,bool multi){
        initialize(true,5);horse=mounted;group=multi;retained=released=0;put(player+0x119,uint8_t(2));put(player+0x210,0u);
        put(player+0x28,environment);put(enemy+0x28,environment);
        if(mounted){
            put(player,engine+0x15e8524);put(player+0x6cc,1u);put(player+0x6d0,links);put(links,rider);
            put(rider,engine+0x15dfec8);put(rider+0xf0,uint8_t(1));put(rider+0x1c8,923u);put(rider+0x214,unit1);
            put(rider+0x44,0u);put(rider+0x4c,1.8f);put(rider+0x28,environment);put(rider+0x210,player);put(rider+0x21c,0u);
        }
        if(multi){
            put(witness+0x24,3u);put(witness+0x14,extra);put(witness+0x18,enemy);
            put(extra,engine+0x15dfec8);put(extra+0xf0,uint8_t(1));put(extra+0x688,0u);put(extra+0x214,unit2);
            put(extra+0x1c8,3u);put(extra+0x44,1u);put(extra+0x21c,0u);
            for(unsigned i=0;i<3;++i)put(extra+0x48+i*4,float(i)+.01f); // deliberately closer than the native-selected enemy
        }
    }
    qte::Owner own(){auto o=owner();o.actor.kind=horse?qte::Horse:qte::Human;return o;}
    qte::SelectionCalls selectors(){return {this,sort,select};}
    qte::BridgeCalls ports(){return {this,PreparedWorld::prepare,World::update,World::routing,nullptr,resume,nullptr,retain};}
};
int main(){try{using namespace qte;ChargeWorld w;unsigned cases=0;
    for(bool mounted:{false,true})for(bool group:{false,true})for(bool counter:{false,true})for(bool success:{false,true}){
        w.init(mounted,group);w.countercharger_selected=counter;NativeBridge bridge(w.reader(),w.selectors(),w.ports(),World::engine,{0,5,true},true,true,true);w.bridge=&bridge;auto owner=w.own();
        check(bridge.prepare(World::witness,1,owner)==0&&bridge.occupied(),"Native charge not admitted");
        check(bridge.snapshot().action==(counter?Action::Defend:Action::Attack),"Pose-only charge flags changed native attack/defend roles");
        check(w.rolls==1&&w.sorts==1&&w.retained==unsigned(group)&&w.committed.defender_address==(counter?World::player:World::enemy),"Native charge target replaced or rerolled");
        check(w.get<uint32_t>(World::witness+0x24)==2,"Selected pair not retained");
        check(w.get<uint32_t>(ChargeWorld::extra+0x688)==0&&w.released==unsigned(group),"Excluded native reservation not released exactly once");
        check(bridge.update(World::witness,1)==0&&w.updates==0,"Native charge advanced before QTE answer");
        auto prompt=bridge.snapshot();bridge.frame(owner,success?prompt.ideal:0,false,true,prompt.serial,!counter,counter);
        check(bridge.prepare(World::witness,1,owner)==0&&w.resumes==1&&w.rolls==1,"Charge did not commit exactly once");
        check(w.committed.outcome==(counter?(success?0u:5u):(success?5u:0u))&&w.committed.defender_address==(counter?World::player:World::enemy),"Charge success/failure or target changed");++cases;
    }
    w.init(true,false);NativeActor human,subject;
    check(combat_identity(w.reader(),ChargeWorld::rider,World::engine,human)&&physical_actor(w.reader(),World::engine,human,subject)&&subject.address==World::player&&subject.kind==Horse,"Camera rider did not map to horse principal");
    w.put(ChargeWorld::rider+0x210,World::enemy);check(!physical_actor(w.reader(),World::engine,human,subject),"Wrong reciprocal mount admitted");
    w.init(true,false);w.put(World::player+0x6cc,2u);check(!physical_actor(w.reader(),World::engine,human,subject),"Ambiguous passengers admitted");
    w.init(true,false);w.put(ChargeWorld::rider+0x28,World::environment+4);check(!physical_actor(w.reader(),World::engine,human,subject),"Foreign rider environment admitted");
    w.init(false,false);w.put(World::player+0x119,uint8_t(0));check(!charge_planning(w.reader(),World::player),"Speed/state alone classified a charge");
    w.put(World::player+0x119,uint8_t(2));w.put(World::player+0x1d0,13u);check(!charge_planning(w.reader(),World::player),"Running classified as native charge");
    std::cout<<"PASS: "<<cases<<" infantry/horse and multi-participant pose-only attack/defend success/failure cases; original native target retained even with a nearer enemy; selector exactly once; native roles retained without a charge prompt; pending ordinary-melee hold; physical rider mapping and ambiguous/foreign/invalid state rejection. Synthetic engine only.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
