#define main retained_ability142_main
#include "verify_native_commands100.cpp"
#undef main
#include <limits>
static float __cdecl zero142(){return 0;}
int main(){SetUnhandledExceptionFilter(&fault100);try{
 bow142::Input input;bow142::Key key{7,3,4};bow142::Release release{};
 require(!input.sample(true,true,false,key,1,release)&&!input.sample(true,true,true,key,2,release),"Held entry key started a bow draw");
 input.sample(false,true,true,key,3,release);input.sample(true,true,true,key,4,release);
 for(unsigned i=0;i<1000;++i)require(!input.sample(true,true,true,key,4+i*.001,release),"Held bow emitted repeated shots");
 require(input.sample(false,true,true,key,5,release)&&release.seconds==1&&release.serial==1,"Bow release duration/edge differs");
 input.sample(true,true,true,key,6,release);input.sample(true,true,false,key,7,release);require(!input.sample(false,true,true,key,8,release),"Lost focus fired bow");
 input.sample(true,true,true,key,9,release);auto other=key;++other.lease;require(!input.sample(false,true,true,other,10,release),"Changed lease fired old bow");
 input.sample(true,true,true,key,11,release);require(!input.sample(false,false,true,key,12,release),"Weapon switch released bow");
 for(float full:{.2f,.5f,1.f,2.5f}){
  float previous=0;
  for(unsigned i=0;i<=2000;++i){float speed=bow142::factor(double(i)*full/1000.,full);require(speed>=.35f&&speed<=1.f&&speed>=previous,"Draw strength not bounded/monotonic");previous=speed;}
  require(bow142::factor(full,full)==1&&bow142::factor(full*5,full)==1,"Full draw exceeded native speed");
 }
 NativeFixture100 f;using P=NativeFixture100;state->actor_id=field<unsigned>(possessed,0x1c8);
 std::vector<unsigned char> record_bytes(0x180),db_bytes(0x20),clip_bytes(8),asset_bytes(0x100);std::vector<uintptr_t> clips(0x4a3);
 auto record=P::ptr(record_bytes),db=P::ptr(db_bytes),clip=P::ptr(clip_bytes),asset=P::ptr(asset_bytes);
 // Binding uses the production family classifier. An explicit native record
 // key provider avoids substituting a unit-specific bow implementation.
 const wchar_t name[]=L"shogun_arrow";field<unsigned>(record,4)=unsigned(wcslen(name));field<uintptr_t>(record,0xc)=reinterpret_cast<uintptr_t>(name);field<unsigned>(record,8)=32;field<unsigned>(record,0x60)=150;field<float>(record,0x6c)=50;
 field<uintptr_t>(possessed,0x23c)=db;field<uintptr_t>(db,0xc)=reinterpret_cast<uintptr_t>(clips.data());field<uintptr_t>(clip,4)=asset;field<unsigned>(asset,0x64)=51;field<float>(base,0x15b2448)=.05f;clips[126]=clips[758]=clip;
 auto key142=bow_key142();bow_input142.cancel();
 bow_input142.sample(false,true,true,key142,9,release);
 bow_input142.sample(true,true,true,key142,10,release);
 require(bow_power142(false,10)==.35f&&bow_power142(false,12.5)==1.f,"Bow meter endpoints differ");
 field<unsigned char>(asset,0x94)=1;field<float>(asset,0x90)=1.5f;
 require(std::abs(bow_power142(true,10.75)-.675f)<.001f,"Meter missed mounted native release marker");
 field<unsigned char>(asset,0x94)=0;
 const float shown=bow_power142(false,11.25);
 require(bow_input142.sample(false,true,true,key142,11.25,release),"Meter fixture did not release");
 bow_release142=release;require(bow_bind142(record,false),"Meter fixture release did not bind");
 require(std::abs(shown*50.f-bow_speed142(record,50))<.001f&&bow_power142(false,11.25)==0,
     "Meter and launch power differed or release left meter active");
 bow_input142.sample(true,true,true,key142,13,release);
 ++state->lease_generation;require(bow_power142(false,13.5)==0,"Stale bow meter escaped its lease");--state->lease_generation;
 bow_input142.sample(true,true,false,key142,14,release);
 require(bow_power142(false,14)==0,"Canceled draw retained power");
 bow_input142.sample(false,true,true,key142,14,release);bow_reset142();
 bow_family142=-1;state->fire_request=0;bow_sample142(true,true);bow_sample142(false,true);require(!state->fire_request,"Unknown weapon state queued a bow release");
 bow_family142=0;bow_sample142(true,true);bow_sample142(false,true);require(!state->fire_request,"Non-bow entered bow release policy");
 bow_family142=1;bow_sample142(true,true);bow_sample142(false,true);require(state->fire_request==1,"Known bow release failed to queue");state->fire_request=0;
 auto k=bow_key142();bow_release142={k,1,.01};bow_bind142(record,false);require(bow_ticket142.record==record&&bow_speed142(record,50)<18,"Weak bow failed to reduce per-shot native launch speed");
 bow_release142={k,2,2.5};bow_bind142(record,false);require(bow_speed142(record,50)==50,"Full draw changed native bow speed");
 bow_release142={k,3,1.25};bow_bind142(record,true);require(std::abs(bow_speed142(record,50)-33.75f)<.001f,"Mounted bow did not use native draw asset duration");
 auto record_before=record_bytes,actor_before=f.actor;auto speed=bow_speed142(record,50);require(record_bytes==record_before&&f.actor==actor_before&&speed<=50,"Draw changed shared record or actor bytes");
 ++state->lease_generation;require(bow_speed142(record,50)==50,"Old lease power affected a new shot");--state->lease_generation;
 require(bow_speed142(record+4,200)==200,"Bow power affected another weapon");
 DWORD old142{};VirtualProtect(reinterpret_cast<void*>(base+0x447720),16,PAGE_EXECUTE_READWRITE,&old142);f.jump(0x447720,reinterpret_cast<void*>(&zero142));VirtualProtect(reinterpret_cast<void*>(base+0x447720),16,old142,&old142);
 alignas(16) unsigned char parameters142[0x48]{},launch142[0x80]{};parameters142[0x44]=1;float target142[3]={0,0,6000};
 auto context142=P::ptr(f.common);field<float>(record,0xf4)=-9.81f;
 for(float duration142:{.01f,.5f,1.25f,2.5f,10.f}){
  bow_release142={k,5,duration142};bow_bind142(record,false);float native142=bow_speed142(record,50);
  reinterpret_cast<Launch>(base+0x40e9d0)(launch142,reinterpret_cast<void*>(context142),reinterpret_cast<void*>(record),parameters142,native142);
  require(reinterpret_cast<Ballistic>(base+0x420050)(launch142,target142),"Original native direct ballistic solve failed");
  auto l142=reinterpret_cast<uintptr_t>(launch142);float actual142=std::hypot(std::hypot(field<float>(l142,0x50),field<float>(l142,0x54)),field<float>(l142,0x58));
  require(std::abs(actual142-native142)<.001f&&field<float>(record,0x6c)==50&&field<float>(record,0xf4)==-9.81f,"Original native launch changed speed/definition/gravity");
 }
 bow_reset142();require(!bow_ticket142.record&&bow_family142==-1,"Exit retained power");
 std::cout<<"PASS: bow hold/release, focus/lease/weapon cancellation,8004 monotonic strength cases, native draw metadata, foot/mounted binding, full-speed ceiling and no shared actor/projectile writes. Native projectile release remains animation-driven.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
