#pragma once
#include <cstdint>
#include "death_pose_geometry.h"
namespace animationview {
inline float eyelid_fraction(uint64_t elapsed_ms){
    // Eyes close 3.0-4.5 s after death, then the view returns to RTS.
    float t=float(elapsed_ms>3000?elapsed_ms-3000:0)/1500.f;t=std::fmin(1.f,t);return t*t*(3.f-2.f*t);
}
// Native134fad0 copies affine translation into the eye, adds column +Z for
// forward, and copies column +Y for up. Preserve those native values rather
// than re-aiming at an opponent through a turning model.
inline bool angles(const deathpose::Eye& eye,float& yaw,float& pitch){
    float direction[3],up[3];
    for(unsigned i=0;i<3;++i){
        if(!std::isfinite(eye.view[i])||!std::isfinite(eye.view[i+3])||std::abs(eye.view[i])>100000)return false;
        direction[i]=eye.view[i+3]-eye.view[i];up[i]=eye.up[i];
    }
    if(!deathpose::normalize(direction)||!deathpose::normalize(up)||std::abs(deathpose::dot(direction,up))>.01f)return false;
    yaw=std::atan2(direction[0],direction[2]);pitch=std::asin(std::fmax(-1.f,std::fmin(1.f,direction[1])));return true;
}
struct Handoff {
    uintptr_t actor{};uint32_t lease{},serial{},unit{};uint64_t encounter{};bool pending{},native_playback{};
    void reset(){*this={};}
    void observe(uintptr_t a,uint32_t l,uint32_t s,uint32_t u,uint64_t event,bool playing=false){
        bool preserve=matches(a,l,s,u)&&encounter==event&&native_playback;
        actor=a;lease=l;serial=s;unit=u;encounter=event;pending=true;native_playback=preserve||playing;
    }
    bool matches(uintptr_t a,uint32_t l,uint32_t s,uint32_t u)const{return pending&&actor==a&&lease==l&&serial==s&&unit==u;}
};
}
