#pragma once
#include <cmath>
#include <cstdint>
namespace deck181 {
inline constexpr unsigned campaign_slots[]={0x94,0x98,0x9c,0xa0,0x108,0x10c};
inline constexpr unsigned campaign_functions[]={0x8c09e0,0x8c08a0,0x8c0d10,0x8c0b20,0x8ef030,0x8f0490};
template<class Read>uintptr_t campaign_camera(uintptr_t base,Read read){
 // Native 11499f0/1135690 -> manager+a98, stack+14 -> 6a2810.
 // 1bc8fe4 is a MUTABLE pointer slot, never treated as immutable code.
 auto global=read(base+0x1bc8fe4);if(!global)return 0;
 auto manager=read(global+0xa98),stack=manager?read(manager+0x14):0;if(!stack)return 0;
 auto index=read(stack+0x10),array=read(stack+0xc);if(index>63||!array)return 0;
 auto node=read(array+index*4);if(!node)return 0;
 auto count=read(node+4),cameras=read(node+8);if(!count||count>64||!cameras)return 0;
 auto camera=read(cameras+(count-1)*4);if(!camera||read(camera)!=base+0x16209f8||!read(camera+0x138))return 0;
 for(unsigned i=0;i<6;++i)if(read(base+0x16209f8+campaign_slots[i])!=base+campaign_functions[i])return 0;
 return camera;
}
struct CampaignStep {uintptr_t camera{};float x{},y{},yaw{},zoom{};uint64_t tick{};};
inline bool fresh_campaign(const CampaignStep& step,uintptr_t current,uint64_t now){return current&&step.camera==current&&step.tick&&now>=step.tick&&now-step.tick<=100;}
template<class Apply>bool apply_campaign(const CampaignStep& step,uintptr_t current,uint64_t now,Apply apply){
 if(!fresh_campaign(step,current,now)||!std::isfinite(step.x)||!std::isfinite(step.y)||!std::isfinite(step.yaw)||!std::isfinite(step.zoom)||std::fabs(step.x)>4||std::fabs(step.y)>4||std::fabs(step.yaw)>.4f||std::fabs(step.zoom)>14.4f)return false;
 if(step.y)apply(step.y>0?0x94:0x98,std::fabs(step.y));
 if(step.x)apply(step.x>0?0x9c:0xa0,std::fabs(step.x));
 if(step.yaw)apply(0x10c,step.yaw);return true;
}
}
