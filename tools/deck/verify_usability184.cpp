#define NOMINMAX
#include <windows.h>
#include "../../native/deck/command_display184.h"
#include "../../native/deck/shortcut_click184.h"
#include "../../native/common/image_fit184.h"
#include "../../native/deck/controls181.h"
#include "../../native/deck/rts_schema178.h"
#include <cassert>
#include <iostream>
struct Host184 {unsigned moves{},down{},up{};bool target(int,int){++moves;return true;}bool button(bool,bool held){held?++down:++up;return true;}};
int main(){unsigned cases=0;command100::Status sample{},out{};sample.magic=command100::magic;sample.version=100;sample.size=sizeof sample;sample.pid=1;sample.lease=2;sample.actor=3;sample.unit=4;sample.tick=100;sample.fire_mode=1;
 deck184::CommandDisplay cache;auto use=[&](deck184::Read state,uint64_t now,unsigned actor=3){return cache.resolve(state,sample,1,2,actor,4,now,out);};
 assert(!use(deck184::Read::Contended,100));++cases;
 assert(use(deck184::Read::Stable,100)&&out.fire_mode==1);++cases;
 for(unsigned n=101;n<=1100;++n)assert(use(deck184::Read::Contended,n)&&out.fire_mode==1);++cases;
 assert(!use(deck184::Read::Contended,1101));++cases;
 assert(use(deck184::Read::Stable,100)&&!use(deck184::Read::Contended,101,7));++cases;
 assert(use(deck184::Read::Stable,100));sample.lease=0;assert(!use(deck184::Read::Stable,101));sample.lease=2;assert(!use(deck184::Read::Contended,102));++cases;
 assert(use(deck184::Read::Stable,100)&&!use(deck184::Read::Missing,100)&&!use(deck184::Read::Contended,101));++cases;
 assert(use(deck184::Read::Stable,100));sample.fire_mode=2;sample.tick=101;assert(use(deck184::Read::Stable,101)&&out.fire_mode==2);++cases;
 for(auto dims:{std::pair<float,float>{128,236},{118,218},{148,273},{236,236}}){auto fit=imagefit184::within({27,81,148,190},dims.first,dims.second);assert(fit.x>=27&&fit.y>=81&&fit.x+fit.w<=175.001f&&fit.y+fit.h<=271.001f&&fabs(fit.w/fit.h-dims.first/dims.second)<.00001);++cases;}
 assert(imagefit184::within({0,0,148,190},0,0).w==0);++cases;
 Host184 host;deck184::ShortcutClick click;assert(click.begin(100,200,1000,1)&&!click.begin(300,400,1000,1));click.step(1000,1,host,true);assert(host.moves==1&&!host.down);click.step(1020,1,host,true);assert(!host.down);click.step(1021,2,host,true);assert(host.down==1&&!host.up);click.step(1040,3,host,true);assert(host.up==1&&!click.phase);++cases;
 assert(click.begin(100,200,2000,4));click.step(2000,4,host,true);click.step(2020,5,host,true);click.step(2021,5,host,false);assert(host.up==2&&!click.phase);++cases;
 assert(click.begin(100,200,3000,6));click.step(3501,7,host,true);assert(!click.phase);++cases;
 auto frame=[](unsigned n){deck174::Frame f;f.sequence=n;f.epoch=1;f.sample_us=1000000+uint64_t(n)*8000;f.source=deck174::Source::Simulation;f.active_buttons=deck174::all_buttons;f.connected=f.focused=f.native_bindings=true;return f;};
 {deck174::RtsRouter178 router;auto f=frame(1);router.update(f,f.sample_us);f=frame(2);f.buttons=deck174::bit(deck174::Button::L3);f.right_trigger=1;f.left_stick={1,0};auto actions=router.update(f,f.sample_us);bool select=false,pan=false;for(unsigned i=0;i<actions.count;++i){auto a=actions.items[i];if(a.action==deck174::Action::SelectBegin)select=a.modifier;if(a.action==deck174::Action::CameraPan){pan=true;assert(!a.modifier);}}assert(select&&pan);++cases;
 f=frame(3);f.buttons=deck174::bit(deck174::Button::L3);actions=router.update(f,f.sample_us);bool release=false;for(unsigned i=0;i<actions.count;++i)if(actions.items[i].action==deck174::Action::SelectEnd)release=actions.items[i].modifier;assert(release);++cases;}
 {deck181::Controls controls;auto f=frame(1);controls.sample(f,deck174::Context::Possession,f.sample_us);f=frame(2);f.buttons=deck174::bit(deck174::Button::L3);auto o=controls.sample(f,deck174::Context::Possession,f.sample_us);assert(o.held(deck181::Shift)&&!o.held(deck181::Middle)&&!o.held(deck181::Control));++cases;f=frame(3);f.buttons=deck174::bit(deck174::Button::L4);o=controls.sample(f,deck174::Context::Possession,f.sample_us);assert(o.held(deck181::Middle)&&!o.held(deck181::Shift));++cases;f=frame(4);f.focused=false;o=controls.sample(f,deck174::Context::Possession,f.sample_us);assert(!o.held(deck181::Middle)&&!o.held(deck181::Shift));++cases;}
 std::cout<<"PASS "<<cases<<" usability184 display identity/freshness/card-ratio/queued-click checks\n";
}
