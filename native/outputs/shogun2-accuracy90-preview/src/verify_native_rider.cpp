// Exercise the actual mounted weapon sequencer with synthetic native objects.
// The engine stubs live in this test process; no game process is accessed.
#include "identity.h"
#include "../../../common/fixture_paths.h"
#include <fstream>
#include <sstream>
#include "steering.h"
#include "aim_geometry.h"
#include "../vendor/include/MinHook.h"
static uintptr_t base,possessed,unit,environment,record,clip_address;
static volatile LONG enabled=1;
static Status storage{},*state=&storage;
static bool held=true,identity_valid=true;
static unsigned shots,plays,clock_calls,event_calls,task_draws,task_releases,task_gates,ready_transition_starts;static float marker;static unsigned clip_length;
static bool mounted=true;
static unsigned lowered,raised,foot_release_frame;
static bool weapon_mounted(){return mounted;}
static bool personal_direct_projectile(uintptr_t p){return p!=0;}
static bool is_bow_projectile(uintptr_t p){return p&&*reinterpret_cast<unsigned*>(p)==3;}
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
static bool followed(uintptr_t actor,bool){return identity_valid&&actor==possessed&&field<uintptr_t>(actor,0x214)==unit;}
static uintptr_t bow_record(uintptr_t u){return u==unit?record:0;}
static bool weapon_ready_held(){return held&&state->aiming;}
using NativeGet=void*(__thiscall*)(void*);
static bool posed_arrow_origin(float* out){out[0]=0;out[1]=2.3f;out[2]=1.f;return true;}
static void pose_manual_origin(float*){}
static bool read_aim_frame(ViewRay& ray){ray.origin[0]=0;ray.origin[1]=2.4f;ray.origin[2]=0;ray.direction[0]=0;ray.direction[1]=0;ray.direction[2]=1;return true;}
static void manual_bow_apply(const uint32_t*,uintptr_t){
    ++shots;++state->shots_spawned;field<float>(unit,0xf78)-=1;
    if(!mounted){
        foot_release_frame=field<unsigned>(possessed,0x1d8);
        auto& rounds=field<unsigned>(possessed,0x394);
        if(!rounds)rounds=6;
        if(!--rounds)field<float>(possessed,0x390)=2.5f;
    }
}
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
struct Actor {
    unsigned char bytes[0x700]{};
    __declspec(noinline) void task_draw(){++task_draws;}
    __declspec(noinline) void task_release(){++task_releases;}
    __declspec(noinline) bool task_gate(){++task_gates;return false;}
    __declspec(noinline) bool events(void*,unsigned){++event_calls;return true;}
    __declspec(noinline) void clock(){
        ++clock_calls;auto p=reinterpret_cast<uintptr_t>(this);
        if(const auto active=field<uintptr_t>(p,0x474)){
            weapon_animation_events_hook(this,nullptr,reinterpret_cast<void*>(active),field<unsigned>(p,0x1d8));
            field<unsigned>(p,0x1d4)=field<unsigned>(active,0);
            field<unsigned>(p,0x1d0)=0x21;
            const auto frames=mounted?clip_length:field<unsigned>(field<uintptr_t>(active,4),0x64);
            if(++field<unsigned>(p,0x1d8)>=frames)field<uintptr_t>(p,0x474)=0;
        }
    }
    __declspec(noinline) void stop(){field<uintptr_t>(reinterpret_cast<uintptr_t>(this),0x474)=0;}
    __declspec(noinline) void reload(){auto& r=field<float>(reinterpret_cast<uintptr_t>(this),0x390);r=std::max(0.f,r-.05f);}
    __declspec(noinline) unsigned choose(const float*){return 130;}
    __declspec(noinline) float release(unsigned clip,unsigned secondary){
        require(clip==(mounted?758u:130u)&&secondary==0,"Wrong marker lookup");return marker;
    }
    __declspec(noinline) void* origin(float* out,unsigned clip,unsigned secondary){
        require(clip==(mounted?758u:130u)&&secondary==0,"Wrong shot origin");out[0]=0;out[1]=2.3f;out[2]=1;return out;
    }
    __declspec(noinline) void play(unsigned clip,unsigned force,unsigned attached,float speed){
        require((mounted?clip==758:(clip==124||clip==130))&&(mounted?force==1:force<=1)&&attached==0&&speed==1,"Invalid native weapon play");
        ++plays;
        auto p=reinterpret_cast<uintptr_t>(this);
        auto clips=field<uintptr_t>(field<uintptr_t>(p,0x23c),0xc);
        field<unsigned>(p,0x1d4)=clip;field<unsigned>(p,0x1d8)=0;field<uintptr_t>(p,0x474)=field<uintptr_t>(clips,clip*4);
    }
    __declspec(noinline) void change(unsigned mode,unsigned skip,unsigned attached){
        if(!mounted){
            require((mode==8||mode==14||mode==16)&&skip<=1&&attached==0,"Invalid foot revolver mode");
            field<unsigned>(reinterpret_cast<uintptr_t>(this),0x1f0)=mode;
            if(mode==8)++lowered;
            if(mode==14&&!skip)++raised;
            stop();
            return;
        }
        require(skip<=1&&attached==0&&(mode==29||mode==32||mode==33),"Mounted state escaped native rider range");
        auto p=reinterpret_cast<uintptr_t>(this);
        require(!skip||(mode==32&&(mounted_carbine_native_profile(p)||mounted_bow_native_profile85(p))),"Transition skip leaked to lowering/reload/another weapon");
        bool changed=field<unsigned>(p,0x1f0)!=mode;field<unsigned>(p,0x1f0)=mode;stop();
        // Model the native automatic mode-entry animation. Unlike the former
        // stop-only fixture, this exposes an extra clip started before758.
        if(changed&&mode==32&&!skip){++ready_transition_starts;field<unsigned>(p,0x1d4)=751;field<unsigned>(p,0x1d8)=0;}
        if(mode==33)field<float>(p,0x390)=.3f;
    }
};
template<class T>static void* address(T member){static_assert(sizeof(member)==4,"x86 member ABI");void* p;memcpy(&p,&member,4);return p;}
static void stub(unsigned rva,void* function){auto p=reinterpret_cast<unsigned char*>(base+rva);p[0]=0xb8;auto target=reinterpret_cast<uintptr_t>(function);memcpy(p+1,&target,4);p[5]=0xff;p[6]=0xe0;}
static void original_foot_clock(){
    // This leaf uses only relative calls to the explicit event/stop providers.
    // Copy its original .text bytes at their decoded RVA into the test image.
    std::ifstream file(fixture_paths::engine(),std::ios::binary);
    IMAGE_DOS_HEADER dos{};file.read(reinterpret_cast<char*>(&dos),sizeof dos);
    require(file&&dos.e_magic==IMAGE_DOS_SIGNATURE,"Native foot clock engine missing");
    IMAGE_NT_HEADERS32 nt{};file.seekg(dos.e_lfanew);file.read(reinterpret_cast<char*>(&nt),sizeof nt);
    require(file&&nt.Signature==IMAGE_NT_SIGNATURE&&nt.FileHeader.NumberOfSections<=32,"Native foot clock PE differs");
    file.seekg(dos.e_lfanew+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+nt.FileHeader.SizeOfOptionalHeader);
    constexpr unsigned rva=0x3f0370,size=0x5a;
    for(unsigned i=0;i<nt.FileHeader.NumberOfSections;++i){
        IMAGE_SECTION_HEADER section{};file.read(reinterpret_cast<char*>(&section),sizeof section);
        if(rva<section.VirtualAddress||rva-section.VirtualAddress+size>section.SizeOfRawData)continue;
        file.seekg(section.PointerToRawData+rva-section.VirtualAddress);
        auto code=reinterpret_cast<unsigned char*>(base+rva);
        file.read(reinterpret_cast<char*>(code),size);
        const unsigned char start[]={0x56,0x8b,0xf1,0x8b,0x8e,0x74,4,0};
        require(file&&!memcmp(code,start,sizeof start)&&code[0x27]==0xe8&&code[0x53]==0xe9&&code[0x59]==0xc3,
            "Original foot animation clock instructions differ");
        stub(0x3f03d0,address(&Actor::events));
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(base+rva),0x67);
        original_animation_clock=reinterpret_cast<RangedAction>(base+rva);
        return;
    }
    throw std::runtime_error("Native foot animation clock section missing");
}
static void tick(unsigned length){
    ++field<LONG>(field<uintptr_t>(environment,8),0x50);
    // Reproduce native 3e4850 before 3f0370: mode32 becomes state48.
    // Test67 failed here because it skipped the clock's state33 refresh.
    field<unsigned>(possessed,0x1d0)=0x30;
    const bool returning_to_aim=!mounted&&held&&state->weapon_phase==WEAPON_FIRED&&
        field<unsigned>(possessed,0x394)>0;
    clip_length=length;weapon_animation_clock_hook(reinterpret_cast<void*>(possessed),nullptr);
    if(returning_to_aim)
        require(field<uintptr_t>(possessed,0x474)!=0,"Held ADS was lost between the native clock and weapon tick");
    weapon_tick(possessed);
}
int main(){try{
    base=reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr,0x15b3000,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));require(base!=0,"Cannot allocate synthetic engine");
    stub(0x3ca920,address(&Actor::stop));stub(0x3d2ef0,address(&Actor::release));stub(0x3e0b10,address(&Actor::origin));field<float>(base,0x15b2448)=.05f;
    original_draw=reinterpret_cast<RangedAction>(address(&Actor::task_draw));
    original_native_release=reinterpret_cast<RangedAction>(address(&Actor::task_release));
    original_ready_gate=original_draw_gate=original_task_reload_gate=original_finish_gate=reinterpret_cast<RangedGate>(address(&Actor::task_gate));
    original_animation_clock=reinterpret_cast<RangedAction>(address(&Actor::clock));
    original_animation_events=reinterpret_cast<AnimationEvents>(address(&Actor::events));
    original_play=reinterpret_cast<PlayAnimation>(address(&Actor::play));original_change=reinterpret_cast<ChangeState>(address(&Actor::change));original_reload_tick=reinterpret_cast<RangedAction>(address(&Actor::reload));
    unsigned char common[0x80]{},env[0x20]{},unit_bytes[0x1100]{},db[0x20]{},asset[0xa0]{};
    uintptr_t clips[0x4a3]{};unsigned clip[2]{758,reinterpret_cast<unsigned>(asset)};
    environment=reinterpret_cast<uintptr_t>(env);field<uintptr_t>(environment,8)=reinterpret_cast<uintptr_t>(common);
    unit=reinterpret_cast<uintptr_t>(unit_bytes);field<uintptr_t>(reinterpret_cast<uintptr_t>(db),0xc)=reinterpret_cast<uintptr_t>(clips);
    clip_address=reinterpret_cast<uintptr_t>(clip);clips[758]=clip_address;
    unsigned native_record[4]{3};record=reinterpret_cast<uintptr_t>(native_record);Actor actor,horse;
    unsigned char horse_before[sizeof horse];memcpy(horse_before,&horse,sizeof horse);
    for(unsigned kind:{3u,60u,61u}){
        actor={};storage={};mounted_draw={};weapon_actor=0;reload_actor=0;shots=plays=ready_transition_starts=0;held=true;identity_valid=true;
        possessed=reinterpret_cast<uintptr_t>(&actor);field<unsigned char>(possessed,0xf0)=1;field<unsigned>(possessed,0x1f0)=29;
        field<uintptr_t>(possessed,0x214)=unit;field<uintptr_t>(possessed,0x23c)=reinterpret_cast<uintptr_t>(db);
        native_record[0]=kind;marker=kind==3?.50f:kind==60?2.35f:1.57f;
        unsigned length=kind==3?18:kind==60?57:69;field<unsigned>(reinterpret_cast<uintptr_t>(asset),0x64)=length;
        static const wchar_t rifle_path[]=L"animations/shogun_animation/battle/men/carbine/carbine_riders/c_car_f_fire_level.anim";
        static const wchar_t bow_path[]=L"animations/shogun_animation/battle/men/bow/bow_riders/rider_bow_f_fire.anim";
        auto ap=reinterpret_cast<uintptr_t>(asset);
        field<unsigned>(ap,8)=kind==60?unsigned(sizeof(rifle_path)/sizeof(wchar_t)-1):0;
        field<uintptr_t>(ap,16)=kind==60?reinterpret_cast<uintptr_t>(rifle_path):0;field<float>(ap,0x90)=marker;
        if(kind==3){field<unsigned>(ap,8)=unsigned(sizeof(bow_path)/sizeof(wchar_t)-1);field<uintptr_t>(ap,16)=reinterpret_cast<uintptr_t>(bow_path);}
        require(mounted_carbine_native_profile(possessed)==(kind==60),"Rifle profile admitted another weapon or missed verified native asset");
        require(mounted_bow_native_profile85(possessed)==(kind==3),"Bow profile admitted another weapon or missed captured Yabusame asset");
        require(!mounted_skip_raise85(possessed,false),"Lowering transition was suppressed");
        if(kind==3){
            field<unsigned>(ap,0x64)=19;require(!mounted_skip_raise85(possessed,true),"Unknown bow frame count admitted");field<unsigned>(ap,0x64)=length;
            field<float>(ap,0x90)=.6f;require(!mounted_skip_raise85(possessed,true),"Unknown bow release marker admitted");field<float>(ap,0x90)=marker;
            field<uintptr_t>(ap,16)=reinterpret_cast<uintptr_t>(rifle_path);require(!mounted_skip_raise85(possessed,true),"Unknown bow path admitted");field<uintptr_t>(ap,16)=reinterpret_cast<uintptr_t>(bow_path);
        }
        field<float>(unit,0xf78)=100;state->mode=POSSESSED;state->aiming=1;state->lease_generation=kind;
        // Captured Test71 ordering: RMB arrives 15..94ms before native draw.
        // The previous release leaves clip758/frame23 but no active animation.
        field<unsigned>(possessed,0x1d4)=758;field<unsigned>(possessed,0x1d8)=23;
        require(!mounted_carbine_pose_aim(possessed,true),"Raw RMB aligned a stale lowered rifle before native draw");
        tick(length);require(state->aim_pose==AIM_BOW_DRAWING&&state->aim_clip==758&&plays==1&&!shots,"RMB did not start native rider draw without firing");
        require(ready_transition_starts==(kind==61?1u:0u),"Verified bow/rifle still double-starts ready animation, or pistol behavior changed");
        require(field<unsigned>(possessed,0x1d8)==0&&mounted_carbine_pose_aim(possessed,true),"Rifle transition did not begin at actual native draw frame zero");
        require(!mounted_carbine_pose_aim(possessed,false)&&!mounted_carbine_pose_aim(reinterpret_cast<uintptr_t>(&horse),true),"Rifle render gate admitted released RMB or another actor");
        uint32_t payload[13]{};payload[11]=unsigned(possessed);
        begin_weapon(payload,123);require(state->weapon_phase==WEAPON_IDLE&&!shots,"Early click fired before mounted ready");
        for(unsigned n=0;state->aim_pose!=AIM_READY&&n<200;++n)tick(length);
        require(state->aim_pose==AIM_READY&&mounted_draw_owned(possessed),"Mounted draw never became ready");
        unsigned hold_frame=field<unsigned>(possessed,0x1d8),old_plays=plays;
        require(float(hold_frame+1)*.05f<marker,"Hold reached native fire/equipment event");
        unsigned clock_before=clock_calls,event_before=event_calls;
        for(unsigned n=0;n<600;++n){
            tick(length);
            require(field<unsigned>(possessed,0x1d0)==0x21&&field<unsigned>(possessed,0x1d4)==758,"Held rider drifted to ready state instead of active animation");
        }
        require(clock_calls==clock_before&&event_calls==event_before,"Held frame still advanced/replayed root events");
        require(!weapon_animation_events_hook(&actor,nullptr,reinterpret_cast<void*>(clip_address),hold_frame)&&event_calls==event_before,
                "Alternate event caller replayed held root motion");
        weapon_animation_clock_hook(&horse,nullptr);require(clock_calls==clock_before+1,"Other actor clock was suppressed");
        require(weapon_animation_events_hook(&horse,nullptr,reinterpret_cast<void*>(clip_address),hold_frame)&&event_calls==event_before+1,
                "Other actor root motion was suppressed");
        require(field<unsigned>(possessed,0x1d8)==hold_frame&&!shots&&field<float>(unit,0xf78)==100&&plays==old_plays,"RMB hold advanced or consumed ammunition");
        begin_weapon(payload,123);require(state->weapon_phase==WEAPON_DRAW&&plays==old_plays&&field<unsigned>(possessed,0x1d8)==hold_frame,"LMB restarted the mounted clip");
        require(mounted_carbine_pose_aim(possessed,true),"LMB dropped rifle pose alignment during native continuation");
        for(unsigned n=0;!shots&&n<10;++n)tick(length);
        require(shots==1&&state->weapon_animation==758&&field<float>(unit,0xf78)==99,"LMB did not release one native projectile");
        for(unsigned n=0;(state->weapon_phase||state->aim_pose!=AIM_READY)&&n<400;++n)tick(length);
        require(state->weapon_phase==WEAPON_IDLE&&state->aim_pose==AIM_READY&&mounted_draw_owned(possessed),"Mounted reload/redraw got stuck");
        begin_weapon(payload,123);for(unsigned n=0;shots<2&&n<10;++n)tick(length);
        require(shots==2&&field<float>(unit,0xf78)==98,"Second mounted release failed or duplicated");
        for(unsigned n=0;(state->weapon_phase||state->aim_pose!=AIM_READY)&&n<400;++n)tick(length);
        held=false;state->aiming=0;tick(length);
        require(!mounted_draw.actor&&!state->aim_pose&&field<unsigned>(possessed,0x1f0)==29&&shots==2,"RMB cancel did not lower without firing");
        held=true;state->aiming=1;
        require(!mounted_carbine_pose_aim(possessed,true),"Repeated RMB reused a cancelled draw's pose");
        tick(length);++state->lease_generation;
        require(!mounted_carbine_pose_aim(possessed,true),"Retired draw lease retained rifle pose alignment");
        tick(length);
        require(!mounted_draw.actor&&!state->aim_pose&&!state->release&&state->mode==POSSESSED,"Retired draw lease survived or returned RTS");
        marker=0;state->aiming=1;tick(length);require(state->shot_error==98&&!state->aim_pose&&!state->release,"Missing native marker did not fail closed in FPS");
        require(!memcmp(&horse,horse_before,sizeof horse),"Rider hold modified horse state");
        // Exact mounted AI principal can complete its squad gates without an AI
        // projectile. Another horse, wrong unit and stale passenger link cannot.
        uintptr_t passenger=possessed;field<uintptr_t>(possessed,0x210)=reinterpret_cast<uintptr_t>(&horse);
        auto h=reinterpret_cast<uintptr_t>(&horse);field<uintptr_t>(h,0)=base+0x15e8524;field<unsigned char>(h,0xf0)=1;
        field<uintptr_t>(h,0x214)=unit;field<unsigned>(h,0x6cc)=1;field<uintptr_t>(h,0x6d0)=reinterpret_cast<uintptr_t>(&passenger);
        require(ranged_subject_owned(h),"Owned horse AI principal blocks unit");
        Actor task;auto tp=reinterpret_cast<uintptr_t>(&task);field<uintptr_t>(tp,4)=h;
        unsigned draws_before=task_draws,releases_before=task_releases,gates_before=task_gates;
        weapon_draw_hook(&task,nullptr);weapon_release_hook(&task,nullptr);
        require(task_draws==draws_before&&task_releases==releases_before,"Owned horse AI task fired independently of player");
        require(field<unsigned char>(tp,0x84)==1&&field<unsigned char>(tp,0x6c)==1&&field<unsigned char>(tp,0x6d)==1&&field<unsigned char>(tp,0x6e)==1,"Owned horse task completion not acknowledged");
        require(weapon_ready_gate(&task,nullptr)&&weapon_draw_gate(&task,nullptr)&&weapon_task_reload_gate(&task,nullptr)&&weapon_finish_gate(&task,nullptr)&&task_gates==gates_before,"Owned horse still blocks squad readiness");
        passenger=0;
        weapon_draw_hook(&task,nullptr);weapon_release_hook(&task,nullptr);
        require(!weapon_ready_gate(&task,nullptr)&&!weapon_draw_gate(&task,nullptr)&&!weapon_task_reload_gate(&task,nullptr)&&!weapon_finish_gate(&task,nullptr),"Stale horse task gate did not forward native result");
        require(task_draws==draws_before+1&&task_releases==releases_before+1&&task_gates==gates_before+4,"Unowned task did not forward each callback exactly once");
        require(!ranged_subject_owned(h),"Stale passenger AI bypass admitted");passenger=possessed;
        field<uintptr_t>(h,0x214)=unit+4;require(!ranged_subject_owned(h),"Other unit horse AI bypass admitted");field<uintptr_t>(h,0x214)=unit;
        Actor other_horse;memcpy(&other_horse,&horse,sizeof horse);require(!ranged_subject_owned(reinterpret_cast<uintptr_t>(&other_horse)),"Other horse AI bypass admitted");
        horse={};field<uintptr_t>(possessed,0x210)=0;
    }
    mounted=false;
    original_foot_clock();
    stub(0x3d28b0,address(&Actor::choose));
    const wchar_t revolver_key[]=L"boshin_officer_revolver";
    native_record[0]=60;
    native_record[1]=native_record[2]=unsigned(std::wcslen(revolver_key));
    native_record[3]=reinterpret_cast<unsigned>(revolver_key);
    unsigned char aim_asset[0xa0]{},fire_asset[0xa0]{};
    unsigned aim_animation[2]{124,reinterpret_cast<unsigned>(aim_asset)};
    unsigned fire_animation[2]{130,reinterpret_cast<unsigned>(fire_asset)};
    clips[124]=reinterpret_cast<uintptr_t>(aim_animation);
    clips[130]=reinterpret_cast<uintptr_t>(fire_animation);
    auto set_asset=[](unsigned char* a,const wchar_t* path,unsigned frames){
        auto p=reinterpret_cast<uintptr_t>(a);
        field<unsigned>(p,8)=unsigned(std::wcslen(path));
        field<uintptr_t>(p,16)=reinterpret_cast<uintptr_t>(path);
        field<unsigned>(p,0x64)=frames;
    };
    for(bool dual:{false,true}){
        actor={};storage={};foot_revolver={};weapon_actor=0;reload_actor=0;
        lowered=raised=shots=0;held=true;identity_valid=true;
        possessed=reinterpret_cast<uintptr_t>(&actor);
        field<unsigned char>(possessed,0xf0)=1;field<unsigned>(possessed,0x1f0)=14;
        field<uintptr_t>(possessed,0x214)=unit;field<uintptr_t>(possessed,0x23c)=reinterpret_cast<uintptr_t>(db);
        field<float>(unit,0xf78)=100;
        state->mode=POSSESSED;state->aiming=1;state->aim_pose=AIM_READY;state->lease_generation=dual?202:201;
        marker=dual?.12f:.02f;
        const unsigned frames=dual?33:37;
        set_asset(aim_asset,dual?
            L"animations/shogun_animation/battle/men/pistol/pistol_poise/dual_pistol_poise.anim":
            L"animations/shogun_animation/battle/men/pistol/pistol_poise/pistol_poise.anim",dual?43:41);
        set_asset(fire_asset,dual?
            L"animations/shogun_animation/battle/men/pistol/pistol_poise/dual_pistol_fire.anim":
            L"animations/shogun_animation/battle/men/pistol/pistol_poise/pistol_fire.anim",frames);
        actor.play(124,1,0,1.f);
        uint32_t payload[13]{};payload[11]=unsigned(possessed);
        for(unsigned cylinder=0;cylinder<2;++cylinder){
            for(unsigned round=1;round<=6;++round){
                // Model a stale reload mode with the ready pose still on-screen.
                if(cylinder&&round==1)field<unsigned>(possessed,0x1f0)=16;
                begin_weapon(payload,123);
                require(state->weapon_phase==WEAPON_DRAW&&state->aim_pose==AIM_READY&&
                    field<unsigned>(possessed,0x1d4)==130,"Foot revolver re-entered its raise animation");
                unsigned ticks=0,events=event_calls;
                for(;ticks<frames&&state->weapon_phase!=WEAPON_IDLE&&state->weapon_phase!=WEAPON_RELOAD;++ticks)
                    tick(frames);
                const unsigned animated_frames=round==6?frames:frames-2;
                require(ticks==(animated_frames*25+35)/36&&event_calls-events==animated_frames,
                    "Revolver changed rate or crossed the decoded recoil seam");
                const unsigned count=cylinder*6+round;
                require(shots==count&&foot_release_frame==(dual?3u:1u)&&field<float>(unit,0xf78)==100-count,
                    "Revolver missed its original release marker or duplicated ammo consumption");
                require(!lowered&&!raised,"Held ADS replayed a raise/lower transition");
                if(round<6)
                    require(state->weapon_phase==WEAPON_IDLE&&state->aim_pose==AIM_READY&&
                        field<unsigned>(possessed,0x1d4)==124,"Recoil did not return to native poise");
            }
            require(state->weapon_phase==WEAPON_RELOAD&&state->reloads_started==cylinder+1&&
                field<float>(possessed,0x390)>0,"Empty revolver skipped reload");
            for(unsigned n=0;n<100&&state->weapon_phase;++n)tick(frames);
            require(state->weapon_phase==WEAPON_IDLE&&state->aim_pose==AIM_READY&&
                field<unsigned>(possessed,0x1d4)==124&&!lowered&&!raised,"Held ADS dipped after reload");
            auto active=field<uintptr_t>(possessed,0x474);
            unsigned loop_frames=dual?43:41;
            field<unsigned>(possessed,0x1d8)=loop_frames-1;
            original_animation_clock(&actor);
            require(!field<uintptr_t>(possessed,0x474),"Stock poise clock did not expose the old gap");
            field<uintptr_t>(possessed,0x474)=active;
            field<unsigned>(possessed,0x1d8)=loop_frames-1;
            unsigned plays_before=plays;
            for(unsigned i=0;i<2*loop_frames;++i){
                tick(frames);
                require(field<uintptr_t>(possessed,0x474)==active&&field<unsigned>(possessed,0x1d0)==0x21&&
                    plays==plays_before&&!lowered&&!raised,"Held poise briefly cleared or replayed the raise");
            }
        }
        held=false;state->aiming=0;tick(frames);
        require(lowered==1&&state->aim_pose==AIM_LOWERED&&shots==12,"Released ADS did not lower normally");
    }
    std::cout<<"PASS combat68: native clock, trimmed pistol recoil, two cylinders, original shot markers and continuous held poise.\n";
    VirtualFree(reinterpret_cast<void*>(base),0,MEM_RELEASE);
    std::cout<<"PASS: production mounted bow/carbine/pistol native clock pause with per-tick active-animation state refresh (no rewind/root replay), exact horse-task ownership, RMB draw/hold, native clip continuation on LMB, cancellation, no early/duplicate shot, repeated reload/fire, missing-marker and lease guards; horse untouched.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
