#pragma once
#include <algorithm>
#include <cmath>
namespace hudlayout108 {
struct Rect {float x,y,w,h;};
inline float scale(float width,float height){
    float s=std::min(width/1920.f,height/1080.f);
    if(width>=1280&&height>=720)s=std::max(.85f,s);
    return s*.85f*1.15f;
}
inline float qte_anchor_y(float height){return height*.25f;}
struct Layout {
    float zoom,width,height,bottom;
    Rect soldier,commands,context;
    Layout(float w,float h):zoom(scale(w,h)),width(w/zoom),height(h/zoom),bottom(height-26.f),
        soldier{26.f,bottom-302.f,466.f,302.f},
        commands{width-250.f,bottom-272.f,224.f,272.f},
        context{std::clamp(width*.5f-195.f,soldier.x+soldier.w+12.f,commands.x-402.f),bottom-80.f,390.f,80.f}{}
};
struct Context {
    bool equipment,ranged,aiming,pending,crouched;
    unsigned phase,climb,error;float ammunition,reload;
};
inline bool context_visible(const Context& s){
    if(s.climb||s.pending||s.crouched)return true;
    if(!s.equipment||!s.ranged)return false;
    return s.aiming||s.phase!=0||s.reload>.05f||s.ammunition<1.f||s.error!=0;
}
// The existing 320-square QTE surface and normalized 244-wide timing track stay
// unchanged. Only placement and presentation change; click judgement uses the
// same acknowledged Snapshot and native response clock.
inline constexpr float qte_surface=320.f,qte_span=244.f;
}
