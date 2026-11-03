// Runs inside the adapted actual adapter fixture; no live game/input actions.
{
 auto saved202=*state;auto third202=third_person152;auto lock202=combat_entry_locked126;
 auto anim202=animation_look_active104;auto mountLock202=mount_camera_locked154;auto mount202=possession_lease.identity.mount;
 state->mode=POSSESSED;combat_entry_locked126=animation_look_active104=mount_camera_locked154=0;possession_lease.identity.mount=0;
 deck202::Key key202{42,0,12};unsigned advances202=0;
 for(bool third:{false,true})for(float blend:{0.f,.5f,1.f})for(unsigned hz:{30u,40u,60u,90u}){
  deck202::LookRate rate;third_person152=third;state->yaw=state->pitch=0;state->aim_blend=blend;
  uint64_t poll=1000000;float previousYaw=0,previousPitch=0;
  for(unsigned f=0;f<180;++f){
   uint64_t now=1000000+uint64_t(f)*1000000/hz;
   while(poll<=now){rate.publish(key202,1,poll,{100,-50},100000);poll+=8000;}
   auto d=rate.advance(key202,now);mouse_look101(d.x,d.y,0,0);
   if(f){
    float dt=float(now-(1000000+uint64_t(f-1)*1000000/hz))*1e-6f;
    assert(std::fabs((state->yaw-previousYaw)-100*dt*.0025f*(1-blend*.55f))<.000002f);
    assert(std::fabs((state->pitch-previousPitch)-50*dt*.0025f*(1-blend*.55f))<.000002f);++advances202;
   }
   auto again=rate.advance(key202,now);assert(!again.x&&!again.y);
   previousYaw=state->yaw;previousPitch=state->pitch;
  }
 }
 deck202::LookRate rate;auto zero=[](deck202::Motion m){return m.x==0&&m.y==0;};
 rate.publish(key202,1,1000000,{100,-50},100000);assert(zero(rate.advance(key202,1000000)));
 assert(rate.advance(key202,1010000).x==1.f);
 rate.publish(key202,1,1015000,{0,0},100000);assert(zero(rate.advance(key202,1020000))); // no tail
 rate.publish(key202,1,1030000,{-100,50},100000);assert(rate.advance(key202,1030000).x<0); // reversal
 assert(zero(rate.advance(key202,1200000))); // stale must not extrapolate
 rate.publish(key202,1,1200000,{100,-50},100000);assert(zero(rate.advance(key202,1200000))); // no backlog
 assert(zero(rate.advance(key202,1210000,false)));assert(zero(rate.advance(key202,1211000))); // focus/menu gating
 auto other=key202;++other.lease;assert(zero(rate.advance(other,1212000)));
 rate.publish(other,2,1213000,{100,-50},100000);assert(zero(rate.advance(other,1213000)));
 assert(rate.advance(other,1270000).x==5.f); //50ms ceiling
 rate.publish(other,2,1271000,{NAN,1},100000);assert(zero(rate.advance(other,1272000)));
 // Both cancellation paths clear the latest analog rate as well as pad deltas.
 for(bool clear:{false,true}){
  auto& q=deck175::controller181::look202;q.publish(key202,1,1000000,{100,-50},100000);q.advance(key202,1000000);
  if(clear)deck175::controller181::clear_input();else deck175::controller181::cancel_window();assert(zero(q.advance(key202,1010000)));
 }
 // The actual Controls path separates analog rate from pad displacements and
 // emits zero rate while help/wheel/menu suppresses looking.
 deck181::Controls controls;deck174::Frame frame{};frame.source=deck174::Source::Simulation;frame.connected=frame.focused=frame.native_bindings=true;frame.active_buttons=deck174::all_buttons;frame.epoch=1;frame.right_stick={.5f,.25f};
 for(unsigned i=0;i<3;++i){frame.sequence=i+1;frame.sample_us=2000000+i*8000;auto out=controls.sample(frame,deck174::Context::Possession,frame.sample_us);assert(out.accepted&&out.stick_rate.x>0&&out.stick_rate.y<0);assert(out.look.x==out.stick_delta.x&&out.look.y==out.stick_delta.y);}
 frame.buttons=deck174::bit(deck174::Button::R1);++frame.sequence;frame.sample_us+=8000;auto wheel=controls.sample(frame,deck174::Context::Possession,frame.sample_us);assert(!wheel.stick_rate.x&&!wheel.stick_delta.x&&!wheel.look.x);
 frame.buttons=0;++frame.sequence;frame.sample_us+=8000;auto rts=controls.sample(frame,deck174::Context::RTS,frame.sample_us);assert(!rts.stick_rate.x&&!rts.look.x);
 // Concurrent rate publication keeps the axes paired. The newest rate replaces
 // older samples rather than accumulating an unbounded movement backlog.
 std::atomic<bool> done{false};deck202::LookRate concurrent;unsigned received=0;
 std::thread producer([&]{for(unsigned n=0;n<20000;++n){float v=float(n%100);concurrent.publish(key202,1,1000000,{v,-2*v},100000);}done=true;});
 uint64_t now=1000000;while(!done){auto d=concurrent.advance(key202,now++);assert(d.y==-2*d.x);++received;}producer.join();
 *state=saved202;third_person152=third202;combat_entry_locked126=lock202;animation_look_active104=anim202;mount_camera_locked154=mountLock202;possession_lease.identity.mount=mount202;
 std::cout<<"PASS look202: "<<advances202<<" actual first/third-person camera frame advances at30/40/60/90Hz and3 aim blends; zero serial, stop/reverse/stale/focus/lease/wheel/cancel and20000 concurrent rate samples\n";
}
