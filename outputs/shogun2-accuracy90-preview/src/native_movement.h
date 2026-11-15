#pragma once
#include "rapid_advance132.h"
// Verified engine functions use ECX=the actor's native unit and x87 float returns.
using NativeIntent=void(__thiscall*)(void*,const float*,unsigned);
using UnitSpeed= float(__thiscall*)(void*,unsigned);
using UnitRunAllowed=bool(__thiscall*)(void*);
static bool native_routing(uintptr_t actor_unit){
    // unit:is_routing binding RVA1c1bd0 calls this on unit+cc4.
    auto predicate=reinterpret_cast<UnitRunAllowed>(base+0x1821e0);
    return predicate(reinterpret_cast<void*>(actor_unit+0xcc4));
}
static bool native_movement_signatures(uintptr_t b){
    const unsigned char intent[]={0x83,0x79,0x18,0,0x75,0x60,0x8b,0x44,0x24,4};
    const unsigned char walk[]={0x8b,0x81,0x9c,0x0f,0,0,0xd9,0x40,0x1c};
    const unsigned char run[]={0x8b,0x81,0x9c,0x0f,0,0,0xd9,0x40,0x20};
    const unsigned char wrapper[]={0x80,0x7c,0x24,4,0,0x56,0x8b,0xf1};
    const unsigned char gait[]={0x56,0x8b,0xf1,0xe8,0x58,0x0a,0xfd,0xff,0x84,0xc0};
    const unsigned char routing[]={0x8b,0x01,0x8b,0x40,8,0x8b,0x80,0xb4,0,0,0};
    return !memcmp(reinterpret_cast<void*>(b+0x3936f0),intent,sizeof intent)&&
        !memcmp(reinterpret_cast<void*>(b+0x1be960),walk,sizeof walk)&&
        !memcmp(reinterpret_cast<void*>(b+0x1a9da0),run,sizeof run)&&
        !memcmp(reinterpret_cast<void*>(b+0x18b470),wrapper,sizeof wrapper)&&
        !memcmp(reinterpret_cast<void*>(b+0x1821e0),routing,sizeof routing)&&
        !memcmp(reinterpret_cast<void*>(b+0x18c120),gait,sizeof gait);
}
static bool native_sprint_requested133(unsigned input){return (input&RUN)!=0;}
static bool squad_registered130(uintptr_t squad,uintptr_t native_unit,uintptr_t env);
static void match_formation_pace72(bool running,float& walk,float& speed){
    // The native main-squad planner uses 18b310(gait, false) with the
    // current formation pace factors. The player's current squad may be
    // independent. Rapid Advance already has a separate peer-speed rule.
    __try{
        if(rapid_advance_active132())return;
        const auto squad=field<uintptr_t>(unit,0xb9c);
        if(!squad_registered130(squad,unit,environment))return;
        const auto count=field<unsigned>(squad,0x18);
        const auto members=field<uintptr_t>(squad,0x1c);
        if(!count||count>1000||!members)return;
        const auto lead=field<uintptr_t>(members,0);
        if(!lead||field<uintptr_t>(lead,0x214)!=unit||field<uintptr_t>(lead,0x28)!=environment||
           field<uintptr_t>(lead,0x3b4)!=squad||!field<uintptr_t>(lead,0x20c))return;
        using SquadSpeed=float(__thiscall*)(void*,unsigned,unsigned);
        const auto pace=reinterpret_cast<SquadSpeed>(base+0x18b310);
        const float next_walk=pace(reinterpret_cast<void*>(squad),0,0);
        const float next_speed=running?pace(reinterpret_cast<void*>(squad),1,0):next_walk;
        if(next_walk>0&&next_walk<=100&&next_speed>0&&next_speed<=100){
            walk=next_walk;speed=next_speed;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}
static bool read_native_movement(unsigned input,float& speed,float& turn_rate){
    __try {
        if(!followed(possessed,false))return false;
        // Native squad18b310 uses the actual entity configuration, the unit's
        // live22c multiplier, and the stock global walk/run coefficients. The
        // unit-only18b470 getters omit that last factor and use unit aggregate
        // configf9c. The current main formation supplies the two pace factors
        // when usable. Shift still selects the gait independently of orders.
        bool running=native_sprint_requested133(input)&&reinterpret_cast<UnitRunAllowed>(base+0x15cb80)(reinterpret_cast<void*>(unit));
        auto config=locomotion_config();float factor=field<float>(unit,0x22c);
        float walk_factor=field<float>(base,0x18d1c80+0x5c),run_factor=field<float>(base,0x18d1ce0+0x5c);
        float raw_walk=field<float>(config,0x1c),raw_run=field<float>(config,0x20);
        for(float value:{factor,walk_factor,run_factor,raw_walk,raw_run})if(!std::isfinite(value)||value<0||value>100)return false;
        float walk=float(raw_walk*factor)*walk_factor;
        speed=running?float(raw_run*factor)*run_factor:walk;
        if(!std::isfinite(walk)||!std::isfinite(speed)||walk<0||speed<0||walk>100||speed>100)return false;
        match_formation_pace72(running,walk,speed);
        // Exact native UTF-16 entity keys and DB values were read back from the
        // current infantry. Config+6c stores turn_speed converted to radians/s.
        // Read this soldier's configuration, never substitute another unit's rig.
        turn_rate=field<float>(locomotion_config(),0x6c);
        if(!std::isfinite(turn_rate)||turn_rate<0||turn_rate>100)return false;
        state->native_walk_speed=walk;state->effective_speed=speed;
        state->native_movement_factor=field<float>(unit,0x22c);
        state->native_turn_rate=turn_rate;
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}

#include "rapid_pace136.h"

static void match_formation_run91(uintptr_t actor,unsigned input,Intent direction,float& speed){
    // Follow Me uses the native run target to lead the formation. Reading
    // back its followers here would feed the old slower pace into that order.
    if(state->follow||!(input&RUN)||(!direction.x&&!direction.z))return;
    __try{
        if(rapid_advance_active132()||
           !reinterpret_cast<UnitRunAllowed>(base+0x15cb80)(reinterpret_cast<void*>(unit)))return;
        const auto main=field<uintptr_t>(unit,0xb9c);
        if(!squad_registered130(main,unit,environment))return;
        const float radius=field<float>(main,0x670);
        if(!std::isfinite(radius)||radius<=0||radius>1024)return;
        const auto count=field<unsigned>(unit,0x2bc);
        const auto roster=field<uintptr_t>(unit,0x2c0);
        if(!roster||count<3||count>2000)return;
        const float x=field<float>(actor,0x48),z=field<float>(actor,0x50);
        const auto surface=field<uintptr_t>(actor,0x10);
        // 3da2a0 shows per-soldier running adjustments beyond 18b310.
        // Compare actual planar travel of nearby, co-directional runners on
        // matching ground. Local grade, braking and collision stay native.
        float paces[32]{};
        unsigned valid=0;
        const unsigned probes=std::min(count,32u);
        for(unsigned i=0;i<probes;++i){
            const auto rider=field<uintptr_t>(roster,(i*count/probes)*sizeof(uintptr_t));
            uintptr_t peer{};
            float observed{};
            if(!rapid_peer_speed136(rider,observed,&peer)||
               field<uintptr_t>(peer,0x3b4)!=main||field<unsigned>(peer,0x244)!=1||
               field<uintptr_t>(peer,0x688)||field<uintptr_t>(peer,0x18)||
               field<uintptr_t>(peer,0x10)!=surface||field<unsigned>(peer,0x1b4)!=state->ground_type)continue;
            const float dx=field<float>(peer,0x48)-x,dz=field<float>(peer,0x50)-z;
            const float along=field<float>(peer,0x8c)*direction.x+field<float>(peer,0x94)*direction.z;
            const float grade=field<float>(peer,0x1b8)-state->terrain_grade;
            if(!(dx*dx+dz*dz<=radius*radius)||!(std::fabs(grade)<=.05f)||
               !(along>=observed*.9f))continue;
            paces[valid++]=observed;
        }
        if(valid<3||field<unsigned>(unit,0x2bc)!=count||field<uintptr_t>(unit,0x2c0)!=roster||
           field<uintptr_t>(unit,0xb9c)!=main)return;
        std::sort(paces,paces+valid);
        speed=paces[valid/2];
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}

static bool issue_native_intent(uintptr_t actor,const Intent& intent,unsigned bearing){
    float x=field<float>(actor,0x48),z=field<float>(actor,0x50);
    float target[2]={x+intent.x,z+intent.z};
    if(!std::isfinite(x)||!std::isfinite(z)||!std::isfinite(target[0])||!std::isfinite(target[1]))return false;
    unsigned heading=static_cast<unsigned short>(bearing+intent.turn);
    // Actual native API, not a copied solver: it establishes horizontal intent,
    // yaw delta, magnitude and the animation-drive flag. The original integrator
    // and caller still own collisions, terrain, actual velocity and animation.
    reinterpret_cast<NativeIntent>(base+0x3936f0)(reinterpret_cast<void*>(actor),target,heading);
    state->native_intent_flag=field<unsigned char>(actor,0x198);
    state->native_intent_magnitude=field<float>(actor,0x150);
    state->native_intent_x=field<float>(actor,0xd8);state->native_intent_z=field<float>(actor,0xe0);
    if(state->native_intent_flag!=1||!std::isfinite(state->native_intent_magnitude)||
        !std::isfinite(state->native_intent_x)||!std::isfinite(state->native_intent_z))return false;
    ++state->native_intent_calls;return true;
}
