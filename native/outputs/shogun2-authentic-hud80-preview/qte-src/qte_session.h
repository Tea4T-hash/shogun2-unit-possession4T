#pragma once
#include "qte_native_bridge.h"
#include "qte_transport.h"
namespace qte {
// Runtime timing/input coordinator. The platform adapter serializes its state
// and releases that lock around ORIGINAL engine ports; no waits inside hooks.
// Native facts/prepare/update are sampled here only from engine callbacks.
class Session {
    NativeBridge bridge_;Timeline timeline_;ClickQueue<> clicks_;
    TimePoint point_{},anchor_{};Owner owner_{};
    double processed_{};bool initialized_{},clock_verified_{},strict_{},presented_input93_{};
    bool left_{},right_{},focused_{};uint32_t errors_{};
    void advance(const Owner& owner,const ClockFrame& frame,bool focused){
        if(owner.observation_busy93)return; // Torn publication: hold; never reinterpret as a confirmed lease loss.
        auto point=timeline_.observe(frame);focused_=focused;
        if(!point.valid){
            // Invalid clock frames cannot turn a click into a guessed result.
            // Keep the pending pair held; native teardown is a separate signal.
            point_.paused=true;point_.speed=0;return;
        }
        point_=point;
        if(!initialized_||point.battle!=anchor_.battle){
            if(initialized_&&bridge_.occupied())bridge_.teardown();
            processed_=point.simulation;initialized_=true;clicks_.clear();
        }
        if(!bridge_.owner_matches(owner)){
            auto key=bridge_.pending_key();
            if(key.valid()&&owner.battle&&owner.battle!=key.pair.battle)bridge_.teardown();
            else bridge_.release(CancelReason92::Owner);
        }
        // Lost or stale input cannot be scored, but is not a combat event.
        if(clicks_.overflowed()){
            if(!strict_){bridge_.release(CancelReason92::InputOverflow);++errors_;}
            clicks_.clear();
        }
        Click click;
        while(clicks_.pop(click)){
            auto current=bridge_.snapshot();
            if(!current.active||click.battle!=point.battle||click.serial!=current.serial)continue;
            if(!std::isfinite(click.simulation)||click.simulation<processed_-(click.visible93?.025:1e-8)||click.simulation>point.horizon+1e-8){
                if(!strict_){bridge_.release(CancelReason92::ClickTimestamp);++errors_;}
                continue;
            }
            bridge_.input_sample(click.paused?0.:std::max(0.,click.simulation-processed_),click.serial,
                                 click.left,click.right,click.paused,click.focused,click.visible93?&click.shown93:nullptr);
            if(!click.paused)processed_=std::max(processed_,click.simulation);
        }
        // Clicks use the facts governing their displayed window BEFORE native
        // facts refresh at this later callback. Paused intervals charge no dt.
        // Keep pre-pause observed time pending until an unpaused native frame
        // can consume it. Charging processed_ while Event is paused silently
        // discards that interval and makes the projected marker snap backward.
        double now=point.paused?processed_:std::max(processed_,point.simulation);
        bridge_.frame(owner,now-processed_,point.paused,focused,presented_input93_?0:bridge_.snapshot().serial,left_,right_);
        processed_=now;owner_=owner;anchor_=point_;anchor_.simulation=processed_;
        anchor_.horizon=std::max(anchor_.horizon,processed_);
    }
public:
    Session(Reader reader,SelectionCalls selection,BridgeCalls calls,uint32_t engine,
            OutcomeBindings outcomes={},bool clock_verified=false,bool preserve_special=false,bool prepared_mode=false,bool strict=false,bool presented_input93=false):
        bridge_(reader,selection,calls,engine,outcomes,preserve_special,prepared_mode,strict),clock_verified_(clock_verified),strict_(strict),presented_input93_(presented_input93){}
    void levels(bool left,bool right){left_=left;right_=right;}
    bool enqueue(const Click& click){return clicks_.push(click);}
    uint32_t prepare(uint32_t witness,uint64_t generation,Owner owner,const ClockFrame& frame,bool focused,bool allow_prompt=true){
        if(owner.observation_busy93&&bridge_.occupied())return 0;
        if(!clock_verified_)owner.verified_melee=false;
        advance(owner,frame,focused);
        uint32_t routed152{};if(bridge_.take_routed152(witness,generation,routed152))return routed152;
        if(!point_.valid||point_.paused||!focused)owner.verified_melee=false;
        auto before=bridge_.snapshot();
        auto returned=bridge_.prepare(witness,generation,owner,allow_prompt);
        auto after=bridge_.snapshot();
        if(after.active&&after.serial!=before.serial){processed_=point_.simulation;anchor_=point_;owner_=owner;}
        return returned;
    }
    uint32_t update(uint32_t witness,uint64_t generation,Owner owner,const ClockFrame& frame,bool focused){
        if(owner.observation_busy93&&bridge_.occupied())return 0;
        if(!clock_verified_)owner.verified_melee=false;
        advance(owner,frame,focused);uint32_t routed152{};if(bridge_.take_routed152(witness,generation,routed152))return routed152;return bridge_.update(witness,generation);
    }
    bool try_replay(uint32_t witness,uint32_t caller,uint32_t context,uint32_t begin,uint32_t end,
                    uint32_t flags,uint32_t flags2,NativeVerdict& output){
        return bridge_.try_replay(witness,caller,context,begin,end,flags,flags2,output);
    }
    void release(CancelReason92 reason=CancelReason92::External){bridge_.release(reason);clicks_.clear();}
    void teardown(){bridge_.teardown();clicks_.clear();}
    void removed(uint32_t witness,uint64_t generation){bridge_.removed(witness,generation);}
    bool occupied()const{return bridge_.occupied();}
    WitnessKey key()const{return bridge_.pending_key();}
    uint32_t acquisition_stage()const{return bridge_.acquisition_stage();}
    RegistrationStats92 registration_stats92()const{return bridge_.registration_stats92();}
    CancellationStats92 cancellation_stats92()const{return bridge_.cancellation_stats92();}
    Snapshot interruption93()const{return bridge_.interruption93();}
    transport::Payload interruption_publication93(uint32_t pid,double wall)const{
        auto p=publication(pid,wall);p.prompt=bridge_.interruption93();p.lease=bridge_.interruption_lease93();
        p.anchor.battle=bridge_.interruption_battle93();p.occupied=p.enabled=1;return p;
    }
    bool participants(NativeActor& player,NativeActor& enemy)const{return bridge_.participants(player,enemy);}
    // A render/input poll may update PAUSE presentation but never refresh
    // native facts or commit a native animation from a UI/helper thread.
    void display_pause(bool paused){point_.paused=paused;point_.speed=paused?0:point_.speed;}
    // Observe read-only clock transitions during paused UI frames. Native unit
    // facts and the combat event are still advanced only by engine callbacks.
    void poll_clock(const ClockFrame& frame){
        auto observed=timeline_.observe(frame);
        if(!observed.valid)return;
        point_=observed;
        anchor_.paused=observed.paused;anchor_.speed=observed.speed;
        if(observed.paused){anchor_.wall=observed.wall;anchor_.speed=0;}
    }
    transport::Payload publication(uint32_t pid,double wall)const{
        auto prompt=bridge_.snapshot();auto anchor=anchor_;
        // Combat facts advance only on native callbacks. Read-only clock polls
        // can already prove a newer model tick between those callbacks. Publish
        // a projected COPY at that current clock anchor; retaining the previous
        // callback's horizon freezes the marker for most of a late callback's
        // tick. No bridge/event mutation, native call or extra horizon is used.
        if(point_.valid&&point_.battle==anchor.battle){
            const double current=std::max(anchor.simulation,point_.simulation);
            if(prompt.active&&prompt.result==Result::Pending){
                prompt.paused=false;project_response(prompt,current-anchor.simulation);
                prompt.paused=point_.paused;
            }
            anchor=point_;anchor.simulation=current;anchor.horizon=std::max(anchor.horizon,current);
        }
        if(point_.paused){prompt.paused=true;anchor.paused=true;anchor.speed=0;}
        return {pid,clock_verified_?1u:0u,bridge_.occupied()?1u:0u,errors_+bridge_.replay_errors(),
                owner_.lease,prompt,anchor,wall};
    }
};
}
