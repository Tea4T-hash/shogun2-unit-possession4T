#pragma once
// Native manager31ec00 iterates 0x282c-byte live ability records. The catalog
// supplies artwork only; native15c530/15be80 remain the unlock/use authority.
#include "ability_target143.h"
static bool active_suppression_projectile98(uintptr_t actor_unit,uintptr_t projectile){
    const auto manager=actor_unit+0xed4;
    if(field<uintptr_t>(manager,0)!=base+0x15cfeec||field<uintptr_t>(manager,8)!=actor_unit)return false;
    const auto count=field<unsigned>(manager,0x28),capacity=field<unsigned>(manager,0x24);
    const auto array=field<uintptr_t>(manager,0x2c);
    if(!array||count>128||capacity<count||capacity>4096)return false;
    for(unsigned i=0;i<count;++i){
        const auto entry=array+i*0x282c;
        if(field<unsigned char>(entry,0x18)!=1)continue;
        const auto record=field<uintptr_t>(entry,4);
        if(record&&field<unsigned>(record,0)==61&&field<uintptr_t>(record,0x28)==projectile)return true;
    }
    return false;
}
static bool ability_collect141(ability141::Status& s){
 auto manager=unit+0xed4;
 if(field<uintptr_t>(manager,0)!=base+0x15cfeec||field<uintptr_t>(manager,8)!=unit)return false;
 auto count=field<unsigned>(manager,0x28),capacity=field<unsigned>(manager,0x24);auto array=field<uintptr_t>(manager,0x2c);
 if(count>128||capacity<count||capacity>4096||(count&&!array))return false;
 for(unsigned i=0;i<count&&s.count<ability141::Capacity;++i){
  __try {
   auto entry=array+i*0x282c,record=field<uintptr_t>(entry,4);if(!record)continue;
   auto id=field<unsigned>(record,0);if(!ability141::art(id)||!equipment_unit_ability81(id,false))continue;
   bool duplicate=false;for(unsigned j=0;j<s.count;++j)if(s.items[j].id==id)duplicate=true;if(duplicate)continue;
   float cooldown=field<float>(entry,0xc);if(!std::isfinite(cooldown)||std::abs(cooldown)>86400)continue;
   auto& item=s.items[s.count++];item.id=id;if(id==35)item.flags|=ability141::Targeted;item.cooldown=std::max(0.f,cooldown);
   if(equipment_unit_ability81(id,true))item.flags|=ability141::Available;
   if(field<unsigned char>(entry,0x18))item.flags|=ability141::Active;
   if(field<uintptr_t>(record,0x28))item.flags|=ability141::Ammunition;
  }__except(EXCEPTION_EXECUTE_HANDLER){continue;}
 }
 return true;
}
static bool ability_send141(const ability141::Status& s,unsigned index){
 if(index>=s.count||!equipment_unit_ability81(s.items[index].id,true))return false;
 const auto& selected=s.items[index];
 bool active=(selected.flags&ability141::Active)!=0;
 if(selected.id==35){
  auto target=ability_aim_friend143();if(!target||!ability_inspire143(target))return false;
  ability_sound143(selected.id);ability_pending_until141=GetTickCount64()+750;return true;
 }
 // Ammunition uses the accepted native two-command transaction. Never change
 // styles while a personal shot/reload is committed. No projectile/stat writes.
 if((selected.flags&ability141::Ammunition)&&(!equipment_boundary81().safe()||equipment_ammo_inflight81||state->aim_pose))return false;
 alignas(16) unsigned char packets[2][QUEUE_CAPACITY+4]{};uint32_t sizes[2]{};unsigned commands=0;
 if(!active&&(selected.flags&ability141::Ammunition))for(unsigned i=0;i<s.count;++i){
  if(i==index||(s.items[i].flags&(ability141::Active|ability141::Ammunition))!=(ability141::Active|ability141::Ammunition))continue;
  if(!serialize_ability81(s.items[i].id,1,packets[commands],sizes[commands]))return false;++commands;break;
 }
 if(!serialize_ability81(selected.id,active?1:0,packets[commands],sizes[commands]))return false;++commands;
 auto queue=field<uintptr_t>(environment,0x28048);if(!queue)return false;
 auto& used=field<uint32_t>(queue+4,QUEUE_CAPACITY);auto key=field<uint32_t>(base,0x192ee34);
 unsigned total=sizes[0]+sizes[1];if(used>QUEUE_CAPACITY||total>QUEUE_CAPACITY-used)return false;
 // All packets already validated; the sole native queue writer appends atomically.
 auto destination=reinterpret_cast<unsigned char*>(queue+4);unsigned previous=used;
 for(unsigned i=0;i<commands;++i){if(!valid_packet(packets[i],sizes[i],key))return false;}
 memcpy(destination+previous,packets[0],sizes[0]);if(commands==2)memcpy(destination+previous+sizes[0],packets[1],sizes[1]);
 MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&used),LONG(previous+total));
 ability_sound143(selected.id);ability_pending_until141=GetTickCount64()+750;return true;
}
static void ability_process141(uintptr_t camera){
 if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||dead_view()||!camera_owned(camera)||!followed(possessed,false)||GetCurrentThreadId()!=state->window_thread)return;
 __try {
  auto now=GetTickCount64();AbilityRequest141 request;
  AcquireSRWLockExclusive(&ability_input_lock141);request=ability_request141;ability_request141.steps=0;ability_request141.use=false;ReleaseSRWLockExclusive(&ability_input_lock141);
  if(ability_lease141!=state->lease_generation){ability_lease141=state->lease_generation;ability_selected141=~0u;ability_next141=0;ability_pending_until141=0;ability_error141=0;}
  if(now<ability_next141&&!request.steps&&!request.use)return;ability_next141=now+100;
  ability141::Status sample{};sample.lease=state->lease_generation;sample.actor=state->actor_id;sample.unit=state->unit_id;sample.tick=now;
  if(!ability_collect141(sample)){ability_error141=1;return;}
  unsigned selected=0;for(unsigned i=0;i<sample.count;++i)if(sample.items[i].id==ability_selected141)selected=i;
  if(request.lease==state->lease_generation&&sample.count){selected=(selected+request.steps)%sample.count;
   if(request.use&&now>=ability_pending_until141)ability_error141=ability_send141(sample,selected)?0u:2u;
  }
  if(sample.count)ability_selected141=sample.items[selected].id;sample.selected=selected;sample.pending=now<ability_pending_until141;sample.error=ability_error141;
  if(ability_shared141){InterlockedIncrement(&ability_shared141->sequence);MemoryBarrier();
   memcpy(&ability_shared141->lease,&sample.lease,sizeof(sample)-offsetof(ability141::Status,lease));MemoryBarrier();InterlockedIncrement(&ability_shared141->sequence);}
 }__except(EXCEPTION_EXECUTE_HANDLER){ability_error141=3;}
}
