#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "../../shogun2-possession-cleanup-v46/src/charge_protocol146.h"
// Player charge intent is primary attack + forward sprint, independent of unit orders.
// Native forward perception, nearest-distance reduction, eligibility and actor virtual154
// choose/approach a real opponent. Native contact/outcome/clip selection owns
// the encounter; no synthetic kill, random roll or animation asset is forced.
static SRWLOCK charge_lock146=SRWLOCK_INIT;
struct ChargeControl146 {
    Charge139 proof{};uintptr_t unit{},squad{},controller{},target_unit{},pair{};
    unsigned physical_serial{},target_id{},contact_tick{};bool entered{},assigning{},queued{},pose_seen149{};
};
static ChargeControl146 charge_control146;
static HANDLE charge_map146;static charge146::Shared* charge_shared146;
static volatile LONG charge_entries146,charge_pairs146,charge_returns146,charge_join_failures146;
static volatile LONG charge_queries148,charge_candidates148,charge_eligibility148,charge_unit_suppressed148,charge_contacts148,charge_reject148;
static volatile LONG charge_pose_wait149,charge_near_reject149;
static volatile LONG charge_pose_entries149,charge_pair_plan149=-1,charge_pair_outcome149=-1,charge_pair_kind149,charge_player_clip149,charge_target_clip149;
static uint64_t charge_probe_tick148;static unsigned charge_probe_lease148;
using ChargeScheduled148=void(__thiscall*)(void*,const void*);
using ChargeContact148=void(__thiscall*)(void*,uintptr_t,uintptr_t);
static ChargeScheduled148 original_charge_schedule148;
static ChargeContact148 original_charge_contact148;
static void charge_mapping146(){wchar_t name[96];charge146::name(name,GetCurrentProcessId());charge_map146=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(charge146::Shared),name);require(charge_map146!=nullptr,"Native charge observation mapping");charge_shared146=static_cast<charge146::Shared*>(MapViewOfFile(charge_map146,FILE_MAP_ALL_ACCESS,0,0,sizeof(charge146::Shared)));require(charge_shared146!=nullptr,"Native charge observation view");new(charge_shared146)charge146::Shared;}
static void charge_publish146(){
    charge146::Value v{};
    if(charge_control146.proof.active&&state&&possession_lease.active){auto id=possession_lease.identity;v={GetCurrentProcessId(),charge_control146.proof.generation,id.actor_id,id.unit_id,charge_control146.physical_serial,unsigned(id.mount!=0),1,GetTickCount64()};}
    charge146::publish(charge_shared146,v);
}
static void charge_reset139(){InterlockedExchange(&charge_input_generation146,0);InterlockedExchange(&charge_reset_request139,1);}
static bool charge_native_task146(uintptr_t actor){return field<unsigned char>(actor,0x2e8)==1&&field<uintptr_t>(actor,0x250)==base+0x15e0a5c&&field<uintptr_t>(actor,0x254)==actor&&!field<unsigned char>(actor,0x258);}
static bool charge_formation147(uintptr_t actor,bool pending){
    auto task=actor+0x2f8;return field<unsigned char>(actor,0x384)==1&&field<uintptr_t>(task,0)==base+0x15dd5cc&&field<uintptr_t>(task,8)==actor+0x2f0&&field<uintptr_t>(actor,0x2f0)==actor&&field<uintptr_t>(task,0x18)==environment&&(!pending||(!field<unsigned char>(task,0x10)&&!field<unsigned char>(task,0x34)));
}
static bool charge_member146(uintptr_t actor){
    if(!charge_input_locked146()||!member_owned130(actor))return false;
    AcquireSRWLockShared(&charge_lock146);auto c=charge_control146;ReleaseSRWLockShared(&charge_lock146);
    return c.proof.active&&c.proof.generation==possession_lease.generation&&c.unit==unit&&actor==c.proof.actor;
}
static uintptr_t charge_squad147(uintptr_t actor){
    if(!charge_member146(actor))return 0;
    AcquireSRWLockShared(&charge_lock146);auto squad=charge_control146.squad;ReleaseSRWLockShared(&charge_lock146);
    return squad_registered130(squad,unit,environment)?squad:0;
}
static bool charge_joined146(uintptr_t actor,uintptr_t expected){return field<uintptr_t>(actor,0x3b4)==expected&&squad_registered130(expected,unit,environment)&&squad_matches130(expected,actor)==1;}
static bool charge_motion146(uintptr_t actor){__try{return actor==locomotion_actor()&&charge_member146(actor);}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
static bool charge_admission146(uintptr_t actor){
    melee60::Request melee;
    return member_owned130(actor)&&actor==locomotion_actor()&&root_motion_subject(actor,false)&&!native_recovery103(actor)&&
        !climb_align152&&!wall_subject97(possessed)&&!native_airborne_subject99(possessed)&&!equipment_committed81()&&!state->aiming&&!state->aim_pose&&!state->weapon_phase&&
        !equipment_crouched81()&&!command_placing100&&!native_routing(unit)&&!field<uintptr_t>(actor,0x688)&&!melee_request(melee)&&
        // Original3e4850/3e44f0 classify0..7 as ground locomotion;8..11
        // are fighting/defending postures. Do not reset a combat posture to0
        // through charge entry3e6d50 merely because W+Shift remains held.
        field<unsigned>(actor,0x1f0)<=7&&
        (!equipment_ranged81||(equipment_slot81==equipment81::Melee&&field<unsigned char>(unit,0xead)));
}
static void charge_end146(){if(charge_control146.proof.active)InterlockedIncrement(&charge_returns146);charge_control146.proof.active=false;InterlockedExchange(&charge_input_generation146,0);charge_publish146();}
static void charge_initialize147(){
    auto& c=charge_control146;
    if(InterlockedExchange(&charge_reset_request139,0)||c.proof.generation!=possession_lease.generation||c.unit!=unit||c.proof.actor!=locomotion_actor()){
        charge_end146();c={};c.unit=unit;c.proof.generation=possession_lease.generation;c.proof.actor=locomotion_actor();
    }
}
static unsigned charge_simulation148(){return field<unsigned>(field<uintptr_t>(environment,8),0x50);}
static void charge_observe147(){
    auto& c=charge_control146;auto actor=c.proof.actor;if(!c.proof.active)return;
    if(!member_owned130(actor)||actor!=locomotion_actor()||field<unsigned>(actor,0x1c8)!=c.physical_serial||native_recovery103(actor)||native_routing(unit)||wall_subject97(possessed)){charge_end146();return;}
    bool task=charge_native_task146(actor),jump=native_mounted_jump140(actor);auto pair=field<uintptr_t>(actor,0x688);
    if(task&&!c.entered){c.entered=true;InterlockedIncrement(&charge_entries146);}
    if(field<unsigned>(actor,0x1d0)==16&&!c.pose_seen149){c.pose_seen149=true;InterlockedIncrement(&charge_pose_entries149);}
    if(pair&&!c.pair){c.pair=pair;InterlockedIncrement(&charge_pairs146);}
    // Numeric observation only, never admission or clip/outcome authority.
    // The exact target must still be in its current master roster before reads.
    __try{if(pair&&charge_target139(c.proof)&&field<uintptr_t>(pair,4)==environment&&field<unsigned>(pair,0x38)==2&&
       field<uintptr_t>(c.proof.target,0x688)==pair&&
       ((field<uintptr_t>(pair,0x30)==actor&&field<uintptr_t>(pair,0x34)==c.proof.target)||(field<uintptr_t>(pair,0x34)==actor&&field<uintptr_t>(pair,0x30)==c.proof.target))){
        auto n=field<unsigned>(pair,0x78);if(n==2)for(unsigned i=0;i<2;++i)if(field<uintptr_t>(pair,0x58+i*16)==actor)InterlockedExchange(&charge_pair_plan149,LONG(field<unsigned>(pair,0x5c+i*16)));
        InterlockedExchange(&charge_pair_outcome149,LONG(field<unsigned>(pair,0x80)));
        InterlockedExchange(&charge_pair_kind149,field<uintptr_t>(pair,0x98)?1:field<uintptr_t>(pair,0xb4)?2:0);
        InterlockedExchange(&charge_player_clip149,LONG(field<unsigned>(actor,0x1d4)));InterlockedExchange(&charge_target_clip149,LONG(field<unsigned>(c.proof.target,0x1d4)));
    }}__except(EXCEPTION_EXECUTE_HANDLER){} // Failed optional observation changes no control state.
    // A confirmed original contact queue entry may outlive its personal task
    // until encounter preparation. Retain observation for that native tick
    // handoff only, not a free-running wall-clock charge grace period.
    bool pending=c.queued&&unsigned(charge_simulation148()-c.contact_tick)<=2;
    if(!c.assigning&&((c.pair&&pair!=c.pair)||(!task&&!jump&&!pair&&!pending))){charge_end146();return;}
    charge_publish146();
}
struct ChargeList148 {uintptr_t vtable{},unknown{};unsigned capacity{},count{};uintptr_t data{};};
static_assert(sizeof(ChargeList148)==20,"Native entity result ABI");
using ChargeQuery148=void(__thiscall*)(void*,void*,float,unsigned,unsigned);
using ChargeEligible148=bool(__thiscall*)(void*,uintptr_t,uintptr_t);
static bool charge_select148(uintptr_t actor,Charge139& selected){
    ChargeList148 list{};bool found=false;
    __try{
        float range=field<float>(base,0x1a4db40+0x5c),minimum=field<float>(base,0x1a4cfe0+0x5c),separation=field<float>(base,0x1a4d960+0x5c);
        float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94);
        if(!std::isfinite(range)||range<=0||range>100||!std::isfinite(minimum)||minimum<0||minimum>100||!std::isfinite(separation)||separation<=0||separation>=range||
           !std::isfinite(vx)||!std::isfinite(vz)||vx*vx+vz*vz<minimum*minimum)return false;
        auto common=field<uintptr_t>(environment,8),manager=common?field<uintptr_t>(common,0xcc):0;if(!manager)return false;
        InterlockedIncrement(&charge_queries148);
        // Original forward perception uses the same 0x71c half-angle as native
        // personal melee awareness. The range is the live stock matchup value.
        reinterpret_cast<ChargeQuery148>(base+0x3d1fa0)(reinterpret_cast<void*>(actor),&list,range,0x71c,1);
        if(!list.data||!list.count||list.count>list.capacity||list.count>4096)return false;
        float center[3]={field<float>(actor,0x48),field<float>(actor,0x4c),field<float>(actor,0x50)};
        for(float value:center)if(!std::isfinite(value))return false;
        // 3b55a0 orders by bearing, not distance. Use the nearest-distance
        // reduction used by native individual targeting (3ed950/174ec0).
        // Cheap spatial reads cover this native query's results; the expensive
        // current-roster/eligibility proofs remain capped at32/2000 entries.
        uintptr_t nearest[32]{};float distances[32]{};unsigned size=0;
        for(unsigned i=0;i<list.count;++i){__try{
            auto target=field<uintptr_t>(list.data,i*4);if(!target||target==actor)continue;
            auto table=field<uintptr_t>(target,0);if(!actor152::human(base,table)&&table!=base+0x15e8524)continue;
            if(actor152::human(base,table)){auto mount=field<uintptr_t>(target,0x210);if(mount)target=mount;}
            float x=field<float>(target,0x48)-center[0],z=field<float>(target,0x50)-center[2],squared=x*x+z*z;
            if(!std::isfinite(squared)||squared<=0||squared>range*range||(size==32&&squared>=distances[31]))continue;
            bool duplicate=false;for(unsigned j=0;j<size;++j)if(nearest[j]==target){duplicate=true;break;}if(duplicate)continue;
            unsigned slot=std::min(size,31u);while(slot&&squared<distances[slot-1]){if(slot<32){nearest[slot]=nearest[slot-1];distances[slot]=distances[slot-1];}--slot;}
            nearest[slot]=target;distances[slot]=squared;if(size<32)++size;
        }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&charge_errors139);}}
        unsigned budget=2000;
        for(unsigned i=0;i<size&&budget;++i){__try{
            auto target=nearest[i];
            Charge139 proof{};
            if(!charge_bind_target139(proof,target,field<uintptr_t>(target,0x214),&budget))continue;
            InterlockedIncrement(&charge_candidates148);
            // Original charge entry completes immediately inside this live
            // minimum range. Do not skip the nearest real opponent and charge
            // another man farther back while already in contact with him.
            if(distances[i]<=separation*separation){InterlockedIncrement(&charge_near_reject149);return false;}
            if(!reinterpret_cast<ChargeEligible148>(base+0x3e53d0)(reinterpret_cast<void*>(manager),actor,target))continue;
            InterlockedIncrement(&charge_eligibility148);
            float x=field<float>(target,0x48)-center[0],z=field<float>(target,0x50)-center[2],distance=std::hypot(x,z);
            if(!std::isfinite(distance)||distance<=0||distance>range)continue;
            proof.forward_x=x/distance;proof.forward_z=z/distance;selected=proof;found=true;break;
        }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&charge_errors139);}}
    }__finally{if(list.data)reinterpret_cast<void(__cdecl*)(void*)>(base+0xf04d0)(reinterpret_cast<void*>(list.data));}
    return found;
}
static void charge_try148(){
    auto actor=locomotion_actor();unsigned generation=possession_lease.generation;
    if((unsigned(InterlockedCompareExchange(&charge_physical_input148,0,0))&(FORWARD|RUN|BACKWARD|LEFT|RIGHT|ChargePrimary199))!=(FORWARD|RUN|ChargePrimary199)||!charge_admission146(actor))return;
    auto now=GetTickCount64();if(charge_probe_lease148==generation&&now-charge_probe_tick148<100)return;charge_probe_lease148=generation;charge_probe_tick148=now;
    sync_members130(unit);auto squad=field<uintptr_t>(actor,0x3b4);if(!charge_joined146(actor,squad)){InterlockedExchange(&charge_reject148,1);return;}
    Charge139 selected{};if(!charge_select148(actor,selected)){InterlockedExchange(&charge_reject148,2);return;}
    if(generation!=possession_lease.generation||!charge_admission146(actor)||!charge_target139(selected)||!charge_joined146(actor,squad))return;
    AcquireSRWLockExclusive(&charge_lock146);
    auto& c=charge_control146;c={};c.proof=selected;c.proof.actor=actor;c.proof.generation=generation;c.proof.active=true;
    c.unit=unit;c.squad=squad;c.target_unit=selected.target_unit;c.target_id=selected.target_unit_serial;c.physical_serial=field<unsigned>(actor,0x1c8);c.assigning=true;
    InterlockedExchange(&entry_movement125,0);InterlockedExchange(&state->input,0);
    InterlockedExchange(&charge_input_generation146,LONG(generation));InterlockedIncrement(&charge_starts139);InterlockedExchange(&charge_reject148,0);charge_publish146();
    ReleaseSRWLockExclusive(&charge_lock146);
    ChargeIntent139 intent{selected.target,selected.target_unit,selected.forward_x,selected.forward_z,0,0,{}};bool completed=false;
    __try{
        // Virtual154 is3d8dd0 for humans,459ea0 for horses; the latter owns
        // the rider3b9060 task. No copied peer task or unit charge is required.
        auto action=field<uintptr_t>(field<uintptr_t>(actor,0),0x154);
        reinterpret_cast<ChargeOrder139>(action)(reinterpret_cast<void*>(actor),&intent);completed=true;
    }__finally{
        AcquireSRWLockExclusive(&charge_lock146);
        if(c.proof.generation==generation){c.assigning=false;if(!completed)charge_end146();else charge_observe147();}
        ReleaseSRWLockExclusive(&charge_lock146);
    }
}
static void charge_boundary146(uintptr_t current_unit,bool after){
    if(current_unit!=unit)return;bool attempt=false;
    AcquireSRWLockExclusive(&charge_lock146);
    __try{__try{charge_initialize147();if(charge_control146.proof.active)charge_observe147();else attempt=!after;}
    __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&charge_errors139);charge_end146();}}
    __finally{ReleaseSRWLockExclusive(&charge_lock146);}
    if(attempt){__try{charge_try148();}__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&charge_errors139);}}
}
static void __fastcall charge_schedule148(void* actor,void*,const void* descriptor){
    auto physical=reinterpret_cast<uintptr_t>(actor);
    // Native unit charging remains unchanged for peers. The manually possessed
    // soldier's personal charge is selected above, not inherited from the unit.
    if(member_owned130(physical)&&physical==locomotion_actor()){InterlockedIncrement(&charge_unit_suppressed148);return;}
    original_charge_schedule148(actor,descriptor);
}
static void __fastcall charge_contact148(void* context,void*,uintptr_t one,uintptr_t two){
    unsigned before=0;bool observe=false;auto manager=reinterpret_cast<uintptr_t>(context);
    __try{auto common=environment?field<uintptr_t>(environment,8):0;
        observe=charge_input_locked146()&&common&&manager==field<uintptr_t>(common,0xcc)&&(one==locomotion_actor()||two==locomotion_actor());
        if(observe){
            auto physical=locomotion_actor();
            // Native3e5d40/4336b0 choose charge families from ACTUAL pose16.
            // A freshly constructed task alone is not proof of that pose.
            // Defer this personal queue call until original motion classifies
            // it; do not fabricate pose/clip/outcome. Native jumps retain their
            // own attack/defence classification and launch/landing authority.
            if(one==physical&&field<unsigned>(physical,0x1d0)!=16&&!native_mounted_jump140(physical)){InterlockedIncrement(&charge_pose_wait149);return;}
            before=field<unsigned>(manager,0x14);
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){observe=false;}
    original_charge_contact148(context,one,two);
    if(!observe)return;
    __try{
        auto count=field<unsigned>(manager,0x14),capacity=field<unsigned>(manager,0x10),list=field<uintptr_t>(manager,0x18);
        if(count!=before+1||count>capacity||count>65536||!list)return;auto record=field<uintptr_t>(list,before*4);
        if(!record||field<uintptr_t>(record,0)!=one||field<uintptr_t>(record,4)!=two)return;
        AcquireSRWLockExclusive(&charge_lock146);
        __try{if(charge_control146.proof.active&&charge_control146.proof.actor==locomotion_actor()){
            charge_control146.queued=true;charge_control146.contact_tick=charge_simulation148();InterlockedIncrement(&charge_contacts148);charge_publish146();
        }}__finally{ReleaseSRWLockExclusive(&charge_lock146);}
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&charge_errors139);}
}
static void create_native_charge147(){
    const unsigned char schedule[]={0x53,0x56,0x57,0x8b,0xf9,0x80,0xbf,0x84,3,0,0,1};
    const unsigned char contact[]={0x53,0x8b,0x5c,0x24,8,0x56,0x53,0x8b,0xf1};
    require(!memcmp(reinterpret_cast<void*>(base+0x3dacf0),schedule,sizeof schedule)&&!memcmp(reinterpret_cast<void*>(base+0x3c0bd0),contact,sizeof contact),"Native personal charge signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3dacf0),reinterpret_cast<void*>(&charge_schedule148),reinterpret_cast<void**>(&original_charge_schedule148))==MH_OK,"Native personal charge assignment hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3c0bd0),reinterpret_cast<void*>(&charge_contact148),reinterpret_cast<void**>(&original_charge_contact148))==MH_OK,"Native charge contact observation hook failed");
}
static void charge_log139(std::ostream& log){
    static LONG starts=-1,returns=-1,entries=-1,pairs=-1,errors=-1,queries=-1,contacts=-1,suppressed=-1;
    if(starts==charge_starts139&&returns==charge_returns146&&entries==charge_entries146&&pairs==charge_pairs146&&errors==charge_errors139&&queries==charge_queries148&&contacts==charge_contacts148&&suppressed==charge_unit_suppressed148)return;
    starts=charge_starts139;returns=charge_returns146;entries=charge_entries146;pairs=charge_pairs146;errors=charge_errors139;queries=charge_queries148;contacts=charge_contacts148;suppressed=charge_unit_suppressed148;
    log<<"{\"event\":\"personal_charge148\",\"tick_ms\":"<<GetTickCount64()<<",\"queries\":"<<queries<<",\"candidates\":"<<charge_candidates148<<",\"native_eligible\":"<<charge_eligibility148<<",\"starts\":"<<starts<<",\"native_task_entries\":"<<entries<<",\"native_pairs\":"<<pairs<<",\"native_contacts\":"<<contacts<<",\"unit_assignments_skipped\":"<<suppressed<<",\"returns\":"<<returns<<",\"on_rails\":"<<charge_input_locked146()<<",\"last_rejection\":"<<charge_reject148<<",\"pose_wait149\":"<<charge_pose_wait149<<",\"near_reject149\":"<<charge_near_reject149<<",\"actual_pose_entries149\":"<<charge_pose_entries149<<",\"last_pair_plan149\":"<<charge_pair_plan149<<",\"last_native_outcome149\":"<<charge_pair_outcome149<<",\"last_pair_kind149\":"<<charge_pair_kind149<<",\"last_player_clip149\":"<<charge_player_clip149<<",\"last_target_clip149\":"<<charge_target_clip149<<",\"errors\":"<<errors<<"}\n";
}
