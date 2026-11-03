// Exercise original interruption and original pose classifiers in mapped engine.
{
 unsigned idle_cases204=0;auto peer204=f.peer;auto savedclip204=clip;auto savedclips204=clips;
 auto expect204=[&](unsigned id,bool moving,bool cancel){
  state->input=FORWARD;state->heartbeat=GetTickCount64();field<unsigned>(a,0x1f0)=4;
  field<unsigned>(P::ptr(clip),0)=id;field<unsigned>(a,0x1d4)=id;
  clips[id]=P::ptr(clip);field<uintptr_t>(a,0x474)=P::ptr(clip);field<unsigned>(a,0x1d0)=33;
  field<unsigned>(a,0x1d8)=27;fixture_vx203=moving?2.f:0.f;fixture_vz203=0;
  tick203(reinterpret_cast<void*>(a));
  require((field<uintptr_t>(a,0x474)==0)==cancel,"Idle interruption ownership differs");
  require(field<unsigned>(a,0x1d0)==(cancel?9u:33u),"Idle/movement pose differs");
  require(field<unsigned>(a,0x1d8)==27,"Idle interruption rewound animation clock");
  require(field<float>(a,0x8c)==fixture_vx203&&field<float>(a,0x94)==0,"Pose fix changed movement");
 };
 for(unsigned id=0;id<0x4a3;++id)if(locomotion_idle204(id)){
  expect204(id,false,false);expect204(id,true,true);++idle_cases204;
 }
 // Ranged, readiness, impact, climb and unknown animations are not standing idles.
 for(unsigned id:{62u,93u,109u,124u,758u,1067u,1186u})expect204(id,true,false);
 // Missing/mismatched animation ownership must not authorize a cancellation.
 field<unsigned>(a,0x1f0)=4;field<unsigned>(P::ptr(clip),0)=156;clips[156]=0;
 field<unsigned>(a,0x1d4)=156;field<uintptr_t>(a,0x474)=P::ptr(clip);field<unsigned>(a,0x1d0)=33;
 tick203(reinterpret_cast<void*>(a));require(field<uintptr_t>(a,0x474)==P::ptr(clip),"Foreign idle clip canceled");
 clips[156]=P::ptr(clip);state->weapon_phase=1;tick203(reinterpret_cast<void*>(a));
 require(field<uintptr_t>(a,0x474)==P::ptr(clip),"Weapon phase interrupted");state->weapon_phase=0;
 field<uintptr_t>(a,0x474)=0;fixture_zero_intent204=true;
 for(unsigned mode:{0u,4u})for(unsigned run:{0u,1u}){
  state->input=FORWARD|(run?RUN:0);field<unsigned>(a,0x1f0)=mode;
  fixture_vx203=2;tick203(reinterpret_cast<void*>(a));
  require(field<float>(a,0x150)==0&&field<float>(a,0x1ac)==2,"Zero-intent moving reproduction failed");
  require(field<unsigned>(a,0x1d0)==(mode==4?(run?14u:9u):(run?13u:8u)),"Consumed intent incorrectly forced idle");
  fixture_vx203=0;tick203(reinterpret_cast<void*>(a));
  require(field<unsigned>(a,0x1d0)==(mode==4?1u:0u),"Stopped actor kept moving pose");
 }
 fixture_zero_intent204=false;require(peer204==f.peer,"Non-possessed soldier changed");
 memcpy(clip.data(),savedclip204.data(),clip.size());clips=savedclips204;
 std::cout<<"PASS locomotion204: "<<idle_cases204<<" native idle categories interrupted only during movement; stationary, combat/weapon, identity guards; consumed-intent moving/stop cases.\n";
}
