#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "aim_geometry.h"
#include "mounted_shot87.h"
#include "personal_weapon_metadata122.h"
#include "revolver65.h"
#include "manual_dispersion.h"
// UI thread creates an ordinary BCQ_FIRE_PROJECTILE packet with native serializers.
// Native simulation dispatch executes the shot, using the observed bow release pipeline.
using BowDispatch=void(__cdecl*)(const uint32_t*,void*);
using NativeGet=void*(__thiscall*)(void*);
using NativePoint=void*(__thiscall*)(void*,const float*);
using WriteUInt=void*(__cdecl*)(void*,uint32_t);
using WriteFloat=void*(__cdecl*)(void*,float);
using WriteObject=void*(__cdecl*)(void*,const void*);
using Parameters=void*(__thiscall*)(void*,const float*,float,void*,float,float,void*,void*,void*);
using Launch=void*(__thiscall*)(void*,void*,void*,void*,float);
using Ballistic=bool(__thiscall*)(void*,const float*);
using Consume=bool(__thiscall*)(void*);
using Spawn=void(__thiscall*)(void*,void*,float);
using Register=void(__thiscall*)(void*,void*);
static BowDispatch original_bow_dispatch;
static void begin_weapon(const uint32_t*,uintptr_t);
static volatile LONG fire_busy;
static uint32_t ticket_actor,ticket_unit,ticket_index;
static uintptr_t ticket_environment;
struct AimFrame {ViewRay ray;uintptr_t actor,env;uint64_t tick;unsigned lease101{},serial101{},unit101{};};
static AimFrame aim_frame;
static volatile LONG aim_sequence;
static void capture_aim_frame(uintptr_t camera){
    if(state->mode!=POSSESSED||!camera_owned(camera))return;
    __try {
        AimFrame frame{};
        if(!camera_ray(reinterpret_cast<float*>(camera+8),reinterpret_cast<float*>(camera+0x14),frame.ray))return;
        frame.actor=possessed;frame.env=environment;frame.tick=GetTickCount64();
        frame.lease101=possession_lease.generation;frame.serial101=possession_lease.identity.actor_id;frame.unit101=possession_lease.identity.unit_id;
        InterlockedIncrement(&aim_sequence);MemoryBarrier();memcpy(&aim_frame,&frame,sizeof(frame));MemoryBarrier();InterlockedIncrement(&aim_sequence);
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedExchange(&state->release,1);state->shot_error=94;}
}
static bool read_aim_frame(ViewRay& ray,bool presentation101=false){
    (void)presentation101; // Compatibility argument: every consumer now proves the same lease.
    for(unsigned i=0;i<4;++i){
        LONG seq=aim_sequence;if(seq&1)continue;
        AimFrame frame{};MemoryBarrier();memcpy(&frame,&aim_frame,sizeof(frame));MemoryBarrier();
        if(seq!=aim_sequence)continue;
        if(frame.actor!=possessed||frame.env!=environment||GetTickCount64()-frame.tick>250)return false;
        if(!possession_lease.active||frame.lease101!=possession_lease.generation||frame.serial101!=possession_lease.identity.actor_id||frame.unit101!=possession_lease.identity.unit_id)return false;
        ray=frame.ray;return true;
    }
    return false;
}
static void manual_accuracy(void* pool,void* launch,float angle){
    bool previous_override=manual_cone_override90;float previous_angle=manual_cone_angle90;manual_cone_override90=true;manual_cone_angle90=angle;
    void* previous=manual_dispersion_launch;manual_dispersion_launch=launch;
    __try {reinterpret_cast<Spawn>(base+0x427400)(pool,launch,1.0f);}
    __finally {manual_dispersion_launch=previous;manual_cone_override90=previous_override;manual_cone_angle90=previous_angle;}
}
static bool native_bow_signatures(uintptr_t b){
    // Exact engine hash is checked separately. Entry prefixes avoid relocated immediates.
    {const unsigned char v[]={0x81,0xec,0xe4,0,0,0,0x53,0x8b};if(memcmp(reinterpret_cast<void*>(b+0x2ac0b0),v,8))return false;}
    {const unsigned char v[]={0x8b,0x44,0x24,4,0x56,0x8b,0xf1};if(memcmp(reinterpret_cast<void*>(b+0x40ebe0),v,7))return false;}
    {const unsigned char v[]={0x8b,0x44,0x24,4,0x56,0x8b,0x74,0x24};if(memcmp(reinterpret_cast<void*>(b+0x40e9d0),v,8))return false;}
    {const unsigned char v[]={0x55,0x8b,0xec,0x83,0xe4,0xf0};if(memcmp(reinterpret_cast<void*>(b+0x420050),v,6))return false;}
    {const unsigned char v[]={0x83,0xec,0x3c,0x53,0x55,0x8b,0xd9};if(memcmp(reinterpret_cast<void*>(b+0x1a9290),v,7))return false;}
    {const unsigned char v[]={0x83,0xec,0x2c,0x53,0x8b,0x5c,0x24,0x34};if(memcmp(reinterpret_cast<void*>(b+0x427400),v,8))return false;}
    {const unsigned char v[]={0x83,0xec,0x44,0x53,0x55,0x8b,0x6c,0x24};if(memcmp(reinterpret_cast<void*>(b+0x41c760),v,8))return false;}
    return true;
}
// Recovered420050 direct branch accepts all native projectile families.
// Aim sets initial direction; gravity, fuse, collision and effects remain native.
static bool personal_direct_projectile(uintptr_t record){return native_personal_kind(record)!=PersonalWeaponKind::Unsupported;}
static bool active_suppression_projectile98(uintptr_t actor_unit,uintptr_t projectile);
static uintptr_t unit_projectile152(uintptr_t actor_unit,bool personal=false){
    if(field<unsigned>(actor_unit,0xf7c)==0)return 0;
    uintptr_t record=reinterpret_cast<uintptr_t>(reinterpret_cast<NativeGet>(base+0x14ad40)(reinterpret_cast<void*>(actor_unit)));
    if(!record)return 0;
    // Suppression selects a formation projectile. Its firearm classification
    // alone does not establish an individual ready/fire animation profile.
    // Keep the verified standard rifle for the player during that ability.
    if(personal){
        const auto standard=field<uintptr_t>(actor_unit,0xf1c);
        if(standard!=record&&active_suppression_projectile98(actor_unit,record)&&
           native_personal_kind(standard)==PersonalWeaponKind::Firearm)return standard;
    }
    return native_personal_kind(record)!=PersonalWeaponKind::Unsupported?record:0;
}
static uintptr_t bow_record(uintptr_t actor_unit){
    if(possessed&&actor_unit==unit&&actor152::individual(base,field<uintptr_t>(possessed,0))){
        auto actions=field<uintptr_t>(possessed,0x23c),root=actions?field<uintptr_t>(actions,0):0;
        auto definitions=root?field<uintptr_t>(root,0x28):0;
        if(!definitions||!field<unsigned>(definitions,0xc44))return 0;
        auto personal=field<uintptr_t>(possessed,0x6cc);
        if(personal)return native_personal_kind(personal)==PersonalWeaponKind::Unsupported?0:personal;
    }
    return unit_projectile152(actor_unit,possessed&&actor_unit==unit);
}
static bool is_bow_projectile(uintptr_t record){return native_personal_kind(record)==PersonalWeaponKind::Bow;}
#include "bow_input142.h"
// Both window and worker observations share the locked edge sampler. No bow
// path may emit a full-power request on mouse-down. Unknown families wait.
static void weapon_mouse143(UINT message,bool active){
    if(bow_family142==1)bow_sample142(message!=WM_LBUTTONUP,active);
    else if(message==WM_LBUTTONDOWN){
        if(active&&bow_family142==0)InterlockedExchange(&state->fire_request,1);
        else if(!active)++state->shots_blocked;
    }
}
#include "native_audio143.h"
static uintptr_t database(uintptr_t env){
    uintptr_t owner=field<uintptr_t>(env,0x2804c);if(!owner)return 0;
    uintptr_t data=field<uintptr_t>(owner,0x10);if(!data)return 0;
    return reinterpret_cast<uintptr_t>(reinterpret_cast<NativeGet>(base+0xbd6980)(reinterpret_cast<void*>(data)));
}
static bool weapon_ready_held(){
    auto request=equipment_requests_snapshot81();
    bool changing_ammo=request.lease==state->lease_generation&&request.tabs&&!state->weapon_phase&&!state->fire_ticket;
    return equipment_ranged_selected81()&&!changing_ammo&&state->aiming&&(deck181::key_state(VK_RBUTTON)&0x8000)!=0;
}
#include "native_accuracy90.h"
static void manual_bow_apply(const uint32_t* payload,uintptr_t context){
    alignas(16) unsigned char parameters[0x48]{},launch[0x80]{};
    __try {
        state->shot_thread=GetCurrentThreadId();
        uintptr_t actor=payload[11],env=field<uintptr_t>(context,0);
        if(!enabled||state->stop||state->release||state->mode!=POSSESSED||actor!=possessed||env!=environment||env!=ticket_environment||!followed(actor,false)){state->shot_error=70;return;}
        if(!actor152::human(base,field<uintptr_t>(actor,0))||field<unsigned char>(actor,0xf0)!=1||field<uintptr_t>(actor,0x214)!=unit
            ||field<unsigned>(actor,0x1c8)!=ticket_actor||field<unsigned>(unit,4)!=ticket_unit){state->shot_error=71;return;}
        // A queued shot must never cancel or replace a native get-up animation.
        if(native_recovery103(actor)){InterlockedExchange(&state->weapon_phase,0);state->aim_pose=0;InterlockedExchange(&state->fire_request,0);++state->shots_canceled;return;}
        // Check again at the native release boundary, before consuming ammunition.
        if(!weapon_ready_held()){
            InterlockedExchange(&state->weapon_phase,0);state->aim_pose=0;++state->shots_canceled;
            using StopPose=void(__thiscall*)(void*);
            using ReadyPose=void(__thiscall*)(void*,unsigned,unsigned,unsigned);
            reinterpret_cast<StopPose>(base+0x3ca920)(reinterpret_cast<void*>(payload[11]));
            reinterpret_cast<ReadyPose>(base+0x3e3f50)(reinterpret_cast<void*>(payload[11]),equipment81::lower_mode(weapon_mounted(),equipment_crouched81()),0,0);
            return;
        }
        uintptr_t record=bow_record(unit),db=database(env);
        if(!record||!db||payload[0]>=field<unsigned>(db,8)||field<uintptr_t>(field<uintptr_t>(db,0xc),payload[0]*4)!=record){state->shot_error=72;return;}
        state->ammo_before=field<float>(unit,0xf78);
        if(!std::isfinite(state->ammo_before)||state->ammo_before<1){state->shot_error=73;return;}
        const float* origin=reinterpret_cast<const float*>(payload+2);
        const float* aim=reinterpret_cast<const float*>(payload+5);
        for(unsigned i=0;i<3;++i)if(!std::isfinite(origin[i])||!std::isfinite(aim[i])||std::abs(origin[i])>100000||std::abs(aim[i])>100000){state->shot_error=74;return;}
        float damage=field<float>(unit,0x164),aux=field<float>(unit,0x170);
        if(!std::isfinite(damage)||!std::isfinite(aux)){state->shot_error=74;return;}
        reinterpret_cast<Parameters>(base+0x40ebe0)(parameters,origin,mountedshot87::native_arc(weapon_mounted()),reinterpret_cast<void*>(actor),damage,aux,nullptr,nullptr,nullptr);
        // The native first-person projectile command sets this direct-aim flag.
        // Manual launch direction follows the reticle for both supported weapons.
        // The native arrow record still supplies gravity during projectile flight.
        bool direct=personal_direct_projectile(record);
        parameters[0x44]=direct?1:0;state->direct_fire=direct?1:0;
        reinterpret_cast<Launch>(base+0x40e9d0)(launch,reinterpret_cast<void*>(context),reinterpret_cast<void*>(record),parameters,bow_speed142(record,field<float>(record,0x6c)));
        if(!reinterpret_cast<Ballistic>(base+0x420050)(launch,aim)){state->shot_error=76;return;}
        float solved_velocity[3];memcpy(solved_velocity,launch+0x50,sizeof(solved_velocity));
        uint32_t accuracy_lease=state->lease_generation;accuracy90::Envelope accuracy{};
        if(!measure_accuracy90(actor,context,record,launch,accuracy)){state->shot_error=105;publish_accuracy90(nullptr,record,6);return;}
        if(state->lease_generation!=accuracy_lease){state->shot_error=106;return;}
        if(!reinterpret_cast<Consume>(base+0x1a9290)(reinterpret_cast<void*>(unit))){state->shot_error=73;return;}
        manual_accuracy(reinterpret_cast<void*>(field<uintptr_t>(context,0x9c)),launch,accuracy.half_angle);
        publish_accuracy90(&accuracy,record,0,accuracy_lease);
        state->spread_removed=manual_spread_removed;
        state->conical_shots=static_cast<uint32_t>(cone_shot_total);state->cone_errors=static_cast<uint32_t>(cone_error_total);
        state->cone_half_angle=cone_metrics.half_angle;state->native_spread_horizontal=cone_metrics.horizontal;
        state->native_spread_vertical=cone_metrics.vertical;
        state->launch_delta=0;
        for(unsigned i=0;i<3;++i){
            state->shot_origin[i]=origin[i];state->shot_velocity[i]=field<float>(reinterpret_cast<uintptr_t>(launch),0x50+i*4);
            state->launch_delta=std::max(state->launch_delta,std::abs(state->shot_velocity[i]-solved_velocity[i]));
        }
        unsigned arrow=arrowcam107::no_projectile;
        const auto pool=field<uintptr_t>(context,0xe8);
        friendly_register109(pool,launch,is_bow_projectile(record)?&arrow:nullptr);
        if(arrow!=arrowcam107::no_projectile)arrow_capture107(context,pool,arrow,accuracy_lease);
        // Match the ordinary ranged task: native unit/weapon modifiers determine reload.
        using ReloadDuration=float(__thiscall*)(void*,void*);
        float duration=reinterpret_cast<ReloadDuration>(base+0x3c86e0)(reinterpret_cast<void*>(actor),reinterpret_cast<void*>(record));
        if(!std::isfinite(duration)||duration<0||duration>300){state->shot_error=86;InterlockedExchange(&state->release,1);return;}
        schedule_personal_reload65(reinterpret_cast<NativeReloadSchedule65>(base+0x3ea970),actor,record,duration);
        state->ammo_after=field<float>(unit,0xf78);++state->shots_spawned;state->shot_error=0;
    }__except(EXCEPTION_EXECUTE_HANDLER){state->shot_error=75;InterlockedExchange(&state->release,1);}
}
static bool is_ticket(const uint32_t* payload){
    __try {return state->fire_ticket&&payload[0]==ticket_index&&payload[11]&&field<unsigned>(payload[11],0x1c8)==ticket_actor
        &&field<unsigned>(field<uintptr_t>(payload[11],0x214),4)==ticket_unit;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static void __cdecl bow_dispatch(const uint32_t* payload,void* context){
    if(is_ticket(payload)){
        begin_weapon(payload,reinterpret_cast<uintptr_t>(context));
        InterlockedExchange(&state->fire_ticket,0);return;
    }
    original_bow_dispatch(payload,context);
}
static bool queue_bow(uintptr_t){
    alignas(16) unsigned char context[16]{},goal[24]{},packet[QUEUE_CAPACITY+4]{};
    __try {
        uintptr_t record=bow_record(unit);if(!record){state->shot_error=80;return false;}
        if(field<float>(unit,0xf78)<1){state->shot_error=73;return false;}
        uintptr_t db=database(environment);if(!db)return false;
        unsigned count=field<unsigned>(db,8);uintptr_t array=field<uintptr_t>(db,0xc);
        if(!array||count>4096){state->shot_error=81;return false;}
        unsigned index=0;for(;index<count&&field<uintptr_t>(array,index*4)!=record;++index){}
        if(index==count){state->shot_error=81;return false;}
        ViewRay ray{};if(!read_aim_frame(ray)){state->shot_error=94;return false;}
        float origin[3],aim[3];for(unsigned i=0;i<3;++i)origin[i]=ray.origin[i]+ray.direction[i]*0.4f;
        crosshair_target(ray,aim,personal_direct_projectile(record)?6000.0f:80.0f);
        for(unsigned i=0;i<3;++i)if(!std::isfinite(origin[i])||!std::isfinite(aim[i]))return false;
        reinterpret_cast<NativePoint>(base+0x3511d0)(goal,aim);
        uint32_t key=field<uint32_t>(base,0x1931ee8);unsigned flags=0;
        uintptr_t native_queue=field<uintptr_t>(environment,0x28048);
        if(!native_queue||!descriptor_flags(key,flags)){state->shot_error=82;return false;}
        reinterpret_cast<ContextConstructor>(base+0x1cc200)(context,packet,key,flags);
        reinterpret_cast<WriteUInt>(base+0x285a00)(context,index);
        reinterpret_cast<WriteFloat>(base+0x285a20)(context,3.141592654f);
        reinterpret_cast<WriteObject>(base+0x285970)(context,origin);
        reinterpret_cast<WriteObject>(base+0x361a10)(context,goal);
        reinterpret_cast<WriteObject>(base+0x285b30)(context,reinterpret_cast<void*>(possessed));
        reinterpret_cast<WriteFloat>(base+0x285a20)(context,field<float>(record,0x6c));
        reinterpret_cast<NativeCleanup>(base+0x1d0d40)(context);
        uint32_t size=field<uint32_t>(reinterpret_cast<uintptr_t>(packet),QUEUE_CAPACITY);
        auto& used=field<uint32_t>(native_queue+4,QUEUE_CAPACITY);
        if(!valid_packet(packet,size,key)||used>QUEUE_CAPACITY||size>QUEUE_CAPACITY-used){state->shot_error=83;return false;}
        ticket_actor=field<uint32_t>(possessed,0x1c8);ticket_unit=field<uint32_t>(unit,4);ticket_index=index;ticket_environment=environment;
        state->shot_actor=ticket_actor;state->shot_unit=ticket_unit;state->shot_index=index;state->shot_packet_size=size;
        for(unsigned i=0;i<3;++i)state->shot_target[i]=aim[i];
        MemoryBarrier();InterlockedExchange(&state->fire_ticket,1);
        if(!append_packet(reinterpret_cast<unsigned char*>(native_queue+4),used,packet,size,key)){InterlockedExchange(&state->fire_ticket,0);state->shot_error=83;return false;}
        ++state->shots_queued;state->shot_origin_thread=GetCurrentThreadId();state->shot_error=0;return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){state->shot_error=85;InterlockedExchange(&state->release,1);return false;}
}
static void process_bow(uintptr_t camera){
    if(!enabled||state->stop||state->release||state->mode!=POSSESSED||!state->fire_request)return;
    if(!weapon_ready_held()){InterlockedExchange(&state->fire_request,0);++state->shots_blocked;return;}
    if(!camera_owned(camera)||GetCurrentThreadId()!=state->window_thread)return;
    // Paused battles have no actor motion frames. A paused click is discarded.
    if(GetTickCount64()-state->last_motion_tick>250){InterlockedExchange(&state->fire_request,0);return;}
    if(state->fire_ticket||state->weapon_phase||field<float>(possessed,0x390)>0){InterlockedExchange(&state->fire_request,0);return;}
    // Keep an early LMB click pending until RMB's native draw transition ends.
    // Releasing RMB above cancels it without consuming ammo or spawning an arrow.
    uintptr_t record=bow_record(unit);
    bool from_type=false;
    const auto kind=native_personal_kind(record,&from_type);
    if((weapon_mounted()||kind==PersonalWeaponKind::Bow||from_type)&&state->aim_pose!=1)return;
    if(InterlockedCompareExchange(&fire_busy,1,0))return;
    __try {
        InterlockedExchange(&state->fire_request,0);
        if(followed(possessed,false)&&selected_possessed_group()){
            if(bow_bind142(record,weapon_mounted()))queue_bow(camera);
            else {state->shot_error=106;++state->shots_canceled;} // No unbound full-power bow shot.
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){state->shot_error=85;InterlockedExchange(&state->release,1);}
    InterlockedExchange(&fire_busy,0);
}
static void create_dispersion_hook(){
    const unsigned char dispersion_signature[]={0x83,0xec,0x0c,0x56,0x57,0x8b,0x7c,0x24,0x18};
    require(memcmp(reinterpret_cast<void*>(base+0x4272e0),dispersion_signature,sizeof(dispersion_signature))==0,"Projectile dispersion signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x4272e0),reinterpret_cast<void*>(&dispersion_hook),reinterpret_cast<void**>(&original_dispersion))==MH_OK,"Manual dispersion hook failed");
}
