#pragma once
#include "input174.h"

namespace deck174 {
struct Range {
    float minimum{std::numeric_limits<float>::max()},maximum{std::numeric_limits<float>::lowest()};
    unsigned samples{},intermediate{};
    void add(float value){if(!std::isfinite(value))return;minimum=std::min(minimum,value);maximum=std::max(maximum,value);++samples;if(value>.2f&&value<.7f)++intermediate;}
    bool bipolar()const{return samples&&minimum<-.6f&&maximum>.6f;}
    bool trigger()const{return samples&&minimum<.1f&&maximum>.8f&&intermediate>2;}
};
struct Proof {
    unsigned frames{},rejected{},connections{},focused_frames{},unfocused_frames{};uint64_t epoch{},pressed{},independent_grips{};
    std::array<unsigned,button_count> presses{};uint64_t previous_buttons{};
    std::array<Range,10> axes{};std::array<Range,3> gyro{},accelerometer{};
    void observe(const Frame& f,uint64_t now) {
        if(!valid(f,now)){++rejected;previous_buttons=0;return;}
        if(epoch!=f.epoch){epoch=f.epoch;++connections;previous_buttons=0;}
        ++frames;uint64_t down=f.buttons&~previous_buttons;previous_buttons=f.buttons;pressed|=f.buttons;
        if(f.focused)++focused_frames;else ++unfocused_frames;
        for(unsigned i=0;i<button_count;++i)if(down&(uint64_t(1)<<i))++presses[i];
        constexpr uint64_t grips=bit(Button::L4)|bit(Button::L5)|bit(Button::R4)|bit(Button::R5);
        uint64_t active=f.buttons&grips;
        if(active&&(active&(active-1))==0)independent_grips|=active;
        const float v[]={f.left_stick.x,f.left_stick.y,f.right_stick.x,f.right_stick.y,
            f.left_pad.x,f.left_pad.y,f.right_pad.x,f.right_pad.y,f.left_trigger,f.right_trigger};
        for(unsigned i=0;i<10;++i)if(i<4||i>=8||(i<6&&(f.buttons&bit(Button::LeftPadTouch)))||
            (i>=6&&i<8&&(f.buttons&bit(Button::RightPadTouch))))axes[i].add(v[i]);
        gyro[0].add(f.gyro.x);gyro[1].add(f.gyro.y);gyro[2].add(f.gyro.z);
        accelerometer[0].add(f.accelerometer.x);accelerometer[1].add(f.accelerometer.y);accelerometer[2].add(f.accelerometer.z);
    }
    bool required_inputs_observed()const {
        const uint64_t required=all_buttons&~(bit(Button::LeftStickTouch)|bit(Button::RightStickTouch));
        if((pressed&required)!=required)return false;
        constexpr uint64_t grips=bit(Button::L4)|bit(Button::L5)|bit(Button::R4)|bit(Button::R5);
        if((independent_grips&grips)!=grips)return false;
        for(unsigned i=0;i<8;++i)if(!axes[i].bipolar())return false;
        if(!axes[8].trigger()||!axes[9].trigger())return false;
        bool motion=false;for(auto& r:gyro)if(r.samples&&r.maximum-r.minimum>.08f)motion=true;
        return frames>=100&&motion;
    }
};
} // namespace deck174
