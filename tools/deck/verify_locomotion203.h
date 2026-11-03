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
 for(unsigned mode:{8u,9u,10u,11u,29u,30u,44u}){field<unsigned>(a,0x1f0)=mode;field<unsigned>(a,0x1d0)=99;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Combat/mount pose forced");}
 field<unsigned>(a,0x1f0)=0;field<unsigned>(a,0x1d0)=99;state->aiming=1;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Aiming pose forced");state->aiming=0;
 state->release=1;tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Released pose forced");state->release=0;
 field<uintptr_t>(a,0x474)=P::ptr(clip);tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Explicit native clip overwritten");field<uintptr_t>(a,0x474)=0;
 auto old_serial=field<unsigned>(a,0x1c8);++field<unsigned>(a,0x1c8);tick203(reinterpret_cast<void*>(a));require(field<unsigned>(a,0x1d0)==99,"Reused actor pose forced");field<unsigned>(a,0x1c8)=old_serial;
 require(motion_callback_depth==0&&!motion_completed203,"Motion completion TLS/depth leaked");
 #include "verify_locomotion204.h"
 memcpy(reinterpret_cast<void*>(a),before203.data(),before203.size());
 std::cout<<"PASS locomotion203: original idle reproduction; "<<checked203<<" actual caller/native pose walk/run/stop cases, combat/aim/release/reuse guards. Integrator velocity and running capability are explicit providers.\n";
}
