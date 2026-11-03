// Production RTS core/mailboxes plus bounded presentation; no game or Steam.
#define main prior_rts175_fixture182
#include "verify_rts175.cpp"
#undef main
#include "../../native/deck/controls181.h"
#include "../../native/deck/presentation182.h"
#include "../../native/deck/navigation182.h"
int main(){passed=0;
 TEST("old independent RTS queue keeps moving while possession queue overflows",{
  Rts r;Config config;config.community_rts=true;r.configure(config);FakeHost host;deck175::Mailbox rts;deck181::Mailbox controls;
  uint64_t now=1000000;
  for(unsigned i=1;i<=120;++i){now+=8000;auto f=frame(now,i);if(i>4)f.left_stick.y=1;rts.put(f);deck181::Output output;output.context=Context::RTS;controls.put(f,output,0,0,0);
   if(i%2==0){std::array<Frame,deck175::Mailbox::capacity> frames;bool reset=false;auto n=rts.take(frames,reset);if(reset)r.reset(host);auto p=proof(host);r.begin(p,host,now);for(unsigned j=0;j<n;++j)r.sample(frames[j],p,host,now);r.finish(host,now);}}
  assert(host.count('M')>50);for(auto event:host.events)if(event.kind=='M')assert(event.value.y>0&&event.dt>0&&event.dt<=.05f);
 });
 TEST("cached glyphs require no new IO or decoding",{deck182::Presentation p;unsigned copies=0;p.poll(1000,[](unsigned){return true;},[&](unsigned){++copies;});assert(p.complete&&!copies);});
 TEST("uncached glyph decoding is limited to one image per poll",{deck182::Presentation p;bool cached[30]{};unsigned copies=0;for(unsigned i=0;i<32;++i){auto before=copies;p.poll(1000+i*8,[&](unsigned slot){return cached[slot];},[&](unsigned slot){cached[slot]=true;++copies;});assert(copies-before<=1);}assert(p.complete&&copies==30);});
 TEST("failed optional glyph retries are bounded and spaced",{deck182::Presentation p;unsigned attempts=0;uint64_t previous=0;for(uint64_t now=1000;now<20000;now+=8)p.poll(now,[](unsigned slot){return slot!=5;},[&](unsigned){if(previous)assert(now-previous>=2000);previous=now;++attempts;});assert(attempts==3&&!p.complete);});
 TEST("late successful glyph does not restart the entire artwork pass",{deck182::Presentation p;bool ready=false;unsigned attempts=0;for(uint64_t now=1000;now<8000;now+=8)p.poll(now,[&](unsigned slot){return slot!=5||ready;},[&](unsigned){ready=++attempts==3;});assert(ready&&p.complete&&attempts==3);});
 TEST("neutral camera stick clears an undelivered backlog",{assert(deck182::accumulate(4,0,.2f,4)==0);assert(deck182::accumulate(.2f,1,.1f,4)>.29f);});
 TEST("reversing camera direction immediately discards old-direction backlog",{assert(deck182::accumulate(4,-1,.25f,4)==-.25f);assert(deck182::accumulate(-4,1,.25f,4)==.25f);});
 std::cout<<"PASS "<<passed<<" navigation182 RTS delivery/presentation/backlog checks; no game or Steam\n";
}
