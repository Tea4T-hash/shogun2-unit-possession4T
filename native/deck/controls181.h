#pragma once
#include "input174.h"
#include "bindings183.h"
#include "qol193.h"
#include <array>
#include "view_help186.h"
#include "help_input188.h"
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
 Context context{Context::Inactive};std::array<uint32_t,8> keys{};Vec2 look{},stick_rate{},stick_delta{};float zoom{};bool accepted{},show_controls{},help_open{},help_blocked{};unsigned help_page{};unsigned shortcuts{};bool fire_at_will{};
 bool held(unsigned key)const{return key<256&&(keys[key/32]&(uint32_t(1)<<(key%32)))!=0;}
 void set(unsigned key){if(key<256)keys[key/32]|=uint32_t(1)<<(key%32);}
};
// Possession has its own map. No RTS camera, selection or order gesture can
// leak into it. All outputs feed the mod's existing native action consumers.
class Controls {
 deck174::Config config_;deck174::Normalizer normalizer_;Context context_{Context::Inactive};
 deck188::HelpInput help188_;R3Gesture r3_;uint64_t epoch_{},last_us_{},sequence_{};bool sprint_{},left_trigger_{},right_trigger_{},pad_seed_{},zoom_seed_{};Vec2 pad_{},zoom_{};
 deck193::Movement movement193_;deck193::GroupGesture groups193_;
 deck174::MotionFilter motion_;bool command_wheel_{},wheel_cancelled_{};int wheel_sector_{-1};uint64_t wheel_directions_{};bool wheel_look_latched_{};
 void clear(){movement193_.reset();groups193_.reset();command_wheel_=wheel_cancelled_=wheel_look_latched_=false;wheel_sector_=-1;wheel_directions_=0;normalizer_.reset();r3_.reset();help188_.reset();sprint_=left_trigger_=right_trigger_=pad_seed_=zoom_seed_=false;last_us_=sequence_=0;motion_.recenter();}
public:
 explicit Controls(deck174::Config c={}):config_(c){}
 void reset(){clear();context_=Context::Inactive;epoch_=0;}
 Output sample(const Frame& frame,Context context,uint64_t now){
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
  if(help_event188.blocked){command_wheel_=false;wheel_sector_=-1;wheel_cancelled_=true;movement193_.reset();groups193_.reset();if(!help188_.open)normalizer_.reset();return out;}
  if(help_event188.toggle_hud)out.set(K);
  if(context!=Context::RTS&&context!=Context::Possession)return out;
  switch(r3_.sample(held(Button::R3),now,context,config_)){
   case Gesture::Perspective:out.set(P);break;default:break;
  }
  if(held(Button::Menu))out.set(Escape);
  if(context==Context::RTS){
   auto request=[&](deck183::Shortcut action){out.shortcuts|=1u<<unsigned(action);};
   if(down(Button::Y))out.set(F9);if(held(Button::A))out.set(Space);if(down(Button::B))request(deck183::Deselect);
   if(down(Button::X))request(deck183::ToggleRun);
   if(down(Button::R4))request(held(Button::L4)?deck183::SelectAll:deck183::ToggleMelee); // L4 + R4 selects all; shoulders own zoom.
   if(held(Button::L4))out.set(Shift);if(held(Button::L5)||held(Button::L3))out.set(Control);
   if(down(Button::R5))request(held(Button::L5)?deck183::LockGroup:deck183::ToggleGroup);
   const Button dirs[]={Button::Up,Button::Down,Button::Left,Button::Right};
   unsigned mask=0;for(unsigned i=0;i<4;++i)if(held(dirs[i]))mask|=1u<<i;
   const int group=groups193_.sample(mask,held(Button::L5),now);
   if(group>=0)request(deck183::Shortcut((group>=4?deck183::Assign1:deck183::Recall1)+(group%4)));
  }
  if(context!=Context::Possession)return out;

  auto move=deck174::deadzone(frame.left_stick,config_.left_deadzone);
  const auto direction193=movement193_.sample(move);
  if(direction193.y>0)out.set(W);if(direction193.y<0)out.set(S);if(direction193.x<0)out.set(A);if(direction193.x>0)out.set(D);
  if(held(Button::L3))out.set(Shift);if(held(Button::L5))out.set(Control);
  left_trigger_=left_trigger_?frame.left_trigger>config_.trigger_release:frame.left_trigger>=config_.trigger_press;
  right_trigger_=right_trigger_?frame.right_trigger>config_.trigger_release:frame.right_trigger>=config_.trigger_press;
  if(left_trigger_)out.set(Aim);if(right_trigger_)out.set(Primary);
  if(held(Button::L4))out.set(Middle); // L3 now owns held sprint; L4 retains orbit.
  // R1 owns the command wheel; release commits exactly one chosen command.
  wheel_directions_&=frame.buttons;
  const uint64_t directions=deck174::bit(Button::Up)|deck174::bit(Button::Down)|deck174::bit(Button::Left)|deck174::bit(Button::Right);
  const bool wheel=held(Button::R1);
  if(held(Button::Menu)||held(Button::Y)){wheel_cancelled_=true;wheel_sector_=-1;}
  if(wheel&&held(Button::B)){wheel_cancelled_=true;wheel_sector_=-1;wheel_directions_|=deck174::bit(Button::B);}
  if(wheel){wheel_directions_|=frame.buttons&directions;if(std::max(std::fabs(frame.right_stick.x),std::fabs(frame.right_stick.y))>=config_.radial_threshold)wheel_look_latched_=true;}
  else if(!deck174::deadzone(frame.right_stick,config_.right_deadzone).x&&!deck174::deadzone(frame.right_stick,config_.right_deadzone).y)wheel_look_latched_=false;
  if(wheel){if(!command_wheel_)wheel_sector_=-1;command_wheel_=true;int chosen=wheel_cancelled_?-1:deck183::wheel_sector(frame,config_.radial_threshold);if(chosen>=0)wheel_sector_=chosen;
   if(!wheel_cancelled_)out.set(deck183::WheelHeld);const unsigned preview[]={deck183::WheelUp,deck183::WheelDown,deck183::WheelLeft,deck183::WheelRight};if(wheel_sector_>=0)out.set(preview[wheel_sector_]);
  }else if(command_wheel_){const unsigned commands[]={C,E,Z,R};if(!wheel_cancelled_&&wheel_sector_>=0&&!held(Button::Menu)&&!held(Button::Y))out.set(commands[wheel_sector_]);command_wheel_=false;wheel_sector_=-1;}
  if(!wheel)wheel_cancelled_=false;
  const std::pair<Button,unsigned> bindings[]={{Button::A,F},{Button::B,M},{Button::X,One},{Button::Y,F10},{Button::L1,Q},{Button::R4,E},{Button::R5,C},{Button::Menu,Escape}};
  for(auto binding:bindings)if(held(binding.first)&&!(binding.first==Button::B&&(wheel||(wheel_directions_&deck174::bit(Button::B)))))out.set(binding.second);
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
  const bool quiet=!(frame.buttons&~deck174::touch_buttons)&&move.x==0&&move.y==0&&look.x==0&&look.y==0&&frame.left_trigger<=config_.trigger_release&&frame.right_trigger<=config_.trigger_release;
  auto gyro=motion_.update(frame.gyro,dt,quiet,config_);
  if(config_.gyro_possession&&held(Button::RightPadTouch)&&(frame.capabilities&deck174::Motion)){
   if(std::fabs(gyro.z)>=config_.gyro_deadzone)out.look.x+=gyro.z*dt*config_.gyro_sensitivity/.0025f;
   if(std::fabs(gyro.x)>=config_.gyro_deadzone)out.look.y+=gyro.x*dt*config_.gyro_sensitivity/.0025f;
  }
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
