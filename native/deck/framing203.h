#pragma once
#include <cmath>
#include <algorithm>
namespace framing203 {
// Camera-space composition is proportional to the scroll distance. The aim ray
// remains the actual centre ray; only the real, collision-tested eye is offset.
inline bool segment(const float* forward,const float* up,float distance,float* delta,float& length){
    if(!std::isfinite(distance)||distance<1.f||distance>12.f)return false;
    float right[]={up[1]*forward[2]-up[2]*forward[1],up[2]*forward[0]-up[0]*forward[2],up[0]*forward[1]-up[1]*forward[0]};
    float n=std::sqrt(right[0]*right[0]+right[1]*right[1]+right[2]*right[2]);
    if(!std::isfinite(n)||n<.001f)return false;
    float vertical[]={forward[1]*right[2]-forward[2]*right[1],forward[2]*right[0]-forward[0]*right[2],forward[0]*right[1]-forward[1]*right[0]};
    float sum=0;
    for(unsigned i=0;i<3;++i){delta[i]=distance*(-forward[i]+.25f*right[i]/n+.08f*vertical[i]/n);if(!std::isfinite(delta[i]))return false;sum+=delta[i]*delta[i];}
    length=std::sqrt(sum);return std::isfinite(length)&&length>.001f;
}
inline float fraction(float clearance,float length){return std::isfinite(clearance)&&std::isfinite(length)&&length>.001f?std::clamp(clearance/length,0.f,1.f):0.f;}
}
