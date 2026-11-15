// Exercise the production weapon hooks outside the game with synthetic native
// object layouts. No process injection, game input, or live battle test.
#include "identity.h"
#include <sstream>
#include "steering.h"
#include "aim_geometry.h"
#include "../vendor/include/MinHook.h"
static uintptr_t base,possessed,unit,environment;
static volatile LONG enabled=1;
static Status storage{},*state=&storage;
static bool identity_valid=true;static bool mounted_weapon=false;
static bool weapon_mounted(){return mounted_weapon;}
static bool personal_direct_projectile(uintptr_t p){return p!=0;}
static bool is_bow_projectile(uintptr_t p){return p&&*reinterpret_cast<unsigned*>(p)==3;}
static uintptr_t record;
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
static bool followed(uintptr_t actor,bool){return identity_valid&&actor==possessed&&field<uintptr_t>(actor,0x214)==unit;}
static uintptr_t bow_record(uintptr_t actor_unit){return actor_unit==unit?record:0;}
static bool weapon_ready_held(){return false;}
using NativeGet=void*(__thiscall*)(void*);
static bool posed_arrow_origin(float*){return false;}
static void pose_manual_origin(float*){}
static bool read_aim_frame(ViewRay&){return false;}
static void manual_bow_apply(const uint32_t*,uintptr_t){}
#include "equipment_policy81.h"
static volatile LONG equipment_personal_override214{};
static LONG equipment_slot81=equipment81::Ranged;
static bool equipment_crouched81(){return false;}
// This isolated fixture covers ordinary native firing. Full command-mode
// behavior is exercised with actual adapter ownership in verify_native_commands100.
static bool command_draw_permitted100(void*){return true;}
static void command_draw_started100(void*){}
static bool command_release_managed100(void*){return false;}
static void command_on_shot100(){}
// Standalone firing fixture: presentation eligibility is an external provider.
// Full native projectile presentation is covered by verify_melee_presentation137.
#include "personal_weapon_catalog.h"
static PersonalWeaponKind native_personal_kind(uintptr_t){return PersonalWeaponKind::Unsupported;}
#include "fixture_weapon_edges143.h"
#include "weapon_animation.h"

static unsigned stops,clocks;
struct Actor82 {
 unsigned char bytes[0x700]{};
 __declspec(noinline) void stop(){++stops;field<uintptr_t>(reinterpret_cast<uintptr_t>(this),0x474)=0;}
 __declspec(noinline) void clock(){++clocks;++field<unsigned>(reinterpret_cast<uintptr_t>(this),0x1d8);}
};
template<class T>static void* address82(T member){static_assert(sizeof(member)==4,"x86 native ABI");void* p;memcpy(&p,&member,4);return p;}
int main(){try{
 original_animation_stop82=reinterpret_cast<RangedAction>(address82(&Actor82::stop));
 original_animation_clock=reinterpret_cast<RangedAction>(address82(&Actor82::clock));
 Actor82 player,other;possessed=reinterpret_cast<uintptr_t>(&player);unit=0x100000;
 unsigned projectile=49;record=reinterpret_cast<uintptr_t>(&projectile);
 field<unsigned char>(possessed,0xf0)=1;field<uintptr_t>(possessed,0x214)=unit;
 field<unsigned>(possessed,0x1f0)=14;
 state->mode=POSSESSED;state->weapon_phase=WEAPON_DRAW;state->lease_generation=82;state->actor_id=92;state->unit_id=3;
 unsigned char asset[0xa0]{};auto ap=reinterpret_cast<uintptr_t>(asset);field<unsigned>(ap,0x64)=35;
 unsigned clip[3]{130,unsigned(ap),0};auto cp=reinterpret_cast<uintptr_t>(clip);
 auto arm=[&](){field<uintptr_t>(possessed,0x474)=cp;field<unsigned>(possessed,0x1d4)=130;field<unsigned>(possessed,0x1d8)=3;};
 arm();auto before=player;weapon_animation_clock_hook(&player,nullptr);
 require(clocks==1&&field<unsigned>(possessed,0x1d8)==4,"Observer changed native advancement");
 field<unsigned>(reinterpret_cast<uintptr_t>(&before),0x1d8)=4;
 require(!memcmp(&before,&player,sizeof player),"Clock observation wrote actor fields");
 require(weapon_observation_write82==2&&weapon_observation_ring82[0].frame==3&&weapon_observation_ring82[1].frame==4,"Missing native frame pair");
 weapon_animation_stop_hook82(&player,nullptr);
 require(stops==1&&!field<uintptr_t>(possessed,0x474),"Stop observer suppressed native cancellation");
 require(weapon_observation_write82==4&&weapon_observation_ring82[2].animation==cp&&!weapon_observation_ring82[3].animation,"Stop event did not bracket cancellation");
 require(weapon_observation_ring82[2].count==35&&weapon_observation_ring82[2].phase==WEAPON_DRAW,"Clip length/phase lost");
 std::ostringstream log;log_weapon_observation82(log);require(log.str().find("weapon_observation82")!=std::string::npos,"Trace did not flush");
 auto seen=weapon_observation_write82;
 for(unsigned guard=0;guard<8;++guard){
  enabled=1;identity_valid=true;state->mode=POSSESSED;state->stop=state->release=0;state->weapon_phase=WEAPON_DRAW;field<unsigned char>(possessed,0xf0)=1;record=reinterpret_cast<uintptr_t>(&projectile);
  switch(guard){case 0:enabled=0;break;case 1:identity_valid=false;break;case 2:state->mode=IDLE;break;case 3:state->stop=1;break;case 4:state->release=1;break;case 5:field<unsigned char>(possessed,0xf0)=0;break;case 6:record=0;break;case 7:state->weapon_phase=0;state->aim_pose=0;break;}
  arm();weapon_animation_stop_hook82(&player,nullptr);weapon_animation_clock_hook(&player,nullptr);
  require(stops==guard+2&&clocks==guard+2,"Ownership guard changed native callbacks");
  require(weapon_observation_write82==seen,"Observer recorded a released/stale subject");
 }
 enabled=1;identity_valid=true;state->mode=POSSESSED;state->weapon_phase=WEAPON_DRAW;
 unsigned char other_before[sizeof other];memcpy(other_before,&other,sizeof other);
 for(unsigned i=0;i<10000;++i)weapon_animation_stop_hook82(&other,nullptr);
 require(stops==10009&&weapon_observation_write82==seen&&!memcmp(other_before,&other,sizeof other),"Other-actor stop forwarding changed");
 for(unsigned i=0;i<800;++i)observe_weapon82(possessed,5,0);
 require(weapon_observation_write82-weapon_observation_read82<=512,"Trace queue is unbounded");
 // Restore a valid owned subject for overflow exercise.
 record=reinterpret_cast<uintptr_t>(&projectile);field<unsigned char>(possessed,0xf0)=1;
 for(unsigned i=0;i<800;++i)observe_weapon82(possessed,5,0);
 require(weapon_observation_write82-weapon_observation_read82==512&&weapon_observation_dropped82>0,"Trace overflow was not bounded and counted");
 std::cout<<"PASS: production observation hooks forward stop/clock exactly once; native frames and animation pointers unchanged; ownership/death/disable guards; 10000 other-actor calls; bounded overflow and log flush. No gameplay fix asserted.\n";
 return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
