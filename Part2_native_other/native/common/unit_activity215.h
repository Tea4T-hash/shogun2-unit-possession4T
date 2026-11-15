#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "../outputs/shogun2-possession-cleanup-v46/src/actor_types152.h"
namespace unitactivity215 {
enum class Activity {Holding,Walking,Running,Firing,Melee,Routing};
struct Snapshot {Activity activity{Activity::Holding};unsigned peers{},moving{},running{},firing{},melee{};};

template<class Read>bool playing_melee(Read read,uintptr_t image,uintptr_t env,uintptr_t actor,uintptr_t unit,uintptr_t witness){
 if(!witness)return false;
 uintptr_t owner_env{},a{},b{};
 unsigned count{};
 unsigned char active{},finished{},playing{};
 // Original 3eaac0 separates aligning reservations from started playback.
 if(!read(witness+4,owner_env)||owner_env!=env||!read(witness+0xc,active)||active!=1||
    !read(witness+0xd0,finished)||finished||!read(witness+0xd1,playing)||playing!=1||
    !read(witness+0x38,count)||count!=2||!read(witness+0x30,a)||!read(witness+0x34,b)||
    a==b||(a!=actor&&b!=actor))return false;
 const auto other=a==actor?b:a;
 uintptr_t table{},other_env{},other_unit{},link{};
 unsigned side{},other_side{};
 return other&&read(other,table)&&(actor152::kind(image,table,0)||actor152::kind(image,table,1))&&
    read(other+0x28,other_env)&&other_env==env&&read(other+0x214,other_unit)&&other_unit&&other_unit!=unit&&
    read(actor+0x44,side)&&read(other+0x44,other_side)&&side!=other_side&&
    read(other+0x688,link)&&link==witness&&read(actor+0x688,link)&&link==witness&&
    read(witness+0xd0,finished)&&!finished&&read(witness+0xd1,playing)&&playing==1;
}
// Read-only observation of CURRENT unit roster members. The possessed soldier
// and its mount never contribute to these counts. This does not issue an order,
// run an engine task or change native unit/soldier state.
template<class Reader>bool sample(Reader read,uintptr_t image,uintptr_t env,uintptr_t unit,unsigned id,uintptr_t player,Snapshot& out){
 auto get=[&](uintptr_t at,auto& v){return read(at,&v,sizeof v);};
 unsigned count{},current_id{};uintptr_t list{};
 if(!unit||!get(unit+4,current_id)||current_id!=id||!get(unit+0x2bc,count)||count>2000||!get(unit+0x2c0,list)||(count&&!list))return false;
 std::array<uintptr_t,2000> members{};if(count&&!read(list,members.data(),count*sizeof(uintptr_t)))return false;
 Snapshot s;
 for(unsigned n=0;n<count;++n){auto actor=members[n];if(!actor||actor==player)continue;
  std::array<unsigned char,0x68c> bytes{};if(!read(actor,bytes.data(),bytes.size()))return false;
  auto field=[&](unsigned offset,auto& v){memcpy(&v,bytes.data()+offset,sizeof v);};
  uintptr_t table{},owner{},environment{},paired{},animation{},mount{},mount_pair{};
  unsigned serial{},mode{},clip{},gait{};
  float vx{},vz{};
  field(0,table);if(!actor152::human(image,table)||bytes[0xf0]!=1)continue;
  field(0x214,owner);field(0x28,environment);if(owner!=unit||environment!=env)return false;
  field(0x1c8,serial);field(0x1f0,mode);field(0x688,paired);field(0x474,animation);field(0x1d4,clip);field(0x210,mount);field(0x244,gait);field(0x8c,vx);field(0x94,vz);
  unsigned final_serial{};uintptr_t final_owner{};
  if(!get(actor+0x1c8,final_serial)||final_serial!=serial||!get(actor+0x214,final_owner)||final_owner!=unit)return false;
  if(mount){
   uintptr_t horse_table{},horse_unit{},horse_env{};unsigned char alive{};
   if(!get(mount,horse_table)||!actor152::kind(image,horse_table,1)||
      !get(mount+0x214,horse_unit)||horse_unit!=unit||!get(mount+0x28,horse_env)||horse_env!=env||
      !get(mount+0xf0,alive)||alive!=1||!get(mount+0x8c,vx)||!get(mount+0x94,vz)||
      !get(mount+0x244,gait)||!get(mount+0x688,mount_pair))return false;
  }
  if(!std::isfinite(vx)||!std::isfinite(vz))return false;
  ++s.peers;
  if(std::hypot(vx,vz)>.1f){
   ++s.moving;
   // Original 3e44f0 interprets +244 as walk (0), run (1), charge (2).
   if(gait==1||gait==2)++s.running;
  }
  if(playing_melee(get,image,env,actor,unit,paired)||
     (mount&&playing_melee(get,image,env,mount,unit,mount_pair)))++s.melee;
  // Native ready/fire/reload modes already used by personal_weapon_profile
  // and equipment81. An active matching native animation is required; a stale
  // clip ID, the player's aiming flag and unit fire-at-will permission are not
  // evidence that the formation is executing its ranged firing cycle.
  if(animation&&clip<0x4a3&&(mode==14||mode==15||mode==16||mode==17||mode==32||mode==33)){
   unsigned playing{};if(!get(animation,playing))return false;if(playing==clip)++s.firing;
  }
 }
 uintptr_t final_list{};unsigned final_count{};
 if(!get(unit+0x2c0,final_list)||final_list!=list||!get(unit+0x2bc,final_count)||final_count!=count||!get(unit+4,current_id)||current_id!=id)return false;
 if(s.melee)s.activity=Activity::Melee;else if(s.firing)s.activity=Activity::Firing;
 else if(s.moving&&s.moving>=std::max(1u,(s.peers+3)/4))
  s.activity=s.running*2>=s.moving?Activity::Running:Activity::Walking;
 out=s;return true;
}
inline const wchar_t* icon(Activity state){
 switch(state){
 case Activity::Walking:return L"ui/icons/data__status_walk.png";
 case Activity::Running:return L"ui/icons/data__status_run.png";
 case Activity::Firing:return L"ui/icons/data__status_firing.png";
 case Activity::Melee:return L"ui/icons/data__status_melee.png";
 case Activity::Routing:return L"ui/icons/data__status_routing.png";
 default:return L"ui/buttons/data__button_halt_active.png";
 }
}
inline float ammunition(float remaining,unsigned capacity){return std::isfinite(remaining)&&remaining>=0&&capacity&&capacity<=10000000?std::clamp(remaining/float(capacity),0.f,1.f):0.f;}
}
