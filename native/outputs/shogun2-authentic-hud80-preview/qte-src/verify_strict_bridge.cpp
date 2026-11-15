#define PREPARED_FIXTURE_ENTRY legacy_prepared_fixture_main
#include "verify_prepared_bridge.cpp"
#undef PREPARED_FIXTURE_ENTRY
#include "strict_melee_gate.h"
static bool write_world(void* c,uint32_t address,const void* value,size_t n){auto& w=*static_cast<World*>(c);if(address<World::start||address-World::start+n>w.bytes.size())return false;memcpy(w.bytes.data()+address-World::start,value,n);return true;}
static unsigned released213;
static void release_world213(void*,uint32_t witness,uint32_t actor){check(witness==World::witness&&actor==World::witness+0xe000,"Wrong excluded actor release");++released213;}
int main(){try{using namespace qte;PreparedWorld w;unsigned cases=0;
 for(uint32_t outcome=0;outcome<=5;++outcome)for(bool attack:{false,true})for(bool success:{false,true}){
  w.initialize(attack,outcome);w.put(World::unit1+0xf7c,1u);w.put(World::unit2+0xf7c,1u);
  NativeBridge b(w.reader(),w.selection(),w.ports(),World::engine,{0,5,true},true,true,true);w.bridge=&b;
  auto own=w.owner();check(b.prepare(World::witness,1,own)==0&&b.occupied(),"Strict special/ranged encounter not gated");
  check(w.committed.outcome==0&&w.rolls==1,"Stock fatal/special result escaped initial registration");
  check(b.update(World::witness,1)==0&&w.updates==0,"Unanswered native animation advanced");
  auto prompt=b.snapshot();b.frame(own,success?prompt.ideal:0,false,true,prompt.serial,attack,!attack);
  check(b.prepare(World::witness,1,own)==0&&w.resumes==1&&w.rolls==1,"Answered strict encounter did not replay once");
  check(w.committed.outcome==(attack?(success?5u:0u):(success?0u:5u)),"QTE did not exclusively choose outcome");++cases;
 }
 for(unsigned mode=0;mode<5;++mode){
  w.initialize(true,5);NativeBridge b(w.reader(),w.selection(),w.ports(),World::engine,{0,5,true},true,true,true);w.bridge=&b;auto own=w.owner();
  if(mode==0){check(b.prepare(World::witness,1,own,false)==1&&w.prepares==0&&w.rolls==0,"Cooldown forwarded stock melee");continue;}
  b.prepare(World::witness,1,own);
  if(mode==1)b.release();if(mode==2){own.lease++;b.frame(own,.1,false,true,b.snapshot().serial,false,false);}
  if(mode==3){
   const auto locked=b.snapshot();w.put(World::unit1+0xce8,99u);
   b.frame(own,locked.ideal,false,true,locked.serial,true,false);
   check(b.snapshot().result==Result::Success,"Unit facts invalidated a locked pair");
   check(b.prepare(World::witness,1,own)==0&&w.resumes==1&&w.rolls==1&&w.committed.outcome==5,
         "Locked pair did not commit the original answer");
   continue;
  }
  if(mode==4)b.teardown();
  check(b.prepare(World::witness,1,own)==1&&w.prepares==1&&w.resumes==0,"Cancellation/context loss replayed a stock animation");
 }
 w.initialize(true,5);w.put(World::unit2+0x2a4,0u);NativeBridge missing(w.reader(),w.selection(),w.ports(),World::engine,{0,5,true},true,true,true);w.bridge=&missing;
 check(missing.prepare(World::witness,1,w.owner())==1&&w.prepares==0,"Unknown facts silently resumed stock melee");
 // Native witness scope includes extras and prepared lists, not only backlinks.
 w.initialize(true,5);w.put(World::player+0x688,0u);w.put(World::witness+0x24,0u);w.put(World::witness+0x48,1u);w.put(World::witness+0x3c,World::player);
 check(strictmelee::includes(w.reader(),World::player,World::witness),"Extra native participant bypassed scope");
 // Mixed native participant list narrowed to a validated free opposing pair.
 w.initialize(true,5);w.put(World::player+0x688,0u);w.put(World::enemy+0x688,0u);w.put(World::witness+0x24,3u);
 const auto extra213=World::witness+0xe000;
 memcpy(w.bytes.data()+extra213-World::start,w.bytes.data()+World::enemy-World::start,0x700);
 w.put(extra213+0x1c8,99u);w.put(World::witness+0x18,extra213);
 check(strictmelee::narrow(w.reader(),write_world,&w,World::engine,World::witness,w.owner().actor,release_world213)&&w.get<uint32_t>(World::witness+0x24)==2&&released213==1,"Multi-participant native melee reservation not released");
 UnitState a{Required|Strength|Environment|EffectiveCombat,2,2,10,.5f,.8f,false,0,5},e=a;Difficulty d,hard,easy;
 check(difficulty(a,e,d),"Native-score context rejected");a.combat_score=35;e.combat_score=-5;
 check(difficulty(a,e,easy)&&easy.duration>d.duration&&easy.tolerance>d.tolerance,"Effective native combat advantage ignored");
 a.combat_score=-5;e.combat_score=35;check(difficulty(a,e,hard)&&hard.duration<d.duration,"Enemy native bonuses did not increase difficulty");
 a=e;a.morale=5;e.morale=5;check(difficulty(a,e,hard)&&hard.duration<.5f&&hard.tolerance<=.014f,"Two wavering units canceled the severe own penalty");
 a.fatigue=5;check(difficulty(a,e,easy)&&easy.duration<hard.duration,"Own exhaustion did not compound wavering");
 a.combat_score=INFINITY;check(!difficulty(a,e,d),"Invalid native score fabricated difficulty");
 std::cout<<"PASS: "<<cases<<" strict special/normal/ranged attack and defence outcomes; pending clip hold; cooldown/cancellation/context-loss stock bypass rejection; mixed participant scope/pair narrowing; native effective scores and severe absolute wavering/exhaustion. Synthetic engine only.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
