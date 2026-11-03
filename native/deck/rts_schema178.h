#pragma once
#include "router174.h"
#include "click190.h"
namespace deck174 {
struct CameraAxes178 {Vec2 pan{};float turn{},pitch{};bool fast{};};
inline CameraAxes178 camera_axes178(const Frame& f,const Config& c){
 auto held=[&](Button b){return(f.buttons&bit(b))!=0;};
 auto stick=deadzone(f.right_stick,c.right_deadzone);
 return {deadzone(f.left_stick,c.left_deadzone),
  std::clamp(stick.x+float(held(Button::R1))-float(held(Button::L1)),-1.f,1.f),stick.y,held(Button::L3)};
}
// Camera feel remains based on the accepted Allyn-derived implementation.
// Buttons now follow the owner's complete table: RT selects, LT orders,
// L4 ranges/queues, L5 adds/assigns. Bumpers no longer add camera yaw.
class RtsRouter178 {
    Config config_;Normalizer normalizer_;uint64_t buttons_{};deck190::SteadyClick click_;
    bool selecting_{},selection_additive_{},ordering_{},left_{},right_{},pad_seed_{},zoom_seed_{};
    Vec2 pad_{},zoom_{};uint64_t order_released_{},order_started_{};Vec2 order_origin_{};bool order_dragged_{};
public:
    explicit RtsRouter178(Config value={}):config_(value){}
    Batch update(const Frame& f,uint64_t now){
        Batch batch;auto emit=[&](Action a,Vec2 v=Vec2{},bool modifier=false){batch.add({a,Context::RTS,now,v,true,modifier,0});};
        Edges edge;
        if(!normalizer_.accept(f,now,config_,edge)){
            if(selecting_||ordering_)emit(Action::ClearHeld);
            click_.reset();order_released_=order_started_=0;order_dragged_=false;buttons_=0;selecting_=selection_additive_=ordering_=left_=right_=pad_seed_=zoom_seed_=false;return batch;
        }
        auto held=[&](Button b){return (f.buttons&bit(b))!=0;};
        auto down=[&](Button b){return (edge.down&bit(b))!=0;};
        const bool left=left_?f.left_trigger>config_.trigger_release:f.left_trigger>=config_.trigger_press;
        const bool right=right_?f.right_trigger>config_.trigger_release:f.right_trigger>=config_.trigger_press;
        const bool select=right; // Right pad click is intentionally unassigned.
        const bool additive=held(Button::L5)||held(Button::L3);
        if(down(Button::B)){
            if(selecting_||ordering_)emit(Action::OrderCancel);
            click_.reset();selecting_=ordering_=false;order_released_=0;
            // A held trigger must not restart a cancelled gesture.
            left_=left;right_=right;buttons_=f.buttons;return batch;
        }
        auto pointer=click_.sample(f.right_pad,held(Button::RightPadTouch),select||left,select&&(additive||held(Button::L4)));
        if(pointer.x||pointer.y)emit(Action::Pointer,pointer);
        const bool previous_select=right_;
        if(select&&!previous_select&&!selecting_){
            if(ordering_)emit(Action::OrderCancel);ordering_=false;
            emit(Action::SelectBegin,{},additive);selecting_=true;selection_additive_=additive;
        }else if(!select&&selecting_){emit(Action::SelectEnd,{},selection_additive_);selecting_=selection_additive_=false;}

        if(left&&!left_&&!selecting_){
            const bool twice=order_released_&&now>=order_released_&&now-order_released_<=uint64_t(config_.double_tap_ms)*1000;
            batch.add({Action::OrderBegin,Context::RTS,now,{},true,held(Button::L4),twice?1u:0u});ordering_=true;order_started_=now;order_origin_=f.right_pad;order_dragged_=false;order_released_=0;
        }else if(left&&ordering_){emit(Action::OrderUpdate,{},held(Button::L4));if(std::fabs(f.right_pad.x-order_origin_.x)>.02f||std::fabs(f.right_pad.y-order_origin_.y)>.02f)order_dragged_=true;}
        else if(!left&&ordering_){emit(Action::OrderCommit,{},held(Button::L4));ordering_=false;order_released_=!order_dragged_&&now>=order_started_&&now-order_started_<=uint64_t(config_.hold_ms)*1000?now:0;}
        auto camera_frame=f;camera_frame.buttons&=~(bit(Button::L1)|bit(Button::R1)|bit(Button::L3));
        const auto camera=camera_axes178(camera_frame,config_);
        if(camera.pan.x||camera.pan.y)emit(Action::CameraPan,camera.pan,camera.fast);
        if(camera.turn)emit(Action::CameraRotate,{camera.turn,0});
        if(camera.pitch)emit(Action::CameraPitch,{0,camera.pitch});
        const float shoulder_zoom=float(held(Button::L1))-float(held(Button::R1));
        if(shoulder_zoom)emit(Action::CameraZoom,{0,shoulder_zoom});
        if(held(Button::LeftPadTouch)){
            if(zoom_seed_){float delta=f.left_pad.y-zoom_.y;if(delta)emit(Action::ScrollZoom,{0,delta});}
            zoom_=f.left_pad;zoom_seed_=true;
        }else zoom_seed_=false;
        left_=left;right_=right;buttons_=f.buttons;return batch;
    }
};
}
