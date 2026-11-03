#pragma once
#include <algorithm>
#include <cmath>
namespace imagefit184 {
struct Rect {float x,y,w,h;};
// Fit the full logical canvas, including original transparent padding. Atlas
// trimming is storage only; it must not change a portrait's native proportions.
inline Rect within(Rect slot,float width,float height){
 if(!std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0||
    !std::isfinite(slot.x)||!std::isfinite(slot.y)||!std::isfinite(slot.w)||!std::isfinite(slot.h)||slot.w<=0||slot.h<=0)return {slot.x,slot.y,0,0};
 const float scale=std::min(slot.w/width,slot.h/height);
 const float w=width*scale,h=height*scale;
 return {slot.x+(slot.w-w)*.5f,slot.y+(slot.h-h)*.5f,w,h};
}
}
