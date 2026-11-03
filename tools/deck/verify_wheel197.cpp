#include "../../native/deck/controls181.h"
#include <cassert>
#include <iostream>
using namespace deck181;using deck174::bit;
struct Session {
 Controls controls;Frame f;uint64_t now=1000000;unsigned count=0;
 Session(){f.source=deck174::Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.active_buttons=deck174::all_buttons;f.epoch=1;step();}
 Output step(uint64_t buttons=0,Vec2 stick={}){f.buttons=buttons;f.right_stick=stick;f.sequence=++count;f.sample_us=now+=8000;return controls.sample(f,Context::Possession,now);}
};
int main(){unsigned cases=0;const unsigned expected[]={C,E,Z,R};const Vec2 sticks[]={{0,1},{0,-1},{-1,0},{1,0}};
 for(bool pad:{false,true})for(unsigned i=0;i<4;++i){Session s;auto b=bit(Button::R1)|(pad?bit(Button(unsigned(Button::Up)+i)):0);auto v=pad?Vec2{}:sticks[i];
  for(unsigned n=0;n<60;++n){auto o=s.step(b,v);assert(o.held(deck183::WheelHeld)&&!o.held(expected[i])&&!o.held(T)&&!o.held(G)&&o.look.x==0&&o.look.y==0&&o.zoom==0);}
  assert(s.step().held(expected[i]));for(unsigned n=0;n<60;++n)assert(!s.step().held(expected[i]));++cases;
 }
 {Session s;s.step(bit(Button::R1));assert(!s.step().held(C)&&!s.step().held(E));++cases;}
 for(unsigned i=0;i<4;++i){Session s;auto b=bit(Button::R1)|bit(Button(unsigned(Button::Up)+i));s.step(b);auto o=s.step(b|bit(Button::B));assert(!o.held(M)&&!o.held(deck183::WheelHeld));o=s.step(bit(Button::B));assert(!o.held(expected[i])&&!o.held(M));s.step();assert(s.step(bit(Button::B)).held(M));++cases;}
 for(unsigned i=0;i<4;++i){Session s;s.step(bit(Button::R1),sticks[i]);s.f.focused=false;s.step();s.f.focused=true;assert(!s.step().held(expected[i]));++cases;}
 for(auto button:{Button::Menu,Button::Y}){Session s;s.step(bit(Button::R1)|bit(Button::Left));auto o=s.step(bit(button));assert(!o.held(Z));++cases;}
 {Session s;s.step(bit(Button::R1)|bit(Button::Left));s.f.epoch++;assert(!s.step().held(Z));++cases;}
 {Session s;s.step(bit(Button::R1)|bit(Button::Left));s.step(bit(Button::R1)|bit(Button::View));s.now+=700000;assert(s.step(bit(Button::R1)|bit(Button::View)).help_open);s.step();s.step(bit(Button::B));assert(!s.step().held(Z));++cases;}
 {Session s;s.step(bit(Button::R1)|bit(Button::Up));s.step(bit(Button::R1)|bit(Button::Right));assert(s.step().held(R));++cases;}
 std::cout<<"PASS wheel197 policy: "<<cases<<" four-way stick/D-pad, release-once, empty/cancel/menu/help/focus/epoch cases\n";
}
