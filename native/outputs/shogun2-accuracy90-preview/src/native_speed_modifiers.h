#pragma once
#include "movement_math.h"
using NativeUnitPredicate=bool(__thiscall*)(void*);
using NativeFatigueRow=uintptr_t(__thiscall*)(void*);
static bool native_speed_modifier_signatures(uintptr_t b){
    const unsigned char fatigue[]={0x56,0x57,0x8b,0xf1,0xe8,0xc7,0x8d,1,0};
    const unsigned char category[]={0x8b,0x41,0x2c,0x83,0x78,0x1c,1};
    const unsigned char infantry[]={0x56,0x8b,0xf1,0xe8,0xf8,0x16,0,0};
    const unsigned char vehicle[]={0x8b,0x41,0x2c,0x8b,0x40,0x1c,0x83,0xf8,2};
    const unsigned char acceleration[]={0x0f,0x2f,0xda,0xf3,0x0f,0x10,0x2d};
    return !memcmp(reinterpret_cast<void*>(b+0x3d1b90),fatigue,sizeof fatigue)&&
        !memcmp(reinterpret_cast<void*>(b+0x180440),category,sizeof category)&&
        !memcmp(reinterpret_cast<void*>(b+0x1804a0),infantry,sizeof infantry)&&
        !memcmp(reinterpret_cast<void*>(b+0x181ba0),vehicle,sizeof vehicle)&&
        !memcmp(reinterpret_cast<void*>(b+0x3aa718),acceleration,sizeof acceleration);
}
static bool read_native_speed_modifiers(uintptr_t actor,unsigned input,float base_speed,float& factor){
    __try {
        uintptr_t common=field<uintptr_t>(environment,8);
        if(common!=possession_lease.identity.common||field<uintptr_t>(actor,0x214)!=unit)return false;
        uintptr_t terrain=field<uintptr_t>(common,0xa4),surface=field<uintptr_t>(actor,0x10);
        unsigned type=0;
        if(surface)type=field<unsigned>(surface,0x60);
        else {
            float scale=field<float>(terrain,0x24),x=field<float>(actor,0x48),z=field<float>(actor,0x50);
            unsigned shift=field<unsigned>(terrain,8),origin=field<unsigned>(terrain,0x10),last=field<unsigned>(terrain,0x14);
            if(!std::isfinite(scale)||scale<=0||!std::isfinite(x)||!std::isfinite(z)||
                std::abs(double(x)*scale)>1000000||std::abs(double(z)*scale)>1000000||
                shift>16||last>=(1u<<shift)||origin>last)return false;
            unsigned cx=native_ground_cell(x,scale,origin,last),cz=native_ground_cell(z,scale,origin,last);
            uintptr_t cells=field<uintptr_t>(terrain,0);
            type=field<unsigned>(cells,((cx<<shift)+cz)*4)&0xffu;
        }
        factor=1;state->surface_speed_factor=1;state->fatigue_speed_factor=1;state->ground_type=type;
        if(!field<unsigned char>(unit,0x19c)&&type<12){
            uintptr_t table=field<uintptr_t>(terrain,0x44),row=field<uintptr_t>(table,type*4);
            if(row){
                auto mounted=reinterpret_cast<NativeUnitPredicate>(base+0x180440);
                auto infantry=reinterpret_cast<NativeUnitPredicate>(base+0x1804a0);
                auto vehicle=reinterpret_cast<NativeUnitPredicate>(base+0x181ba0);
                bool m=mounted(reinterpret_cast<void*>(unit));
                unsigned subtype=m?field<unsigned>(field<uintptr_t>(unit,0x2c),0x20):0;
                bool i=!m&&infantry(reinterpret_cast<void*>(unit));
                bool v=!m&&!i&&vehicle(reinterpret_cast<void*>(unit));
                unsigned column=native_surface_column(m,subtype,i,v);
                if(column)state->surface_speed_factor=field<float>(row,column);
            }
        }
        auto fatigue=reinterpret_cast<NativeFatigueRow>(base+0x3d1b90);
        uintptr_t fatigue_row=fatigue(reinterpret_cast<void*>(actor));
        // Match manual Shift, including the ability-scoped final pace ceiling.
        // A rejected native run request has no run penalty.
        if(fatigue_row&&native_sprint_requested133(input)&&base_speed>state->native_walk_speed)
            state->fatigue_speed_factor=field<float>(fatigue_row,0x10);
        float surface_factor=state->surface_speed_factor,fatigue_factor=state->fatigue_speed_factor;
        if(!std::isfinite(surface_factor)||!std::isfinite(fatigue_factor)||surface_factor<0||
            surface_factor>10||fatigue_factor<0||fatigue_factor>10)return false;
        factor=surface_factor*fatigue_factor;++state->speed_modifier_queries;return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool apply_native_speed_step(uintptr_t actor,Intent direction,bool requested,float dt,float factor,float& speed,bool resumed96=false){
    __try {
        float previous=field<float>(actor,0x1ac),acceleration=field<float>(actor,0x160),deceleration=field<float>(actor,0x104);
        float negative_dt=field<float>(base,0x15de874);
        if(!std::isfinite(previous)||!std::isfinite(acceleration)||!std::isfinite(deceleration)||
            !std::isfinite(negative_dt)||previous<0||previous>100||acceleration<0||acceleration>100||
            deceleration<0||deceleration>100||negative_dt>=0||negative_dt<-.2f)return false;
        // AI +110 may still be the formation's walking cap while manual Shift
        // requests this unit's native run speed. Use completed actual speed here
        // so that the AI cap cannot reset manual running to walking every tick.
        if(resumed96)previous=0;
        else if(requested&&!possession_lease.identity.mount){
            const float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94);
            if(!std::isfinite(vx)||!std::isfinite(vz))return false;
            previous=directed_speed95(previous,vx,vz,direction.x,direction.z);
        }
        float target=requested?speed:0;
        state->requested_speed=target;state->previous_motion_speed=previous;
        speed=native_speed_step(previous,target,acceleration*factor,deceleration,dt,negative_dt);
        if(!std::isfinite(speed)||speed<0||speed>100)return false;
        // A stationary actor has no braking direction. Never invent one.
        if(direction.x==0&&direction.z==0)speed=0;
        state->effective_speed=speed;state->native_acceleration=acceleration;
        state->native_deceleration=deceleration;++state->speed_step_updates;return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
