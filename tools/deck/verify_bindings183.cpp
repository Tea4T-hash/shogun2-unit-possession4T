#include "../../native/deck/controls181.h"
#include "../../native/deck/channel181.h"
#include "../../native/deck/rts_schema178.h"
#include <cassert>
#include <iostream>
using namespace deck181;using deck174::bit;
static Frame frame(unsigned n){Frame f;f.sequence=n;f.epoch=1;f.sample_us=1000000+uint64_t(n)*8000;f.source=deck174::Source::Simulation;f.active_buttons=deck174::all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
int main(){unsigned checks=0;
 auto button=[&](Context context,Button b){Controls c;auto f=frame(1);c.sample(f,context,f.sample_us);f=frame(2);f.buttons=bit(b);return c.sample(f,context,f.sample_us);};
 for(auto item:{std::pair<Button,unsigned>{Button::A,F},{Button::B,Control},{Button::X,One},{Button::Y,F10},{Button::L1,Q},{Button::L3,Shift},{Button::Menu,deck181::Escape}}){assert(button(Context::Possession,item.first).held(item.second));++checks;}
 assert(!button(Context::Possession,Button::View).held(K));++checks;
 assert(button(Context::RTS,Button::Y).held(F9)&&button(Context::RTS,Button::R3).held(Space)&&button(Context::RTS,Button::RightPadClick).held(Space)&&!button(Context::RTS,Button::A).held(Space)&&!button(Context::RTS,Button::RightPadClick).shortcuts&&!button(Context::RTS,Button::R1).fire_at_will&&!button(Context::RTS,Button::L1).shortcuts);++checks;
 for(unsigned i=0;i<4;++i){auto b=deck174::Button(unsigned(Button::Up)+i);Controls tap;auto tf=frame(1);tap.sample(tf,Context::RTS,tf.sample_us);tf=frame(2);tf.buttons=bit(b);assert(!tap.sample(tf,Context::RTS,tf.sample_us).shortcuts);tf=frame(3);auto o=tap.sample(tf,Context::RTS,tf.sample_us);assert(o.shortcuts==(1u<<(deck183::Recall1+i)));Controls c;auto f=frame(1);c.sample(f,Context::RTS,f.sample_us);f=frame(2);f.buttons=bit(b);
 assert(!c.sample(f,Context::RTS,f.sample_us).shortcuts);
 f.sequence++;f.sample_us+=550000;o=c.sample(f,Context::RTS,f.sample_us);
 assert(o.shortcuts==(1u<<(deck183::Assign1+i))&&!o.held(Control));
 f.sequence++;f.sample_us+=8000;f.buttons=0;assert(!c.sample(f,Context::RTS,f.sample_us).shortcuts);++checks;}
 for(unsigned i=0;i<4;++i){
  Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);
  f=frame(2);f.buttons=bit(Button::R1);const Vec2 stick[]={{0,1},{0,-1},{-1,0},{1,0}};f.right_stick=stick[i];
  auto o=c.sample(f,Context::Possession,f.sample_us);assert(o.held(deck183::WheelHeld)&&!o.held(T)&&!o.held(G)&&o.look.x==0);
  f=frame(3);f.buttons=bit(Button::R1);f.right_trigger=1;o=c.sample(f,Context::Possession,f.sample_us);
  assert(!o.held(Primary)&&o.wheel_orders[0].sector==(i==2?-1:int(i)));
  f=frame(4);o=c.sample(f,Context::Possession,f.sample_us);
  assert(o.wheel_orders[0].sector==(i==2?int(i):-1)&&!o.wheel_orders[0].run);++checks;
 }
 {Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);f=frame(2);f.buttons=bit(Button::R1)|bit(Button::Left);c.sample(f,Context::Possession,f.sample_us);f=frame(3);f.focused=false;c.sample(f,Context::Possession,f.sample_us);f=frame(4);assert(!c.sample(f,Context::Possession,f.sample_us).held(Z));++checks;}
 {Controls c;auto f=frame(1);c.sample(f,Context::RTS,f.sample_us);
  f=frame(2);f.buttons=bit(Button::LeftPadClick);assert(c.sample(f,Context::RTS,f.sample_us).fine216.active);
  f=frame(3);f.buttons=bit(Button::LeftPadClick)|bit(Button::A);assert(c.sample(f,Context::RTS,f.sample_us).fine216.cancel);
  f=frame(4);f.buttons=bit(Button::LeftPadClick);assert(!c.sample(f,Context::RTS,f.sample_us).fine216.active);
  f=frame(5);assert(!c.sample(f,Context::RTS,f.sample_us).fine216.released);
  f=frame(6);f.buttons=bit(Button::LeftPadClick);assert(c.sample(f,Context::RTS,f.sample_us).fine216.began);}
 {deck183::CampaignButtons c;auto f=frame(1);deck174::Edges edges;f.buttons=bit(Button::Y)|bit(Button::R4);assert(!c.sample(f,edges,1000000));assert(!c.sample(f,edges,1599999));assert(c.sample(f,edges,1600000)==(deck183::EndTurn|deck183::QuickSave));for(unsigned n=0;n<1000;++n)assert(!c.sample(f,edges,1600001+n));c.reset();assert(!c.sample(f,edges,3000000));++checks;}
 {deck183::CampaignButtons c;auto f=frame(1);deck174::Edges e;e.down=bit(Button::Up)|bit(Button::Down)|bit(Button::Left)|bit(Button::Right)|bit(Button::X)|bit(Button::R5)|bit(Button::View)|bit(Button::L1)|bit(Button::R1);assert(c.sample(f,e,1000000)==(deck183::Arts|deck183::Finance|deck183::Diplomacy|deck183::Clan|deck183::Home|deck183::MoveSpeed|deck183::Previous|deck183::Next));++checks;}
 {ShortcutQueue183 q;deck183::Chord chord;assert(!q.take(1000,chord));assert(q.put(deck183::Assign1,1000));assert(q.take(1000,chord)&&chord.scan==2&&chord.control);assert(q.put(deck183::ToggleMelee,1001));assert(!q.take(1001,chord));assert(q.take(1002,chord)&&chord.scan==0x21);q.clear();assert(q.put(deck183::SelectAll,1000)&&!q.take(1101,chord));assert(!q.put(deck183::ShortcutCount,1000));++checks;}
 {deck174::RtsRouter178 r;auto f=frame(1);r.update(f,f.sample_us);f=frame(2);f.left_trigger=1;auto b=r.update(f,f.sample_us);assert(b.items[0].action==deck174::Action::OrderBegin&&b.items[0].index==0);f=frame(3);r.update(f,f.sample_us);f=frame(4);f.left_trigger=1;b=r.update(f,f.sample_us);assert(b.items[0].action==deck174::Action::OrderBegin&&b.items[0].index==1);++checks;}
 {deck174::RtsRouter178 r;auto f=frame(1);r.update(f,f.sample_us);f=frame(2);f.buttons=bit(Button::L4);f.left_trigger=1;auto b=r.update(f,f.sample_us);assert(b.items[0].action==deck174::Action::OrderBegin&&b.items[0].modifier);++checks;}
 {Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);f=frame(2);f.buttons=bit(Button::R1)|bit(Button::Down);f.right_stick={0,-1};c.sample(f,Context::Possession,f.sample_us);f=frame(3);f.buttons=bit(Button::Down);auto o=c.sample(f,Context::Possession,f.sample_us);assert(o.wheel_orders[0].sector<0&&!o.held(E)&&!o.held(G));f=frame(4);f.buttons=bit(Button::Down);assert(!c.sample(f,Context::Possession,f.sample_us).held(G));f=frame(5);c.sample(f,Context::Possession,f.sample_us);f=frame(6);f.buttons=bit(Button::Down);assert(c.sample(f,Context::Possession,f.sample_us).held(G));++checks;}
 {Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);f=frame(2);f.buttons=bit(Button::R1);f.right_stick={1,0};c.sample(f,Context::Possession,f.sample_us);f=frame(3);f.right_stick={1,0};auto o=c.sample(f,Context::Possession,f.sample_us);assert(o.wheel_orders[0].sector<0&&!o.held(R)&&o.look.x==0);f=frame(4);c.sample(f,Context::Possession,f.sample_us);f=frame(5);f.right_stick={1,0};assert(c.sample(f,Context::Possession,f.sample_us).look.x>0);++checks;}
 assert(button(Context::Possession,Button::R4).toggle_slow214);++checks;
 assert(button(Context::Possession,Button::R5).toggle_pause214);++checks;
 assert(!button(Context::Possession,Button::L5).held(Control));++checks;
 unsigned possession_skirmish=0;
 for(bool hold:{false,true})for(bool sample_while_held:{false,true}){
  Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);
  f=frame(2);f.buttons=bit(Button::L5);const auto started=f.sample_us;
  auto o=c.sample(f,Context::Possession,f.sample_us);assert(!o.skirmish&&!o.fire_at_will);
  if(sample_while_held){
   f.sequence++;f.sample_us=started+(hold?350000:300000);
   o=c.sample(f,Context::Possession,f.sample_us);assert(o.skirmish==hold&&!o.fire_at_will);
   f.sequence++;f.sample_us+=8000;
   o=c.sample(f,Context::Possession,f.sample_us);assert(!o.skirmish&&!o.fire_at_will);
  }
  f.sequence++;f.sample_us=started+(hold?400000:340000);f.buttons=0;
  o=c.sample(f,Context::Possession,f.sample_us);assert(o.skirmish==(hold&&!sample_while_held)&&!o.fire_at_will);
  f.sequence++;f.sample_us+=8000;
  o=c.sample(f,Context::Possession,f.sample_us);assert(!o.skirmish&&!o.fire_at_will);++possession_skirmish;
 }
 for(auto next:{Context::Possession,Context::RTS}){
  Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);
  f=frame(2);f.buttons=bit(Button::L5);c.sample(f,Context::Possession,f.sample_us);
  f.sequence++;f.sample_us+=350000;
  if(next==Context::Possession)f.buttons|=bit(Button::R1);
  assert(!c.sample(f,next,f.sample_us).skirmish);
  f.sequence++;f.sample_us+=8000;f.buttons=bit(Button::L5);
  assert(!c.sample(f,next,f.sample_us).skirmish);
  f.sequence++;f.sample_us+=8000;f.buttons=0;
  assert(!c.sample(f,next,f.sample_us).skirmish);++possession_skirmish;
 }
 std::cout<<"PASS possession64: "<<possession_skirmish<<" L5 hold, tap, wheel and context cases\n";
 unsigned grips=0;
 for(auto b:{Button::L5,Button::R5})for(bool long_press:{false,true})for(bool sample_while_held:{false,true}){
  Controls c;auto f=frame(1);c.sample(f,Context::RTS,f.sample_us);
  f=frame(2);f.buttons=bit(b);const auto started=f.sample_us;
  auto o=c.sample(f,Context::RTS,f.sample_us);
  assert(!o.shortcuts&&!o.fire_at_will&&!o.skirmish&&!o.held(Control));
  auto action=[&](const Output& result,bool hold){
   if(b==Button::L5)assert(result.fire_at_will==!hold&&result.skirmish==hold&&!result.shortcuts);
   else assert(!result.fire_at_will&&!result.skirmish&&
       result.shortcuts==(1u<<(hold?deck183::LockGroup:deck183::ToggleGroup)));
  };
  if(sample_while_held){
   f.sequence++;f.sample_us=started+(long_press?350000:349999);
   o=c.sample(f,Context::RTS,f.sample_us);
   if(long_press)action(o,true);
   else assert(!o.shortcuts&&!o.fire_at_will&&!o.skirmish);
  }
  f.sequence++;f.sample_us=started+(long_press?400000:349999);f.buttons=0;
  o=c.sample(f,Context::RTS,f.sample_us);
  if(long_press&&sample_while_held)assert(!o.shortcuts&&!o.fire_at_will&&!o.skirmish);
  else action(o,long_press);
  f.sequence++;f.sample_us+=8000;o=c.sample(f,Context::RTS,f.sample_us);
  assert(!o.shortcuts&&!o.fire_at_will&&!o.skirmish);++grips;
 }
 for(auto b:{Button::L5,Button::R5}){
  Controls c;auto f=frame(1);c.sample(f,Context::RTS,f.sample_us);
  f=frame(2);f.buttons=bit(b);c.sample(f,Context::RTS,f.sample_us);
  f=frame(3);f.focused=false;c.sample(f,Context::RTS,f.sample_us);
  f=frame(4);auto o=c.sample(f,Context::RTS,f.sample_us);
  assert(!o.shortcuts&&!o.fire_at_will&&!o.skirmish);++grips;
 }
 {deck174::RtsRouter178 r;auto f=frame(1);r.update(f,f.sample_us);
  f=frame(2);f.buttons=bit(Button::L5);f.right_trigger=1;
  auto events=r.update(f,f.sample_us);
  assert(events.count==1&&events.items[0].action==deck174::Action::SelectBegin&&!events.items[0].modifier);++grips;}
 std::cout<<"PASS rear grips56: "<<grips<<" tap, hold, reset and selection checks\n";
 std::cout<<"PASS "<<checks<<" bindings183 full-map/wheel/group/hold/queue/double-order cases; native UI effect remains a live gate\n";
}
