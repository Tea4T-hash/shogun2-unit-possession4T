#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include <algorithm>
// Numeric observations only. No cached actor pointer grants control, and no
// peer/unit/order is written. Native AI velocity is already terrain/fatigue
// adjusted; use it as a final ceiling, never multiply the ability into it again.
struct RapidPace136 {
    uintptr_t unit{},config{};unsigned generation{},unit_id{},cursor{},samples{};
    uint64_t checked{},observed{};float factor{},speed{};unsigned refreshes{};
};
static RapidPace136 rapid_pace136;
static SRWLOCK rapid_pace_lock136=SRWLOCK_INIT;
static volatile LONG rapid_pace_present136;
static bool rapid_peer_speed136(uintptr_t rider,float& speed,uintptr_t* verified_actor=nullptr){
    __try {
        if(!rider||rider==possessed||rider==locomotion_actor())return false;
        if(!actor152::human(base,field<uintptr_t>(rider,0))||field<uintptr_t>(rider,0x214)!=unit||
           field<uintptr_t>(rider,0x28)!=environment||field<unsigned char>(rider,0xf0)!=1)return false;
        unsigned serial=field<unsigned>(rider,0x1c8);uintptr_t actor=rider;
        if(possession_lease.identity.mount){
            NativeSubject subject{};
            if(!native_subject(rider,unit,possession_lease.identity.view,false,subject))return false;
            actor=subject.actor;if(actor==rider)return false;
        }
        auto config=locomotion_config(),squad=field<uintptr_t>(actor,0x3b4);
        if(field<uintptr_t>(actor,0x20c)!=config||field<unsigned>(actor,0x1f0)>7||
           field<unsigned>(actor,0x244)>1||!squad||field<uintptr_t>(squad,0)!=base+0x15bb21c||
           field<uintptr_t>(squad,0x510)!=unit||field<uintptr_t>(squad,4)!=environment)return false;
        float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94);
        speed=std::hypot(vx,vz);
        const bool valid=std::isfinite(speed)&&speed>.05f&&speed<=100&&
            field<unsigned>(rider,0x1c8)==serial&&field<unsigned char>(rider,0xf0)==1&&
            field<uintptr_t>(rider,0x214)==unit&&field<uintptr_t>(rider,0x28)==environment&&
            field<uintptr_t>(actor,0x20c)==config&&field<uintptr_t>(actor,0x3b4)==squad;
        if(valid&&verified_actor)*verified_actor=actor;
        return valid;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static float rapid_reference136(uint64_t now){
    float answer=0;AcquireSRWLockExclusive(&rapid_pace_lock136);
    __try {
        auto config=locomotion_config();unsigned uid=field<unsigned>(unit,4),generation=possession_lease.generation;
        float factor=field<float>(unit,0x22c);
        if(rapid_pace136.unit!=unit||rapid_pace136.config!=config||rapid_pace136.unit_id!=uid||
           rapid_pace136.generation!=generation||rapid_pace136.factor!=factor){
            rapid_pace136={};rapid_pace136.unit=unit;rapid_pace136.config=config;
            rapid_pace136.unit_id=uid;rapid_pace136.generation=generation;rapid_pace136.factor=factor;
        }
        InterlockedExchange(&rapid_pace_present136,1);
        if(!rapid_pace136.checked||now-rapid_pace136.checked>=100){
            rapid_pace136.checked=now;++rapid_pace136.refreshes;
            unsigned off=field<unsigned>(unit,0x384)?0x384:0x2bc;
            unsigned count=field<unsigned>(unit,off);auto array=field<uintptr_t>(unit,off+4);
            float samples[9]{};unsigned n=0;
            if(count&&count<=2000&&array){
                unsigned cursor=rapid_pace136.cursor%count;
                for(unsigned scanned=0;scanned<count&&scanned<32&&n<9;++scanned){
                    unsigned i=cursor;cursor=(cursor+1)%count;float value=0;
                    __try {if(rapid_peer_speed136(field<uintptr_t>(array,i*4),value))samples[n++]=value;}
                    __except(EXCEPTION_EXECUTE_HANDLER){}
                }
                rapid_pace136.cursor=cursor;
                if(field<unsigned>(unit,off)!=count||field<uintptr_t>(unit,off+4)!=array)n=0;
            }
            rapid_pace136.samples=n;
            if(n){std::sort(samples,samples+n);rapid_pace136.speed=samples[(n-1)/2];rapid_pace136.observed=now;}
        }
        // Brief stops/one empty slice cannot select the theoretical boosted
        // sprint. Expire observations promptly; the fallback is native walk.
        if(rapid_pace136.observed&&now-rapid_pace136.observed<=1000)answer=rapid_pace136.speed;
    }__finally {ReleaseSRWLockExclusive(&rapid_pace_lock136);}
    return answer;
}
static void clear_rapid_pace136(){
    if(!InterlockedCompareExchange(&rapid_pace_present136,0,0))return;
    AcquireSRWLockExclusive(&rapid_pace_lock136);rapid_pace136={};
    InterlockedExchange(&rapid_pace_present136,0);ReleaseSRWLockExclusive(&rapid_pace_lock136);
}
static float cap_rapid_pace136(unsigned input,float& speed){
    __try {
        if(!rapid_advance_active132()){clear_rapid_pace136();return -1;}
        if(!(input&RUN)||!reinterpret_cast<UnitRunAllowed>(base+0x15cb80)(reinterpret_cast<void*>(unit)))return -1;
        if(!followed(possessed,false))return -1;
        float reference=rapid_reference136(GetTickCount64());
        // Current actor surface/grade still impose their own lower limit. The
        // peer's actual final velocity is a ceiling, not a second stat bonus.
        float fallback=state->native_walk_speed*state->surface_speed_factor*state->terrain_speed_factor;
        float limit=reference>0?reference:fallback;
        if(std::isfinite(limit)&&limit>=0){speed=std::min(speed,limit);return speed;}
    }__except(EXCEPTION_EXECUTE_HANDLER){
        float fallback=state->native_walk_speed*state->surface_speed_factor*state->terrain_speed_factor;
        if(std::isfinite(fallback)&&fallback>=0){speed=std::min(speed,fallback);return speed;}
    }
    return -1;
}
static void rapid_pace_log136(std::ostream& log){
    AcquireSRWLockShared(&rapid_pace_lock136);RapidPace136 p=rapid_pace136;ReleaseSRWLockShared(&rapid_pace_lock136);
    log<<",\"rapid_reference_unit136\":"<<p.unit_id<<",\"rapid_reference_lease136\":"<<p.generation<<",\"rapid_reference_speed136\":"<<p.speed<<",\"rapid_reference_samples136\":"<<p.samples
       <<",\"rapid_reference_tick136\":"<<p.observed<<",\"rapid_reference_refreshes136\":"<<p.refreshes;
}
