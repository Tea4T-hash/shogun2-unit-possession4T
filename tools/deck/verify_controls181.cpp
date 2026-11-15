#include "../../native/deck/controls181.h"
#include "../../native/deck/channel181.h"
#include <cassert>
#include <iostream>
using namespace deck181;
using deck174::bit;
static unsigned passed;
#define TEST(label,...) do{__VA_ARGS__;++passed;std::cout<<"PASS " label "\n";}while(0)
static Frame frame(uint64_t n,uint64_t us){Frame f;f.sequence=n;f.epoch=1;f.sample_us=us;f.source=deck174::Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.active_buttons=deck174::all_buttons;return f;}
int main(){
 deck174::Config c;
 TEST("both battle buttons share one held formations and range input",{
  Controls p;auto step=[&](uint64_t n,uint64_t buttons,Context context=Context::RTS){
   auto f=frame(n,1000000+n*8000);f.buttons=buttons;return p.sample(f,context,f.sample_us);
  };
  assert(!step(1,0).held(Space));
  assert(step(2,bit(Button::R3)).held(Space));
  assert(step(3,bit(Button::R3)|bit(Button::RightPadClick)).held(Space));
  assert(step(4,bit(Button::RightPadClick)).held(Space));
  assert(!step(5,0).held(Space));
  assert(!step(6,0,Context::Possession).held(Space));
  assert(!step(7,bit(Button::R3),Context::Possession).held(Space));
 });
 TEST("R3 double tap cannot return RTS",{R3Gesture g;g.sample(true,1000000,Context::Possession,c);assert(g.sample(false,1040000,Context::Possession,c)==Gesture::None);g.sample(true,1100000,Context::Possession,c);assert(g.sample(false,1140000,Context::Possession,c)==Gesture::None);assert(g.sample(false,1500000,Context::Possession,c)==Gesture::None);});
 TEST("long R3 toggles once without tap",{R3Gesture g;g.sample(true,1000000,Context::Possession,c);assert(g.sample(true,1349999,Context::Possession,c)==Gesture::None);assert(g.sample(true,1350000,Context::Possession,c)==Gesture::Perspective);assert(g.sample(true,1800000,Context::Possession,c)==Gesture::None);assert(g.sample(false,1900000,Context::Possession,c)==Gesture::None);assert(g.sample(false,2300000,Context::Possession,c)==Gesture::None);});
 TEST("single in possession does not eject",{R3Gesture g;g.sample(true,1000000,Context::Possession,c);g.sample(false,1050000,Context::Possession,c);assert(g.sample(false,1500000,Context::Possession,c)==Gesture::None);});
 TEST("Y is the only possess and return binding",{for(auto context:{Context::RTS,Context::Possession}){Controls p;auto f=frame(1,1000000);p.sample(f,context,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::Y);auto o=p.sample(f,context,f.sample_us);assert(o.held(context==Context::RTS?F9:F10)&&!o.held(context==Context::RTS?F10:F9));}});
 TEST("long RTS press does not enter or change camera",{R3Gesture g;g.sample(true,1000000,Context::RTS,c);assert(g.sample(true,1400000,Context::RTS,c)==Gesture::None);g.sample(false,1500000,Context::RTS,c);assert(g.sample(false,2000000,Context::RTS,c)==Gesture::None);});
 TEST("possession sticks never emit RTS orders",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.left_stick={.5f,1};f.right_stick={1,.5f};auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.held(W)&&o.held(D)&&!o.held(Q)&&o.look.x>0&&o.look.y<0);});
 TEST("RTS sticks are not soldier input",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::RTS,f.sample_us);f=frame(2,1008000);f.left_stick={1,1};f.right_trigger=1;auto o=p.sample(f,Context::RTS,f.sample_us);for(auto k:o.keys)assert(k==0);assert(o.look.x==0&&o.look.y==0);});
 TEST("campaign cannot acquire soldier",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Campaign,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::R3);p.sample(f,Context::Campaign,f.sample_us);f=frame(3,1020000);p.sample(f,Context::Campaign,f.sample_us);f=frame(4,1500000);auto o=p.sample(f,Context::Campaign,f.sample_us);assert(!o.held(F9));});
 TEST("held trigger cannot cross into possession",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::RTS,f.sample_us);f=frame(2,1008000);f.right_trigger=1;p.sample(f,Context::RTS,f.sample_us);f=frame(3,1016000);f.right_trigger=1;assert(!p.sample(f,Context::Possession,f.sample_us).held(Primary));f=frame(4,1024000);p.sample(f,Context::Possession,f.sample_us);f=frame(5,1032000);f.right_trigger=1;assert(p.sample(f,Context::Possession,f.sample_us).held(Primary));});
 TEST("trigger release hysteresis",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.right_trigger=1;assert(p.sample(f,Context::Possession,f.sample_us).held(Primary));f=frame(3,1016000);f.right_trigger=.2f;assert(p.sample(f,Context::Possession,f.sample_us).held(Primary));f=frame(4,1024000);assert(!p.sample(f,Context::Possession,f.sample_us).held(Primary));});
 TEST("L3 toggles sprint, resets on context/device change, and can override Follow Me",{
  Controls p;unsigned seq=0;uint64_t epoch=1;
  auto step=[&](uint64_t buttons=0,Context context=Context::Possession,bool follow=false){
   ++seq;auto f=frame(seq,1000000+seq*8000);f.buttons=buttons;f.epoch=epoch;
   return p.sample(f,context,f.sample_us,0,0,follow);
  };
  const auto l3=bit(Button::L3);
  assert(!step().held(Shift));
  assert(step(l3).held(Shift)&&step(l3).held(Shift)&&step().held(Shift));
  assert(!step(l3).held(Shift)&&!step().held(Shift));
  assert(step(l3).held(Shift)&&step().held(Shift));
  assert(!step(0,Context::RTS).held(Shift));
  assert(step(l3,Context::RTS).held(Control));
  assert(!step(l3).held(Shift)&&!step().held(Shift)); // held across entry
  assert(step(l3).held(Shift));++epoch;
  assert(!step(l3).held(Shift)&&!step().held(Shift));
  assert(!step(0,Context::Possession,true).held(deck183::WalkPace));
  assert(step(l3,Context::Possession,true).held(deck183::WalkPace));
  assert(step(0,Context::Possession,true).held(deck183::WalkPace));
  auto sprint=step(l3,Context::Possession,true);
  assert(sprint.held(Shift)&&!sprint.held(deck183::WalkPace));
 });
 TEST("aim temporarily overrides the Deck sprint toggle and Follow Me pace",{
  for(bool follow:{false,true}){
   Controls p;unsigned n=0;
   auto step=[&](float lt=0,uint64_t buttons=0){
    ++n;auto f=frame(n,1000000+n*8000);
    f.left_trigger=lt;f.buttons=buttons;f.left_stick.y=n>1?1.f:0.f;
    return p.sample(f,Context::Possession,f.sample_us,0,0,follow);
   };
   const auto l3=bit(Button::L3);
   assert(step().accepted);
   if(!follow){assert(step(0,l3).held(Shift));assert(step().held(Shift));}
   auto aim=step(1);
   assert(aim.held(Aim)&&aim.held(W)&&!aim.held(Shift));
   assert(aim.held(deck183::WalkPace)==follow);
   auto held=step(.2f);
   assert(held.held(Aim)&&!held.held(Shift));
   auto resumed=step();
   assert(!resumed.held(Aim)&&resumed.held(Shift)==!follow&&!resumed.held(deck183::WalkPace));
   // An L3 click during aim can change the stored pace without sending sprint.
   auto toggle=step(1,l3);
   assert(toggle.held(Aim)&&!toggle.held(Shift));
   auto stopped=step();
   assert(!stopped.held(Aim)&&!stopped.held(Shift)&&stopped.held(deck183::WalkPace)==follow);
  }
 });
 TEST("Dpad owns abilities and distance outside wheel",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::Up)|bit(Button::Left);auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.held(T)&&o.zoom>0&&!o.held(Q)&&!o.held(Z));});
 TEST("native ability melee crouch binds",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::Up)|bit(Button::Down)|bit(Button::X)|bit(Button::B);auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.held(T)&&o.held(G)&&o.held(One)&&o.held(Control));});
 TEST("orbit does not become RTS turn",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::L4);assert(p.sample(f,Context::Possession,f.sample_us).held(Middle));});
 TEST("touch onset does not jump camera",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::RightPadTouch);f.right_pad={.8f,.8f};auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.look.x==0&&o.look.y==0);});
 TEST("motion data does not turn the camera and trackpad swipes still work",{
  Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);
  for(unsigned i=2;i<1000;++i){
   f=frame(i,1000000+i*8000);f.buttons=bit(Button::RightPadTouch);f.right_pad={.3f,.4f};
   f.capabilities|=deck174::Motion;f.gyro={.9f,0,1.2f};
   auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.look.x==0&&o.look.y==0);
  }
  f=frame(1000,9000000);f.buttons=bit(Button::RightPadTouch);f.right_pad={.5f,.2f};
  auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.look.x>0&&o.look.y>0);
 });
 TEST("focus loss clears held fire and gestures",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.right_trigger=1;p.sample(f,Context::Possession,f.sample_us);f=frame(3,1016000);f.focused=false;assert(!p.sample(f,Context::Possession,f.sample_us).held(Primary));});
 TEST("stale sample clears input",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.right_trigger=1;assert(!p.sample(f,Context::Possession,1300000).accepted);});
 TEST("new device epoch cannot inherit a held trigger",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.epoch=2;f.right_trigger=1;assert(!p.sample(f,Context::Possession,f.sample_us).held(Primary));});
 TEST("duplicate sample never repeats gesture",{Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);assert(!p.sample(f,Context::Possession,f.sample_us).accepted);});
 TEST("stick look rate stays stable at different sample rates",{for(unsigned hz:{30u,60u,125u,250u}){Controls p;auto f=frame(1,1000000);p.sample(f,Context::Possession,f.sample_us);double sum=0;for(unsigned i=1;i<=hz;++i){f=frame(i+1,1000000+uint64_t(i)*1000000/hz);f.right_stick.x=1;sum+=p.sample(f,Context::Possession,f.sample_us).look.x;}assert(std::fabs(sum-1000)<.05);}});
 TEST("in-memory channel rejects stale data",{Sample s;s.focused=1;s.context=unsigned(Context::Possession);s.tick=1000;assert(fresh(s,1100)&&!fresh(s,1101)&&!fresh(s,999));});
 TEST("wrong channel shape and PID refused",{Shared s;s.magic=channel_magic;s.version=181;s.bytes=sizeof s;s.pid=1;assert(valid(s,1)&&!valid(s,2));s.bytes++;assert(!valid(s,1));});
 TEST("queue overflow drops pending input instead of replaying fire",{Mailbox m;Output o;o.set(Primary);for(unsigned i=1;i<=Mailbox::capacity;++i)assert(m.put(frame(i,1000000+i),o,1,2,3));assert(!m.put(frame(33,1000033),o,1,2,3));std::array<Queued,Mailbox::capacity> values;bool reset=false;assert(m.take(values,reset)==0&&reset);});
 TEST("controller pause is not the Escape-to-RTS polling path",{Sample s;s.context=unsigned(Context::Possession);s.lease=1;s.keys[deck181::Escape/32]|=uint32_t(1)<<(deck181::Escape%32);s.keys[W/32]|=uint32_t(1)<<(W%32);assert(!controller_held(s,deck181::Escape)&&controller_held(s,W));});
 TEST("native pause and hide pulses preserve unrelated keys",{unsigned char keys[256]{};keys[5]=0x80;InterlockedExchange(&native_key_pending,3);native_keys(keys,false);assert(native_key_pending==3&&!keys[1]);native_keys(keys,true);assert(!native_key_pending&&keys[1]==0x80&&keys[0x25]==0x80&&keys[5]==0x80);});
 TEST("L3 run and R4 slow motion are disjoint",{Controls control(c);auto f=frame(1,1000000);control.sample(f,Context::Possession,f.sample_us);f=frame(2,1008000);f.buttons=bit(Button::R4);auto out=control.sample(f,Context::Possession,f.sample_us);assert(out.toggle_slow214&&!out.held(E)&&!out.held(Shift));f=frame(3,1016000);out=control.sample(f,Context::Possession,f.sample_us);assert(!out.toggle_slow214&&!out.held(E)&&!out.held(Shift));});
 TEST("B toggles crouch on new presses and wheel cancel and focus loss are safe",{
  Controls p;auto step=[&](uint64_t n,uint64_t buttons,bool focused=true,Context context=Context::Possession){
   auto f=frame(n,1000000+n*8000);f.buttons=buttons;f.focused=focused;
   if(buttons&bit(Button::R1))f.right_stick={1,0};
   return p.sample(f,context,f.sample_us);
  };
  const auto b=bit(Button::B);
  assert(!step(1,0).held(Control));
  auto o=step(2,b);assert(o.held(Control)&&!o.held(M)&&!o.held(deck181::Escape)&&!o.held(F10));
  assert(step(3,b).held(Control)&&step(4,0).held(Control));
  assert(!step(5,bit(Button::R1)).held(Control)); // Wheel owns the frame.
  o=step(6,b);assert(o.held(Control)&&!o.held(R)); // B and wheel release on the same sample.
  assert(step(7,0).held(Control)&&!step(8,b).held(Control)&&!step(9,0).held(Control));
  assert(step(10,b).held(Control)&&!step(11,b,false).held(Control));
  assert(!step(12,b).held(Control)&&!step(13,0).held(Control));
  assert(step(14,b).held(Control));
  assert(!step(15,0,true,Context::RTS).held(Control)&&!step(16,0).held(Control));
 });
 TEST("LT aim rearms independently after every release and RT shot",{Controls p;uint64_t n=1;auto f=frame(n,1000000);p.sample(f,Context::Possession,f.sample_us);for(unsigned shot=0;shot<100;++shot){f=frame(++n,1000000+n*8000);f.left_trigger=1;auto o=p.sample(f,Context::Possession,f.sample_us);assert(o.held(Aim)&&!o.held(Primary));f=frame(++n,1000000+n*8000);f.left_trigger=f.right_trigger=1;o=p.sample(f,Context::Possession,f.sample_us);assert(o.held(Aim)&&o.held(Primary));f=frame(++n,1000000+n*8000);o=p.sample(f,Context::Possession,f.sample_us);assert(!o.held(Aim)&&!o.held(Primary));}});
 std::cout<<"PASS "<<passed<<" controls181 context/R3/lifecycle checks; no game or Deck execution\n";
}
