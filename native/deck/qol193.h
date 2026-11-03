#pragma once
#include "input174.h"
namespace deck193 {
// Only possession converts the stick to digital movement. Enter diagonals at
// 26.6 degrees from a cardinal axis; leave at 19.3 degrees to avoid chatter.
struct Movement {
 bool x{},y{};
 void reset(){x=y=false;}
 deck174::Vec2 sample(deck174::Vec2 v){
  if(!deck174::finite(v)){reset();return {};}
  const float ax=std::fabs(v.x),ay=std::fabs(v.y);
  x=ax>=(x?.12f:.18f)&&ax>=ay*(x?.35f:.50f);
  y=ay>=(y?.12f:.18f)&&ay>=ax*(y?.35f:.50f);
  return {x?(v.x>0?1.f:-1.f):0.f,y?(v.y>0?1.f:-1.f):0.f};
 }
};
struct GroupGesture {
 int active{-1};bool fired{},blocked{};uint64_t began{};
 void reset(){*this={};}
 // Result: 0..3 recall, 4..7 assign, -1 no action. A hold never recalls first.
 int sample(unsigned mask,bool assign_modifier,uint64_t now){
  if(!mask){int action=active>=0&&!fired&&!blocked?active:-1;reset();return action;}
  if(blocked)return -1;
  if(mask&(mask-1)){active=-1;blocked=true;return -1;}
  int direction=0;while(!(mask&(1u<<direction)))++direction;
  if(active<0){active=direction;began=now;}
  if(active!=direction||now<began){active=-1;blocked=true;return -1;}
  if(!fired&&(assign_modifier||now-began>=550000)){fired=true;return direction+4;}
  return -1;
 }
};
// Standard double-click classification at the actual native cursor location.
// Native window procedure performs hit testing and camera focus, not this code.
struct DoubleClick {
 int x{},y{};uint64_t began{};bool held{},eligible{},released{};
 void reset(){*this={};}
 bool within(int px,int py,int sx,int sy)const{return std::abs(px-x)<=sx&&std::abs(py-y)<=sy;}
 void move(int px,int py,int sx,int sy){if(!within(px,py,sx,sy))eligible=false;}
 bool button(bool down,bool modified,int px,int py,uint64_t now,unsigned interval,int sx,int sy){
  move(px,py,sx,sy);
  if(modified)eligible=false;
  if(!down){released=held&&eligible;held=false;return false;}
  const bool twice=!held&&released&&eligible&&!modified&&now>=began&&now-began<=interval;
  x=px;y=py;began=now;held=true;released=false;eligible=!modified&&!twice;return twice;
 }
};
}
