#pragma once
#include "personal_weapon_metadata122.h"
static void ammunition_item83(uintptr_t record,ammo83::Item& item){
 if(!record)return;item.projectile=field<unsigned>(record,0);
 const unsigned n=field<unsigned>(record,4),capacity=field<unsigned>(record,8);
 const auto chars=field<uintptr_t>(record,0xc);
 if(chars>=65536&&n&&n<=128&&capacity>=n&&capacity<=1048576){
  wchar_t key[129]{};const bool wide=field<unsigned char>(chars,1)==0;
  for(unsigned i=0;i<n;++i)key[i]=wide?field<wchar_t>(chars,i*2):field<unsigned char>(chars,i);
  item.icon=ammo83::icon_for_key(key);
 }
 const auto launcher=launcher_sight123(record);
 if(launcher==LauncherSight123::Rocket)item.icon=ammo83::Rocket;
 else if(launcher==LauncherSight123::Mortar)item.icon=ammo83::Mortar;
 const auto kind=native_personal_kind(record);
 if((item.icon==ammo83::Rocket||item.icon==ammo83::Mortar)&&kind==PersonalWeaponKind::Firearm)
  item.icon=ammo83::Bullet;
 if(item.icon==ammo83::Unknown){
  switch(kind){
   case PersonalWeaponKind::Bow:item.icon=ammo83::Arrow;break;
   case PersonalWeaponKind::Firearm:item.icon=ammo83::Bullet;break;
   case PersonalWeaponKind::Thrown:item.icon=ammo83::Bomb;break;
   default:break;
  }
 }
 wcscpy_s(item.label,ammo83::label(ammo83::Icon(item.icon)));
}
static void publish_ammunition83(const equipment81::AmmoOption* options,unsigned count,uintptr_t active,unsigned pending,unsigned error){
 if(!ammunition_status83||!state)return;
 ammo83::Status sample{};sample.lease=state->lease_generation;sample.actor=state->actor_id;sample.unit=state->unit_id;
 sample.tick=GetTickCount64();sample.pending=pending;sample.error=error;bool matched=false;
 for(unsigned i=0;i<count&&i<33;++i){auto& item=sample.items[sample.count++];
  ammunition_item83(options[i].record,item);item.ability=options[i].id;
  if(i==0)item.flags|=ammo83::Normal;
  bool available=i==0?field<float>(unit,0xf78)>=1:equipment_unit_ability81(options[i].id,true);
  if(available)item.flags|=ammo83::Available;
  if(options[i].record==active){sample.current=i;item.flags|=ammo83::Selected;matched=true;}
 }
 // An inconsistent native snapshot must never present normal ammo as selected.
 if(!matched){sample.count=0;sample.current=0;}
 AcquireSRWLockExclusive(&ammunition_publish_lock83);InterlockedIncrement(&ammunition_status83->sequence);MemoryBarrier();
 memcpy(&ammunition_status83->lease,&sample.lease,sizeof(sample)-offsetof(ammo83::Status,lease));
 MemoryBarrier();InterlockedIncrement(&ammunition_status83->sequence);ReleaseSRWLockExclusive(&ammunition_publish_lock83);
}
