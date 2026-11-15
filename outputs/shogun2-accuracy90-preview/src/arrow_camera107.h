#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace arrowcam107 {
constexpr uint32_t no_projectile=0xffffffffu;
constexpr uint64_t acquisition_ms=8000;
constexpr double landing_ms=2000., unreadable_ms=350.;
constexpr double max_sample_interval_ms=200.;
struct Shot {uintptr_t context{},pool{};uint32_t lease{},id{no_projectile};uint64_t tick{};};
inline bool eligible(const Shot& shot,uint32_t lease,uint64_t now,bool tracking=false){
    return lease&&shot.lease==lease&&shot.id!=no_projectile&&now>=shot.tick&&
        (tracking||now-shot.tick<=acquisition_ms);
}
struct Flight {
    float previous[3],position[3],velocity[3];
};
inline bool finite(const Flight& flight){
    for(const auto* vector:{flight.previous,flight.position,flight.velocity})
        for(unsigned i=0;i<3;++i)if(!std::isfinite(vector[i])||std::abs(vector[i])>100000.f)return false;
    return true;
}
inline bool valid(const Flight& flight){
    return finite(flight)&&std::hypot(flight.velocity[0],flight.velocity[1],flight.velocity[2])>=.25f;
}
struct Presentation {
    uint32_t id{no_projectile},lease{};
    uint64_t shot_tick{};
    double last_motion{},interval{16.},landed_at{};
    Flight from{},destination{},flight{};
    bool active{},cadence_known{},landed{};
    bool matches(const Shot& shot)const{return id==shot.id&&lease==shot.lease&&shot_tick==shot.tick;}
    bool tracking(const Shot& shot)const{return active&&matches(shot);}
    Flight at(double now)const{
        auto value=destination;
        const float weight=float(std::clamp((now-last_motion)/interval,0.,1.));
        for(unsigned i=0;i<3;++i){
            value.position[i]=from.position[i]+(destination.position[i]-from.position[i])*weight;
            value.velocity[i]=from.velocity[i]+(destination.velocity[i]-from.velocity[i])*weight;
        }
        return value;
    }
    bool update(const Shot& shot,double now,const Flight* next,bool ended=false){
        if(!matches(shot)){*this={};id=shot.id;lease=shot.lease;shot_tick=shot.tick;}
        if(!std::isfinite(now)||(active&&now<last_motion))return false;
        if(landed){
            if(now-landed_at>=landing_ms)return false;
            flight=at(now);
            return true;
        }
        if(ended){
            if(!next&&!active)return false;
            auto impact=next?*next:destination;
            if(!finite(impact))return false;
            // The engine zeros velocity on impact. Retain the approach direction
            // so the camera stays on the side the arrow arrived from.
            if(!valid(impact))for(unsigned i=0;i<3;++i)
                impact.velocity[i]=active?destination.velocity[i]:impact.position[i]-impact.previous[i];
            if(!valid(impact))return false;
            from=active?at(now):impact;
            destination=impact;
            interval=std::min(interval,120.);
            last_motion=landed_at=now;
            active=landed=true;
            flight=at(now);
            return true;
        }
        if(next&&valid(*next)){
            bool moved=!active;
            for(unsigned i=0;i<3;++i)moved=moved||destination.position[i]!=next->position[i];
            if(moved){
                const double elapsed=now-last_motion;
                const bool interpolate=active&&elapsed<=max_sample_interval_ms;
                from=interpolate?at(now):*next;
                // Native flight can advance less often than camera frames.
                // Span the whole observed step, including slow battle rates.
                if(interpolate){
                    const double observed=std::clamp(elapsed,4.,max_sample_interval_ms);
                    interval=cadence_known?interval*.25+observed*.75:observed;
                }
                cadence_known=interpolate;
                destination=*next;
                last_motion=now;
                active=true;
            }
        }
        if(!active||now-last_motion>unreadable_ms)return false;
        flight=at(now);
        return true;
    }
};
inline bool chase(const Flight& flight,float (&eye)[3],float (&target)[3]){
    if(!valid(flight))return false;
    const float speed=std::hypot(flight.velocity[0],flight.velocity[1],flight.velocity[2]);
    const float horizontal=std::hypot(flight.velocity[0],flight.velocity[2]);
    const float heading_x=horizontal>.01f?flight.velocity[0]/horizontal:0.f;
    const float heading_z=horizontal>.01f?flight.velocity[2]/horizontal:1.f;
    for(unsigned i=0;i<3;++i)target[i]=flight.position[i]+1.8f*flight.velocity[i]/speed;
    constexpr float distance=6.f,side=.20f,height=.35f;
    eye[0]=flight.position[0]-distance*heading_x+side*heading_z;
    eye[1]=flight.position[1]+height;
    eye[2]=flight.position[2]-distance*heading_z-side*heading_x;
    return true;
}
}
