#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace bow142 {
struct Key {unsigned lease{},actor{},unit{};};
inline bool same(Key a,Key b){return a.lease&&a.lease==b.lease&&a.actor==b.actor&&a.unit==b.unit;}
struct Release {Key key{};uint64_t serial{};double seconds{};};
// Monotonic input time avoids frame-count or repeated-key dependence. The
// native draw asset defines full strength. This curve is a player-control
// policy, not a claim that stock RTS bows have a variable-draw mechanic.
inline float factor(double held,float duration){
 if(!std::isfinite(held)||held<0||!std::isfinite(duration)||duration<=0)return 1.f;
 return .35f+.65f*float(std::clamp(held/duration,0.,1.));
}
struct Input {
 bool previous{},drawing{};Key key{};double began{};uint64_t serial{};
 bool sample(bool down,bool bow,bool active,Key current,double now,Release& release){
  bool pressed=down&&!previous,released=!down&&previous;previous=down;
  if(!active||!bow||!std::isfinite(now)||!same(key,current)){drawing=false;key=current;}
  if(active&&pressed&&bow){key=current;began=now;drawing=true;}
  if(active&&released&&bow){
   const double held=held_for(current,now);
   if(held>=0){release={key,++serial,held};drawing=false;return true;}
  }
  if(!down)drawing=false;return false;
 }
 double held_for(Key current,double now)const{
  return drawing&&same(key,current)&&std::isfinite(now)&&now>=began?std::min(now-began,60.):-1.;
 }
 void cancel(){drawing=false;key={};}
};
}
