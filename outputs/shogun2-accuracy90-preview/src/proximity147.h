#pragma once
#include <cfloat>
// Native kind2 groups already exist for independently controlled members:
// 1504c2/1718f0/176954 call1668e0(2,unit,0,0) in unit+b64.
// Regular formation groups are kind3 in unit+b80. Preserve unit+214 and the
// master roster. A separated player keeps a REAL registered targetable group.
struct Proximity147 {
    uintptr_t environment{},phase{},unit{},actor{},config{},formation{},independent{};
    unsigned unit_id{},actor_id{},generation{};uint64_t checked{};bool joined{};
};
static Proximity147 proximity_records147[128];
static volatile LONG proximity_count147,proximity_generation147,proximity_joined_generation147;
static volatile LONG proximity_joins147,proximity_leaves147,proximity_created147,proximity_retired147,proximity_errors147;
using TargetSquad147=uintptr_t(__thiscall*)(void*);
static TargetSquad147 original_target_squad147;
static bool proximity_eligible147(uintptr_t actor){
    return actor==locomotion_actor()&&member_owned130(actor)&&field<uintptr_t>(actor,0x28)==environment&&
        !dismount_native_motion143()&&((!wall_subject97(possessed)&&!native_airborne_subject99(possessed))||climb_independent200(actor))&&
        !native_recovery103(actor)&&!native_routing(unit)&&(!field<uintptr_t>(actor,0x18)||climb_independent200(actor))&&!field<uintptr_t>(actor,0x688);
}
static bool proximity_capable147(uintptr_t native_unit,uintptr_t env){
    // Current native registries/ID object are prerequisites for original factory
    // execution. No synthetic controller and no saved pointer grants authority.
    return field<uintptr_t>(native_unit,0x14)==env&&field<uintptr_t>(native_unit,0xb7c)==env&&
        field<uintptr_t>(native_unit,0xb98)==env&&field<uintptr_t>(native_unit,0x10)&&
        field<uintptr_t>(native_unit,0x440)&&field<unsigned>(native_unit,0xb70)<=1000&&field<unsigned>(native_unit,0xb8c)<=1000;
}
static Proximity147* proximity_find147(uintptr_t actor){for(auto& p:proximity_records147)if(p.actor==actor)return &p;return nullptr;}
static bool proximity_managed147(uintptr_t actor){
    if(actor!=locomotion_actor()||!member_owned130(actor)||proximity_generation147!=LONG(possession_lease.generation))return false;
    return true; // Numeric gate is not mutation authority; native calls re-prove.
}
static uintptr_t proximity_squad147(uintptr_t actor){
    if(!proximity_managed147(actor))return 0;bool locked=!member_sync_active130;
    if(locked)AcquireSRWLockShared(&member_lock130);uintptr_t result=0;
    __try{auto p=proximity_find147(actor);if(p&&p->generation==possession_lease.generation&&p->unit==unit){auto target=p->joined?p->formation:p->independent;if(squad_registered130(target,unit,environment))result=target;}}
    __finally{if(locked)ReleaseSRWLockShared(&member_lock130);}return result;
}
static bool proximity_deny147(uintptr_t squad,uintptr_t actor){
    // Native bulk regrouping1baec0 drains a group's member count in a while
    // loop. Refusing one transfer there can prevent the native loop ending.
    // Native lifecycle always wins; re-evaluate manual proximity at the next
    // existing unit boundary, after original regrouping has completed.
    (void)squad;(void)actor;return false;
}
static void proximity_forget147(Proximity147& p){
    if(proximity_generation147==LONG(p.generation)){InterlockedExchange(&proximity_generation147,0);InterlockedExchange(&proximity_joined_generation147,0);}
    p={};InterlockedDecrement(&proximity_count147);
}
static bool proximity_retire147(Proximity147& p){
    if(!p.independent)return true;
    if(!squad_registered130(p.independent,p.unit,p.environment)){p.independent=0;return true;}
    if(field<uintptr_t>(p.unit,0xb9c)==p.independent)return false;
    if(field<unsigned>(p.independent,0x18))return false;
    using Retire=bool(__thiscall*)(void*,uintptr_t);
    if(!reinterpret_cast<Retire>(base+0x169130)(reinterpret_cast<void*>(p.unit+0xb64),p.independent))return false;
    p.independent=0;InterlockedIncrement(&proximity_retired147);return true;
}
static bool proximity_transfer147(Proximity147& p,uintptr_t target,bool returning=false){
    if(!squad_registered130(target,p.unit,p.environment))return false;
    auto source=field<uintptr_t>(p.actor,0x3b4);
    const bool officer=field<uintptr_t>(p.actor,0)==base+0x15dbfb4;
    if(source==target){
        if(squad_matches130(target,p.actor)!=1)return false;
        if(!returning)return true;
        // Leave ordinary soldiers' unrelated tasks to finish on their own.
        // An officer can keep a personal task after possession has ended.
        if(!officer&&field<unsigned char>(p.actor,0x384)==1&&
           field<uintptr_t>(p.actor,0x2f8)!=base+0x15dd7ac)return false;
    }else if(source&&!squad_registered130(source,p.unit,p.environment))return false;
    // The native regroup path clears the old task and requests a new slot.
    // The release caller already excludes combat, recovery and special motion.
    const bool reset_formation=returning&&(source==target||officer);
    original_member_transfer130(reinterpret_cast<void*>(target),p.actor,reset_formation?1u:0u);
    return field<uintptr_t>(p.actor,0x3b4)==target&&squad_matches130(target,p.actor)==1;
}
static bool proximity_near147(const Proximity147& p,uintptr_t formation,bool& within){
    if(!squad_registered130(formation,p.unit,p.environment)||formation==p.independent||!field<unsigned>(formation,0x18))return false;
    // Exclude the player from the distance reference. Including the player's
    // own position in the native bounding box makes that box follow them away.
    // One current OWN formation only, at most1000 entries at10Hz; no enemy or
    // global army scan. The native callback supplies the current member array.
    unsigned count=field<unsigned>(formation,0x18);auto members=field<uintptr_t>(formation,0x1c);
    if(count>1000||!members)return false;float nearest=FLT_MAX;
    float x=field<float>(p.actor,0x48),y=field<float>(p.actor,0x4c),z=field<float>(p.actor,0x50);
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z))return false;
    for(unsigned i=0;i<count;++i){__try{auto peer=field<uintptr_t>(members,i*4);if(!peer||peer==p.actor)continue;
        if(field<uintptr_t>(peer,0x214)!=p.unit||field<uintptr_t>(peer,0x28)!=p.environment||field<uintptr_t>(peer,0x3b4)!=formation||field<unsigned char>(peer,0xf0)!=1)continue;
        float dx=field<float>(peer,0x48)-x,dy=field<float>(peer,0x4c)-y,dz=field<float>(peer,0x50)-z;
        float distance=dx*dx+dy*dy+dz*dz;if(std::isfinite(distance))nearest=std::min(nearest,distance);
    }__except(EXCEPTION_EXECUTE_HANDLER){continue;}}
    if(nearest==FLT_MAX)return false;
    float margin=p.joined?8.f:4.f;within=nearest<=margin*margin;return true;
}
static bool proximity_identity147(const Proximity147& p,uintptr_t current){
    return current==p.unit&&field<unsigned>(current,4)==p.unit_id&&field<uintptr_t>(current,0x14)==p.environment&&
        unit_member130(current,p.actor)&&field<unsigned>(p.actor,0x1c8)==p.actor_id&&field<uintptr_t>(p.actor,0x20c)==p.config&&
        field<uintptr_t>(p.actor,0x214)==current&&field<unsigned char>(p.actor,0xf0)==1;
}
struct ReformPoint70 {float position[3];uintptr_t surface;unsigned surface_id;};
struct ReformDestination70 {
    uintptr_t unit_reference;
    ReformPoint70 point;
    unsigned short facing, reserved;
    float width;
};
struct ReformList70 {unsigned capacity,count;const ReformDestination70* entries;};
static_assert(sizeof(ReformPoint70)==20&&sizeof(ReformDestination70)==32&&sizeof(ReformList70)==12);

static bool reform_near_release70(const Proximity147& p,uintptr_t formation){
    // 42dfd0 reads the stock unit pose at +440. Reuse that exact point,
    // heading and width in the single-unit apply path of 45e930. The caller's
    // acknowledgment event, UI marker and move sound are outside 4668f0.
    if(!enabled||!state||state->stop||state->release||state->mode!=IDLE||
       possession_lease.active||state->lease_generation!=p.generation||
       field<uintptr_t>(p.unit,0xb9c)!=formation||field<unsigned>(p.unit,0x9ec))return false;
    const auto pose=field<uintptr_t>(p.unit,0x440);
    if(!pose||!field<uintptr_t>(p.unit,0xc))return false;
    ReformDestination70 destination{};
    destination.unit_reference=p.unit+0xc;
    memcpy(&destination.point,reinterpret_cast<const void*>(pose+0x10),sizeof destination.point);
    destination.width=field<float>(pose,0x24);
    destination.facing=field<unsigned short>(pose,0x28);
    if(!std::isfinite(destination.point.position[0])||!std::isfinite(destination.point.position[1])||
       !std::isfinite(destination.point.position[2])||!std::isfinite(destination.width)||destination.width<=0)return false;
    const unsigned char apply[]={0x83,0xec,0x64,0x53,0x8b,0x5c,0x24,0x78};
    const unsigned char move[]={0x81,0xec,0x94,0,0,0,0x56,0x8b,0xf1};
    if(memcmp(reinterpret_cast<const void*>(base+0x4668f0),apply,sizeof apply)||
       memcmp(reinterpret_cast<const void*>(base+0x187000),move,sizeof move))return false;
    // An empty native queue means append begins immediately. The native append
    // branch leaves any other command state intact and queues one ordinary walk.
    const ReformList70 groups{},units{1,1,&destination};
    using Apply=void(__stdcall*)(uintptr_t,unsigned,unsigned,const ReformList70*,const ReformList70*,unsigned);
    reinterpret_cast<Apply>(base+0x4668f0)(p.environment,0,1,&groups,&units,1);
    return field<unsigned>(p.unit,0x9ec)==1;
}

static void proximity_sync147(uintptr_t current,uintptr_t env,uintptr_t phase,unsigned phase_id){
    // Invoked only inside the native unit callback's existing member lock.
    for(auto& p:proximity_records147)if(p.actor){
        if(p.environment!=env||p.phase!=phase||phase_id<5||phase_id>8){proximity_forget147(p);continue;}
        if(p.unit!=current)continue;
        if(!proximity_identity147(p,current)){if(proximity_retire147(p))proximity_forget147(p);continue;}
        if(!member_owned130(p.actor)||p.actor!=locomotion_actor()||p.generation!=possession_lease.generation){
            if((dismount_native_motion143()&&current==unit)||field<uintptr_t>(p.actor,0x18)||field<uintptr_t>(p.actor,0x688)||native_recovery103(p.actor)||wall_subject97(p.actor)||native_airborne_subject99(p.actor)||native_routing(current))continue;
            auto target=squad_registered130(p.formation,current,env)?p.formation:field<uintptr_t>(current,0xb9c);
            if(target==p.independent){
                // Last surviving soldier: create the ordinary native formation
                // through its own factory, then let native main-group selection
                // choose it. Never leave b9c pointing into a retired pool slot.
                using Policy=unsigned(__thiscall*)(void*);using Factory=uintptr_t(__thiscall*)(void*,unsigned);
                auto policy=reinterpret_cast<Policy>(base+0x1672b0)(reinterpret_cast<void*>(current));
                target=reinterpret_cast<Factory>(base+0x166d60)(reinterpret_cast<void*>(current),policy);
                if(!squad_registered130(target,current,env)){InterlockedIncrement(&proximity_errors147);continue;}
            }
            bool within=false;
            const bool release_in_range=p.joined&&target==p.formation&&field<uintptr_t>(p.actor,0x3b4)==target&&
                proximity_near147(p,target,within)&&within;
            // The unit thread can see a pending release before the input
            // thread drops the lease. Keep the near record until that handoff.
            if(release_in_range&&state->release&&state->mode==POSSESSED&&possession_lease.active&&
               p.generation==possession_lease.generation&&p.actor==locomotion_actor())continue;
            if(target!=p.independent&&proximity_transfer147(p,target,true)){
                if(!release_in_range||!reform_near_release70(p,target))reissue_officer_slot68(p.actor,target,current,env);
                if(field<uintptr_t>(current,0xb9c)==p.independent)reinterpret_cast<UnitTick130>(base+0x16b180)(reinterpret_cast<void*>(current));
                if(proximity_retire147(p))proximity_forget147(p);
            }
        }
    }
    auto actor=locomotion_actor();if(current!=unit||!proximity_eligible147(actor)||!proximity_capable147(current,env))return;
    auto p=proximity_find147(actor);
    if(!p){
        auto formation=field<uintptr_t>(current,0xb9c);if(!squad_registered130(formation,current,env))return;
        for(auto& slot:proximity_records147)if(!slot.actor){p=&slot;break;}
        if(!p){InterlockedIncrement(&proximity_errors147);return;}
        *p={env,phase,current,actor,field<uintptr_t>(actor,0x20c),formation,0,field<unsigned>(current,4),field<unsigned>(actor,0x1c8),possession_lease.generation,0,true};InterlockedIncrement(&proximity_count147);
    }
    if(!proximity_identity147(*p,current))return;
    if(p->independent&&!squad_registered130(p->independent,current,env))p->independent=0;
    if(p->independent&&field<uintptr_t>(actor,0x3b4)!=p->independent){
        // Do not lose the only reference to an empty registered group when
        // native regrouping has moved its actor. Retire it before replacement.
        if(!proximity_retire147(*p))p->independent=0; // Native adopted a nonempty/main group.
    }
    InterlockedExchange(&proximity_generation147,LONG(p->generation));
    InterlockedExchange(&proximity_joined_generation147,p->joined?LONG(p->generation):0);
    if(charge_input_locked146()){
        auto target=charge_squad147(actor);if(target&&!proximity_transfer147(*p,target))InterlockedIncrement(&proximity_errors147);return;
    }
    if(charge_native_task146(actor)||native_mounted_jump140(actor))return;
    const bool personal_wall=climb_independent200(actor);
    auto now=GetTickCount64();if(!personal_wall&&now-p->checked<100)return;p->checked=now;
    auto main=field<uintptr_t>(current,0xb9c);if(main==p->independent)main=p->formation;
    bool within=false;
    // A personal climb never rejoins a nearby formation, even at the foot of
    // the wall. Keep the native owner/roster and a targetable independent group.
    if(!personal_wall&&!proximity_near147(*p,main,within))return;
    if(squad_registered130(main,current,env))p->formation=main;
    if(!within&&!p->independent){
        using Factory=uintptr_t(__thiscall*)(void*,unsigned,uintptr_t,unsigned,unsigned);
        p->independent=reinterpret_cast<Factory>(base+0x1668e0)(reinterpret_cast<void*>(current+0xb64),2,current,0,0);
        if(!squad_registered130(p->independent,current,env)){p->independent=0;InterlockedIncrement(&proximity_errors147);return;}
        InterlockedIncrement(&proximity_created147);
    }
    auto target=within?p->formation:p->independent;
    if(!proximity_transfer147(*p,target)){InterlockedIncrement(&proximity_errors147);return;}
    if(p->joined!=within)InterlockedIncrement(within?&proximity_joins147:&proximity_leaves147);p->joined=within;
    if(within&&p->independent){
        // Retire immediately after a proved transfer, rather than retaining an
        // empty pooled object across native regrouping/lifetime reuse.
        if(!proximity_retire147(*p))p->independent=0; // Native controller adopted it; engine owns it.
    }
    InterlockedExchange(&proximity_joined_generation147,within?LONG(p->generation):0);
}
static uintptr_t independent_group_locked65(uintptr_t actor){
    auto p=proximity_find147(actor);
    if(!p||p->joined||!p->independent||p->generation!=possession_lease.generation||
       !proximity_identity147(*p,unit)||!squad_registered130(p->independent,unit,environment)||
       field<uintptr_t>(actor,0x3b4)!=p->independent||squad_matches130(p->independent,actor)!=1)return 0;
    return p->independent;
}
static bool proximity_independent200(uintptr_t actor){
    bool result=false;const bool locked=!member_sync_active130;
    if(locked)AcquireSRWLockShared(&member_lock130);
    __try{result=independent_group_locked65(actor)!=0;}
    __finally{if(locked)ReleaseSRWLockShared(&member_lock130);}return result;
}
static void proximity_log147(std::ostream& log){
    static LONG joins=-1,leaves=-1,created=-1,retired=-1,errors=-1;
    if(joins==proximity_joins147&&leaves==proximity_leaves147&&created==proximity_created147&&retired==proximity_retired147&&errors==proximity_errors147)return;
    joins=proximity_joins147;leaves=proximity_leaves147;created=proximity_created147;retired=proximity_retired147;errors=proximity_errors147;
    log<<"{\"event\":\"formation_proximity147\",\"tick_ms\":"<<GetTickCount64()<<",\"joins\":"<<joins<<",\"leaves\":"<<leaves<<",\"groups_created\":"<<created<<",\"groups_retired\":"<<retired<<",\"pending\":"<<proximity_count147<<",\"errors\":"<<errors<<"}\n";
}
#include "../../../deck/acquisition203.h"
static uintptr_t __fastcall target_squad147(void* context,void*){
    __try{
        auto current=reinterpret_cast<uintptr_t>(context),target=field<uintptr_t>(current,0x34);
        // Resolve an EXISTING native actor target. Never choose a new enemy,
        // waive range/visibility, or redirect a formation-wide target.
        if(target&&(target==possessed||target==locomotion_actor())&&field<uintptr_t>(current,0x24)==unit){
            auto physical=locomotion_actor();if(member_owned130(physical)){
                auto squad=proximity_squad147(physical);if(squad&&field<uintptr_t>(physical,0x3b4)==squad)return squad;
            }
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&proximity_errors147);}
    auto native_result203=original_target_squad147(context);
    __try{return acquisition_group203(reinterpret_cast<uintptr_t>(context),native_result203);}__except(EXCEPTION_EXECUTE_HANDLER){return native_result203;}
}
static void create_proximity147(){
    create_acquisition203();
    const unsigned char target[]={0x8b,0x41,0x34,0x85,0xc0,0x74,0x0a,0x8b,0x80,0xb4,3,0,0};
    const unsigned char factory[]={0x83,0xec,0x10,0x53,0x56,0x57,0xff,0x74,0x24,0x2c};
    require(!memcmp(reinterpret_cast<void*>(base+0x1679c0),target,sizeof target)&&!memcmp(reinterpret_cast<void*>(base+0x1668e0),factory,sizeof factory),"Native independent group/target signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1679c0),reinterpret_cast<void*>(&target_squad147),reinterpret_cast<void**>(&original_target_squad147))==MH_OK,"Native independent target group hook failed");
}
