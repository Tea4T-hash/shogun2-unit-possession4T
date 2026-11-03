#pragma once
#include <cstdint>
#include <algorithm>
#include <cwchar>
namespace climb97 {
constexpr uint32_t Magic=0x37394c43;
enum Mode:uint32_t {Off,NearbyUp,NearbyDown,Climbing,Crest,Blocked,Approaching};
struct Status {uint32_t magic=Magic,version=97,size=sizeof(Status),pid{};volatile long sequence{};uint32_t lease{},actor{},unit{},mode{};uint64_t tick{};float progress{};int32_t direction{};uint32_t reserved[2]{};};
static_assert(sizeof(Status)==64,"Climb97 numeric optional channel");
inline void mapping_name(wchar_t (&out)[96],uint32_t pid){swprintf_s(out,L"Local\\Shogun2Climb97-%u",pid);}
inline bool valid(const Status& s,uint32_t pid,uint32_t lease,uint32_t actor,uint32_t unit,uint64_t now){return s.magic==Magic&&s.version==97&&s.size==sizeof s&&s.pid==pid&&s.lease==lease&&s.actor==actor&&s.unit==unit&&s.mode<=Approaching&&now>=s.tick&&now-s.tick<=500;}
inline int direction(unsigned input){return bool(input&1)==bool(input&2)?0:(input&1)?1:-1;}
inline float reverse_target(float previous,float advanced){return std::clamp(2*previous-advanced,0.f,1.f);}
}
