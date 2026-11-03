#include "../../native/deck/cursor178.h"
#include "../../native/deck/menu178.h"
#include "../../native/deck/rts_schema178.h"
#include "../../native/deck/config174.h"
#include <cassert>
#include <iostream>
#include <vector>
using namespace deck174;
static unsigned passed;
#define TEST(label,...) do{__VA_ARGS__;++passed;std::cout<<"PASS " label "\n";}while(0)
struct Host {
    deck178::Cursor cursor;int x{10000},y{10000};unsigned downs{},ups{},right_downs{},right_ups{},cancels{},scrolls{};
    bool pointer(float dx,float dy){return cursor.move(x,y,32767,32767,dx*1280,-dy*800);}
    void reset_cursor(){cursor.reset();}
    bool button(bool left,bool down){if(left){if(down)++downs;else ++ups;}else{if(down)++right_downs;else ++right_ups;}return true;}
    bool scroll(float){++scrolls;return true;}
    void release_owned(bool left,bool right){if(left||right)++cancels;}
};
static Frame frame(uint64_t sequence=1){Frame f;f.sample_us=1000000+sequence*8000;f.sequence=sequence;f.epoch=1;f.source=Source::Simulation;f.active_buttons=all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
static bool has(const Batch& b,Action a){for(unsigned i=0;i<b.count;++i)if(b.items[i].action==a)return true;return false;}
int main(){
    TEST("old absolute rounding reproduces stationary up-left drift",{int x=10000,y=10000;for(int i=0;i<1000;++i){double jitter=i%2?.05:-.05;x=int(x+jitter);y=int(y+jitter);}assert(x==9500&&y==9500);});
    TEST("resting-thumb jitter has no directional drift",{deck178::Cursor c;int x=10000,y=10000;for(int i=0;i<100000;++i){double j=i%2?.05:-.05;assert(c.move(x,y,32767,32767,j,j));}assert(x==10000&&y==10000);});
    TEST("slow positive and negative movements retain equal precision",{deck178::Cursor a,b;int x=10000,y=10000,u=x,v=y;for(int i=0;i<1000;++i){a.move(x,y,32767,32767,.125,.125);b.move(u,v,32767,32767,-.125,-.125);}assert(x-10000==10000-u&&y-10000==10000-v&&x==10125);});
    TEST("cursor clamp clears outward remainder",{deck178::Cursor c;int x=0,y=0;for(int i=0;i<100;++i)c.move(x,y,1280,800,-.2,-.2);assert(x==0&&y==0&&c.remainder_x==0&&c.remainder_y==0);c.move(x,y,1280,800,1,1);assert(x==1&&y==1);});
    TEST("bad cursor data is rejected and residue cleared",{deck178::Cursor c;int x=10,y=10;c.move(x,y,100,100,.5,.5);assert(!c.move(x,y,100,100,NAN,0)&&c.remainder_x==0&&x==10);});
    TEST("native tick scaling is independent of callback and display rate",{for(int fps:{30,60,120,240})for(int hz:{30,60,125,250}){double sum=0;float t=1.f/fps;for(int i=0;i<hz;++i){float r;assert(deck178::camera_ticks(1.f/hz,t,r));sum+=r*t;}assert(std::fabs(sum-1)<.0001);}});
    TEST("native camera refuses invalid timing",{float r;assert(!deck178::camera_ticks(.016f,0,r)&&!deck178::camera_ticks(NAN,.016f,r)&&!deck178::camera_ticks(.1f,.016f,r));});
    TEST("menu LT selection does not depend on a UI callback",{deck178::Menu178 m;Host h;bool back;auto f=frame();assert(m.sample(f,f.sample_us,{},h,back));f=frame(2);f.left_trigger=1;m.sample(f,f.sample_us,{},h,back);f=frame(3);m.sample(f,f.sample_us,{},h,back);assert(h.downs==1&&h.ups==1);});
    TEST("menu A selects and pad click stays unassigned",{deck178::Menu178 m;Host h;bool back;for(unsigned i=1;i<=5;++i){auto f=frame(i);if(i==2)f.buttons=bit(Button::A);if(i==3)f.buttons=bit(Button::A)|bit(Button::RightPadClick);if(i==4)f.buttons=bit(Button::RightPadClick);m.sample(f,f.sample_us,{},h,back);}assert(h.downs==1&&h.ups==1);});
    TEST("menu focus loss cancels without activating or rearming held LT",{deck178::Menu178 m;Host h;bool back;auto f=frame();m.sample(f,f.sample_us,{},h,back);f=frame(2);f.left_trigger=1;m.sample(f,f.sample_us,{},h,back);f=frame(3);f.focused=false;f.left_trigger=1;m.sample(f,f.sample_us,{},h,back);f=frame(4);f.left_trigger=1;assert(!m.sample(f,f.sample_us,{},h,back));assert(h.downs==1&&h.cancels==1&&h.ups==0);});
    TEST("zero trigger-release setting still releases",{deck178::Menu178 m;Host h;bool back;Config c;c.trigger_release=0;for(unsigned i=1;i<=3;++i){auto f=frame(i);if(i==2)f.right_trigger=1;m.sample(f,f.sample_us,c,h,back);}assert(h.right_downs==1&&h.right_ups==1);});
    TEST("back cancels a held gesture once",{deck178::Menu178 m;Host h;bool back;auto f=frame();m.sample(f,f.sample_us,{},h,back);f=frame(2);f.left_trigger=1;m.sample(f,f.sample_us,{},h,back);f=frame(3);f.left_trigger=1;f.buttons=bit(Button::B);m.sample(f,f.sample_us,{},h,back);assert(back&&h.cancels==1);f=frame(4);f.left_trigger=1;m.sample(f,f.sample_us,{},h,back);assert(!back&&h.downs==1);});
    TEST("RTS uses owner rebound triggers without bumper camera leaks",{RtsRouter178 r;r.update(frame(),1008000);auto f=frame(2);f.right_trigger=1;f.buttons=bit(Button::R1);auto b=r.update(f,f.sample_us);assert(has(b,Action::SelectBegin)&&!has(b,Action::CameraRotate)&&!has(b,Action::TacticalWheel));f=frame(3);b=r.update(f,f.sample_us);assert(has(b,Action::SelectEnd));f=frame(4);f.left_trigger=1;b=r.update(f,f.sample_us);assert(has(b,Action::OrderBegin));f=frame(5);b=r.update(f,f.sample_us);assert(has(b,Action::OrderCommit));});
    TEST("RTS pad click ignored while R2 selects and cancels",{RtsRouter178 r;r.update(frame(),1008000);auto f=frame(2);f.buttons=bit(Button::RightPadClick);assert(!has(r.update(f,f.sample_us),Action::SelectBegin));f=frame(3);f.buttons=bit(Button::RightPadClick);f.right_trigger=1;assert(has(r.update(f,f.sample_us),Action::SelectBegin));f=frame(4);f.buttons=bit(Button::B);f.right_trigger=1;assert(has(r.update(f,f.sample_us),Action::OrderCancel));f=frame(5);f.right_trigger=1;assert(!has(r.update(f,f.sample_us),Action::SelectBegin));});
    TEST("left pad scroll has discrete native zoom intent",{RtsRouter178 r;r.update(frame(),1008000);auto f=frame(2);f.buttons=bit(Button::LeftPadTouch);r.update(f,f.sample_us);f=frame(3);f.buttons=bit(Button::LeftPadTouch);f.left_pad.y=.2f;auto b=r.update(f,f.sample_us);assert(has(b,Action::ScrollZoom)&&!has(b,Action::GroupPanel));});
    TEST("community settings are bounded and optional",{Config c;std::string error;assert(!c.community_rts&&parse_config("community_rts=1\ncamera_pan_sensitivity=1.0\ncamera_turn_sensitivity=1.0",c,error)&&c.community_rts);assert(!parse_config("camera_pan_sensitivity=500",c,error));});
    TEST("multi-select pins chosen point until RT release and retains Ctrl after L3 release",{RtsRouter178 r;auto f=frame();r.update(f,f.sample_us);f=frame(2);f.buttons=bit(Button::RightPadTouch)|bit(Button::L3);r.update(f,f.sample_us);f=frame(3);f.buttons=bit(Button::RightPadTouch)|bit(Button::L3);f.right_trigger=1;f.right_pad={.1f,.1f};auto b=r.update(f,f.sample_us);assert(has(b,Action::SelectBegin)&&!has(b,Action::Pointer));f=frame(4);f.buttons=bit(Button::RightPadTouch);f.right_trigger=1;f.right_pad={.8f,.8f};b=r.update(f,f.sample_us);assert(!has(b,Action::Pointer));f=frame(5);f.buttons=bit(Button::RightPadTouch);f.right_pad={.8f,.8f};b=r.update(f,f.sample_us);assert(has(b,Action::SelectEnd)&&!has(b,Action::Pointer));for(unsigned i=0;i<b.count;++i)if(b.items[i].action==Action::SelectEnd)assert(b.items[i].modifier);f=frame(6);f.buttons=bit(Button::RightPadTouch);f.right_pad={.81f,.81f};b=r.update(f,f.sample_us);assert(has(b,Action::Pointer));});
    TEST("ordinary RT box selection still permits pad movement",{RtsRouter178 r;auto f=frame();r.update(f,f.sample_us);f=frame(2);f.buttons=bit(Button::RightPadTouch);r.update(f,f.sample_us);f=frame(3);f.buttons=bit(Button::RightPadTouch);f.right_trigger=1;f.right_pad={.1f,.1f};auto b=r.update(f,f.sample_us);assert(has(b,Action::SelectBegin)&&!has(b,Action::Pointer));f=frame(4);f.buttons=bit(Button::RightPadTouch);f.right_trigger=1;f.right_pad={.2f,.2f};assert(has(r.update(f,f.sample_us),Action::Pointer));});
    std::cout<<"PASS "<<passed<<" controls178 drift/timing/menu/RTS checks; no game or Deck used\n";
}
