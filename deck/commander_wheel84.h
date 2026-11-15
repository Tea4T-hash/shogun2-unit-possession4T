#pragma once

#include <cmath>
#include <algorithm>
#include <array>
#include <cstdint>

namespace commander76 {
inline constexpr unsigned group_count = 10;
inline constexpr int army_sector = 0, unit_sector = 1, first_group = 2;
inline constexpr unsigned unit_bit = 1u << group_count;
inline constexpr unsigned valid_targets = (unit_bit << 1) - 1;
inline constexpr int last_sector = first_group + int(group_count) - 1;

inline float wheel_radius(float width, float height) {
    return (std::min)({280.f, width*.31f, height*.36f});
}
struct WheelPointer {
    float x{},y{};
    bool valid()const{
        const float squared=x*x+y*y;
        return (x==0&&y==0)||(squared>=.999f&&squared<=1.001f);
    }
};
inline WheelPointer wheel_direction(float x,float y,float minimum){
    const float length=std::hypot(x,y);
    if(!std::isfinite(length)||length==0||length<minimum)return {};
    return {x/length,y/length};
}
inline constexpr unsigned pc_page_key='V';
struct UnitOrder {int sector{-1};bool run{};};
struct OrderClicks {
    int pending{-1};
    uint64_t tick{},follow_tick{};
    void reset(){*this={};}
    UnitOrder finish(uint64_t now,uint64_t delay,bool closing=false){
        if(pending<0)return {};
        if(now<tick){reset();return {};}
        if(!closing&&now-tick<delay)return {};
        const UnitOrder result{pending,false};
        pending=-1;tick=0;
        return result;
    }
    std::array<UnitOrder,2> sample(int sector,bool pressed,uint64_t now,uint64_t delay){
        pressed=pressed&&sector>=0&&sector<4;
        if(pressed){
            if(sector==0&&follow_tick&&now>=follow_tick&&now-follow_tick<=delay)pressed=false;
            else follow_tick=sector==0?now:0;
        }
        if(pressed&&pending==sector&&now>=tick&&now-tick<=delay){
            reset();
            return {UnitOrder{sector,true},UnitOrder{}};
        }
        std::array<UnitOrder,2> orders{finish(now,delay,pressed),UnitOrder{}};
        if(pressed){
            if(sector==2){pending=sector;tick=now;}
            else orders[orders[0].sector>=0?1:0]={sector,false};
        }
        return orders;
    }
};
struct OrderFeedback {
    std::array<uint64_t,4> issued{};
    unsigned hold_pace{}; // 0: unset, 1: walk, 2: run
    void confirm(unsigned sector,bool run,uint64_t now){
        if(sector>=issued.size())return;
        issued[sector]=now;
        if(sector==2)hold_pace=run?2u:1u;
    }
    bool flash(unsigned sector,uint64_t now)const{
        if(sector>=issued.size()||!issued[sector]||now<issued[sector])return false;
        const auto age=now-issued[sector];
        return age<720&&age/120%2==0;
    }
};
inline int order_point(float x,float y,float threshold){
    if(!std::isfinite(x)||!std::isfinite(y)||std::max(std::fabs(x),std::fabs(y))<threshold)return -1;
    return std::fabs(y)>=std::fabs(x)?(y<0?0:1):(x<0?2:3);
}
struct WheelLayout {
    int sectors[2 + group_count]{army_sector};
    unsigned size{1};

    explicit WheelLayout(unsigned targets) {
        for (unsigned i = 0; i < group_count; ++i)
            if (targets & (1u << i)) sectors[size++] = first_group + int(i);
        if (targets & unit_bit) sectors[size++] = unit_sector;
    }
    float sweep() const {
        // Keep the army toggle at the bottom and give the remaining
        // choices equal space around the other side of the wheel.
        return size == 1 ? 126.f : size == 2 ? 144.f : size == 3 ? 110.f : 360.f / float(size);
    }
    float angle(unsigned index) const {
        if (!index) return 90.f;
        return 270.f + (float(index - 1) - .5f * float(size - 2)) * sweep();
    }
    int point(float x, float y, float minimum) const {
        if (!std::isfinite(x) || !std::isfinite(y) || x*x+y*y < minimum*minimum) return -1;
        constexpr float degrees = 57.295779513f;
        const float bearing = std::atan2(y, x) * degrees;
        for (unsigned i = 0; i < size; ++i)
            if (std::abs(std::remainder(bearing - angle(i), 360.f)) <= sweep()*.5f)
                return sectors[i];
        return -1;
    }

};
}
