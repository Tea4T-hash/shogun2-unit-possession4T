#pragma once
#include <cmath>
#include "protocol.h"
inline bool mounted_forward95(unsigned input){return (input&(FORWARD|BACKWARD))==FORWARD;}
inline bool mounted_backward95(unsigned input){return (input&(FORWARD|BACKWARD))==BACKWARD;}
inline unsigned personal_travel_input91(unsigned held,bool follow,bool aiming,bool mounted){
    const bool translating=bool(held&FORWARD)!=bool(held&BACKWARD)||
        (!mounted&&(bool(held&LEFT)!=bool(held&RIGHT)));
    if(mounted&&mounted_backward95(held))return held&~RUN;
    return follow&&!aiming&&translating&&!(held&WALK_OVERRIDE)?held|RUN:held;
}
struct Intent { float x,z; short turn; };
inline Intent steering_view(unsigned input,float a,float dt,float speed) {
    float f=float(bool(input&FORWARD))-float(bool(input&BACKWARD));
    float r=float(bool(input&RIGHT))-float(bool(input&LEFT));
    float n=std::sqrt(f*f+r*r);if(n>1){f/=n;r/=n;}
    // Verified native bearing table: zero faces +Z; quarter-turn faces +X.
    float x=(std::sin(a)*f+std::cos(a)*r)*speed*dt;
    float z=(std::cos(a)*f-std::sin(a)*r)*speed*dt;
    int turn=int(bool(input&TURN_RIGHT))-int(bool(input&TURN_LEFT));
    return {x,z,short(turn*int(65536.0f*0.20f*dt))};
}
// Legacy fixture overload only. The runtime supplies the native effective speed.
inline Intent steering_view(unsigned input,float a,float dt){return steering_view(input,a,dt,(input&RUN)?3.0f:1.8f);}
inline Intent steering(unsigned input,unsigned bearing,float dt){return steering_view(input,float(bearing)*6.2831853071795864769f/65536.0f,dt);}
inline short face_view(unsigned bearing,float yaw,float dt,float native_turn_rate) {
    if(!std::isfinite(yaw)||!std::isfinite(dt)||!std::isfinite(native_turn_rate)||dt<=0||native_turn_rate<0)return 0;
    constexpr double circle=6.2831853071795864769;
    int wanted=int(std::lround(std::remainder(double(yaw),circle)*65536.0/circle));
    int diff=int(short(wanted-int(bearing&65535u)));
    double ticks=double(native_turn_rate)*double(dt)*65536.0/circle;
    int limit=ticks>=32767.0?32767:int(ticks);
    return short(diff>limit?limit:(diff<-limit?-limit:diff));
}
// Legacy fixture overload; runtime always supplies the native record's rad/s.
inline short face_view(unsigned bearing,float yaw,float dt){return face_view(bearing,yaw,dt,1.2566370614359172f);}
// Mounted control never reads camera yaw. A/D steer at the horse's native turn
// rate; W/S move along its heading. Shift is handled by the native speed getter.
inline Intent steering_mount(unsigned input,unsigned bearing,float dt,float speed,float native_turn_rate){
    if(!std::isfinite(dt)||!std::isfinite(speed)||!std::isfinite(native_turn_rate)||dt<=0||dt>.2f||speed<0||native_turn_rate<0)return {};
    int turn=int(bool(input&RIGHT))-int(bool(input&LEFT));
    turn+=int(bool(input&TURN_RIGHT))-int(bool(input&TURN_LEFT));
    turn=turn>0?1:(turn<0?-1:0);
    double ticks=double(native_turn_rate)*double(dt)*65536.0/6.2831853071795864769;
    short delta=short(turn*int(ticks>=32767.0?32767:ticks));
    // Move along the heading reached this tick, not the independently turned view.
    float yaw=float(static_cast<unsigned short>(bearing+delta))*6.2831853071795864769f/65536.0f;
    float forward=float(bool(input&FORWARD))-float(bool(input&BACKWARD));
    return {std::sin(yaw)*forward*speed*dt,std::cos(yaw)*forward*speed*dt,delta};
}
