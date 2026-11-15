#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
// Native primary SQUAD membership is actor+3b4, separate from unit+214.
// Use native removal/insertion; never alter unit ownership, physical unit
// roster, secondary mount/passenger groups (+3b8), camera or actor serials.
using UnitTick130=void(__thiscall*)(void*);
using MemberInsert130=void(__thiscall*)(void*,uintptr_t,unsigned);
using MemberRemove130=void(__thiscall*)(void*,uintptr_t);
static UnitTick130 original_unit_tick130;
static MemberInsert130 original_member_insert130,original_member_transfer130;
static void* original_regroup_gate130;
static uintptr_t regroup_skip130;
static SRWLOCK member_lock130=SRWLOCK_INIT;
static thread_local bool member_sync_active130;
struct Detached130 {
    uintptr_t environment{},phase{},unit{},actor{},config{},preferred{};
    unsigned unit_id{},actor_id{},generation{};
};
static Detached130 detached130[128];
// Numeric negative filter only: a hit grants no identity or mutation authority.
static volatile LONG detached_buckets130[256],detached_proofs130;
static unsigned detached_bucket130(uintptr_t actor){return unsigned((actor>>4)^(actor>>13))&255u;}
static volatile LONG detached_count130,member_detaches130,member_rejoins130,member_denials130,member_errors130;
static bool member_owned130(uintptr_t actor){
    __try{
        if(!actor||!possessed||!possession_lease.active)return false;
        if(actor!=possessed&&actor!=possession_lease.identity.mount)return false;
        if(dismount_native_motion143())return false;
        return enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED&&
            GetTickCount64()-state->heartbeat<=500&&!dead_view()&&field<unsigned char>(possessed,0xf0)==1&&
            followed(possessed,false,false);
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool squad_owner130(uintptr_t squad,uintptr_t native_unit,uintptr_t env){
    return squad&&field<uintptr_t>(squad,0)==base+0x15bb21c&&
        field<uintptr_t>(squad,0x510)==native_unit&&field<uintptr_t>(squad,4)==env;
}
static bool squad_registered130(uintptr_t squad,uintptr_t native_unit,uintptr_t env){
    if(!squad)return false;bool found=false;
    for(unsigned off:{0xb64u,0xb80u}){
        unsigned count=field<unsigned>(native_unit,off+0xc);auto array=field<uintptr_t>(native_unit,off+0x10);
        if(count>1000||(!array&&count))return false;
        for(unsigned i=0;i<count;++i)if(field<uintptr_t>(array,i*4)==squad)found=true;
    }
    // Dereference a former squad only after the native owner's current lists
    // prove it is live. No saved squad address grants restoration authority.
    return found&&squad_owner130(squad,native_unit,env);
}
static unsigned squad_matches130(uintptr_t squad,uintptr_t actor){
    auto n=field<unsigned>(squad,0x18);auto a=field<uintptr_t>(squad,0x1c);
    if(n>1000||(!a&&n))return ~0u;unsigned matches=0;
    for(unsigned i=0;i<n;++i)if(field<uintptr_t>(a,i*4)==actor)++matches;
    return matches;
}
static bool unit_member130(uintptr_t native_unit,uintptr_t actor){
    // Original unit entity lists remain intact during detachment. Restoration
    // reads actor fields only after an engine-supplied current unit lists it.
    for(unsigned off:{0x384u,0x2bcu}){
        unsigned n=field<unsigned>(native_unit,off);auto a=field<uintptr_t>(native_unit,off+4);
        if(n>2000||(!a&&n))return false;
        for(unsigned i=0;i<n;++i){auto p=field<uintptr_t>(a,i*4);if(p==actor)return true;}
    }
    // A mount may be represented through its live rider in the master roster.
    for(unsigned off:{0x384u,0x2bcu}){
        unsigned n=field<unsigned>(native_unit,off);auto a=field<uintptr_t>(native_unit,off+4);
        for(unsigned i=0;i<n;++i){auto p=field<uintptr_t>(a,i*4);
            if(p&&actor152::human(base,field<uintptr_t>(p,0))&&field<uintptr_t>(p,0x210)==actor)return true;}
    }
    return false;
}
static bool deny_member_insert130(uintptr_t squad,uintptr_t actor){
    __try{if(proximity_managed147(actor))return proximity_deny147(squad,actor);return member_owned130(actor)&&!charge_member146(actor)&&squad_owner130(squad,unit,environment);}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static void __fastcall member_insert130(void* squad,void*,uintptr_t actor,unsigned clear){
    if(deny_member_insert130(reinterpret_cast<uintptr_t>(squad),actor)){InterlockedIncrement(&member_denials130);return;}
    original_member_insert130(squad,actor,clear);
}
static void __fastcall member_transfer130(void* squad,void*,uintptr_t actor,unsigned clear){
    // Reject before transfer removes the old membership, not only at insertion.
    if(deny_member_insert130(reinterpret_cast<uintptr_t>(squad),actor)){InterlockedIncrement(&member_denials130);return;}
    original_member_transfer130(squad,actor,clear);
}
static int __cdecl skip_regroup_member130(uintptr_t actor){return member_owned130(actor)&&!charge_member146(actor)&&!proximity_managed147(actor)?1:0;}
static __declspec(naked) void regroup_gate130(){
    __asm{
        pushfd
        pushad
        push eax
        call skip_regroup_member130
        add esp,4
        test eax,eax
        jz native_member
        popad
        popfd
        jmp dword ptr [regroup_skip130]
    native_member:
        popad
        popfd
        jmp dword ptr [original_regroup_gate130]
    }
}
static void forget_member130(Detached130& r){InterlockedDecrement(&detached_buckets130[detached_bucket130(r.actor)]);r={};InterlockedDecrement(&detached_count130);}
// Original 3e1780 -> 3da190 -> 3dbd70 gives each member its own slot
// request. 14bc50 only restores membership and clears the old task. The
// actor's native update consumes +648 only when its +665 flag is clear.
static void reissue_officer_slot68(uintptr_t actor,uintptr_t squad,uintptr_t native_unit,uintptr_t env){
    if(field<uintptr_t>(actor,0)!=base+0x15dbfb4||field<uintptr_t>(actor,0x210)||
       field<uintptr_t>(actor,0x18)||field<uintptr_t>(actor,0x688)||
       native_recovery103(actor)||wall_subject97(actor)||native_airborne_subject99(actor)||
       field<uintptr_t>(actor,0x28)!=env||native_routing(native_unit)||
       field<unsigned char>(actor,0x384)||field<unsigned char>(actor,0x665)!=1||
       !squad_registered130(squad,native_unit,env)||field<uintptr_t>(actor,0x3b4)!=squad||
       squad_matches130(squad,actor)!=1)return;
    if(field<unsigned>(actor,0x658))return; // Absolute land slot only.
    const float x=field<float>(actor,0x648),y=field<float>(actor,0x64c),z=field<float>(actor,0x650);
    const float dx=x-field<float>(squad,0x644),dz=z-field<float>(squad,0x648);
    const float radius=field<float>(squad,0x670);
    const float away_x=x-field<float>(actor,0x48),away_z=z-field<float>(actor,0x50);
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)||!std::isfinite(radius)||radius<0||
       (x==0&&y==0&&z==0)||!(dx*dx+dz*dz<=(radius+4.f)*(radius+4.f))||
       !(away_x*away_x+away_z*away_z>.0625f))return;
    // The existing position and heading are still valid. Arm their two
    // native request flags as 3dbd70 does for a freshly issued slot order.
    // The normal actor update handles the actual route and gait.
    field<unsigned char>(actor,0x664)=0;
    field<unsigned char>(actor,0x665)=0;
}
static bool restore_member130(const Detached130& r,uintptr_t current_unit){
    if(field<unsigned>(current_unit,4)!=r.unit_id||field<uintptr_t>(current_unit,0x14)!=r.environment||
       !unit_member130(current_unit,r.actor))return true;
    auto actor=r.actor;
    if(field<unsigned>(actor,0x1c8)!=r.actor_id||field<uintptr_t>(actor,0x20c)!=r.config||
       field<uintptr_t>(actor,0x214)!=current_unit||field<unsigned char>(actor,0xf0)!=1)return true;
    auto charge_target=charge_squad147(actor);if(!charge_target)charge_target=proximity_squad147(actor);
    // A native mounted rider can start with no primary membership. Recording
    // that state must not invent a rider slot in the horse formation on exit.
    if(!r.preferred&&!charge_target&&!field<uintptr_t>(actor,0x3b4))return true;
    if(field<uintptr_t>(actor,0x3b4)){
        if(!charge_target||field<uintptr_t>(actor,0x3b4)==charge_target){
            reissue_officer_slot68(actor,field<uintptr_t>(actor,0x3b4),current_unit,r.environment);
            return true;
        }
        original_member_transfer130(reinterpret_cast<void*>(charge_target),actor,1);
        return field<uintptr_t>(actor,0x3b4)==charge_target&&squad_matches130(charge_target,actor)==1;
    }
    auto target=charge_target?charge_target:r.preferred;
    if(!squad_registered130(target,current_unit,r.environment))target=field<uintptr_t>(current_unit,0xb9c);
    if(!squad_registered130(target,current_unit,r.environment))return false;
    original_member_insert130(reinterpret_cast<void*>(target),actor,1);
    if(field<uintptr_t>(actor,0x3b4)!=target||squad_matches130(target,actor)!=1)return false;
    reissue_officer_slot68(actor,target,current_unit,r.environment);
    InterlockedIncrement(&member_rejoins130);return true;
}
static void detach_member130(uintptr_t actor,const PossessionIdentity& id,unsigned generation){
    if(!actor||!member_owned130(actor)||charge_member146(actor)||proximity_managed147(actor)||!unit_member130(id.unit,actor))return;
    Detached130* slot=nullptr;
    for(auto& r:detached130){if(r.actor==actor&&r.unit==id.unit&&r.actor_id==field<unsigned>(actor,0x1c8)){slot=&r;break;}if(!r.actor&&!slot)slot=&r;}
    if(!slot){InterlockedIncrement(&member_errors130);InterlockedExchange(&state->release,1);return;}
    auto squad=field<uintptr_t>(actor,0x3b4);
    if(squad&&(!squad_registered130(squad,id.unit,id.environment)||squad_matches130(squad,actor)!=1))return;
    if(!slot->actor){*slot={id.environment,id.phase,id.unit,actor,field<uintptr_t>(actor,0x20c),squad,id.unit_id,field<unsigned>(actor,0x1c8),generation};InterlockedIncrement(&detached_buckets130[detached_bucket130(actor)]);InterlockedIncrement(&detached_count130);}
    else slot->generation=generation;
    if(!squad)return;
    if(!member_owned130(actor)||possession_lease.generation!=generation||!same_soldier(id,possession_lease.identity))return;
    // Retire only the exact native formation-move task. Combat, weapon,
    // recovery, climb and mount/passenger tasks are not cancelled here.
    if(!field<uintptr_t>(actor,0x18)&&!field<uintptr_t>(actor,0x688)&&field<unsigned char>(actor,0x384)==1&&
       field<uintptr_t>(actor,0x2f8)==base+0x15dd7ac)
        reinterpret_cast<UnitTick130>(base+0x3ca8f0)(reinterpret_cast<void*>(actor+0x2f0));
    reinterpret_cast<MemberRemove130>(base+0x1a80a0)(reinterpret_cast<void*>(squad),actor);
    if(field<uintptr_t>(actor,0x3b4)||squad_matches130(squad,actor)!=0){InterlockedIncrement(&member_errors130);InterlockedExchange(&state->release,1);return;}
    InterlockedIncrement(&member_detaches130);
}
#include "proximity147.h"
static void sync_members130(uintptr_t current_unit){
    AcquireSRWLockExclusive(&member_lock130);member_sync_active130=true;
    __try{
        __try{
            auto root=field<uintptr_t>(base,0x1bc8180),env=root?field<uintptr_t>(root,0x110):0;
            auto common=env?field<uintptr_t>(env,8):0,phase=common?field<uintptr_t>(common,0xb4):0;
            unsigned phase_id=phase?field<unsigned>(phase,0x14):0;
            proximity_sync147(current_unit,env,phase,phase_id);
            for(auto& r:detached130)if(r.actor&&(r.environment!=env||r.phase!=phase||phase_id<5||phase_id>8))forget_member130(r);
            for(auto& r:detached130)if(r.actor&&r.unit==current_unit&&(!member_owned130(r.actor)||charge_member146(r.actor)||proximity_managed147(r.actor)))
                if(restore_member130(r,current_unit))forget_member130(r);
            if(current_unit==unit&&member_owned130(possessed)){
                auto id=possession_lease.identity;unsigned generation=possession_lease.generation;
                if(id.unit==current_unit&&field<unsigned>(current_unit,4)==id.unit_id&&field<uintptr_t>(current_unit,0x14)==id.environment){
                    detach_member130(id.actor,id,generation);if(id.mount)detach_member130(id.mount,id,generation);
                }
            }
        }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&member_errors130);}
    }__finally{member_sync_active130=false;ReleaseSRWLockExclusive(&member_lock130);}
}
static void __fastcall unit_tick130(void* current,void*){
    auto native_unit=reinterpret_cast<uintptr_t>(current);
    charge_boundary146(native_unit,false);
    if(detached_count130||proximity_count147||native_unit==unit)sync_members130(native_unit);
    original_unit_tick130(current);
    charge_boundary146(native_unit,true);
    if(detached_count130||proximity_count147||native_unit==unit)sync_members130(native_unit);
}
#include "unit_detach_combat130.h"
static void create_unit_detach130(){
    const unsigned char tick[]={0x51,0x56,0x8b,0xf1,0x57,0x89,0x74,0x24,8};
    const unsigned char add[]={0x56,0x8b,0x74,0x24,8,0x57,0x8b,0xf9};
    const unsigned char transfer[]={0x56,0x8b,0x74,0x24,8,0x57,0x8b,0xf9};
    const unsigned char candidate[]={0x83,0xb8,0xb4,3,0,0,0,0x0f,0x85,0xab,0,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x1b7ed0),tick,sizeof tick)&&!memcmp(reinterpret_cast<void*>(base+0x14bc50),add,sizeof add)&&
        !memcmp(reinterpret_cast<void*>(base+0x1b33f0),transfer,sizeof transfer)&&!memcmp(reinterpret_cast<void*>(base+0x1afed6),candidate,sizeof candidate),"Native squad detachment signatures differ");
    const unsigned char remove[]={0x8b,0x54,0x24,4,0xc7,0x82,0xb4,3,0,0,0,0,0,0};
    const unsigned char cancel[]={0x56,0x8b,0xf1,0x80,0xbe,0x94,0,0,0,1};
    const unsigned char resume[]={0x8b,0x8f,0x84,3,0,0,0x83,0xc6,4,0x8b,0x87,0x88,3,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x1a80a0),remove,sizeof remove)&&!memcmp(reinterpret_cast<void*>(base+0x3ca8f0),cancel,sizeof cancel)&&
        !memcmp(reinterpret_cast<void*>(base+0x1aff8e),resume,sizeof resume)&&field<uintptr_t>(base+0x15bb21c,0)==base+0x14bcd0&&
        field<uintptr_t>(base+0x15bb21c,4)==base+0x1a8110,"Native squad lifecycle signatures differ");
    regroup_skip130=base+0x1aff8e;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1b7ed0),reinterpret_cast<void*>(&unit_tick130),reinterpret_cast<void**>(&original_unit_tick130))==MH_OK,"Unit detachment boundary hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x14bc50),reinterpret_cast<void*>(&member_insert130),reinterpret_cast<void**>(&original_member_insert130))==MH_OK,"Squad insertion hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1b33f0),reinterpret_cast<void*>(&member_transfer130),reinterpret_cast<void**>(&original_member_transfer130))==MH_OK,"Squad transfer hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1afed6),reinterpret_cast<void*>(&regroup_gate130),&original_regroup_gate130)==MH_OK,"Squad regroup candidate hook failed");
    create_detach_combat130();
    create_proximity147();
    create_native_charge147();
}

static MH_STATUS disable_control_hooks130(){
    MH_STATUS result=MH_OK;
    for(unsigned rva:{0x14bc50u,0x1b33f0u,0x1afed6u,0x3dacf0u,0x3c0bd0u,0x1679c0u,0x174ec0u,0x16aa00u,0x1b8740u,0x48d6f3u,0x459b4du,0x175360u,0x175346u,0x1b8830u,0x48d6a0u,0x48d860u,0x1752d0u,0x47efc0u,0x154140u}){
        auto status=MH_DisableHook(reinterpret_cast<void*>(base+rva));
        if(status!=MH_OK&&status!=MH_ERROR_DISABLED)result=MH_UNKNOWN;
    }
    // Pending native rejoin keeps both its callback and read-only personal
    // combat compatibility resident. No off-thread native actor writes.
    if(!detached_count130&&!proximity_count147)for(unsigned rva:{0x1b7ed0u,0x3eba00u,0x3ed950u,0x3ee240u,0x15d5d0u,0x14ad80u,0x17bee0u,0x3edcbau,0x3ee767u}){
        auto status=MH_DisableHook(reinterpret_cast<void*>(base+rva));
        if(status!=MH_OK&&status!=MH_ERROR_DISABLED)result=MH_UNKNOWN;
    }
    return result;
}
