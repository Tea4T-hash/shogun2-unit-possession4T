// Production backend; fake SteamInput006 ABI, no hardware/Steam client.
#define S2UP_CONTEXT181_EMBEDDED 1
#include "verify_context_backend181.cpp"
#include "../../native/deck/controls181.h"
#include "../../native/deck/recovery218.h"
struct Screen214:ContextSteam181 {
 bool inactive{};unsigned activations{},starts{};
 virtual bool init(bool manual)override{++starts;return ContextSteam181::init(manual);}
 virtual void activate(uint64_t h,uint64_t set)override{++activations;inactive=false;ContextSteam181::activate(h,set);}
 virtual steam006::Digital digital(uint64_t h,uint64_t a)override{auto d=ContextSteam181::digital(h,a);if(inactive)d.active=false;return d;}
 virtual steam006::Analog analog(uint64_t h,uint64_t a)override{auto d=ContextSteam181::analog(h,a);if(inactive)d.active=false;return d;}
};
struct Shutdown218:Screen214 {
 bool failed{};
 virtual bool shutdown()override{ContextSteam181::shutdown();return !failed;}
};
int main(){Screen214 fake;SteamBackend backend;Frame f;unsigned cases{};
 assert(backend.select_context(Context::Possession)&&backend.initialize({&fake},"inert fixture",true));
 assert(backend.sample(1000000,true,f));auto epoch=f.epoch;auto set=fake.current_set;++cases;
 // Same handle/current action set, inactive stream after screen saver focus.
 fake.inactive=true;backend.sample(1008000,false,f);unsigned before=fake.activations;
 assert(backend.sample(1016000,true,f)&&f.epoch>epoch&&fake.current_set==set&&fake.activations>before&&fake.starts==1);++cases;
 // Some SteamOS overlays retain game focus. Bounded inactive-data retry must
 // recover without overwriting the layout or restarting Steam Input.
 fake.inactive=true;epoch=f.epoch;assert(!backend.sample(1024000,true,f));
 before=fake.activations;assert(!backend.sample(1520000,true,f));
 assert(backend.sample(1528000,true,f)&&fake.activations>before&&fake.starts==1);++cases;
 // Recovered controls require neutral before reusing a held gameplay action.
 deck181::Controls c;auto good=f;good.buttons=0;good.sequence++;good.sample_us+=8000;c.sample(good,Context::Possession,good.sample_us);
 good.epoch++;good.sequence++;good.sample_us+=8000;good.buttons=bit(Button::R5);assert(!c.sample(good,Context::Possession,good.sample_us).toggle_pause214);++cases;
 good.sequence++;good.sample_us+=8000;good.buttons=0;c.sample(good,Context::Possession,good.sample_us);
 good.sequence++;good.sample_us+=8000;good.buttons=bit(Button::R5);assert(c.sample(good,Context::Possession,good.sample_us).toggle_pause214);++cases;
 good.sequence++;good.sample_us+=8000;assert(!c.sample(good,Context::Possession,good.sample_us).toggle_pause214);++cases;
 assert(backend.close()&&fake.closed);++cases;
 // Failed client Shutdown must not strand a running flag with a cleared reader.
 Shutdown218 shutdown;SteamBackend resumed;volatile LONG running=1;
 assert(resumed.select_context(Context::Possession)&&resumed.initialize({&shutdown},"inert fixture",true));++cases;
 shutdown.failed=true;assert(deck218::close_reader(resumed,running)&&!running&&!resumed.initialized());++cases;
 shutdown.failed=false;assert(resumed.initialize({&shutdown},"inert fixture",true)&&resumed.sample(2000000,true,f));++cases;
 assert(deck218::close_reader(resumed,running)&&!resumed.initialized());++cases;
 std::cout<<"PASS recovery214: "<<cases<<" production screensaver/inactive-stream/neutral recovery cases; fake Steam ABI only\n";
}
