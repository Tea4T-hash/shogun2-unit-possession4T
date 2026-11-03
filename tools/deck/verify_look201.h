// Included inside the actual adapted adapter dispatch fixture.
{
 auto savedMount201=possession_lease.identity.mount;auto saved201=*state;auto third201=third_person152;auto lock201=combat_entry_locked126;
 auto animation201=animation_look_active104;auto mounted201=mount_camera_locked154;
 state->mode=POSSESSED;third_person152=combat_entry_locked126=animation_look_active104=mount_camera_locked154=0;
 possession_lease.identity.mount=0;
 unsigned advances201=0;
 for(float blend201:{0.f,.5f,1.f})for(float step201:{.03125f,.125f,.25f,.75f}){
  state->yaw=state->pitch=0;state->aim_blend=blend201;
  deck175::controller181::look201.clear();
  for(unsigned i201=0;i201<120;++i201){
   deck175::controller181::look201.add(step201,-step201);
   auto d201=deck175::controller181::look201.take();
   float before201=state->yaw,beforePitch201=state->pitch;
   mouse_look101(d201.x,d201.y,0,1.f/60.f);
   const float expected201=step201*.0025f*(1.f-blend201*.55f);
   assert(state->yaw>before201&&state->pitch>beforePitch201);
   assert(std::fabs((state->yaw-before201)-expected201)<.000001f);
   assert(std::fabs((state->pitch-beforePitch201)-expected201)<.000001f);++advances201;
  }
 }
 // The real cancellation paths must not leave a delayed look step.
 deck175::controller181::look201.add(.5f,-.5f);deck175::controller181::clear_input();
 auto empty201=deck175::controller181::look201.take();assert(!empty201.x&&!empty201.y);
 deck175::controller181::look201.add(.5f,-.5f);deck175::controller181::cancel_window();
 empty201=deck175::controller181::look201.take();assert(!empty201.x&&!empty201.y);
 // Two axes arrive as one sample; invalid input discards accumulated motion.
 deck201::LookQueue q201;q201.add(.25f,.5f);q201.add(NAN,0);
 empty201=q201.take();assert(!empty201.x&&!empty201.y);
 for(unsigned n201=0;n201<10000;++n201)q201.add(500,500);
 auto bounded201=q201.take();assert(bounded201.x==12800&&bounded201.y==12800);
 std::atomic<bool> done201{false};double totalX201=0,totalY201=0;
 std::thread producer201([&](){for(unsigned n201=0;n201<10000;++n201)q201.add(.03125f,-.0625f);done201=true;});
 while(!done201){auto d201=q201.take();totalX201+=d201.x;totalY201+=d201.y;}
 producer201.join();auto tail201=q201.take();totalX201+=tail201.x;totalY201+=tail201.y;
 assert(totalX201==312.5&&totalY201==-625.);
 possession_lease.identity.mount=savedMount201;*state=saved201;third_person152=third201;combat_entry_locked126=lock201;animation_look_active104=animation201;mount_camera_locked154=mounted201;
 std::cout<<"PASS aim201: "<<advances201<<" actual camera fractional yaw/pitch advances across hip/partial/full ADS; cancellation, bounds and10000 concurrent samples\n";
}
