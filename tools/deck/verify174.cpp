#include "../../native/deck/config174.h"
#include "../../native/deck/router174.h"
#include "../../native/deck/proof174.h"
#include "../../native/deck/steam_input174.h"
#include "../../native/deck/diagnostic174.h"
#include <cassert>
#include <iostream>
#include <cstring>
using namespace deck174;

static Frame frame(uint64_t now=1000000,uint64_t seq=1){Frame f;f.source=Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.epoch=1;f.sequence=seq;f.sample_us=now;f.active_buttons=all_buttons;f.capabilities=Sticks|Triggers|Pads|Grips|Motion;return f;}
static bool has(const Batch& b,Action a){for(unsigned i=0;i<b.count;++i)if(b.items[i].action==a)return true;return false;}
static Facts rts(){Facts f;f.focused=f.device=f.battle=true;return f;}
static Facts fps(){auto f=rts();f.possessed=true;return f;}
static unsigned passed=0;
#define TEST(name,...) do{__VA_ARGS__;++passed;std::cout<<"PASS " name "\n";}while(0)

// Real C++ virtual member ABI, including packed structure returns. Unused slots
// remain explicit so the tested indices cannot silently slide by one.
struct FakeSteam {
    bool closed{},manifest_ok{true},manifest_called{};int type{14},count{1},bad_origin{-1},gamepad{-1};bool disconnected{};
    int trigger_origins{};uint64_t buttons{};float l2{},r2{};
    uint32_t remote{};uint64_t current_set{};
    virtual bool init(bool){return true;} // 0
    virtual bool shutdown(){closed=true;return true;}
    virtual bool manifest(const char*){manifest_called=true;return manifest_ok;}
    virtual void run(bool){}
    virtual void s4(){} virtual void s5(){}
    virtual int connected(uint64_t* p){p[0]=44;return disconnected?0:count;}
    virtual void s7(){} virtual void s8(){}
    virtual uint64_t set(const char*){return 3;}
    virtual void activate(uint64_t,uint64_t a){current_set=a;}
    virtual uint64_t current(uint64_t){return current_set;}
    virtual void s12(){}virtual void s13(){}virtual void s14(){}virtual void s15(){}
    virtual uint64_t digital_handle(const char* n){for(unsigned i=0;i<button_count;++i)if(!strcmp(n,steam006::digital[i].name))return i+1;return 0;}
    virtual steam006::Digital digital(uint64_t,uint64_t action){return {(buttons&(uint64_t(1)<<(action-1)))!=0,true};}
    virtual int origins(uint64_t,uint64_t,uint64_t a,int* o){o[0]=int(a)-1==bad_origin?333:steam006::digital[a-1].origin;return 1;}
    virtual void s19(){}
    virtual uint64_t analog_handle(const char* n){for(unsigned i=0;i<6;++i)if(!strcmp(n,steam006::analog[i]))return i+100;return 0;}
    virtual steam006::Analog analog(uint64_t,uint64_t a){return {a>=104?10:6,a==104?l2:a==105?r2:0,0,true};}
    virtual int aorigins(uint64_t,uint64_t,uint64_t a,int* o){
        o[0]=steam006::analog_origin[a-100];
        if(a>=104){
            if(trigger_origins==1)++o[0];
            if(trigger_origins==2){o[1]=o[0]+1;return 2;}
            if(trigger_origins==3)o[0]=a==104?357:355;
        }return 1;}
    virtual const char* s23(int,int,uint32_t){return "owned-glyph-fixture.png";}virtual void s24(){}virtual void s25(){}virtual void s26(){}virtual void s27(){}virtual void s28(){}
    virtual steam006::MotionData motion(uint64_t){return {0,0,0,1,0,0,16383.5f,100,200,300};}
    virtual void s30(){}virtual void s31(){}virtual void s32(){}virtual void s33(){}virtual void s34(){}virtual void s35(){}virtual bool s36(uint64_t h){assert(h==44);return true;}
    virtual int input_type(uint64_t){return type;}
    virtual void s38(){}
    virtual int index(uint64_t){return gamepad;}
    virtual void s40(){}virtual void s41(){}virtual void s42(){}virtual void s43(){}virtual void s44(){}
    virtual uint32_t remote_session(uint64_t){return remote;}
    virtual void s46(){}virtual void s47(){}
};

int main(){
    std::cout.setf(std::ios::unitbuf);
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);_set_error_mode(_OUT_TO_STDERR);
    TEST("default settings",{assert(Config{}.valid());});
    TEST("finite and bounded frame",{auto f=frame();assert(valid(f,1000000));f.right_pad.x=NAN;assert(!valid(f,1000000));f=frame();f.source=Source::None;assert(!valid(f,1000000));});
    TEST("deadzone preserves diagonal direction",{auto a=deadzone({1,1},.12f);assert(a.x>.70f&&a.x<.71f&&a.x==a.y);assert(deadzone({.1f,0},.12f).x==0);});
    TEST("neutral reacquisition",{Normalizer n;Edges e;auto f=frame();f.buttons=bit(Button::R4);assert(!n.accept(f,1000000,{},e));f=frame(1008000,2);assert(n.accept(f,1008000,{},e));f=frame(1016000,3);f.buttons=bit(Button::R4);assert(n.accept(f,1016000,{},e)&&e.down==bit(Button::R4));});
    TEST("stale and repeated sequence refused",{Normalizer n;Edges e;auto f=frame();assert(n.accept(f,1000000,{},e));assert(!n.accept(f,1000001,{},e));f=frame(1000000,2);assert(!n.accept(f,1200000,{},e));});
    TEST("trigger hysteresis",{Normalizer n;Edges e;n.accept(frame(),1000000,{},e);auto f=frame(1008000,2);f.right_trigger=.3f;assert(n.accept(f,1008000,{},e)&&e.right_fire);f=frame(1016000,3);f.right_trigger=.2f;assert(n.accept(f,1016000,{},e)&&!e.right_fire);});
    TEST("priority hierarchy",{Facts f=rts();f.possessed=true;assert(context(f)==Context::Possession);f.targeting=true;assert(context(f)==Context::Targeting);f.radial=true;assert(context(f)==Context::Radial);f.menu=true;assert(context(f)==Context::Menu);f.modal=true;assert(context(f)==Context::Modal);f.focused=false;assert(context(f)==Context::Inactive);});
    TEST("duplicate sample retains held intent until stale",{Router r;r.update(frame(),fps(),1000000);auto f=frame(1008000,2);f.left_stick={0,1};r.update(f,fps(),1008000);assert(!has(r.update(f,fps(),1010000),Action::ClearHeld));assert(has(r.update(f,fps(),1200000),Action::ClearHeld));assert(!has(r.update(f,fps(),1210000),Action::ClearHeld));});
    TEST("focus loss clears analog-only movement",{Router r;r.update(frame(),fps(),1000000);auto f=frame(1008000,2);f.left_stick={0,1};r.update(f,fps(),1008000);f=frame(1016000,3);f.focused=false;assert(has(r.update(f,fps(),1016000),Action::ClearHeld));});
    TEST("wheel suppresses camera and firing",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.left_trigger=.8f;f.right_stick={.9f,.9f};auto b=r.update(f,rts(),1008000);assert(has(b,Action::TacticalWheel)&&!has(b,Action::CameraRotate)&&!has(b,Action::CameraZoom)&&!has(b,Action::OrderBegin));});
    TEST("radial conflict arbitration",{Router r;auto facts=rts();facts.radial=true;r.update(frame(),facts,1000000);auto f=frame(1008000,2);f.right_stick={.9f,0};f.right_trigger=1;auto b=r.update(f,facts,1008000);assert(has(b,Action::WheelPoint)&&!has(b,Action::CameraRotate)&&!has(b,Action::OrderBegin));});
    TEST("opening trigger release commits wheel once",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.left_trigger=1;assert(has(r.update(f,rts(),1008000),Action::TacticalWheel));auto radial=rts();radial.radial=true;f=frame(1016000,3);f.left_trigger=1;r.update(f,radial,1016000);f=frame(1024000,4);f.right_stick={1,0};assert(has(r.update(f,radial,1024000),Action::WheelCommit));assert(!has(r.update(frame(1032000,5),radial,1032000),Action::WheelCommit));});
    TEST("wheel exit cannot turn held trigger into a shot",{Router r;r.update(frame(),fps(),1000000);auto f=frame(1008000,2);f.buttons=bit(Button::Y);r.update(f,fps(),1008000);auto radial=fps();radial.radial=true;f=frame(1016000,3);f.right_trigger=1;r.update(f,radial,1016000);f=frame(1024000,4);f.right_trigger=1;assert(!has(r.update(f,fps(),1024000),Action::Primary));});
    TEST("R1 chord suppresses run toggle",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.buttons=bit(Button::R1);r.update(f,rts(),1008000);f=frame(1016000,3);f.buttons=bit(Button::R1);f.right_trigger=1;r.update(f,rts(),1016000);f=frame(1024000,4);f.buttons=bit(Button::R1);assert(has(r.update(f,rts(),1024000),Action::OrderCommit));f=frame(1032000,5);assert(!has(r.update(f,rts(),1032000),Action::ToggleRun));});
    TEST("cancel cannot commit placement",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.right_trigger=1;r.update(f,rts(),1008000);f=frame(1016000,3);f.buttons=bit(Button::B);auto b=r.update(f,rts(),1016000);assert(has(b,Action::OrderCancel)&&!has(b,Action::OrderCommit));});
    TEST("held trigger cannot cross possession",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.right_trigger=1;r.update(f,rts(),1008000);f=frame(1016000,3);f.right_trigger=1;auto b=r.update(f,fps(),1016000);assert(has(b,Action::ClearHeld)&&!has(b,Action::Primary));});
    TEST("R4 only possession request",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.buttons=bit(Button::R4);auto b=r.update(f,rts(),1008000);assert(has(b,Action::PossessionToggle)&&!has(b,Action::OrderCommit));});
    TEST("campaign cannot possess",{Router r;Facts c;c.focused=c.device=c.campaign=true;r.update(frame(),c,1000000);auto f=frame(1008000,2);f.buttons=bit(Button::R4);assert(!has(r.update(f,c,1008000),Action::PossessionToggle));});
    TEST("L4 single and double are distinct",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.buttons=bit(Button::L4);r.update(f,rts(),1008000);r.update(frame(1016000,3),rts(),1016000);f=frame(1100000,4);f.buttons=bit(Button::L4);auto b=r.update(f,rts(),1100000);assert(has(b,Action::SelectArmy)&&!has(b,Action::SelectVisible));r.update(frame(1110000,5),rts(),1110000);assert(!has(r.update(frame(1500000,6),rts(),1500000),Action::SelectVisible));});
    TEST("R3 pitch gesture is not focus tap",{Router r;r.update(frame(),rts(),1000000);auto f=frame(1008000,2);f.buttons=bit(Button::R3);f.right_stick={0,.8f};assert(has(r.update(f,rts(),1008000),Action::CameraPitch));assert(!has(r.update(frame(1020000,3),rts(),1020000),Action::FocusSelection));});
    TEST("native ten groups and bank center",{GroupPanel p;assert(p.region({0,0})==0);assert(p.native_group(5)==5);p.switch_bank();assert(p.native_group(5)==10&&p.native_group(1)==6);assert(!p.native_group(0));});
    TEST("group overwrite requires hold and one commit",{Overwrite o;Config c;assert(!o.request(3,true,true,1000000,c));assert(!o.request(3,true,true,1200000,c));assert(o.request(3,true,true,1600000,c));assert(!o.request(3,true,true,1800000,c));o.request(3,true,false,1900000,c);assert(!o.request(3,true,true,2000000,c));});
    TEST("center and unavailable wheel sectors cancel",{Menu m;m.count=4;m.items[0].available=true;m.point({0,0},{});assert(!m.commit());m.point({0,1},{});assert(m.commit());m.point({1,0},{});assert(!m.commit());});
    TEST("strict settings preserve prior values",{Config c;std::string e;assert(parse_config("left_deadzone=0.2\nhold_ms=400\n",c,e)&&c.left_deadzone==.2f&&c.hold_ms==400);auto before=c;assert(!parse_config("hold_ms=1.5",c,e)&&c.hold_ms==before.hold_ms);assert(!parse_config("x=1",c,e));assert(!parse_config("left_deadzone=nan",c,e));assert(!parse_config("gyro_rts=2",c,e));assert(!parse_config("hold_ms=350\nhold_ms=400",c,e));});
    TEST("gyro bias and nonfinite reset",{MotionFilter m;Config c;for(int i=0;i<100;++i)m.update({.01f,.02f,.01f},.008f,true,c);auto a=m.update({.01f,.02f,.01f},.008f,false,c);assert(std::fabs(a.x)<.00001f);auto b=m.update({NAN,0,0},.008f,false,c);assert(b.x==0);});
    TEST("unverified engine dispatch disabled",{DispatchGate g;Intent i;i.action=Action::OrderCommit;assert(!g.permits(i));g.verified[unsigned(Action::Halt)]=true;assert(!g.permits(i));});
    TEST("SteamInput006 packed return ABI",{FakeSteam fake;SteamBackend backend;assert(backend.initialize({&fake},"fixture"));Frame f;assert(backend.sample(1000000,true,f)&&valid(f,1000000));assert(f.capabilities&Grips);assert(f.accelerometer.z>9.80f&&f.accelerometer.z<9.81f);assert(backend.close()&&fake.closed);});
    TEST("playable preflight does not replace nonDeck or remote bindings",{for(unsigned i=0;i<2;++i){FakeSteam fake;if(i)fake.remote=1;else fake.type=3;SteamBackend b;assert(!b.initialize({&fake},"fixture",true)&&fake.closed&&!fake.manifest_called);}});
    TEST("playable preflight accepts a native local Deck before manifest",{FakeSteam fake;SteamBackend b;assert(b.initialize({&fake},"fixture",true)&&fake.manifest_called);Frame f;assert(b.sample(1000000,true,f));b.close();});
    TEST("asynchronous device discovery retains its owned initialization",{FakeSteam fake;fake.disconnected=true;SteamBackend b;assert(b.initialize({&fake},"fixture",true)&&b.initialized()&&!fake.closed&&!fake.manifest_called);Frame f;assert(!b.sample(1000000,true,f)&&!fake.closed);fake.disconnected=false;assert(b.sample(1008000,true,f)&&fake.manifest_called&&b.state()==BackendState::Ready);b.close();});
    TEST("native glyph path uses verified SteamInput006 slot23",{FakeSteam fake;SteamBackend b;assert(b.initialize({&fake},"fixture",true));assert(!strcmp(b.glyph_file(steam006::digital[0].origin),"owned-glyph-fixture.png"));b.close();assert(!b.glyph_file(steam006::digital[0].origin));});
    TEST("native stick clicks retain separate press and release edges",{FakeSteam fake;SteamBackend b;assert(b.initialize({&fake},"fixture"));Proof p;uint64_t t=1000000;auto read=[&](){Frame f;assert(b.sample(t,true,f));p.observe(f,t);t+=8000;};read();fake.buttons=bit(Button::L3);read();read();fake.buttons=0;read();fake.buttons=bit(Button::R3);read();read();fake.buttons=0;read();assert(p.frames==7&&p.rejected==0&&p.presses[unsigned(Button::L3)]==1&&p.presses[unsigned(Button::R3)]==1&&p.previous_buttons==0);b.close();});
    TEST("required coverage stays incomplete until both stick clicks",{Proof p;p.frames=200;p.pressed=all_buttons&~(bit(Button::L3)|bit(Button::R3));p.independent_grips=bit(Button::L4)|bit(Button::L5)|bit(Button::R4)|bit(Button::R5);for(unsigned i=0;i<8;++i){p.axes[i].add(-1);p.axes[i].add(1);}for(unsigned i=8;i<10;++i){p.axes[i].add(0);for(unsigned j=0;j<3;++j)p.axes[i].add(.5f);p.axes[i].add(1);}p.gyro[0].add(-1);p.gyro[0].add(1);assert(!p.required_inputs_observed());auto f=frame();f.buttons=bit(Button::L3);p.observe(f,1000000);assert(!p.required_inputs_observed());f=frame(1008000,2);f.buttons=bit(Button::R3);p.observe(f,1008000);assert(p.required_inputs_observed());});
    TEST("same-trigger full-pull origins preserve analog values",{FakeSteam fake;fake.trigger_origins=1;fake.l2=.35f;fake.r2=.65f;fake.buttons=bit(Button::A);SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(b.sample(1000000,true,f));assert(f.left_trigger==.35f&&f.right_trigger==.65f&&(f.buttons&bit(Button::A)));b.close();});
    TEST("soft and full origin arrays are the same physical triggers",{FakeSteam fake;fake.trigger_origins=2;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(b.sample(1000000,true,f));Proof p;p.observe(f,1000000);assert(p.frames==1&&p.rejected==0);b.close();});
    TEST("opposite physical trigger origins are refused",{FakeSteam fake;fake.trigger_origins=3;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(!b.sample(1000000,true,f)&&b.state()==BackendState::WaitingBindings);b.close();});
    TEST("digital and unrelated analog origins stay exact",{assert(!steam006::same_physical_origin(357,358,true));assert(!steam006::same_physical_origin(359,360,false));assert(!steam006::same_physical_origin(355,358,false));});
    TEST("recovered binding clears stale mismatch",{FakeSteam fake;fake.trigger_origins=3;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(!b.sample(1000000,true,f)&&b.info().bad_origin_expected);fake.trigger_origins=2;assert(b.sample(1008000,true,f)&&b.info().bad_origin_expected==0&&b.info().bad_origin_observed==0);b.close();});
    TEST("grip aliases refused",{FakeSteam fake;fake.bad_origin=int(Button::R4);SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(!b.sample(1000000,true,f)&&b.state()==BackendState::WaitingBindings);b.close();});
    TEST("nonDeck and remote controllers refused",{FakeSteam fake;fake.type=3;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(!b.sample(1000000,true,f));fake.type=14;fake.remote=2;assert(!b.sample(1008000,true,f)&&b.state()==BackendState::RemoteDevice);b.close();});
    TEST("invalid controller count refused",{FakeSteam fake;fake.count=17;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(!b.sample(1000000,true,f)&&b.state()==BackendState::Failed);b.close();});
    TEST("manifest failure shuts down owned interface",{FakeSteam fake;fake.manifest_ok=false;SteamBackend b;assert(!b.initialize({&fake},"fixture")&&fake.closed);});
    TEST("disconnect changes input epoch",{FakeSteam fake;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;assert(b.sample(1000000,true,f));auto first=f.epoch;fake.disconnected=true;assert(!b.sample(1008000,true,f));fake.disconnected=false;assert(b.sample(1016000,true,f)&&f.epoch>first);b.close();});
    TEST("virtual gamepad index is measured",{FakeSteam fake;fake.gamepad=2;SteamBackend b;assert(b.initialize({&fake},"fixture"));Frame f;b.sample(1000000,true,f);assert(b.emulated_gamepad_index()==2);b.close();});
    TEST("partial samples are not hardware proof",{Proof p;for(unsigned i=1;i<200;++i)p.observe(frame(1000000+i*8000,i),1000000+i*8000);assert(!p.required_inputs_observed());});
    TEST("user prompt delay does not consume the ninety second measurement",{uint64_t now=1000000,elapsed=0,sequence=0;Measurement timer;Proof p;bool ok=measure(timer,p,elapsed,[&](){return now;},[&](){now+=120000000;return true;},[&](uint64_t t,Frame& f){f=frame(t,++sequence);return true;},[&](){now+=1000000;});assert(ok&&elapsed==90000000&&p.frames==90&&timer.start_us==121000000);});
    TEST("cancelled prompt never starts or samples",{uint64_t elapsed=1;Measurement timer;Proof p;bool sampled=false;assert(!measure(timer,p,elapsed,[](){return uint64_t(1000000);},[](){return false;},[&](uint64_t,Frame&){sampled=true;return true;},[](){}));assert(timer.cancelled&&!timer.started&&!sampled&&elapsed==0);});
    TEST("onscreen summary exposes measured values and incomplete coverage",{Proof p;p.frames=150;p.presses[unsigned(Button::L4)]=7;p.presses[unsigned(Button::L3)]=2;p.presses[unsigned(Button::R3)]=3;p.axes[8].add(0);p.axes[8].add(1);Measurement timer;timer.start(1000000);auto text=result_text(p,BackendState::Ready,{},timer,90000000,0,-1,true);assert(text.find(L"revision 3")!=text.npos&&text.find(L"Measured time: 90.0 seconds")!=text.npos&&text.find(L"L4: 7")!=text.npos&&text.find(L"Stick clicks - L3: 2   R3: 3")!=text.npos&&text.find(L"coverage: INCOMPLETE")!=text.npos&&text.find(L"Send a photo of THIS screen")!=text.npos&&text.size()<3000);});
    std::cout<<"PASS "<<passed<<" Deck simulation/ABI checks; actual Deck/Proton verification pending\n";
}
