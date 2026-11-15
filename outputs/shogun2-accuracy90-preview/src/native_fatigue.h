#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "fatigue_action.h"
using NativeFatigueUpdate=void(__thiscall*)(void*);
using NativeFatigueAction=void(__thiscall*)(void*,int*);
static NativeFatigueUpdate original_fatigue_update;
static NativeFatigueAction original_fatigue_action;
static __declspec(thread) uintptr_t fatigue_owner;
static __declspec(thread) unsigned fatigue_generation;
static bool fatigue_owned(uintptr_t actor){
    __try {return enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED&&
        actor==locomotion_actor()&&field<unsigned char>(actor,0xf0)==1&&followed(possessed,false)&&field<unsigned>(actor,0x1f0)<18;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool native_fatigue_signatures(uintptr_t b){
    const unsigned char update[]={0x83,0xec,0x10,0x53,0x55,0x56,0x8b,0xf1};
    const unsigned char action[]={0x56,0x8b,0xf1,0x57,0x8b,0x46,0x28,0x8b,0x40,8};
    const unsigned char value[]={0x8d,0x41,0x5c,0xc3};
    return !memcmp(reinterpret_cast<void*>(b+0x3f06e0),update,sizeof update)&&
        !memcmp(reinterpret_cast<void*>(b+0x3c73e0),action,sizeof action)&&
        !memcmp(reinterpret_cast<void*>(b+0x1be3a0),value,sizeof value);
}
static void __fastcall fatigue_action_hook(void* object,void*,int* cost){
    original_fatigue_action(object,cost);
    uintptr_t actor=reinterpret_cast<uintptr_t>(object);
    if(fatigue_owner!=actor||fatigue_generation!=possession_lease.generation||!fatigue_owned(actor)||!cost)return;
    __try {
        uintptr_t kv=field<uintptr_t>(actor,0x55c);if(!kv)return;
        // Live UTF-16 record names identify +24c as shooting and +4ec as running.
        int idle=field<int>(kv,0xc+0x5c),walking=field<int>(kv,0x3cc+0x5c),run=field<int>(kv,0x4ec+0x5c);
        if(idle<-100000||idle>100000||walking<-100000||walking>100000||run<-100000||run>100000){++state->fatigue_errors;return;}
        Intent desired=possession_lease.identity.mount?steering_mount(unsigned(state->input),field<unsigned short>(actor,0x64),.1f,10,0):
            steering_view(unsigned(state->input),state->yaw,1,1);
        float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94);
        if(!std::isfinite(vx)||!std::isfinite(vz)){++state->fatigue_errors;return;}
        bool moving=desired.x!=0||desired.z!=0||vx!=0||vz!=0;
        auto speed_getter=reinterpret_cast<UnitSpeed>(base+0x18b470);
        float walk=speed_getter(reinterpret_cast<void*>(unit),0),run_speed=speed_getter(reinterpret_cast<void*>(unit),1);
        if(!std::isfinite(walk)||!std::isfinite(run_speed)){++state->fatigue_errors;return;}
        const unsigned gait_input=personal_travel_input91(unsigned(state->input),
            state->follow!=0,state->aiming||state->aim_pose,possession_lease.identity.mount!=0);
        bool running=native_sprint_requested133(gait_input)&&run_speed>walk;
        bool weapon_active=state->weapon_phase||state->aim_pose||state->aiming||field<float>(actor,0x390)>0;
        int native_cost=*cost;
        // Native actors choose one action. Personal control can combine running
        // and weapon use; retain the more strenuous native cost, rather than
        // charging both callbacks or discarding native weapon/reload fatigue.
        *cost=personal_fatigue_cost(moving,running,weapon_active,native_cost,idle,walking,run);
        state->fatigue_native_action_cost=native_cost;state->fatigue_personal_action_cost=*cost;
        state->fatigue_action_mode=moving?(running?2:1):0;++state->fatigue_action_updates;
    }__except(EXCEPTION_EXECUTE_HANDLER){++state->fatigue_errors;}
}
static void restore_fatigue_grade(uintptr_t actor,const PossessionIdentity& identity,float old_grade,float injected_grade){
    __try {
        // Restore after F10/focus loss/disable too, if the native object survives.
        if(field<uintptr_t>(field<uintptr_t>(base,0x1bc8180),0x110)==identity.environment&&
            actor152::kind(base,field<uintptr_t>(actor,0),identity.mount?1:0)&&
            actor==(identity.mount?identity.mount:identity.actor)&&
            field<unsigned>(actor,0x1c8)==(identity.mount?identity.mount_id:identity.actor_id)&&
            field<uintptr_t>(actor,0x214)==identity.unit&&field<unsigned>(identity.unit,4)==identity.unit_id&&
            field<float>(actor,0x1b8)==injected_grade){field<float>(actor,0x1b8)=old_grade;++state->fatigue_grade_restores;}
    }__except(EXCEPTION_EXECUTE_HANDLER){++state->fatigue_errors;}
}
static void __fastcall fatigue_update_hook(void* object,void*){
    uintptr_t actor=reinterpret_cast<uintptr_t>(object);
    if(!fatigue_owned(actor)){original_fatigue_update(object);return;}
    uintptr_t prior_owner=fatigue_owner;unsigned prior_generation=fatigue_generation;
    PossessionIdentity identity=possession_lease.identity;
    float old_grade=0,injected_grade=0;bool grade_saved=false;
    __try {
        __try {
            Intent direction=identity.mount?steering_mount(unsigned(state->input),field<unsigned short>(actor,0x64),.1f,10,0):
                steering_view(unsigned(state->input),state->yaw,1,1);
            if(direction.x==0&&direction.z==0){
                float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94),length=std::hypot(vx,vz);
                if(std::isfinite(length)&&length>0){direction.x=vx/length;direction.z=vz/length;}
            }
            float grade=0,factor=1,distance=0,dt=field<float>(base,0x15b2448);
            if(!std::isfinite(dt)||dt<=0||dt>.2f||!sample_native_grade(actor,direction,dt,grade,factor,distance)){
                ++state->fatigue_errors;
            }else {
                old_grade=field<float>(actor,0x1b8);grade_saved=true;
                injected_grade=grade;
                field<float>(actor,0x1b8)=grade;state->fatigue_personal_grade=grade;
                fatigue_owner=actor;fatigue_generation=possession_lease.generation;
                state->fatigue_before=field<int>(actor,0x39c);
            }
        }__except(EXCEPTION_EXECUTE_HANDLER){++state->fatigue_errors;}
        // The original applies immunity, directional incline multiplier, unit
        // resistance, weather, campaign effects, thresholds and clamping once.
        original_fatigue_update(object);
        if(grade_saved&&fatigue_owned(actor)){
            state->fatigue_after=field<int>(actor,0x39c);++state->fatigue_updates;
        }
    }__finally {
        fatigue_owner=prior_owner;fatigue_generation=prior_generation;
        if(grade_saved)restore_fatigue_grade(actor,identity,old_grade,injected_grade);
    }
}
static void create_fatigue_hooks(){
    require(native_fatigue_signatures(base),"Native fatigue signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3f06e0),reinterpret_cast<void*>(&fatigue_update_hook),
        reinterpret_cast<void**>(&original_fatigue_update))==MH_OK,"Fatigue update hook creation failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3c73e0),reinterpret_cast<void*>(&fatigue_action_hook),
        reinterpret_cast<void**>(&original_fatigue_action))==MH_OK,"Fatigue action hook creation failed");
}
