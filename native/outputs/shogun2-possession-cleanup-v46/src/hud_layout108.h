#pragma once
#include <algorithm>
#include <cmath>
namespace hudlayout108 {
inline bool deck_ui192=false;
struct Rect {float x,y,w,h;};
inline float scale(float width,float height){
    float s=std::min(width/1920.f,height/1080.f);
    if(width>=1280&&height>=720)s=std::max(.85f,s);
    return s*.85f*(deck_ui192?1.15f:1.f);
}
inline float qte_anchor_y(float height){return height*(deck_ui192?.25f:.33125f);} // PC: closer to the centre crosshair (our kit)
struct Layout {
    float zoom,width,height,bottom;
    Rect soldier,commands,context,ability;
    Layout(float w,float h):zoom(scale(w,h)),width(w/zoom),height(h/zoom),bottom(height-26.f),
        soldier{26.f,bottom-322.f,346.f,322.f},
        commands{width-296.f,height*.5f-122.f,270.f,244.f},
        context{width*.5f-167.f,52.f,334.f,82.f},ability{width-386.f,52.f,360.f,120.f}{}
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
