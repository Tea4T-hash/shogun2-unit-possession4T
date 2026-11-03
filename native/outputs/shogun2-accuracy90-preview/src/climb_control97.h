#pragma once
#include "climb_protocol97.h"
static volatile LONG native_air_updates99,native_air_active99;
static SRWLOCK climb_publish_lock97=SRWLOCK_INIT;
static climb97::Status* climb_shared97;static HANDLE climb_mapping97;
static volatile LONG climb_attaches97,climb_steps97,climb_holds97,climb_down97,climb_endpoints97,climb_rejected97;
static unsigned climb_lease97;
static uint64_t climb_feedback_until199;
static uintptr_t climb_budget_pipe98;static unsigned climb_budget_id98,climb_budget_tick98,climb_budget_lease98;
static bool climb_budget_valid98;static float climb_rate98,climb_step98;static volatile LONG climb_duplicate98,climb_rate_errors98;static int climb_last_direction97=1;
static bool climb_align152;
struct ClimbEntry153 {uintptr_t actor{},pipe{},phase{};unsigned generation{},serial{},pipe_id{};unsigned goal[5]{};bool started{};unsigned tick{};float elapsed{},stalled{},position[3]{};unsigned short heading{};};
static ClimbEntry153 climb_entry153;
static unsigned climb_traversal200;
using ClimbPathRequest200=uintptr_t(__thiscall*)(void*,void*,const void*,unsigned,unsigned,unsigned);
static ClimbPathRequest200 original_climb_path_request200;
static volatile LONG climb_requests153,climb_arrivals153,climb_cancels153;
static volatile LONG climb_error154,climb_pipes154,climb_valid154,climb_route_state154;
static bool climb_fail154(unsigned code){InterlockedExchange(&climb_error154,LONG(code));return false;}
static uint64_t climb_scan97;static uintptr_t climb_near97;static unsigned climb_near_id97;static bool climb_near_top97;
using PipeAdvance97=void(__thiscall*)(void*,uintptr_t,float);
using PipeMode97=unsigned(__thiscall*)(void*,uintptr_t);
using PipeBind97=void(__thiscall*)(void*,uintptr_t);
using PipePoint97=uintptr_t(__thiscall*)(void*,float*,float);
using NativeReset97=void(__thiscall*)(void*,const float*,unsigned,uintptr_t,unsigned);
static PipeAdvance97 original_pipe_advance97;static PipeMode97 original_pipe_mode97;
static void climb_publish97(unsigned mode,float progress=0,int direction=0){
    InterlockedExchange(&keyboard_wall98,(mode==climb97::Climbing||mode==climb97::Crest||mode==climb97::Approaching)?1:0);
    if(!climb_shared97)return;
    AcquireSRWLockExclusive(&climb_publish_lock97);
    InterlockedIncrement(&climb_shared97->sequence);MemoryBarrier();
    climb_shared97->lease=possession_lease.generation;climb_shared97->actor=state?state->actor_id:0;climb_shared97->unit=state?state->unit_id:0;
    climb_shared97->mode=mode;climb_shared97->progress=progress;climb_shared97->direction=direction;climb_shared97->tick=GetTickCount64();
    MemoryBarrier();InterlockedIncrement(&climb_shared97->sequence);ReleaseSRWLockExclusive(&climb_publish_lock97);
}
static void climb_reset97(bool preserve_request=false){climb_feedback_until199=0;climb_traversal200=0;climb_align152=false;climb_entry153={};InterlockedExchange(&native_air_active99,0);climb_budget_valid98=false;climb_budget_pipe98=0;climb_rate98=climb_step98=0;climb_near97=0;climb_scan97=0;climb_lease97=0;climb_last_direction97=1;if(!preserve_request)climb_request199.clear();climb_publish97(climb97::Off);}
static bool climb_owned97(uintptr_t actor,uintptr_t pipe){
    if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||actor!=possessed||!possession_lease.active||weapon_mounted()||
        GetTickCount64()-state->heartbeat>500||field<uintptr_t>(actor,0x18)!=pipe)return false;
    if(!followed(actor,false,false)||dead_view())return false;
    melee60::Request qte;if(melee_request(qte))return false;
    Wall97 w;return read_wall97(actor,pipe,w);
}
// Exact native portal navigation location. Shared by entry and endpoint exit;
// navigation surface(+c) and coordinate frame(+10) are distinct native objects.
static bool climb_goal155(uintptr_t portal,unsigned (&goal)[5]){
 if(!portal)return false;
    auto table=field<uintptr_t>(portal,0),getter=table?field<uintptr_t>(table,0x10):0;
    // Portal virtual10 is the native navigation location, not a bearing pose.
    // 397e40/397e80 ->1656b0 ->359070 preserve the navigation surface at+c
    // and coordinate frame at+10. 351e40 puts a BEARING at+c; passing that
    // different20-byte type to3df290 crashed native3b2470 on bearing0x2a9.
    if(table!=base+0x15de28c&&table!=base+0x15de2ac)return false;
    if(getter!=base+(table==base+0x15de28c?0x397e40:0x397e80))return false;
    auto result=reinterpret_cast<uintptr_t(__thiscall*)(void*,void*)>(getter)(reinterpret_cast<void*>(portal),goal);
    if(result!=reinterpret_cast<uintptr_t>(goal))return false;
    for(unsigned k=0;k<3;++k){float v;memcpy(&v,goal+k,4);if(!std::isfinite(v)||std::abs(v)>100000)return false;}
    return true;
}
static bool climb_exit97(uintptr_t actor,const Wall97& w,bool top){
    auto portal=field<uintptr_t>(w.pipe,top?0x2c:0x28);if(!portal)return false;
    unsigned goal155[5]{};if(!climb_goal155(portal,goal155))return false;
    auto surface=uintptr_t(goal155[3]); // Native3983c0 expects navigation surface, not its frame.

    auto node=w.nodes+(top?w.count-1:0)*72;float position[2]={field<float>(node,top?0x30:0x24),field<float>(node,top?0x38:0x2c)};
    // Only a true path endpoint, within one native step. Native spatial remove,
    // ground/surface resolution and re-registration stay in original3dfe30.
    float distance2=0;for(unsigned k=0;k<3;++k){float d=field<float>(actor,0x48+k*4)-field<float>(node,(top?0x30:0x24)+k*4);distance2+=d*d;}
    if(!std::isfinite(distance2)||distance2>.25f)return false;
    reinterpret_cast<NativeReset97>(base+0x3dfe30)(reinterpret_cast<void*>(actor),position,field<unsigned short>(actor,0x64),surface,0);
    if(field<uintptr_t>(actor,0x18))return false;
    original_change(reinterpret_cast<void*>(actor),8,0,0);InterlockedIncrement(&climb_endpoints97);return true;
}
static bool native_climb_rate98(uintptr_t actor,const Wall97& wall,int direction,float& rate){
    auto native_unit=field<uintptr_t>(actor,0x214);bool fast=field<unsigned char>(native_unit,0x294)==1;
    // Native3dda00 scales ascending68/70 only. Descending69/71 uses
    // the authored segment rate without an ascending parameter multiplier.
    float native=direction<0?1.f:reinterpret_cast<float(__thiscall*)(void*)>(base+0xa0b60)(reinterpret_cast<void*>(base+(fast?0x1a4d2e0:0x1a4d3a0)));
    float segment=reinterpret_cast<float(__thiscall*)(void*,float)>(base+0x3a2370)(reinterpret_cast<void*>(wall.definition),field<float>(actor,0x20));
    rate=native*segment;return std::isfinite(rate)&&rate>=0&&rate<=20;
}
static void __fastcall pipe_advance_hook97(void* motion,void*,uintptr_t pipe,float distance){
    auto actor=reinterpret_cast<uintptr_t>(motion)-0x14;bool handled=false;
    __try{
        if(climb_owned97(actor,pipe)){
            if(crest_subject97(actor)){climb_publish97(climb97::Crest,field<float>(actor,0x20));}
            else {Wall97 wall;if(read_wall97(actor,pipe,wall)){
                int direction=climb97::direction(unsigned(state->input));auto common=possession_lease.identity.common;
                unsigned tick=field<unsigned>(common,0x50);float dt=field<float>(base,0x15b2448);
                if(climb_budget_valid98&&climb_budget_pipe98==pipe&&climb_budget_id98==wall.id&&climb_budget_lease98==possession_lease.generation&&climb_budget_tick98==tick){InterlockedIncrement(&climb_duplicate98);handled=true;}
                else {
                    climb_budget_valid98=true;climb_budget_pipe98=pipe;climb_budget_id98=wall.id;climb_budget_lease98=possession_lease.generation;climb_budget_tick98=tick;
                    if(climb_lease97!=possession_lease.generation){climb_lease97=possession_lease.generation;climb_last_direction97=direction<0?-1:1;}
                    if(direction)climb_last_direction97=direction;
                    climb_rate98=0;if(!native_climb_rate98(actor,wall,direction,climb_rate98)){direction=0;InterlockedIncrement(&climb_rate_errors98);}
                    if(!std::isfinite(dt)||dt<=0||dt>.2f)direction=0;
                    climb_step98=direction?climb_rate98*dt:0;
                    // Never feed a prior path correction/ground velocity back
                    // into native max(distance,pending). One native tick budget.
                    for(unsigned off:{0x70u,0x74u,0x78u,0x7cu,0xd8u,0xdcu,0xe0u})field<float>(actor,off)=0;
                    original_pipe_advance97(motion,pipe,climb_step98);handled=true;
                    if(direction<0){float previous=field<float>(actor,0x24),length=reinterpret_cast<float(__thiscall*)(void*)>(base+0x3931f0)(reinterpret_cast<void*>(wall.definition));
                        if(std::isfinite(previous)&&previous>=0&&previous<=1&&std::isfinite(length)&&length>.001f){
                            float target=std::clamp(previous-climb_step98/length,0.f,1.f),point[4]{};
                            reinterpret_cast<PipePoint97>(base+0x39a2e0)(reinterpret_cast<void*>(wall.definition),point,target);
                            bool finite=true;for(unsigned k=0;k<3;++k)if(!std::isfinite(point[k])||std::abs(point[k]-field<float>(actor,0x48+k*4))>2)finite=false;
                            if(finite){field<float>(actor,0x20)=target;for(unsigned k=0;k<3;++k)field<float>(actor,0xd8+k*4)=point[k]-field<float>(actor,0x48+k*4);InterlockedIncrement(&climb_down97);}
                        }
                    }
                    if(!direction)InterlockedIncrement(&climb_holds97);InterlockedIncrement(&climb_steps97);
                    float progress=field<float>(actor,0x20);climb_publish97(climb97::Climbing,progress,direction);
                    if(direction<0&&progress<=.000001f)climb_exit97(actor,wall,false);
                    else if(direction>0&&progress>=.999999f)climb_exit97(actor,wall,true);
                }
            }}
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&climb_rejected97);}
    if(!handled)original_pipe_advance97(motion,pipe,distance);
}
static bool climb_native_air99(uintptr_t actor){
    bool active=native_airborne_subject99(actor);InterlockedExchange(&native_air_active99,active?1:0);
    if(!active)return false;
    climb_request199.clear();InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);
    climb_publish97(climb97::Off);InterlockedExchange(&keyboard_wall98,1);return true;
}
static void climb_adopt98(uintptr_t actor){climb_budget_valid98=false;climb_near97=0;climb_lease97=possession_lease.generation;
    if(climb_native_air99(actor))return;
    if(wall_subject97(actor))climb_publish97(crest_subject97(actor)?climb97::Crest:climb97::Climbing,field<float>(actor,0x20));else climb_publish97(climb97::Off);
}
static void climb_observe98(uintptr_t actor){if(climb_native_air99(actor))return;if(wall_subject97(actor))climb_publish97(crest_subject97(actor)?climb97::Crest:climb97::Climbing,field<float>(actor,0x20),climb97::direction(unsigned(state->input)));}
static unsigned __fastcall pipe_mode_hook97(void* object,void*,uintptr_t pipe){
    unsigned mode=original_pipe_mode97(object,pipe);
    __try{auto actor=reinterpret_cast<uintptr_t>(object);
        if(climb_owned97(actor,pipe)&&field<unsigned>(actor,0x1f0)<72){
            int direction=climb97::direction(unsigned(state->input));if(!direction&&climb_lease97==possession_lease.generation)direction=climb_last_direction97;
            if(direction<0&&(mode>=68&&mode<=75))return mode==70||mode==71||(mode>=72&&field<unsigned char>(field<uintptr_t>(actor,0x214),0x294)==1)?71:69;
            if(direction>0&&(mode==69||mode==71))return mode==71?70:68;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&climb_rejected97);}
    return mode;
}
// F uses the opposite portal native navigation location (not a bearing pose). Native
// 3df290/3df3f0/394e80 own approach, portal facing, entry and pipe attachment.
// Do not derive body heading from a long climb segment or bind3c1ba0 early.
static bool climb_entry_live153(uintptr_t actor,const Wall97& wall){
    const auto& e=climb_entry153;
    return climb_align152&&e.actor==actor&&e.generation==possession_lease.generation&&
        e.serial==field<unsigned>(actor,0x1c8)&&e.pipe==wall.pipe&&e.pipe_id==wall.id&&e.phase==wall.phase&&
        (!e.started||!memcmp(e.goal,reinterpret_cast<void*>(actor+0x420),sizeof e.goal));
}
static bool climb_begin153(uintptr_t actor,const Wall97& wall,bool top){
    // Only offer entry from the foot of a wall. Do not start a descent from its crest.
    if(top)return climb_fail154(11);
    auto entry_node153=wall.nodes+(top?wall.count-1:0)*72;unsigned off=top?0x30:0x24;
    float dx=field<float>(entry_node153,off)-field<float>(actor,0x48),dy=field<float>(entry_node153,off+4)-field<float>(actor,0x4c),dz=field<float>(entry_node153,off+8)-field<float>(actor,0x50);
    if(!std::isfinite(dx)||!std::isfinite(dy)||!std::isfinite(dz)||dx*dx+dz*dz>2.3f*2.3f||std::abs(dy)>1.25f)return climb_fail154(1);
    auto portal=field<uintptr_t>(wall.pipe,top?0x28:0x2c);if(!portal)return climb_fail154(2);
    unsigned goal[5]{};
    if(!climb_goal155(portal,goal))return climb_fail154(3);
    // Stage the request before asking the existing unit boundary for a native
    // independent group. Do not mutate squad membership from the integrator.
    climb_entry153={actor,wall.pipe,wall.phase,possession_lease.generation,field<unsigned>(actor,0x1c8),wall.id,{}};
    memcpy(climb_entry153.goal,goal,sizeof goal);climb_align152=true;climb_last_direction97=top?-1:1;
    climb_entry153.tick=field<unsigned>(possession_lease.identity.common,0x50);
    memcpy(climb_entry153.position,reinterpret_cast<void*>(actor+0x48),12);
    climb_entry153.heading=field<unsigned short>(actor,0x64);
    InterlockedExchange(&climb_error154,0);return true;
}
static bool climb_independent200(uintptr_t actor){
    if(!actor||actor!=possessed||!member_owned130(actor)||weapon_mounted())return false;
    return (climb_align152&&climb_entry153.actor==actor&&climb_entry153.generation==possession_lease.generation&&
        climb_entry153.serial==field<unsigned>(actor,0x1c8))||wall_subject97(actor)||
        (climb_traversal200==possession_lease.generation&&native_airborne_subject99(actor));
}
// Original3ccd00 creates the per-actor request. +34 permits portal traversal
// (unit14f750); +35 admits castle walls (unit14f720). Both depend on unit state.
// +35 admits native wall
// kinds11..14; +40 is the unit's selected wall list (unit+fa8), consumed by
// 384120/1be3c0. Only this explicitly requested personal route gets independent
// wall permission. Geometry, edge availability, faction, width and native solve
// remain original371b60/3b0b60; no unit bytes or global planner policy are changed.
static uintptr_t __fastcall climb_path_request200(void* object,void*,void* out,const void* goal,unsigned kind,unsigned flags,unsigned policy){
    auto result=original_climb_path_request200(object,out,goal,kind,flags,policy);
    auto actor=reinterpret_cast<uintptr_t>(object);
    __try{Wall97 wall;if(result==reinterpret_cast<uintptr_t>(out)&&climb_entry153.started&&
        climb_independent200(actor)&&proximity_independent200(actor)&&!field<uintptr_t>(actor,0x18)&&
        read_wall97(actor,climb_entry153.pipe,wall)&&climb_entry_live153(actor,wall)&&
        !memcmp(goal,climb_entry153.goal,20)&&!memcmp(static_cast<unsigned char*>(out)+0x18,goal,20)){
        *reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(out)+0x34)=1;
        *reinterpret_cast<unsigned char*>(reinterpret_cast<uintptr_t>(out)+0x35)=1;
        field<uintptr_t>(reinterpret_cast<uintptr_t>(out),0x40)=0;
    }}__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&climb_rejected97);}
    return result;
}
static void climb_cancel200(uintptr_t actor){
    // Retire only our still-owned approach, never an attached traversal or a
    // goal installed by another native action. Original3df380 releases its route.
    const auto e=climb_entry153;const bool pending=climb_align152;
    if(climb_align152&&e.started&&actor==e.actor&&e.generation==possession_lease.generation&&
       member_owned130(actor)&&e.serial==field<unsigned>(actor,0x1c8)&&!field<uintptr_t>(actor,0x18)&&
       !memcmp(e.goal,reinterpret_cast<void*>(actor+0x420),sizeof e.goal))
        reinterpret_cast<void(__thiscall*)(void*,unsigned)>(base+0x3df380)(reinterpret_cast<void*>(actor),1);
    climb_align152=false;climb_entry153={};climb_near97=0;if(pending)InterlockedIncrement(&climb_cancels153);
    climb_publish97(climb97::Off);
}
static bool climb_approach153(uintptr_t actor,const Wall97& wall){
    auto& e=climb_entry153;
    if(!climb_entry_live153(actor,wall)){climb_cancel200(actor);return false;}
    auto tick=field<unsigned>(possession_lease.identity.common,0x50);
    if(tick!=e.tick){
        e.tick=tick;float dt=field<float>(base,0x15b2448);
        if(std::isfinite(dt)&&dt>0&&dt<=.2f){
            e.elapsed+=dt;e.stalled+=dt;float distance=0;
            for(unsigned k=0;k<3;++k){float d=field<float>(actor,0x48+k*4)-e.position[k];distance+=d*d;}
            auto heading=field<unsigned short>(actor,0x64);
            if(distance>.0025f||std::abs(int(short(heading-e.heading)))>180){
                e.stalled=0;memcpy(e.position,reinterpret_cast<void*>(actor+0x48),12);e.heading=heading;
            }
        }
    }
    // Simulation time: pausing the game does not consume the approach budget.
    if(e.stalled>=2.f||e.elapsed>=8.f){climb_cancel200(actor);climb_fail154(10);return false;}
    if(!e.started){
        if(!proximity_independent200(actor)){climb_publish97(climb97::Approaching);return true;}
        float speed{},turn{};if(!read_native_movement(0,speed,turn)||speed<=0){climb_cancel200(actor);return climb_fail154(5);}
        unsigned char parameters[32]{};
        reinterpret_cast<void(__thiscall*)(void*,unsigned,float,unsigned)>(base+0x3ba540)(parameters,0,speed,0);
        e.started=true;
        reinterpret_cast<void(__thiscall*)(void*,const void*,const void*)>(base+0x3df290)(reinterpret_cast<void*>(actor),e.goal,parameters);
        InterlockedIncrement(&climb_requests153);
    }
    unsigned native=field<unsigned>(actor,0x3bc);InterlockedExchange(&climb_route_state154,LONG(native));
    if(native<1||native>5){climb_cancel200(actor);return false;}
    climb_publish97(climb97::Approaching);return true;
}
static bool climb_pending153(uintptr_t actor){
    if(!climb_align152||!actor||actor!=possessed||!state||state->mode!=POSSESSED||state->stop||state->release||
       !possession_lease.active||GetTickCount64()-state->heartbeat>500)return false;
    __try{Wall97 wall;return followed(actor,false,false)&&!dead_view()&&!field<uintptr_t>(actor,0x18)&&
        read_wall97(actor,climb_entry153.pipe,wall)&&climb_entry_live153(actor,wall)&&
        (!climb_entry153.started||(field<unsigned>(actor,0x3bc)>=1&&field<unsigned>(actor,0x3bc)<=5));
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool climb_blocked199(unsigned reason,bool requested){
    if(requested){InterlockedIncrement(&climb_rejected97);climb_fail154(reason);climb_feedback_until199=GetTickCount64()+1200;}
    climb_publish97(GetTickCount64()<climb_feedback_until199?climb97::Blocked:climb97::Off);return false;
}
static bool climb_prepare97(uintptr_t actor){
    if(climb_native_air99(actor)){InterlockedIncrement(&native_air_updates99);return true;}
    if(wall_subject97(actor)){climb_traversal200=possession_lease.generation;if(climb_align152){InterlockedIncrement(&climb_attaches97);InterlockedIncrement(&climb_arrivals153);}climb_align152=false;climb_entry153={};climb_request199.clear();InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);climb_publish97(crest_subject97(actor)?climb97::Crest:climb97::Climbing,field<float>(actor,0x20),climb97::direction(unsigned(state->input)));return true;}
    climb_traversal200=0;
    auto now=GetTickCount64();bool action=climb_request199.take(possession_lease.generation,now);
    if(climb_lease97!=possession_lease.generation){climb_reset97(true);climb_lease97=possession_lease.generation;}
    // Once admitted, the native route owns movement. Fresh readiness/aim input
    // cannot tear down an otherwise valid asynchronous approach.
    if(climb_align152){
        if(action){climb_cancel200(actor);return false;}
        melee60::Request qte;bool blocked=weapon_mounted()||field<unsigned>(actor,0x1f0)>=18||melee_request(qte);
        if(blocked){climb_cancel200(actor);return climb_blocked199(6,true);}
        Wall97 pending;bool owned=read_wall97(actor,climb_entry153.pipe,pending)&&climb_entry_live153(actor,pending);
        if(owned&&climb_approach153(actor,pending)){InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);return true;}
        climb_cancel200(actor);
        return climb_blocked199(climb_error154==10?10:9,true);
    }
    if(weapon_mounted()||field<unsigned>(actor,0x1f0)>=18||field<uintptr_t>(actor,0x18)||state->weapon_phase!=WEAPON_IDLE||state->aiming){climb_align152=false;climb_near97=0;return climb_blocked199(6,action);}
    melee60::Request qte;if(melee_request(qte)){climb_align152=false;climb_near97=0;return climb_blocked199(7,action);}
    if(!climb_align152&&(action||now>=climb_scan97)){
        climb_scan97=now+100;climb_near97=0;uintptr_t phase=0;
        if(wall_world97(actor,phase)){
            auto array=field<uintptr_t>(phase,0x304);unsigned n=field<unsigned>(phase,0x300);InterlockedExchange(&climb_pipes154,LONG(n));InterlockedExchange(&climb_valid154,0);float best=2.3f*2.3f;
            if(array&&n<=8192)for(unsigned i=0;i<n;++i){auto pipe=field<uintptr_t>(array,i*4);Wall97 w;if(!read_wall97(actor,pipe,w,i))continue;InterlockedIncrement(&climb_valid154);
                {constexpr unsigned top=0;auto node=w.nodes;constexpr unsigned off=0x24;
                    float dx=field<float>(node,off)-field<float>(actor,0x48),dy=field<float>(node,off+4)-field<float>(actor,0x4c),dz=field<float>(node,off+8)-field<float>(actor,0x50),d=dx*dx+dz*dz;
                    if(std::abs(dy)<=1.25f&&d<best&&(d<.16f||dx*std::sin(state->yaw)+dz*std::cos(state->yaw)>0)){
                        best=d;climb_near97=pipe;climb_near_id97=w.id;climb_near_top97=top!=0;
                    }
                }
            }
        }
    }
    if(!climb_near97)return climb_blocked199(8,action);
    Wall97 w;if(!read_wall97(actor,climb_near97,w)||w.id!=climb_near_id97){climb_align152=false;climb_near97=0;return climb_blocked199(8,action);}
    climb_publish97(climb97::NearbyUp);
    if(action&&!climb_align152&&!climb_begin153(actor,w,climb_near_top97)){return climb_blocked199(unsigned(climb_error154),true);}
    if(!climb_align152)return false;
    return climb_approach153(actor,w);
}
static void climb_log97(std::ostream& log){if(state&&state->mode!=POSSESSED)climb_publish97(climb97::Off);log<<",\"siege97\":1,\"climb_attaches97\":"<<climb_attaches97<<",\"climb_steps97\":"<<climb_steps97<<",\"climb_holds97\":"<<climb_holds97<<",\"climb_down97\":"<<climb_down97<<",\"climb_endpoints97\":"<<climb_endpoints97<<",\"climb_rejected97\":"<<climb_rejected97;log<<",\"siege98\":1,\"climb_rate98\":"<<climb_rate98<<",\"climb_step98\":"<<climb_step98<<",\"climb_duplicate98\":"<<climb_duplicate98<<",\"climb_rate_errors98\":"<<climb_rate_errors98<<",\"view_count_bypasses98\":"<<view_count_bypasses98<<",\"view_pose_bypasses98\":"<<view_pose_bypasses98<<",\"view_native_count98\":"<<view_native_count98<<",\"view_zero_count98\":"<<view_zero_count98;log<<",\"landing99\":1,\"native_air_updates99\":"<<native_air_updates99<<",\"native_air_active99\":"<<native_air_active99;log<<",\"native_wall153\":1,\"native_approach_requests153\":"<<climb_requests153<<",\"native_approach_arrivals153\":"<<climb_arrivals153<<",\"native_approach_cancels153\":"<<climb_cancels153;log<<",\"climb_error154\":"<<climb_error154<<",\"wall_resources154\":"<<climb_pipes154<<",\"valid_walls154\":"<<climb_valid154<<",\"native_route_state154\":"<<climb_route_state154;if(climb_shared97)log<<",\"climb_mode97\":"<<climb_shared97->mode<<",\"climb_progress97\":"<<climb_shared97->progress;}
static void create_siege_hooks97(){
    struct Signature97 {unsigned rva;unsigned char bytes[8];};
    const Signature97 signatures[]={
        {0xa0b60,{0xd9,0x41,0x5c,0xc3,0xcc,0xcc,0xcc,0xcc}},
        {0x2d6210,{0x8b,0x41,0x1c,0x8b,0x00,0x8b,0x40,0x08}},
        {0x340d30,{0x81,0xec,0xac,0,0,0,0x56,0x8b}},
        {0x340cf0,{0xff,0x74,0x24,4,0xe8,0x77,0x10,0xfe}},
        {0x3945c0,{0x83,0xec,0x0c,0x53,0x56,0x8b,0xf1,0x57}},
        {0x3cf280,{0x53,0x55,0x56,0x57,0x8b,0x7c,0x24,0x14}},
        {0x3c1ba0,{0x53,0x56,0x8b,0xf1,0x57,0x8b,0x7c,0x24}},
        {0x351e40,{0x8b,0x44,0x24,0x04,0x56,0x8b,0xf1,0xf3}},
        {0x3ba540,{0x8a,0x44,0x24,0x04,0xf3,0x0f,0x10,0x44}},
        {0x3df290,{0x56,0x8b,0xf1,0xe8,0x18,0xe7,0xff,0xff}},
        {0x3dfe30,{0x56,0xff,0x74,0x24,0x14,0x8b,0xf1,0xff}},
        {0x39a2e0,{0x51,0xf3,0x0f,0x10,0x44,0x24,0x0c,0x53}}};
    for(const auto& sig:signatures)require(!memcmp(reinterpret_cast<void*>(base+sig.rva),sig.bytes,8),"Native siege signature differs");
    const unsigned char request200[]={0x83,0xec,0x1c,0x53,0x56,0x57,0x8b,0xf9};
    const unsigned char cancel200[]={0x56,0x8b,0xf1,0x8b,0x8e,0x50,0x04,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x3ccd00),request200,8)&&
        !memcmp(reinterpret_cast<void*>(base+0x3df380),cancel200,8),"Native personal wall route signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3ccd00),reinterpret_cast<void*>(&climb_path_request200),reinterpret_cast<void**>(&original_climb_path_request200))==MH_OK,"Personal wall route hook failed");
    const unsigned char count98[]={0x81,0xec,0xb8,0,0,0,0x56,0x8b},pose98[]={0x81,0xec,0xb8,0,0,0,0xf3,0x0f};
    require(!memcmp(reinterpret_cast<void*>(base+0x2fcb10),count98,8)&&!memcmp(reinterpret_cast<void*>(base+0x341560),pose98,8),"Native siege view count/pose signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x2fcb10),reinterpret_cast<void*>(&view_count_hook98),reinterpret_cast<void**>(&original_view_count98))==MH_OK,"Siege view count hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x341560),reinterpret_cast<void*>(&view_pose_hook98),reinterpret_cast<void**>(&original_view_pose98))==MH_OK,"Siege view pose hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x2d6210),reinterpret_cast<void*>(&view_index_hook97),reinterpret_cast<void**>(&original_view_index97))==MH_OK,"Siege view index hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x340d30),reinterpret_cast<void*>(&view_actor_hook97),reinterpret_cast<void**>(&original_view_actor97))==MH_OK,"Siege view actor hook failed");
    const unsigned char update104[]={0x83,0xec,0x78,0xf3,0x0f,0x10,0x05},holder104[]={0x53,0x56,0x8b,0x71,0x18,0x33,0xd2,0x57};
    require(!memcmp(reinterpret_cast<void*>(base+0x325ca0),update104,sizeof update104)&&
        field<uintptr_t>(base+0x325ca0,7)==base+0x1931f64&&!memcmp(reinterpret_cast<void*>(base+0x321d70),holder104,8),"Native camera roster104 signatures differ");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x325ca0),reinterpret_cast<void*>(&camera_update104),reinterpret_cast<void**>(&original_camera_update104))==MH_OK,"Native camera scope104 hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x321d70),reinterpret_cast<void*>(&view_holder104),reinterpret_cast<void**>(&original_view_holder104))==MH_OK,"Native camera holder104 hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3945c0),reinterpret_cast<void*>(&pipe_advance_hook97),reinterpret_cast<void**>(&original_pipe_advance97))==MH_OK,"Siege traversal hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x3cf280),reinterpret_cast<void*>(&pipe_mode_hook97),reinterpret_cast<void**>(&original_pipe_mode97))==MH_OK,"Siege mode hook failed");
    wchar_t name[96];climb97::mapping_name(name,GetCurrentProcessId());climb_mapping97=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(climb97::Status),name);
    require(climb_mapping97!=nullptr,"Climb telemetry mapping failed");climb_shared97=static_cast<climb97::Status*>(MapViewOfFile(climb_mapping97,FILE_MAP_ALL_ACCESS,0,0,sizeof(climb97::Status)));
    require(climb_shared97!=nullptr,"Climb telemetry view failed");*climb_shared97=climb97::Status{};climb_shared97->pid=GetCurrentProcessId();
}
