#pragma once
#include <algorithm>
#include <cmath>
namespace deck178 {
// Preserve signed fractional movement. Rounding the absolute positive screen
// coordinate floors every tiny negative change but discards positive ones.
// With resting-thumb jitter that old calculation drifts up/left indefinitely.
struct Cursor {
    double remainder_x{},remainder_y{};
    void reset(){remainder_x=remainder_y=0;}
    bool move(int& x,int& y,int width,int height,double dx,double dy){
        if(width<1||height<1||width>32767||height>32767||!std::isfinite(dx)||!std::isfinite(dy)||
           std::fabs(dx)>32767||std::fabs(dy)>32767){reset();return false;}
        double total_x=remainder_x+dx,total_y=remainder_y+dy;
        int step_x=int(std::trunc(total_x)),step_y=int(std::trunc(total_y));
        remainder_x=total_x-step_x;remainder_y=total_y-step_y;
        x=std::clamp(x+step_x,0,width-1);y=std::clamp(y+step_y,0,height-1);
        if((x==0&&remainder_x<0)||(x==width-1&&remainder_x>0))remainder_x=0;
        if((y==0&&remainder_y<0)||(y==height-1&&remainder_y>0))remainder_y=0;
        return true;
    }
};
// The native camera's normal/override time factor already participates in its
// height-dependent movement and rotation rates (325c50/2f7770/324950). Convert
// our callback duration to native ticks rather than multiplying by dt twice.
inline bool camera_ticks(float elapsed,float native_tick,float& result){
    result=0;
    if(!std::isfinite(elapsed)||!std::isfinite(native_tick)||elapsed<0||elapsed>.05f||
       native_tick<.0001f||native_tick>.25f)return false;
    result=elapsed/native_tick;return std::isfinite(result);
}
}
