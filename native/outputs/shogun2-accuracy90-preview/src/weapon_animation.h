#include "native_air99.h"
#include "native_recovery103.h"
#pragma once
#include "view152.h"
#include "ranged_task_completion.h"
#include "personal_weapon_profile.h"
#include "revolver65.h"
// Owned weapon sequencing runs at the controlled actor's native motion update.
// AI ranged tasks remain untouched for every other actor.
using PlayAnimation=void(__thiscall*)(void*,unsigned,unsigned,unsigned,float);
using ChangeState=void(__thiscall*)(void*,unsigned,unsigned,unsigned);
using RangedAction=void(__thiscall*)(void*);
using ChooseAnimation=unsigned(__thiscall*)(void*,const float*);
using ReleaseTime=float(__thiscall*)(void*,unsigned,unsigned);
using WeaponOrigin=void*(__thiscall*)(void*,float*,unsigned,unsigned);
static PlayAnimation original_play;
static ChangeState original_change;
static RangedAction original_draw,original_native_release;
static RangedAction original_reload_tick;
using RangedGate=bool(__thiscall*)(void*);
static RangedGate original_ready_gate,original_draw_gate,original_task_reload_gate,original_finish_gate;
static RangedGate original_task_animation_gate95;
static RangedGate original_task_lower_gate96,original_task_idle_gate96;
static volatile LONG lower_barrier_bypasses96,idle_barrier_bypasses96;
static volatile LONG animation_barrier_bypasses95,same_unit_ai_draws95,same_unit_ai_releases95;
static uintptr_t reload_actor;
static RangedAction original_animation_clock;
using AnimationEvents=bool(__thiscall*)(void*,void*,unsigned);
static AnimationEvents original_animation_events;
static volatile LONG mounted_clock_holds,mounted_root_skips,mounted_horse_task_bypasses;
static void log_mounted_sequence69(std::ostream& log){
    static LONG clock_seen=-1,root_seen=-1,task_seen=-1;
    LONG c=mounted_clock_holds,r=mounted_root_skips,t=mounted_horse_task_bypasses;
    if(c==clock_seen&&r==root_seen&&t==task_seen)return;
    clock_seen=c;root_seen=r;task_seen=t;
    log<<"{\"event\":\"mounted_control69\",\"tick_ms\":"<<GetTickCount64()
        <<",\"clock_holds\":"<<c<<",\"root_event_skips\":"<<r<<",\"horse_ai_task_bypasses\":"<<t<<"}\n";
}
static SRWLOCK mounted_pose_failure_lock=SRWLOCK_INIT;
static unsigned mounted_pose_failure,mounted_pose_failure_lease;
static uintptr_t mounted_pose_failure_actor;
struct MountedPoseDiagnostic {unsigned code,lease,actor,unit,clip,count,source;uint64_t tick;bool world_valid,hips_valid;float world[12],hips[12];};
static MountedPoseDiagnostic mounted_pose_diagnostic;
static void report_mounted_pose_failure(unsigned code,unsigned count=0,unsigned source=0,const float* world=nullptr,const float* hips=nullptr){
    MountedPoseDiagnostic sample{};sample.code=code;sample.lease=state->lease_generation;sample.actor=state->actor_id;
    sample.unit=state->unit_id;sample.count=count;sample.source=source;sample.tick=GetTickCount64();
    __try {sample.clip=field<unsigned>(possessed,0x1d4);
        if(world){memcpy(sample.world,world,sizeof sample.world);sample.world_valid=true;}
        if(hips){memcpy(sample.hips,hips,sizeof sample.hips);sample.hips_valid=true;}}
    __except(EXCEPTION_EXECUTE_HANDLER){}
    AcquireSRWLockExclusive(&mounted_pose_failure_lock);
    mounted_pose_failure=code;mounted_pose_failure_actor=possessed;mounted_pose_failure_lease=state->lease_generation;
    if(!mounted_pose_diagnostic.code||mounted_pose_diagnostic.lease!=sample.lease)mounted_pose_diagnostic=sample;
    ReleaseSRWLockExclusive(&mounted_pose_failure_lock);
}
static void log_mounted_pose_diagnostic(std::ostream& log){
    static unsigned logged_lease;MountedPoseDiagnostic sample;
    AcquireSRWLockShared(&mounted_pose_failure_lock);sample=mounted_pose_diagnostic;ReleaseSRWLockShared(&mounted_pose_failure_lock);
    if(!sample.code||logged_lease==sample.lease)return;logged_lease=sample.lease;
    log<<"{\"event\":\"mounted_pose_rejection65\",\"tick_ms\":"<<sample.tick<<",\"code\":"<<sample.code
        <<",\"lease\":"<<sample.lease<<",\"actor\":"<<sample.actor<<",\"unit\":"<<sample.unit<<",\"clip\":"<<sample.clip
        <<",\"bone_count\":"<<sample.count<<",\"source\":"<<sample.source;
    auto matrix=[&](const char* name,bool valid,const float* m){log<<",\""<<name<<"\":";if(!valid){log<<"null";return;}log<<'[';
        for(unsigned i=0;i<12;++i){if(i)log<<',';if(std::isfinite(m[i]))log<<m[i];else log<<"null";}log<<']';};
    matrix("world",sample.world_valid,sample.world);matrix("hips",sample.hips_valid,sample.hips);log<<"}\n";
}
static unsigned take_mounted_pose_failure(uintptr_t actor){
    AcquireSRWLockExclusive(&mounted_pose_failure_lock);
    unsigned code=mounted_pose_failure_actor==actor&&mounted_pose_failure_lease==state->lease_generation?mounted_pose_failure:0;
    mounted_pose_failure=0;
    ReleaseSRWLockExclusive(&mounted_pose_failure_lock);return code;
}
static volatile LONG reload_tick_stamp=-1;
static __declspec(thread) unsigned own_weapon_call;
enum {WEAPON_IDLE=0,WEAPON_START=1,WEAPON_DRAW=2,WEAPON_FIRED=3,WEAPON_RELOAD=4};
enum {AIM_LOWERED=0,AIM_READY=1,AIM_BOW_DRAWING=2};
struct FootAimLower {uintptr_t actor,clip;unsigned lease;};
static __declspec(thread) FootAimLower foot_aim_lower;
static uint32_t weapon_payload[13];
static bool valid_weapon_clip(uintptr_t,unsigned);
struct MountedDraw {uintptr_t actor,animation;unsigned lease,clip,hold_frame;float release;};
static MountedDraw mounted_draw{};
#include "mounted_blend_clock88.h"
static SRWLOCK mounted_blend_lock88=SRWLOCK_INIT;
static mountedblend88::Clock mounted_blend_clock88;
static mountedblend88::Key mounted_blend_key88(){
    return {possessed,mounted_draw.animation,state->lease_generation,state->draws_started};
}
static uint32_t mounted_blend_tick88(){return field<uint32_t>(field<uintptr_t>(environment,8),0x50);}
static constexpr unsigned MOUNTED_FORWARD_LEVEL=758; // Recovered RIDER_FIRE_FORWARD_LEVEL.
static bool mounted_draw_owned(uintptr_t actor){
    return mounted_draw.actor==actor&&mounted_draw.lease==state->lease_generation&&
        field<uintptr_t>(actor,0x474)==mounted_draw.animation&&field<unsigned>(actor,0x1d4)==mounted_draw.clip;
}
static bool mounted_carbine_native_profile(uintptr_t actor){
    auto db=field<uintptr_t>(actor,0x23c),clips=db?field<uintptr_t>(db,0xc):0;
    auto animation=clips?field<uintptr_t>(clips,MOUNTED_FORWARD_LEVEL*4):0;
    if(!animation||field<unsigned>(animation,0)!=MOUNTED_FORWARD_LEVEL)return false;
    auto asset=field<uintptr_t>(animation,4);if(!asset)return false;
    if(field<unsigned>(asset,0x64)!=57||!std::isfinite(field<float>(asset,0x90))||std::abs(field<float>(asset,0x90)-2.35f)>.0001f)return false;
    static const wchar_t path[]=L"animations/shogun_animation/battle/men/carbine/carbine_riders/c_car_f_fire_level.anim";
    auto length=field<unsigned>(asset,8),text=field<uintptr_t>(asset,16);
    return length==sizeof(path)/sizeof(wchar_t)-1&&text&&!memcmp(reinterpret_cast<void*>(text),path,length*sizeof(wchar_t));
}
static bool mounted_bow_native_profile85(uintptr_t actor){
    auto db=field<uintptr_t>(actor,0x23c),clips=db?field<uintptr_t>(db,0xc):0;
    auto animation=clips?field<uintptr_t>(clips,MOUNTED_FORWARD_LEVEL*4):0;
    if(!animation||field<unsigned>(animation,0)!=MOUNTED_FORWARD_LEVEL)return false;
    auto asset=field<uintptr_t>(animation,4);if(!asset)return false;
    if(field<unsigned>(asset,0x64)!=18||!std::isfinite(field<float>(asset,0x90))||std::abs(field<float>(asset,0x90)-.5f)>.0001f)return false;
    static const wchar_t path[]=L"animations/shogun_animation/battle/men/bow/bow_riders/rider_bow_f_fire.anim";
    auto length=field<unsigned>(asset,8),text=field<uintptr_t>(asset,16);
    return length==sizeof(path)/sizeof(wchar_t)-1&&text&&!memcmp(reinterpret_cast<void*>(text),path,length*sizeof(wchar_t));
}
static unsigned mounted_skip_raise85(uintptr_t actor,bool desired){
    return desired&&weapon_mounted()&&(mounted_carbine_native_profile(actor)||mounted_bow_native_profile85(actor))?1u:0u;
}
static bool start_mounted_draw(uintptr_t actor){
    mounted_draw={};
    if(!valid_weapon_clip(actor,MOUNTED_FORWARD_LEVEL))return false;
    float marker=reinterpret_cast<ReleaseTime>(base+0x3d2ef0)(reinterpret_cast<void*>(actor),MOUNTED_FORWARD_LEVEL,0);
    float step=field<float>(base,0x15b2448);
    if(!std::isfinite(marker)||!std::isfinite(step)||step<=0||step>.25f||marker<step*3||marker>30)return false;
    unsigned hold=unsigned(std::ceil(marker/step))-2;
    uintptr_t db=field<uintptr_t>(actor,0x23c),array=field<uintptr_t>(db,0xc),animation=field<uintptr_t>(array,MOUNTED_FORWARD_LEVEL*4);
    auto asset=field<uintptr_t>(animation,4);unsigned length=field<unsigned>(asset,0x64);
    if(length<4||length>30000||hold>=length-1)return false;
    own_weapon_call=1;
    __try {original_play(reinterpret_cast<void*>(actor),MOUNTED_FORWARD_LEVEL,1,0,1.f);}
    __finally {own_weapon_call=0;}
    if(field<uintptr_t>(actor,0x474)!=animation||field<unsigned>(actor,0x1d4)!=MOUNTED_FORWARD_LEVEL)return false;
    mounted_draw={actor,animation,state->lease_generation,MOUNTED_FORWARD_LEVEL,hold,marker};
    state->aim_clip=MOUNTED_FORWARD_LEVEL;state->aim_pose=AIM_BOW_DRAWING;++state->draws_started;
    if(mounted_bow_native_profile85(actor)){
        auto key=mounted_blend_key88();auto tick=mounted_blend_tick88();
        AcquireSRWLockExclusive(&mounted_blend_lock88);mounted_blend_clock88.begin(key,tick);ReleaseSRWLockExclusive(&mounted_blend_lock88);
    }
    return true;
}
static uintptr_t weapon_actor,weapon_context;
static unsigned weapon_clip;
static float release_time;
static volatile LONG manual_arrow_released;

struct FootRevolverCycle {
    uintptr_t actor{}, fire_animation{}, aim_animation{};
    unsigned lease{}, aim_clip{}, return_frame{}, aim_frames{}, clock_tick{}, extra_frame_credit{};
    bool has_clock_tick{};
};
static FootRevolverCycle foot_revolver;

static bool weapon_asset_is(uintptr_t asset, const wchar_t* path) {
    if(!asset)return false;
    const auto length=std::wcslen(path);
    const auto text=field<uintptr_t>(asset,16);
    return text&&field<unsigned>(asset,8)==length&&
        CompareStringOrdinal(reinterpret_cast<const wchar_t*>(text),int(length),
                             path,int(length),TRUE)==CSTR_EQUAL;
}

static FootRevolverCycle foot_revolver_profile(uintptr_t actor, uintptr_t fire_animation, float marker) {
    const unsigned aim_clip=equipment_crouched81()?127:124;
    if(!valid_weapon_clip(actor,aim_clip))return {};
    const auto clips=field<uintptr_t>(field<uintptr_t>(actor,0x23c),0xc);
    const auto aim_animation=field<uintptr_t>(clips,aim_clip*4);
    const auto aim_asset=field<uintptr_t>(aim_animation,4);
    const auto fire_asset=field<uintptr_t>(fire_animation,4);
    // Cut only the last two stock 20 Hz samples. After samples 34/30,
    // both hands and weapons already match poise within 19 mm and 2.3 degrees.
    if(field<unsigned>(fire_asset,0x64)==37&&field<unsigned>(aim_asset,0x64)==41&&std::abs(marker-.02f)<.0001f&&
       weapon_asset_is(fire_asset,L"animations/shogun_animation/battle/men/pistol/pistol_poise/pistol_fire.anim")&&
       weapon_asset_is(aim_asset,L"animations/shogun_animation/battle/men/pistol/pistol_poise/pistol_poise.anim"))
        return {actor,fire_animation,aim_animation,state->lease_generation,aim_clip,35,41};
    if(field<unsigned>(fire_asset,0x64)==33&&field<unsigned>(aim_asset,0x64)==43&&std::abs(marker-.12f)<.0001f&&
       weapon_asset_is(fire_asset,L"animations/shogun_animation/battle/men/pistol/pistol_poise/dual_pistol_fire.anim")&&
       weapon_asset_is(aim_asset,L"animations/shogun_animation/battle/men/pistol/pistol_poise/dual_pistol_poise.anim"))
        return {actor,fire_animation,aim_animation,state->lease_generation,aim_clip,31,43};
    return {};
}
static bool foot_revolver_shot(uintptr_t actor) {
    return foot_revolver.actor==actor&&actor==weapon_actor&&
        foot_revolver.lease==state->lease_generation&&foot_revolver.aim_clip;
}
static bool bow_draw_ready(){
    return is_bow_projectile(bow_record(unit))&&state->aim_pose==AIM_READY;
}
static bool valid_weapon_clip(uintptr_t actor,unsigned clip_id){
    uintptr_t db=field<uintptr_t>(actor,0x23c),array=db?field<uintptr_t>(db,0xc):0;
    if(!array||clip_id>=0x4a3)return false;
    uintptr_t clip=field<uintptr_t>(array,clip_id*4);
    return clip&&field<unsigned>(clip,0)==clip_id&&field<uintptr_t>(clip,4);
}
static bool weapon_owned(uintptr_t actor){
    __try {return enabled&&!state->stop&&!state->release&&state->mode==POSSESSED&&actor==possessed&&!dismount_native_motion143()&&field<unsigned char>(actor,0xf0)==1&&!native_airborne_subject99(actor)&&followed(actor,false)&&personal_weapon_mode(field<unsigned>(actor,0x1f0),weapon_mounted())&&bow_record(unit)!=0;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// RMB arrives on the window thread before the next native actor boundary.
// A lowered rifle can retain clip758/frame23 with animation474 cleared. Do
// not align that stale pose as if it were the new draw. The render transition
// must begin with the actual owned native animation, not the input edge.
static bool mounted_carbine_pose_aim(uintptr_t actor,bool requested){
    if(!requested||!weapon_mounted()||!weapon_owned(actor))return false;
    if(!state->weapon_phase)return state->aim_pose&&mounted_draw_owned(actor);
    if(actor!=weapon_actor||weapon_clip!=MOUNTED_FORWARD_LEVEL||
       (state->weapon_phase!=WEAPON_DRAW&&state->weapon_phase!=WEAPON_FIRED))return false;
    auto animation=field<uintptr_t>(actor,0x474);
    return animation&&field<unsigned>(animation,0)==weapon_clip&&field<unsigned>(actor,0x1d4)==weapon_clip;
}
static bool ranged_subject_owned(uintptr_t subject){
    if(weapon_owned(subject))return true;
    // Some mounted AI tasks address the horse principal rather than its rider.
    // The validated rider lease proves horse serial/config/holder identity;
    // also require the same unit and an exact reciprocal passenger link here.
    __try{
        if(!weapon_mounted()||!weapon_owned(possessed)||subject!=field<uintptr_t>(possessed,0x210)||
           field<uintptr_t>(subject,0)!=base+0x15e8524||field<unsigned char>(subject,0xf0)!=1||
           field<uintptr_t>(subject,0x214)!=unit)return false;
        unsigned count=field<unsigned>(subject,0x6cc);auto links=field<uintptr_t>(subject,0x6d0);
        if(count!=1||!links||field<uintptr_t>(links,0)!=possessed)return false;
        InterlockedIncrement(&mounted_horse_task_bypasses);return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
#include "ranged_position128.h"
static bool mounted_clock_held(uintptr_t actor){
    __try{return actor==possessed&&weapon_mounted()&&weapon_owned(actor)&&!state->weapon_phase&&
        weapon_ready_held()&&mounted_draw_owned(actor)&&field<unsigned>(actor,0x1d8)>=mounted_draw.hold_frame;}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
#include "weapon_observation82.h"
static bool mounted_bow_blend_ready88(){
    auto key=mounted_blend_key88();
    AcquireSRWLockShared(&mounted_blend_lock88);bool ready=mounted_blend_clock88.ready(key);ReleaseSRWLockShared(&mounted_blend_lock88);
    return ready;
}
static bool advance_foot_revolver(uintptr_t actor) {
    __try {
        if(!state||!foot_revolver_shot(actor)||
           (state->weapon_phase!=WEAPON_DRAW&&state->weapon_phase!=WEAPON_FIRED)||
           field<uintptr_t>(actor,0x474)!=foot_revolver.fire_animation||!weapon_owned(actor))
            return false;
        const auto common=environment?field<uintptr_t>(environment,8):0;
        if(!common)return false;
        const auto tick=field<unsigned>(common,0x50);
        if(!foot_revolver.has_clock_tick||tick!=foot_revolver.clock_tick){
            foot_revolver.has_clock_tick=true;
            foot_revolver.clock_tick=tick;
            foot_revolver.extra_frame_credit+=11;
        }
        if(foot_revolver.extra_frame_credit<25)return false;
        const float frame_time=float(field<unsigned>(actor,0x1d8))*field<float>(base,0x15b2448);
        // Give the simulation boundary its chance to fire at the original
        // marker before an extra native recoil step can move past it.
        if(state->weapon_phase==WEAPON_DRAW&&frame_time>=release_time)return false;
        foot_revolver.extra_frame_credit-=25;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool resume_foot_revolver_aim(uintptr_t actor);

static bool foot_revolver_aim_held(uintptr_t actor){
    return state&&foot_revolver_shot(actor)&&weapon_ready_held()&&
        (equipment_crouched81()||!(state->input&RUN))&&weapon_owned(actor)&&
        field<float>(actor,0x390)<=0&&field<float>(unit,0xf78)>=1;
}

static bool finish_foot_revolver_tail(uintptr_t actor){
    __try{
        if(!foot_revolver_aim_held(actor)||state->weapon_phase!=WEAPON_FIRED||
           field<uintptr_t>(actor,0x474)!=foot_revolver.fire_animation||
           field<unsigned>(actor,0x1d8)<foot_revolver.return_frame||
           !field<unsigned>(actor,0x394)||!resume_foot_revolver_aim(actor))return false;
        InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}

static void wrap_foot_revolver_aim(uintptr_t actor){
    __try{
        if(foot_revolver_aim_held(actor)&&!state->weapon_phase&&state->aim_pose==AIM_READY&&
           field<uintptr_t>(actor,0x474)==foot_revolver.aim_animation&&
           field<unsigned>(actor,0x1d8)==foot_revolver.aim_frames-1){
            // The native clock clears the active clip at the duplicate seam.
            // Keep the same poise active and advance the first sample instead.
            field<unsigned>(actor,0x1d8)=0;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}

static void finish_foot_revolver_clock(uintptr_t actor,uintptr_t previous_animation){
    __try{
        if(!previous_animation||!state||!foot_revolver_shot(actor)||
           field<uintptr_t>(actor,0x474)||!weapon_owned(actor))
            return;
        const LONG phase=state->weapon_phase;
        const bool recoil_finished=phase==WEAPON_FIRED&&
            previous_animation==foot_revolver.fire_animation&&field<unsigned>(actor,0x394)!=0;
        const bool reload_finished=phase==WEAPON_RELOAD&&field<float>(actor,0x390)<=0;
        // Restore native poise before returning to the actor's caller. An
        // animation-free interval can hand the renderer a lowered fallback.
        if((recoil_finished||reload_finished)&&resume_foot_revolver_aim(actor))
            InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}

static void __fastcall weapon_animation_clock_hook(void* actor,void*){
    if(reinterpret_cast<uintptr_t>(actor)==possessed&&weapon_mounted()&&weapon_owned(possessed)&&
       mounted_draw_owned(possessed)&&mounted_bow_native_profile85(possessed)){
        auto key=mounted_blend_key88();auto tick=mounted_blend_tick88();float delta=field<float>(base,0x15b2448);
        AcquireSRWLockExclusive(&mounted_blend_lock88);mounted_blend_clock88.advance(key,tick,delta);ReleaseSRWLockExclusive(&mounted_blend_lock88);
    }
    if(mounted_clock_held(reinterpret_cast<uintptr_t>(actor))){
        // Stop before the native increment/root event, rather than rewinding
        // after it. Other actors and the horse retain their original clocks.
        // Native 3e4850 sets ready mode32 to state48 on every update.
        // Native 3f0370 then restores active-animation state33 and clip ID.
        // Preserve those side effects while withholding increment/root motion.
        auto p=reinterpret_cast<uintptr_t>(actor);
        field<unsigned>(p,0x1d4)=mounted_draw.clip;
        field<unsigned>(p,0x1d0)=0x21;
        InterlockedIncrement(&mounted_clock_holds);state->aim_pose=AIM_READY;return;
    }
    auto caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
    const auto subject=reinterpret_cast<uintptr_t>(actor);
    const auto previous_animation=state&&foot_revolver_shot(subject)?field<uintptr_t>(subject,0x474):0;
    observe_weapon82(subject,1,caller);
    wrap_foot_revolver_aim(subject);
    if(!finish_foot_revolver_tail(subject)){
        original_animation_clock(actor);
        if(!finish_foot_revolver_tail(subject)){
            // Keep the existing 36/25 rate. The native play scalar is root
            // displacement, and does not change the animation clock.
            if(advance_foot_revolver(subject))original_animation_clock(actor);
            if(!finish_foot_revolver_tail(subject))finish_foot_revolver_clock(subject,previous_animation);
        }
    }
    observe_weapon82(subject,2,caller);
}
static bool __fastcall weapon_animation_events_hook(void* actor,void*,void* clip,unsigned frame){
    if(mounted_clock_held(reinterpret_cast<uintptr_t>(actor))&&
       reinterpret_cast<uintptr_t>(clip)==mounted_draw.animation&&frame>=mounted_draw.hold_frame){
        // This native routine adds the frame's root motion to actor intent.
        // A held frame must not add that displacement again on each update.
        InterlockedIncrement(&mounted_root_skips);return false;
    }
    return original_animation_events(actor,clip,frame);
}
static void __fastcall weapon_reload_tick_hook(void* actor,void*){
    if(weapon_owned(reinterpret_cast<uintptr_t>(actor))){
        if(reload_actor!=reinterpret_cast<uintptr_t>(actor)){reload_actor=reinterpret_cast<uintptr_t>(actor);InterlockedExchange(&reload_tick_stamp,-1);}
        LONG tick=field<LONG>(field<uintptr_t>(environment,8),0x50);
        // Native AI and our actor boundary may both request this update. Execute once.
        if(InterlockedExchange(&reload_tick_stamp,tick)==tick)return;
    }
    original_reload_tick(actor);
}
// Diagnostic counts only: never authorizes a task or changes native behavior.
static void observe_peer_fire95(uintptr_t subject,bool release){
    if(!enabled||!state||state->mode!=POSSESSED||!unit||!subject||subject==possessed)return;
    __try {if(field<uintptr_t>(subject,0x214)==unit)
        InterlockedIncrement(release?&same_unit_ai_releases95:&same_unit_ai_draws95);}
    __except(EXCEPTION_EXECUTE_HANDLER){}
}
static void __fastcall weapon_draw_hook(void* task,void*){
    if(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4))){
        acknowledge_skipped_ranged_draw(task);++state->ai_shots_suppressed;return;
    }
    command_draw_started100(task);
    observe_peer_fire95(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4),false);
    original_draw(task);
}
static void __fastcall weapon_release_hook(void* task,void*){
    if(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4))){acknowledge_skipped_ranged_release(task);return;}
    if(command_release_managed100(task))return;
    observe_peer_fire95(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4),true);
    original_native_release(task);
}
// These queries belong to the AI ranged task, not the actor's animation player.
// The controlled soldier is non-blocking for the squad's aim/reload barriers.
// Manual draw/release/reload still uses the actor's real animation and timer.
static bool __fastcall weapon_ready_gate(void* task,void*){
    return ranged_task_nonblocking(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4)),task,original_ready_gate);
}
static bool __fastcall weapon_draw_gate(void* task,void*){
    if(!command_draw_permitted100(task))return false;
    return ranged_task_nonblocking(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4)),task,original_draw_gate);
}
static bool __fastcall weapon_task_reload_gate(void* task,void*){
    return ranged_task_nonblocking(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4)),task,original_task_reload_gate);
}
static bool __fastcall weapon_finish_gate(void* task,void*){
    return ranged_task_nonblocking(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4)),task,original_finish_gate);
}
// Original native state3b2f50 calls3d3480 ->3d3e90 directly, bypassing
// all four older gates. This is a RANGED TASK query (+4 actor), not the
// actor-global animation-completion vmethod. Only the proven owned subject
// must be non-blocking while its personal animation is player-controlled.
static bool __fastcall weapon_task_animation_gate95(void* task,void*){
    if(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4))){
        InterlockedIncrement(&animation_barrier_bypasses95);return true;
    }
    return original_task_animation_gate95(task);
}
// These native task-local queries call actor3c2030 directly; they never pass
// through the five existing gates. Do not change the global actor predicate.
static bool __fastcall weapon_task_lower_gate96(void* task,void*){
    if(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4))){InterlockedIncrement(&lower_barrier_bypasses96);return true;}
    return original_task_lower_gate96(task);
}
static bool __fastcall weapon_task_idle_gate96(void* task,void*){
    if(ranged_subject_owned(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4))){InterlockedIncrement(&idle_barrier_bypasses96);return true;}
    return original_task_idle_gate96(task);
}
#include "melee_presentation137.h"
static void __fastcall weapon_play_hook(void* actor,void*,unsigned clip,unsigned force,unsigned attached,float speed){
    if(!own_weapon_call&&recovery_clip103(clip)&&weapon_owned(reinterpret_cast<uintptr_t>(actor))){
        state->weapon_phase=WEAPON_IDLE;state->aim_pose=0;InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);
        original_play(actor,clip,force,attached,speed);return;
    }
    if(!own_weapon_call&&(state->weapon_phase||state->aim_pose)&&weapon_owned(reinterpret_cast<uintptr_t>(actor)))return;
    if(!own_weapon_call&&weapon_owned(reinterpret_cast<uintptr_t>(actor))&&equipment_slot81==equipment81::Melee&&!equipment_personal_override214&&
       ((clip>=124&&clip<=140)||clip==750||clip==751||(clip>=753&&clip<=786)))return;
    original_play(actor,clip,force,attached,speed);
}
static void __fastcall weapon_change_hook(void* actor,void*,unsigned mode,unsigned skip,unsigned attached){
    if(!own_weapon_call&&(state->weapon_phase||state->aim_pose)&&weapon_owned(reinterpret_cast<uintptr_t>(actor))){
        // Let death, impacts and melee transitions interrupt the sequence.
        if(prone_mode103(mode)||!personal_weapon_mode(mode,weapon_mounted())){InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->aim_pose=0;InterlockedExchange(&state->aiming,0);}
        else return;
    }
    if(!own_weapon_call&&weapon_owned(reinterpret_cast<uintptr_t>(actor))){
        if(equipment_slot81==equipment81::Melee&&!equipment_personal_override214){
            if(mode==14||mode==15||mode==16||mode==17||mode==32||mode==33)return;

        }else if(!weapon_mounted()){
            if(equipment_crouched81()){if(mode==8)mode=9;else if(mode==14)mode=15;else if(mode==16)mode=17;}
            else {if(mode==9)mode=8;else if(mode==15)mode=14;else if(mode==17)mode=16;}
        }
    }
    if(!own_weapon_call)mode=melee_presentation_mode137(reinterpret_cast<uintptr_t>(actor),mode,attached);
    original_change(actor,mode,skip,attached);
}
static void begin_weapon_impl(const uint32_t* payload,uintptr_t context){
    __try {
        if(weapon_owned(payload[11])&&native_recovery103(payload[11])){InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->aim_pose=0;InterlockedExchange(&state->fire_request,0);++state->shots_canceled;return;}
        if(!weapon_ready_held()){++state->shots_canceled;return;}
        if(!weapon_owned(payload[11])||state->weapon_phase||field<float>(payload[11],0x390)>0){state->shot_error=87;return;}
        weapon_actor=payload[11];weapon_context=context;memcpy(weapon_payload,payload,sizeof(weapon_payload));
        foot_revolver={};
        InterlockedExchange(&manual_arrow_released,0);
        if(weapon_mounted()){
            if(state->aim_pose!=AIM_READY||!mounted_draw_owned(weapon_actor)){
                ++state->shots_blocked;state->shot_error=97;return;
            }
            // Continue the same mounted animation from its pre-release pose.
            // Do not restart the draw or substitute an infantry clip.
            weapon_clip=mounted_draw.clip;release_time=mounted_draw.release;mounted_draw={};
            state->aim_pose=0;InterlockedExchange(&state->weapon_phase,WEAPON_DRAW);
            state->weapon_record=field<unsigned>(bow_record(unit),0);state->weapon_animation=weapon_clip;
            state->weapon_release_time=release_time;state->shot_error=0;return;
        }
        const auto record=bow_record(unit);
        const bool revolver=revolver_record65(record);
        const bool drawn_bow=bow_draw_ready();
        const unsigned ready_mode=equipment81::ready_mode(false,equipment_crouched81());
        const bool already_aiming=revolver&&state->aim_pose==AIM_READY;
        if(!already_aiming){
            if(state->aim_pose)reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(weapon_actor));
            state->aim_pose=0;
            // A drawn bow proceeds directly to its native release clip.
            if(!drawn_bow)original_change(reinterpret_cast<void*>(weapon_actor),ready_mode,0,0);
        }
        InterlockedExchange(&state->weapon_phase,WEAPON_START);
        weapon_clip=reinterpret_cast<ChooseAnimation>(base+0x3d28b0)(reinterpret_cast<void*>(weapon_actor),reinterpret_cast<float*>(weapon_payload+5));
        // The native task always evaluates its primary shot. Missing markers use time zero.
        uintptr_t db=field<uintptr_t>(weapon_actor,0x23c),array=field<uintptr_t>(db,0xc);
        if(weapon_clip>=0x4a3||!array){state->shot_error=88;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);return;}
        uintptr_t anim=field<uintptr_t>(array,weapon_clip*4);
        if(!anim||field<unsigned>(anim,0)!=weapon_clip||!field<uintptr_t>(anim,4)){
            state->shot_error=88;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);return;
        }
        release_time=reinterpret_cast<ReleaseTime>(base+0x3d2ef0)(reinterpret_cast<void*>(weapon_actor),weapon_clip,0);
        if(!std::isfinite(release_time)||release_time<0||release_time>30){state->shot_error=89;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);return;}
        if(revolver)foot_revolver=foot_revolver_profile(weapon_actor,anim,release_time);
        if(already_aiming){
            if(foot_revolver.aim_clip){
                if(field<unsigned>(weapon_actor,0x1f0)!=ready_mode)
                    original_change(reinterpret_cast<void*>(weapon_actor),ready_mode,1,0);
                original_play(reinterpret_cast<void*>(weapon_actor),weapon_clip,0,0,1.f);
                if(field<uintptr_t>(weapon_actor,0x474)==anim)
                    InterlockedExchange(&state->weapon_phase,WEAPON_DRAW);
            }else{
                reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(weapon_actor));
                state->aim_pose=0;
                original_change(reinterpret_cast<void*>(weapon_actor),ready_mode,0,0);
            }
        }
        state->weapon_record=field<unsigned>(record,0);state->weapon_animation=weapon_clip;state->weapon_release_time=release_time;
        ++state->draws_started;state->shot_error=0;
    }__except(EXCEPTION_EXECUTE_HANDLER){state->shot_error=90;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);InterlockedExchange(&state->release,1);}
}
static void begin_weapon(const uint32_t* payload,uintptr_t context){
    own_weapon_call=1;
    __try {begin_weapon_impl(payload,context);}__finally {own_weapon_call=0;}
}
static bool resume_foot_revolver_aim(uintptr_t actor) {
    if(!foot_revolver_aim_held(actor))return false;
    const unsigned clip=foot_revolver.aim_clip;
    if(!valid_weapon_clip(actor,clip))return false;
    const unsigned ready_mode=equipment81::ready_mode(false,equipment_crouched81());
    own_weapon_call=1;
    __try {
        if(field<unsigned>(actor,0x1f0)!=ready_mode)
            original_change(reinterpret_cast<void*>(actor),ready_mode,1,0);
        original_play(reinterpret_cast<void*>(actor),clip,0,0,1.f);
    } __finally {own_weapon_call=0;}
    if(field<uintptr_t>(actor,0x474)!=foot_revolver.aim_animation||field<unsigned>(actor,0x1d4)!=clip)return false;
    state->aim_clip=clip;
    state->aim_pose=AIM_READY;
    return true;
}
static void weapon_tick(uintptr_t actor){
    __try {
        if(weapon_owned(actor)&&native_recovery103(actor)){
            state->weapon_phase=WEAPON_IDLE;state->aim_pose=0;InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);return;
        }
        if(!weapon_owned(actor)){mounted_draw={};InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->aim_pose=0;return;}
        // A rejected rendered pose lowers the owned weapon at its simulation
        // boundary. Keep FPS ownership; invalid actor/engine contexts still use
        // the original release guards. No animation calls from a render thread.
        unsigned pose_fault=weapon_mounted()?take_mounted_pose_failure(actor):0;
        if(pose_fault&&weapon_mounted()){
            own_weapon_call=1;
            __try {reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(actor));
                original_change(reinterpret_cast<void*>(actor),personal_lower_mode(true),0,0);}
            __finally {own_weapon_call=0;}
            state->aim_pose=0;InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);
            InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->pose_error=unsigned(pose_fault);return;
        }
        weapon_reload_tick_hook(reinterpret_cast<void*>(actor),nullptr);
        state->weapon_reload_remaining=field<float>(actor,0x390);
        state->native_pose_clip=field<unsigned>(actor,0x1d4);
        if(actor==weapon_actor&&(state->weapon_phase==WEAPON_START||state->weapon_phase==WEAPON_DRAW)&&!weapon_ready_held()){
            own_weapon_call=1;
            reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(actor));
            original_change(reinterpret_cast<void*>(actor),equipment81::lower_mode(weapon_mounted(),equipment_crouched81()),0,0);own_weapon_call=0;
            InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->aim_pose=0;++state->shots_canceled;
        }
        if(!state->weapon_phase){
            uintptr_t record=bow_record(unit);
            bool from_type=false;
            const bool bow=native_personal_kind(record,&from_type)==PersonalWeaponKind::Bow;
            const bool mounted=weapon_mounted();
            bool desired=weapon_ready_held()&&state->weapon_reload_remaining<=0&&(equipment_crouched81()||mounted||!(state->input&RUN))&&field<float>(unit,0xf78)>=1;
            if(desired&&!mounted&&(bow||from_type)&&
               ((bow&&!valid_weapon_clip(actor,bow_raise_clip(false)))||
                !valid_weapon_clip(actor,equipment_crouched81()&&!bow?127:bow_hold_clip(false)))){
                state->shot_error=95;
                InterlockedExchange(&state->fire_request,0);
                desired=false;
            }
            if(desired!=bool(state->aim_pose)){
                foot_aim_lower={};
                if(state->aim_pose)reinterpret_cast<RangedAction>(base+0x3ca920)(reinterpret_cast<void*>(actor));
                // Native29->32 starts bow/rifle clip751. We immediately start758
                // below; playing both poisons the draw's initial blend history.
                // Native change-mode's second argument skips ONLY automatic
                // transition playback after updating the mode (verified asm).
                unsigned skip_transition=mounted_skip_raise85(actor,desired);
                own_weapon_call=1;original_change(reinterpret_cast<void*>(actor),desired?equipment81::ready_mode(weapon_mounted(),equipment_crouched81()):equipment81::lower_mode(weapon_mounted(),equipment_crouched81()),skip_transition,0);own_weapon_call=0;
                state->aim_pose=desired?AIM_READY:AIM_LOWERED;
                if(!desired&&!weapon_mounted()&&!equipment_crouched81()&&
                   field<unsigned>(actor,0x1f0)==equipment81::lower_mode(false,false)){
                    // Only the clip installed by this owned ADS exit may yield to walking.
                    foot_aim_lower={actor,field<uintptr_t>(actor,0x474),state->lease_generation};
                }
                if(weapon_mounted()){
                    mounted_draw={};
                    if(desired){
                        InterlockedExchange(&manual_arrow_released,0);
                        if(!start_mounted_draw(actor)){
                            own_weapon_call=1;
                            __try {original_change(reinterpret_cast<void*>(actor),personal_lower_mode(true),0,0);}
                            __finally {own_weapon_call=0;}
                            state->aim_pose=AIM_LOWERED;InterlockedExchange(&state->aiming,0);
                            InterlockedExchange(&state->fire_request,0);state->shot_error=98;return;
                        }
                    }
                }else if(desired&&bow){
                    InterlockedExchange(&manual_arrow_released,0);
                    // Native asset126 is bow_stand_to_aim_25: raise and draw.
                    // Native asset124 is bow_aim_25: hold at full draw.
                    own_weapon_call=1;original_play(reinterpret_cast<void*>(actor),bow_raise_clip(weapon_mounted()),1,0,1.0f);own_weapon_call=0;
                    state->aim_pose=bow?AIM_BOW_DRAWING:AIM_READY;state->aim_clip=bow_raise_clip(weapon_mounted());++state->draws_started;
                }
            }
            if(desired&&weapon_mounted()){
                if(!mounted_draw_owned(actor)){
                    mounted_draw={};state->aim_pose=AIM_LOWERED;state->shot_error=99;
                    InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);return;
                }
                if(field<unsigned>(actor,0x1d8)>=mounted_draw.hold_frame){
                    // The scoped native clock hook holds before advancement.
                    // This observer never rewinds a live animation frame.
                    state->aim_pose=AIM_READY;
                }
            }else if(desired&&!field<uintptr_t>(actor,0x474)){
                // Native standing AIM/POISE clip, distinct from its transition.
                unsigned hold_clip=equipment_crouched81()?127:bow_hold_clip(weapon_mounted());
                if(valid_weapon_clip(actor,hold_clip)){
                    own_weapon_call=1;original_play(reinterpret_cast<void*>(actor),hold_clip,1,0,1.0f);own_weapon_call=0;state->aim_clip=hold_clip;
                    state->aim_pose=AIM_READY;
                }
            }
        }
        if(actor!=weapon_actor||!state->weapon_phase)return;
        uintptr_t anim=field<uintptr_t>(actor,0x474);
        if(state->weapon_phase==WEAPON_START){
            if(!anim){own_weapon_call=1;original_play(reinterpret_cast<void*>(actor),weapon_clip,1,0,1.0f);own_weapon_call=0;InterlockedExchange(&state->weapon_phase,WEAPON_DRAW);}
        }else if(state->weapon_phase==WEAPON_DRAW){
            if(!anim||field<unsigned>(anim,0)!=weapon_clip){observe_weapon82(actor,5,0);state->shot_error=91;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);return;}
            float phase=float(field<unsigned>(actor,0x1d8))*field<float>(base,0x15b2448);
            if(phase>=release_time){
                float origin[3]{};
                reinterpret_cast<WeaponOrigin>(base+0x3e0b10)(reinterpret_cast<void*>(actor),origin,weapon_clip,0);
                if(is_bow_projectile(bow_record(unit))&&!third_person152){
                    float previous[3];memcpy(previous,origin,sizeof previous);
                    if(!posed_arrow_origin(origin)){
                        ++state->bow_attachment_misses;state->shot_error=96;
                        InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);
                        InterlockedExchange(&state->release,1);return;
                    }
                    ++state->bow_attachment_origins;
                    for(unsigned i=0;i<3;++i)state->bow_attachment_origin[i]=origin[i];
                    float squared=0;for(unsigned i=0;i<3;++i){float d=origin[i]-previous[i];squared+=d*d;}
                    state->bow_origin_correction=std::sqrt(squared);++state->bow_origin_aligned;
                }else{
                    pose_manual_origin(origin);state->bow_origin_correction=0;
                }
                ViewRay ray{};
                if(!read_aim_frame(ray)){state->shot_error=94;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);InterlockedExchange(&state->release,1);return;}
                // Bow uses the actual rendered Weapon3 attachment, already pitched.
                // Matchlock retains its native metadata origin and existing pose transform.
                for(unsigned i=0;i<3;++i)state->shot_camera_origin[i]=ray.origin[i];
                float aim[3];crosshair_target(ray,aim,personal_direct_projectile(bow_record(unit))?6000.0f:80.0f);
                for(unsigned i=0;i<3;++i)state->shot_view_direction[i]=ray.direction[i];
                memcpy(weapon_payload+2,origin,sizeof(origin));memcpy(weapon_payload+5,aim,sizeof(aim));
                for(unsigned i=0;i<3;++i)state->shot_target[i]=aim[i];
                InterlockedExchange(&state->weapon_phase,WEAPON_FIRED);
                unsigned previous_shots=state->shots_spawned;
                manual_bow_apply(weapon_payload,weapon_context);
                if(state->shots_spawned==previous_shots+1)command_on_shot100();
                InterlockedExchange(&manual_arrow_released,is_bow_projectile(bow_record(unit))&&state->shots_spawned==previous_shots+1);
            }
        }else if(state->weapon_phase==WEAPON_FIRED&&!anim){
            // The revolver reloads only after the last round. Between rounds
            // the stock recoil endpoint blends directly into its poise loop.
            const auto record=bow_record(unit);
            if(!reload_after_recoil65(actor,record)||(state->shot_error&&revolver_record65(record))){
                resume_foot_revolver_aim(actor);
                InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);
                return;
            }
            if(foot_revolver_shot(actor))state->aim_pose=AIM_LOWERED;
            own_weapon_call=1;original_change(reinterpret_cast<void*>(actor),equipment81::reload_mode(weapon_mounted(),equipment_crouched81()),0,0);own_weapon_call=0;
            ++state->reloads_started;InterlockedExchange(&state->weapon_phase,WEAPON_RELOAD);
        }else if(state->weapon_phase==WEAPON_RELOAD&&field<float>(actor,0x390)<=0&&!anim){
            if(!resume_foot_revolver_aim(actor)){
                own_weapon_call=1;original_change(reinterpret_cast<void*>(actor),equipment81::lower_mode(weapon_mounted(),equipment_crouched81()),0,0);own_weapon_call=0;
            }
            InterlockedExchange(&manual_arrow_released,0);
            InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){own_weapon_call=0;state->shot_error=92;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);InterlockedExchange(&state->release,1);}
}
static void create_weapon_hooks(){
    create_ranged_position128();
    const unsigned char stop82[]={0x56,0x8b,0xf1,0x8b,0x86,0x74,4,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x3ca920),stop82,sizeof stop82),"Animation stop observation signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3ca920),reinterpret_cast<void*>(&weapon_animation_stop_hook82),reinterpret_cast<void**>(&original_animation_stop82))==MH_OK,"Animation stop observation hook failed");
    const unsigned char clock[]={0x56,0x8b,0xf1,0x8b,0x8e,0x74,4,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x3f0370),clock,sizeof clock),"Active animation clock signature differs");
    const unsigned char events[]={0x51,0x53,0x56,0x57,0x8b,0x7c,0x24,0x14};
    require(!memcmp(reinterpret_cast<void*>(base+0x3f03d0),events,sizeof events),"Animation root event signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3f0370),reinterpret_cast<void*>(&weapon_animation_clock_hook),reinterpret_cast<void**>(&original_animation_clock))==MH_OK,"Mounted clock hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3f03d0),reinterpret_cast<void*>(&weapon_animation_events_hook),reinterpret_cast<void**>(&original_animation_events))==MH_OK,"Mounted root event hook failed");
    const unsigned char task_animation95[]={0x8b,0x49,0x04,0x8b,0x01,0x8b,0x80,0xf8,0,0,0,0xff,0xe0};
    const unsigned char task_thunk95[]={0xe9,0x0b,0x0a,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x3d3e90),task_animation95,sizeof task_animation95)&&
            !memcmp(reinterpret_cast<void*>(base+0x3d3480),task_thunk95,sizeof task_thunk95),"Ranged task animation barrier signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3e90),reinterpret_cast<void*>(&weapon_task_animation_gate95),reinterpret_cast<void**>(&original_task_animation_gate95))==MH_OK,"Ranged task animation barrier hook failed");
    const unsigned char lower96[]={0x8b,0x49,4,0xe9,0xc8,0xe1,0xfe,0xff};
    const unsigned char idle96[]={0x8b,0x49,4,0xe8,0x68,0xe0,0xfe,0xff};
    require(!memcmp(reinterpret_cast<void*>(base+0x3d3e60),lower96,sizeof lower96)&&
        !memcmp(reinterpret_cast<void*>(base+0x3d3fc0),idle96,sizeof idle96),"Ranged lower/idle completion signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3e60),reinterpret_cast<void*>(&weapon_task_lower_gate96),reinterpret_cast<void**>(&original_task_lower_gate96))==MH_OK,"Ranged lower task hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3fc0),reinterpret_cast<void*>(&weapon_task_idle_gate96),reinterpret_cast<void**>(&original_task_idle_gate96))==MH_OK,"Ranged idle task hook failed");
    // Engine identity is checked before any hooks. ABI recovered from the ret instructions.
    const unsigned char play[]={0x83,0xec,0x14,0x80,0x7c,0x24,0x1c,1};
    const unsigned char release[]={0x81,0xec,0xe4,0,0,0,0x56,0x8b};
    require(memcmp(reinterpret_cast<void*>(base+0x3e8b50),play,8)==0&&memcmp(reinterpret_cast<void*>(base+0x3d4320),release,8)==0,"Weapon animation signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3e8b50),reinterpret_cast<void*>(&weapon_play_hook),reinterpret_cast<void**>(&original_play))==MH_OK,"Weapon animation hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3e3f50),reinterpret_cast<void*>(&weapon_change_hook),reinterpret_cast<void**>(&original_change))==MH_OK,"Weapon state hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3630),reinterpret_cast<void*>(&weapon_draw_hook),reinterpret_cast<void**>(&original_draw))==MH_OK,"AI draw hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d4320),reinterpret_cast<void*>(&weapon_release_hook),reinterpret_cast<void**>(&original_native_release))==MH_OK,"AI release hook failed");
    const unsigned char tick[]={0xf3,0x0f,0x10,0x81,0x90,3,0,0};
    require(memcmp(reinterpret_cast<void*>(base+0x3e1a20),tick,8)==0,"Reload tick signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3e1a20),reinterpret_cast<void*>(&weapon_reload_tick_hook),reinterpret_cast<void**>(&original_reload_tick))==MH_OK,"Reload tick hook failed");
    const unsigned char ready[]={0x56,0x8b,0xf1,0x8b,0x4e,4,0x80,0xb9,0x1f,1,0,0,0};
    const unsigned char draw[]={0x51,0x56,0x8b,0xf1,0x80,0x7e,0x7c,1};
    const unsigned char reload[]={0x56,0x8b,0xf1,0x8b,0x46,0x44,0x83,0xf8,7};
    const unsigned char finish[]={0x80,0x79,0x6e,0,0x74};
    require(!memcmp(reinterpret_cast<void*>(base+0x3d3ce0),ready,sizeof ready)&&
        !memcmp(reinterpret_cast<void*>(base+0x3d33f0),draw,sizeof draw)&&
        !memcmp(reinterpret_cast<void*>(base+0x3d3d40),reload,sizeof reload)&&
        !memcmp(reinterpret_cast<void*>(base+0x3d3de0),finish,sizeof finish),"Ranged task readiness signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3ce0),reinterpret_cast<void*>(&weapon_ready_gate),reinterpret_cast<void**>(&original_ready_gate))==MH_OK,"Ranged aim gate hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d33f0),reinterpret_cast<void*>(&weapon_draw_gate),reinterpret_cast<void**>(&original_draw_gate))==MH_OK,"Ranged draw gate hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3d40),reinterpret_cast<void*>(&weapon_task_reload_gate),reinterpret_cast<void**>(&original_task_reload_gate))==MH_OK,"Ranged reload gate hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3d3de0),reinterpret_cast<void*>(&weapon_finish_gate),reinterpret_cast<void**>(&original_finish_gate))==MH_OK,"Ranged finish gate hook failed");
}
