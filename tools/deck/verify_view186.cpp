#define NOMINMAX
#define S2UP_CONTEXT181_EMBEDDED 1
#include "verify_context_backend181.cpp"
#include "../../native/deck/controls181.h"
#include <thread>
int main(){unsigned tests=0;deck186::ViewGesture v;
 assert(v.sample(true,1000000,true)==deck186::ViewEvent::None);assert(v.sample(false,1100000,true)==deck186::ViewEvent::HideHud);++tests;
 assert(v.sample(true,2000000,true)==deck186::ViewEvent::None);assert(v.sample(true,2600000,true)==deck186::ViewEvent::Controls);assert(v.sample(true,9000000,true)==deck186::ViewEvent::None);assert(v.sample(false,9010000,true)==deck186::ViewEvent::None);++tests;
 v.sample(true,10000000,true);v.reset();assert(v.sample(false,10001000,true)==deck186::ViewEvent::None);++tests;
 v.sample(true,11000000,false);assert(v.sample(true,12000000,false)==deck186::ViewEvent::Controls);assert(v.sample(false,12001000,false)==deck186::ViewEvent::None);++tests;
 ContextSteam181 steam;deck174::SteamBackend backend;assert(!backend.show_bindings());++tests;
 for(auto context:{deck174::Context::Menu,deck174::Context::Campaign,deck174::Context::RTS,deck174::Context::Possession}){
  assert(backend.select_context(context));if(!backend.initialized())assert(backend.initialize({&steam},"owned inert manifest",true));deck174::Frame f;assert(backend.sample(13000000+tests*8000,true,f));assert(backend.show_bindings()&&steam.current_set==10+steam.selected);++tests;
 }
 bool other=true;std::thread worker([&](){other=backend.show_bindings();});worker.join();assert(!other);++tests;assert(backend.close()&&!backend.show_bindings());++tests;
 auto frame=[](unsigned n,uint64_t us){deck174::Frame f;f.source=deck174::Source::Simulation;f.epoch=1;f.sequence=n;f.sample_us=us;f.connected=f.focused=f.native_bindings=true;f.active_buttons=deck174::all_buttons;return f;};
 for(auto context:{deck174::Context::RTS,deck174::Context::Possession}){
  deck181::Controls controls;auto f=frame(1,1000000);controls.sample(f,context,f.sample_us);f=frame(2,1008000);f.buttons=deck174::bit(deck174::Button::View);auto o=controls.sample(f,context,f.sample_us);assert(!o.show_controls&&!o.held(deck181::K));f=frame(3,1100000);o=controls.sample(f,context,f.sample_us);assert(!o.show_controls&&o.held(deck181::K)&&!o.help_open);++tests;
  unsigned sequence=4;for(uint64_t us=1200000;us<1800000;us+=8000){f=frame(sequence++,us);f.buttons=deck174::bit(deck174::Button::View);o=controls.sample(f,context,f.sample_us);if(us<1800000)assert(!o.held(deck181::K)&&!o.show_controls);}
  // Ensure at least600ms elapsed since initial press (steps end at1.8s).
  f=frame(sequence++,1800000);f.buttons=deck174::bit(deck174::Button::View);o=controls.sample(f,context,f.sample_us);assert(!o.held(deck181::K)&&o.help_open&&o.help_blocked);
  f=frame(sequence++,1808000);o=controls.sample(f,context,f.sample_us);assert(!o.held(deck181::K)&&!o.show_controls&&o.help_open);
  f=frame(sequence++,1816000);f.buttons=deck174::bit(deck174::Button::B);o=controls.sample(f,context,f.sample_us);assert(!o.help_open&&o.help_blocked&&!o.held(deck181::M)&&!o.held(deck181::Escape));++tests;
 }
 std::cout<<"PASS "<<tests<<" View186 tap HUD/hold first-party guide/B-close blocking plus retained unused Steam API ABI checks; fake Steam only\n";
}
