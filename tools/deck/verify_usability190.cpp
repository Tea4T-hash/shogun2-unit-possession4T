#define NOMINMAX
#include "../../native/deck/menu178.h"
#include "../../native/deck/rts_schema178.h"
#include "../../native/deck/campaign_keys185.h"
#include "../../native/deck/help_input188.h"
#include <cassert>
#include <iostream>
using namespace deck174;
struct H {unsigned back{},down{},up{};float x{},y{};void release_owned(bool,bool){} void reset_cursor(){} bool pointer(float a,float b){x+=a;y+=b;return true;}bool button(bool,bool d){if(d)++down;else++up;return true;}bool scroll(float){return true;}};
static Frame frame(unsigned n){Frame f;f.sample_us=1000000+n*8000;f.sequence=n;f.epoch=1;f.source=Source::Simulation;f.active_buttons=all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
int main(){unsigned cases=0;
 {Normalizer old;Edges e;auto f=frame(1);assert(old.accept(f,f.sample_us,{},e));f=frame(2);f.buttons=bit(Button::B)|bit(Button::L1);assert(old.accept(f,f.sample_us,{},e));old.reset();f=frame(3);f.buttons=bit(Button::L1);assert(!old.accept(f,f.sample_us,{},e));f=frame(4);f.buttons|=bit(Button::B);assert(!old.accept(f,f.sample_us,{},e));
  deck178::Menu178 m;H h;bool back{};for(unsigned i=1;i<=4;++i){f=frame(i);if(i>=2)f.buttons=bit(Button::L1);if(i==2||i==4)f.buttons|=bit(Button::B);m.sample(f,f.sample_us,{},h,back);h.back+=back;}assert(h.back==2);++cases;}

 {deck178::Menu178 m;H h;bool back{};for(unsigned i=1;i<=100;++i){auto f=frame(i);if(i>=2&&i<90)f.buttons=bit(Button::B);m.sample(f,f.sample_us,{},h,back);h.back+=back;}assert(h.back==1);++cases;}
 for(auto modifiers:{uint64_t(0),bit(Button::L3),bit(Button::L4)}){deck178::Menu178 m;H h;bool back{};auto f=frame(1);m.sample(f,f.sample_us,{},h,back);
  f=frame(2);f.buttons=bit(Button::RightPadTouch)|modifiers;m.sample(f,f.sample_us,{},h,back);
  f=frame(3);f.buttons=bit(Button::RightPadTouch)|modifiers;f.left_trigger=1;f.right_pad={.01f,.01f};m.sample(f,f.sample_us,{},h,back);assert(h.down==1&&!h.x&&!h.y);++cases;
  f=frame(4);f.buttons=bit(Button::RightPadTouch)|modifiers;f.left_trigger=1;f.right_pad={.02f,0};m.sample(f,f.sample_us,{},h,back);assert(!h.x&&!h.y);++cases;
  f=frame(5);f.buttons=bit(Button::RightPadTouch)|modifiers;f.left_trigger=1;f.right_pad={.25f,.25f};m.sample(f,f.sample_us,{},h,back);assert(modifiers?(!h.x&&!h.y):(h.x>0&&h.y>0));++cases;
  float x=h.x,y=h.y;f=frame(6);f.buttons=bit(Button::RightPadTouch);f.right_pad={.5f,.5f};m.sample(f,f.sample_us,{},h,back);assert(h.up==1&&h.x==x&&h.y==y);++cases;
 }
 {deck190::SteadyClick c;c.sample({},true,false,false);c.sample({},true,true,false);c.reset();assert(!c.sample({1,1},true,false,false).x);++cases;}
 for(auto button:{Button::L1,Button::R1}){RtsRouter178 r;auto f=frame(1);r.update(f,f.sample_us);f=frame(2);f.buttons=bit(button);auto b=r.update(f,f.sample_us);bool zoom=false;for(unsigned i=0;i<b.count;++i){assert(b.items[i].action!=Action::CameraRotate&&b.items[i].action!=Action::OrderBegin);if(b.items[i].action==Action::CameraZoom){assert(b.items[i].value.y==(button==Button::L1?1.f:-1.f));zoom=true;}}assert(zoom);++cases;}
 {RtsRouter178 r;auto f=frame(1);r.update(f,f.sample_us);f=frame(2);f.buttons=bit(Button::L1)|bit(Button::R1);auto b=r.update(f,f.sample_us);for(unsigned i=0;i<b.count;++i)assert(b.items[i].action!=Action::CameraZoom);++cases;}
 {auto f=frame(1);f.buttons=bit(Button::A)|bit(Button::L3)|bit(Button::L4);auto transformed=deck188::campaign_menu_frame189(f,false);assert(!(transformed.buttons&bit(Button::A))&&(transformed.buttons&bit(Button::L3))&&(transformed.buttons&bit(Button::L4)));Edges e;e.down=bit(Button::A);deck183::CampaignButtons buttons;assert(buttons.sample(f,e,f.sample_us)==deck183::MoveSpeed);++cases;}
 {deck185::CampaignKeys k;unsigned char keys[256]{};k.hold(MK_CONTROL|MK_SHIFT,1000,7);k.apply(keys,true,true,1001,7);assert(keys[0x1d]==0x80&&keys[0x2a]==0x80);++cases;
  for(auto mode:{0,1,2}){memset(keys,0,256);k.apply(keys,true,mode!=0,mode==1?1101:1001,mode==2?8:7);assert(!keys[0x1d]&&!keys[0x2a]);++cases;}
  k.hold(MK_CONTROL,1200,7);k.clear();k.apply(keys,true,true,1201,7);assert(!keys[0x1d]);++cases;
  assert(k.put(deck183::MoveSpeed,1300,7));assert(k.apply(keys,true,true,1300,7)&&keys[0x39]==0x80);++cases;
 }
 std::cout<<"PASS usability190: "<<cases<<" Back hold/click stability/intentional drag/shoulder zoom/campaign modifier+Space cases; offline only\n";
}
