#pragma once
#include "router174.h"
#include "rts_schema178.h"
#include "shoulder_zoom195.h"

namespace deck175 {
using deck174::Vec2;using deck174::Action;using deck174::Intent;using deck174::Context;
struct Identity {
    uintptr_t root{},environment{},common{},phase{},manager{},camera{};
    bool operator==(const Identity& other)const {
        return root==other.root&&environment==other.environment&&common==other.common&&
            phase==other.phase&&manager==other.manager&&camera==other.camera;
    }
};
struct ContextProof {
    Identity identity{};bool focused{},owner_thread{},rts{},blocked{},possessed{};
    bool permits_rts()const{return focused&&owner_thread&&rts&&!blocked&&!possessed&&
        identity.root&&identity.environment&&identity.common&&identity.phase&&identity.manager&&identity.camera;}
};
struct Host {
    virtual bool current(const Identity&)=0;
    virtual void cancel_owned(const Identity&)=0;
    virtual bool pointer(Vec2)=0;
    virtual void reset_pointer(){}
    virtual void reset_clicks(){}
    virtual bool scroll(float){return false;}
    virtual void modifiers(bool,bool){}
    virtual bool select(bool,bool)=0;
    virtual bool order(unsigned,bool)=0; // 0 begin, 1 update, 2 commit
    virtual bool halt()=0;
    virtual bool camera(Vec2,float,float,float,float)=0;
    virtual ~Host()=default;
};
// Preserve input timestamps for button/gesture arbitration; apply continuous
// camera intent once per actual game callback, capped after a stall. Every
// discrete game call separately validates the borrowed native identity.
class Rts {
    deck174::Config config_{};deck174::Router router_{};Identity identity_{};bool live_{},selecting_{},ordering_{};
    deck174::RtsRouter178 community_{};deck195::ShoulderZoom shoulder_zoom_{};
    Vec2 pan_{};float rotate_{},zoom_{},pitch_{};bool fast_{};
    uint64_t callback_us_{};bool have_axes_{};
    void cancel(Host& host){
        host.reset_pointer();host.reset_clicks();shoulder_zoom_.reset();
        if(live_&&(selecting_||ordering_))host.cancel_owned(identity_);
        selecting_=ordering_=live_=have_axes_=false;pan_={};rotate_=zoom_=pitch_=0;fast_=false;
    }
public:
    unsigned rejected{},unsupported{},committed{},cancelled{};
    void configure(deck174::Config config){if(config.valid()){shoulder_zoom_.reset();config_=config;router_.configure(config);community_=deck174::RtsRouter178{config};}}
    void reset(Host& host){cancel(host);router_=deck174::Router{config_};community_=deck174::RtsRouter178{config_};callback_us_=0;++cancelled;}
    void begin(const ContextProof& proof,Host& host,uint64_t now) {
        if(!proof.permits_rts()||(live_&&!(identity_==proof.identity))){reset(host);return;}
        if(!live_){identity_=proof.identity;live_=true;callback_us_=now;}
    }
    void sample(const deck174::Frame& f,const ContextProof& proof,Host& host,uint64_t now) {
        if(!proof.permits_rts()||!live_||!(identity_==proof.identity)||!host.current(identity_)||
            !deck174::valid(f,now)||now-f.sample_us>uint64_t(config_.stale_ms)*1000){++rejected;reset(host);return;}
        if(config_.community_rts)host.modifiers((f.buttons&deck174::bit(deck174::Button::L4))!=0,(f.buttons&(deck174::bit(deck174::Button::L5)|deck174::bit(deck174::Button::L3)))!=0);
        deck174::Facts facts;facts.focused=facts.device=facts.battle=true;
        // Router timestamps use the sample's acquisition time. Checking against
        // the real callback time above prevents a queued old command from firing.
        if(!(f.buttons&deck174::bit(deck174::Button::RightPadTouch)))host.reset_pointer();
        auto batch=config_.community_rts?community_.update(f,f.sample_us):router_.update(f,facts,f.sample_us);
        if(batch.overflow){++rejected;reset(host);return;}
        pan_={};rotate_=zoom_=pitch_=0;fast_=false;have_axes_=true;
        for(unsigned i=0;i<batch.count;++i){const auto& intent=batch.items[i];
            if(!host.current(identity_)){++rejected;reset(host);return;}
            switch(intent.action){
            case Action::ClearHeld:
                shoulder_zoom_.reset();
                if(selecting_||ordering_)host.cancel_owned(identity_);
                selecting_=ordering_=false;break;
            case Action::CameraPan:pan_=intent.value;fast_=intent.modifier;break;
            case Action::CameraRotate:rotate_=intent.value.x;break;
            case Action::CameraZoom:zoom_=intent.value.y;break;
            case Action::CameraPitch:pitch_=intent.value.y;break;
            case Action::Pointer:if(!host.pointer(intent.value)){++rejected;reset(host);return;}break;
            case Action::ScrollZoom:if(!host.scroll(intent.value.y)){++rejected;reset(host);return;}break;
            case Action::SelectBegin:
                if(ordering_){host.cancel_owned(identity_);ordering_=false;}
                selecting_=host.select(true,intent.modifier);if(!selecting_){++rejected;reset(host);return;}break;
            case Action::SelectEnd:
                if(selecting_){if(!host.select(false,intent.modifier)){++rejected;reset(host);return;}selecting_=false;}break;
            case Action::SelectConfirm:
                if(!ordering_&&!selecting_&&host.select(true,intent.modifier)){
                    selecting_=true;
                    if(!host.current(identity_)||!host.select(false,intent.modifier)){++rejected;reset(host);return;}
                    selecting_=false;
                }break;
            case Action::OrderBegin:
                if(selecting_){host.cancel_owned(identity_);selecting_=false;}
                // Queued orders need their separate verified native modifier
                // path. Until then reject the chord; never silently make it a
                // replacing order or also fire the run/walk action.
                if(intent.modifier&&!config_.community_rts){++unsupported;break;}
                ordering_=host.order(intent.index?3:0,intent.modifier);break;
            case Action::OrderUpdate:if(ordering_&&!host.order(1,intent.modifier)){++rejected;reset(host);return;}break;
            case Action::OrderCommit:
                if(ordering_){if(intent.modifier&&!config_.community_rts){host.cancel_owned(identity_);++unsupported;}
                    else if(host.order(2,intent.modifier))++committed;
                    else{++rejected;reset(host);return;}ordering_=false;}break;
            case Action::Cancel:case Action::OrderCancel:
                host.reset_clicks();
                if(selecting_||ordering_){host.cancel_owned(identity_);++cancelled;}
                selecting_=ordering_=false;break;
            case Action::Halt:
                host.reset_clicks();
                if(selecting_||ordering_)host.cancel_owned(identity_);
                selecting_=ordering_=false;if(!host.halt())++rejected;break;
            case Action::ClassWheel:case Action::TacticalWheel:case Action::AbilityWheel:
                ++unsupported;reset(host);return;
            default:++unsupported;break;
            }
        }
        // A release sampled between callbacks must still restart the ramp.
        if(config_.community_rts&&!zoom_)shoulder_zoom_.reset();
    }
    void finish(Host& host,uint64_t now) {
        if(!live_||!host.current(identity_)||!have_axes_){shoulder_zoom_.reset();callback_us_=now;return;}
        const float dt=callback_us_&&now>=callback_us_?float(std::min<uint64_t>(now-callback_us_,50000))/1000000.f:0;
        if(callback_us_&&(now<callback_us_||now-callback_us_>100000))shoulder_zoom_.reset();
        callback_us_=now;have_axes_=false;
        if(config_.community_rts)zoom_=shoulder_zoom_.step(zoom_,dt);
        if(dt&&(pan_.x||pan_.y||rotate_||zoom_||pitch_))
            if(!host.camera({pan_.x*(fast_?2.f:1.f),pan_.y*(fast_?2.f:1.f)},rotate_,zoom_,pitch_,dt)){++rejected;reset(host);}
    }
};
}
