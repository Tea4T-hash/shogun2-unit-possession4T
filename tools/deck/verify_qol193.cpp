#define NOMINMAX
#include <windows.h>
#include "../../native/deck/controls181.h"
#include "../../native/deck/rts175.h"
#include "../../native/deck/command_display184.h"
#include <cassert>
#include <iostream>
#include <vector>
using namespace deck174;
static unsigned cases;
#define CHECK(...) do{assert((__VA_ARGS__));++cases;}while(0)
static Frame frame(uint64_t n,uint64_t now){Frame f;f.sequence=n;f.epoch=1;f.sample_us=now;f.source=Source::Simulation;f.active_buttons=all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
struct Controller {deck181::Controls c;uint64_t n{},now{1000000};Frame f;
 deck181::Output sample(Context mode,uint64_t buttons=0,Vec2 stick={},uint64_t step=8000,bool focus=true){now+=step;f=frame(++n,now);f.buttons=buttons;f.left_stick=stick;f.focused=focus;return c.sample(f,mode,now);}
};
struct ClickHost:deck175::Host {
 deck175::Identity identity{1,2,3,4,5,6};deck193::DoubleClick clicks;int x{100},y{100};uint64_t now{};bool ctrl{},shift{};unsigned resets{};std::vector<UINT> messages;
 bool current(const deck175::Identity& value)override{return value==identity;}
 void cancel_owned(const deck175::Identity&)override{clicks.reset();}
 void reset_clicks()override{clicks.reset();++resets;}
 void modifiers(bool s,bool c)override{shift=s;ctrl=c;}
 bool pointer(Vec2 delta)override{x+=int(delta.x*1000);y+=int(delta.y*1000);clicks.move(x,y,4,4);return true;}
 bool select(bool down,bool additive)override{bool twice=clicks.button(down,additive||ctrl||shift,x,y,now,500,4,4);messages.push_back(down?(twice?WM_LBUTTONDBLCLK:WM_LBUTTONDOWN):WM_LBUTTONUP);return true;}
 bool order(unsigned,bool)override{clicks.reset();return true;}
 bool halt()override{return true;}
 bool camera(Vec2,float,float,float,float)override{return true;}
};
int main(){
 // Full Controls tests: forward wobble, diagonal intent, hysteresis, directions,
 // release/focus/context boundaries; campaign and RTS never get soldier keys.
 for(int sign:{-1,1})for(bool sideways:{false,true}){
  Controller c;c.sample(Context::Possession);
  auto direction=[&](float minor){return sideways?Vec2{float(sign),minor}:Vec2{minor,float(sign)};};
  auto main=sideways?(sign>0?deck181::D:deck181::A):(sign>0?deck181::W:deck181::S);
  auto other=sideways?deck181::W:deck181::D;
  for(float drift:{0.f,.1f,.2f,.30f,.40f}){auto o=c.sample(Context::Possession,0,direction(drift));CHECK(o.held(main)&&!o.held(other));}
  CHECK(c.sample(Context::Possession,0,direction(.65f)).held(other));
  CHECK(c.sample(Context::Possession,0,direction(.4f)).held(other));
  CHECK(!c.sample(Context::Possession,0,direction(.2f)).held(other));
  auto stop=c.sample(Context::Possession);CHECK(!stop.held(main)&&!stop.held(other));
 }
 {Controller c;c.sample(Context::Possession);CHECK(c.sample(Context::Possession,bit(Button::L3),{0,1}).held(deck181::Shift));auto lost=c.sample(Context::Possession,0,{1,1},8000,false);CHECK(!lost.held(deck181::W));c.sample(Context::Possession);auto straight=c.sample(Context::Possession,0,{.4f,1});CHECK(straight.held(deck181::W)&&!straight.held(deck181::D));}
 for(auto mode:{Context::RTS,Context::Campaign}){Controller c;c.sample(mode);auto o=c.sample(mode,0,{1,1});CHECK(!o.held(deck181::W)&&!o.held(deck181::D));}
 // Hold assignment must not replace the selected unit before the hold completes.
 for(unsigned direction=0;direction<4;++direction){auto b=bit(Button(unsigned(Button::Up)+direction));Controller c;c.sample(Context::RTS);
  CHECK(!c.sample(Context::RTS,b).shortcuts);CHECK(!c.sample(Context::RTS,b,{},549999).shortcuts);
  CHECK(c.sample(Context::RTS,b,{},1).shortcuts==(1u<<(deck183::Assign1+direction)));
  for(unsigned i=0;i<100;++i)CHECK(!c.sample(Context::RTS,b).shortcuts);
  CHECK(!c.sample(Context::RTS).shortcuts);CHECK(!c.sample(Context::RTS,b).shortcuts);
  CHECK(c.sample(Context::RTS).shortcuts==(1u<<(deck183::Recall1+direction)));
 }
 {Controller c;c.sample(Context::RTS);c.sample(Context::RTS,bit(Button::Up));c.sample(Context::RTS,bit(Button::Up),{},8000,false);CHECK(!c.sample(Context::RTS).shortcuts);c.sample(Context::RTS,bit(Button::Left));CHECK(!c.sample(Context::RTS,bit(Button::Right)).shortcuts);CHECK(!c.sample(Context::RTS).shortcuts);}
 {Controller c;c.sample(Context::RTS);c.sample(Context::RTS,bit(Button::Up));c.sample(Context::Possession);CHECK(!c.sample(Context::RTS).shortcuts);c.sample(Context::RTS,bit(Button::Up)|bit(Button::Down));CHECK(!c.sample(Context::RTS).shortcuts);}
 // Post-copy clock: deliberately advance time while acquiring the status. The
 // old early timestamp rejects this valid sample; future/stale/reset still fail.
 command100::Status sample{},out{};sample.magic=command100::magic;sample.version=100;sample.size=sizeof sample;sample.pid=1;sample.lease=2;sample.actor=3;sample.unit=4;sample.tick=101;sample.fire_mode=1;
 deck184::CommandDisplay display;CHECK(!command100::valid(sample,1,2,3,4,100));uint64_t clock=100;
 auto read=[&](command100::Status& value){clock=101;value=sample;return deck184::Read::Stable;};
 CHECK(display.read(read,[&](){return clock;},1,2,3,4,out)&&out.fire_mode==1);
 sample.tick=102;CHECK(!display.read(read,[&](){return clock;},1,2,3,4,out));sample.tick=101;sample.lease=0;CHECK(!display.read(read,[&](){return clock;},1,2,3,4,out));sample.lease=2;
 CHECK(!display.read(read,[](){return 1102ull;},1,2,3,4,out));CHECK(!display.read(read,[&](){return clock;},1,2,9,4,out));
 // Actual RTS routing through the selection host. Verify the standard native
 // DOWN, UP, DOUBLE, UP sequence, multi-select, range and intentional drag.
 for(unsigned scenario=0;scenario<7;++scenario){ClickHost host;deck175::Rts r;deck174::Config cfg;cfg.community_rts=true;r.configure(cfg);uint64_t sequence=0,t=1000000;
  auto feed=[&](float trigger,uint64_t buttons=0,Vec2 pad={},uint64_t step=8000){t+=step;host.now=t/1000;auto f=frame(++sequence,t);f.right_trigger=trigger;f.buttons=buttons;f.right_pad=pad;deck175::ContextProof proof{host.identity,true,true,true,false,false};r.begin(proof,host,t);r.sample(f,proof,host,t);r.finish(host,t);};
  uint64_t modifier=scenario==1?bit(Button::L3):scenario==2?bit(Button::L4):0;
  feed(0);feed(0,bit(Button::RightPadTouch)|modifier);feed(1,bit(Button::RightPadTouch)|modifier);if(scenario==3){feed(1,bit(Button::RightPadTouch),{.1f,0});feed(1,bit(Button::RightPadTouch),{0,0});}feed(0,modifier);
  if(scenario==4)host.x+=10;
  if(scenario==5)feed(0,0,{},600000);
  if(scenario==6){r.reset(host);feed(0);}
  feed(1,modifier);feed(0,modifier);
  CHECK(host.messages.size()==4&&host.messages[0]==WM_LBUTTONDOWN&&host.messages[1]==WM_LBUTTONUP&&host.messages[3]==WM_LBUTTONUP);
  CHECK(host.messages[2]==(scenario==0?WM_LBUTTONDBLCLK:WM_LBUTTONDOWN));
  feed(1,modifier);CHECK(host.messages.back()==(scenario==0?WM_LBUTTONDOWN:WM_LBUTTONDBLCLK)||scenario==1||scenario==2);
 }
 std::cout<<"PASS qol193: "<<cases<<" movement/group/native-click/post-copy-clock assertions; no game inputs\n";
}
