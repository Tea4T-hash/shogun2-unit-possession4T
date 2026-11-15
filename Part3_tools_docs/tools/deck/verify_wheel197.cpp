#include "../../native/deck/controls181.h"
#include <cassert>
#include <iostream>
using namespace deck181;using deck174::bit;
struct Session {
 Controls controls;Frame f;uint64_t now=1000000;unsigned count=0;
 Session(){f.source=deck174::Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.active_buttons=deck174::all_buttons;f.epoch=1;step();}
 Output step(uint64_t buttons=0,Vec2 stick={},Context mode=Context::Possession,unsigned generation=0,unsigned targets=0){
  f.buttons=buttons;f.right_stick=stick;f.sequence=++count;f.sample_us=now+=8000;
  return controls.sample(f,mode,now,generation,targets);
 }
};
int main(){unsigned cases=0;const Vec2 sticks[]={{0,1},{0,-1},{-1,0},{1,0}};
 const auto rb_open=bit(Button::R1);
 for(unsigned i=0;i<4;++i){
  Session s;assert(s.step(rb_open,sticks[i]).held(deck183::WheelHeld));
  assert(s.step().wheel_orders[0].sector<0); // Closing without confirmation.
  s.step(rb_open,sticks[i]);s.f.right_trigger=1;
  auto first=s.step(rb_open);
  assert(!first.held(Primary)&&first.held(deck183::WheelHeld));
  assert(first.wheel_orders[0].sector==(i==2?-1:int(i)));
  for(unsigned n=0;n<10;++n)assert(s.step(rb_open).wheel_orders[0].sector<0);
  s.f.right_trigger=0;s.step(rb_open);
  if(i==2){
   s.f.right_trigger=1;auto twice=s.step(rb_open);
   assert(twice.wheel_orders[0].sector==int(i)&&twice.wheel_orders[0].run&&!twice.held(Primary));
   assert(!s.step().held(Primary));
  }
  ++cases;
 }
 {const unsigned i=2;
  Session s;s.step(rb_open,sticks[i]);s.f.right_trigger=1;
  assert(s.step(rb_open).wheel_orders[0].sector<0);
  s.f.right_trigger=0;s.step(rb_open);
  s.now+=260000;auto once=s.step(rb_open);
  assert(once.wheel_orders[0].sector==int(i)&&!once.wheel_orders[0].run&&once.held(deck183::WheelHeld));
  assert(s.step().wheel_orders[0].sector<0);++cases;
 }
 {Session s;s.step(rb_open,{0,1});s.f.right_trigger=1;
  auto once=s.step(rb_open);
  assert(once.wheel_orders[0].sector==0&&!once.wheel_orders[0].run&&!once.held(Primary));
  s.f.right_trigger=0;s.step(rb_open);s.f.right_trigger=1;
  assert(s.step(rb_open).wheel_orders[0].sector<0);
  assert(s.step().wheel_orders[0].sector<0);
  s.f.right_trigger=0;s.step();s.f.right_trigger=1;assert(s.step().held(Primary));++cases;}
 {Session s;s.step(rb_open|bit(Button::Up));s.f.right_trigger=1;
  auto o=s.step(rb_open|bit(Button::Up));assert(o.wheel_orders[0].sector<0&&!o.held(T)&&!o.held(Primary));++cases;}
 for(auto button:{Button::B,Button::Menu,Button::Y}){
  Session s;s.step(rb_open,{-1,0});s.f.right_trigger=1;s.step(rb_open);
  auto o=s.step(rb_open|bit(button));assert(o.wheel_orders[0].sector<0&&!o.held(Primary));
  assert(s.step().wheel_orders[0].sector<0&&!s.step().held(Primary));++cases;
 }
 {Session s;s.step(rb_open,{0,1});s.f.right_trigger=1;s.step(rb_open);
  s.f.focused=false;s.step();s.f.focused=true;assert(s.step().wheel_orders[0].sector<0);++cases;}
 {Session s;s.step(rb_open,{0,1});s.f.right_trigger=1;s.step(rb_open);++s.f.epoch;
  assert(s.step().wheel_orders[0].sector<0);++cases;}
 using namespace commander76;
 for(unsigned mask=0;mask<=valid_targets;++mask){
  const WheelLayout layout(mask);
  unsigned expected=1;
  for(unsigned i=0;i<=group_count;++i)if(mask&(1u<<i))++expected;
  assert(layout.size==expected&&layout.sectors[0]==army_sector);
  assert(layout.point(0,0,.45f)==-1&&layout.point(0,1,.45f)==army_sector);
  for(unsigned i=0;i<layout.size;++i){
   const auto angle=layout.angle(i)*.01745329252f;
   assert(layout.point(std::cos(angle),std::sin(angle),.45f)==layout.sectors[i]);
  }
 }
 const auto l3=bit(Button::L3),a=bit(Button::A),b=bit(Button::B),rb=bit(Button::R1);
 const unsigned targets=unit_bit|1|8|512;
 const auto direction=[](unsigned mask,int sector){
  const WheelLayout layout(mask);
  for(unsigned i=0;i<layout.size;++i)if(layout.sectors[i]==sector){
   const auto angle=layout.angle(i)*.01745329252f;
   return Vec2{std::cos(angle),-std::sin(angle)};
  }
  assert(false);return Vec2{};
 };
 auto rts=[&](Session& s,uint64_t buttons=0,unsigned generation=7,Vec2 stick={}){
  return s.step(buttons,stick,Context::RTS,generation,targets&~unit_bit);
 };
 auto open=[&](Session& s){
  rts(s);
  assert(rts(s,l3).held(Control));
  const auto o=rts(s);
  assert(o.commander_rts_generation==7&&o.held(deck183::WheelHeld)&&o.held(deck183::CommanderPage));
 };
 {
  Session s;open(s);
  for(int sector:{army_sector,first_group,first_group+3,first_group+9}){
   auto o=rts(s,0,7,direction(targets&~unit_bit,sector));
   assert(o.commander_sector==sector&&!o.commander_confirm);
   s.f.right_trigger=1;
   o=rts(s);
   assert(o.commander_confirm&&o.commander_sector==sector&&o.held(deck183::WheelHeld));
   assert(!rts(s).commander_confirm);
   s.f.right_trigger=0;assert(!rts(s).commander_confirm);
  }
 }
 {
  Session s;open(s);s.f.right_trigger=1;
  auto o=rts(s,b);assert(!o.commander_confirm&&!o.held(deck183::WheelHeld)&&o.commander_rts_generation==7&&!o.shortcuts);
  assert(rts(s).commander_rts_generation==7);
  s.f.right_trigger=0;assert(!rts(s).commander_rts_generation);
  assert(rts(s,b).shortcuts&(1u<<deck183::Deselect));
 }
 {
  Session s;rts(s);assert(rts(s,l3).held(Control));
  s.f.right_trigger=1;assert(rts(s,l3).held(Control));
  s.f.right_trigger=0;rts(s,l3);assert(!rts(s).commander_rts_generation);
  assert(!rts(s,rb|a).commander_rts_generation);
 }
 {
  Session s;rts(s);assert(rts(s,l3).held(Control));
  s.now+=400000;assert(rts(s,l3).held(Control));
  assert(!rts(s).held(deck183::WheelHeld));
 }
 {
  Session s;rts(s,0,0);assert(rts(s,l3,0).held(Control));
  assert(!rts(s,0,0).commander_rts_generation);
 }
 {
  Session s;open(s);s.f.right_trigger=1;
  auto o=rts(s,0,8);assert(!o.commander_confirm&&!o.held(deck183::WheelHeld)&&o.commander_rts_generation==7);
  assert(rts(s,0,8).commander_rts_generation==7);
  s.f.right_trigger=0;assert(!rts(s,0,8).commander_rts_generation);
 }
 {
  Session s;open(s);s.f.focused=false;assert(!rts(s).accepted);
  s.f.focused=true;s.f.right_trigger=1;assert(!rts(s).commander_confirm);
  assert(!rts(s).held(deck183::WheelHeld));
 }
 {
  Session s;open(s);rts(s,rb);rts(s);
  auto o=s.step(l3,{},Context::Possession);assert(!o.commander_confirm);
  s.step();assert(s.step(l3).held(Shift));
  auto poss=[&](uint64_t buttons,Vec2 stick={}){return s.step(buttons,stick,Context::Possession,0,targets);};
  o=poss(rb|a,direction(targets,first_group));assert(o.held(deck183::CommanderPage)&&!o.held(F));
  s.f.right_trigger=1;o=poss(rb);
  assert(o.commander_confirm&&o.commander_sector==first_group&&!o.held(Primary));
  assert(!poss(rb).commander_confirm&&!poss(rb).held(Primary));
  s.f.right_trigger=0;poss(rb,direction(targets,unit_sector));
  s.f.right_trigger=1;assert(poss(rb).commander_sector==unit_sector);
  o=poss(0);assert(!o.commander_confirm&&!o.held(Primary));
  assert(!poss(0).held(Primary));s.f.right_trigger=0;poss(0);
  s.f.right_trigger=1;assert(poss(0).held(Primary));
 }
 {
  Session s;s.f.right_trigger=1;
  auto o=s.step(rb|a,direction(targets,unit_sector),Context::Possession,0,targets);
  assert(o.held(deck183::CommanderPage)&&!o.commander_confirm&&!o.held(Primary));
  o=s.step(rb|b,{},Context::Possession,0,targets);assert(!o.held(deck183::WheelHeld)&&!o.held(Primary));
 }
 std::cout<<"PASS Deck Commander84: 9 input/lifecycle cases and all 2048 wheel layouts\n";
 {Session s;
  s.f.left_stick={-1,1};
  auto orders=s.step(rb_open,{.6f,.8f});
  assert(orders.held(W)&&orders.held(A)&&!orders.held(Primary)&&!orders.held(Aim));
  assert(std::fabs(orders.wheel_pointer.x-.6f)<.00001f&&std::fabs(orders.wheel_pointer.y+.8f)<.00001f);
  auto ai=s.step(rb_open|bit(Button::A),{.8f,.6f});
  assert(ai.held(deck183::CommanderPage)&&ai.held(W)&&ai.held(A)&&!ai.held(F));
  assert(std::fabs(ai.wheel_pointer.x-.8f)<.00001f&&std::fabs(ai.wheel_pointer.y+.6f)<.00001f);
  auto neutral=s.step(rb_open,{});
  assert(neutral.held(W)&&neutral.held(A)&&!neutral.wheel_pointer.x&&!neutral.wheel_pointer.y);
  assert(commander76::pc_page_key=='V');
  assert(!commander76::wheel_direction(NAN,1,0).x&&!commander76::wheel_direction(0,0,0).y);
  ++cases;
 }
 {commander76::OrderFeedback feedback;assert(!feedback.flash(2,1000)&&!feedback.hold_pace);
  feedback.confirm(0,true,1000);assert(feedback.flash(0,1000)&&!feedback.hold_pace);
  feedback.confirm(2,false,1100);assert(feedback.hold_pace==1&&feedback.flash(2,1101)&&!feedback.flash(2,1220)&&feedback.flash(2,1340));
  feedback.confirm(2,true,1350);assert(feedback.hold_pace==2&&feedback.flash(2,1350)&&!feedback.flash(2,2070));++cases;
 }
 std::cout<<"PASS wheel197 policy: "<<cases<<" analog stick selection/D-pad refusal, single/double-confirm, empty/cancel/menu/help/focus/epoch cases\n";
}
