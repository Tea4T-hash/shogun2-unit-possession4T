// Run production backend against a fake ABI that refuses unused channels.
#define S2UP_CONTEXT181_EMBEDDED 1
#include "verify_context_backend181.cpp"
struct SparseSteam182:ContextSteam181 {
 Context context_value{Context::Inactive};unsigned queried{};
 virtual steam006::Digital digital(uint64_t h,uint64_t a)override{auto i=unsigned(a%1000)-1;assert(deck182::buttons(context_value)&(uint64_t(1)<<i));++queried;return ContextSteam181::digital(h,a);}
 virtual steam006::Analog analog(uint64_t h,uint64_t a)override{auto i=unsigned(a%1000)-100;assert(deck182::analogs(context_value)&(1u<<i));return ContextSteam181::analog(h,a);}
};
int main(){SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);_set_error_mode(_OUT_TO_STDERR);unsigned checks=0;SparseSteam182 fake;SteamBackend backend;Frame frame;uint64_t now=1000000;
 for(auto context:{Context::Menu,Context::Campaign,Context::RTS,Context::Possession}){fake.context_value=context;assert(backend.select_context(context));if(!backend.initialized())assert(backend.initialize({&fake},"inert native fixture",true));assert(backend.sample(now,true,frame)&&frame.native_bindings);assert(!(frame.active_buttons&~deck182::buttons(context)));now+=8000;++checks;}
 fake.bad_origin=unsigned(Button::R4);assert(!backend.sample(now,true,frame));++checks;
 fake.bad_origin=-1;fake.missing=true;assert(!backend.sample(now+8000,true,frame));++checks;
 assert(backend.close());++checks;
 std::cout<<"PASS "<<checks<<" binding_profiles182 real-action/required-origin checks; fake ABI only\n";
}
