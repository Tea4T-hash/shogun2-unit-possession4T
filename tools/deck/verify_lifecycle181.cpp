// Actual production request/lease and backend ABI; no Steam/game worker starts.
#define S2UP_CONTEXT181_EMBEDDED 1
#include "verify_context_backend181.cpp"
#include "../../native/deck/ownership177.h"
#include "../../native/deck/campaign181.h"
#include <map>
#include <vector>
struct CountedSteam181:ContextSteam181 {
 static inline int live{},peak{};
 bool init(bool)override{++live;peak=std::max(peak,live);assert(live==1);closed=false;return true;}
 bool shutdown()override{--live;assert(live==0);return FakeSteam::shutdown();}
};
int main(){unsigned checks=0;deck177::Lease front,battle;CountedSteam181 fs,bs;SteamBackend f,b;Frame frame;
 assert(deck177::battle_request().set(false));
 for(unsigned round=0;round<100;++round){
  // campaign -> deployment -> battle -> results -> campaign -> next battle
  for(auto ctx:{Context::Menu,Context::Campaign}){assert(deck177::claim_frontend(front));assert(f.select_context(ctx));if(!f.initialized())assert(f.initialize({&fs},"inert",true));assert(f.sample(1000000+round*100000,true,frame));}
  assert(deck177::battle_request().set(true));assert(!deck177::claim_battle(battle));assert(!deck177::claim_frontend(front));
  // Existing lease must stay owned until its SAME worker has shut input down.
  assert(f.close());front.release();assert(deck177::claim_battle(battle));assert(!deck177::claim_frontend(front));assert(b.select_context(Context::RTS));assert(b.initialize({&bs},"inert",true));assert(b.sample(1010000+round*100000,true,frame));
  assert(b.select_context(Context::Possession));assert(b.sample(1020000+round*100000,true,frame));assert(b.select_context(Context::Menu));assert(b.sample(1030000+round*100000,true,frame));
  assert(deck177::battle_request().set(false));assert(!deck177::claim_frontend(front));assert(b.close());battle.release();++checks;
 }
 assert(CountedSteam181::live==0&&CountedSteam181::peak==1);++checks;
 const uintptr_t base=0x10000000;std::map<uintptr_t,uintptr_t> mem;
 mem[base+0x1bc8fe4]=0x20000;mem[0x20a98]=0x30000;mem[0x30014]=0x40000;mem[0x40010]=0;mem[0x4000c]=0x50000;mem[0x50000]=0x60000;mem[0x60004]=1;mem[0x60008]=0x70000;mem[0x70000]=0x80000;mem[0x80000]=base+0x16209f8;mem[0x80138]=0x90000;
 for(unsigned i=0;i<6;++i)mem[base+0x16209f8+deck181::campaign_slots[i]]=base+deck181::campaign_functions[i];
 auto read=[&](uintptr_t at){return mem[at];};assert(deck181::campaign_camera(base,read)==0x80000);++checks;
 for(auto at:{0x60004u,0x60008u,0x80000u,0x80138u}){auto old=mem[at];mem[at]=0;assert(!deck181::campaign_camera(base,read));mem[at]=old;++checks;}
 mem[0x60004]=65;assert(!deck181::campaign_camera(base,read));mem[0x60004]=1;++checks;
 for(auto at:deck181::campaign_slots){auto key=base+0x16209f8+at;auto old=mem[key];mem[key]+=4;assert(!deck181::campaign_camera(base,read));mem[key]=old;++checks;}
 for(auto input:{std::pair<float,float>{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,-1},{1,-1},{-1,1}}){deck181::CampaignStep s{0x80000,input.first,input.second,0,0,1000};std::vector<unsigned> calls;assert(deck181::apply_campaign(s,0x80000,1050,[&](unsigned slot,float amount){assert(amount==1);calls.push_back(slot);}));if(input.first)assert(std::find(calls.begin(),calls.end(),input.first>0?0x9c:0xa0)!=calls.end());if(input.second)assert(std::find(calls.begin(),calls.end(),input.second>0?0x94:0x98)!=calls.end());++checks;}
 deck181::CampaignStep step{0x80000,0,0,.1f,0,1000};std::vector<unsigned> slots;assert(deck181::apply_campaign(step,0x80000,1050,[&](unsigned slot,float){slots.push_back(slot);}));assert(slots==std::vector<unsigned>({0x10c}));++checks;
 assert(!deck181::apply_campaign(step,0x80004,1050,[](unsigned,float){assert(false);}));assert(!deck181::apply_campaign(step,0x80000,1101,[](unsigned,float){assert(false);}));step.x=NAN;assert(!deck181::apply_campaign(step,0x80000,1050,[](unsigned,float){assert(false);}));++checks;
 std::cout<<"PASS "<<checks<<" lifecycle181 repeated ownership/native campaign dispatch checks; no hardware claim\n";
}
