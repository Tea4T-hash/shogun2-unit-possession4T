// Exercise the captured horse/attached-rider pair in the real motion completion
// and pose routines. Velocity integration and unit running permission are providers.
{
 auto rider206=f.actor,peer206=f.peer;state->aiming=state->aim_pose=state->weapon_phase=state->release=0;
 state->heartbeat=GetTickCount64();state->input=FORWARD;
 field<uintptr_t>(h,0x23c)=P::ptr(db);field<unsigned>(h,0x1f0)=0;field<unsigned>(h,0x1d0)=0;
 field<unsigned>(a,0x1f0)=29;field<uintptr_t>(a,0x474)=field<uintptr_t>(a,0x688)=0;
 field<unsigned char>(a,0x488)=field<unsigned char>(a,0x489)=0;
 rider206=f.actor;auto tick206=reinterpret_cast<UnitTick130>(base+0x366a10);
 fixture_vx203=3;fixture_vz203=0;fixture_mark203=false;
 tick206(reinterpret_cast<void*>(h));
 require(field<float>(h,0x1ac)==3&&field<unsigned>(h,0x1d0)==0,"Original moving horse/idle reproduction failed");
 fixture_mark203=true;tick206(reinterpret_cast<void*>(h));
 require(field<float>(h,0x1ac)==3&&field<unsigned>(h,0x1d0)==8,"Owned horse still moving in idle pose");
 unsigned directions206=0,idles206=0,guards206=0;
 for(unsigned rider_mode:{29u,30u})for(unsigned run:{0u,1u})for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz){
  state->input=FORWARD|(run?RUN:0);state->heartbeat=GetTickCount64();field<unsigned>(a,0x1f0)=rider_mode;
  rider206=f.actor;field<unsigned>(h,0x1d0)=33;field<unsigned>(h,0x1d8)=19;
  fixture_vx203=float(dx*3);fixture_vz203=float(dz*3);fixture_zero_intent204=true;
  tick206(reinterpret_cast<void*>(h));
  require(field<unsigned>(h,0x1d0)==((dx||dz)?(run?13u:8u):0u),"Horse walk/run/stop classifier differs");
  require(field<unsigned>(h,0x244)==run&&field<unsigned>(h,0x1d8)==19&&field<float>(h,0x150)==0,"Horse gait, clock or consumed intent differs");
  require(field<float>(h,0x8c)==fixture_vx203&&field<float>(h,0x94)==fixture_vz203,"Horse pose altered physical movement");
  require(f.actor==rider206&&f.peer==peer206,"Horse pose rewrote rider or peer");++directions206;
 }
 fixture_zero_intent204=false;state->input=FORWARD;field<unsigned>(a,0x1f0)=29;rider206=f.actor;
 for(unsigned id=141;id<=148;++id)for(bool moving:{false,true}){
  state->heartbeat=GetTickCount64();field<unsigned>(P::ptr(clip),0)=id;clips[id]=P::ptr(clip);
  field<unsigned>(h,0x1d4)=id;field<uintptr_t>(h,0x474)=P::ptr(clip);field<unsigned>(h,0x1d0)=33;
  field<unsigned>(h,0x1d8)=27;fixture_vx203=moving?3.f:0.f;fixture_vz203=0;tick206(reinterpret_cast<void*>(h));
  require(field<uintptr_t>(h,0x474)==(moving?0:P::ptr(clip)),"Owned horse idle interruption differs");
  require(field<unsigned>(h,0x1d0)==(moving?8u:33u)&&field<unsigned>(h,0x1d8)==27,"Horse idle pose/clock differs");++idles206;
 }
 field<uintptr_t>(h,0x474)=0;fixture_vx203=3;fixture_vz203=0;
 // Each forbidden case runs the actual completion hook but must leave pose/gait.
 auto refuse206=[&](){field<unsigned>(h,0x1d0)=99;field<unsigned>(h,0x244)=2;
  state->heartbeat=GetTickCount64();tick206(reinterpret_cast<void*>(h));
  require(field<unsigned>(h,0x1d0)==99&&field<unsigned>(h,0x244)==2,"Special/foreign horse pose overwritten");++guards206;};
 for(unsigned mode:{8u,9u,12u,13u,16u,29u,30u,44u}){field<unsigned>(h,0x1f0)=mode;refuse206();}field<unsigned>(h,0x1f0)=0;
 for(unsigned mode:{0u,8u,12u,44u}){field<unsigned>(a,0x1f0)=mode;refuse206();}field<unsigned>(a,0x1f0)=29;
 state->aiming=1;refuse206();state->aiming=0;state->aim_pose=1;refuse206();state->aim_pose=0;
 state->weapon_phase=1;refuse206();state->weapon_phase=0;state->release=1;refuse206();state->release=0;
 field<uintptr_t>(a,0x688)=1;refuse206();field<uintptr_t>(a,0x688)=0;
 field<uintptr_t>(h,0x688)=1;refuse206();field<uintptr_t>(h,0x688)=0;
 field<uintptr_t>(a,0x474)=P::ptr(clip);refuse206();field<uintptr_t>(a,0x474)=0;
 for(auto actor206:{a,h})for(unsigned offset206:{0x488u,0x489u}){field<unsigned char>(actor206,offset206)=1;refuse206();field<unsigned char>(actor206,offset206)=0;}
 field<unsigned>(h,0x248)=16;field<unsigned char>(h,0x2e8)=1;field<uintptr_t>(h,0x250)=base+0x15e0cec;field<uintptr_t>(h,0x254)=h;
 require(native_mounted_jump140(h),"Jump guard fixture not native jump");refuse206();
 field<unsigned>(h,0x248)=0;field<unsigned char>(h,0x2e8)=0;field<uintptr_t>(h,0x250)=field<uintptr_t>(h,0x254)=0;
 field<unsigned char>(h,0x2e8)=1;field<uintptr_t>(h,0x250)=base+0x15e0a5c;field<uintptr_t>(h,0x254)=h;
 require(charge_native_task146(h),"Charge guard fixture not native charge");refuse206();
 field<unsigned char>(h,0x2e8)=0;field<uintptr_t>(h,0x250)=field<uintptr_t>(h,0x254)=0;
 auto serial206=field<unsigned>(h,0x1c8);++field<unsigned>(h,0x1c8);refuse206();field<unsigned>(h,0x1c8)=serial206;
 auto rider_serial206=field<unsigned>(a,0x1c8);++field<unsigned>(a,0x1c8);refuse206();field<unsigned>(a,0x1c8)=rider_serial206;
 passenger=P::ptr(f.peer);refuse206();passenger=a;field<uintptr_t>(a,0x210)=0;refuse206();field<uintptr_t>(a,0x210)=h;
 for(unsigned id:{62u,93u,109u,124u,758u,1067u,1186u}){
  field<unsigned>(P::ptr(clip),0)=id;clips[id]=P::ptr(clip);field<unsigned>(h,0x1d4)=id;field<uintptr_t>(h,0x474)=P::ptr(clip);
  refuse206();require(field<uintptr_t>(h,0x474)==P::ptr(clip),"Horse non-idle clip canceled");
 }
 field<unsigned>(P::ptr(clip),0)=141;field<unsigned>(h,0x1d4)=141;clips[141]=0;refuse206();
 require(field<uintptr_t>(h,0x474)==P::ptr(clip),"Foreign horse idle canceled");field<uintptr_t>(h,0x474)=0;
 require(motion_callback_depth==0&&!motion_completed203&&f.actor==rider206&&f.peer==peer206,"Mounted correction leaked state or rewrote rider/peer");
 std::cout<<"PASS locomotion206: observed horse idle reproduction; "<<directions206<<" mounted directional gait cases; "<<idles206<<" native horse idle cases; "<<guards206<<" special/identity guards. Rider/peer/velocity/clocks preserved.\n";
}
