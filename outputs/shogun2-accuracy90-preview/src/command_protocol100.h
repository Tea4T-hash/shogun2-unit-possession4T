#pragma once
#include <cstdint>
#include <cmath>
namespace command100 {
enum class Feedback:std::uint32_t {None,Move,Formation,Stopped,Holding,Following,FollowEnded,FireHold,FireAtWill,FireCommand,NoGround,Busy,Unavailable,SkirmishOn,SkirmishOff};
constexpr std::uint32_t magic=0x30304355u;
#pragma pack(push,8)
struct Status {
    std::uint32_t magic{},version{},size{},pid{};volatile long sequence{};
    std::uint32_t lease{},actor{},unit{},move_mode{},fire_mode{},feedback{},placing{};
    std::uint64_t tick{},feedback_until{};
    std::uint32_t volley{},releases{},blocked{},error{};
};
#pragma pack(pop)
static_assert(sizeof(Status)==80,"Command100 numeric layout changed");
inline bool valid(const Status& s,unsigned pid,unsigned lease,unsigned actor,unsigned unit,std::uint64_t now){
    return s.magic==magic&&s.version==100&&s.size==sizeof s&&s.pid==pid&&s.lease==lease&&s.actor==actor&&s.unit==unit&&
        s.move_mode<=4&&s.fire_mode<=2&&s.feedback<=static_cast<std::uint32_t>(Feedback::SkirmishOff)&&s.placing<=1&&now>=s.tick&&now-s.tick<=1000;
}
}
