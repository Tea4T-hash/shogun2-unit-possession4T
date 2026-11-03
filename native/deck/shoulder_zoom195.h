#pragma once
#include <algorithm>
#include <cmath>
namespace deck195 {
// Battle shoulders only. Integrate the smoothstep ramp over each callback so
// the distance does not depend on display/controller polling frequency.
struct ShoulderZoom {
 static constexpr double ramp_seconds=.45,peak=.8;
 double elapsed{};int direction{};
 void reset(){elapsed=0;direction=0;}
 static double integral(double seconds){
  const double u=std::clamp(seconds/ramp_seconds,0.,1.);
  return ramp_seconds*(u*u*u-.5*u*u*u*u)+(std::max)(0.,seconds-ramp_seconds);
 }
 float step(float input,float dt){
  if(!std::isfinite(input)||std::fabs(input)>1||!std::isfinite(dt)||dt<=0||dt>.05f||input==0){reset();return 0;}
  const int wanted=input>0?1:-1;
  if(wanted!=direction){reset();direction=wanted;}
  const double next=elapsed+dt;
  const double average=(integral(next)-integral(elapsed))/dt;
  elapsed=(std::min)(next,ramp_seconds);
  return float(input*peak*std::clamp(average,0.,1.));
 }
};
}
