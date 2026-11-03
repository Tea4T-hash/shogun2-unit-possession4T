#pragma once
// First-party normalized Deck data. No engine pointers, keyboard emulation or IPC.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace deck174 {
enum class Button : unsigned {
    A, B, X, Y, L1, R1, L3, R3, Up, Down, Left, Right, View, Menu,
    L4, L5, R4, R5, LeftPadTouch, RightPadTouch, LeftPadClick, RightPadClick,
    LeftStickTouch, RightStickTouch, Count
};
constexpr unsigned button_count = unsigned(Button::Count);
constexpr uint64_t bit(Button b) { return uint64_t(1) << unsigned(b); }
constexpr uint64_t all_buttons = (uint64_t(1) << button_count) - 1;
constexpr uint64_t touch_buttons = bit(Button::LeftPadTouch)|bit(Button::RightPadTouch)|
    bit(Button::LeftStickTouch)|bit(Button::RightStickTouch);
enum Capability : uint32_t {
    Sticks=1, Triggers=2, Pads=4, Grips=8, Motion=16, StickTouch=32,
    PadPressure=64, Touchscreen=128
};
enum class Source : uint32_t { None, SteamInput006, Simulation };
struct Vec2 { float x{}, y{}; };
struct Vec3 { float x{}, y{}, z{}; };
struct Frame {
    uint32_t version{174}, bytes{sizeof(Frame)}, capabilities{};
    Source source{Source::None};
    uint64_t sequence{}, epoch{}, sample_us{}, buttons{}, active_buttons{};
    bool connected{}, focused{}, native_bindings{};
    Vec2 left_stick{}, right_stick{}, left_pad{}, right_pad{}, touch{};
    float left_trigger{}, right_trigger{}, left_pressure{}, right_pressure{};
    Vec3 gyro{}, accelerometer{};
};
struct Config {
    float left_deadzone{.12f}, right_deadzone{.10f}, radial_threshold{.35f};
    float trigger_press{.25f}, trigger_release{.15f};
    float look_sensitivity{2.5f}, cursor_sensitivity{1.f}, gyro_sensitivity{1.f};
    float gyro_smoothing{.15f}, gyro_deadzone{.005f};
    float camera_pan_sensitivity{1.f},camera_turn_sensitivity{1.f};
    uint32_t double_tap_ms{250}, hold_ms{350}, overwrite_ms{600}, stale_ms{100};
    bool gyro_rts{}, gyro_possession{true}, invert_x{}, invert_y{},community_rts{};
    bool valid() const {
        const float values[]={left_deadzone,right_deadzone,radial_threshold,trigger_press,
            trigger_release,look_sensitivity,cursor_sensitivity,gyro_sensitivity,
            gyro_smoothing,gyro_deadzone,camera_pan_sensitivity,camera_turn_sensitivity};
        for(float v:values) if(!std::isfinite(v)) return false;
        return left_deadzone>=0&&left_deadzone<.9f&&right_deadzone>=0&&right_deadzone<.9f&&
            radial_threshold>.1f&&radial_threshold<1&&trigger_release>=0&&
            trigger_release<trigger_press&&trigger_press<=1&&look_sensitivity>0&&look_sensitivity<=20&&
            cursor_sensitivity>0&&cursor_sensitivity<=10&&gyro_sensitivity>=0&&gyro_sensitivity<=20&&
            gyro_smoothing>=0&&gyro_smoothing<1&&gyro_deadzone>=0&&gyro_deadzone<1&&
            camera_pan_sensitivity>=.1f&&camera_pan_sensitivity<=4&&camera_turn_sensitivity>=.1f&&camera_turn_sensitivity<=4&&
            double_tap_ms>=100&&double_tap_ms<=500&&hold_ms>=150&&hold_ms<=1000&&
            overwrite_ms>=hold_ms&&overwrite_ms<=2000&&stale_ms>=20&&stale_ms<=250;
    }
};
inline bool finite(Vec2 v) { return std::isfinite(v.x)&&std::isfinite(v.y); }
inline bool finite(Vec3 v) { return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z); }
inline bool unit(Vec2 v) { return finite(v)&&std::fabs(v.x)<=1&&std::fabs(v.y)<=1; }
inline bool valid(const Frame& f,uint64_t now) {
    return f.version==174&&f.bytes==sizeof(Frame)&&f.epoch&&f.sequence&&
        (f.source==Source::SteamInput006||f.source==Source::Simulation)&&
        f.connected&&f.native_bindings&&f.sample_us<=now&&now-f.sample_us<=250000&&
        (f.buttons&~all_buttons)==0&&(f.active_buttons&~all_buttons)==0&&
        (f.buttons&~f.active_buttons)==0&&unit(f.left_stick)&&unit(f.right_stick)&&
        unit(f.left_pad)&&unit(f.right_pad)&&finite(f.gyro)&&finite(f.accelerometer)&&
        std::isfinite(f.left_trigger)&&f.left_trigger>=0&&f.left_trigger<=1&&
        std::isfinite(f.right_trigger)&&f.right_trigger>=0&&f.right_trigger<=1&&
        std::isfinite(f.left_pressure)&&f.left_pressure>=0&&f.left_pressure<=1&&
        std::isfinite(f.right_pressure)&&f.right_pressure>=0&&f.right_pressure<=1;
}
inline Vec2 deadzone(Vec2 v,float dz) {
    const float length=std::sqrt(v.x*v.x+v.y*v.y);
    if(!finite(v)||length<=dz||dz<0||dz>=1) return {};
    const float scale=(std::min(length,1.f)-dz)/(1.f-dz)/length;
    return {v.x*scale,v.y*scale};
}
struct Edges { uint64_t down{},up{}; bool left_fire{},right_fire{}; };
// Input must return to neutral after a context/device transition. Held triggers
// cannot become an accidental shot or RTS order in the new context.
class Normalizer {
    uint64_t sequence_{},epoch_{},held_{};
    bool ready_{},lt_{},rt_{};
public:
    void reset() { sequence_=epoch_=held_=0;ready_=lt_=rt_=false; }
    bool accept(const Frame& f,uint64_t now,const Config& c,Edges& e) {
        e={};
        if(!c.valid()||!valid(f,now)||!f.focused||now-f.sample_us>uint64_t(c.stale_ms)*1000) {
            e.up=held_;reset();return false;
        }
        if(f.epoch!=epoch_) { reset();epoch_=f.epoch; }
        if(f.sequence<=sequence_) return false;
        sequence_=f.sequence;
        if(!ready_) {
            // Resting a thumb is not a held command. Keeping capacitive touch
            // in this gate prevented native input from ever arming normally.
            if((f.buttons&~touch_buttons)||f.left_trigger>c.trigger_release||f.right_trigger>c.trigger_release) return false;
            held_=f.buttons;ready_=true;return true;
        }
        e.down=f.buttons&~held_;e.up=held_&~f.buttons;held_=f.buttons;
        bool l=lt_?f.left_trigger>c.trigger_release:f.left_trigger>=c.trigger_press;
        bool r=rt_?f.right_trigger>c.trigger_release:f.right_trigger>=c.trigger_press;
        e.left_fire=l&&!lt_;e.right_fire=r&&!rt_;lt_=l;rt_=r;
        return true;
    }
};
class MotionFilter {
    Vec3 bias_{},sum_{},filtered_{};unsigned samples_{};bool seeded_{};
public:
    void recenter(){bias_={};sum_={};filtered_={};samples_=0;seeded_=false;}
    Vec3 update(Vec3 raw,float dt,bool calibrate,const Config& c) {
        if(!finite(raw)||dt<0||!std::isfinite(dt)||dt>.25f){seeded_=false;return {};}
        if(calibrate&&samples_<100&&std::fabs(raw.x)<.05f&&std::fabs(raw.y)<.05f&&std::fabs(raw.z)<.05f) {
            sum_.x+=raw.x;sum_.y+=raw.y;sum_.z+=raw.z;++samples_;
            if(samples_==100){bias_={sum_.x/100,sum_.y/100,sum_.z/100};filtered_={};seeded_=false;}
        }
        Vec3 value{raw.x-bias_.x,raw.y-bias_.y,raw.z-bias_.z};
        float retained=seeded_?std::pow(c.gyro_smoothing,std::max(dt,.000001f)*125.f):0;
        filtered_={filtered_.x*retained+value.x*(1-retained),
            filtered_.y*retained+value.y*(1-retained),filtered_.z*retained+value.z*(1-retained)};
        seeded_=true;return filtered_;
    }
};
enum class Context : unsigned { Neutral,Campaign,RTS,Possession,Targeting,Radial,Menu,Modal,Inactive };
struct Facts {
    bool focused{},device{},modal{},menu{},radial{},targeting{},possessed{},battle{},campaign{};
};
inline Context context(Facts f) {
    if(!f.focused||!f.device) return Context::Inactive;
    if(f.modal) return Context::Modal;
    if(f.menu) return Context::Menu;
    if(f.radial) return Context::Radial;
    if(f.targeting) return Context::Targeting;
    if(f.possessed) return Context::Possession;
    if(f.battle) return Context::RTS;
    if(f.campaign) return Context::Campaign;
    return Context::Neutral;
}
} // namespace deck174
