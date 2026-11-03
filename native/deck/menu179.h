#pragma once
#include "input174.h"
#include "cursor178.h"
namespace deck179 {
// Shared production input policy, independent of Lua/drawing. Host dispatches
// only to the owned foreground game window; no keyboard or system input events.
struct Menu179 {
    deck174::Normalizer normalizer;
    deck174::Vec2 pad{},scroll_pad{};bool seeded{},scroll_seeded{},left{},right{};
    uint64_t previous_us{},repeat_at{};int direction{};
    template<class Host>void release(Host& host){
        host.release_owned(left,right);left=right=seeded=scroll_seeded=false;previous_us=0;normalizer.reset();host.reset_cursor();direction=0;repeat_at=0;host.clear_navigation();
    }
    template<class Host>bool sample(const deck174::Frame& frame,uint64_t now,const deck174::Config& config,Host& host,bool& back){
        back=false;deck174::Edges edge;
        if(!normalizer.accept(frame,now,config,edge)){release(host);return false;}
        using deck174::Button;auto held=[&](Button b){return (frame.buttons&deck174::bit(b))!=0;};
        if(edge.down&deck174::bit(Button::B)){release(host);back=true;return true;}
        previous_us=now;
        auto stick=deck174::deadzone(frame.left_stick,config.left_deadzone);
        int next=held(Button::Up)?1:held(Button::Down)?2:held(Button::Left)?3:held(Button::Right)?4:0;
        if(!next&&std::max(std::fabs(stick.x),std::fabs(stick.y))>.6f)
            next=std::fabs(stick.y)>=std::fabs(stick.x)?(stick.y>0?1:2):(stick.x<0?3:4);
        if(next&&(next!=direction||now>=repeat_at)){
            host.navigate(next);repeat_at=now+(next!=direction?350000:140000);
        }direction=next;if(!next)repeat_at=0;
        float dx=0,dy=0; // directional focus owns stick/D-pad; pad remains pointer
        if(held(Button::RightPadTouch)){
            if(seeded){dx+=(frame.right_pad.x-pad.x)*.6f;dy+=(frame.right_pad.y-pad.y)*.6f;}
            pad=frame.right_pad;seeded=true;
        }else {seeded=false;if(!dx&&!dy)host.reset_cursor();}
        if((dx||dy)&&!host.pointer(dx*config.cursor_sensitivity,dy*config.cursor_sensitivity)){release(host);return false;}
        if(held(Button::LeftPadTouch)){
            if(scroll_seeded&&!host.scroll(frame.left_pad.y-scroll_pad.y)){release(host);return false;}
            scroll_pad=frame.left_pad;scroll_seeded=true;
        }else scroll_seeded=false;
        bool want_left=(left?frame.left_trigger>config.trigger_release:frame.left_trigger>=config.trigger_press)||held(Button::A);
        bool want_right=right?frame.right_trigger>config.trigger_release:frame.right_trigger>=config.trigger_press;
        if(want_left&&!left&&host.navigation_pending())want_left=false;
        if(want_left!=left){if(!host.button(true,want_left)){release(host);return false;}left=want_left;}
        if(want_right!=right){if(!host.button(false,want_right)){release(host);return false;}right=want_right;}
        back=(edge.down&deck174::bit(Button::B))!=0;return true;
    }
};
}
