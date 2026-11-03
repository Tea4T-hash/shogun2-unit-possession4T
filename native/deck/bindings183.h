#pragma once
#include "input174.h"
namespace deck183 {
using deck174::Button;using deck174::Frame;using deck174::bit;
// Button policy only. Camera vectors, rates, deadzones and native setters are
// deliberately owned by the accepted181 camera implementation.
enum Campaign:unsigned {Home=1,Arts=2,Finance=4,Diplomacy=8,Clan=16,EndTurn=32,QuickSave=64,MoveSpeed=128,Encyclopedia=256,Next=512,Previous=1024,Back=2048};
struct HoldOnce {
 uint64_t since{};bool latched{};
 void reset(){since=0;latched=false;}
 bool sample(bool held,uint64_t now,unsigned ms){if(!held){reset();return false;}if(!since)since=now;if(now<since){reset();return false;}if(!latched&&now-since>=uint64_t(ms)*1000){latched=true;return true;}return false;}
};
struct CampaignButtons {
 HoldOnce end_turn,save;
 void reset(){end_turn.reset();save.reset();}
 unsigned sample(const Frame& f,const deck174::Edges& e,uint64_t now,bool map_ready=true){
  auto down=[&](Button b){return(e.down&bit(b))!=0;};auto held=[&](Button b){return(f.buttons&bit(b))!=0;};unsigned actions=0;
  if(down(Button::X))actions|=Home;if(down(Button::Up))actions|=Arts;if(down(Button::Down))actions|=Finance;
  if(down(Button::Left))actions|=Diplomacy;if(down(Button::Right))actions|=Clan;if(down(Button::A)||down(Button::R5))actions|=MoveSpeed;
  if(down(Button::R1))actions|=Next;if(down(Button::L1))actions|=Previous;
  // A blocked panel must not consume a long press or carry an old timer back
  // onto the map. Require the full hold after the native map becomes available.
  if(map_ready){if(end_turn.sample(held(Button::Y),now,600))actions|=EndTurn;if(save.sample(held(Button::R4),now,600))actions|=QuickSave;}
  else{end_turn.reset();save.reset();}
  return actions;
 }
};
enum Shortcut:unsigned {SelectAll=0,Deselect,ToggleRun,ToggleMelee,ToggleGroup,LockGroup,Recall1,Recall2,Recall3,Recall4,Assign1,Assign2,Assign3,Assign4,ShortcutCount};
struct Chord {unsigned scan{},control{};};
// Original data/text/default_keys.xml (v63): native game's own shortcut route.
// No SendInput, Steam keyboard layout, global key state or OS input injection.
inline constexpr Chord chords[]={{0x1e,1},{0x1c,0},{0x13,0},{0x21,0},{0x22,0},{0x22,1},
 {2,0},{3,0},{4,0},{5,0},{2,1},{3,1},{4,1},{5,1}};
inline constexpr unsigned WheelHeld=254,WheelUp=250,WheelDown=251,WheelLeft=252,WheelRight=253;
inline int wheel_sector(const Frame& f,float threshold){
 if(f.buttons&bit(Button::Up))return 0;if(f.buttons&bit(Button::Down))return 1;if(f.buttons&bit(Button::Left))return 2;if(f.buttons&bit(Button::Right))return 3;
 auto v=f.right_stick;if(!deck174::unit(v)||std::max(std::fabs(v.x),std::fabs(v.y))<threshold)return -1;
 return std::fabs(v.y)>=std::fabs(v.x)?(v.y>0?0:1):(v.x<0?2:3);
}
}
