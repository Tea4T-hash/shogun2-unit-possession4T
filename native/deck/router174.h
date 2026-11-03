#pragma once
#include "input174.h"
#include "ui174.h"

namespace deck174 {
enum class Action : unsigned {
    ClearHeld,Cancel,Pause,Confirm,MenuNavigate,Pointer,SelectBegin,SelectEnd,
    SelectConfirm,CameraPan,CameraRotate,CameraZoom,CameraPitch,FocusSelection,
    Overview,SelectVisible,SelectArmy,SelectType,ClassWheel,TacticalWheel,
    AbilityWheel,AlliedWheel,WeaponWheel,WheelPoint,WheelCommit,GroupPanel,
    GroupPoint,GroupRecall,GroupAssign,GroupPrevious,GroupNext,UnitPrevious,
    UnitNext,Halt,ToggleRun,OrderBegin,OrderUpdate,OrderCommit,OrderCancel,
    PossessionToggle,SoldierMove,Look,Aim,Primary,Secondary,Interact,
    EquipmentSlot,GyroRecenter,ScrollZoom,Count
};
struct Intent {
    Action action{};Context context{};uint64_t timestamp_us{};
    Vec2 value{};bool active{},modifier{};unsigned index{};
};
struct Batch {
    std::array<Intent,32> items{};unsigned count{};bool overflow{};
    void add(Intent v) {if(count<items.size())items[count++]=v;else overflow=true;}
};
// The router produces requests, not permissions. A native adapter must separately
// validate current identity/selection/thread/capability before dispatching one.
class Router {
    Config config_{};Normalizer normalizer_{};Context previous_{Context::Inactive};
    uint64_t previous_us_{},input_epoch_{},consumed_sequence_{},last_buttons_{},l1_at_{},r1_at_{},r2_at_{},r3_at_{};
    uint64_t l4_deadline_{};bool l4_second_{},l1_wheel_{},r1_chord_{},placing_{};
    bool left_trigger_{},right_trigger_{},pad_seed_{};Vec2 previous_pad_{};
    bool l3_run_{},r3_used_{},input_live_{};MotionFilter motion_{};
    void cancel_state() {
        l1_at_=r1_at_=r2_at_=r3_at_=l4_deadline_=last_buttons_=0;
        l4_second_=l1_wheel_=r1_chord_=placing_=left_trigger_=right_trigger_=pad_seed_=l3_run_=r3_used_=false;
        motion_.recenter();
        input_live_=false;
    }
public:
    explicit Router(Config c={}) {if(c.valid())config_=c;}
    bool configure(Config c) {if(!c.valid())return false;config_=c;normalizer_.reset();cancel_state();return true;}
    Batch update(const Frame& f,Facts facts,uint64_t now) {
        Batch out;Context next=context(facts);
        auto emit=[&](Action a,Vec2 v=Vec2{},bool active=true,bool modifier=false,unsigned index=0){
            out.add({a,next,now,v,active,modifier,index});};
        if(next!=previous_) {
            // A wheel or placement consumes the release of the control that
            // opened it. Preserve that input history on entry; returning to
            // gameplay still requires a neutral reacquisition.
            const bool handoff=(next==Context::Radial||next==Context::Targeting)&&
                (previous_==Context::RTS||previous_==Context::Possession||previous_==Context::Campaign)&&input_live_;
            const auto buttons=last_buttons_;const bool lt=left_trigger_,rt=right_trigger_;
            emit(Action::ClearHeld);if(!handoff)normalizer_.reset();cancel_state();previous_=next;
            if(handoff){last_buttons_=buttons;left_trigger_=lt;right_trigger_=rt;input_live_=true;}
            else consumed_sequence_=0;
            previous_us_=now;
        }
        if(f.epoch&&input_epoch_!=f.epoch){input_epoch_=f.epoch;consumed_sequence_=0;emit(Action::ClearHeld);normalizer_.reset();cancel_state();previous_us_=now;}
        // Rendering or another consumer may see the same sample more than once.
        // Ignore an already consumed fresh sample without dropping held intent.
        if(next!=Context::Inactive&&facts.focused&&f.focused&&valid(f,now)&&
            now-f.sample_us<=uint64_t(config_.stale_ms)*1000&&f.sequence<=consumed_sequence_)return out;
        consumed_sequence_=f.sequence;
        Edges e;
        if(!normalizer_.accept(f,now,config_,e)||next==Context::Inactive) {
            if(input_live_||last_buttons_||left_trigger_||right_trigger_||placing_)emit(Action::ClearHeld);
            cancel_state();previous_us_=now;return out;
        }
        input_live_=true;
        const float dt=previous_us_&&now>=previous_us_?float(std::min<uint64_t>(now-previous_us_,50000))/1000000.f:0;
        previous_us_=now;
        auto held=[&](Button b){return (f.buttons&bit(b))!=0;};
        auto down=[&](Button b){return (e.down&bit(b))!=0;};
        auto up=[&](Button b){return (e.up&bit(b))!=0;};
        const bool lt=left_trigger_?f.left_trigger>config_.trigger_release:f.left_trigger>=config_.trigger_press;
        const bool rt=right_trigger_?f.right_trigger>config_.trigger_release:f.right_trigger>=config_.trigger_press;
        const bool ldown=lt&&!left_trigger_,lup=!lt&&left_trigger_;
        const bool rdown=rt&&!right_trigger_,rup=!rt&&right_trigger_;
        if(next==Context::Modal||next==Context::Menu) {
            if(down(Button::B))emit(Action::Cancel);
            if(down(Button::A))emit(Action::Confirm);
            Vec2 nav=deadzone(f.left_stick,config_.left_deadzone);
            if(nav.x||nav.y)emit(Action::MenuNavigate,nav);
        } else if(next==Context::Radial) {
            emit(Action::WheelPoint,deadzone(f.right_stick,config_.right_deadzone));
            if(down(Button::B))emit(Action::Cancel);
            else if(down(Button::A)||lup||up(Button::L1))emit(Action::WheelCommit,f.right_stick);
        } else if(next==Context::Targeting) {
            if(down(Button::B)){emit(Action::OrderCancel);placing_=false;}
            else if(rup)emit(Action::OrderCommit,{},true,held(Button::R1));
            else if(rt)emit(Action::OrderUpdate,f.right_pad);
        } else if(next==Context::RTS||next==Context::Campaign||next==Context::Possession) {
            const bool fps=next==Context::Possession;
            if(down(Button::Menu))emit(Action::Pause);
            if(down(Button::B)){emit(Action::Cancel);if(placing_)emit(Action::OrderCancel);placing_=false;r2_at_=0;}
            if(down(Button::View))emit(Action::Overview);
            if(down(Button::R4)&&next!=Context::Campaign){emit(Action::ClearHeld);emit(Action::PossessionToggle);cancel_state();normalizer_.reset();return out;}
            if(held(Button::LeftPadTouch)) {
                emit(Action::GroupPanel);emit(Action::GroupPoint,f.left_pad);
                if(down(Button::LeftPadClick))emit(held(Button::L5)?Action::GroupAssign:Action::GroupRecall,f.left_pad);
            }
            if(down(Button::Left))emit(Action::GroupPrevious);
            if(down(Button::Right))emit(Action::GroupNext);
            if(down(Button::Up))emit(Action::UnitNext);
            if(down(Button::Down))emit(Action::UnitPrevious);
            if(down(Button::L4)) {
                l4_second_=l4_deadline_&&now<=l4_deadline_;
                if(l4_second_){emit(Action::SelectArmy);l4_deadline_=0;}
            }
            if(up(Button::L4)&&!l4_second_)l4_deadline_=now+uint64_t(config_.double_tap_ms)*1000;
            if(l4_deadline_&&now>l4_deadline_){emit(Action::SelectVisible);l4_deadline_=0;}
            if(down(Button::L1))l1_at_=now;
            if(l1_at_&&held(Button::L1)&&!l1_wheel_&&now-l1_at_>=uint64_t(config_.hold_ms)*1000) {
                emit(fps?Action::AlliedWheel:Action::ClassWheel);l1_wheel_=true;
                last_buttons_=f.buttons;left_trigger_=lt;right_trigger_=rt;return out;
            }
            if(up(Button::L1)) {
                if(!l1_wheel_)emit(Action::SelectType);
                l1_at_=0;l1_wheel_=false;
            }
            if(down(Button::Y)||(!fps&&ldown)) {
                emit(fps?Action::WeaponWheel:down(Button::Y)?Action::AbilityWheel:Action::TacticalWheel);
                last_buttons_=f.buttons;left_trigger_=lt;right_trigger_=rt;return out;
            }
            Vec2 pad_delta{};
            if(held(Button::RightPadTouch)) {
                if(pad_seed_)pad_delta={f.right_pad.x-previous_pad_.x,f.right_pad.y-previous_pad_.y};
                previous_pad_=f.right_pad;pad_seed_=true;
            }else pad_seed_=false;
            if(fps) {
                if(down(Button::L3))l3_run_=!l3_run_;
                emit(Action::SoldierMove,deadzone(f.left_stick,config_.left_deadzone),true,l3_run_);
                Vec2 look=deadzone(f.right_stick,config_.right_deadzone);
                look={look.x*config_.look_sensitivity*dt+pad_delta.x*config_.cursor_sensitivity,
                      look.y*config_.look_sensitivity*dt+pad_delta.y*config_.cursor_sensitivity};
                const bool quiet=!f.buttons&&std::fabs(f.left_stick.x)<=config_.left_deadzone&&
                    std::fabs(f.left_stick.y)<=config_.left_deadzone&&std::fabs(f.right_stick.x)<=config_.right_deadzone&&
                    std::fabs(f.right_stick.y)<=config_.right_deadzone&&f.left_trigger<=config_.trigger_release&&f.right_trigger<=config_.trigger_release;
                const auto gyro=motion_.update(f.gyro,dt,quiet,config_);
                if(config_.gyro_possession&&held(Button::RightPadTouch)&&(f.capabilities&Motion)) {
                    if(std::fabs(gyro.z)>=config_.gyro_deadzone)look.x+=gyro.z*dt*config_.gyro_sensitivity;
                    if(std::fabs(gyro.x)>=config_.gyro_deadzone)look.y+=gyro.x*dt*config_.gyro_sensitivity;
                }
                if(config_.invert_x)look.x=-look.x;if(config_.invert_y)look.y=-look.y;
                if(look.x||look.y)emit(Action::Look,look);
                if(ldown||lup)emit(Action::Aim,{},lt);
                if(rdown||rup)emit(Action::Primary,{},rt);
                if(down(Button::R1)||up(Button::R1))emit(Action::Secondary,{},held(Button::R1));
                if(down(Button::A))emit(Action::Interact);
                if(down(Button::X))emit(Action::EquipmentSlot);
                if(down(Button::R3)){motion_.recenter();emit(Action::GyroRecenter);}
            }else {
                Vec2 pan=deadzone(f.left_stick,config_.left_deadzone);
                if(pan.x||pan.y)emit(Action::CameraPan,pan,true,held(Button::L3));
                Vec2 camera=deadzone(f.right_stick,config_.right_deadzone);
                if(held(Button::R3)) {
                    if(camera.y){emit(Action::CameraPitch,{0,camera.y});r3_used_=true;}
                }else {
                    if(camera.x)emit(Action::CameraRotate,{camera.x,0});
                    if(camera.y)emit(Action::CameraZoom,{0,camera.y});
                }
                if(down(Button::R3)){r3_at_=now;r3_used_=camera.y!=0;}
                if(up(Button::R3)) {
                    if(r3_at_&&!r3_used_&&now-r3_at_<uint64_t(config_.hold_ms)*1000)emit(Action::FocusSelection);
                    r3_at_=0;r3_used_=false;
                }
                if(pad_delta.x||pad_delta.y)emit(Action::Pointer,pad_delta);
                if(down(Button::RightPadClick))emit(Action::SelectBegin,{},true,held(Button::R5));
                if(up(Button::RightPadClick))emit(Action::SelectEnd,{},false,held(Button::R5));
                if(down(Button::A))emit(Action::SelectConfirm,{},true,held(Button::R5));
                if(down(Button::X))emit(Action::Halt);
                if(down(Button::R1)){r1_at_=now;r1_chord_=false;}
                if(rdown) {
                    r2_at_=now;placing_=true;
                    if(held(Button::R1))r1_chord_=true;
                    emit(Action::OrderBegin,{},true,held(Button::R1));
                }
                if(rt&&placing_) {
                    if(held(Button::R1))r1_chord_=true;
                    emit(Action::OrderUpdate,f.right_pad,true,held(Button::R1));
                }
                if(rup&&placing_) {
                    emit(Action::OrderCommit,{},true,r1_chord_);placing_=false;r2_at_=0;
                }
                if(up(Button::R1)) {
                    if(r1_at_&&!r1_chord_)emit(Action::ToggleRun);
                    r1_at_=0;r1_chord_=false;
                }
            }
        }
        last_buttons_=f.buttons;left_trigger_=lt;right_trigger_=rt;return out;
    }
};

struct DispatchGate {
    std::array<bool,unsigned(Action::Count)> verified{};
    // Populated only by an actual engine adapter. Merely selecting an option
    // in a simulated wheel cannot grant authority to call the game.
    bool permits(const Intent& i) const {return unsigned(i.action)<verified.size()&&verified[unsigned(i.action)];}
};
} // namespace deck174
