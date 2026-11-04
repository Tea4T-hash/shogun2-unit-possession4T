#pragma once
#include "native_selection_prefix.h"
#include "native_combat_actor.h"
#include "qte_pair_gate.h"
// Kit v12: kill tally built in here (no separate file needed).
#ifndef S2UP_KILL_TALLY173
#define S2UP_KILL_TALLY173
// Kills made by the possessed soldier in matched melee (QTE exchanges and routed exchanges).
// Arrows, gunfire and charge impacts are not reported to this runtime and are not counted.
#include <windows.h>
#include <cstdint>
namespace tally173 {
inline SRWLOCK lock=SRWLOCK_INIT;
inline uint64_t current_lease=0;
inline unsigned current_kills=0,total_kills=0;
inline void kill(uint64_t lease){
    AcquireSRWLockExclusive(&lock);
    if(lease!=current_lease){current_lease=lease;current_kills=0;}
    ++current_kills;++total_kills;
    ReleaseSRWLockExclusive(&lock);
}
// Kills of the life that just ended; the count for that life starts again from zero.
inline unsigned end_life(uint64_t lease){
    AcquireSRWLockExclusive(&lock);
    unsigned kills=(lease&&(lease&0xffffffffull)==(current_lease&0xffffffffull))?current_kills:0;
    current_kills=0;current_lease=0;
    ReleaseSRWLockExclusive(&lock);return kills;
}
inline unsigned total(){AcquireSRWLockShared(&lock);unsigned t=total_kills;ReleaseSRWLockShared(&lock);return t;}
}
#endif

namespace qte {
inline bool actor_identity(const Reader& r,uint32_t actor,uint32_t witness,
                           uint32_t engine,NativeActor& out,bool unprepared=false){
    if(!actor||actor>UINT32_MAX-0x700)return false;
    uint32_t backlink{};NativeActor identity,rider;
    if(!combat_identity(r,actor,engine,identity)||
       (identity.kind==Horse&&!horse_rider(r,engine,identity,rider))||!r.get(actor+0x688,backlink)||
       (backlink!=witness&&(!unprepared||backlink||!preparation_participant(r,witness,engine,actor))))return false;
    out=identity;return true;
}
struct Owner {
    uint64_t battle{},lease{};uint32_t environment{};
    NativeActor actor{};
    // Supplied by the adapter's existing native possession/weapon guards.
    bool verified_melee{};bool observation_busy93{};
};
struct NativePlan {uint32_t actor{},state{},clip{},kind{};};
struct BridgeCalls {
    void* context{};
    uint32_t(*prepare)(void*,uint32_t){};
    uint32_t(*update)(void*,uint32_t){};
    bool(*routing)(void*,uint32_t,bool&){};
    bool(*facing_ready)(void*,uint32_t,uint32_t){};
    uint32_t(*retarget)(void*,uint32_t,const NativeVerdict&,const NativePlan*){};
    bool(*effective_pair)(void*,uint32_t,uint32_t,float&,float&){};
    bool(*retain_selected_pair)(void*,uint32_t,const NativeActor&,const NativeActor&){};
    uint32_t(*routed152)(void*,uint32_t,const NativeActor&,const NativeActor&,const NativeVerdict&,const NativePlan*){};
};
// Native callback coordinator. A witness migrates between engine workers;
// the adapter serializes owned state and scopes replay to the current callback.
// Native ABI wrappers, generation registration and IPC remain adapter concerns.
enum class CancelReason92:uint32_t {External=1,Owner=2,NativeIdentity=3,NativeRemoval=4,Facts=5,Score=6,InputOverflow=7,ClickTimestamp=8,InvalidInputDelta=9,Presentation93=10};
struct CancellationStats92 {uint32_t total{},reason{};uint64_t serial{};};
struct RegistrationStats92 {uint32_t ready{},waiting{},rejected{},reason_mask{};};
class NativeBridge {
    Reader reader_;SelectionCalls selection_;BridgeCalls calls_;uint32_t engine_{};
    PairGate gate_{};SelectorReplay replay_{};InputEdges edges_{};
    WitnessKey key_{};SelectionInput input_{};NativeVerdict native_{};
    NativeActor player_{},enemy_{};
    UnitState own_facts_{},enemy_facts_{};
    bool paused_{},left_{},right_{},aligning_{};
    bool invalidate_{};
    bool outcomes_valid_{};
    bool preserve_special_{};
    bool prepared_mode_{};bool strict_{};
    bool routed_frame152_{};uint32_t routed_answer152_{};
    uint32_t forward_or_reject(uint32_t witness,bool owned){return strict_&&owned?1:calls_.prepare(calls_.context,witness);}
    uint32_t reject_selected(const SelectionInput& input,const NativeVerdict& selected){return strict_?1:cached_prepare(input,selected);}
    NativePlan plans_[2]{};
    uint32_t replay_errors_{};
    bool preparing92_{},preparation_canceled92_{},preparation_charge92_{};
    RegistrationStats92 registration92_{};
    CancellationStats92 cancellation92_{};Snapshot interrupted93_{};uint64_t interrupted_lease93_{},interrupted_battle93_{};
    void canceled92(CancelReason92 reason){
        if(gate_.occupied()&&gate_.snapshot().active&&gate_.snapshot().result==Result::Pending){
            interrupted_lease93_=key_.pair.lease;interrupted_battle93_=key_.pair.battle;interrupted93_=gate_.snapshot();interrupted93_.active=true;interrupted93_.paused=false;interrupted93_.result=Result::Canceled;
            ++cancellation92_.total;cancellation92_.reason=uint32_t(reason);cancellation92_.serial=interrupted93_.serial;
        }
    }
    enum class Registration92 {Invalid,Waiting,Ready};
    Registration92 registration_state92(uint32_t returned){
        uint32_t env{},count{},a{},b{},outcome{},attacker{},defender{},matched{},fallback{};
        uint8_t active{},playing{},waiting{};uint32_t mask=0;
        if(!reader_.get(key_.address+4,env)||!reader_.get(key_.address+0xc,active)||
           !reader_.get(key_.address+0xd0,waiting)||!reader_.get(key_.address+0xd1,playing)||
           !reader_.get(key_.address+0x38,count)||!reader_.get(key_.address+0x30,a)||!reader_.get(key_.address+0x34,b)||
           !reader_.get(key_.address+0x80,outcome)||!reader_.get(key_.address+0x84,attacker)||!reader_.get(key_.address+0x88,defender)||
           !reader_.get(key_.address+0x98,matched)||!reader_.get(key_.address+0xb4,fallback))mask|=1;
        if(returned&0xff)mask|=2;
        if(env!=input_.environment)mask|=4;
        const bool pending=active==0&&waiting==1&&playing==0&&matched!=0;
        if(active>1||playing!=0||(!active&&!pending))mask|=8;
        if(count!=2||!((a==player_.address&&b==enemy_.address)||(a==enemy_.address&&b==player_.address)))mask|=16;
        if(outcome!=native_.outcome||attacker!=native_.attacker_address||defender!=native_.defender_address)mask|=32;
        if(!matched&&!fallback)mask|=64;
        NativeActor player,enemy;
        // Pending native preparation legitimately has no registered backlinks yet.
        // Nonzero foreign backlinks, dead/reused actors and changed units still fail.
        auto binding=[&](const NativeActor& expected,NativeActor& current){
            if(!pending)return actor_identity(reader_,expected.address,key_.address,engine_,current);
            uint32_t link{};NativeActor rider;
            // selection_input intentionally rejects +D0 states. Here the exact
            // native principals/roles above prove membership, while these checks
            // retain alive/type/unit/serial/rider and zero-or-owned backlink proof.
            return combat_identity(reader_,expected.address,engine_,current)&&
                (current.kind!=Horse||horse_rider(reader_,engine_,current,rider))&&
                reader_.get(expected.address+0x688,link)&&(link==0||link==key_.address);
        };
        if(!binding(player_,player)||!binding(enemy_,enemy)||
           !same_actor(player,player_)||!same_actor(enemy,enemy_))mask|=128;
        if(!mask){player_=player;enemy_=enemy;} // Refresh native heights without changing actor identity.
        registration92_.reason_mask=mask;
        return mask?Registration92::Invalid:pending?Registration92::Waiting:Registration92::Ready;
    }
    uint32_t finish_registration92(uint32_t returned){
        const auto state=registration_state92(returned);
        if(state==Registration92::Waiting){++registration92_.waiting;acquisition_stage_=12;return returned;}
        preparing92_=false;
        if(state!=Registration92::Ready){++registration92_.rejected;return strict_?1:returned;}
        // A native waiting interval consumes no response budget. Refresh current
        // facts once before admitting the visible, timed event.
        if(!facts(player_,own_facts_)||!facts(enemy_,enemy_facts_)){
            registration92_.reason_mask=256;++registration92_.rejected;return 1;
        }
        if(calls_.effective_pair){
            if(!calls_.effective_pair(calls_.context,player_.address,enemy_.address,own_facts_.combat_score,enemy_facts_.combat_score)){
                registration92_.reason_mask=512;++registration92_.rejected;return 1;
            }
            own_facts_.known|=EffectiveCombat;enemy_facts_.known|=EffectiveCombat;
        }
        if(strict_)own_facts_.routing=false;
        const bool attacks=native_.attacker_address==player_.address;
        Verdict verdict{native_.outcome,attacks?player_.serial:enemy_.serial,attacks?enemy_.serial:player_.serial};
        if(!gate_.acquire(key_,verdict,own_facts_,enemy_facts_,preparation_charge92_)){
            registration92_.reason_mask=1024;++registration92_.rejected;return 1;
        }
        ++registration92_.ready;acquisition_stage_=11;
        aligning_=calls_.facing_ready&&!calls_.facing_ready(calls_.context,player_.address,enemy_.address);
        gate_.update(key_,own_facts_,enemy_facts_,0,paused_);edges_.arm(gate_.snapshot().serial,left_,right_);
        return 0;
    }
    uint32_t continue_registration92(const Owner& owner){
        if(preparation_canceled92_||invalidate_||owner.battle!=key_.pair.battle||owner.lease!=key_.pair.lease||
           owner.environment!=input_.environment||!same_actor(owner.actor,player_)){
            preparing92_=false;registration92_.reason_mask=2048;++registration92_.rejected;return 1;
        }
        if(paused_)return 0;
        auto before=registration_state92(0);
        if(before==Registration92::Invalid){preparing92_=false;++registration92_.rejected;return 1;}
        if(before==Registration92::Ready)return finish_registration92(0);
        // RVA3E5D40's +D0 continuation skips the selector. Do not arm its cached
        // replay or consume selection/RNG again. Native code owns its10s limit.
        return finish_registration92(calls_.prepare(calls_.context,key_.address));
    }
    // Acquisition trace: 1 eligibility, 2 input, 3 identity, 4 participant,
    // 5 selector, 6 special outcome, 7 enemy, 8 own facts, 9 enemy facts,
    // 10 gate, 11 acquired. No logging or allocation in engine callbacks.
    uint32_t acquisition_stage_{};
    bool identities_valid()const{
        if(prepared_mode_){
            if(invalidate_)return false;
            uint32_t env{},count{},a{},b{};uint8_t active{},playing{};
            NativeActor player,enemy;
            return reader_.get(key_.address+4,env)&&env==input_.environment&&
                reader_.get(key_.address+0xc,active)&&active==1&&
                reader_.get(key_.address+0xd1,playing)&&playing==0&&
                reader_.get(key_.address+0x38,count)&&count==2&&
                reader_.get(key_.address+0x30,a)&&reader_.get(key_.address+0x34,b)&&
                ((a==player_.address&&b==enemy_.address)||(a==enemy_.address&&b==player_.address))&&
                actor_identity(reader_,player_.address,key_.address,engine_,player)&&
                actor_identity(reader_,enemy_.address,key_.address,engine_,enemy)&&
                same_actor(player,player_)&&same_actor(enemy,enemy_);
        }
        NativeActor player,enemy;SelectionInput current;
        if(invalidate_||!selection_input(reader_,key_.address,engine_,current)||
           current.environment!=input_.environment||current.context!=input_.context||
           current.begin!=input_.begin||current.end!=input_.end||current.count!=input_.count)return false;
        bool includes_player=false,includes_enemy=false;
        for(uint32_t i=0;i<current.count;++i){uint32_t actor{};
            if(!reader_.get(current.begin+i*4,actor))return false;
            includes_player|=actor==player_.address;includes_enemy|=actor==enemy_.address;
        }
        if(!includes_player||!includes_enemy)return false;
        return actor_identity(reader_,player_.address,key_.address,engine_,player,true)&&
               actor_identity(reader_,enemy_.address,key_.address,engine_,enemy,true)&&
               same_actor(player,player_)&&same_actor(enemy,enemy_);
    }
    bool facts(const NativeActor& actor,UnitState& out)const{
        bool routing{};
        return calls_.routing&&calls_.routing(calls_.context,actor.unit,routing)&&
            unit_context(reader_,actor.unit,actor.height,routing,out);
    }
    uint32_t cached_prepare(const SelectionInput& input,const NativeVerdict& result){
        // Never call the original selector a second time after preview consumed
        // it. The ABI wrapper calls selector() from within original prepare.
        if(!replay_.arm(input,result)){++replay_errors_;return 1;}
        uint32_t returned=calls_.prepare(calls_.context,input.witness);
        if(!replay_.finish())++replay_errors_;
        return returned;
    }
public:
    NativeBridge(Reader reader,SelectionCalls selection,BridgeCalls calls,uint32_t engine,OutcomeBindings outcomes={},bool preserve_special=false,bool prepared_mode=false,bool strict=false):
        reader_(reader),selection_(selection),calls_(calls),engine_(engine),gate_(outcomes),outcomes_valid_(outcomes.valid()),preserve_special_(preserve_special),prepared_mode_(prepared_mode),strict_(strict){}
    // The hook wrapper uses this strict lookup before forwarding a foreign
    // selector call itself. No engine function runs under its state lock.
    bool try_replay(uint32_t witness,uint32_t return_rva,uint32_t context,
                    uint32_t begin,uint32_t end,uint32_t flags,uint32_t flags2,NativeVerdict& out){
        return replay_.take(witness,return_rva,context,begin,end,flags,flags2,out);
    }
    bool occupied()const{return preparing92_||gate_.occupied();}
    bool take_routed152(uint32_t witness,uint64_t generation,uint32_t& answer){if(!routed_frame152_||witness!=key_.address||generation!=key_.generation)return false;routed_frame152_=false;answer=routed_answer152_;return true;}
    WitnessKey pending_key()const{return occupied()?key_:WitnessKey{};}
    bool owner_matches(const Owner& owner)const{
        return !occupied()||(owner.verified_melee&&owner.battle==key_.pair.battle&&
            owner.lease==key_.pair.lease&&owner.environment==input_.environment&&same_actor(owner.actor,player_));
    }
    bool selector(uint32_t scoped_witness,uint32_t return_rva,uint32_t context,
                  uint32_t begin,uint32_t end,uint32_t flags,uint32_t flags2,NativeVerdict& out){
        if(replay_.take(scoped_witness,return_rva,context,begin,end,flags,flags2,out))return true;
        return selection_.select&&selection_.select(selection_.context,context,begin,end,flags,flags2,out);
    }
    uint32_t prepare(uint32_t witness,uint64_t generation,const Owner& owner,bool allow_prompt=true){
        if(!calls_.prepare)return 1;
        if(preparing92_&&witness==key_.address&&generation==key_.generation)return continue_registration92(owner);
        if(gate_.occupied()&&witness==key_.address&&generation==key_.generation){
            if(!identities_valid()){
                canceled92(CancelReason92::NativeIdentity);gate_.removed(witness,generation);
                // Native manager sees AL1 and invokes its ordinary removal.
                // Never replay a cached role pointer after death/address reuse.
                return 1;
            }
            auto decision=gate_.prepare(key_);if(decision==Prepare::Defer)return 0;
            if(decision==Prepare::Replay){
                if(strict_&&gate_.snapshot().result==Result::Canceled){gate_.removed(witness,generation);return 1;}
                Verdict verdict;
                if(!gate_.begin_replay(key_,true,verdict))return 1;
                auto result=native_;result.outcome=verdict.outcome&0xff;
                if(verdict.outcome&ReverseRoles)std::swap(result.attacker_address,result.defender_address);
                if(result.outcome==5&&result.attacker_address==player_.address)tally173::kill(key_.pair.lease); // kit v10: kill by the possessed soldier
                uint32_t returned;
                if(prepared_mode_){
                    if(!replay_.arm(input_,result)){++replay_errors_;return 1;}
                    returned=calls_.retarget?calls_.retarget(calls_.context,witness,result,plans_):1;
                    if(!replay_.finish())++replay_errors_;
                }else returned=cached_prepare(input_,result);
                gate_.finish_replay(key_);return returned;
            }
        }
        acquisition_stage_=1;
        bool route_candidate152=false;
        if(!allow_prompt&&calls_.routed152&&owner.verified_melee&&calls_.routing){
            SelectionInput route_input152;
            if(selection_input(reader_,witness,engine_,route_input152)&&route_input152.environment==owner.environment){
                for(unsigned i=0;i<route_input152.count;++i){uint32_t a{};NativeActor candidate;bool routed=false;
                    if(reader_.get(route_input152.begin+i*4,a)&&actor_identity(reader_,a,witness,engine_,candidate,true)&&
                       calls_.routing(calls_.context,candidate.unit,routed)&&routed)route_candidate152=true;
                }
            }
        }
        if((!allow_prompt&&!route_candidate152)||!outcomes_valid_||occupied()||!owner.verified_melee||!owner.battle||!owner.lease||!generation)
            return forward_or_reject(witness,owner.verified_melee);
        SelectionInput input;
        acquisition_stage_=2;
        if(!selection_input(reader_,witness,engine_,input)||input.environment!=owner.environment)
            return forward_or_reject(witness,owner.verified_melee);
        if(prepared_mode_&&!calls_.retarget)return forward_or_reject(witness,owner.verified_melee);
        NativeActor owned;
        acquisition_stage_=3;
        if(!actor_identity(reader_,owner.actor.address,witness,engine_,owned,true)||!same_actor(owned,owner.actor))
            return forward_or_reject(witness,owner.verified_melee);
        bool participant=false;
        acquisition_stage_=4;
        for(uint32_t i=0;i<input.count;++i){uint32_t actor{};
            if(!reader_.get(input.begin+i*4,actor))return forward_or_reject(witness,owner.verified_melee);
            if(actor==owned.address)participant=true;
        }
        if(!participant)return forward_or_reject(witness,owner.verified_melee);
        NativeVerdict selected;
        SelectionProgress progress;
        acquisition_stage_=5;
        if(!select_once(reader_,selection_,input,selected,&progress)){
            if(progress==SelectionProgress::Attempted){++replay_errors_;return 1;}
            return forward_or_reject(witness,owner.verified_melee);
        }
        // Normal rolls: keep the game's own kill (5); any special outcome becomes a surviving exchange.
        if(strict_&&selected.outcome!=5)selected.outcome=0;
        // Committed native charges are forwarded before this QTE-only path.
        // A pose alone must never reorient the stock attacker or force a kill.
        bool attacks=selected.attacker_address==owned.address;
        bool defends=selected.defender_address==owned.address;
        acquisition_stage_=6;
        if(!strict_&&preserve_special_&&selected.outcome!=0&&selected.outcome!=5)return reject_selected(input,selected);
        NativeActor enemy;
        uint32_t mount{};
        acquisition_stage_=7;
        if((!attacks&&!defends)||attacks==defends||
           !actor_identity(reader_,attacks?selected.defender_address:selected.attacker_address,witness,engine_,enemy,true)||
           enemy.unit==owned.unit||enemy.side==owned.side||!reader_.get(enemy.address+0x21c,mount)||mount)
            return reject_selected(input,selected);
        uint32_t enemy_ranged{};
        if(!strict_&&prepared_mode_&&(!reader_.get(enemy.unit+0xf7c,enemy_ranged)||enemy_ranged))return reject_selected(input,selected);
        const bool charge=false;
        if(prepared_mode_&&input.count!=2){
            // Select from the ORIGINAL native group exactly once, then retain
            // its chosen pair. Never substitute a nearest enemy before selection.
            if(!calls_.retain_selected_pair||!calls_.retain_selected_pair(calls_.context,witness,owned,enemy))return reject_selected(input,selected);
            SelectionInput retained;uint32_t first{},selector_word{};
            if(!selection_input(reader_,witness,engine_,retained)||retained.count!=2||
               retained.environment!=input.environment||retained.context!=input.context||
               !reader_.get(retained.begin,first)||!reader_.get(first+0x21c,selector_word)||
               bool(selector_word)!=bool(input.flags)){++replay_errors_;return 1;}
            retained.flags=input.flags;input=retained;
        }
        UnitState own_facts,enemy_facts;
        acquisition_stage_=8;
        if(!facts(owned,own_facts))return reject_selected(input,selected);
        acquisition_stage_=9;
        if(!facts(enemy,enemy_facts))return reject_selected(input,selected);
        if(calls_.routed152&&(own_facts.routing||enemy_facts.routing)){
            // User policy: an engaged broken/shattered player loses; otherwise
            // the engaged broken/shattered enemy loses. Native role/clip
            // selection consumes the original pair once, without a QTE event.
            selected.outcome=5;selected.attacker_address=own_facts.routing?enemy.address:owned.address;
            selected.defender_address=own_facts.routing?owned.address:enemy.address;
            if(!replay_.arm(input,selected)){++replay_errors_;return 1;}
            if(!own_facts.routing)tally173::kill(owner.lease); // kit v10: routed enemy killed by the possessed soldier
            uint32_t returned=calls_.routed152(calls_.context,witness,owned,enemy,selected,nullptr);
            if(!replay_.finish())++replay_errors_;return returned;
        }
        if(!allow_prompt)return reject_selected(input,selected);
        if(calls_.effective_pair){
            if(!calls_.effective_pair(calls_.context,owned.address,enemy.address,own_facts.combat_score,enemy_facts.combat_score))return reject_selected(input,selected);
            own_facts.known|=EffectiveCombat;enemy_facts.known|=EffectiveCombat;
        }
        if(strict_)own_facts.routing=false; // Morale still carries a severe penalty; an admitted FPS exchange cannot bypass its prompt.
        // A missed attack (no click / wrong button) forces this exchange to be a guard.
        const int64_t now_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        const int64_t last_ms=last_exchange_ms.exchange(now_ms);
        const bool rested=last_ms==0||now_ms-last_ms>OutOfCombatMs;
        if(attacks&&(force_guard_next.exchange(0)||rested)){
            std::swap(selected.attacker_address,selected.defender_address);selected.outcome=0;attacks=false;defends=true;
        }else if(defends)force_guard_next.store(0);
        Pair pair{owner.battle,owner.lease,owned.serial,enemy.serial,owned.unit_serial,enemy.unit_serial};
        WitnessKey key{witness,generation,pair};
        Verdict verdict{selected.outcome,attacks?owned.serial:enemy.serial,attacks?enemy.serial:owned.serial};
        acquisition_stage_=10;
        if(prepared_mode_){
            // Reserve the exact selection internally before original prepare;
            // do not start the visible Event until native registration is ready.
            key_=key;input_=input;native_=selected;player_=owned;enemy_=enemy;
            own_facts_=own_facts;enemy_facts_=enemy_facts;invalidate_=false;
            preparing92_=true;preparation_canceled92_=false;preparation_charge92_=charge;
            plans_[0]={owned.address,0,0,owned.kind};plans_[1]={enemy.address,0,0,enemy.kind};
            for(auto& plan:plans_)if(!reader_.get(plan.actor+0x1d0,plan.state)||!reader_.get(plan.actor+0x1d4,plan.clip)){
                preparing92_=false;registration92_.reason_mask=4096;++registration92_.rejected;return reject_selected(input,selected);
            }
            return finish_registration92(cached_prepare(input,selected));
        }
        if(!gate_.acquire(key,verdict,own_facts,enemy_facts,charge))return reject_selected(input,selected);
        acquisition_stage_=11;
        key_=key;input_=input;native_=selected;player_=owned;enemy_=enemy;invalidate_=false;
        own_facts_=own_facts;enemy_facts_=enemy_facts;
        aligning_=calls_.facing_ready&&!calls_.facing_ready(calls_.context,player_.address,enemy_.address);
        // Facing is a native preparation diagnostic, not a timing grace
        // period. Begin movement and accept new input on the first prompt.
        // Pair commitment and post-verdict native alignment remain separate.
        gate_.update(key_,own_facts,enemy_facts,0,paused_);
        edges_.arm(gate_.snapshot().serial,left_,right_);
        return 0;
    }
    uint32_t update(uint32_t witness,uint64_t generation){
        if(preparing92_&&witness==key_.address&&generation==key_.generation)return 0;
        auto key=key_;key.address=witness;key.generation=generation;
        if(gate_.hold_update(key)){
            if(!identities_valid()){canceled92(CancelReason92::NativeIdentity);gate_.removed(witness,generation);return 1;}
            return 0;
        }
        return calls_.update?calls_.update(calls_.context,witness):1;
    }
    // Drain timestamped input BEFORE refreshing facts at a later native tick.
    // dt is the simulation interval from the last processed sample to this
    // click, not the worker callback's wall delay. Stale prompts never advance
    // the current event or alter its armed button levels.
    void input_sample(double dt,uint64_t serial,bool left,bool right,bool paused,bool focused,const Snapshot* visible93=nullptr){
        if(!gate_.occupied()||serial!=gate_.snapshot().serial)return;
        if(!std::isfinite(dt)||dt<0){release(CancelReason92::InvalidInputDelta);return;}
        paused_=paused;left_=left;right_=right;
        gate_.update(key_,own_facts_,enemy_facts_,dt,paused);
        auto edge=edges_.sample(serial,left,right,!paused&&focused);
        if(edge.left){if(visible93)gate_.press_visible93(key_,edge.serial,Button::Left,true,*visible93);else gate_.press(key_,edge.serial,Button::Left,true);}
        if(edge.right){if(visible93)gate_.press_visible93(key_,edge.serial,Button::Right,true,*visible93);else gate_.press(key_,edge.serial,Button::Right,true);}
    }
    void frame(const Owner& owner,double dt,bool paused,bool focused,
               uint64_t input_serial,bool left,bool right){
        paused_=paused;left_=left;right_=right;
        if(!gate_.occupied())return;
        if((owner.battle&&owner.battle!=key_.pair.battle)||
           (owner.environment&&owner.environment!=input_.environment)){
            invalidate_=true;return;
        }
        if(!owner.verified_melee||owner.battle!=key_.pair.battle||owner.lease!=key_.pair.lease||
           !same_actor(owner.actor,player_)){release(CancelReason92::Owner);return;}
        NativeActor player,enemy;UnitState own_facts,enemy_facts;
        if(!actor_identity(reader_,player_.address,key_.address,engine_,player,true)||
           !actor_identity(reader_,enemy_.address,key_.address,engine_,enemy,true)||
           !same_actor(player,player_)||!same_actor(enemy,enemy_))return;
        // Refresh local native heights as well as whole-unit context every tick.
        if(!facts(player,own_facts)||!facts(enemy,enemy_facts)){release(CancelReason92::Facts);return;}
        if(calls_.effective_pair){
            if(!calls_.effective_pair(calls_.context,player.address,enemy.address,own_facts.combat_score,enemy_facts.combat_score)){release(CancelReason92::Score);return;}
            own_facts.known|=EffectiveCombat;enemy_facts.known|=EffectiveCombat;
        }
        own_facts_=own_facts;enemy_facts_=enemy_facts;
        if(!paused&&focused&&calls_.routed152&&(own_facts.routing||enemy_facts.routing)){
            NativeVerdict desired{5,own_facts.routing?enemy.address:player.address,own_facts.routing?player.address:enemy.address};
            if(!replay_.arm(input_,desired)){release(CancelReason92::Facts);return;}
            if(!own_facts.routing)tally173::kill(key_.pair.lease); // kit v10: routed enemy killed by the possessed soldier
            gate_.removed(key_.address,key_.generation);
            routed_answer152_=calls_.routed152(calls_.context,key_.address,player,enemy,desired,prepared_mode_?plans_:nullptr);routed_frame152_=true;
            if(!replay_.finish())++replay_errors_;return;
        }
        // Routing still prevents a NEW encounter. An admitted encounter gets
        // to finish before the unit resumes its native retreat.
        if(calls_.facing_ready)own_facts_.routing=false;
        aligning_=calls_.facing_ready&&!calls_.facing_ready(calls_.context,player_.address,enemy_.address);
        gate_.update(key_,own_facts_,enemy_facts_,dt,paused);
        auto edge=edges_.sample(input_serial,left,right,!paused&&focused);
        if(edge.left)gate_.press(key_,edge.serial,Button::Left,true);
        if(edge.right)gate_.press(key_,edge.serial,Button::Right,true);
    }
    void release(CancelReason92 reason=CancelReason92::External){canceled92(reason);if(preparing92_)preparation_canceled92_=true;gate_.release();}
    // Called while native hooks are still installed. Matching prepare/update
    // returns AL1 so the native manager removes the unprepared witness before
    // its memory is freed or hooks are restored. This is distinct from F10.
    void teardown(){canceled92(CancelReason92::NativeIdentity);invalidate_=true;}
    void removed(uint32_t witness,uint64_t generation){if(gate_.occupied()&&witness==key_.address&&generation==key_.generation)canceled92(CancelReason92::NativeRemoval);if(preparing92_&&witness==key_.address&&generation==key_.generation)preparing92_=false;gate_.removed(witness,generation);}
    Snapshot snapshot()const{auto value=gate_.snapshot();value.aligning=aligning_&&value.active;return value;}
    bool participants(NativeActor& player,NativeActor& enemy)const{
        if(!occupied())return false;player=player_;enemy=enemy_;return true;
    }
    unsigned replay_errors()const{return replay_errors_;}
    uint32_t acquisition_stage()const{return acquisition_stage_;}
    RegistrationStats92 registration_stats92()const{return registration92_;}
    CancellationStats92 cancellation_stats92()const{return cancellation92_;}
    Snapshot interruption93()const{return interrupted93_;}
    uint64_t interruption_lease93()const{return interrupted_lease93_;}
    uint64_t interruption_battle93()const{return interrupted_battle93_;}
};
}
