#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
// Verified RVA3aa718..3aa76d. The native negative timestep is read directly;
// acceleration scales with surface/fatigue/incline, deceleration does not.
inline float native_speed_step(float previous,float target,float scaled_acceleration,
                               float deceleration,float dt,float negative_dt){
    if(target>previous)return previous+std::min(target-previous,scaled_acceleration*dt);
    if(previous>target)return previous+std::max(target-previous,deceleration*negative_dt);
    return previous;
}
// Physical velocity from another direction cannot seed acceleration here.
inline float directed_speed95(float speed,float vx,float vz,float x,float z){
    return std::clamp(vx*x+vz*z,0.f,speed);
}
// Native terrain cell conversion rounds (2*float(coordinate*scale)-.5) using
// the active rounding mode, arithmetic-shifts by one, adds the grid origin,
// then caps as unsigned. Negative out-of-map cells follow the native cap.
inline uint32_t native_ground_cell(float coordinate,float scale,uint32_t origin,uint32_t last){
    float scaled=coordinate*scale;
    int32_t rounded=static_cast<int32_t>(std::lrint(double(scaled)*2.0-0.5));
    return std::min(uint32_t((rounded>>1)+origin),last);
}
inline unsigned native_surface_column(bool mounted,unsigned subtype,bool infantry,bool vehicle){
    if(mounted)return subtype==1?0xc:(subtype==2?0x10:0);
    if(infantry)return 0x14;
    if(vehicle)return 0x18;
    return 0;
}
