#define PREPARED_FIXTURE_ENTRY prepared_legacy_main
#include "verify_prepared_bridge.cpp"
#undef PREPARED_FIXTURE_ENTRY
struct CrowdWorld92:PreparedWorld {
 unsigned continuations{},wait_calls=3,reservations{};bool initial=true,fail{};
 static bool reserve(void* p,const qte::WitnessKey& key,const qte::NativeActor& own,const qte::NativeActor& target){
  auto& w=*static_cast<CrowdWorld92*>(p);++w.reservations;
  check(w.initial&&key.address==witness&&own.address==player&&target.address==enemy,
        "Exact pair was not reserved before native preparation");
  return true;
 }
 static uint32_t native_prepare92(void* p,uint32_t address){auto& w=*static_cast<CrowdWorld92*>(p);
  if(w.initial){check(w.reservations==1,"Native prepare ran before pair protection");w.initial=false;auto answer=PreparedWorld::prepare(p,address);
   w.put(address+0xc,uint8_t(0));w.put(address+0xd0,uint8_t(1));w.put(player+0x688,0u);w.put(enemy+0x688,0u);return answer;
  }
  ++w.prepares;++w.continuations;
  check(w.get<uint8_t>(address+0xd0)==1&&w.get<uint8_t>(address+0xc)==0,"Continuation used outside exact native pending state");
  if(w.fail)return 1;
  if(w.continuations<w.wait_calls)return 0;
  w.put(address+0xc,uint8_t(1));w.put(address+0xd0,uint8_t(0));w.put(player+0x688,address);w.put(enemy+0x688,address);return 0;
 }
 qte::BridgeCalls ports92(){auto calls=ports();calls.prepare=native_prepare92;calls.reserve_pair=reserve;return calls;}
 void begin92(bool attack){initialize(attack,0);continuations=reservations=0;wait_calls=3;initial=true;fail=false;}
};
int main(){try{using namespace qte;unsigned cases=0;
 for(bool attack:{false,true})for(unsigned waits:{1u,3u,20u}){
  CrowdWorld92 w;w.begin92(attack);w.wait_calls=waits;NativeBridge bridge(w.reader(),w.selection(),w.ports92(),World::engine,{0,5,true},true,true,true);w.bridge=&bridge;auto owner=w.owner();
  check(bridge.prepare(World::witness,1,owner)==0&&bridge.occupied()&&!bridge.snapshot().active,"Native waiting state was rejected or shown prematurely");
  auto pending=bridge.pending_key();check(pending.address==World::witness&&pending.pair.player==owner.actor.serial,"Waiting state lost its exact owner");
  w.put(World::player+0x4c,4.f);
  for(unsigned i=1;i<=waits;++i){
   bridge.frame(owner,.05,false,true,0,false,false);
   check(bridge.update(World::witness,1)==0&&w.updates==0,"Waiting pair advanced native clip");
   check(bridge.prepare(World::witness,1,owner)==0&&w.rolls==1,"Waiting preparation rerolled selection or failed continuation");
   if(i<waits)check(bridge.occupied()&&!bridge.snapshot().active,"Prompt started before native registration");
  }
  NativeActor refreshed,opponent;check(bridge.participants(refreshed,opponent)&&refreshed.height==4.f,"Native waiting retained stale terrain height");
  auto shown=bridge.snapshot();check(shown.active&&shown.result==Result::Pending&&shown.response_elapsed==0&&shown.elapsed==0,"Native waiting consumed the visible QTE budget");
  check(w.prepares==waits+1&&w.continuations==waits&&w.rolls==1&&w.reservations==1,"Native preparation call contract changed");
  bridge.frame(owner,3.001,false,true,shown.serial,false,false);check(bridge.snapshot().result==Result::Timeout,"Three-second deadline changed");
  check(bridge.prepare(World::witness,1,owner)==0&&!bridge.occupied()&&w.resumes==1&&w.rolls==1,"Timed verdict failed exactly-once native replay");++cases;
 }
 // Foreign callbacks cannot replace this pending pair, and stale removals cannot clear it.
 for(unsigned invalid=0;invalid<9;++invalid){
  CrowdWorld92 w;w.begin92(true);NativeBridge bridge(w.reader(),w.selection(),w.ports92(),World::engine,{0,5,true},true,true,true);w.bridge=&bridge;auto owner=w.owner();
  bridge.prepare(World::witness,1,owner);
  if(invalid==0)w.put(World::enemy+0xf0,uint8_t(0));
  if(invalid==1)w.put(World::enemy+0x688,World::witness+0x100u);
  if(invalid==2)w.put(World::enemy+0x1c8,99u);
  if(invalid==3)w.put(World::witness+4,World::environment+0x100u);
  if(invalid==4)w.put(World::witness+0xd0,uint8_t(0));
  if(invalid==5)w.put(World::witness+0x88,World::player);
  if(invalid==6)bridge.release();
  if(invalid==7){++owner.lease;}
  if(invalid==8)w.fail=true;
  check(bridge.prepare(World::witness,1,owner)==1&&!bridge.occupied()&&!bridge.snapshot().active&&w.rolls==1,"Invalid native pending state survived or rerolled");++cases;
 }
 {
  CrowdWorld92 w;w.begin92(true);NativeBridge bridge(w.reader(),w.selection(),w.ports92(),World::engine,{0,5,true},true,true,true);w.bridge=&bridge;auto owner=w.owner();bridge.prepare(World::witness,1,owner);
  bridge.removed(World::witness,2);check(bridge.occupied(),"Reused-generation removal cleared held pair");
  check(bridge.prepare(World::witness+0x100u,2,owner)==1&&bridge.pending_key().address==World::witness&&w.rolls==1,"Foreign crowd encounter replaced selected pair");
  auto calls=w.prepares;bridge.frame(owner,10,true,true,0,false,false);check(bridge.prepare(World::witness,1,owner)==0&&w.prepares==calls&&!bridge.snapshot().active,"Pause advanced native pending preparation");
  bridge.removed(World::witness,1);check(!bridge.occupied(),"Actual witness removal did not release pending ownership");++cases;
 }
 std::cout<<"PASS: "<<cases<<" deferred-native/crowd cases; exact-pair reservation, original selector once, no hidden QTE time, paused continuation, native rejection/death/backlink/reuse/lease guards and exactly-once timed replay. Synthetic native ports.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
