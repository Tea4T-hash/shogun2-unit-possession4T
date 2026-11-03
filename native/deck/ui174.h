#pragma once
#include "input174.h"

namespace deck174 {
constexpr float pi=3.14159265358979323846f;
// Pure hit geometry: the renderer and game adapters consume numeric models.
// No Steam overlay, fake unit data or alternative group storage is created.
inline int sector(Vec2 v,unsigned count,float threshold) {
    if(!unit(v)||count<1||count>16||v.x*v.x+v.y*v.y<threshold*threshold) return -1;
    float angle=std::atan2(v.x,v.y);if(angle<0)angle+=2*pi;
    return int(std::floor(angle/(2*pi)*count+.5f))%int(count);
}
struct Group {
    unsigned native_index{},unit_count{},strength{};bool valid{},selected{};
};
struct GroupPanel {
    std::array<Group,10> groups{};
    unsigned bank{};
    int region(Vec2 point) const {
        if(!unit(point))return -1;
        if(point.x*point.x+point.y*point.y<.09f)return 0;
        int s=sector(point,5,.30f);return s<0?-1:s+1;
    }
    unsigned native_group(int region) const {
        return region>=1&&region<=5&&bank<2?bank*5+unsigned(region):0;
    }
    void switch_bank() { bank=1-bank; }
};
struct MenuItem {
    uint32_t action{},native_icon{};bool available{},requires_target{};
};
struct Menu {
    std::array<MenuItem,8> items{};unsigned count{};int highlighted{-1};
    void point(Vec2 stick,const Config& c) {highlighted=sector(stick,count,c.radial_threshold);}
    const MenuItem* commit() const {
        return highlighted>=0&&unsigned(highlighted)<count&&items[highlighted].available?
            &items[highlighted]:nullptr;
    }
};
struct Overwrite {
    unsigned group{};uint64_t started{};bool confirmed{};
    void reset(){group=0;started=0;confirmed=false;}
    bool request(unsigned next,bool occupied,bool held,uint64_t now,const Config& c) {
        if(next<1||next>10||!held){reset();return false;}
        if(group==next&&confirmed)return false;
        if(!occupied){group=next;confirmed=true;return true;}
        if(group!=next||now<started){group=next;started=now;confirmed=false;return false;}
        if(now-started<uint64_t(c.overwrite_ms)*1000)return false;
        confirmed=true;return true;
    }
};
} // namespace deck174
