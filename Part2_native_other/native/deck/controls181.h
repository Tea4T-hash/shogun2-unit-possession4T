#pragma once
#include "input174.h"
#include "bindings183.h"
#include "qol193.h"
#include "fine216.h"
#include <array>
#include "view_help186.h"
#include "help_input188.h"
#include "commander_wheel84.h"
namespace deck181 {
using deck174::Button;using deck174::Frame;using deck174::Context;using deck174::Vec2;
enum class Gesture {None,Possess,ReturnRTS,Perspective};
// Y owns possession/return. R3 only changes perspective while held in possession.
struct R3Gesture {
 bool held{},long_fired{};uint64_t pressed{};
 void reset(){*this={};}
 Gesture sample(bool down,uint64_t now,Context context,const deck174::Config& c){
  if(context!=Context::Possession||!down){reset();return Gesture::None;}
  if(!held){held=true;pressed=now;}
  if(!long_fired&&now>=pressed&&now-pressed>=uint64_t(c.hold_ms)*1000){long_fired=true;return Gesture::Perspective;}
  return Gesture::None;
 }
};

enum Key:unsigned {Primary=1,Aim=2,Cancel=3,Middle=4,Shift=16,Control=17,Escape=27,Space=32,One=49,A=65,C=67,D=68,E=69,F=70,G=71,K=75,M=77,P=80,Q=81,R=82,S=83,T=84,W=87,Z=90,F9=120,F10=121};
struct Output {
 Context context{Context::Inactive};std::array<uint32_t,8> keys{};Vec2 look{},stick_rate{},stick_delta{};float zoom{};bool accepted{},show_controls{},help_open{},help_blocked{};unsigned help_page{};unsigned shortcuts{},commander_rts_generation{};int commander_sector{-1};commander76::WheelPointer wheel_pointer{};bool commander_confirm{};std::array<commander76::UnitOrder,2> wheel_orders{};bool fire_at_will{},skirmish{},toggle_slow214{},toggle_pause214{};deck216::FineIntent fine216;
 bool held(unsigned key)const{return key<256&&(keys[key/32]&(uint32_t(1)<<(key%32)))!=0;}
 void set(unsigned key){if(key<256)keys[key/32]|=uint32_t(1)<<(key%32);}
};
// Possession has its own map. No RTS camera, selection or order gesture can
// leak into it. All outputs feed the mod's existing native action consumers.
class Controls {
 deck174::Config config_;deck174::Normalizer normalizer_;Context context_{Context::Inactive};
 deck188::HelpInput help188_;R3Gesture r3_;uint64_t epoch_{},last_us_{},sequence_{};bool crouched_{},fine_cancelled_{},left_trigger_{},right_trigger_{},pad_seed_{},zoom_seed_{};Vec2 pad_{},zoom_{};
 enum class Sprint {Default, Walk, Run};
 Sprint sprint_{Sprint::Default};
 bool following_{};
 deck216::FinePad fine216_;deck193::Movement movement193_;deck193::GroupGesture groups193_;
 deck183::TapHold fire_grip_,group_grip_;
 bool command_wheel_{},wheel_cancelled_{},commander_page_{},wheel_a_{};int wheel_sector_{-1};uint64_t wheel_directions_{};bool wheel_look_latched_{};
 unsigned rts_generation_{};
 uint64_t rts_l3_pressed_{};
 bool rts_wheel_{},rts_drain_{},rts_rt_{},wheel_rt_block_{};
 int rts_sector_{-1},commander_sector_{-1};
 commander76::OrderClicks order_clicks_;
 void clear_rts_commander(){rts_generation_=0;rts_l3_pressed_=0;rts_wheel_=rts_drain_=rts_rt_=false;rts_sector_=-1;}
 int commander_point(int previous,const Frame& frame,unsigned targets)const{
  const commander76::WheelLayout layout(targets);
  const int sector=layout.point(frame.right_stick.x,-frame.right_stick.y,config_.radial_threshold);
  if(sector>=0)return sector;
  for(unsigned i=0;i<layout.size;++i)if(layout.sectors[i]==previous)return previous;
  return -1;
 }
 bool rts_commander(const Frame& frame,const deck174::Edges& edges,uint64_t now,unsigned generation,unsigned targets,Output& out){
  const auto l3=deck174::bit(Button::L3),cancel=deck174::bit(Button::B);
  const bool right=frame.right_trigger>=config_.trigger_press;
  const bool rt_held=frame.right_trigger>config_.trigger_release;
  const bool left=frame.left_trigger>config_.trigger_release;
  const bool back=(frame.buttons&cancel)!=0;
  if(generation!=rts_generation_){
   rts_drain_=rts_drain_||(rts_wheel_&&(rt_held||back));
   rts_l3_pressed_=0;rts_wheel_=false;rts_sector_=-1;
  }
  if(rts_drain_&&(rt_held||back)){
   out.commander_rts_generation=rts_generation_;return true;
  }
  rts_drain_=false;rts_generation_=generation;
  if(!generation)return false;
  if(rts_wheel_){
   if(back||(edges.down&l3)||left||(frame.buttons&~(l3|cancel|deck174::touch_buttons))){
    rts_wheel_=false;rts_sector_=-1;
    rts_drain_=rt_held||back;
   }else{
    rts_sector_=commander_point(rts_sector_,frame,targets);
    out.commander_confirm=right&&!rts_rt_&&rts_sector_>=0;
    out.commander_sector=rts_sector_;
   }
  }else{
   if(rt_held||left||(frame.buttons&~(l3|deck174::touch_buttons)))rts_l3_pressed_=0;
   else if(edges.down&l3)rts_l3_pressed_=now;
   if(edges.up&l3){
    rts_wheel_=rts_l3_pressed_&&now>=rts_l3_pressed_&&now-rts_l3_pressed_<uint64_t(config_.hold_ms)*1000;
    rts_l3_pressed_=0;
   }
   if(!rts_wheel_){rts_rt_=rt_held;return false;}
  }
  rts_rt_=rts_rt_?rt_held:right;
  if(rts_wheel_){
   out.set(deck183::WheelHeld);out.set(deck183::CommanderPage);
   out.wheel_pointer=commander76::wheel_direction(frame.right_stick.x,-frame.right_stick.y,config_.radial_threshold);
  }
  out.commander_rts_generation=generation;
  return true;
 }
 void clear(){order_clicks_.reset();sprint_=Sprint::Default;following_=false;fine216_.reset();movement193_.reset();groups193_.reset();fire_grip_.reset();group_grip_.reset();clear_rts_commander();command_wheel_=wheel_cancelled_=wheel_look_latched_=commander_page_=wheel_a_=wheel_rt_block_=false;wheel_sector_=commander_sector_=-1;wheel_directions_=0;normalizer_.reset();r3_.reset();help188_.reset();crouched_=fine_cancelled_=left_trigger_=right_trigger_=pad_seed_=zoom_seed_=false;last_us_=sequence_=0;}
public:
 explicit Controls(deck174::Config c={}):config_(c){}
 void reset(){clear();context_=Context::Inactive;epoch_=0;}
 Output sample(const Frame& frame,Context context,uint64_t now,unsigned commander_rts_generation=0,unsigned commander_targets=0,bool following=false){
  Output out;out.context=context;
  if(context!=context_||frame.epoch!=epoch_){clear();context_=context;epoch_=frame.epoch;}
  if(frame.sequence<=sequence_&&deck174::valid(frame,now))return out;
  sequence_=frame.sequence;deck174::Edges edges;
  if(!normalizer_.accept(frame,now,config_,edges)){clear();return out;}
  out.accepted=true;float dt=last_us_&&now>=last_us_?float(std::min<uint64_t>(now-last_us_,50000))*1e-6f:0;last_us_=now;
  auto held=[&](Button b){return (frame.buttons&deck174::bit(b))!=0;};
  auto down=[&](Button b){return (edges.down&deck174::bit(b))!=0;};
  auto help_event188=help188_.sample(frame,edges,now,context==Context::RTS||context==Context::Possession);
  out.help_open=help188_.open;out.help_page=help188_.page;
  out.help_blocked=help_event188.blocked;
  if(help_event188.blocked){order_clicks_.reset();sprint_=Sprint::Default;following_=false;crouched_=false;fine216_.reset();command_wheel_=false;commander_page_=false;wheel_sector_=-1;wheel_cancelled_=true;clear_rts_commander();movement193_.reset();groups193_.reset();fire_grip_.reset();group_grip_.reset();if(!help188_.open)normalizer_.reset();return out;}
  if(help_event188.toggle_hud)out.set(K);
  if(context!=Context::RTS&&context!=Context::Possession)return out;
  switch(r3_.sample(held(Button::R3),now,context,config_)){
   case Gesture::Perspective:out.set(P);break;default:break;
  }
  if(held(Button::Menu))out.set(Escape);
  if(context==Context::RTS){
   if(rts_commander(frame,edges,now,commander_rts_generation,commander_targets,out)){
    fine216_.reset();fine_cancelled_=held(Button::LeftPadClick);out.fine216.cancel=true;
    return out;
   }
   if(!held(Button::LeftPadClick))fine_cancelled_=false;
   if(down(Button::A)||down(Button::B)){
    fine216_.reset();
    fine_cancelled_=held(Button::LeftPadClick);
    out.fine216.cancel=true;
   }else if(!fine_cancelled_)out.fine216=fine216_.sample(frame,now);
   auto request=[&](deck183::Shortcut action){out.shortcuts|=1u<<unsigned(action);};
   if(down(Button::Y))out.set(F9);if(held(Button::R3)||(held(Button::RightPadClick)&&!out.fine216.owns_right_pad))out.set(Space);if(down(Button::B))request(deck183::Deselect);
   if(down(Button::X))request(deck183::ToggleRun);
   if(down(Button::R4))request(held(Button::L4)?deck183::SelectAll:deck183::ToggleMelee); // L4 + R4 selects all; shoulders own zoom.
   if(held(Button::L4))out.set(Shift);if(held(Button::L3))out.set(Control);
   const auto fire=fire_grip_.sample(held(Button::L5),down(Button::L5),now,config_.hold_ms);
   out.fire_at_will=fire==deck183::PressAction::Tap;
   out.skirmish=fire==deck183::PressAction::Hold;
   const auto group_press=group_grip_.sample(held(Button::R5),down(Button::R5),now,config_.hold_ms);
   if(group_press==deck183::PressAction::Tap)request(deck183::ToggleGroup);
   if(group_press==deck183::PressAction::Hold)request(deck183::LockGroup);
   const Button dirs[]={Button::Up,Button::Down,Button::Left,Button::Right};
   unsigned mask=0;for(unsigned i=0;i<4;++i)if(held(dirs[i]))mask|=1u<<i;
   const int group=groups193_.sample(mask,now);
   if(group>=0)request(deck183::Shortcut((group>=4?deck183::Assign1:deck183::Recall1)+(group%4)));
  }
  if(context!=Context::Possession)return out;

  auto move=deck174::deadzone(frame.left_stick,config_.left_deadzone);
  const auto direction193=movement193_.sample(move);
  if(following&&!following_)sprint_=Sprint::Default;
  following_=following;
  if(down(Button::L3)&&!held(Button::R1)&&!command_wheel_&&!held(Button::Menu)&&!held(Button::Y)){
   const bool running=sprint_==Sprint::Run||(sprint_==Sprint::Default&&following_);
   sprint_=running?Sprint::Walk:Sprint::Run;
  }
  left_trigger_=left_trigger_?frame.left_trigger>config_.trigger_release:frame.left_trigger>=config_.trigger_press;
  // LT suspends the output, leaving the L3 toggle ready to resume on release.
  auto set_movement=[&](){
   if(direction193.y>0)out.set(W);if(direction193.y<0)out.set(S);
   if(direction193.x<0)out.set(A);if(direction193.x>0)out.set(D);
   if(!left_trigger_&&sprint_==Sprint::Run)out.set(Shift);
   if(following_&&(left_trigger_||sprint_==Sprint::Walk))out.set(deck183::WalkPace);
  };
  const bool was_right=right_trigger_;
  right_trigger_=right_trigger_?frame.right_trigger>config_.trigger_release:frame.right_trigger>=config_.trigger_press;
  if(!right_trigger_)wheel_rt_block_=false;
  if(left_trigger_)out.set(Aim);
  if(held(Button::L4))out.set(Middle); // L4 retains orbit.
  // Both pages consume RT. Only a fresh press confirms a selected slice.
  wheel_directions_&=frame.buttons;
  const uint64_t directions=deck174::bit(Button::Up)|deck174::bit(Button::Down)|deck174::bit(Button::Left)|deck174::bit(Button::Right);
  const bool wheel=held(Button::R1);
  const auto double_time=uint64_t(config_.double_tap_ms)*1000;
  if(wheel||command_wheel_||held(Button::Menu)||held(Button::Y))fire_grip_.reset();
  else out.skirmish=fire_grip_.sample(held(Button::L5),down(Button::L5),now,config_.hold_ms)==deck183::PressAction::Hold;
  if(held(Button::Menu)||held(Button::Y)||(wheel&&held(Button::B))||(command_wheel_&&held(Button::B))){
   wheel_cancelled_=true;wheel_sector_=-1;order_clicks_.reset();
   if(wheel||command_wheel_)wheel_directions_|=frame.buttons&deck174::bit(Button::B);
  }
  if(wheel){wheel_directions_|=frame.buttons&directions;if(std::max(std::fabs(frame.right_stick.x),std::fabs(frame.right_stick.y))>=config_.radial_threshold)wheel_look_latched_=true;}
  else if(!deck174::deadzone(frame.right_stick,config_.right_deadzone).x&&!deck174::deadzone(frame.right_stick,config_.right_deadzone).y)wheel_look_latched_=false;
  if(!held(Button::A))wheel_a_=false;
  if(wheel){
   pad_seed_=zoom_seed_=false;
   if(!command_wheel_){wheel_sector_=commander_sector_=-1;commander_page_=false;order_clicks_.reset();}
   command_wheel_=true;
   if(down(Button::A)&&!wheel_cancelled_){
    out.wheel_orders[0]=order_clicks_.finish(now,double_time,true);
    commander_page_=!commander_page_;wheel_sector_=commander_sector_=-1;
    if(right_trigger_)wheel_rt_block_=true;
   }
   if(held(Button::A))wheel_a_=true;
   const bool confirm=right_trigger_&&!was_right&&!wheel_rt_block_;
   if(!wheel_cancelled_){
    if(commander_page_){
     commander_sector_=commander_point(commander_sector_,frame,commander_targets);
     out.commander_sector=commander_sector_;
     out.commander_confirm=confirm&&commander_sector_>=0;
    }else{
     const int selected=deck183::wheel_sector(frame,config_.radial_threshold);
     if(selected>=0)wheel_sector_=selected;
     const auto orders=order_clicks_.sample(wheel_sector_,confirm,now,double_time);
     if(out.wheel_orders[0].sector<0)out.wheel_orders=orders;
    }
   }
   if(right_trigger_)wheel_rt_block_=true;
   out.keys={};
   set_movement();
   if(!wheel_cancelled_){
    out.set(deck183::WheelHeld);
    out.wheel_pointer=commander76::wheel_direction(frame.right_stick.x,-frame.right_stick.y,config_.radial_threshold);
    if(commander_page_)out.set(deck183::CommanderPage);
    else if(wheel_sector_>=0){
     const unsigned preview[]={deck183::WheelUp,deck183::WheelDown,deck183::WheelLeft,deck183::WheelRight};
     out.set(preview[wheel_sector_]);
    }
   }
   if(held(Button::Y))out.set(F10);
   if(held(Button::Menu))out.set(Escape);
   return out;
  }
  if(command_wheel_){
   if(!wheel_cancelled_&&!commander_page_)out.wheel_orders[0]=order_clicks_.finish(now,double_time,true);
   if(right_trigger_)wheel_rt_block_=true;
   command_wheel_=false;commander_page_=false;wheel_sector_=commander_sector_=-1;
   order_clicks_.reset();
  }
  wheel_cancelled_=false;
  set_movement();
  if(right_trigger_&&!wheel_rt_block_)out.set(Primary);
  if(held(Button::A)&&!wheel_a_)out.set(F);
  const std::pair<Button,unsigned> bindings[]={{Button::X,One},{Button::Y,F10},{Button::L1,Q},{Button::Menu,Escape}};
  for(auto binding:bindings)if(held(binding.first))out.set(binding.second);
  if(down(Button::B)&&!(wheel_directions_&deck174::bit(Button::B))&&!held(Button::Menu)&&!held(Button::Y))crouched_=!crouched_;
  if(crouched_)out.set(Control);
  out.toggle_slow214=down(Button::R4)&&!wheel;out.toggle_pause214=down(Button::R5)&&!wheel;
  if(!wheel){if(held(Button::Up)&&!(wheel_directions_&deck174::bit(Button::Up)))out.set(T);if(held(Button::Down)&&!(wheel_directions_&deck174::bit(Button::Down)))out.set(G);}
  auto look=deck174::deadzone(frame.right_stick,config_.right_deadzone);
  // Existing mouse-look sensitivity is .0025 radians/count. Keep a continuous
  // degrees/second stick rate and retain fractional counts in the runtime.
  out.stick_rate={look.x*config_.look_sensitivity/.0025f,-look.y*config_.look_sensitivity/.0025f};
  out.stick_delta={out.stick_rate.x*dt,out.stick_rate.y*dt};out.look=out.stick_delta;
  if(held(Button::RightPadTouch)){
   if(pad_seed_){out.look.x+=(frame.right_pad.x-pad_.x)*320.f*config_.cursor_sensitivity;out.look.y-=(frame.right_pad.y-pad_.y)*320.f*config_.cursor_sensitivity;}
   pad_=frame.right_pad;pad_seed_=true;
  }else pad_seed_=false;
  if(held(Button::LeftPadTouch)){
   if(zoom_seed_)out.zoom=frame.left_pad.y-zoom_.y;
   zoom_=frame.left_pad;zoom_seed_=true;
  }else zoom_seed_=false;
  if(wheel||wheel_look_latched_){out.look={};out.stick_rate={};out.stick_delta={};}if(wheel)out.zoom=0;else{if(held(Button::Left)&&!(wheel_directions_&deck174::bit(Button::Left)))out.zoom+=dt*2.f;if(held(Button::Right)&&!(wheel_directions_&deck174::bit(Button::Right)))out.zoom-=dt*2.f;}
  if(config_.invert_x){out.look.x=-out.look.x;out.stick_rate.x=-out.stick_rate.x;out.stick_delta.x=-out.stick_delta.x;}
  if(config_.invert_y){out.look.y=-out.look.y;out.stick_rate.y=-out.stick_rate.y;out.stick_delta.y=-out.stick_delta.y;}
  return out;
 }
};

struct Queued {Frame frame;Output output;unsigned lease{},actor{},unit{};};
struct Mailbox {
 static constexpr unsigned capacity=32;std::array<Queued,capacity> frames{};unsigned count{};bool discontinuity{};
 uint64_t sequence{},epoch{};
 void invalidate(){count=0;discontinuity=true;sequence=epoch=0;}
 bool put(const Frame& frame,const Output& output,unsigned lease,unsigned actor,unsigned unit){
  if(frame.epoch==epoch&&frame.sequence<=sequence)return false;
  if(frame.epoch!=epoch){count=0;discontinuity=true;}
  sequence=frame.sequence;epoch=frame.epoch;if(count==capacity){count=0;discontinuity=true;return false;}
  frames[count++]={frame,output,lease,actor,unit};return true;
 }
 unsigned take(std::array<Queued,capacity>& out,bool& reset){reset=discontinuity;discontinuity=false;unsigned size=count;for(unsigned i=0;i<size;++i)out[i]=frames[i];count=0;return size;}
};
} // deck181
