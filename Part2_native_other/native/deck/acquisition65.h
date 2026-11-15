#pragma once
#include <intrin.h>

// Three stock ranged pairing paths enumerate only the selected unit's main
// group. Append the detached physical actor through the same enumeration.
using RangedGroup65=void(__thiscall*)(void*,void*);
static RangedGroup65 original_ranged_group65;
static constexpr unsigned ranged_sites65[]={0x102bff,0x102ccf,0x102daf};
static bool ranged_builder65(uintptr_t caller){
    for(const auto call:ranged_sites65)
        if(caller==base+call+5)return true;
    return false;
}
static void append_ranged_group65(uintptr_t group,void* vector){
    if(!unit||!possessed||!group||!vector)return;
    const bool take_lock=!member_sync_active130;
    if(take_lock)AcquireSRWLockShared(&member_lock130);
    __try{
        __try{
            auto actor=locomotion_actor();
            if(!member_owned130(actor)||field<uintptr_t>(unit,0xb9c)!=group||
               !squad_registered130(group,unit,environment)||squad_matches130(group,actor)!=0)return;
            auto independent=independent_group_locked65(actor);
            if(independent&&independent!=group&&field<unsigned>(independent,0x18)==1)
                original_ranged_group65(reinterpret_cast<void*>(independent),vector);
        }__except(EXCEPTION_EXECUTE_HANDLER){}
    }__finally{if(take_lock)ReleaseSRWLockShared(&member_lock130);}
}
static void __fastcall ranged_group65(void* group,void*,void* vector){
    const auto caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
    InterlockedIncrement(&motion_callback_depth);
    __try{
        original_ranged_group65(group,vector);
        if(unit&&vector&&ranged_builder65(caller))
            append_ranged_group65(reinterpret_cast<uintptr_t>(group),vector);
    }__finally{InterlockedDecrement(&motion_callback_depth);}
}
static void create_ranged_group65(){
    const unsigned char signature[]={0x56,0x57,0x8b,0xf9,0x8b,0x77,0x1c};
    require(!memcmp(reinterpret_cast<void*>(base+0x154140),signature,sizeof signature),"Native ranged target enumeration differs");
    for(const auto call:ranged_sites65)
        require(field<unsigned char>(base,call)==0xe8&&
                call+5+field<int>(base,call+1)==0x154140,"Native ranged builder call differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x154140),reinterpret_cast<void*>(&ranged_group65),
            reinterpret_cast<void**>(&original_ranged_group65))==MH_OK,"Native ranged candidate enumeration hook failed");
}
