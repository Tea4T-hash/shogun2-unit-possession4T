#include "../../native/deck/mailbox175.h"
#include "../../native/deck/rts175.h"
#include <cassert>
#include <iostream>
#include <vector>
using namespace deck174;using namespace deck175;
struct Event {char kind;Vec2 value;float rotate{},zoom{},pitch{},dt{};bool modifier{};};
struct FakeHost:Host {
    Identity identity{1,2,3,4,5,6};bool alive{true},reject_pointer{},retire_after_press{},reject_release{},reject_commit{};
    std::vector<Event> events;
    bool current(const Identity& i)override{return alive&&identity==i;}
    void cancel_owned(const Identity& i)override{if(current(i))events.push_back({'C'});}
    bool pointer(Vec2 v)override{events.push_back({'P',v});return !reject_pointer;}
    bool select(bool down,bool modifier)override{events.push_back({down?'D':'U',{},0,0,0,0,modifier});if(down&&retire_after_press)alive=false;return down||!reject_release;}
    bool order(unsigned phase,bool)override{events.push_back({phase==0?'B':phase==1?'V':'E'});return phase!=2||!reject_commit;}
    bool halt()override{events.push_back({'H'});return true;}
    bool camera(Vec2 pan,float rotate,float zoom,float pitch,float dt)override{events.push_back({'M',pan,rotate,zoom,pitch,dt});return true;}
    unsigned count(char kind)const{unsigned n=0;for(auto e:events)if(e.kind==kind)++n;return n;}
};
static ContextProof proof(FakeHost& h){return {h.identity,true,true,true,false,false};}
static Frame frame(uint64_t now=1000000,uint64_t sequence=1){Frame f;f.source=Source::Simulation;f.sample_us=now;f.sequence=sequence;f.epoch=1;f.connected=f.focused=f.native_bindings=true;f.active_buttons=all_buttons;return f;}
struct Exercise {
    Rts r;FakeHost host;uint64_t now{1000000},sequence{};
    void feed(uint64_t buttons=0,Vec2 pan={},float trigger=0,Vec2 right={},uint64_t step=8000){
        now+=step;auto p=proof(host);r.begin(p,host,now);auto f=frame(now,++sequence);f.buttons=buttons;f.left_stick=pan;f.right_stick=right;f.right_trigger=trigger;r.sample(f,p,host,now);r.finish(host,now);
    }
};
static unsigned passed;
#define TEST(label,...) do{__VA_ARGS__;++passed;std::cout<<"PASS " label "\n";}while(0)
int main(){
    std::cout.setf(std::ios::unitbuf);
    TEST("mailbox preserves a short press and release between game callbacks",{Mailbox q;auto a=frame(),b=frame(1008000,2),c=frame(1016000,3);b.buttons=bit(Button::RightPadClick);assert(q.put(a)&&q.put(b)&&q.put(c));std::array<Frame,Mailbox::capacity> frames;bool reset=false;assert(q.take(frames,reset)==3&&reset&&frames[1].buttons&&!frames[2].buttons);});
    TEST("duplicate native sample is not queued twice",{Mailbox q;assert(q.put(frame()));assert(!q.put(frame()));assert(q.count==1);});
    TEST("device epoch drops pending old actions",{Mailbox q;q.put(frame());auto f=frame(1008000,1);f.epoch=2;q.put(f);std::array<Frame,Mailbox::capacity> frames;bool reset=false;assert(q.take(frames,reset)==1&&reset&&frames[0].epoch==2);});
    TEST("bounded overflow requests neutral reacquisition",{Mailbox q;for(unsigned i=1;i<=Mailbox::capacity+1;++i)q.put(frame(1000000+i*8000,i));assert(q.count==1&&q.discontinuity);q.invalidate();assert(q.count==0&&q.discontinuity);});
    TEST("native selection drag retains modifier and release",{Exercise x;x.feed();x.feed(bit(Button::RightPadClick)|bit(Button::R5));x.feed(bit(Button::RightPadClick)|bit(Button::R5));x.feed();assert(x.host.count('D')==1&&x.host.count('U')==1&&x.host.events.front().modifier);});
    TEST("confirm uses native press and release once",{Exercise x;x.feed();x.feed(bit(Button::A));x.feed(bit(Button::A));x.feed();assert(x.host.count('D')==1&&x.host.count('U')==1);});
    TEST("native order gesture commits once on release",{Exercise x;x.feed();x.feed(0,{},1);x.feed(0,{},1);x.feed();x.feed();assert(x.host.count('B')==1&&x.host.count('E')==1&&x.r.committed==1);});
    TEST("cancel discards placement without releasing through commit",{Exercise x;x.feed();x.feed(0,{},1);x.feed(bit(Button::B),{},1);x.feed();assert(x.host.count('C')==1&&!x.host.count('E'));});
    TEST("halt cancels owned gesture before native halt",{Exercise x;x.feed();x.feed(0,{},1);x.feed(bit(Button::X),{},1);x.feed();assert(x.host.count('C')==1&&x.host.count('H')==1&&!x.host.count('E'));});
    TEST("unverified queue modifier never becomes a replacement order",{Exercise x;x.feed();x.feed(bit(Button::R1));x.feed(bit(Button::R1),{},1);x.feed(bit(Button::R1));x.feed();assert(!x.host.count('B')&&!x.host.count('E')&&x.r.unsupported);});
    TEST("focus loss cancels an owned drag without commit",{Exercise x;x.feed();x.feed(0,{},1);auto p=proof(x.host);p.focused=false;x.r.begin(p,x.host,x.now+8000);assert(x.host.count('C')==1&&!x.host.count('E'));});
    TEST("menu modal possession and wrong thread deny native actions",{for(unsigned mode=0;mode<4;++mode){Rts r;FakeHost h;auto p=proof(h);if(mode==0)p.blocked=true;if(mode==1)p.possessed=true;if(mode==2)p.owner_thread=false;if(mode==3)p.rts=false;r.begin(p,h,1000000);auto f=frame();f.buttons=bit(Button::A);r.sample(f,p,h,1000000);r.finish(h,1000000);assert(h.events.empty());}});
    TEST("retired identity refuses the rest of the batch",{Exercise x;x.feed();x.host.retire_after_press=true;x.feed(bit(Button::A));assert(x.host.count('D')==1&&x.host.count('U')==0);});
    TEST("stale queued input cannot issue an order",{Rts r;FakeHost h;auto p=proof(h);r.begin(p,h,1200000);auto f=frame();f.right_trigger=1;r.sample(f,p,h,1200000);assert(h.events.empty()&&r.rejected);});
    TEST("held trigger after context return requires neutral",{Exercise x;x.feed();x.feed(0,{},1);auto p=proof(x.host);p.focused=false;x.r.begin(p,x.host,x.now);x.feed(0,{},1);x.feed(0,{},1);assert(x.host.count('B')==1);x.feed();x.feed(0,{},1);assert(x.host.count('B')==2);});
    TEST("camera intent uses one capped callback delta",{Exercise x;x.feed();x.feed(0,{0,1},0,{.5f,.5f},100000);assert(x.host.count('M')==1);auto e=x.host.events.back();assert(e.dt==.05f&&e.value.y>0&&e.rotate>0&&e.zoom>0);});
    TEST("L3 fast pan does not double rotate or zoom",{Exercise x;x.feed();x.feed(bit(Button::L3),{0,1},0,{.5f,.5f});auto fast=x.host.events.back();Exercise y;y.feed();y.feed(0,{0,1},0,{.5f,.5f});auto normal=y.host.events.back();assert(fast.value.y==normal.value.y*2&&fast.rotate==normal.rotate&&fast.zoom==normal.zoom&&fast.dt==normal.dt);});
    TEST("unimplemented wheels suppress conflicting camera and commands",{Exercise x;x.feed();x.feed(bit(Button::Y),{0,1},0,{1,1});assert(!x.host.count('M')&&!x.host.count('B')&&x.r.unsupported);});
    TEST("custom stale threshold survives reset",{Rts r;Config c;c.stale_ms=20;r.configure(c);FakeHost h;auto p=proof(h);r.begin(p,h,1000000);auto f=frame();r.sample(f,p,h,1030000);assert(r.rejected);r.begin(p,h,1040000);f=frame(1040000,2);r.sample(f,p,h,1070000);assert(r.rejected==2);});
    TEST("failed native selection release cancels the owned gesture",{Exercise x;x.feed();x.feed(bit(Button::RightPadClick));x.host.reject_release=true;x.feed();assert(x.r.rejected&&x.host.count('C')==1);});
    TEST("failed native confirm release cancels the owned gesture",{Exercise x;x.feed();x.host.reject_release=true;x.feed(bit(Button::A));assert(x.r.rejected&&x.host.count('C')==1);});
    TEST("failed native order commit cancels without reporting success",{Exercise x;x.feed();x.feed(0,{},1);x.host.reject_commit=true;x.feed();assert(x.r.rejected&&!x.r.committed&&x.host.count('C')==1);});
    std::cout<<"PASS "<<passed<<" RTS175 policy/queue checks; engine and hardware not exercised\n";
}
