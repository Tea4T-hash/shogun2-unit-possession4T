// Exercise production SteamInput006 context selection through a fake ABI only.
#define main old_abi_fixture181
#include "verify174.cpp"
#undef main
struct ContextSteam181:FakeSteam {
 unsigned selected{};bool missing{};unsigned lookups{};
 static unsigned context(const char* text,const char* suffix){const char* names[]={"Menu","Campaign","RTS","Possession"};for(unsigned i=0;i<4;++i)if(std::string(text)==std::string(names[i])+suffix)return i+1;return 0;}
 virtual uint64_t set(const char* name)override{selected=context(name,"181");assert(selected);return 10+selected;}
 virtual uint64_t digital_handle(const char* name)override{++lookups;if(missing)return 0;const char* prefix=selected==1?"Menu_":selected==2?"Campaign_":selected==3?"RTS_":"Possession_";std::string key(name);assert(key.rfind(prefix,0)==0);return selected*1000+FakeSteam::digital_handle(key.substr(strlen(prefix)).c_str());}
 virtual uint64_t analog_handle(const char* name)override{const char* prefix=selected==1?"Menu_":selected==2?"Campaign_":selected==3?"RTS_":"Possession_";std::string key(name);assert(key.rfind(prefix,0)==0);return selected*1000+FakeSteam::analog_handle(key.substr(strlen(prefix)).c_str());}
 virtual steam006::Digital digital(uint64_t h,uint64_t a)override{assert(a/1000==selected);return FakeSteam::digital(h,a%1000);}
 virtual int origins(uint64_t h,uint64_t set,uint64_t a,int* out)override{assert(set==10+selected&&a/1000==selected);return FakeSteam::origins(h,set,a%1000,out);}
 virtual steam006::Analog analog(uint64_t h,uint64_t a)override{assert(a/1000==selected);return FakeSteam::analog(h,a%1000);}
 virtual int aorigins(uint64_t h,uint64_t set,uint64_t a,int* out)override{assert(set==10+selected&&a/1000==selected);return FakeSteam::aorigins(h,set,a%1000,out);}
};
#ifndef S2UP_CONTEXT181_EMBEDDED
int main(){unsigned checks=0;ContextSteam181 fake;SteamBackend backend;assert(backend.select_context(Context::RTS));assert(backend.initialize({&fake},"inert fixture",true));Frame f;assert(backend.sample(1000000,true,f)&&fake.current_set==13);++checks;
 uint64_t epoch=f.epoch;assert(backend.select_context(Context::Possession));assert(backend.sample(1008000,true,f)&&fake.current_set==14&&f.epoch>epoch);++checks;
 epoch=f.epoch;assert(backend.select_context(Context::Possession));assert(backend.sample(1016000,true,f)&&f.epoch==epoch);++checks;
 assert(backend.select_context(Context::Campaign));assert(backend.sample(1024000,true,f)&&fake.current_set==12);assert(backend.select_context(Context::Menu));assert(backend.sample(1032000,true,f)&&fake.current_set==11);++checks;
 fake.missing=true;assert(backend.select_context(Context::RTS));assert(!backend.sample(1040000,true,f)&&backend.state()==BackendState::WaitingBindings);assert(backend.close()&&fake.closed);++checks;
 std::cout<<"PASS "<<checks<<" context181 production backend transitions/prefixed handles; fake ABI only\n";
}

#endif
