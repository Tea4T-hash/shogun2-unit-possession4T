#pragma once
// Both original consumers already have a native target UNIT. Include its real
// registered independent group without changing ownership, orders or AI goals.
static bool acquisition_enemy203(uintptr_t observer,uintptr_t actor){
    if(!observer||observer==actor||field<unsigned char>(observer,0xf0)!=1||field<uintptr_t>(observer,0x28)!=environment||field<uintptr_t>(observer,0x214)==unit)return false;
    auto a=field<uintptr_t>(observer,0x44),b=field<uintptr_t>(actor,0x44);
    return a&&b&&field<unsigned>(a,0x10)<=31&&field<unsigned>(b,0x10)<=31&&field<unsigned>(a,0x10)!=field<unsigned>(b,0x10);
}
static uintptr_t acquisition_group203(uintptr_t context,uintptr_t original_group){
    if(!context||!unit||field<uintptr_t>(context,0x24)!=unit||field<uintptr_t>(context,0x34))return original_group;
    auto actor=locomotion_actor();if(!member_owned130(actor)||!proximity_independent200(actor))return original_group;
    auto independent=field<uintptr_t>(actor,0x3b4),observer_group=field<uintptr_t>(context,4);
    if(!observer_group||original_group==independent||!squad_registered130(original_group,unit,environment))return original_group;
    auto observer_unit=field<uintptr_t>(observer_group,0x510);
    if(!observer_unit||observer_unit==unit||field<uintptr_t>(observer_unit,0x14)!=environment||!squad_registered130(observer_group,observer_unit,environment))return original_group;
    auto count=field<unsigned>(observer_group,0x18),members=field<uintptr_t>(observer_group,0x1c);
    if(!count||count>1000||!members||!acquisition_enemy203(field<uintptr_t>(members,0),actor))return original_group;
    float old_distance=0,new_distance=0;
    for(unsigned offset:{0x644u,0x648u}){
        float origin=field<float>(observer_group,offset),old=field<float>(original_group,offset),candidate=field<float>(independent,offset);
        if(!std::isfinite(origin)||!std::isfinite(old)||!std::isfinite(candidate))return original_group;
        old_distance+=(old-origin)*(old-origin);new_distance+=(candidate-origin)*(candidate-origin);
    }
    // Original downstream visibility, range, terrain and projectile tests run
    // on this current registered group's original native geometry.
    return new_distance<old_distance?independent:original_group;
}
using ChargePick203=uintptr_t(__thiscall*)(void*,uintptr_t,const float*,float);
static ChargePick203 original_charge_pick203;
static uintptr_t __fastcall charge_pick203(void* context,void*,uintptr_t observer,const float* forward,float width){
    auto result=original_charge_pick203(context,observer,forward,width);
    __try{
        if(!unit||field<uintptr_t>(reinterpret_cast<uintptr_t>(context),0x18)!=unit)return result;
        auto actor=locomotion_actor();if(!member_owned130(actor)||!proximity_independent200(actor)||!acquisition_enemy203(observer,actor)||!forward)return result;
        // Exact174ec0 corridor and nearest-distance rule; preserve native ties.
        float dx=field<float>(actor,0x48)-field<float>(observer,0x48),dz=field<float>(actor,0x50)-field<float>(observer,0x50);
        float cross=forward[1]*dx-dz*forward[0],distance=dx*dx+dz*dz;
        if(!std::isfinite(cross)||!std::isfinite(distance)||!std::isfinite(width)||std::fabs(cross)>=width)return result;
        if(result){float x=field<float>(result,0x48)-field<float>(observer,0x48),z=field<float>(result,0x50)-field<float>(observer,0x50);if(!(distance<x*x+z*z))return result;}
        return actor;
    }__except(EXCEPTION_EXECUTE_HANDLER){return result;}
}
#include "acquisition207.h"
static void create_acquisition203(){
    create_acquisition207();
    const unsigned char first[]={0x8b,0x41,0x18,0xf3,0x0f,0x10,0x25};
    const unsigned char tail[]={0x56,0x8b,0x90,0x9c,0x0b,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x174ec0),first,sizeof first)&&field<uintptr_t>(base+0x174ec7,0)==base+0x15c2764&&!memcmp(reinterpret_cast<void*>(base+0x174ecb),tail,sizeof tail),"Native charge candidate selector differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x174ec0),reinterpret_cast<void*>(&charge_pick203),reinterpret_cast<void**>(&original_charge_pick203))==MH_OK,"Native independent charge candidate hook failed");
}
