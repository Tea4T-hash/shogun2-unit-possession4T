#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstring>
#include <cwchar>
#include "qte_core.h"
#include "qte_timeline.h"
namespace qte { namespace transport {
constexpr uint32_t Magic=0x31515453,Version=91;
// Numeric publication only: actor/unit serials, never native pointers. This
// channel has its own writer sequence, independent of movement telemetry.
struct Payload {
    uint32_t pid{},enabled{},occupied{},error{};
    uint64_t lease{};
    Snapshot prompt{};TimePoint anchor{};
    double published_wall{};
};
struct alignas(8) Publication {
    volatile LONG sequence{};
    uint32_t magic=Magic,version=Version,size=sizeof(Publication);
    Payload payload{};
};
inline void mapping_name(wchar_t* out,size_t count,DWORD pid){
    swprintf_s(out,count,L"Local\\Shogun2MeleeQTE%u-%lu",Version,pid);
}
// Caller serializes writers. Readers map the channel READ ONLY; using a
// read/modify/write atomic to read the sequence would fault on that mapping.
inline void publish(Publication& target,const Payload& payload){
    InterlockedIncrement(&target.sequence);MemoryBarrier();
    target.payload=payload;MemoryBarrier();InterlockedIncrement(&target.sequence);
}
inline bool read(const Publication& source,Payload& output){
    if(source.magic!=Magic||source.version!=Version||source.size!=sizeof(Publication))return false;
    for(unsigned i=0;i<4;++i){LONG sequence=source.sequence;if(sequence&1)continue;
        MemoryBarrier();Payload copy;memcpy(&copy,&source.payload,sizeof copy);MemoryBarrier();
        if(sequence==source.sequence){output=copy;return true;}
    }return false;
}
inline bool valid(const Payload& value){
    const auto& s=value.prompt;const auto& a=value.anchor;
    if(!value.pid||!std::isfinite(value.published_wall)||value.published_wall<0||value.enabled>1||value.occupied>1)return false;
    if(!s.active)return true;
    return value.enabled&&value.occupied&&value.lease&&s.serial&&s.opponent_unit&&
        (attacking(s.action)||s.action==Action::Defend)&&uint32_t(s.result)<=uint32_t(Result::Blue)&&
        std::isfinite(s.response_elapsed)&&s.response_elapsed>=0&&s.response_elapsed<=MaximumResponseSeconds95&&s.response_elapsed<=response_deadline(s)+1e-6&&
        (s.result!=Result::Pending||s.response_elapsed<ResponseWindowSeconds91||s.elapsed<2.*s.duration)&&
        (s.result!=Result::Timeout||(s.response_elapsed+1e-6>=ResponseWindowSeconds91&&s.elapsed+1e-5>=2.*s.duration))&&
        std::isfinite(s.elapsed)&&s.elapsed>=0&&std::isfinite(s.duration)&&s.duration>=.30f&&s.duration<=2.21f&&
        std::isfinite(s.ideal)&&s.ideal>0&&s.ideal<s.duration&&
        std::isfinite(s.tolerance)&&s.tolerance>0&&s.tolerance<s.duration*.5f&&
        std::isfinite(s.placement)&&s.placement>=0&&s.placement<=1&&
        std::isfinite(s.zone_width)&&s.zone_width>0&&s.zone_width<1&&
        std::isfinite(s.advantage)&&s.advantage>=-1&&s.advantage<=1&&a.valid&&a.battle&&
        std::isfinite(a.wall)&&std::isfinite(a.simulation)&&std::isfinite(a.horizon)&&
        a.horizon>=a.simulation&&a.horizon-a.simulation<=.201&&
        std::isfinite(a.speed)&&a.speed>=0&&a.speed<=16;
}
inline bool presentation(const Payload& value,double now,Snapshot& output,double* simulation=nullptr){
    if(!valid(value)||!value.prompt.active||!std::isfinite(now)||now<value.published_wall||
       now-value.published_wall>(value.prompt.result==Result::Canceled?1.15:.5))return false;
    const auto& anchor=value.anchor;
    // The publication anchors elapsed to the exact interval already judged by
    // the native coordinator. Display and click projection use this SAME rule.
    const double projected=interpolate(anchor,now);
    Snapshot copy=value.prompt;
    if(copy.result==Result::Pending&&!copy.paused&&!anchor.paused)
        project_response(copy,projected-anchor.simulation);
    copy.paused=copy.paused||anchor.paused;
    output=copy;if(simulation)*simulation=projected;return true;
}
inline bool click(const Payload& value,double wall,bool left,bool right,bool focused,Click& output){
    Snapshot shown;double simulation{};
    if(!presentation(value,wall,shown,&simulation)||shown.result!=Result::Pending)return false;
    output={shown.serial,value.anchor.battle,simulation,left,right,focused,shown.paused};return true;
}
} }
