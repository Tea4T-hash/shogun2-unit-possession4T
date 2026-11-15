// Original native state validation/change, moving pose dispatch and turn selector.
// Transition lookup and projectile selection are explicit external providers.
#define main retained_commands137
#include "verify_native_commands100.cpp"
#undef main
static uintptr_t projectile137;
static unsigned no_transition137[2]={0x4a3,0};
static void* __fastcall projectile_provider137(void*,void*){return reinterpret_cast<void*>(projectile137);}
static bool __fastcall ability_provider137(void*,void*,unsigned){return false;}
static void* __fastcall transition_provider137(void*,void*,unsigned,unsigned,void*){return no_transition137;}
#include "../../../../tools/deck/verify_locomotion203_provider.h"
int main(){SetUnhandledExceptionFilter(&fault100);try{
 NativeFixture100 f;using P=NativeFixture100;auto a=possessed;
 std::vector<unsigned char> projectile(0x100),db(0x20),owner(0x40),metadata(0x7000),asset(0x100),clip(0x18);
 std::vector<uintptr_t> clips(0x4a3);projectile137=P::ptr(projectile);
 field<uintptr_t>(a,0x23c)=P::ptr(db);field<uintptr_t>(P::ptr(db),0)=P::ptr(owner);field<uintptr_t>(P::ptr(owner),0x28)=P::ptr(metadata);
 field<uintptr_t>(P::ptr(db),0xc)=reinterpret_cast<uintptr_t>(clips.data());
 field<unsigned>(P::ptr(clip),0)=109;field<uintptr_t>(P::ptr(clip),4)=P::ptr(asset);clips[109]=P::ptr(clip);
 field<unsigned>(P::ptr(metadata),109*0x18+0x14)=1;
 DWORD protect{};require(VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&protect)!=0,"Fixture protection");
 f.jump(0x14ad40,reinterpret_cast<void*>(&projectile_provider137));f.jump(0x15c530,reinterpret_cast<void*>(&ability_provider137));f.jump(0x3a8a40,reinterpret_cast<void*>(&transition_provider137));
 VirtualProtect(f.image,0x1580000,protect,&protect);FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
 require(MH_Initialize()==MH_OK,"Init presentation hook");
 require(MH_CreateHook(reinterpret_cast<void*>(base+0x3e3f50),reinterpret_cast<void*>(&weapon_change_hook),reinterpret_cast<void**>(&original_change))==MH_OK,"Native state hook");
 require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Enable presentation hook");
 using State=void(__thiscall*)(void*,unsigned,unsigned,unsigned);auto change=reinterpret_cast<State>(base+0x3e3f50);
 using Tick=void(__thiscall*)(void*);auto pose=reinterpret_cast<Tick>(base+0x3dd9d0);
 auto turn=reinterpret_cast<unsigned(__thiscall*)(void*,short)>(base+0x3e3010);
 equipment_slot81=equipment81::Melee;field<unsigned char>(unit,0xead)=1;equipment_generation81=state->lease_generation;
 unsigned cases=0;
 for(const auto& row:PERSONAL_WEAPONS){
  field<unsigned>(projectile137,0)=254;field<unsigned>(projectile137,4)=unsigned(std::wcslen(row.key));field<unsigned>(projectile137,8)=128;field<uintptr_t>(projectile137,0xc)=reinterpret_cast<uintptr_t>(row.key);
  field<unsigned>(projectile137,0x60)=100;field<float>(projectile137,0x6c)=100;
  require(native_personal_kind(projectile137)==row.kind,"Native projectile family classification");
  for(unsigned ammo:{0u,10u})for(unsigned mode:{0u,4u,8u,9u})for(unsigned gait:{0u,1u,2u}){
   field<unsigned>(unit,0xf7c)=ammo;field<unsigned>(a,0x1f0)=8;field<unsigned>(a,0x244)=gait;
   field<float>(a,0x1ac)=gait?6.f:0.f;field<float>(a,0x150)=gait?1.f:0.f;field<unsigned>(a,0x1d8)=23;
   if(mode==0||mode==4){
    // Compare with the unhooked native result, including a native refusal.
    // Restore identical actor bytes before each call; do not assume every state is available.
    auto snapshot=f.actor;
    original_change(reinterpret_cast<void*>(a),mode,0,0);
    auto native_result=f.actor;
    memcpy(reinterpret_cast<void*>(a),snapshot.data(),snapshot.size());
    unsigned before{},after{};__asm mov before,esp
    change(reinterpret_cast<void*>(a),mode,0,0);__asm mov after,esp
    require(before==after&&!memcmp(reinterpret_cast<void*>(a),native_result.data(),native_result.size()),"Ordinary locomotion differs from the original native state call");
    // Readiness states may legitimately synchronize; ordinary states must not.
    for(unsigned ordinary:{0u,4u}){
     field<unsigned>(a,0x1f0)=ordinary;equipment_prepare_motion81();
     require(field<unsigned>(a,0x1f0)==ordinary,"Equipment boundary restored the rejected combat locomotion");
    }
    ++cases;continue;
   }
   unsigned before{},after{};__asm mov before,esp
   change(reinterpret_cast<void*>(a),mode,0,0);__asm mov after,esp
   require(before==after,"Native state call stack changed");
   require(field<unsigned>(a,0x1f0)==10,"Native readiness lost melee state");
   pose(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==20,"Original native moving pose dispatch lost combat-ready state");
   require(field<unsigned>(a,0x244)==gait&&field<float>(a,0x1ac)==(gait?6.f:0.f)&&field<unsigned>(a,0x1d8)==23,"Presentation changed gait, speed or animation frame");
   for(short angle:{short(-0x4000),short(0),short(0x4000)})require(turn(reinterpret_cast<void*>(a),angle)==0x4a3,"Generic bow turn clip replaced native combat-ready blend");
   field<unsigned>(a,0x48c+4)=3;field<float>(a,0x48c+0xc)=2.f;
   change(reinterpret_cast<void*>(a),mode,0,0);require(field<unsigned>(a,0x490)==3&&field<float>(a,0x498)==2.f,"Same native melee state reset idle/animation sampler every update");
   ++cases;
  }
 }
 // Leave ordinary native locomotion alone; synchronize only an actual readiness request.
 field<float>(base,0x15b2448)=.04f;equipment_ranged81=1;equipment_slot81=equipment81::Melee;
 field<unsigned>(unit,0xf7c)=10;field<unsigned>(a,0x1f0)=0;field<unsigned>(a,0x490)=0;
 equipment_prepare_motion81();require(field<unsigned>(a,0x1f0)==0,"Already-melee entry replaced native locomotion");
 field<unsigned>(a,0x1f0)=4;equipment_prepare_motion81();require(field<unsigned>(a,0x1f0)==4,"Equipment boundary replaced mode4 locomotion");
 field<unsigned>(a,0x1f0)=8;
 require(!equipment_prepare_motion81()&&field<unsigned>(a,0x1f0)==10,"Native melee readiness did not synchronize");
 auto switches=equipment_switches81;for(unsigned i=0;i<1000;++i){field<unsigned>(a,0x1d8)=i;equipment_prepare_motion81();require(equipment_switches81==switches&&field<unsigned>(a,0x1d8)==i,"Melee pose was repeatedly forced/reset");}
 field<unsigned>(unit,0xf7c)=10;field<unsigned>(a,0x1f0)=8;
 field<unsigned char>(unit,0xead)=0;change(reinterpret_cast<void*>(a),8,0,0);require(field<unsigned>(a,0x1f0)==8,"Native ranged mode changed");field<unsigned char>(unit,0xead)=1;
 field<unsigned>(a,0x1f0)=8;state->release=1;change(reinterpret_cast<void*>(a),8,0,0);require(field<unsigned>(a,0x1f0)==8,"Released actor presentation retained");state->release=0;
 field<unsigned>(a,0x1f0)=8;own_weapon_call=1;change(reinterpret_cast<void*>(a),8,1,0);own_weapon_call=0;require(field<unsigned>(a,0x1f0)==8,"Explicit owned state call changed");
 field<unsigned>(a,0x1f0)=8;field<uintptr_t>(a,0x688)=1;change(reinterpret_cast<void*>(a),8,1,0);require(field<unsigned>(a,0x1f0)==8,"Native paired combat state rewritten");field<uintptr_t>(a,0x688)=0;
 field<unsigned>(a,0x1f0)=8;clips[109]=0;change(reinterpret_cast<void*>(a),8,1,0);require(field<unsigned>(a,0x1f0)==8,"Missing native melee animation forced");clips[109]=P::ptr(clip);
 field<unsigned>(a,0x1f0)=8;auto serial=field<unsigned>(a,0x1c8);++field<unsigned>(a,0x1c8);require(melee_presentation_mode137(a,8,0)==8,"Reused actor identity accepted");field<unsigned>(a,0x1c8)=serial;
 require(melee_presentation_mode137(P::ptr(f.peer),8,0)==8,"AI peer presentation altered");
 // Personal ranged readiness must not change the formation's melee order.
 for(const auto& row:PERSONAL_WEAPONS){
  field<unsigned>(projectile137,4)=unsigned(std::wcslen(row.key));field<uintptr_t>(projectile137,0xc)=reinterpret_cast<uintptr_t>(row.key);
  equipment_personal152=1;equipment_slot81=equipment81::Melee;equipment_personal_override214=1;
  field<unsigned char>(unit,0xead)=1;field<unsigned>(a,0x1f0)=8;
  require(equipment_ranged_selected81()&&!melee_presentation_owned137(a),"Personal ranged override blocked by unit melee");
  change(reinterpret_cast<void*>(a),8,1,0);require(field<unsigned>(a,0x1f0)==8&&field<unsigned char>(unit,0xead)==1,"Personal shot pose changed formation mode");
  equipment_personal_override214=0;field<unsigned>(a,0x1f0)=8;
  change(reinterpret_cast<void*>(a),8,1,0);require(field<unsigned>(a,0x1f0)==10&&!equipment_ranged_selected81(),"Releasing personal ranged override lost native melee pose");++cases;
 }
 #include "../../../../tools/deck/verify_locomotion203.h"
 // Native mounted principal proof, no infantry-only projectile allowlist.
 std::vector<unsigned char> horse(0x700),horse_holder(0x210),mount_config(0x100),record(0x100);
 auto h=P::ptr(horse);uintptr_t passenger=a;uintptr_t holders[]={P::ptr(f.holder),P::ptr(f.peer_holder),P::ptr(horse_holder)};
 field<uintptr_t>(h,0)=base+0x15e8524;field<uintptr_t>(h,0x214)=unit;field<uintptr_t>(h,0x20c)=P::ptr(mount_config);field<uintptr_t>(h,0x28)=environment;
 field<unsigned char>(h,0xf0)=field<unsigned char>(h,0xa1)=1;field<unsigned>(h,0x1c8)=44;field<unsigned>(h,0x6cc)=1;field<uintptr_t>(h,0x6d0)=reinterpret_cast<uintptr_t>(&passenger);
 field<uintptr_t>(a,0x210)=h;field<uintptr_t>(unit,0xf9c)=P::ptr(mount_config);field<uintptr_t>(unit,0x2c)=P::ptr(record);
 field<uintptr_t>(P::ptr(horse_holder),8)=h;field<unsigned>(P::ptr(f.view),0x18)=3;field<uintptr_t>(P::ptr(f.view),0x1c)=reinterpret_cast<uintptr_t>(holders);
 field<unsigned>(a,0x1f0)=29;require(followed(a,true),"Mounted native lease");state->heartbeat=GetTickCount64();
 for(const auto& row:PERSONAL_WEAPONS){
  field<unsigned>(projectile137,4)=unsigned(std::wcslen(row.key));field<uintptr_t>(projectile137,0xc)=reinterpret_cast<uintptr_t>(row.key);
  for(unsigned ammo:{0u,10u}){field<unsigned>(unit,0xf7c)=ammo;field<unsigned>(a,0x1f0)=29;change(reinterpret_cast<void*>(a),29,1,0);require(field<unsigned>(a,0x1f0)==30,"Mounted projectile lost melee presentation");++cases;}
 }
 #include "../../../../tools/deck/verify_locomotion206.h"
 require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK,"Disable presentation hook");MH_Uninitialize();
 std::cout<<"PASS: "<<cases<<" all-catalog projectile/native state cases, ordinary native locomotion pass-through, melee-ready pose dispatch and bow-turn rejection, empty ammo, native no-op sampler/frame preservation, actual state validation/change, all-catalog mounted reciprocal proof, ranged mode, release, serial reuse, peers, own calls, native paired combat and missing-asset guards. No renderer/game/scanout execution; transition lookup and projectile getter are providers.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
