#include "qte_service94.h"
#include "../../../common/runtime_storage166.h"
#include "../../../common/component_error168.h"
#include "../../shogun2-possession-cleanup-v46/src/identity.h"
#include "../../shogun2-possession-cleanup-v46/src/game_window105.h"
#include "../../shogun2-possession-cleanup-v46/src/overlay_surface106.h"
#include "playable_protocol.h"
#include "native_observer.h"
#include "possession_observation.h"
#include "native_battle_clock.h"
#include "battle_epoch151.h"
#include "qte_session.h"
#include "qte_presentation93.h"
#include "qte_presented113.h"
#include "encounter_admission.h"
#include "melee_commitment.h"
#include "native_retarget.h"
#include "native_reservation.h"
#include "native_animation_snapshot.h"
#include "qte_render.h"
#include "strict_melee_gate.h"
#include "native_effective_combat.h"
// Kit v12: kill tally comes with qte_native_bridge.h; statistics code built in here.
#ifndef S2UP_LIFE_STATS174
#define S2UP_LIFE_STATS174
// Kill/death statistics shared by the possession engine (deaths), the QTE runtime (kills,
// rounds, battle totals) and the HUD (display). Identical copy in each component (kit v13).
// The single statistics file is replaced in place every time it is written (never grows).
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string>
namespace life173 {
constexpr uint32_t Magic=0x3346494C,Version=174;
struct Shared {
    uint32_t magic,version,size,pid;
    volatile LONG deaths;            // possessed-soldier deaths since the game started (engine)
    volatile LONG last_death_lease;  // possession lease of the latest death (engine)
    volatile LONG sequence;          // QTE writer: odd while the fields below change
    uint32_t in_battle,battle_kills,battle_deaths;      // this battle (QTE)
    uint32_t life_lease,life_kills,life_rounds;         // current possession (QTE)
    uint32_t key_lease,key_actor,key_unit;              // last possessed soldier (overlay key)
    uint64_t tick;                                      // QTE heartbeat
};
inline void mapping_name(wchar_t* out,DWORD pid){swprintf_s(out,96,L"Local\\S2UPLife174-%lu",pid);}
inline bool stats_path(std::wstring& out){
    wchar_t base[32768]{};DWORD n=GetEnvironmentVariableW(L"APPDATA",base,32768);
    if(!n||n>=32768)return false;
    out=std::wstring(base)+L"\\The Creative Assembly\\Shogun2";
    DWORD attr=GetFileAttributesW(out.c_str());
    if(attr==INVALID_FILE_ATTRIBUTES||!(attr&FILE_ATTRIBUTE_DIRECTORY)||(attr&FILE_ATTRIBUTE_REPARSE_POINT))return false;
    out+=L"\\possession_battle_stats.txt";return true;
}
inline bool write_stats(const char* session,unsigned kills,unsigned deaths){
    std::wstring path;if(!stats_path(path))return false;
    char text[160];int n=sprintf_s(text,"S2UP_STATS 1\r\nsession %s\r\nkills %u\r\ndeaths %u\r\n",session,kills,deaths);
    if(n<=0)return false;
    HANDLE f=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;bool ok=WriteFile(f,text,DWORD(n),&written,nullptr)&&written==DWORD(n);
    CloseHandle(f);return ok;
}
}
#endif
#include "../../shogun2-possession-cleanup-v46/vendor/include/MinHook.h"
using namespace qte;
static HMODULE module;
static uint32_t engine;
static Status* state;
static const observation::PossessionPrefix* possession;
static Session* session;
static NativeSelectionPorts selection_ports;
static NativeWitnessCall native_prepare,native_update,native_contact_prepare,native_contact_update;
using NativeAvoidPair=uint32_t(__thiscall*)(void*,void*);
static NativeAvoidPair native_avoid_pair;
static __declspec(thread) uint32_t protected_update;
static volatile LONG contacts_blocked,preplay_avoidance_blocked;
static NativeWitnessRemove native_remove;
static retarget::Ports retarget_ports;
static volatile LONG last_retarget_stage;
static retarget::ReleaseActor native_release_actor;
// The recovered target setter has 13 four-byte stack slots and RET0x34.
// Forward raw words, including float bits and pointers, without conversion.
using NativeTarget=uint32_t(__thiscall*)(void*,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
static NativeTarget native_target;
static effective::Score native_score;
static effective::MitigationPorts native_mitigation;
static effective::Capture combat_capture;
static effective::Capture combat_candidates[16];static unsigned combat_candidate_count;
static uint32_t combat_candidates_witness;static uint64_t combat_candidates_generation;
static strictmelee::Authorization authorization;
static __declspec(thread) uint32_t candidate_witness;
static __declspec(thread) uint64_t candidate_generation;
static __declspec(align(8)) volatile LONG64 presented_serial,presented_tick;
static volatile LONG melee_rejected,native_score_samples;
struct ScoreMetrics {float own{},enemy{};effective::Decomposition detail{};uint32_t player{},opponent{};uint64_t generation{};};
static volatile LONG score_sequence;
static ScoreMetrics score_metrics;
static bool read_score_metrics(ScoreMetrics& out){
    for(unsigned i=0;i<4;++i){LONG sequence=score_sequence;if(sequence&1)continue;MemoryBarrier();
        ScoreMetrics copy=score_metrics;MemoryBarrier();if(sequence==score_sequence){out=copy;return sequence!=0;}}
    return false;
}
static volatile LONG score_capture_rejected,effective_score_rejected;
static SRWLOCK authorization_lock=SRWLOCK_INIT;
static volatile LONG conflicts_blocked,foreign_releases_preserved;
static observation::Registry lifetimes;
static EncounterAdmission admission;
static ClockFrame delivery_clock;
static BattleEpoch151 battle_epoch151;
static volatile LONG delivery_address,suppressed_prompts;
static melee60::Submit submit_commitment;
static melee60::Request commitment;
static NativeActor committed_player,committed_enemy;
static volatile LONG coordinator,active_callbacks,enabled,pending_address,last_environment,player_address;
static __declspec(thread) uint32_t replay_parent;
static __declspec(thread) bool retargeting;
static __declspec(thread) bool selector_missed;
static uint32_t(__cdecl* request_rts)();
static __declspec(thread) bool owns_coordinator;
static SRWLOCK click_lock=SRWLOCK_INIT;
static ClickQueue<128> queued;
struct Removal {uint32_t address;uint64_t generation;};
static Removal removal_queue[128];static unsigned removal_head,removal_tail;
static transport::Publication publication;
static transport::Publication completion_publication;
static SRWLOCK frame_lock93=SRWLOCK_INIT;static Frame93 frame93;
static volatile LONG frames93,click_frames93,stale_frames93,precise_input93,precise_render93,refresh_hz93=60;
static __declspec(align(8)) volatile LONG64 last_frame_us93,max_frame_us93,last_input_us93,max_input_us93;
static uint32_t interrupted_seen93;
static transport::Publication freeze_publication93;static volatile LONG stale_press93;
static __declspec(align(8)) volatile LONG64 clicked_frame_age_us93;
static volatile LONG clicked_frame_inside93;
struct PlaybackWatch {
    uint32_t address{};uint64_t generation{},serial{},revision{};
    double resolved_wall{},prepared_wall{},aligned_wall{},playing_wall{},removed_wall{};
    uint32_t prepared{},aligned{},playing{},native_outcome{},prepare_return{};
    bool prepare_return_valid{};
    observation::AnimationSnapshot before_variant{},after_variant{};
};
static PlaybackWatch playback;
static HWND game,overlay;
static volatile LONG render_stop,render_ready,render_error,render_stage;
static __declspec(align(8)) volatile LONG64 render_heartbeat;
static HANDLE render_initialized;
static double frequency;
static uint64_t acquisitions,resolutions;
static volatile LONG last_result,last_action,last_outcome;
static uint64_t last_serial;
static volatile LONG registration_ready92,registration_waiting92,registration_rejected92,registration_reason92;
static volatile LONG canceled_total92,canceled_reason92;
static __declspec(align(8)) volatile LONG64 canceled_serial92;
static volatile LONG owner_rejected[10],bridge_stages[12],foreign_filtered,clock_rejected,owner_accepted,player_polls;
static storage166::DiscardLog log_file;
static double wall(){LARGE_INTEGER value;QueryPerformanceCounter(&value);return double(value.QuadPart)/frequency;}
static bool read_bytes(void*,uint32_t address,void* out,size_t size){SIZE_T n=0;return
    ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,size,&n)&&n==size;}
static Reader reader(){return {read_bytes,nullptr};}
static bool focused();
static bool possession_snapshot93(observation::PossessionPrefix& p);
static void clear_commitment(){commitment={};if(submit_commitment)submit_commitment(&commitment);}
static bool refresh_committed_pose(const NativeActor& actor,float* position){
    auto r=reader();uint32_t table{},serial{},unit{},id{};
    std::array<uint8_t,0x218> prefix121{};
    if(r.get(actor.address,prefix121)){
        memcpy(&table,prefix121.data(),4);memcpy(&serial,prefix121.data()+0x1c8,4);
        memcpy(&unit,prefix121.data()+0x214,4);
        if(!actor152::kind(engine,table,actor.kind)||serial!=actor.serial||unit!=actor.unit||
           !r.get(unit+4,id)||id!=actor.unit_serial)return false;
        for(unsigned i=0;i<3;++i){
            memcpy(position+i,prefix121.data()+0x48+i*4,4);
            if(!std::isfinite(position[i]))return false;
        }return true;
    }
    if(!r.get(actor.address,table)||!actor152::kind(engine,table,actor.kind)||!r.get(actor.address+0x1c8,serial)||serial!=actor.serial||
       !r.get(actor.address+0x214,unit)||unit!=actor.unit||!r.get(unit+4,id)||id!=actor.unit_serial)return false;
    for(unsigned i=0;i<3;++i)if(!r.get(actor.address+0x48+i*4,position[i])||!std::isfinite(position[i]))return false;
    return true;
}
static void stock_charge_heartbeat140();
static void publish_commitment(){
    stock_charge_heartbeat140();
    if(commitment.phase==melee60::Off)return;
    observation::PossessionPrefix current93{};if(!possession_snapshot93(current93))return;
    if(current93.stop||current93.release||current93.mode!=2||
       current93.lease_generation!=commitment.lease||!focused()||state->stop){clear_commitment();return;}
    if(!refresh_committed_pose(committed_player,commitment.player_position)||!refresh_committed_pose(committed_enemy,commitment.opponent_position)){
        clear_commitment();return;
    }
    commitment.heartbeat=GetTickCount64();if(submit_commitment&&!submit_commitment(&commitment)){++state->faults;state->stop=1;}
}
static bool facing_ready(void*,uint32_t player,uint32_t opponent){
    float p[3]{},e[3]{};uint16_t pb{},eb{};auto r=reader();
    for(unsigned i=0;i<3;++i)if(!r.get(player+0x48+i*4,p[i])||!r.get(opponent+0x48+i*4,e[i])||!std::isfinite(p[i])||!std::isfinite(e[i]))return false;
    if(!r.get(player+0x64,pb)||!r.get(opponent+0x64,eb))return false;
    const auto shown=session->publication(GetCurrentProcessId(),wall()).prompt;
    if(uint64_t(InterlockedCompareExchange64(&presented_serial,0,0))!=shown.serial||
       GetTickCount64()-uint64_t(InterlockedCompareExchange64(&presented_tick,0,0))>100)return false;
    constexpr float tolerance=.261799388f;
    return melee60::angle_error(melee60::facing(p[0],p[2],e[0],e[2]),pb)<=tolerance&&
        melee60::angle_error(melee60::facing(e[0],e[2],p[0],p[2]),eb)<=tolerance;
}
static void sample_playback(uint32_t address){
    if(!owns_coordinator||address!=playback.address||playback.removed_wall||lifetimes.get(address)!=playback.generation)return;
    uint8_t active{},aligned{},playing{};auto r=reader();
    if(!r.get(address+0xc,active)||active>1||!r.get(address+0xd2,aligned)||aligned>1||
       !r.get(address+0xd1,playing)||playing>1)return;
    playback.prepared=active;playback.aligned=aligned;playback.playing=playing;
    auto observed_phase=melee60::playback_phase(commitment.phase,playing!=0);
    if(observed_phase!=commitment.phase){commitment.phase=observed_phase;publish_commitment();}
    double now=wall();
    if(active&&!playback.prepared_wall){playback.prepared_wall=now;++playback.revision;}
    if(aligned&&!playback.aligned_wall){playback.aligned_wall=now;++playback.revision;}
    if(playing&&!playback.playing_wall){playback.playing_wall=now;++playback.revision;admission.started(address,playback.generation);}
}
static bool focused(){DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);return pid==GetCurrentProcessId();}
static bool clock_frame(uint32_t env,ClockFrame& frame){
    return battle_epoch151.sample(reader(),engine,env,wall(),frame);}
static bool routing(void*,uint32_t unit,bool& value){uint32_t root{},env{},common{},phase{},routing_state{};uint8_t on{};
    auto r=reader();if(!r.get(engine+0x1bc8180,root)||!r.get(root+0x110,env)||!r.get(env+8,common)||
       !r.get(common+0xb4,phase)||!r.get(phase+0x2778,on)||!r.get(unit+0xcec,routing_state))return false;
    value=on&&(routing_state==2||routing_state==3);return true;}
// Fresh numeric lease and actor identity proof; camera pointers cached by the
// helper are never enough to exempt a native melee callback.
static bool possession_snapshot93(observation::PossessionPrefix& p){
    if(!possession)return false;
    for(unsigned i=0;i<4;++i){LONG seq=possession->sequence;if(seq&1)continue;
        MemoryBarrier();memcpy(&p,possession,sizeof p);MemoryBarrier();if(seq==possession->sequence)return true;}
    return false;
}
// Callback-local fields from the SAME current actor read as the identity.
// These never survive a call and never substitute for the numeric lease proof.
struct CameraScope121 {bool valid{};uint32_t mount{},backlink{};};
static uint32_t fps_camera_actor(uint64_t* lease=nullptr,CameraScope121* scope121=nullptr){
    if(!possession||!state)return 0;
    observation::PossessionPrefix p{};if(!possession_snapshot93(p))return 0;
    if(p.magic!=0x31504353||p.version!=46||p.pid!=GetCurrentProcessId()||
       !p.ready||p.stop||p.release||p.mode!=2||!p.lease_generation||GetTickCount64()-p.heartbeat>1000)return 0;
    uint32_t root{},env{},manager{},camera{},actor{},table{},serial{},unit{},id{};auto r=reader();uint8_t alive{};
    if(!r.get(engine+0x1bc8180,root)||!r.get(root+0x110,env)||!r.get(env+0x281fc,manager)||
       !r.get(manager+0x8e0,camera)||!r.get(camera+0x188,actor))return 0;
    // One fresh actor prefix replaces four separate reads. Dependent pointers
    // and the unit serial still come from the current camera/lease; nothing
    // is retained across callbacks. Preserve sparse-readable fallback.
    std::array<uint8_t,0x218> prefix119{};
    bool scope_read121=false;
    if(scope121){
        *scope121={};std::array<uint8_t,0x68c> full121{};
        if(r.get(actor,full121)){
            memcpy(prefix119.data(),full121.data(),prefix119.size());
            memcpy(&scope121->mount,full121.data()+0x210,4);
            memcpy(&scope121->backlink,full121.data()+0x688,4);
            scope121->valid=scope_read121=true;
        }
    }
    // Unreadable/guarded tail: retain the exact earlier prefix/field fallback.
    if(scope_read121||r.get(actor,prefix119)){
        memcpy(&table,prefix119.data(),4);memcpy(&serial,prefix119.data()+0x1c8,4);
        memcpy(&unit,prefix119.data()+0x214,4);alive=prefix119[0xf0];
        if(!actor152::human(engine,table)||serial!=p.actor_id||!r.get(unit+4,id)||id!=p.unit_id||alive>1)return 0;
    }else if(!r.get(actor,table)||!actor152::human(engine,table)||
       !r.get(actor+0x1c8,serial)||serial!=p.actor_id||!r.get(actor+0x214,unit)||!r.get(unit+4,id)||id!=p.unit_id||
       !r.get(actor+0xf0,alive)||alive>1)return 0;
    if(lease)*lease=p.lease_generation;return actor;
}
static uint32_t fps_actor(uint64_t* lease=nullptr){
    uint64_t generation{};auto camera=fps_camera_actor(&generation);NativeActor human,subject;auto r=reader();
    if(!camera||!combat_identity(r,camera,engine,human,false)||!physical_actor(r,engine,human,subject,false))return 0;
    if(lease)*lease=generation;return subject.address;
}
static uint32_t owned_combat_actor(uint32_t witness,uint64_t* lease=nullptr){
    auto physical=fps_actor(lease);
    if(physical&&strictmelee::includes(reader(),physical,witness))return physical;
    return 0;
}
static bool scoped(uint32_t witness){
    if(!witness||witness>UINT32_MAX-0x100)return false;
    CameraScope121 scope;auto camera=fps_camera_actor(nullptr,&scope);if(!camera)return false;
    auto r=reader();
    if(scope.valid){
        if(scope.backlink==witness)return true;
        std::array<uint32_t,20> participants{};
        if(r.get(witness,participants)){
            auto contains=[&](uint32_t actor){
                for(auto offsets:{std::pair<unsigned,unsigned>{0x10,0x24},{0x30,0x38},{0x3c,0x48}}){
                    uint32_t count=participants[offsets.second/4];if(count>5)continue;
                    for(uint32_t i=0;i<count;++i)if(participants[offsets.first/4+i]==actor)return true;
                }return false;
            };
            if(contains(camera))return true;
            if(!scope.mount)return false;
            uint32_t backlink{};
            if(!(r.get(scope.mount+0x688,backlink)&&backlink==witness)&&!contains(scope.mount))return false;
            // A possible mounted alias still needs the original fresh,
            // reciprocal ownership proof; a participant pointer cannot grant it.
            return owned_combat_actor(witness)!=0;
        }
    }
    // Preserve readable individual groups when either large snapshot fails.
    if(strictmelee::includes(r,camera,witness))return true;
    uint32_t mount{};if(!r.get(camera+0x210,mount)||!mount)return false;
    if(!strictmelee::includes(r,mount,witness))return false;
    return owned_combat_actor(witness)!=0;
}
static bool approved(uint32_t address,uint64_t generation){uint64_t lease{};if(!fps_actor(&lease))return false;
    AcquireSRWLockShared(&authorization_lock);auto copy=authorization;ReleaseSRWLockShared(&authorization_lock);
    if(!copy.matches(address,generation,lease)||!copy.sealed)return false;
    NativeVerdict verdict{};uint32_t matched{},fallback{};auto verify=reader();
    if(!verify.get(address+0x80,verdict)||memcmp(&verdict,&copy.verdict,sizeof verdict)||
       !verify.get(address+0x98,matched)||matched!=copy.matched||!verify.get(address+0xb4,fallback)||fallback!=copy.fallback)return false;
    auto r=reader();for(auto identity:{copy.player,copy.enemy}){uint32_t table{},id{},unit{},serial{},link{};uint8_t alive{};
        if(!r.get(identity.address,table)||!actor152::kind(engine,table,identity.kind)||!r.get(identity.address+0x1c8,serial)||serial!=identity.serial||
           !r.get(identity.address+0x214,unit)||unit!=identity.unit||!r.get(unit+4,id)||id!=identity.unit_serial||
           !r.get(identity.address+0xf0,alive)||alive>1||!r.get(identity.address+0x688,link)||
           (link!=address&&(alive||link)))return false;
    }return true;}
#include "native_charge_contact140.h"
static void retire_authorization(uint32_t address,uint64_t generation){AcquireSRWLockExclusive(&authorization_lock);authorization.removed(address,generation);ReleaseSRWLockExclusive(&authorization_lock);}
static bool authorize(const WitnessKey& key,uint64_t serial,Result result,const NativeActor& p,const NativeActor& e){
    AcquireSRWLockExclusive(&authorization_lock);bool ok=authorization.permit(key,serial,result,p,e);ReleaseSRWLockExclusive(&authorization_lock);return ok;}
static bool seal_authorization(uint32_t address,uint64_t generation,const NativeVerdict& expected){
    NativeVerdict verdict{};uint32_t matched{},fallback{};auto r=reader();
    if(!r.get(address+0x80,verdict)||memcmp(&verdict,&expected,sizeof verdict)||!r.get(address+0x98,matched)||
       !r.get(address+0xb4,fallback)||(!matched&&!fallback))return false;
    AcquireSRWLockExclusive(&authorization_lock);
    bool ok=authorization.key.address==address&&authorization.key.generation==generation&&authorization.serial;
    if(ok){authorization.verdict=verdict;authorization.matched=matched;authorization.fallback=fallback;authorization.sealed=true;}
    ReleaseSRWLockExclusive(&authorization_lock);return ok;
}
static uint32_t reject_melee(){InterlockedIncrement(&melee_rejected);return 1;}
static bool write_local(void*,uint32_t address,const void* value,size_t size){SIZE_T n{};return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),value,size,&n)&&n==size;}
static bool retain_selected_pair(void*,uint32_t witness,const NativeActor& player,const NativeActor& enemy){
    return strictmelee::retain_selected(reader(),write_local,nullptr,engine,witness,player,enemy);
}
static void candidate_scope(uint32_t address,uint64_t generation){
    // Called under the coordinator; native worker TLS alone is not a lifetime.
    if(combat_candidates_witness!=address||combat_candidates_generation!=generation){
        combat_candidate_count=0;combat_capture={};combat_candidates_witness=address;combat_candidates_generation=generation;
    }
    candidate_witness=address;candidate_generation=generation;
}
static bool effective_pair(void*,uint32_t player,uint32_t enemy,float& own,float& other){
    for(unsigned i=0;i<combat_candidate_count;++i){const auto& value=combat_candidates[i];
        if(value.witness==candidate_witness&&value.generation==candidate_generation&&
           ((value.a.actor==player&&value.b.actor==enemy)||(value.a.actor==enemy&&value.b.actor==player))){combat_capture=value;break;}}
    if(!owns_coordinator||!combat_capture.valid||combat_capture.witness!=candidate_witness||
       combat_capture.generation!=candidate_generation){InterlockedIncrement(&effective_score_rejected);return false;}
    __try{effective::Decomposition detail;bool ok=effective::powers(combat_capture,native_score,native_mitigation,player,enemy,own,other,&detail);
        if(ok){InterlockedIncrement(&score_sequence);MemoryBarrier();score_metrics={own,other,detail,player,enemy,candidate_generation};
            MemoryBarrier();InterlockedIncrement(&score_sequence);InterlockedIncrement(&native_score_samples);}
        else InterlockedIncrement(&effective_score_rejected);return ok;}
    __except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&effective_score_rejected);return false;}
}
static int __fastcall score_hook(void* self,void*,void* a,void* b,void* pair,uint32_t debug){
    InterlockedIncrement(&active_callbacks);int answer{};
    __try{
        answer=native_score(self,a,b,pair,debug);
        if(enabled&&owns_coordinator&&candidate_witness&&scoped(candidate_witness)){
            effective::Capture value;
            if(effective::capture(reader(),engine,candidate_witness,candidate_generation,uint32_t(self),uint32_t(a),uint32_t(b),uint32_t(pair),value)&&
               effective::belongs(reader(),engine,value)&&
               (value.a.actor==owned_combat_actor(candidate_witness)||value.b.actor==owned_combat_actor(candidate_witness))){
                combat_capture=value;unsigned index=0;
                while(index<combat_candidate_count&&!(combat_candidates[index].a.actor==value.a.actor&&combat_candidates[index].b.actor==value.b.actor))++index;
                if(index<16){combat_candidates[index]=value;if(index==combat_candidate_count)++combat_candidate_count;}
            }
            else InterlockedIncrement(&score_capture_rejected);
        }
    }__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static Owner owner(uint32_t env,uint32_t witness){Owner result;ClockFrame clock;
    if(!clock_frame(env,clock)){InterlockedIncrement(owner_rejected+1);return result;}
    result.battle=clock.battle;result.environment=env;
    if(!possession||state->stop){InterlockedIncrement(owner_rejected+2);return result;}
    observation::PossessionPrefix p{};
    if(!possession_snapshot93(p)){result.observation_busy93=true;InterlockedIncrement(owner_rejected+3);return result;}
    if(p.magic!=0x31504353||p.version!=46||p.pid!=GetCurrentProcessId()){
        InterlockedIncrement(owner_rejected+4);return result;}
    if(!p.ready||p.stop||p.release||p.mode!=2||!p.lease_generation||GetTickCount64()-p.heartbeat>1000){
        InterlockedIncrement(owner_rejected+5);return result;}
    auto r=reader();uint32_t actor=owned_combat_actor(witness),mount{};NativeActor identity;
    if(!actor||
       !actor_identity(r,actor,witness,engine,identity,true)){
        InterlockedIncrement(owner_rejected+6);return result;}
    uint64_t camera_lease{};auto camera_actor=fps_camera_actor(&camera_lease);uint32_t camera_serial{};
    if(!camera_actor||!r.get(camera_actor+0x1c8,camera_serial)||camera_serial!=p.actor_id||
       camera_lease!=p.lease_generation||identity.unit_serial!=p.unit_id){InterlockedIncrement(owner_rejected+7);return result;}
    if(!r.get(actor+0x21c,mount)||mount){InterlockedIncrement(owner_rejected+8);return result;}
    // Ranged infantry also use this gate when they engage in melee.
    InterlockedIncrement(&owner_accepted);
    result.lease=p.lease_generation;result.actor=identity;result.verified_melee=true;return result;
}
static uint32_t prepare_port(void*,uint32_t address){
    if(scoped(address)&&!approved(address,lifetimes.get(address))&&
       !(owns_coordinator&&session&&session->occupied()&&candidate_witness==address))return reject_melee();
    uint32_t before=replay_parent;bool missed_before=selector_missed;selector_missed=false;replay_parent=address;
    uint32_t answer=0;
    if(commitment.phase!=melee60::Off&&admission.matches(address,lifetimes.get(address))){
        const auto resolved=session->publication(GetCurrentProcessId(),wall()).prompt;
        auto phase=melee60::preparation_phase(commitment.phase,resolved.result==Result::Canceled,
            session->occupied()&&resolved.serial==commitment.serial);
        if(phase==melee60::Off)clear_commitment();
        else {commitment.phase=phase;publish_commitment();}
    }
    __try{answer=native_prepare(reinterpret_cast<void*>(address));sample_playback(address);
        if(owns_coordinator&&address==playback.address&&!playback.prepare_return_valid&&lifetimes.get(address)==playback.generation){
            playback.prepare_return=answer;playback.prepare_return_valid=true;++playback.revision;}}
    __finally{if(selector_missed&&scoped(address))answer=reject_melee();selector_missed=missed_before;replay_parent=before;}return answer;
}
static uint32_t update_port(void*,uint32_t address){
    if(scoped(address)&&!approved(address,lifetimes.get(address)))return reject_melee();
    auto before=protected_update;protected_update=address;uint32_t answer{};
    __try{answer=native_update(reinterpret_cast<void*>(address));sample_playback(address);}
    __finally{protected_update=before;}return answer;}
static uint32_t retarget_port(void*,uint32_t address,const NativeVerdict& desired,const NativePlan* plans){
    const double now=wall();auto completed=session->publication(GetCurrentProcessId(),now);
    const auto& resolved=completed.prompt;const auto key=session->key();
    NativeActor p{},e{};
    if(!session->participants(p,e)||uint64_t(InterlockedCompareExchange64(&presented_serial,0,0))!=resolved.serial||
       !authorize(key,resolved.serial,resolved.result,p,e))return reject_melee();
    const auto revision=playback.revision+1;
    playback={};playback.address=address;playback.generation=key.generation;playback.serial=resolved.serial;
    playback.revision=revision;playback.resolved_wall=now;playback.native_outcome=desired.outcome;
    last_serial=resolved.serial;last_result=LONG(resolved.result);last_action=LONG(resolved.action);last_outcome=LONG(desired.outcome);
    completed.prompt.active=true;completed.occupied=1;transport::publish(completion_publication,completed);
    observation::animation_snapshot(reader(),address,playback.before_variant);
    uint32_t prior=replay_parent;bool prior_retarget=retargeting;replay_parent=address;retargeting=true;uint32_t answer=1,stage=0;
    // Role-swapped results (Perfect Guard, Countered) use the same role-flexible rebuild as routing.
    __try{answer=retarget::rebuild(address,desired,plans,retarget_ports,stage,true);
        if((answer&0xff)==0&&!seal_authorization(address,key.generation,desired)){answer=reject_melee();retire_authorization(address,key.generation);}
        if((answer&0xff)==0){commitment.phase=melee60::Aligning;publish_commitment();}else clear_commitment();
        sample_playback(address);
        observation::animation_snapshot(reader(),address,playback.after_variant);
        playback.prepare_return=answer;playback.prepare_return_valid=true;++playback.revision;last_retarget_stage=LONG(stage);}
    __finally{replay_parent=prior;retargeting=prior_retarget;}
    return answer;
}
static void drain_input(){AcquireSRWLockExclusive(&click_lock);Click value;
    if(queued.overflowed()){session->release(CancelReason92::InputOverflow);queued.clear();InterlockedIncrement(&state->dropped);}
    while(queued.pop(value))session->enqueue(value);ReleaseSRWLockExclusive(&click_lock);
    session->levels((GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0,(GetAsyncKeyState(VK_RBUTTON)&0x8000)!=0);
}
static void drain_removals(){AcquireSRWLockExclusive(&click_lock);
    while(removal_tail!=removal_head){const auto& value=removal_queue[removal_tail++%128];stock_charge_removed140(value.address,value.generation);session->removed(value.address,value.generation);
        if(admission.removed(value.address,value.generation,delivery_clock)){InterlockedExchange(&delivery_address,0);clear_commitment();}
        if(value.address==playback.address&&value.generation==playback.generation&&!playback.removed_wall){playback.removed_wall=wall();++playback.revision;}}
    ReleaseSRWLockExclusive(&click_lock);}
static void publish(){auto data=session->publication(GetCurrentProcessId(),wall());
    auto canceled=session->cancellation_stats92();InterlockedExchange(&canceled_total92,LONG(canceled.total));InterlockedExchange(&canceled_reason92,LONG(canceled.reason));InterlockedExchange64(&canceled_serial92,LONG64(canceled.serial));
    auto reg=session->registration_stats92();InterlockedExchange(&registration_ready92,LONG(reg.ready));InterlockedExchange(&registration_waiting92,LONG(reg.waiting));InterlockedExchange(&registration_rejected92,LONG(reg.rejected));InterlockedExchange(&registration_reason92,LONG(reg.reason_mask));
    if(canceled.total!=interrupted_seen93){
        interrupted_seen93=canceled.total;auto feedback=session->interruption_publication93(GetCurrentProcessId(),wall());
        feedback.occupied=1;feedback.enabled=1;feedback.published_wall=wall();
        if(feedback.prompt.serial)transport::publish(completion_publication,feedback);
    }
    publish_commitment();
    transport::publish(publication,data);InterlockedExchange(&pending_address,LONG(session->key().address));
    state->mode=data.occupied?1:0;if(data.error){state->faults=LONG(data.error);InterlockedExchange(&state->stop,1);}}
// A selected routing encounter uses the retained native pair lifecycle and
// adapter alignment channel. No visible prompt or QTE admission is created.
static volatile LONG routing_prepares152,routing_transitions152;
static uint64_t routed_generation152(uint32_t address){
    return candidate_witness==address&&lifetimes.matches140(address,candidate_generation)?candidate_generation:0;
}

static uint32_t routed_prepare152(void*,uint32_t address,const NativeActor& player,const NativeActor& enemy,const NativeVerdict& desired,const NativePlan* plans){
    uint64_t lease{};auto physical=owned_combat_actor(address,&lease);auto r=reader();NativeActor current,opponent;uint32_t table{},env{};
    auto generation=routed_generation152(address);
    if(!owns_coordinator||!generation||!lease||physical!=player.address||
       !combat_identity(r,physical,engine,current)||!same_actor(current,player)||
       !combat_identity(r,enemy.address,engine,opponent)||!same_actor(opponent,enemy)||player.side==enemy.side||player.unit==enemy.unit||
       !r.get(address,table)||!r.get(address+4,env)||!env)return 1;
    if(plans){admission.removed(address,generation,delivery_clock);InterlockedExchange(&delivery_address,0);retire_authorization(address,generation);}
    stock_charge_clear140();stock_charge140={address,table,env,generation,lease,generation|(uint64_t(1)<<63),player,false};
    if(!stock_charge_current140(address,generation)){stock_charge_clear140();return 1;}
    if(plans)InterlockedIncrement(&routing_transitions152);else InterlockedIncrement(&routing_prepares152);
    auto prior_scope=stock_charge_scope140,prior_parent=replay_parent;stock_charge_scope140=address;replay_parent=address;uint32_t answer=1;bool completed=false;
    __try{if(plans){uint32_t stage{};answer=retarget::rebuild(address,desired,plans,retarget_ports,stage,true);}else answer=native_prepare(reinterpret_cast<void*>(address));if(answer&0xff)stock_charge_clear140();else{stock_charge_heartbeat140();if(commitment.serial==stock_charge140.serial)publish_commitment();}completed=true;}
    __finally{stock_charge_scope140=prior_scope;replay_parent=prior_parent;if(!completed)stock_charge_clear140();}
    return answer;
}
static uint32_t controlled(uint32_t address,uint64_t generation,bool updating){
    drain_removals();
    const bool owned=scoped(address);
    if(owned&&approved(address,generation))return updating?update_port(nullptr,address):prepare_port(nullptr,address);
    uint32_t stock_answer140{};
    if(owned&&stock_charge_dispatch140(address,generation,updating,stock_answer140))return stock_answer140;
    if(owned&&session->occupied()&&address!=session->key().address)return reject_melee();
    if(session->occupied()&&address!=session->key().address)
        return updating?update_port(nullptr,address):prepare_port(nullptr,address);
    // Other encounters must neither advance nor cancel the player's session.
    // The worker's pointer is only a filter; owner() proves it afresh below.
    uint32_t player=uint32_t(player_address);
    if(!owned&&address!=uint32_t(pending_address)&&!player_encounter(reader(),player,address,engine,updating)){
        InterlockedIncrement(&foreign_filtered);
        return updating?update_port(nullptr,address):prepare_port(nullptr,address);
    }
    uint32_t env=0;reader().get(address+4,env);ClockFrame clock;
    if(!clock_frame(env,clock)){
        InterlockedIncrement(&clock_rejected);
        if(owned||address==uint32_t(pending_address))return 0;
        return updating?update_port(nullptr,address):prepare_port(nullptr,address);
    }
    InterlockedExchange(&last_environment,LONG(env));delivery_clock=clock;drain_input();
    const auto before=session->publication(GetCurrentProcessId(),wall());
    if(state->stop)session->release();
    Owner current=owner(env,address);
    if(owned&&(!focused()||clock.step<=0||clock.speed<=0))return 0;
    if(current.observation_busy93&&address==session->key().address)return 0;
    if(owned&&!current.verified_melee)return reject_melee();
    if(owned&&!session->occupied()){
        if(updating)return reject_melee(); // Stock encounter already active at FPS entry: native cleanup, never stock replay.
    }
    // Native witnesses migrate between workers. The bounded captured contexts
    // belong to this coordinated generation, not to a worker's TLS history.
    candidate_scope(address,generation);
    const bool admit=admission.allowed(current.battle,current.lease,clock.tick);
    if(!admit&&!updating&&!session->occupied()&&current.verified_melee)InterlockedIncrement(&suppressed_prompts);
    uint32_t answer;
    if(updating&&address==session->key().address&&generation==session->key().generation){
        // The held native update cannot run until preparation has replayed.
        answer=session->prepare(address,generation,current,clock,focused(),admit);
        if((answer&0xff)==0)answer=session->update(address,generation,current,clock,focused());
    }else answer=updating?session->update(address,generation,current,clock,focused()):
                          session->prepare(address,generation,current,clock,focused(),admit);
    const auto after=session->publication(GetCurrentProcessId(),wall());
    if(!updating&&session->acquisition_stage()<12)InterlockedIncrement(bridge_stages+session->acquisition_stage());
    if(after.prompt.active&&after.prompt.serial!=before.prompt.serial){++acquisitions;
        if(!admission.acquire(session->key(),clock.tick)){session->release();++state->faults;state->stop=1;}
        if(session->participants(committed_player,committed_enemy)){
            commitment={};commitment.pid=GetCurrentProcessId();commitment.phase=melee60::Facing;commitment.lease=uint32_t(current.lease);
            commitment.serial=after.prompt.serial;commitment.player=committed_player.serial;commitment.player_unit=committed_player.unit_serial;
            commitment.opponent=committed_enemy.serial;commitment.opponent_unit=committed_enemy.unit_serial;
            commitment.player_kind=committed_player.kind;commitment.opponent_kind=committed_enemy.kind;
            if(!refresh_committed_pose(committed_player,commitment.player_anchor)||!refresh_committed_pose(committed_enemy,commitment.opponent_anchor)){session->release();clear_commitment();}
            else publish_commitment();
        }
        // Publish the reservation only after its immutable actor identities.
        InterlockedExchange(&delivery_address,LONG(admission.address()));
    }
    if(before.occupied&&!after.occupied)++resolutions;
    drain_removals();publish();return answer;
}
static uint32_t contact_dispatch(uint32_t address,bool updating){
    if(enabled&&reservation::overlaps(reader(),engine,uint32_t(InterlockedCompareExchange(&delivery_address,0,0)),address,committed_player,committed_enemy)){
        InterlockedIncrement(&contacts_blocked);return 1; // Native removes this foreign contact, not the owned pair.
    }
    return updating?native_contact_update(reinterpret_cast<void*>(address)):native_contact_prepare(reinterpret_cast<void*>(address));
}
static uint32_t __fastcall contact_prepare_hook(void* self,void*){
    InterlockedIncrement(&active_callbacks);uint32_t answer{};
    __try{answer=contact_dispatch(uint32_t(self),false);}__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static uint32_t __fastcall contact_update_hook(void* self,void*){
    InterlockedIncrement(&active_callbacks);uint32_t answer{};
    __try{answer=contact_dispatch(uint32_t(self),true);}__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static uint32_t __fastcall avoid_pair_hook(void* self,void*,void* other){
    InterlockedIncrement(&active_callbacks);uint32_t answer{};
    __try{
        auto held=uint32_t(InterlockedCompareExchange(&delivery_address,0,0));auto one=uint32_t(self),two=uint32_t(other);
        uint8_t playing{};
        if(enabled&&held&&held==protected_update&&reader().get(held+0xd1,playing)&&!playing&&
           ((one==committed_player.address&&two==committed_enemy.address)||(two==committed_player.address&&one==committed_enemy.address))&&
           reservation::owns(reader(),engine,held,committed_player)&&reservation::owns(reader(),engine,held,committed_enemy)){
            // Native locomotion avoidance cannot cancel an already committed pair.
            // Life/identity checks still allow projectile death/dismount teardown.
            InterlockedIncrement(&preplay_avoidance_blocked);return 0;
        }
        answer=native_avoid_pair(self,other);
    }__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static uint32_t dispatch_current(uint32_t address,uint64_t generation,bool updating){
    if(!enabled)return updating?native_update(reinterpret_cast<void*>(address)):native_prepare(reinterpret_cast<void*>(address));
    if(reservation::overlaps(reader(),engine,uint32_t(InterlockedCompareExchange(&delivery_address,0,0)),address,committed_player,committed_enemy)){
        InterlockedIncrement(&conflicts_blocked);return 1;
    }
    if(InterlockedCompareExchange(&coordinator,1,0)){
        // Never wait inside an engine callback. A different pair forwards once.
        if(address==uint32_t(pending_address)||scoped(address))return 0;
        return updating?native_update(reinterpret_cast<void*>(address)):native_prepare(reinterpret_cast<void*>(address));
    }
    owns_coordinator=true;uint32_t answer=0;
    __try{answer=controlled(address,generation,updating);}
    __finally{owns_coordinator=false;InterlockedExchange(&coordinator,0);}return answer;
}
static uint32_t dispatch(uint32_t address,uint64_t generation,bool updating){
    // A worker may service hundreds of unrelated witnesses between our pair's
    // frames. Its old TLS candidate is not the current native score caller.
    const auto previous_witness=candidate_witness;const auto previous_generation=candidate_generation;
    candidate_witness=0;candidate_generation=0;uint32_t answer{};
    __try{answer=dispatch_current(address,generation,updating);}
    __finally{candidate_witness=previous_witness;candidate_generation=previous_generation;}
    return answer;
}
static uint32_t __fastcall prepare_hook(void* self,void*){InterlockedIncrement(&active_callbacks);InterlockedIncrement(state->calls);
    uint32_t address=uint32_t(self),answer=0;auto caller=uintptr_t(_ReturnAddress());
    auto generation=lifetimes.get(address,caller==engine+0x3e210e);
    __try{answer=dispatch(address,generation,false);}
    __finally{InterlockedDecrement(&active_callbacks);}return answer;}
static uint32_t __fastcall update_hook(void* self,void*){InterlockedIncrement(&active_callbacks);InterlockedIncrement(state->calls+1);
    uint32_t address=uint32_t(self),answer=0;auto generation=lifetimes.get(address);
    __try{answer=dispatch(address,generation,true);}
    __finally{InterlockedDecrement(&active_callbacks);}return answer;}
static void __fastcall remove_hook(void* self,void*,void* witness){InterlockedIncrement(&active_callbacks);InterlockedIncrement(state->calls+2);
    uint32_t address=uint32_t(witness);auto generation=lifetimes.get(address);
    retire_authorization(address,generation);
    if(enabled&&!InterlockedCompareExchange(&coordinator,1,0)){
        drain_removals();stock_charge_removed140(address,generation);session->removed(address,generation);
        if(admission.removed(address,generation,delivery_clock)){InterlockedExchange(&delivery_address,0);clear_commitment();}
        if(address==playback.address&&generation==playback.generation&&!playback.removed_wall){playback.removed_wall=wall();++playback.revision;}
        publish();InterlockedExchange(&coordinator,0);
    }else if(enabled&&(address==uint32_t(pending_address)||address==uint32_t(delivery_address))){
        AcquireSRWLockExclusive(&click_lock);
        if(removal_head-removal_tail<128)removal_queue[removal_head++%128]={address,generation};
        else {InterlockedIncrement(&state->faults);state->stop=1;}
        ReleaseSRWLockExclusive(&click_lock);
    }
    __try{native_remove(self,witness);}
    __finally{lifetimes.removed(address,generation);InterlockedDecrement(&active_callbacks);}}
static void __fastcall release_actor_hook(void* witness,void*,void* actor){
    InterlockedIncrement(&active_callbacks);
    __try{
        auto held=uint32_t(InterlockedCompareExchange(&delivery_address,0,0));
        if(enabled&&reservation::preserve_release(reader(),engine,held,uint32_t(witness),uint32_t(actor),committed_player,committed_enemy)){
            InterlockedIncrement(&foreign_releases_preserved);return;
        }
        native_release_actor(witness,actor);
    }__finally{InterlockedDecrement(&active_callbacks);}
}
static uint32_t __fastcall target_hook(void* witness,void*,uint32_t actor,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g,uint32_t h,uint32_t i,uint32_t j,uint32_t k,uint32_t l,uint32_t m){
    InterlockedIncrement(&active_callbacks);uint32_t answer=0;
    __try{
        auto held=uint32_t(InterlockedCompareExchange(&delivery_address,0,0));
        if(enabled&&reservation::preserve_release(reader(),engine,held,uint32_t(witness),actor,committed_player,committed_enemy)){
            InterlockedIncrement(&conflicts_blocked);return 0;
        }
        if(enabled&&scoped(uint32_t(witness))&&!stock_charge_target_scope140(uint32_t(witness))&&!approved(uint32_t(witness),lifetimes.get(uint32_t(witness)))&&
           !(owns_coordinator&&replay_parent==uint32_t(witness)&&session&&session->occupied())){
            InterlockedIncrement(&melee_rejected);return 0;
        }
        answer=native_target(witness,actor,b,c,d,e,f,g,h,i,j,k,l,m);
    }__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static bool selector_contains_proved122(void*const* begin,void*const* end){
    bool contains=false;uint32_t player=fps_actor(),camera=fps_camera_actor();
    uintptr_t start=uintptr_t(begin),finish=uintptr_t(end);
    if(player&&finish>=start&&finish-start<=20&&((finish-start)&3)==0){
        for(uintptr_t slot=start;slot<finish;slot+=4){uint32_t actor{};if(!reader().get(uint32_t(slot),actor))break;contains|=actor==player||actor==camera;}
    }return contains;
}
static bool selector_contains122(void*const* begin,void*const* end){
    uintptr_t start=uintptr_t(begin),finish=uintptr_t(end);
    if(finish<=start||finish-start>20||((finish-start)&3))return false;
    CameraScope121 scope;auto camera=fps_camera_actor(nullptr,&scope);if(!camera)return false;
    uint32_t mount=scope.mount;
    if(!scope.valid&&!reader().get(camera+0x210,mount))return selector_contains_proved122(begin,end);
    std::array<uint32_t,5> actors{};auto r=reader();unsigned count=unsigned((finish-start)/4);
    bool snapshot=start>=0x10000&&finish<=UINT32_MAX&&r.read&&r.read(r.context,uint32_t(start),actors.data(),count*4);
    bool possible=false;
    for(unsigned i=0;i<count;++i){uint32_t actor{};
        if(snapshot)actor=actors[i];else if(!r.get(uint32_t(start+i*4),actor))break;
        possible|=actor==camera||(mount&&actor==mount);
    }
    // A possible match re-reads the original physical identity and selector
    // participants. Neither stale snapshot values nor failed bulk reads grant it.
    return possible&&selector_contains_proved122(begin,end);
}
static NativeVerdict* __fastcall select_hook(void* self,void*,NativeVerdict* output,
    void*const* begin,void*const* end,uint32_t flags,uint32_t flags2){
    InterlockedIncrement(&active_callbacks);InterlockedIncrement(state->calls+3);NativeVerdict* answer=nullptr;
    __try{
        NativeVerdict verdict;
        if(owns_coordinator&&replay_parent&&output&&session->try_replay(replay_parent,
           uint32_t(uintptr_t(_ReturnAddress())-engine),uint32_t(self),uint32_t(begin),uint32_t(end),flags,flags2,verdict)){
            const double now=wall();auto completed=session->publication(GetCurrentProcessId(),now);
            const auto& resolved=completed.prompt;
            // Special stock verdict replays are not QTE answers and must not
            // overwrite the last defence/attack feedback or timing record.
            if(!retargeting&&session->occupied()&&resolved.serial&&resolved.result!=Result::Pending&&resolved.result!=Result::Canceled){
                last_serial=resolved.serial;last_result=LONG(resolved.result);last_action=LONG(resolved.action);last_outcome=LONG(verdict.outcome);
                const auto key=session->key();const auto revision=playback.revision+1;
                playback={};playback.address=key.address;playback.generation=key.generation;playback.serial=resolved.serial;
                playback.revision=revision;playback.resolved_wall=now;playback.native_outcome=verdict.outcome;
                completed.prompt.active=true;completed.occupied=1;transport::publish(completion_publication,completed);
            }
            *output=verdict;answer=output;
        }else {
            bool contains=false;
            // Without our replay scope this hook always forwards the stock
            // verdict. Do not prove player identity merely to discard it.
            if(enabled&&owns_coordinator&&replay_parent){
                contains=selector_contains122(begin,end);
            }
            answer=selection_ports.select(self,output,begin,end,flags,flags2);
            if(answer&&enabled&&owns_coordinator&&stock_charge_scope140&&stock_charge_target_scope140(stock_charge_scope140)){
                auto r=reader();auto player=owned_combat_actor(stock_charge_scope140);NativeActor p,e;bool own_route=false,enemy_route=false;
                uint32_t other=answer->attacker_address==player?answer->defender_address:answer->defender_address==player?answer->attacker_address:0;
                if(player&&other&&combat_identity(r,player,engine,p)&&combat_identity(r,other,engine,e)&&p.side!=e.side&&p.unit!=e.unit&&routing(nullptr,p.unit,own_route)&&routing(nullptr,e.unit,enemy_route)&&(own_route||enemy_route)){
                    answer->outcome=5;answer->attacker_address=own_route?e.address:p.address;answer->defender_address=own_route?p.address:e.address;
                }
            }
            // Never return null role pointers to native prepare: it immediately
            // dereferences both. A cache miss is a surviving registration that
            // its prepare wrapper rejects before any native clip update.
            if(enabled&&contains&&owns_coordinator&&replay_parent){
                if(answer)answer->outcome=0;selector_missed=true;InterlockedIncrement(&melee_rejected);
            }
        }
    }__finally{InterlockedDecrement(&active_callbacks);}return answer;
}
static volatile LONG display_reset105;
static volatile LONG moving_frames105,repeated_frames105;
static __declspec(align(8)) volatile LONG64 max_marker_hold_us105;
static LRESULT CALLBACK overlay_proc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==WM_NCHITTEST)return HTTRANSPARENT;if(msg==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
    if(msg==WM_DISPLAYCHANGE||msg==WM_DPICHANGED){InterlockedExchange(&display_reset105,1);return 0;}
    return DefWindowProcW(w,msg,wp,lp);}
struct Surface93 {
    HDC screen{},dc{};HBITMAP bitmap{};HGDIOBJ old{};void* pixels{};int size{};
    void clear(){if(dc&&old)SelectObject(dc,old);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);if(screen)ReleaseDC(nullptr,screen);screen=dc=nullptr;bitmap=nullptr;old=nullptr;pixels=nullptr;size=0;}
    bool ensure(int n){if(size==n&&pixels)return true;clear();screen=GetDC(nullptr);if(!screen)return false;dc=CreateCompatibleDC(screen);if(!dc){clear();return false;}
        BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=n;info.bmiHeader.biHeight=-n;
        info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
        bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&pixels,nullptr,0);if(!bitmap||!pixels){clear();return false;}
        old=SelectObject(dc,bitmap);if(!old||old==HGDI_ERROR){clear();return false;}size=n;return true;}
};
static Surface93 surface93;
static overlay106::Publisher pixels106;
static PresentedImages113 images113;
static Snapshot drawn_prompt106;static double drawn_wall106;
static volatile LONG unique_frames107,moving_frames107,repeated_frames107;
static __declspec(align(8)) volatile LONG64 max_marker_hold_us107;
static double last_submission107=-1,marker_elapsed107,marker_changed107;static uint64_t marker_serial107;
static void record_presented107(const Snapshot& shown,double submitted,bool click_freeze){
    if(!std::isfinite(submitted)||submitted<=last_submission107)return;
    last_submission107=submitted;InterlockedIncrement(&unique_frames107);
    if(shown.result!=Result::Pending||shown.paused||click_freeze){marker_serial107=0;return;}
    if(marker_serial107!=shown.serial||std::abs(marker_elapsed107-shown.response_elapsed)>1e-6){
        InterlockedIncrement(&moving_frames107);marker_changed107=submitted;
    }else{
        InterlockedIncrement(&repeated_frames107);LONG64 held=LONG64(std::max(0.,submitted-marker_changed107)*1e6);
        if(held>max_marker_hold_us107)InterlockedExchange64(&max_marker_hold_us107,held);
    }
    marker_serial107=shown.serial;marker_elapsed107=shown.response_elapsed;
}
static bool acknowledged_image106(const overlay106::Ack& ack,overlay106::Key key,uint64_t serial,uint64_t battle){
    Frame93 image;if(!images113.read(ack,key,battle,serial,wall(),image))return false;
    drawn_prompt106=image.shown;drawn_wall106=image.submitted;return true;
}
// ---- Kit v10: kills per life, deaths and the battle statistics file -------------------------
static life173::Shared* life_view173=nullptr;static HANDLE life_map173=nullptr;static ULONGLONG life_retry173=0;
static LONG life_seen_deaths173=-1;static unsigned stats_written_kills173=~0u,stats_written_deaths173=~0u;
static ULONGLONG banner_until173=0;static wchar_t banner_text173[64]{};static char session173[48]{};
static BattleEpoch151 battle_watch174;static uint64_t battle_id174=0;static unsigned battle_kill_base174=0,battle_death_base174=0;
static void life_poll173(){
    const ULONGLONG now=GetTickCount64();
    if(!session173[0]){FILETIME c{},e{},k{},u{};GetProcessTimes(GetCurrentProcess(),&c,&e,&k,&u);
        sprintf_s(session173,"%lu-%llu",GetCurrentProcessId(),(unsigned long long)((uint64_t(c.dwHighDateTime)<<32)|c.dwLowDateTime));}
    if(!life_view173&&now>=life_retry173){
        life_retry173=now+1000;wchar_t name[96];life173::mapping_name(name,GetCurrentProcessId());
        HANDLE map=OpenFileMappingW(FILE_MAP_WRITE,FALSE,name);
        if(map){auto view=static_cast<life173::Shared*>(MapViewOfFile(map,FILE_MAP_WRITE,0,0,sizeof(life173::Shared)));
            if(view&&view->magic==life173::Magic&&view->version==life173::Version&&view->size==sizeof(life173::Shared)&&view->pid==GetCurrentProcessId()){life_view173=view;life_map173=map;}
            else{if(view)UnmapViewOfFile(view);CloseHandle(map);}}
    }
    LONG deaths=0;
    if(life_view173){
        deaths=life_view173->deaths;
        if(life_seen_deaths173<0)life_seen_deaths173=deaths;
        if(deaths!=life_seen_deaths173){
            life_seen_deaths173=deaths;
            const unsigned kills=tally173::end_life(uint64_t(uint32_t(life_view173->last_death_lease)));
            swprintf_s(banner_text173,L"Kills this life: %u",kills);banner_until173=now+4000;
        }
    }
    const unsigned total=tally173::total(),dead=unsigned(deaths<0?0:deaths);
    if((total||dead)&&(total!=stats_written_kills173||dead!=stats_written_deaths173)){
        if(life173::write_stats(session173,total,dead)){stats_written_kills173=total;stats_written_deaths173=dead;}
    }
    // Kit v13: battle totals and the current life for the HUD panels.
    ClockFrame clock{};const bool battle=engine&&battle_watch174.sample(reader(),engine,0,wall(),clock);
    if(battle&&clock.battle!=battle_id174){battle_id174=clock.battle;battle_kill_base174=total;battle_death_base174=dead;}
    if(life_view173){
        uint64_t lease=0;unsigned life_kills=0,life_rounds=0;tally173::life(lease,life_kills,life_rounds);
        observation::PossessionPrefix p{};bool stable=false;
        if(possession)for(unsigned attempt=0;attempt<3&&!stable;++attempt){auto q=possession->sequence;if(q&1)continue;MemoryBarrier();memcpy(&p,possession,sizeof p);MemoryBarrier();stable=q==possession->sequence;}
        auto* v=life_view173;InterlockedIncrement(&v->sequence);MemoryBarrier();
        v->in_battle=battle?1u:0u;v->battle_kills=total-battle_kill_base174;v->battle_deaths=dead-battle_death_base174;
        v->life_lease=uint32_t(lease);v->life_kills=life_kills;v->life_rounds=life_rounds;
        if(stable&&p.lease_generation){v->key_lease=p.lease_generation;v->key_actor=p.actor_id;v->key_unit=p.unit_id;}
        v->tick=now;MemoryBarrier();InterlockedIncrement(&v->sequence);
    }
}
static bool draw_banner173(){
    if(GetTickCount64()>=banner_until173||!possession)return false;
    game=gamewindow105::refresh(game,GetCurrentProcessId());
    if(!focused()||!IsWindow(game)||IsIconic(game))return false;
    RECT client{};POINT corner{};if(!gamewindow105::client(game,client,corner))return false;
    const float scale=hudlayout108::scale(float(client.right),float(client.bottom));
    const int size=int(std::ceil(320.f*scale));if(!surface93.ensure(size))return false;
    memset(surface93.pixels,0,size_t(size)*size*4);
    {Gdiplus::Bitmap image(size,size,size*4,PixelFormat32bppPARGB,static_cast<BYTE*>(surface93.pixels));
     Gdiplus::Graphics graphics(&image);graphics.ScaleTransform(scale,scale);render_banner173(graphics,160,160,banner_text173);}
    POINT pos={corner.x+client.right/2-size/2,corner.y+int(std::lround(hudlayout108::qte_anchor_y(float(client.bottom))))-size/2},origin{};SIZE extent={size,size};
    if(pixels106.open(GetCurrentProcessId(),overlay106::QTE)){
        observation::PossessionPrefix p{};bool stable=false;
        for(unsigned attempt=0;attempt<3;++attempt){auto s=possession->sequence;if(s&1)continue;MemoryBarrier();memcpy(&p,possession,sizeof p);MemoryBarrier();if(s==possession->sequence){stable=true;break;}}
        if(stable&&p.lease_generation){
            overlay106::Frame f{};f.key={unsigned(p.lease_generation),p.actor_id,p.unit_id};f.width=f.height=size;f.view_width=client.right;f.view_height=client.bottom;f.rect_count=1;
            f.active=2;f.rect[0]={pos.x-corner.x,pos.y-corner.y,size,size};
            if(pixels106.publish(f,surface93.pixels)){ShowWindow(overlay,SW_HIDE);return true;}
        }
    }
    BLENDFUNCTION alpha={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    bool painted=UpdateLayeredWindow(overlay,surface93.screen,&pos,&extent,surface93.dc,&origin,0,&alpha,ULW_ALPHA);
    bool positioned=SetWindowPos(overlay,HWND_TOPMOST,0,0,0,0,SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    return painted&&positioned;
}
static bool draw(const Snapshot& prompt,uint64_t lease,uint64_t battle){
    game=gamewindow105::refresh(game,GetCurrentProcessId());
    if(InterlockedExchange(&display_reset105,0))surface93.clear();
    if(!prompt.active||!focused()||!IsWindow(game)||IsIconic(game)){pixels106.hide();ShowWindow(overlay,SW_HIDE);return false;}
    RECT client{};POINT corner{};if(!gamewindow105::client(game,client,corner)){ShowWindow(overlay,SW_HIDE);return false;}
    const float scale=hudlayout108::scale(float(client.right),float(client.bottom));
    const int size=int(std::ceil(320.f*scale));if(!surface93.ensure(size))return false;
    memset(surface93.pixels,0,size_t(size)*size*4);
    {Gdiplus::Bitmap image(size,size,size*4,PixelFormat32bppPARGB,static_cast<BYTE*>(surface93.pixels));
     Gdiplus::Graphics graphics(&image);graphics.ScaleTransform(scale,scale);render(graphics,160,160,prompt);}
    POINT pos={corner.x+client.right/2-size/2,corner.y+int(std::lround(hudlayout108::qte_anchor_y(float(client.bottom))))-size/2},origin{};SIZE extent={size,size};
    if(pixels106.open(GetCurrentProcessId(),overlay106::QTE)&&possession){
        observation::PossessionPrefix p{};bool stable=false;
        for(unsigned attempt=0;attempt<3;++attempt){auto s=possession->sequence;if(s&1)continue;MemoryBarrier();memcpy(&p,possession,sizeof p);MemoryBarrier();if(s==possession->sequence){stable=true;break;}}
        if(!stable||p.lease_generation!=lease){pixels106.hide();ShowWindow(overlay,SW_HIDE);return false;}
        overlay106::Frame f{};f.key={unsigned(lease),p.actor_id,p.unit_id};f.width=f.height=size;f.view_width=client.right;f.view_height=client.bottom;f.rect_count=1;
        f.active=prompt.result==Result::Pending?1:2;f.rect[0]={pos.x-corner.x,pos.y-corner.y,size,size};
        auto number=pixels106.publish(f,surface93.pixels);if(number){images113.remember(number,f.key,battle,prompt);overlay106::Ack ack;
            if(pixels106.ack(ack)){ShowWindow(overlay,SW_HIDE);return acknowledged_image106(ack,f.key,prompt.serial,battle);}
        }
    }
    BLENDFUNCTION alpha={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    bool painted=!renderer_failed&&UpdateLayeredWindow(overlay,surface93.screen,&pos,&extent,surface93.dc,&origin,0,&alpha,ULW_ALPHA);
    bool positioned=SetWindowPos(overlay,HWND_TOPMOST,0,0,0,0,SWP_ASYNCWINDOWPOS|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    if(painted&&positioned){drawn_prompt106=prompt;drawn_wall106=wall();return true;}return false;
}
static void log_variant(const char* name,const observation::AnimationSnapshot& s){
    log_file<<",\""<<name<<"\":{\"valid\":"<<s.valid<<",\"outcome\":"<<s.outcome<<",\"matched\":"<<s.matched<<",\"fallback\":"<<s.fallback<<",\"requests\":[";
    for(unsigned i=0;i<s.request_count&&i<2;++i){if(i)log_file<<',';const auto& r=s.requests[i];
        log_file<<"{\"actor\":"<<r.actor<<",\"start\":"<<r.start<<",\"end\":"<<r.end<<",\"attacks\":"<<r.attacks<<"}";}
    log_file<<"]}";
}
// Presentation must never prevent input polling, publication or lease release.
// Win32 positioning of owned windows can synchronously message another thread.
// Use an unowned popup and isolate all window/GDI calls on this renderer.
static DWORD WINAPI render_worker(void*){
    ULONG_PTR gdiplus=0;
    try{
        Gdiplus::GdiplusStartupInput input;require(Gdiplus::GdiplusStartup(&gdiplus,&input,nullptr)==Gdiplus::Ok,"QTE drawing init failed");
        WNDCLASSW cls{};cls.hInstance=module;cls.lpfnWndProc=overlay_proc;cls.lpszClassName=L"ShogunMeleeQTE79";
        require(RegisterClassW(&cls)!=0,"QTE overlay class failed");
        overlay=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,
            cls.lpszClassName,L"Shogun melee QTE",WS_POPUP,0,0,320,320,nullptr,nullptr,module,nullptr);
        require(overlay!=nullptr,"QTE overlay failed");InterlockedExchange(&render_ready,1);SetEvent(render_initialized);
        PreciseWait93 pace93;precise_render93=pace93.precise();double last_frame93=wall(),next_refresh93=0;
        uint64_t marker_serial105=0;double marker_elapsed105=0,marker_changed105=0;
        while(!render_stop){
            InterlockedExchange64(&render_heartbeat,LONG64(GetTickCount64()));render_stage=1;
            MSG msg;while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
            transport::Payload data;Snapshot shown;double now=wall();
            bool show=transport::read(publication,data)&&transport::presentation(data,now,shown);
            if(!show){transport::Payload done;
                if(transport::read(completion_publication,done)&&done.prompt.serial>=data.prompt.serial&&
                   now-done.published_wall<(done.prompt.result==Result::Canceled?1.1:3.0)){show=transport::presentation(done,now,shown);if(show)data=done;}
            }
            bool click_freeze105=false;
            if(show&&shown.result==Result::Pending&&!shown.paused){transport::Payload frozen;
                if(transport::read(freeze_publication93,frozen)&&frozen.prompt.serial==shown.serial&&
                   frozen.lease==data.lease&&frozen.anchor.battle==data.anchor.battle&&now>=frozen.published_wall&&now-frozen.published_wall<.12){shown=frozen.prompt;click_freeze105=true;}
            }
            render_stage=2;
            life_poll173();
            if(!state->stop&&!show&&draw_banner173()){InterlockedExchange64(&presented_tick,0);}
            else if(!state->stop&&show&&draw(shown,data.lease,data.anchor.battle)){
                const double submitted=drawn_wall106;const auto& presented=drawn_prompt106;
                record_presented107(presented,submitted,click_freeze105);
                AcquireSRWLockExclusive(&frame_lock93);frame93={presented,data.anchor.battle,data.lease,submitted};ReleaseSRWLockExclusive(&frame_lock93);
                InterlockedIncrement(&frames93);LONG64 us=last_frame93>0?LONG64((submitted-last_frame93)*1e6):0;last_frame93=submitted;
                InterlockedExchange64(&last_frame_us93,us);if(us>max_frame_us93)InterlockedExchange64(&max_frame_us93,us);
                InterlockedExchange64(&presented_tick,LONG64(GetTickCount64()));InterlockedExchange64(&presented_serial,LONG64(presented.serial));
                if(presented.result==Result::Pending&&!presented.paused&&!click_freeze105){
                    if(marker_serial105!=presented.serial||std::abs(marker_elapsed105-presented.response_elapsed)>1e-6){
                        InterlockedIncrement(&moving_frames105);marker_changed105=submitted;
                    }else{
                        InterlockedIncrement(&repeated_frames105);LONG64 held=LONG64((submitted-marker_changed105)*1e6);
                        if(held>max_marker_hold_us105)InterlockedExchange64(&max_marker_hold_us105,held);
                    }
                    marker_serial105=presented.serial;marker_elapsed105=presented.response_elapsed;
                }else marker_serial105=0;
            }else {if(!show||state->stop)pixels106.hide();last_frame93=0;marker_serial105=marker_serial107=0;InterlockedExchange64(&presented_tick,0);ShowWindowAsync(overlay,SW_HIDE);}
            render_stage=3;
            if(now>=next_refresh93){MONITORINFOEXW mi{};mi.cbSize=sizeof(mi);DEVMODEW dm{};dm.dmSize=sizeof(dm);
                if(GetMonitorInfoW(MonitorFromWindow(game,MONITOR_DEFAULTTONEAREST),reinterpret_cast<MONITORINFO*>(&mi))&&EnumDisplaySettingsW(mi.szDevice,ENUM_CURRENT_SETTINGS,&dm)&&dm.dmDisplayFrequency>=30&&dm.dmDisplayFrequency<=360)refresh_hz93=LONG(dm.dmDisplayFrequency);
                next_refresh93=now+1.;}
            pace93.seconds(std::max(.0005,1./double(refresh_hz93)-(wall()-now)));
        }
        pixels106.hide();DestroyWindow(overlay);overlay=nullptr;surface93.clear();release_renderer();Gdiplus::GdiplusShutdown(gdiplus);render_ready=0;
    }catch(const std::exception&){render_error=1;render_ready=0;SetEvent(render_initialized);}
    return 0;
}
static DWORD WINAPI worker(void*){void* targets[10]{};bool hooked=false;HANDLE renderer=nullptr;
    try{
        LARGE_INTEGER f;QueryPerformanceFrequency(&f);frequency=double(f.QuadPart);
        (void)random_placement(); // Initialize entropy before installing hooks.
        wchar_t name[80];mapping_name(name,GetCurrentProcessId());HANDLE channel=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Status),name);
        require(channel&&GetLastError()!=ERROR_ALREADY_EXISTS,"QTE runtime already resident");
        state=static_cast<Status*>(MapViewOfFile(channel,FILE_MAP_ALL_ACCESS,0,0,sizeof(Status)));require(state!=nullptr,"QTE channel failed");
        memset(state,0,sizeof *state);state->magic=POC_MAGIC;state->version=POC_VERSION;state->pid=GetCurrentProcessId();state->size=sizeof *state;
        auto native=find_module(GetCurrentProcessId());require(native.size==0x1d88000,"Unsupported engine address layout");
        const auto binding172=engine172::admit(native.path,hash_file(native.path));engine=uint32_t(native.base);engine172::live_hooks(engine,2,binding172);
        auto movement=GetModuleHandleW(L"possession-v46.dll");require(movement!=nullptr,"Possession module unavailable");
        submit_commitment=reinterpret_cast<melee60::Submit>(GetProcAddress(movement,"PossessionMeleeCommitment63"));
        if(!submit_commitment)submit_commitment=reinterpret_cast<melee60::Submit>(GetProcAddress(movement,"_PossessionMeleeCommitment63"));
        request_rts=reinterpret_cast<uint32_t(__cdecl*)()>(GetProcAddress(movement,"PossessionRTS63"));
        if(!request_rts)request_rts=reinterpret_cast<uint32_t(__cdecl*)()>(GetProcAddress(movement,"_PossessionRTS63"));
        require(request_rts!=nullptr,"QTE79 RTS recovery port missing");
        require(submit_commitment!=nullptr,"QTE79 requires its possession preview adapter; restart with the preview build before loading");
        swprintf_s(name,L"Local\\Shogun2PossessionPOC46-%lu",GetCurrentProcessId());HANDLE map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);require(map!=nullptr,"Possession is not loaded");
        possession=static_cast<const observation::PossessionPrefix*>(MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof *possession));require(possession!=nullptr,"Possession channel failed");CloseHandle(map);
        uint32_t rvas[10]={0x3e5d40,0x3eaac0,0x3cea10,0x433b80,0x3e1930,0x3e3de0,0xb92800,0x3e5aa0,0x3eaa10,0x3c4170};
        const BYTE signatures[10][8]={{0x55,0x8b,0xec,0x83,0xe4,0xf0,0x81,0xec},{0x83,0xec,0x08,0x53,0x55,0x56,0x8b,0xf1},
            {0x56,0x8b,0x74,0x24,0x08,0x57,0x8b,0xf9},{0x55,0x8b,0xec,0x83,0xe4,0xf0,0x81,0xec},{0x56,0x8b,0x74,0x24,0x08,0x57,0x56,0x8b},{0x8b,0x54,0x24,0x04,0x8b,0x44,0x24,0x08},{0x81,0xec,0x84,0x00,0x00,0x00,0x53,0x55},{0x83,0xec,0x24,0x56,0x8b,0xf1,0x80,0x7e},{0x53,0x8b,0xd9,0x56,0x57,0x8b,0x43,0x04},{0x83,0xec,0x4c,0x56,0x57,0x8b,0xf1,0xe8}};
        for(unsigned i=0;i<10;++i){targets[i]=reinterpret_cast<void*>(engine+rvas[i]);require(!memcmp(targets[i],signatures[i],8),"Combat hook occupied at engine RVA "+hex(rvas[i]));if(i<6)memcpy(state->original_code+i*16,targets[i],16);}
        const BYTE sort_signature[]={0x83,0xec,0x08,0x53,0x55,0x56,0x8b,0x74};
        const BYTE select_call[]={0xe8,0x04,0xdc,0x04,0x00};
        require(!memcmp(reinterpret_cast<void*>(engine+0x3b55a0),sort_signature,8)&&
                !memcmp(reinterpret_cast<void*>(engine+0x3e5f77),select_call,5),"Native selection prefix differs");
        require(MH_Initialize()==MH_OK,"QTE hook init failed");hooked=true;
        void* hooks[10]={reinterpret_cast<void*>(prepare_hook),reinterpret_cast<void*>(update_hook),reinterpret_cast<void*>(remove_hook),reinterpret_cast<void*>(select_hook),reinterpret_cast<void*>(release_actor_hook),reinterpret_cast<void*>(target_hook),reinterpret_cast<void*>(score_hook),reinterpret_cast<void*>(contact_prepare_hook),reinterpret_cast<void*>(contact_update_hook),reinterpret_cast<void*>(avoid_pair_hook)};void** originals[10]={reinterpret_cast<void**>(&native_prepare),reinterpret_cast<void**>(&native_update),reinterpret_cast<void**>(&native_remove),reinterpret_cast<void**>(&selection_ports.select),reinterpret_cast<void**>(&native_release_actor),reinterpret_cast<void**>(&native_target),reinterpret_cast<void**>(&native_score),reinterpret_cast<void**>(&native_contact_prepare),reinterpret_cast<void**>(&native_contact_update),reinterpret_cast<void**>(&native_avoid_pair)};
        for(unsigned i=0;i<10;++i)require(MH_CreateHook(targets[i],hooks[i],originals[i])==MH_OK,"QTE hook create failed");
        selection_ports.sort=reinterpret_cast<NativeSort>(engine+0x3b55a0);
        const BYTE defence_signature[]={0x56,0x8b,0x74,0x24,0x08,0x57,0x8b,0xf9};
        const BYTE armour_signature[]={0x53,0x55,0x56,0x8b,0x74,0x24,0x14,0x8b};
        const BYTE facing_signature[]={0x83,0xec,0x10,0x8b,0x51,0x0c,0x8b,0x41};
        require(!memcmp(reinterpret_cast<void*>(engine+0xb94f50),defence_signature,8)&&
                !memcmp(reinterpret_cast<void*>(engine+0xb950d0),armour_signature,8)&&
                !memcmp(reinterpret_cast<void*>(engine+0x42a450),facing_signature,8)&&
                *reinterpret_cast<uint32_t*>(engine+0x15e73f8)==engine+0x42a450,"Native mitigation attribution differs");
        native_mitigation={reinterpret_cast<effective::Defence>(engine+0xb94f50),reinterpret_cast<effective::Armour>(engine+0xb950d0),reinterpret_cast<effective::Facing>(engine+0x42a450)};
        // Native0 selects a surviving exchange; native5 commits a stock casualty.
        const BYTE release_actor_sig[]={0x56,0x8b,0x74,0x24,0x08,0x57,0x56,0x8b};
        const BYTE release_aux_sig[]={0x56,0x8b,0xf1,0x8b,0x8e,0xfc,0x00,0x00};
        require(!memcmp(reinterpret_cast<void*>(engine+0x3e1930),release_actor_sig,sizeof release_actor_sig)&&
            !memcmp(reinterpret_cast<void*>(engine+0x3e18f0),release_aux_sig,sizeof release_aux_sig),"Native pair release differs");
        retarget_ports={native_prepare,native_release_actor,reinterpret_cast<retarget::ReleaseAux>(engine+0x3e18f0),engine};
        session=new Session(reader(),selection_ports.calls(),{nullptr,prepare_port,update_port,routing,facing_ready,retarget_port,effective_pair,retain_selected_pair,routed_prepare152},engine,{0,5,true},true,true,true,true,true);
        wchar_t path[MAX_PATH];GetModuleFileNameW(module,path,MAX_PATH);std::wstring folder=path;folder=folder.substr(0,folder.find_last_of(L'\\'));
        log_file<<std::setprecision(12);
        // The renderer reacquires a replaced window and waits through minimization.
        game=gamewindow105::find(GetCurrentProcessId());
        render_initialized=CreateEventW(nullptr,TRUE,FALSE,nullptr);require(render_initialized!=nullptr,"Renderer event failed");
        renderer=CreateThread(nullptr,0,render_worker,nullptr,0,nullptr);require(renderer!=nullptr,"Renderer thread failed");
        require(WaitForSingleObject(render_initialized,5000)==WAIT_OBJECT_0&&render_ready,"Renderer initialization failed");
        enabled=1;require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"QTE hook enable failed");state->ready=1;
        bool previous_left=false,previous_right=false,previous_focus=false;uint64_t next_log=0,logged_serial=0,logged_acquisitions=0,logged_resolutions=0,logged_playback=0;
        PlaybackWatch recorded_playback;
        uint32_t recorded_phase=melee60::Off,logged_phase=melee60::Off;
        uint64_t recorded_epoch151=0,recorded_restarts151=0,recorded_tick151=0;
        PreciseWait93 input_wait93;precise_input93=input_wait93.precise();double previous_poll93=wall();
        ServiceCadence94 cadence94;uint64_t input_polls94=0,service_runs94=0;
        overlay106::Reader input_surface113;
        while(true){const double poll93=wall();LONG64 poll_us93=LONG64((poll93-previous_poll93)*1e6);previous_poll93=poll93;InterlockedExchange64(&last_input_us93,poll_us93);if(poll_us93>max_input_us93)InterlockedExchange64(&max_input_us93,poll_us93);state->heartbeat=GetTickCount64();
            ++input_polls94;
            bool focus=focused(),left=focus&&(GetAsyncKeyState(VK_LBUTTON)&0x8000),right=focus&&(GetAsyncKeyState(VK_RBUTTON)&0x8000);
            if(cadence94.due(poll93,focus!=previous_focus||state->stop||stale_press93)){
                ++service_runs94;
            uint32_t root=0,env=0,manager=0,camera=0,actor=0;
            if(possession->ready&&possession->mode==2&&!possession->release&&!possession->stop){
                auto r=reader();if(r.get(engine+0x1bc8180,root)&&r.get(root+0x110,env)&&r.get(env+0x281fc,manager)&&
                   r.get(manager+0x8e0,camera))r.get(camera+0x188,actor);
            }
            InterlockedExchange(&player_address,LONG(actor));
            if(actor)InterlockedIncrement(&player_polls);
            if(!InterlockedCompareExchange(&coordinator,1,0)){
                drain_removals();ClockFrame clock;if(clock_frame(0,clock)){delivery_clock=clock;session->poll_clock(clock);}
                observation::PossessionPrefix latest93{};
                if(state->stop||!focus)session->release();
                else if(possession_snapshot93(latest93)&&(!latest93.ready||latest93.stop||latest93.release||latest93.mode!=2))session->release(CancelReason92::Owner);
                if(InterlockedExchange(&stale_press93,0))session->release(CancelReason92::Presentation93);
                publish();recorded_playback=playback;recorded_phase=commitment.phase;
                recorded_epoch151=battle_epoch151.epoch();recorded_restarts151=battle_epoch151.restarts();recorded_tick151=delivery_clock.tick;
                InterlockedExchange(&coordinator,0);
            }
            }
            transport::Payload data;Snapshot shown;bool show=transport::read(publication,data)&&transport::presentation(data,wall(),shown);
            const bool input_changed=left!=previous_left||right!=previous_right||focus!=previous_focus;
            bool click_queued=false;double click_simulation=0;Frame93 click_image113{};uint64_t click_number113=0;bool direct_ack113=false;double click_wall113=0;
            if(show&&(left!=previous_left||right!=previous_right)){Click click;Frame93 drawn;
                click_wall113=wall();
                AcquireSRWLockShared(&frame_lock93);drawn=frame93;ReleaseSRWLockShared(&frame_lock93);
                observation::PossessionPrefix current113{};overlay106::Ack ack113;
                if(possession_snapshot93(current113)&&current113.ready&&!current113.stop&&!current113.release&&current113.mode==2&&current113.lease_generation==data.lease&&
                   input_surface113.open(GetCurrentProcessId(),overlay106::QTE)&&input_surface113.ack(ack113)&&
                   images113.newest(ack113,{unsigned(data.lease),current113.actor_id,current113.unit_id},data,click_wall113,drawn)){
                    direct_ack113=true;click_number113=ack113.number;
                }
                click_image113=drawn;
                if(frame_click93(data,drawn,click_wall113,left,right,focus,click)){
                    AcquireSRWLockExclusive(&click_lock);click_queued=queued.push(click);ReleaseSRWLockExclusive(&click_lock);click_simulation=click.simulation;
                    if(click_queued){InterlockedIncrement(&click_frames93);InterlockedExchange64(&clicked_frame_age_us93,LONG64((click_wall113-drawn.submitted)*1e6));InterlockedExchange(&clicked_frame_inside93,in_success_zone(drawn.shown)?1:0);
                        if(!click.paused&&((left&&!previous_left)||(right&&!previous_right))){auto freeze=data;freeze.prompt=drawn.shown;freeze.published_wall=wall();transport::publish(freeze_publication93,freeze);}
                    }
                }else {InterlockedIncrement(&stale_frames93);if((left&&!previous_left)||(right&&!previous_right))InterlockedExchange(&stale_press93,1);}
            }
            if(log_file.enabled()&&input_changed){log_file<<"{\"event\":\"mouse_sample\",\"tick_ms\":"<<GetTickCount64()<<",\"focused\":"<<focus
                <<",\"fps\":"<<(possession->mode==2)<<",\"left\":"<<left<<",\"right\":"<<right<<",\"prompt_visible\":"<<show
                <<",\"prompt_serial\":"<<(show?shown.serial:0)<<",\"prompt_action\":"<<(show?uint32_t(shown.action):0)
                <<",\"facing_opponent\":"<<(show&&shown.aligning)<<",\"commitment_phase\":"<<recorded_phase
                <<",\"elapsed\":"<<(show?shown.elapsed:0)<<",\"duration\":"<<(show?shown.duration:0)
                <<",\"response_elapsed\":"<<(show?shown.response_elapsed:0)<<",\"response_seconds\":"<<(show?qte::response_deadline(shown):qte::ResponseWindowSeconds91)<<",\"response_remaining\":"<<(show?qte::response_remaining(shown):0)
                <<",\"reticle\":"<<(show?reticle_position(shown):0)<<",\"zone_left\":"<<(show?zone_left(shown):0)<<",\"zone_width\":"<<(show?shown.zone_width:0)<<",\"placement\":"<<(show?shown.placement:0)
                <<",\"ideal\":"<<(show?shown.ideal:0)<<",\"tolerance\":"<<(show?shown.tolerance:0)
                <<",\"paused\":"<<(show&&shown.paused)<<",\"queued\":"<<click_queued<<",\"click_simulation\":"<<click_simulation;
                if(click_queued)log_file<<",\"click_image113\":{\"native_ack\":"<<direct_ack113<<",\"number\":"<<click_number113
                    <<",\"age_us\":"<<LONG64((click_wall113-click_image113.submitted)*1e6)<<",\"response_elapsed\":"<<click_image113.shown.response_elapsed
                    <<",\"reticle\":"<<reticle_position(click_image113.shown)<<",\"zone_left\":"<<zone_left(click_image113.shown)<<",\"zone_width\":"<<click_image113.shown.zone_width
                    <<",\"inside\":"<<in_success_zone(click_image113.shown)<<"}";
                log_file<<"}\n";log_file.flush();}
            previous_left=left;previous_right=right;previous_focus=focus;
            transport::Payload completed;Snapshot feedback;const double now=wall();
            bool feedback_visible=!show&&transport::read(completion_publication,completed)&&
                now-completed.published_wall<(completed.prompt.result==Result::Canceled?1.1:3.0)&&transport::presentation(completed,now,feedback);
            if(log_file.enabled()&&(GetTickCount64()>=next_log||logged_serial!=last_serial||logged_acquisitions!=acquisitions||logged_resolutions!=resolutions||logged_playback!=recorded_playback.revision||logged_phase!=recorded_phase)){
                const auto registry94=lifetimes.metrics94();
                ScoreMetrics metrics;const bool metrics_valid=read_score_metrics(metrics);
                log_file<<"{\"tick_ms\":"<<GetTickCount64()<<",\"ready\":"<<state->ready<<",\"pending\":"<<pending_address<<",\"acquired\":"<<acquisitions<<",\"resolved\":"<<resolutions<<",\"faults\":"<<state->faults
                <<",\"qte_window95\":1,\"minimum_response_seconds95\":3,\"minimum_cycles95\":1"
                <<",\"performance94\":1,\"service_period_us94\":8000,\"input_polls94\":"<<input_polls94<<",\"service_runs94\":"<<service_runs94
                <<",\"registry_capacity94\":"<<observation::Registry::capacity94<<",\"registry_live94\":"<<registry94.live<<",\"registry_peak94\":"<<registry94.peak
                <<",\"registry_gets94\":"<<registry94.gets<<",\"registry_get_probes94\":"<<registry94.get_probes<<",\"registry_max_get_probes94\":"<<registry94.max_get_probes
                <<",\"registry_removes94\":"<<registry94.removes<<",\"registry_remove_probes94\":"<<registry94.remove_probes<<",\"registry_shift_probes94\":"<<registry94.shift_probes
                <<",\"registry_moves94\":"<<registry94.moves<<",\"registry_full94\":"<<registry94.full
                <<",\"timing93\":1,\"precise_input93\":"<<precise_input93<<",\"precise_render93\":"<<precise_render93<<",\"refresh_hz93\":"<<refresh_hz93
                <<",\"frames93\":"<<frames93<<",\"click_frames93\":"<<click_frames93<<",\"stale_frames93\":"<<stale_frames93
                <<",\"input_interval_us93\":"<<InterlockedCompareExchange64(&last_input_us93,0,0)<<",\"max_input_interval_us93\":"<<InterlockedCompareExchange64(&max_input_us93,0,0)
                <<",\"clicked_frame_age_us93\":"<<InterlockedCompareExchange64(&clicked_frame_age_us93,0,0)<<",\"clicked_frame_inside93\":"<<clicked_frame_inside93
                <<",\"frame_interval_us93\":"<<InterlockedCompareExchange64(&last_frame_us93,0,0)<<",\"max_frame_interval_us93\":"<<InterlockedCompareExchange64(&max_frame_us93,0,0)
                <<",\"clock105\":1,\"anchor_simulation105\":"<<data.anchor.simulation<<",\"anchor_horizon105\":"<<data.anchor.horizon
                <<",\"battle_epoch151\":"<<recorded_epoch151<<",\"battle_restarts151\":"<<recorded_restarts151<<",\"native_tick151\":"<<recorded_tick151
                <<",\"moving_frames105\":"<<moving_frames105<<",\"repeated_frames105\":"<<repeated_frames105<<",\"max_marker_hold_us105\":"<<InterlockedCompareExchange64(&max_marker_hold_us105,0,0)
                <<",\"clock107\":1,\"unique_frames107\":"<<unique_frames107<<",\"moving_frames107\":"<<moving_frames107<<",\"repeated_frames107\":"<<repeated_frames107<<",\"max_marker_hold_us107\":"<<InterlockedCompareExchange64(&max_marker_hold_us107,0,0)
                <<",\"canceled92_total\":"<<canceled_total92<<",\"canceled92_reason\":"<<canceled_reason92<<",\"canceled92_serial\":"<<InterlockedCompareExchange64(&canceled_serial92,0,0)
                <<",\"registration92_ready\":"<<registration_ready92<<",\"registration92_waiting\":"<<registration_waiting92<<",\"registration92_rejected\":"<<registration_rejected92<<",\"registration92_reason_mask\":"<<registration_reason92
                <<",\"commitment_phase\":"<<recorded_phase
                <<",\"last_serial\":"<<last_serial<<",\"last_action\":"<<last_action<<",\"last_result\":"<<last_result<<",\"last_native_outcome\":"<<last_outcome<<",\"retarget_stage\":"<<last_retarget_stage
                <<",\"render_heartbeat\":"<<InterlockedCompareExchange64(&render_heartbeat,0,0)<<",\"render_stage\":"<<render_stage<<",\"render_error\":"<<render_error
                <<",\"prompt_visible\":"<<(show?1:0)<<",\"elapsed\":"<<(show?shown.elapsed:0)<<",\"duration\":"<<(show?shown.duration:0)
                <<",\"response_elapsed\":"<<(show?shown.response_elapsed:0)<<",\"response_seconds\":"<<(show?qte::response_deadline(shown):qte::ResponseWindowSeconds91)<<",\"response_remaining\":"<<(show?qte::response_remaining(shown):0)
                <<",\"prompt_serial\":"<<(show?shown.serial:0)<<",\"prompt_action\":"<<(show?uint32_t(shown.action):0)
                <<",\"player\":"<<player_address<<",\"player_polls\":"<<player_polls<<",\"filtered\":"<<foreign_filtered<<",\"clock_rejected\":"<<clock_rejected
                <<",\"delivery_witness\":"<<delivery_address<<",\"suppressed_prompts\":"<<suppressed_prompts
                <<",\"contacts_blocked\":"<<contacts_blocked<<",\"preplay_avoidance_blocked\":"<<preplay_avoidance_blocked<<",\"conflicts_blocked\":"<<conflicts_blocked<<",\"foreign_releases_preserved\":"<<foreign_releases_preserved<<",\"unapproved_melee_rejected\":"<<melee_rejected<<",\"native_score_samples\":"<<native_score_samples<<",\"stock_charge_prepares140\":"<<stock_charge_prepares140<<",\"stock_charge_updates140\":"<<stock_charge_updates140<<",\"routing_prepares152\":"<<routing_prepares152<<",\"routing_transitions152\":"<<routing_transitions152
                <<",\"score_capture_rejected\":"<<score_capture_rejected<<",\"effective_score_rejected\":"<<effective_score_rejected
                <<",\"score_metrics_valid\":"<<metrics_valid<<",\"score_player\":"<<metrics.player<<",\"score_enemy\":"<<metrics.opponent<<",\"score_generation\":"<<metrics.generation
                <<",\"player_power\":"<<metrics.own<<",\"enemy_power\":"<<metrics.enemy<<",\"player_margin\":"<<metrics.detail.own_margin<<",\"enemy_margin\":"<<metrics.detail.enemy_margin
                <<",\"player_mitigation\":"<<metrics.detail.own_mitigation<<",\"enemy_mitigation\":"<<metrics.detail.enemy_mitigation
                <<",\"owner_accepted\":"<<owner_accepted<<",\"owner_rejected\":[";
                for(unsigned i=0;i<10;++i){if(i)log_file<<',';log_file<<owner_rejected[i];}log_file<<"],\"bridge_stages\":[";
                for(unsigned i=0;i<12;++i){if(i)log_file<<',';log_file<<bridge_stages[i];}
                log_file<<"],\"feedback_visible\":"<<(feedback_visible?1:0)<<",\"playback\":{\"serial\":"<<recorded_playback.serial
                    <<",\"outcome\":"<<recorded_playback.native_outcome<<",\"resolved_wall\":"<<recorded_playback.resolved_wall
                    <<",\"prepared_wall\":"<<recorded_playback.prepared_wall<<",\"aligned_wall\":"<<recorded_playback.aligned_wall
                    <<",\"playing_wall\":"<<recorded_playback.playing_wall<<",\"removed_wall\":"<<recorded_playback.removed_wall
                    <<",\"prepared\":"<<recorded_playback.prepared<<",\"aligned\":"<<recorded_playback.aligned<<",\"playing\":"<<recorded_playback.playing
                    <<",\"prepare_return_valid\":"<<recorded_playback.prepare_return_valid<<",\"prepare_return\":"<<recorded_playback.prepare_return;
                log_variant("before_variant",recorded_playback.before_variant);log_variant("after_variant",recorded_playback.after_variant);log_file<<"}}\n";
                log_file.flush();next_log=GetTickCount64()+(pending_address?100:1000);
                logged_serial=last_serial;logged_acquisitions=acquisitions;logged_resolutions=resolutions;logged_playback=recorded_playback.revision;logged_phase=recorded_phase;}
            if(state->stop&&request_rts)request_rts();
            if(state->stop&&!pending_address&&!fps_actor())break;input_wait93.seconds(.001);
        }
        clear_commitment();enabled=0;require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK,"QTE restoration failed");
        while(active_callbacks)Sleep(1);
        for(unsigned i=6;i<10;++i)require(!memcmp(targets[i],signatures[i],8),"Native extra hook restoration differs");
        for(unsigned i=0;i<6;++i){memcpy(state->restored_code+i*16,targets[i],16);require(!memcmp(state->original_code+i*16,state->restored_code+i*16,16),"QTE original code differs");}
        InterlockedExchange(&render_stop,1);if(renderer){WaitForSingleObject(renderer,1500);CloseHandle(renderer);renderer=nullptr;}
        state->ready=0;state->mode=DISABLED;
    }catch(const std::exception& error){startup167::remember(error.what());if(!pending_address){enabled=0;if(hooked)MH_DisableHook(MH_ALL_HOOKS);}if(state){state->error=1;state->ready=0;}if(log_file)log_file<<"{\"error\":\""<<error.what()<<"\"}\n";}
    InterlockedExchange(&render_stop,1);return 0;
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){module=instance;HANDLE thread=CreateThread(nullptr,0,worker,nullptr,0,nullptr);if(!thread)return FALSE;CloseHandle(thread);}return TRUE;}

