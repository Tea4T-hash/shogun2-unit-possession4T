#pragma once
#include "animation_view.h"
namespace look96 {
inline bool pitched_eye(const deathpose::Eye& native,float requested,deathpose::Eye& result){
 float yaw{},pitch{};if(!std::isfinite(requested)||!animationview::angles(native,yaw,pitch))return false;
 requested=std::clamp(requested,-1.553343034f,1.553343034f);
 const float axis[3]={std::cos(yaw),0,-std::sin(yaw)};float a=pitch-requested,c=std::cos(a),sn=std::sin(a);
 result=native;
 // Rotate forward and up together, retaining native eye translation, yaw and roll.
 float forward[3];for(unsigned k=0;k<3;++k)forward[k]=native.view[k+3]-native.view[k];
 if(!deathpose::normalize(forward))return false;
 auto rotate=[&](const float* v,float* out){
  float cross[3]={axis[1]*v[2]-axis[2]*v[1],axis[2]*v[0]-axis[0]*v[2],axis[0]*v[1]-axis[1]*v[0]},d=deathpose::dot(axis,v);
  for(unsigned k=0;k<3;++k)out[k]=v[k]*c+cross[k]*sn+axis[k]*d*(1-c);
 };
 float direction[3];rotate(forward,direction);rotate(native.up,result.up);
 for(unsigned k=0;k<3;++k)result.view[k+3]=native.view[k]+direction[k];
 return animationview::angles(result,yaw,pitch);
}
inline float mouse_pitch(float pitch,float dy,float sensitivity){return std::clamp(pitch-float(dy)*sensitivity,-1.553343034f,1.553343034f);}
}
