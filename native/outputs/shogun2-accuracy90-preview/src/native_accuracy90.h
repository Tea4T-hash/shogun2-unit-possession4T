#pragma once
#include "accuracy_geometry90.h"
#include "accuracy_protocol90.h"
// Exact native interfaces: attacker vtable15e736c, projectile15e73bc.
// Unit marksmanship (+234) is read through the native attacker getter; native
// control/weather/ammunition/angle/difficulty providers retain current effects.
struct AccuracyProvider90 {uintptr_t vtable,object;uint32_t difficulty;unsigned char human;unsigned char padding[3];};
static_assert(sizeof(AccuracyProvider90)==16,"Native accuracy provider ABI");
using AccuracyDifficulty90=void(__thiscall*)(void*,void*,unsigned char*,uint32_t*);
using AccuracyChance90=float(__cdecl*)(void*,void*,void*,unsigned);
using AccuracyFloat90=float(__thiscall*)(void*);
static HANDLE accuracy_mapping90;static accuracy90::Status* accuracy_shared90;
static SRWLOCK accuracy_lock90=SRWLOCK_INIT;
static volatile LONG accuracy_update_total90,accuracy_error_total90;
static float accuracy_projection_y90;
static uint64_t next_accuracy90;
static uint32_t rts_captures90,rts_restores90,rts_rejects90,rts_saved90;
static bool accuracy_signatures90(){
 const unsigned char difficulty[]={0x51,0x55,0x56,0x8b,0x74,0x24,0x10};
 const unsigned char core[]={0x51,0x53,0x56,0x8b,0xf1,0x57,0x8b,0x4e};
 const unsigned char projection[]={0x83,0xec,0x08,0x53,0x8b,0xd9};
 return !memcmp(reinterpret_cast<void*>(base+0x4324f0),difficulty,sizeof difficulty)&&!memcmp(reinterpret_cast<void*>(base+0x43d500),core,sizeof core)&&
  !memcmp(reinterpret_cast<void*>(base+0x1367e20),projection,sizeof projection)&&field<uintptr_t>(base+0x15e736c,0x1c)==base+0x43d500&&
  field<uintptr_t>(base+0x15e73bc,0x18)==base+0x436220&&field<uintptr_t>(base+0x15d22e8,0xd0)==base+0x1367e20&&
  field<uintptr_t>(base+0x15d1a74,0xf4)==base+0x134fe50&&field<uintptr_t>(base+0x15d26fc,0xf4)==base+0x134fe50;
}
static void accuracy_initialize90(){wchar_t name[96];accuracy90::mapping_name(name,GetCurrentProcessId());
 accuracy_mapping90=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(accuracy90::Status),name);
 require(accuracy_mapping90&&GetLastError()!=ERROR_ALREADY_EXISTS,"Accuracy90 mapping already exists");
 accuracy_shared90=static_cast<accuracy90::Status*>(MapViewOfFile(accuracy_mapping90,FILE_MAP_ALL_ACCESS,0,0,sizeof(accuracy90::Status)));
 require(accuracy_shared90!=nullptr,"Accuracy90 mapping failed");memset(accuracy_shared90,0,sizeof(*accuracy_shared90));
 accuracy_shared90->magic=accuracy90::Magic;accuracy_shared90->version=90;accuracy_shared90->size=sizeof(*accuracy_shared90);accuracy_shared90->pid=GetCurrentProcessId();
}
static bool measure_accuracy90(uintptr_t actor,uintptr_t common,uintptr_t record,const void* solved,accuracy90::Envelope& out){
 alignas(16) unsigned char copy[0x80]{};AccuracyProvider90 attacker{},projectile{};
 __try {
  if(!actor||!common||!record||!solved)return false;
  memcpy(copy,solved,sizeof copy);auto p=reinterpret_cast<uintptr_t>(copy);
  float range=field<float>(record,0xe8);if(!std::isfinite(range)||range<=0||range>100000)return false;
  // Accuracy is calibrated at the projectile's own native half-hit distance.
  // A 6000m direction marker must not force every unit to minimum chance-to-hit.
  field<float>(p,0x68)=range;
  auto pool=field<uintptr_t>(common,0x9c);if(!pool)return false;
  unsigned char human=1;uint32_t difficulty=0;
  reinterpret_cast<AccuracyDifficulty90>(base+0x4324f0)(reinterpret_cast<void*>(pool),reinterpret_cast<void*>(actor),&human,&difficulty);
  attacker={base+0x15e736c,actor,difficulty,human,{}};projectile={base+0x15e73bc,p,difficulty,human,{}};
  float chance=reinterpret_cast<AccuracyChance90>(base+0xb92b60)(reinterpret_cast<void*>(pool+4),&attacker,&projectile,0);
  float variance=field<float>(record,0xec),ratio=field<float>(base,0x179af38);
  using Flag=unsigned char(__thiscall*)(void*);using Get=void*(__thiscall*)(void*);
  if(reinterpret_cast<Flag>(base+0x458e10)(&attacker)==1){
   auto native_unit=reinterpret_cast<uintptr_t>(reinterpret_cast<Get>(field<uintptr_t>(field<uintptr_t>(actor,0),0xa8))(reinterpret_cast<void*>(actor)));
   if(native_unit){unsigned kind=field<unsigned>(native_unit,0x748);if(kind==0||kind==11||kind==12||kind==14||kind==15||kind==16)ratio=field<float>(base,0x179af3c);}
  }
  unsigned count=field<unsigned>(p,0x44);if(count)variance=std::max(variance*.5f,variance*(1.f-.1f*float(count)));
  if(field<uintptr_t>(p,0x1c)&&field<unsigned char>(p,0x78)){
   auto owner=field<uintptr_t>(field<uintptr_t>(common,0),0x2804c);auto data=owner?field<uintptr_t>(owner,0x10):0;
   auto globals=data?reinterpret_cast<uintptr_t>(reinterpret_cast<Get>(base+0xbd1800)(reinterpret_cast<void*>(data))):0;
   auto value=globals?reinterpret_cast<uintptr_t>(reinterpret_cast<Get>(base+0x1be3a0)(reinterpret_cast<void*>(globals+0x2d6c))):0;
   float divisor=value?field<float>(value,0):0;if(!std::isfinite(divisor)||divisor<=0)return false;variance/=divisor;
  }
  if(!accuracy90::envelope(variance,chance,range,ratio,1,out))return false;
  out.core=reinterpret_cast<AccuracyFloat90>(base+0x43d500)(&attacker);
  out.control=reinterpret_cast<AccuracyFloat90>(base+0x43d250)(&attacker);
  if(!std::isfinite(out.core)||!std::isfinite(out.control))return false;
  const auto kind=native_personal_kind(record);
  if(kind==PersonalWeaponKind::Bow||kind==PersonalWeaponKind::Firearm){
   // The preview and manual projectile release both use this final cone.
   const float miss=std::clamp((1.f-out.chance)/.99f,0.f,1.f);
   const float spread=std::tan(out.half_angle)*(.35f+.65f*miss)+.135f*miss*miss*miss;
   out.half_angle=std::atan(std::clamp(spread,.006f,accuracy90::maximum_aimed_spread125));
  }else if(kind==PersonalWeaponKind::Thrown){
   out.half_angle=std::atan(std::tan(out.half_angle)*accuracy90::fixed_sight_width_scale125);
  }
  return true;
 }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&accuracy_error_total90);return false;}
}
static void publish_accuracy90(const accuracy90::Envelope* e,uintptr_t record,unsigned error,uint32_t expected_lease=0){
 if(e&&(!possession_lease.active||state->mode!=POSSESSED||(expected_lease&&expected_lease!=state->lease_generation))){e=nullptr;error=7;}
 if(!accuracy_shared90)return;unsigned record_id=0,simulation_tick=0;__try{if(record)record_id=field<unsigned>(record,0);if(e&&possession_lease.active)simulation_tick=field<unsigned>(possession_lease.identity.common,0x50);}__except(EXCEPTION_EXECUTE_HANDLER){e=nullptr;error=8;}
 AcquireSRWLockExclusive(&accuracy_lock90);InterlockedIncrement(&accuracy_shared90->sequence);MemoryBarrier();
 accuracy_shared90->lease=state->lease_generation;accuracy_shared90->actor=state->actor_id;accuracy_shared90->unit=state->unit_id;
 accuracy_shared90->record=record_id;accuracy_shared90->valid=e?1:0;accuracy_shared90->error=error;
 accuracy_shared90->simulation_tick=simulation_tick;accuracy_shared90->tick=GetTickCount64();accuracy_shared90->updates=static_cast<uint32_t>(accuracy_update_total90);
 if(e){accuracy_shared90->half_angle=e->half_angle;accuracy_shared90->horizontal=e->horizontal;accuracy_shared90->vertical=e->vertical;accuracy_shared90->chance=e->chance;
  accuracy_shared90->calibration=e->calibration;accuracy_shared90->core=e->core;accuracy_shared90->control=e->control;}
 accuracy_shared90->projection_y=accuracy_projection_y90;
 accuracy_shared90->rts_captures=rts_captures90;accuracy_shared90->rts_restores=rts_restores90;accuracy_shared90->rts_rejects=rts_rejects90;accuracy_shared90->rts_saved=rts_saved90;
 MemoryBarrier();InterlockedIncrement(&accuracy_shared90->sequence);ReleaseSRWLockExclusive(&accuracy_lock90);
}
static void refresh_accuracy90(uintptr_t actor){
 uint64_t now=GetTickCount64();if(now<next_accuracy90)return;next_accuracy90=now+100;
 alignas(16) unsigned char launch[0x80]{},parameters[0x48]{};
 __try {
  if(actor!=possessed||!followed(actor,false)||!equipment_ranged_selected81()){publish_accuracy90(nullptr,0,0);return;}
  auto record=bow_record(unit);auto common=possession_lease.identity.common;uint32_t lease=state->lease_generation;
  if(!record||!common){publish_accuracy90(nullptr,0,1);return;}
  ViewRay ray{};if(!read_aim_frame(ray)){publish_accuracy90(nullptr,record,2);return;}
  // Match release's direct-fire solve. The chance provider still evaluates
  // accuracy at the projectile's native calibration distance.
  const bool direct=personal_direct_projectile(record);
  float origin[3],target[3];
  for(unsigned i=0;i<3;++i)origin[i]=ray.origin[i]+ray.direction[i]*.4f;
  crosshair_target(ray,target,direct?6000.f:80.f);
  reinterpret_cast<Parameters>(base+0x40ebe0)(parameters,origin,mountedshot87::native_arc(weapon_mounted()),reinterpret_cast<void*>(actor),field<float>(unit,0x164),field<float>(unit,0x170),nullptr,nullptr,nullptr);
  parameters[0x44]=direct?1:0;
  reinterpret_cast<Launch>(base+0x40e9d0)(launch,reinterpret_cast<void*>(common),reinterpret_cast<void*>(record),parameters,field<float>(record,0x6c));
  if(!reinterpret_cast<Ballistic>(base+0x420050)(launch,target)){publish_accuracy90(nullptr,record,3);return;}
  accuracy90::Envelope e{};bool ok=measure_accuracy90(actor,common,record,launch,e);if(ok)InterlockedIncrement(&accuracy_update_total90);
  publish_accuracy90(ok?&e:nullptr,record,ok?0:4,lease);
 }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&accuracy_error_total90);publish_accuracy90(nullptr,0,5);}
}
static void accuracy_projection90(float value){
 if(!std::isfinite(value)||value<=0)return;accuracy_projection_y90=value;if(!accuracy_shared90)return;
 AcquireSRWLockExclusive(&accuracy_lock90);InterlockedIncrement(&accuracy_shared90->sequence);MemoryBarrier();accuracy_shared90->projection_y=value;
 MemoryBarrier();InterlockedIncrement(&accuracy_shared90->sequence);ReleaseSRWLockExclusive(&accuracy_lock90);
}
static void accuracy_log90(std::ostream& log){
 if(!accuracy_shared90)return;accuracy90::Status s{};AcquireSRWLockShared(&accuracy_lock90);memcpy(&s,accuracy_shared90,sizeof s);ReleaseSRWLockShared(&accuracy_lock90);
 log<<",\"accuracy90_valid\":"<<s.valid<<",\"accuracy90_error\":"<<s.error<<",\"accuracy90_updates\":"<<s.updates<<",\"effective_accuracy\":"<<s.core<<",\"accuracy_control\":"<<s.control
 <<",\"calibration_chance\":"<<s.chance<<",\"calibration_range\":"<<s.calibration<<",\"live_cone_half_angle\":"<<s.half_angle<<",\"projection_y\":"<<s.projection_y
 <<",\"rts_view_captures\":"<<rts_captures90<<",\"rts_view_restores\":"<<rts_restores90<<",\"rts_view_rejects\":"<<rts_rejects90<<",\"rts_view_saved\":"<<rts_saved90;
}
