#include "input174.h"
#include <cassert>
#include <iostream>
using namespace deck174;
static Frame frame177(uint64_t sequence=1){Frame f;f.epoch=1;f.sequence=sequence;f.sample_us=1000000+sequence*8000;f.source=Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.active_buttons=all_buttons;return f;}
int main(){
    unsigned passed=0;
    const uint64_t touch=bit(Button::LeftPadTouch)|bit(Button::RightPadTouch)|bit(Button::LeftStickTouch)|bit(Button::RightStickTouch);
    for(auto b:{Button::LeftPadTouch,Button::RightPadTouch,Button::LeftStickTouch,Button::RightStickTouch}){
        Normalizer n;Edges e;auto f=frame177();f.buttons=bit(b);
        bool ready=n.accept(f,f.sample_us,{},e);
        f=frame177(2);f.buttons=bit(b)|bit(Button::A);bool select=n.accept(f,f.sample_us,{},e);
#ifdef S2UP_EXPECT_OLD_TOUCH177
        assert(!ready&&!select&&!e.down);
#else
        assert(ready&&select&&(e.down&bit(Button::A)));
#endif
        ++passed;
    }
    {Normalizer n;Edges e;auto f=frame177();f.buttons=touch|bit(Button::A);assert(!n.accept(f,f.sample_us,{},e));++passed;}
    {Normalizer n;Edges e;auto f=frame177();f.buttons=touch;f.right_trigger=1;assert(!n.accept(f,f.sample_us,{},e));++passed;}
#ifndef S2UP_EXPECT_OLD_TOUCH177
    {Normalizer n;Edges e;auto f=frame177();f.buttons=touch;assert(n.accept(f,f.sample_us,{},e));f=frame177(2);f.buttons=touch;assert(n.accept(f,f.sample_us,{},e)&&!e.down&&!e.up);++passed;}
    {Normalizer n;Edges e;auto f=frame177();f.buttons=touch;assert(n.accept(f,f.sample_us,{},e));f=frame177(2);f.focused=false;f.buttons=touch|bit(Button::A);assert(!n.accept(f,f.sample_us,{},e));f=frame177(3);f.buttons=touch|bit(Button::A);assert(!n.accept(f,f.sample_us,{},e));f=frame177(4);f.buttons=touch;assert(n.accept(f,f.sample_us,{},e));f=frame177(5);f.buttons=touch|bit(Button::A);assert(n.accept(f,f.sample_us,{},e)&&(e.down&bit(Button::A)));++passed;}
    std::cout<<"PASS "<<passed<<" touch177 corrected arming checks; touches allow input, held physical buttons/triggers still require release\n";
#else
    std::cout<<"PASS "<<passed<<" touch177 OLD-path checks; resting touches prevent every confirm event\n";
#endif
}
