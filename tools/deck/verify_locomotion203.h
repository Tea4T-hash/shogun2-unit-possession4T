{
 auto before203=f.actor;state->aiming=state->aim_pose=state->weapon_phase=state->release=0;state->heartbeat=GetTickCount64();
 field<uintptr_t>(a,0x18)=field<uintptr_t>(a,0x688)=0;field<unsigned char>(a,0x145)=field<unsigned char>(a,0x488)=field<unsigned char>(a,0x489)=0;
 field<unsigned char>(a,0x199)=0;field<unsigned short>(a,0x64)=0;field<float>(base,0x1bd9560)=0;field<float>(base,0x1bd9564)=1;
 DWORD old203{};VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old203);
 f.jump(0x367600,reinterpret_cast<void*>(integration203));f.jump(0x15cb80,reinterpret_cast<void*>(can_run203));f.jump(0x38a850,reinterpret_cast<void*>(special_gait203));
 VirtualProtect(f.image,0x1580000,old203,&old203);FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
 create_locomotion203();require(MH_EnableHook(reinterpret_cast<void*>(base+0x366a10))==MH_OK,"Motion completion hook enable");
 auto tick203=reinterpret_cast<UnitTick130>(base+0x366a10);unsigned checked203=0;
 // Actual original caller computes1ac but leaves old idle pose without handoff.
 fixture_mark203=false;fixture_vx203=2;fixture_vz203=0;field<unsigned>(a,0x1f0)=0;field<unsigned>(a,0x1d0)=0;
 tick203(reinterpret_cast<void*>(a));require(field<float>(a,0x1ac)==2&&field<unsigned>(a,0x1d0)==0,"Original idle-pose reproduction failed");
 fixture_mark203=true;
 for(unsigned mode:{0u,4u})for(unsigned gait:{0u,1u})for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz){
  state->input=FORWARD|(gait?RUN:0);state->heartbeat=GetTickCount64();field<unsigned>(a,0x1f0)=mode;field<unsigned>(a,0x1d0)=0;field<unsigned>(a,0x1d8)=77;
  fixture_vx203=float(dx*2);fixture_vz203=float(dz*2);tick203(reinterpret_cast<void*>(a));
  if(dx||dz)require(field<unsigned>(a,0x1d0)==(mode==4?(gait?14u:9u):(gait?13u:8u)),"Moving actor kept idle/wrong gait");
  else require(field<unsigned>(a,0x1d0)==(mode==4?1u:0u),"No actual movement produced walking pose");
  require(field<unsigned>(a,0x244)==gait&&field<unsigned>(a,0x1d8)==77,"Pose handoff changed animation clock");++checked203;
 }
 // The engine may keep the same combat idle active over several movement updates.
 // Do not interrupt it or replace the pose every frame while combat owns it.
 const auto saved_combat_clip=clip;const auto saved_combat_slot=clips[431];
 const auto combat_clip=P::ptr(clip);
 field<unsigned>(combat_clip,0)=431;clips[431]=combat_clip;
 field<unsigned>(a,0x1d4)=431;field<uintptr_t>(a,0x474)=combat_clip;
 field<unsigned>(a,0x1f0)=0;field<unsigned>(a,0x1d0)=33;
 state->input=FORWARD;fixture_vx203=2;fixture_vz203=0;
 for(unsigned frame=0;frame<8;++frame){
  field<unsigned>(a,0x1d8)=frame;
  tick203(reinterpret_cast<void*>(a));
  require(field<uintptr_t>(a,0x474)==combat_clip&&field<unsigned>(a,0x1d4)==431&&
      field<unsigned>(a,0x1d8)==frame&&field<unsigned>(a,0x1d0)==33,"Movement interrupted an active combat idle");
 }
 clips[431]=saved_combat_slot;memcpy(clip.data(),saved_combat_clip.data(),clip.size());
 field<uintptr_t>(a,0x474)=0;
 std::cout<<"PASS possession63: active combat-idle clip survives repeated motion updates\n";
 // Release through production weapon_tick and the original state change.
 auto old_reload203=original_reload_tick;
 original_reload_tick=reinterpret_cast<RangedAction>(&fixture_reload203);
 for(unsigned run:{0u,1u}){
  field<unsigned>(a,0x1f0)=14;state->aim_pose=AIM_READY;
  state->input=0;fixture_vx203=0;
  require(weapon_owned(a),"ADS release fixture has no weapon ownership");
  weapon_tick(a);
  require(!state->aim_pose&&field<unsigned>(a,0x1f0)==8&&foot_aim_exit203(a,8),
      "Native ADS exit did not record the lowered stance");
  tick203(reinterpret_cast<void*>(a));
  require(field<unsigned>(a,0x1f0)==8,"Stationary ADS exit entered travel stance");

  // An unrelated active clip takes precedence over the pending gait change.
  field<uintptr_t>(a,0x474)=P::ptr(clip);
  field<unsigned>(P::ptr(clip),0)=431;field<unsigned>(a,0x1d4)=431;clips[431]=P::ptr(clip);
  state->input=FORWARD|(run?RUN:0);fixture_vx203=2;
  tick203(reinterpret_cast<void*>(a));
  require(field<unsigned>(a,0x1f0)==8&&field<uintptr_t>(a,0x474)==P::ptr(clip),"ADS exit interrupted an unrelated clip");
  // Supply the owned transition clip and let the original interruption
  // routine decide whether it can end before its natural completion.
  const unsigned lower_id=run?128u:131u;
  const auto previous_lower=clips[lower_id];
  field<unsigned>(P::ptr(clip),0)=lower_id;
  field<unsigned>(a,0x1d4)=lower_id;clips[lower_id]=P::ptr(clip);
  foot_aim_lower.clip=P::ptr(clip);
  tick203(reinterpret_cast<void*>(a));
  if(run){
   require(field<unsigned>(a,0x1f0)==8&&field<uintptr_t>(a,0x474)==P::ptr(clip),
       "ADS exit canceled a native noninterruptible clip");
   field<uintptr_t>(a,0x474)=0;
   tick203(reinterpret_cast<void*>(a));
  }else require(!field<uintptr_t>(a,0x474),"ADS exit kept an interruptible lower clip during the walk");
  memcpy(clip.data(),saved_combat_clip.data(),clip.size());
  clips[431]=saved_combat_slot;clips[lower_id]=previous_lower;
  auto travel=field<unsigned>(a,0x1f0);
  require((travel==0||travel==4)&&field<unsigned>(a,0x1d0)==(travel==4?(run?14u:9u):(run?13u:8u)),
      "Released ADS kept the lowered stance while moving");
  require(!foot_aim_lower.actor,"ADS exit remained active after travel began");
 }
 original_reload_tick=old_reload203;
 state->input=FORWARD;
 std::cout<<"PASS ADS gait: native ADS release, stationary and unrelated-clip guards, and normal walk/run after release\n";
 combat_presence73={GetTickCount64(),possession_lease.generation,true};
 field<unsigned>(a,0x1f0)=0;state->input=FORWARD;fixture_vx203=2;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)==10&&field<unsigned>(a,0x1d0)==20,"Walking near a hostile missed the native combat gait");
 field<unsigned>(a,0x490)=99;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)==10&&field<unsigned>(a,0x490)==99,"Combat walking restarted its blend each update");
 state->input=FORWARD|RUN;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)<=7&&field<unsigned>(a,0x244)==1&&
     (field<unsigned>(a,0x1d0)==13||field<unsigned>(a,0x1d0)==14),"Sprint stayed in the combat shuffle");
 state->input=FORWARD;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)==10,"Walking again beside the enemy did not resume combat gait");
 combat_presence73={GetTickCount64(),possession_lease.generation,false};
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)<=7&&
     (field<unsigned>(a,0x1d0)==8||field<unsigned>(a,0x1d0)==9),"Walking away retained combat gait");
 combat_presence73={GetTickCount64(),possession_lease.generation+1,true};
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)<=7,"Another possession lease affected walking gait");
 combat_presence73={GetTickCount64(),possession_lease.generation,true};
 const auto ready_clip=clips[109];
 clips[109]=0;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)<=7,"Missing combat clip blocked ordinary walking");
 clips[109]=ready_clip;
 auto& ready_metadata=field<unsigned>(P::ptr(metadata),109*0x18+0x14);
 const auto saved_ready_metadata=ready_metadata;
 ready_metadata=0;
 tick203(reinterpret_cast<void*>(a));
 require(field<unsigned>(a,0x1f0)<=7,"Missing combat rig metadata blocked ordinary walking");
 ready_metadata=saved_ready_metadata;
 combat_presence73={};
 std::cout<<"PASS combat gait73: native close combat shuffle, unchanged blend, sprint, leaving range, lease and rig guards.\n";
 for(unsigned mode:{8u,9u,11u,29u,30u,44u}){field<unsigned>(a,0x1f0)=mode;field<unsigned>(a,0x1d0)=99;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Combat/mount pose forced");}
 field<unsigned>(a,0x1f0)=0;field<unsigned>(a,0x1d0)=99;state->aiming=1;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Aiming pose forced");state->aiming=0;
 state->release=1;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Released pose forced");state->release=0;
 field<uintptr_t>(a,0x474)=P::ptr(clip);tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Explicit native clip overwritten");field<uintptr_t>(a,0x474)=0;
 auto old_serial=field<unsigned>(a,0x1c8);++field<unsigned>(a,0x1c8);tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Reused actor pose forced");field<unsigned>(a,0x1c8)=old_serial;
 require(motion_callback_depth==0&&!motion_completed203,"Motion completion TLS/depth leaked");
 #include "verify_locomotion204.h"
 memcpy(reinterpret_cast<void*>(a),before203.data(),before203.size());
 std::cout<<"PASS locomotion203: original idle reproduction; "<<checked203<<" actual caller/native pose walk/run/stop cases, combat/aim/release/reuse guards. Integrator velocity and running capability are explicit providers.\n";
}
