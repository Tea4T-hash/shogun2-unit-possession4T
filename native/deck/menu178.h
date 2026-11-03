#pragma once
#include "input174.h"
#include "cursor178.h"
#include "click190.h"
namespace deck178 {
// Shared production input policy, independent of Lua/drawing. Host dispatches
// only to the owned foreground game window; no keyboard or system input events.
struct Menu178 {
    deck174::Normalizer normalizer;
    deck174::Vec2 pad{},scroll_pad{};bool seeded{},scroll_seeded{},left{},right{};
    uint64_t previous_us{};deck190::SteadyClick click;bool cancel_triggers{};
    template<class Host>void release(Host& host){
        host.release_owned(left,right);left=right=seeded=scroll_seeded=false;previous_us=0;normalizer.reset();click.reset();cancel_triggers=false;host.reset_cursor();
    }
    template<class Host>bool sample(const deck174::Frame& frame,uint64_t now,const deck174::Config& config,Host& host,bool& back){
        back=false;deck174::Edges edge;
        if(!normalizer.accept(frame,now,config,edge)){release(host);return false;}
        using deck174::Button;auto held=[&](Button b){return (frame.buttons&deck174::bit(b))!=0;};
        if(edge.down&deck174::bit(Button::B)){
            // Keep edge history: resetting it here requires all OTHER controls
            // to return to neutral before B can work again.
            host.release_owned(left,right);left=right=seeded=scroll_seeded=false;
            previous_us=0;click.reset();cancel_triggers=true;host.reset_cursor();back=true;return true;
        }
        float dt=previous_us&&now>=previous_us?float(std::min<uint64_t>(now-previous_us,50000))*.000001f:0;previous_us=now;
        auto stick=deck174::deadzone(frame.left_stick,config.left_deadzone);
        float dx=(stick.x*.55f+float(held(Button::Right)-held(Button::Left))*.3f)*dt;
        float dy=(stick.y*.55f+float(held(Button::Up)-held(Button::Down))*.3f)*dt;
        bool want_left=(left?frame.left_trigger>config.trigger_release:frame.left_trigger>=config.trigger_press)||held(Button::A);
        bool want_right=right?frame.right_trigger>config.trigger_release:frame.right_trigger>=config.trigger_press;
        if(cancel_triggers){if(!want_left&&!want_right)cancel_triggers=false;want_left=want_right=false;}
        const bool pin=want_left&&(held(Button::L3)||held(Button::L5)||held(Button::L4));
        auto movement=click.sample(frame.right_pad,held(Button::RightPadTouch),want_left||want_right,pin);
        dx+=movement.x*.6f;dy+=movement.y*.6f;
        if(!held(Button::RightPadTouch)&&!dx&&!dy)host.reset_cursor();
        if((dx||dy)&&!host.pointer(dx*config.cursor_sensitivity,dy*config.cursor_sensitivity)){release(host);return false;}
        if(held(Button::LeftPadTouch)){
            if(scroll_seeded&&!host.scroll(frame.left_pad.y-scroll_pad.y)){release(host);return false;}
            scroll_pad=frame.left_pad;scroll_seeded=true;
        }else scroll_seeded=false;
        if(want_left!=left){if(!host.button(true,want_left)){release(host);return false;}left=want_left;}
        if(want_right!=right){if(!host.button(false,want_right)){release(host);return false;}right=want_right;}
        back=(edge.down&deck174::bit(Button::B))!=0;return true;
    }
};
}
