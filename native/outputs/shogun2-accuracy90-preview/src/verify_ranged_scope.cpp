// Exercise the production weapon hooks outside the game with synthetic native
// object layouts. No process injection, game input, or live battle test.
#include "identity.h"
#include <sstream>
#include "steering.h"
#include "aim_geometry.h"
#include "../vendor/include/MinHook.h"
static uintptr_t base,possessed,unit,environment;
static volatile LONG enabled=1;
static Status storage{},*state=&storage;
static bool identity_valid=true;static bool mounted_weapon=false;
static bool weapon_mounted(){return mounted_weapon;}
static bool personal_direct_projectile(uintptr_t p){return p!=0;}
static bool is_bow_projectile(uintptr_t p){return p&&*reinterpret_cast<unsigned*>(p)==3;}
static uintptr_t record;
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
static bool followed(uintptr_t actor,bool){return identity_valid&&actor==possessed&&field<uintptr_t>(actor,0x214)==unit;}
static uintptr_t bow_record(uintptr_t actor_unit){return actor_unit==unit?record:0;}
static bool weapon_ready_held(){return false;}
using NativeGet=void*(__thiscall*)(void*);
static bool posed_arrow_origin(float*){return false;}
static void pose_manual_origin(float*){}
static bool read_aim_frame(ViewRay&){return false;}
static void manual_bow_apply(const uint32_t*,uintptr_t){}
#include "equipment_policy81.h"
static volatile LONG equipment_personal_override214{};
static LONG equipment_slot81=equipment81::Ranged;
static bool equipment_crouched81(){return false;}
// This isolated fixture covers ordinary native firing. Full command-mode
// behavior is exercised with actual adapter ownership in verify_native_commands100.
static bool command_draw_permitted100(void*){return true;}
static void command_draw_started100(void*){}
static bool command_release_managed100(void*){return false;}
static void command_on_shot100(){}
// Standalone firing fixture: presentation eligibility is an external provider.
// Full native projectile presentation is covered by verify_melee_presentation137.
#include "personal_weapon_catalog.h"
static PersonalWeaponKind native_personal_kind(uintptr_t){return PersonalWeaponKind::Unsupported;}
#include "fixture_weapon_edges143.h"
#include "weapon_animation.h"
static unsigned gate_calls,draw_calls,release_calls,reload_calls,play_calls,change_calls,stop_calls;
static bool recovery_test;
static uintptr_t last_actor;
struct Task {
    uintptr_t vtable{},actor{};
    unsigned char body[0x98]{};
    __declspec(noinline) bool gate(){++gate_calls;return body[0x88]!=0;}
    __declspec(noinline) void draw(){++draw_calls;last_actor=actor;}
    __declspec(noinline) void release(){++release_calls;last_actor=actor;}
};
struct Actor {
    unsigned char bytes[0x600]{};
    __declspec(noinline) void stop(){++stop_calls;last_actor=reinterpret_cast<uintptr_t>(this);}
    __declspec(noinline) void reload(){++reload_calls;last_actor=reinterpret_cast<uintptr_t>(this);}
    __declspec(noinline) void play(unsigned clip,unsigned force,unsigned attached,float speed){
        ++play_calls;last_actor=reinterpret_cast<uintptr_t>(this);
        require(clip==130&&force==1&&attached==0&&speed==1.25f,"AI animation arguments changed");}
    __declspec(noinline) void change(unsigned mode,unsigned skip,unsigned attached){
        ++change_calls;last_actor=reinterpret_cast<uintptr_t>(this);
        require(recovery_test?(mode==29&&skip==0&&attached==0):(mode==16&&skip==0&&attached==1),"AI state arguments changed");}
};
template<class T>static void* address(T member){static_assert(sizeof(member)==4,"x86 member ABI");void* p;memcpy(&p,&member,4);return p;}
static void check_forwarded(Task& task,Actor& actor){
    auto before=task;unsigned old_gate=gate_calls;
    for(bool ready:{false,true}){
        task.body[0x88]=ready;
        require(weapon_ready_gate(&task,nullptr)==ready&&weapon_draw_gate(&task,nullptr)==ready&&
                weapon_task_reload_gate(&task,nullptr)==ready&&weapon_finish_gate(&task,nullptr)==ready&&weapon_task_animation_gate95(&task,nullptr)==ready&&weapon_task_lower_gate96(&task,nullptr)==ready&&weapon_task_idle_gate96(&task,nullptr)==ready,"AI readiness result changed");}
    require(gate_calls==old_gate+14,"AI gates were skipped or duplicated");task.body[0x88]=before.body[0x88];
    unsigned old_draw=draw_calls,old_release=release_calls,old_reload=reload_calls,old_play=play_calls,old_change=change_calls;
    weapon_draw_hook(&task,nullptr);weapon_release_hook(&task,nullptr);
    weapon_reload_tick_hook(&actor,nullptr);weapon_reload_tick_hook(&actor,nullptr);
    weapon_play_hook(&actor,nullptr,130,1,0,1.25f);weapon_change_hook(&actor,nullptr,16,0,1);
    require(draw_calls==old_draw+1&&release_calls==old_release+1&&reload_calls==old_reload+2&&play_calls==old_play+1&&change_calls==old_change+1,"AI callbacks did not forward exactly once per call");
    require(last_actor==reinterpret_cast<uintptr_t>(&actor)&&!memcmp(&task,&before,sizeof task),"Another AI actor or task changed");
}
static unsigned recovery_plays103,recovery_changes103,recovery_expected_clip103;
static void __fastcall recovery_play103(void*,void*,unsigned clip,unsigned force,unsigned attached,float speed){require(clip==recovery_expected_clip103&&force==1&&attached==0&&speed==1.25f,"Recovery playback arguments changed");++recovery_plays103;}
static void __fastcall recovery_change103(void*,void*,unsigned mode,unsigned skip,unsigned attached){require(prone_mode103(mode)&&skip==0&&attached==1,"Prone transition arguments changed");++recovery_changes103;}
int main(){try{
    original_task_lower_gate96=original_task_idle_gate96=original_task_animation_gate95=original_ready_gate=original_draw_gate=original_task_reload_gate=original_finish_gate=reinterpret_cast<RangedGate>(address(&Task::gate));
    original_draw=reinterpret_cast<RangedAction>(address(&Task::draw));original_native_release=reinterpret_cast<RangedAction>(address(&Task::release));
    original_reload_tick=reinterpret_cast<RangedAction>(address(&Actor::reload));original_play=reinterpret_cast<PlayAnimation>(address(&Actor::play));original_change=reinterpret_cast<ChangeState>(address(&Actor::change));
    Actor player,same_unit,other_bow,matchlock,enemy,unsupported;
    for(Actor* a:{&player,&same_unit,&other_bow,&matchlock,&enemy,&unsupported})field<unsigned char>(reinterpret_cast<uintptr_t>(a),0xf0)=1;
    unit=0x100000;possessed=reinterpret_cast<uintptr_t>(&player);field<uintptr_t>(possessed,0x214)=unit;
    field<uintptr_t>(reinterpret_cast<uintptr_t>(&same_unit),0x214)=unit;
    unsigned native_record=3;record=reinterpret_cast<uintptr_t>(&native_record);
    Task controlled;controlled.actor=possessed;
    state->mode=POSSESSED;state->aim_pose=AIM_READY;state->weapon_phase=WEAPON_RELOAD;
    unsigned scenarios=0;
    for(unsigned weapon:{3u,60u,4u,15u,47u,82u}){
        native_record=weapon;
        for(unsigned i=0;i<1000;++i){
            for(Actor* ai:{&same_unit,&other_bow,&matchlock,&enemy,&unsupported}){
                Task task;task.actor=reinterpret_cast<uintptr_t>(ai);check_forwarded(task,*ai);++scenarios;}
            unsigned old=gate_calls;
            require(weapon_ready_gate(&controlled,nullptr)&&weapon_draw_gate(&controlled,nullptr)&&weapon_task_reload_gate(&controlled,nullptr)&&weapon_finish_gate(&controlled,nullptr)&&weapon_task_animation_gate95(&controlled,nullptr)&&weapon_task_lower_gate96(&controlled,nullptr)&&weapon_task_idle_gate96(&controlled,nullptr)&&gate_calls==old,"Controlled AI task still blocks its squad");
            unsigned draw_before=draw_calls,release_before=release_calls;
            weapon_draw_hook(&controlled,nullptr);weapon_release_hook(&controlled,nullptr);
            require(draw_calls==draw_before&&release_calls==release_before,"Controlled actor emitted native AI shot");
            auto b=reinterpret_cast<unsigned char*>(&controlled);require(b[0x84]==1&&b[0x6c]==1&&b[0x6d]==1&&b[0x6e]==1,"Controlled task not acknowledged");
        }
    }
    // Mounted bow ownership uses the rider modes, never a squad-wide gate.
    mounted_weapon=true;native_record=3;
    for(unsigned mode=29;mode<=33;++mode){
        field<unsigned>(possessed,0x1f0)=mode;
        unsigned old=gate_calls;
        require(weapon_owned(possessed)&&weapon_ready_gate(&controlled,nullptr)&&weapon_draw_gate(&controlled,nullptr)&&
            weapon_task_reload_gate(&controlled,nullptr)&&weapon_finish_gate(&controlled,nullptr)&&weapon_task_animation_gate95(&controlled,nullptr)&&weapon_task_lower_gate96(&controlled,nullptr)&&weapon_task_idle_gate96(&controlled,nullptr)&&gate_calls==old,"Mounted rider still blocks squad readiness");
        for(Actor* ai:{&same_unit,&other_bow,&matchlock,&enemy,&unsupported}){
            Task task;task.actor=reinterpret_cast<uintptr_t>(ai);check_forwarded(task,*ai);++scenarios;
        }
    }
    // Selecting melee on a ranged soldier must still complete that member's
    // squad barriers; other members continue their normal ranged pipeline.
    equipment_slot81=equipment81::Melee;state->weapon_phase=0;state->aim_pose=0;
    unsigned old_gate=gate_calls,old_draw=draw_calls,old_release=release_calls,old_play=play_calls,old_change=change_calls;
    require(weapon_ready_gate(&controlled,nullptr)&&weapon_draw_gate(&controlled,nullptr)&&weapon_task_reload_gate(&controlled,nullptr)&&weapon_finish_gate(&controlled,nullptr)&&weapon_task_animation_gate95(&controlled,nullptr)&&weapon_task_lower_gate96(&controlled,nullptr)&&weapon_task_idle_gate96(&controlled,nullptr)&&gate_calls==old_gate,"Melee equipment blocked the ranged squad");
    weapon_draw_hook(&controlled,nullptr);weapon_release_hook(&controlled,nullptr);weapon_play_hook(&player,nullptr,130,1,0,1.25f);weapon_change_hook(&player,nullptr,14,0,0);
    require(draw_calls==old_draw&&release_calls==old_release&&play_calls==old_play&&change_calls==old_change,"Melee equipment admitted an AI ranged shot/pose");
    Task peer;peer.actor=reinterpret_cast<uintptr_t>(&same_unit);check_forwarded(peer,same_unit);++scenarios;
    equipment_slot81=equipment81::Ranged;state->weapon_phase=WEAPON_RELOAD;state->aim_pose=AIM_READY;
    for(unsigned mode:{0u,18u,28u,34u,99u}){
        field<unsigned>(possessed,0x1f0)=mode;require(!weapon_owned(possessed),"Unsupported mounted transition admitted");
        check_forwarded(controlled,player);
    }
    mounted_weapon=false;field<unsigned>(possessed,0x1f0)=0;
    // Recovery blocks manual sequencing without restoring this actor's native
    // ranged waits. Peers still run their independent original task pipeline.
    for(unsigned clip:{0x455u,0x456u}){
        unsigned active[4]={clip,0,0,0};field<uintptr_t>(possessed,0x474)=reinterpret_cast<uintptr_t>(active);field<unsigned>(possessed,0x1d4)=clip;
        state->weapon_phase=WEAPON_DRAW;state->aim_pose=AIM_READY;state->aiming=state->fire_request=1;auto original_actor=player;
        auto gates=gate_calls;
        require(weapon_owned(possessed)&&weapon_task_lower_gate96(&controlled,nullptr)&&weapon_task_idle_gate96(&controlled,nullptr)&&weapon_task_animation_gate95(&controlled,nullptr)&&gate_calls==gates,"Recovery blocked same-unit ranged task completion");
        weapon_tick(possessed);require(!state->weapon_phase&&!state->aim_pose&&!state->aiming&&!state->fire_request&&!memcmp(&player,&original_actor,sizeof player),"Recovery tick canceled native animation or retained manual sequence");
        Task peer;peer.actor=reinterpret_cast<uintptr_t>(&same_unit);check_forwarded(peer,same_unit);
        field<uintptr_t>(possessed,0x474)=0;field<unsigned>(possessed,0x1d4)=0;state->weapon_phase=WEAPON_DRAW;state->aim_pose=AIM_READY;
        auto saved_play=original_play;original_play=reinterpret_cast<PlayAnimation>(&recovery_play103);recovery_expected_clip103=clip;auto plays=recovery_plays103;
        weapon_play_hook(&player,nullptr,clip,1,0,1.25f);original_play=saved_play;
        require(recovery_plays103==plays+1&&!state->weapon_phase&&!state->aim_pose,"Manual aim suppressed incoming get-up clip");
    }
    for(unsigned mode:{12u,13u}){state->weapon_phase=WEAPON_DRAW;state->aim_pose=AIM_READY;
        auto saved_change=original_change;original_change=reinterpret_cast<ChangeState>(&recovery_change103);auto changes=recovery_changes103;
        weapon_change_hook(&player,nullptr,mode,0,1);original_change=saved_change;
        require(recovery_changes103==changes+1&&!state->weapon_phase&&!state->aim_pose,"Manual aim suppressed native prone transition");
    }
    state->weapon_phase=WEAPON_RELOAD;state->aim_pose=AIM_READY;
    // Releasing, disabling, invalid identity, unsupported personal weapons and
    // native melee/death transitions must restore all original AI behavior.
    for(unsigned scenario=0;scenario<8;++scenario){
        state->mode=POSSESSED;state->stop=state->release=0;enabled=1;identity_valid=true;record=reinterpret_cast<uintptr_t>(&native_record);field<unsigned>(possessed,0x1f0)=0;
        if(scenario==0)state->mode=IDLE;if(scenario==1)state->release=1;if(scenario==2)state->stop=1;
        if(scenario==3)enabled=0;if(scenario==4)identity_valid=false;if(scenario==5)record=0;if(scenario==6)field<unsigned>(possessed,0x1f0)=18;
        if(scenario==7)field<unsigned char>(possessed,0xf0)=0;
        check_forwarded(controlled,player);
    }
    // Ownership changes to a different actor without affecting the old soldier.
    state->mode=POSSESSED;state->stop=state->release=0;enabled=1;identity_valid=true;record=reinterpret_cast<uintptr_t>(&native_record);
    possessed=reinterpret_cast<uintptr_t>(&same_unit);check_forwarded(controlled,player);
    // Actual production recovery must lower once at the owned simulation tick,
    // retain FPS mode, cancel an unfired shot, and discard another lease's fault.
    possessed=reinterpret_cast<uintptr_t>(&player);field<unsigned char>(possessed,0xf0)=1;field<unsigned>(possessed,0x1f0)=32;
    mounted_weapon=true;native_record=3;record=reinterpret_cast<uintptr_t>(&native_record);state->lease_generation=71;
    report_mounted_pose_failure(44);++state->lease_generation;require(take_mounted_pose_failure(possessed)==0,"Retired lease fault applied to new possession");
    report_mounted_pose_failure(43);require(take_mounted_pose_failure(reinterpret_cast<uintptr_t>(&same_unit))==0,"Another actor received pose fault");
    state->aim_pose=AIM_BOW_DRAWING;state->aiming=1;state->fire_request=1;state->weapon_phase=WEAPON_DRAW;
    base=reinterpret_cast<uintptr_t>(address(&Actor::stop))-0x3ca920;recovery_test=true;
    unsigned changes=change_calls;report_mounted_pose_failure(44);weapon_tick(possessed);
    require(stop_calls==1&&change_calls==changes+1&&state->mode==POSSESSED&&!state->release&&
        !state->aim_pose&&!state->aiming&&!state->fire_request&&state->weapon_phase==WEAPON_IDLE&&state->pose_error==44,
        "Mounted pose failure did not lower safely while retaining FPS");
    std::ostringstream diagnostic;log_mounted_pose_diagnostic(diagnostic);
    require(diagnostic.str().find("mounted_pose_rejection65")!=std::string::npos,"Mounted fault diagnostic was not logged");
    std::ostringstream duplicate;log_mounted_pose_diagnostic(duplicate);require(duplicate.str().empty(),"Mounted diagnostic repeated for one lease");
    std::cout<<"PASS: actual production draw/release, seven readiness gates, reload, animation and state hooks; "<<scenarios<<" other-actor scenarios across bow/firearm/thrown projectile records; same-unit, other bow, matchlock, enemy and unsupported actors forwarded exactly; release/disable/identity/death/weapon guards and ownership switching.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
