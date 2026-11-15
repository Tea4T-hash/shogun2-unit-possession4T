#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "../../shogun2-possession-cleanup-v46/src/friendly_fire_protocol109.h"
#include <array>
#include <intrin.h>
namespace friendly109 {
struct Tag {uintptr_t projectile{},environment{};uint32_t id{},lease{},actor{},unit{};bool notified{};};
struct Registry {
    std::array<Tag,128> tags{};unsigned next{};
    void add(Tag t){tags[next++%tags.size()]=t;}
    bool find(uintptr_t address,uint32_t id,uint32_t lease,Tag& out)const{
        for(const auto& t:tags)if(t.projectile==address&&t.id==id&&t.lease==lease){out=t;return !t.notified;}return false;
    }
    bool consume(const Tag& t){for(auto& current:tags)if(current.projectile==t.projectile&&current.id==t.id&&
        current.environment==t.environment&&current.lease==t.lease&&current.actor==t.actor&&current.unit==t.unit){
            if(current.notified)return false;current.notified=true;return true;}return false;}
};
}
static friendly109::Registry friendly_registry109;
static SRWLOCK friendly_lock109=SRWLOCK_INIT;
static friendly109::Status* friendly_status109;static HANDLE friendly_mapping109;
static volatile LONG friendly_hits109,friendly_errors109;
using FriendlyImpact109=uintptr_t(__thiscall*)(void*,uintptr_t,uintptr_t,uint32_t,uint32_t);
static FriendlyImpact109 original_friendly_impact109;
using NativeLeaderRole=bool(__thiscall*)(void*);
static NativeLeaderRole original_leader_role;
static __declspec(thread) uintptr_t projectile_impact_victim;
static bool proximity_independent200(uintptr_t actor);
// Pure reads only: this observer never acquires/releases a lease, starts a death
// view, changes native combat, or calls the ownership predicates with side effects.
static bool friendly_owned109(PossessionIdentity& out){
    if(!state||!enabled||state->stop||state->release||!state->ready||state->mode!=POSSESSED||
       !possession_lease.active||GetTickCount64()-state->heartbeat>500)return false;
    auto id=possession_lease.identity;unsigned generation=possession_lease.generation;
    if(id.actor!=possessed||id.unit!=unit||id.environment!=environment||!id.holder||!id.config||
       state->lease_generation!=generation||state->actor_id!=id.actor_id||state->unit_id!=id.unit_id)return false;
    auto root=field<uintptr_t>(base,0x1bc8180);
    if(!root||field<uintptr_t>(root,0x110)!=id.environment||field<uintptr_t>(id.environment,8)!=id.common||
       field<uintptr_t>(id.common,0xb4)!=id.phase||field<unsigned>(id.phase,0x14)<5||field<unsigned>(id.phase,0x14)>8||
       field<uintptr_t>(id.environment,0x281fc)!=id.manager||field<uintptr_t>(id.manager,0x8e0)!=id.camera||
       field<uintptr_t>(id.camera,0)!=base+0x15d22e8||field<uintptr_t>(id.camera,0x180)!=id.manager||
       field<uintptr_t>(id.camera,0x188)!=id.actor||field<uintptr_t>(id.manager,0x8fc)!=id.actor||
       field<uintptr_t>(id.camera,0x184)!=id.view||!actor152::human(base,field<uintptr_t>(id.actor,0))||
       field<unsigned char>(id.actor,0xf0)!=1||field<unsigned>(id.actor,0x1c8)!=id.actor_id||
       field<uintptr_t>(id.actor,0x214)!=id.unit||field<unsigned>(id.unit,4)!=id.unit_id||
       field<uintptr_t>(id.actor,0x20c)!=id.config||field<uintptr_t>(id.actor,0x28)!=id.environment||
       field<uintptr_t>(id.holder,8)!=id.actor)return false;
    unsigned count=field<unsigned>(id.view,0x18);auto holders=field<uintptr_t>(id.view,0x1c);
    if(!count||count>1000||!holders)return false;bool found=false;
    for(unsigned i=0;i<count;++i)if(field<uintptr_t>(holders,i*4)==id.holder){found=true;break;}
    if(!found)return false;
    auto selected=field<uintptr_t>(id.environment,0x2826c);if(!selected)return false;
    auto owner=field<uintptr_t>(selected,0x4a8);if(!owner)return false;auto group=owner+0xb0;
    if(field<uintptr_t>(group,0)!=base+0x15c4da0||field<unsigned>(group,0x14)!=1||field<unsigned>(group,0x10)<1)return false;
    auto members=field<uintptr_t>(group,0x18);if(!members||field<uintptr_t>(members,0)!=id.unit)return false;
    if(!possession_lease.active||generation!=possession_lease.generation||!same_soldier(id,possession_lease.identity)||
       state->mode!=POSSESSED||state->release||state->stop)return false;
    out=id;return true;
}
// Native Register41c760 appends the newly constructed projectiles to pool+14,
// count+10. Each projectile has its native monotonically assigned id at+4 and
// shooter at+1c. Record ONLY this personal-shot call's newly appended objects.
static void friendly_tag109(uintptr_t pool,unsigned first,const PossessionIdentity& shot,unsigned generation,unsigned* arrow=nullptr){
    PossessionIdentity current;if(!friendly_owned109(current)||!same_soldier(current,shot)||generation!=possession_lease.generation)return;
    unsigned count=field<unsigned>(pool,0x10);auto entries=field<uintptr_t>(pool,0x14);
    if(count<first||count-first>128||count>262144||!entries)return;
    for(unsigned i=first;i<count;++i){auto projectile=field<uintptr_t>(entries,i*4);if(!projectile||
        field<uintptr_t>(projectile,0x1c)!=shot.actor)continue;
        friendly109::Tag tag{projectile,shot.environment,field<unsigned>(projectile,4),generation,shot.actor_id,shot.unit_id,false};
        AcquireSRWLockExclusive(&friendly_lock109);friendly_registry109.add(tag);ReleaseSRWLockExclusive(&friendly_lock109);
        if(arrow&&*arrow==arrowcam107::no_projectile)*arrow=tag.id;
    }
}
static void friendly_register109(uintptr_t pool,void* launch,unsigned* arrow=nullptr){
    unsigned first=0,generation=0;PossessionIdentity shot{};bool track=false;
    __try {track=friendly_owned109(shot);if(track){first=field<unsigned>(pool,0x10);generation=possession_lease.generation;}}
    __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&friendly_errors109);track=false;}
    // Always forward exactly once, outside the observation exception handler.
    using Register109=void(__thiscall*)(void*,void*);
    reinterpret_cast<Register109>(base+0x41c760)(reinterpret_cast<void*>(pool),launch);
    if(track){__try {friendly_tag109(pool,first,shot,generation,arrow);}
        __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&friendly_errors109);}}
}
static bool friendly_candidate109(uintptr_t victim,uintptr_t projectile,friendly109::Tag& tag,unsigned& victim_id,unsigned& victim_unit){
    if(!state||!enabled||state->stop||state->release||state->mode!=POSSESSED||!projectile||
       field<uintptr_t>(projectile,0x1c)!=possessed)return false;
    auto id=field<unsigned>(projectile,4);unsigned generation=possession_lease.generation;
    AcquireSRWLockShared(&friendly_lock109);bool found=friendly_registry109.find(projectile,id,generation,tag);ReleaseSRWLockShared(&friendly_lock109);
    if(!found)return false;PossessionIdentity current;
    if(!friendly_owned109(current)||tag.environment!=current.environment||tag.actor!=current.actor_id||tag.unit!=current.unit_id||
       victim==current.actor||!victim||field<unsigned char>(victim,0xf0)!=1||field<uintptr_t>(victim,0x28)!=current.environment)return false;
    auto table=field<uintptr_t>(victim,0);if(!actor152::human(base,table)&&table!=base+0x15e8524)return false;
    auto own_side=field<uintptr_t>(current.actor,0x44),other_side=field<uintptr_t>(victim,0x44);
    if(!own_side||!other_side)return false;
    // Retail combat's433b80 compares side+10 for battle-team membership;
    // separate allied armies can have different side pointers but the same team.
    unsigned team=field<unsigned>(own_side,0x10);if(team>31||field<unsigned>(other_side,0x10)!=team)return false;
    auto target_unit=field<uintptr_t>(victim,0x214);if(!target_unit)return false;
    victim_id=field<unsigned>(victim,0x1c8);victim_unit=field<unsigned>(target_unit,4);return victim_unit!=0;
}
static bool __fastcall leader_role_hook(void* actor,void*){
    const auto caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
    const bool leader=original_leader_role(actor);
    if(!leader||caller!=base+0x3a61db||!projectile_impact_victim||
       reinterpret_cast<uintptr_t>(actor)!=possessed)return leader;
    __try{
        PossessionIdentity current;
        if(!friendly_owned109(current)||field<uintptr_t>(current.actor,0)!=base+0x15dbd68)
            return leader;
        const auto physical=current.mount?current.mount:current.actor;
        // Only the lethal projectile check for the current detached general.
        // The engine still calculates hit chance, armor, damage and death.
        if((projectile_impact_victim==current.actor||projectile_impact_victim==physical)&&
           proximity_independent200(physical))return false;
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    return leader;
}
static uintptr_t __fastcall friendly_impact109(void* object,void*,uintptr_t victim,uintptr_t projectile,uint32_t third,uint32_t fourth){
    friendly109::Tag tag{};unsigned victim_id=0,victim_unit=0;bool notify=false;
    __try {notify=friendly_candidate109(victim,projectile,tag,victim_id,victim_unit);}
    __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&friendly_errors109);}
    const auto previous_victim=projectile_impact_victim;
    projectile_impact_victim=projectile?victim:0;
    uintptr_t result;
    __try {result=original_friendly_impact109(object,victim,projectile,third,fourth);}
    __finally {projectile_impact_victim=previous_victim;}
    if(notify&&friendly_status109){__try {PossessionIdentity current;
        if(friendly_owned109(current)&&tag.lease==possession_lease.generation&&tag.actor==current.actor_id&&tag.unit==current.unit_id){
            AcquireSRWLockExclusive(&friendly_lock109);
            if(friendly_registry109.consume(tag)){
                auto s=friendly_status109;InterlockedIncrement(&s->sequence);MemoryBarrier();
                s->lease=tag.lease;s->actor=tag.actor;s->unit=tag.unit;s->tick=GetTickCount64();
                s->victim=victim_id;s->victim_unit=victim_unit;s->projectile=tag.id;++s->event;
                MemoryBarrier();InterlockedIncrement(&s->sequence);InterlockedIncrement(&friendly_hits109);
            }
            ReleaseSRWLockExclusive(&friendly_lock109);
        }}__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&friendly_errors109);}}
    return result;
}
static void friendly_initialize109(){
    wchar_t name[96];friendly109::mapping_name(name,GetCurrentProcessId());
    friendly_mapping109=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(friendly109::Status),name);
    require(friendly_mapping109&&GetLastError()!=ERROR_ALREADY_EXISTS,"Friendly-fire109 mapping unavailable");
    friendly_status109=static_cast<friendly109::Status*>(MapViewOfFile(friendly_mapping109,FILE_MAP_ALL_ACCESS,0,0,sizeof(friendly109::Status)));
    require(friendly_status109!=nullptr,"Friendly-fire109 view unavailable");memset(friendly_status109,0,sizeof *friendly_status109);
    friendly_status109->magic=friendly109::magic;friendly_status109->version=friendly109::version;
    friendly_status109->size=sizeof *friendly_status109;friendly_status109->pid=GetCurrentProcessId();
}
static void create_friendly_hook109(){
    const unsigned char signature[]={0x83,0xec,0x20,0x53,0x56,0x57,0x8b,0x7c,0x24,0x34};
    auto target=reinterpret_cast<void*>(base+0x473f80);require(!memcmp(target,signature,sizeof signature),"Native projectile impact109 signature differs");
    require(MH_CreateHook(target,reinterpret_cast<void*>(&friendly_impact109),reinterpret_cast<void**>(&original_friendly_impact109))==MH_OK,"Friendly-fire109 observer hook failed");
    const unsigned char role_entry[]={0x8b,0x01,0xff,0x90,0xbc,0,0,0};
    const unsigned char lethal_call[]={0xe8,0x25,0x29,0x03,0,0x84,0xc0,0x0f,0x84};
    auto role=reinterpret_cast<void*>(base+0x3d8b00);
    require(!memcmp(role,role_entry,sizeof role_entry)&&
        !memcmp(reinterpret_cast<void*>(base+0x3a61d6),lethal_call,sizeof lethal_call),
        "Native projectile leader protection signature differs");
    require(MH_CreateHook(role,reinterpret_cast<void*>(&leader_role_hook),
        reinterpret_cast<void**>(&original_leader_role))==MH_OK,"Projectile leader protection hook failed");
}
