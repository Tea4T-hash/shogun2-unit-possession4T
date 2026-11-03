#pragma once
#include "input174.h"
namespace deck190 {
// Absorb thumb jitter while clicking, then allow deliberate native drags.
// Modifier clicks remain pinned until release. Always seed through suppression
// so releasing a trigger or touching the pad again cannot jump the cursor.
struct SteadyClick {
 deck174::Vec2 last{},pending{};bool seeded{},pressed{},drag{},pinned{};
 void reset(){*this={};}
 deck174::Vec2 sample(deck174::Vec2 at,bool touch,bool down,bool pin){
  const bool begin=down&&!pressed,ending=!down&&pressed;
  if(begin){pending={};drag=false;pinned=pin;}
  deck174::Vec2 delta{};
  if(touch&&seeded){delta={at.x-last.x,at.y-last.y};}
  last=at;seeded=touch;pressed=down;
  if(begin||ending){delta={};if(ending){pending={};drag=pinned=false;}return delta;}
  if(down&&!drag){
   pending.x+=delta.x;pending.y+=delta.y;
   if(pinned)return {};
   const float distance=std::max(std::fabs(pending.x),std::fabs(pending.y));
   constexpr float slop=.025f;
   if(distance<=slop)return {};
   drag=true;const float excess=(distance-slop)/distance;
   delta={pending.x*excess,pending.y*excess};pending={};
  }
  return delta;
 }
};
}
