#pragma once
// Included after the native ranged animation pipeline. Native calls execute
// only at validated simulation/camera boundaries, never on the input worker.
using UnitAbility81=bool(__thiscall*)(void*,unsigned);
using AbilityConstructor81=void*(__thiscall*)(void*,const void*,unsigned,unsigned,unsigned);
static unsigned equipment_generation81,equipment_error81,equipment_switches81,equipment_ammo_changes81;
static uintptr_t equipment_expected_projectile81;
static uint64_t equipment_ammo_deadline81;
static SRWLOCK equipment_publish_lock81=SRWLOCK_INIT;
#include "unit_weapon131.h"
static bool equipment_signatures81(uintptr_t b){
    const unsigned char constructor[]={0x56,0x8b,0x74,0x24,8,0x57,0x8b,0xf9,0x8d,0x46,4,0x50};
    const unsigned char serializer[]={0x57,0x8b,0xf9,0x8d,0x47,4,0x50,0xff,0x74,0x24,0x0c};
    const unsigned char unlocked[]={0x56,0x8b,0x74,0x24,8,0x57,0x8b,0xf9,0x8b,0x87,0xd4,0x0e};
    const unsigned char eligible[]={0x53,0x56,0x8b,0x74,0x24,0x0c,0x57,0x56,0x8b,0xf9};
    const unsigned char action134[]={0xf3,0x0f,0x10,0x44,0x24,0x08,0x56,0x8b,0xf1,0x0f,0xb6,0x86,0x8c,0x05,0,0,0x50};
    return !memcmp(reinterpret_cast<void*>(b+0x10dfb80),action134,sizeof action134)&&!memcmp(reinterpret_cast<void*>(b+0x255290),constructor,sizeof constructor)&&
        !memcmp(reinterpret_cast<void*>(b+0x2e6cd0),serializer,sizeof serializer)&&
        !memcmp(reinterpret_cast<void*>(b+0x15c530),unlocked,sizeof unlocked)&&
        !memcmp(reinterpret_cast<void*>(b+0x15be80),eligible,sizeof eligible)&&field<uintptr_t>(b+0x15d0030,0)==b+0x2e6cd0;
}
static bool equipment_unit_ability81(unsigned id,bool eligible){
    return reinterpret_cast<UnitAbility81>(base+(eligible?0x15be80:0x15c530))(reinterpret_cast<void*>(unit),id);
}
static bool equipment_committed81(){
    melee60::Request request;return melee_request(request)||
        (possession_lease.active&&melee_handoff78.matches(locomotion_actor(),possession_lease.generation,
            possession_lease.identity.mount?possession_lease.identity.mount_id:possession_lease.identity.actor_id,possession_lease.identity.unit_id));
}
static equipment81::Boundary equipment_boundary81(){
    float dt=field<float>(base,0x15b2448),reload=field<float>(possessed,0x390);
    return {field<unsigned char>(possessed,0xf0)==1&&!dead_view(),std::isfinite(dt)&&dt>=.001f&&dt<=.2f,
        equipment_committed81(),false,bool(state->fire_ticket||state->weapon_phase),!std::isfinite(reload)||reload>0};
}
static unsigned collect_ammunition81(equipment81::AmmoOption (&options)[33],unsigned& current,bool eligibility){
    uintptr_t normal=field<uintptr_t>(unit,0xf1c);auto kind=native_personal_kind(normal);
    if(kind==PersonalWeaponKind::Unsupported)return 0;
    options[0]={0,normal,false,true};unsigned count=1;current=0;
    uintptr_t active=bow_record(unit),controller=unit+0xed4;
    if(field<uintptr_t>(controller,0)!=base+0x15cfeec||field<uintptr_t>(controller,8)!=unit)return 0;
    unsigned n=field<unsigned>(controller,0x28),capacity=field<unsigned>(controller,0x24);auto array=field<uintptr_t>(controller,0x2c);
    if(n>128||capacity<n||capacity>4096||(n&&!array))return 0;
    for(unsigned i=0;i<n;++i){auto state_ptr=array+i*0x282c,record=field<uintptr_t>(state_ptr,4);if(!record)continue;
        unsigned id=field<unsigned>(record,0);auto projectile=field<uintptr_t>(record,0x28);
        if(!projectile||projectile==normal||id>255||!equipment_unit_ability81(id,false)||native_personal_kind(projectile)!=kind)continue;
        bool duplicate=false;for(unsigned j=1;j<count;++j)if(options[j].id==id)duplicate=true;
        if(duplicate)continue;if(count==33)break;
        bool active_style=field<unsigned char>(state_ptr,0x18)!=0;
        options[count]={id,projectile,active_style,eligibility?equipment_unit_ability81(id,true):false};
        if(projectile==active&&active_style)current=count;
        ++count;
    }
    return count;
}
static bool serialize_ability81(unsigned id,unsigned action,unsigned char* packet,uint32_t& size){
    alignas(16) unsigned char order[0x6c]{},context[16]{};bool constructed=false,context_live=false,ok=false;
    __try {
        auto group=selected_possessed_group();unsigned flags=0;auto key=field<uint32_t>(base,0x192ee34);
        if(!group||!descriptor_flags(key,flags))return false;
        reinterpret_cast<AbilityConstructor81>(base+0x255290)(order,reinterpret_cast<void*>(group),id,0,action);constructed=true;
        auto vtable=field<uintptr_t>(reinterpret_cast<uintptr_t>(order),0);
        if(vtable==base+0x15d0030&&field<uintptr_t>(vtable,0)==base+0x2e6cd0){
            reinterpret_cast<ContextConstructor>(base+0x1cc200)(context,packet,key,flags);context_live=true;
            ok=reinterpret_cast<SerializeOrder>(base+0x2e6cd0)(order,context)!=0;
            reinterpret_cast<NativeCleanup>(base+0x1d0d40)(context);context_live=false;
            size=field<uint32_t>(reinterpret_cast<uintptr_t>(packet),QUEUE_CAPACITY);ok=ok&&valid_packet(packet,size,key);
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){ok=false;}
    __try {
        if(context_live)reinterpret_cast<NativeCleanup>(base+0x1d0d40)(context);
        if(constructed)reinterpret_cast<NativeCleanup>(base+0x1d10f0)(order+4);
    }__except(EXCEPTION_EXECUTE_HANDLER){ok=false;}
    return ok;
}
static bool cycle_ammunition81(){
    // Two commands are serialized and capacity-checked BEFORE either is appended.
    // The native executor rechecks eligibility and owns duration/charges/recharge.
    equipment81::AmmoOption options[33]{};unsigned current=0,count=collect_ammunition81(options,current,true);
    if(count<2){equipment_error81=1;return false;}
    unsigned next=equipment81::next_ammo(options,count,current);if(next==current){equipment_error81=2;return false;}
    alignas(16) unsigned char first[QUEUE_CAPACITY+4]{},second[QUEUE_CAPACITY+4]{};
    uint32_t sizes[2]{};unsigned commands=0;
    if(current&&options[current].active){if(!serialize_ability81(options[current].id,1,first,sizes[commands++])){equipment_error81=3;return false;}}
    if(next){if(!serialize_ability81(options[next].id,0,commands?second:first,sizes[commands])){equipment_error81=3;return false;}++commands;}
    if(!commands)return false;
    auto queue=field<uintptr_t>(environment,0x28048);if(!queue){equipment_error81=3;return false;}
    auto& used=field<uint32_t>(queue+4,QUEUE_CAPACITY);auto key=field<uint32_t>(base,0x192ee34);
    uint32_t total=sizes[0]+sizes[1];
    if(used>QUEUE_CAPACITY||total>QUEUE_CAPACITY-used){equipment_error81=4;return false;}
    equipment_expected_projectile81=options[next].record;equipment_ammo_deadline81=GetTickCount64()+2000;
    InterlockedExchange(&equipment_ammo_inflight81,1);InterlockedExchange(&state->fire_request,0);
    // Both packets passed the same validator; append is now infallible on this
    // sole native queue writer. No partial toggle on a full queue.
    if(!append_packet(reinterpret_cast<unsigned char*>(queue+4),used,first,sizes[0],key)||
       (commands==2&&!append_packet(reinterpret_cast<unsigned char*>(queue+4),used,second,sizes[1],key))){
        equipment_error81=4;InterlockedExchange(&equipment_ammo_inflight81,0);return false;
    }
    equipment_error81=0;return true;
}
static void ammunition_label81(uintptr_t record,wchar_t (&label)[64]){
    wcscpy_s(label,L"Standard");if(!record)return;
    unsigned length=field<unsigned>(record,4);auto text=field<uintptr_t>(record,0xc);if(!text||!length||length>128)return;
    wchar_t key[129]{};bool wide=field<unsigned char>(text,1)==0;
    for(unsigned i=0;i<length;++i)key[i]=wide?field<wchar_t>(text,i*2):field<unsigned char>(text,i);
    if(wcsstr(key,L"flaming"))wcscpy_s(label,L"Fire arrows");
    else if(wcsstr(key,L"whistling"))wcscpy_s(label,L"Whistling arrows");
    else if(wcsstr(key,L"blinding"))wcscpy_s(label,L"Blinding grenades");
    else if(wcsstr(key,L"suppressive"))wcscpy_s(label,L"Suppressive fire");
}
#include "ammunition_publish83.h"
static void publish_equipment81(){
    if(!equipment_status81||!possession_lease.active)return;
    EquipmentStatus81 sample{};sample.lease=state->lease_generation;sample.actor=state->actor_id;sample.unit=state->unit_id;
    sample.slot=equipment_personal_override214?equipment81::Ranged:equipment_slot81;auto record=bow_record(unit);sample.kind=unsigned(native_personal_kind(record));
    sample.launcher_sight=launcher_sight123(record);
    sample.projectile=record?field<unsigned>(record,0):0;sample.crouch_supported=equipment_crouch_supported81;sample.crouched=equipment_crouch81;
    auto request=equipment_requests_snapshot81();sample.pending=request.slot||request.toggle||request.tabs||equipment_ammo_inflight81;
    sample.error=equipment_error81;sample.switches=equipment_switches81;sample.ammo_changes=equipment_ammo_changes81;sample.tick=GetTickCount64();
    equipment81::AmmoOption options[33]{};unsigned current=0;sample.styles=collect_ammunition81(options,current,false);sample.ability=current?options[current].id:0;
    if(sample.slot==equipment81::Ranged&&sample.kind==unsigned(PersonalWeaponKind::Bow)&&
        state->aiming&&!state->fire_ticket&&!state->weapon_phase&&state->weapon_reload_remaining<=0)
        sample.bow_power=bow_power142(weapon_mounted(),bow_clock142());
    ammunition_label81(record,sample.ammunition);
    publish_ammunition83(options,sample.styles,record,sample.pending,sample.error);
    AcquireSRWLockExclusive(&equipment_publish_lock81);InterlockedIncrement(&equipment_status81->sequence);MemoryBarrier();
    // Header and sequence are not copied over a reader's seqlock.
    memcpy(&equipment_status81->lease,&sample.lease,offsetof(EquipmentStatus81,arrow_camera)-offsetof(EquipmentStatus81,lease));
    MemoryBarrier();InterlockedIncrement(&equipment_status81->sequence);ReleaseSRWLockExclusive(&equipment_publish_lock81);
}
#include "ability_native141.h"
static void process_equipment_commands81(uintptr_t camera){
    if(!enabled||state->stop||state->release||state->mode!=POSSESSED||dead_view()||!camera_owned(camera)||
       GetCurrentThreadId()!=state->window_thread||equipment_generation81!=state->lease_generation)return;
    __try {
        if(equipment_ammo_inflight81){
            if(bow_record(unit)==equipment_expected_projectile81){InterlockedExchange(&equipment_ammo_inflight81,0);++equipment_ammo_changes81;equipment_error81=0;}
            else if(GetTickCount64()>equipment_ammo_deadline81){InterlockedExchange(&equipment_ammo_inflight81,0);equipment_error81=5;}
        }
        InterlockedExchange(&equipment_unit_melee134,field<unsigned char>(unit,0xead)?1:0);
        auto request=equipment_toggle134();
        // Unit orders follow the native RTS action boundary. A personal draw,
        // shot, reload or melee animation must not block the unit's mode order.
        // Personal animation changes still use the existing simulation boundary.
        equipment_mode_poll131(request,true);
        if(request.lease==state->lease_generation&&request.tabs&&equipment_ranged_selected81()&&
           equipment_boundary81().safe()&&GetTickCount64()-state->last_motion_tick<=250&&!state->aim_pose){
            cycle_ammunition81();equipment_consume81(request.lease,0,true);
        }
        ability_process141(camera);publish_equipment81();
    }__except(EXCEPTION_EXECUTE_HANDLER){equipment_error81=6;InterlockedExchange(&state->release,1);}
}
static bool equipment_prepare_motion81(){
    // Caller has validated the lease and excluded committed native melee.
    const auto kind142=native_personal_kind(bow_record(unit));
    const bool ranged=kind142!=PersonalWeaponKind::Unsupported;
    if(equipment_generation81!=state->lease_generation){
        InterlockedExchange(&equipment_personal_override214,0);
        equipment_generation81=state->lease_generation;InterlockedExchange(&equipment_lease81,LONG(equipment_generation81));
        InterlockedExchange(&equipment_personal152,ranged?1:0);
        InterlockedExchange(&equipment_ranged81,unit_projectile152(unit)?1:0);
        bool melee=field<unsigned char>(unit,0xead)!=0;InterlockedExchange(&equipment_unit_melee134,melee?1:0);
        InterlockedExchange(&equipment_slot81,ranged&&!melee?equipment81::Ranged:equipment81::Melee);
        InterlockedExchange(&equipment_crouch81,0);InterlockedExchange(&equipment_ammo_inflight81,0);equipment_error81=0;
    }
    InterlockedExchange(&bow_family142,!ranged?-1:kind142==PersonalWeaponKind::Bow?1:0);
    bool firearm=kind142==PersonalWeaponKind::Firearm;
    bool supported=equipment81::crouch_supported(weapon_mounted(),firearm,equipment_unit_ability81(57,false),
        valid_weapon_clip(possessed,106),valid_weapon_clip(possessed,127),valid_weapon_clip(possessed,132),valid_weapon_clip(possessed,138));
    InterlockedExchange(&equipment_crouch_supported81,supported?1:0);
    auto request=equipment_requests_snapshot81();
    if(!personal_weapon_mode(field<unsigned>(possessed,0x1f0),weapon_mounted())){
        // Native impacts and other non-weapon states retain their own movement.
        InterlockedExchange(&equipment_crouch81,0);return false;
    }
    bool safe=equipment_boundary81().safe()&&!equipment_ammo_inflight81;
    if(safe&&bool(equipment_personal152)!=ranged){
        // The formation's active projectile can change after possession.
        InterlockedExchange(&equipment_personal152,ranged?1:0);
        InterlockedExchange(&equipment_ranged81,unit_projectile152(unit)?1:0);
    }
    // Synchronize to the actual native unit mode, including native hotkey/UI
    // changes not represented by a private pending request (for example pause
    // and resume). Native unit state, not our requested target, is the authority.
    bool native_melee=field<unsigned char>(unit,0xead)!=0;
    InterlockedExchange(&equipment_unit_melee134,native_melee?1:0);
    unsigned native_slot=equipment_personal152&&!native_melee?equipment81::Ranged:equipment81::Melee;
    const bool personal_ready214=equipment_personal152&&native_melee&&state->aiming;
    const unsigned pose_slot214=personal_ready214?equipment81::Ranged:native_slot;
    bool restore_melee137=safe&&!personal_ready214&&native_slot==equipment81::Melee&&
        meleedisplay137::ready_request(field<unsigned>(possessed,0x1f0),weapon_mounted())&&
        melee_presentation_owned137(possessed);
    if(safe&&(unsigned(equipment_slot81)!=native_slot||bool(equipment_personal_override214)!=personal_ready214||restore_melee137)){
        own_weapon_call=1;
        __try {
            reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(possessed));mounted_draw={};state->aim_pose=0;
            InterlockedExchange(&state->fire_request,0);InterlockedExchange(&manual_arrow_released,0);
            unsigned mode=pose_slot214==equipment81::Melee?(weapon_mounted()?30:10):personal_lower_mode(weapon_mounted());
            original_change(reinterpret_cast<void*>(possessed),mode,0,0);
        }__finally {own_weapon_call=0;}
        InterlockedExchange(&equipment_slot81,LONG(native_slot));InterlockedExchange(&equipment_personal_override214,personal_ready214?1:0);InterlockedExchange(&equipment_crouch81,0);++equipment_switches81;
        equipment_render_reset81();
    }
    if(safe&&request.lease==state->lease_generation&&request.slot==native_slot){
        equipment_error81=0;equipment_consume81(request.lease,request.slot,false);
    }
    if(safe){
        bool crouched=supported&&equipment_ranged_selected81()&&request.lease==state->lease_generation&&request.ctrl;
        unsigned native_mode=field<unsigned>(possessed,0x1f0);
        bool native_kneeling=!weapon_mounted()&&(native_mode==9||native_mode==15||native_mode==17);
        if(crouched!=bool(equipment_crouch81)||(!crouched&&native_kneeling)){
            own_weapon_call=1;
            __try {
                reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(possessed));state->aim_pose=0;
                InterlockedExchange(&state->fire_request,0);
                original_change(reinterpret_cast<void*>(possessed),equipment81::lower_mode(false,crouched),0,0);
            }__finally {own_weapon_call=0;}
            InterlockedExchange(&equipment_crouch81,crouched?1:0);
        }
    }
    return equipment_crouch81!=0;
}
