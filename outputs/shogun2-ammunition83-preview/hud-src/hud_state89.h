#pragma once
#include <cmath>
namespace hudstate89 {
enum class Reticle {None,Firearm,Drop};
struct Input {bool equipment,selected,ammo_known,pending,aiming;unsigned kind,phase,aim_pose,icon,ammo_error;float ammunition,reload;};
struct State {const wchar_t* label;bool fire_prompt;Reticle reticle;};
inline State resolve(const Input& s){
 if(!s.equipment||!s.selected||s.kind<1||s.kind>3)return {L"Unavailable",false,Reticle::None};
 Reticle reticle=s.kind==2?Reticle::Firearm:Reticle::Drop;
 if(s.pending)return {L"Switching...",false,Reticle::None};
 if(!s.ammo_known||s.ammo_error||!std::isfinite(s.ammunition)||s.ammunition<0)return {L"Unavailable",false,reticle};
 if(s.ammunition<1)return {L"Out of ammo",false,reticle};
 if(s.phase==4||s.reload>.05f)return {L"Reloading",false,reticle};
 if(s.phase==3)return {L"Released",false,reticle};
 if(s.aim_pose==2||s.phase==1)return {s.kind==1?L"Drawing":L"Preparing",false,reticle};
 if(s.aiming&&s.aim_pose==1)return {s.kind==1?L"Drawn":s.kind==2?L"Raised":L"Readied",true,reticle};
 return {L"Lowered",false,reticle};
}
// Angular reference marks only. No range labels or ballistic impact prediction.
inline constexpr float drop_y[]={0.f,14.f,31.f,51.f};
inline constexpr float drop_width[]={16.f,12.f,9.f,6.f};
}
