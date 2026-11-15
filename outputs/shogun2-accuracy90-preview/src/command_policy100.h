#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "command_protocol100.h"
namespace command100 {
constexpr float tau=6.2831853071795864769f;
enum class MoveMode:std::uint32_t {None,Advance,Stop,Hold,Follow};
enum class FireMode:std::uint32_t {Hold,AtWill,OnCommand};
struct Point {float x{},y{},z{};};
inline bool finite(const Point& p){return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z)&&std::abs(p.x)<100000&&std::abs(p.y)<100000&&std::abs(p.z)<100000;}
inline std::uint16_t bearing(float yaw){return std::uint16_t(std::int32_t(std::lround(std::remainder(double(yaw),double(tau))*65536.0/double(tau))));}
inline FireMode next_fire(FireMode mode){return FireMode((std::uint32_t(mode)+1)%3);}
// The native unit keeps its own target/reload/animation logic. Command mode
// authorizes one release per actual actor identity after a successful player shot.
// Entries are discarded for each new volley, mode or lease, so churn cannot
// accumulate tombstones across the session. No pointers are dereferenced here.
struct ActorKey {std::uintptr_t actor{},holder{};std::uint32_t serial{};};
inline bool same(const ActorKey& a,const ActorKey& b){return a.actor==b.actor&&a.holder==b.holder&&a.serial==b.serial;}
struct Volley {
    static constexpr unsigned capacity=4096;
    ActorKey consumed[capacity]{};std::uint32_t epoch{};unsigned count{};
    void reset(){std::memset(consumed,0,sizeof consumed);epoch=0;count=0;}
    void shot(){std::memset(consumed,0,sizeof consumed);if(!++epoch)++epoch;count=0;}
    unsigned slot(const ActorKey& key)const{return unsigned((key.actor>>4)^(key.holder>>3)^(std::uint64_t(key.serial)*2654435761u))&(capacity-1);}
    bool available(const ActorKey& key)const{
        if(!epoch||!key.actor||!key.holder)return false;
        unsigned i=slot(key);for(unsigned n=0;n<capacity;++n,i=(i+1)&(capacity-1)){
            if(!consumed[i].actor)return true;if(same(consumed[i],key))return false;
        }return false;
    }
    bool consume(const ActorKey& key){
        if(!available(key))return false;
        unsigned i=slot(key);for(unsigned n=0;n<capacity;++n,i=(i+1)&(capacity-1))if(!consumed[i].actor){consumed[i]=key;++count;return true;}
        return false;
    }
};
// Native DIK scan codes, separate from the existing equipment-key protocol.
constexpr unsigned scans[5]={0x10,0x12,0x2c,0x2e,0x13};
inline unsigned key_bit(unsigned key){return key=='Q'?1u:key=='E'?2u:key=='Z'?4u:key=='C'?8u:key=='R'?16u:0u;}
struct KeyFilter {
    unsigned held{};
    unsigned apply(std::uint8_t* current,std::uint8_t* previous,bool owned,bool valid){
        if(!valid||!current||!previous)return 0;
        unsigned physical=0;for(unsigned i=0;i<5;++i)if(current[scans[i]]&0x80)physical|=1u<<i;
        const unsigned mask=(owned?31u:0u)|held;held=owned?physical:held&physical;
        for(unsigned i=0;i<5;++i)if(mask&(1u<<i))current[scans[i]]=previous[scans[i]]=0;
        return mask&physical;
    }
};
struct Edges {
    unsigned held{},drain{};
    unsigned sample(unsigned physical,bool active){
        if(!active){drain|=physical;held=0;return 0;}
        drain&=physical;const unsigned accepted=physical&~drain;
        unsigned pressed=accepted&~held;held=accepted;return pressed;
    }
};
// A completed first tap arms a second press; a pair consumes the arm, so a
// third press starts a new pair. Window repeat never reaches this policy.
struct Tap102 {
    bool armed{};std::uint64_t tick{};unsigned lease{};
    void reset(){*this={};}
    void arm(std::uint64_t now,unsigned generation){armed=true;tick=now;lease=generation;}
    bool consume(std::uint64_t now,unsigned interval,unsigned generation){
        bool pair=armed&&lease==generation&&now>=tick&&now-tick<=interval;reset();return pair;
    }
};
// Catch-up uses the current peer formation center. Hysteresis avoids repeated
// walk/run task changes at one boundary. Individual path/formation AI stays native.
struct FollowPace102 {
    bool catchup{};
    void reset(){*this={};}
    bool update(float gap,bool leader_running){
        if(std::isfinite(gap)&&gap>=0){if(gap>4.f)catchup=true;else if(gap<=2.f)catchup=false;}
        return catchup||leader_running;
    }
};
// Follow tracks the leader's translation, never mouse-look/body facing. The
// first order uses the captured heading; later headings come from travel. A
// stationary order finishes without continual rotation/path/task restarts.
struct FollowTarget {
    bool valid{},running102{};Point position{};float yaw{};std::uint64_t next{};
    void reset(){*this={};}
    bool translated(Point p)const{return !valid||std::hypot(p.x-position.x,p.z-position.z)>=.75f;}
    bool changed(Point p,std::uint64_t now,bool running=false)const{
        return finite(p)&&now>=next&&(!valid||translated(p)||running!=running102);
    }
    Point target(Point p)const{return translated(p)?p:position;}
    float heading(Point p,float initial)const{return !valid?initial:translated(p)?std::atan2(p.x-position.x,p.z-position.z):yaw;}
    void accepted(Point p,float angle,std::uint64_t now,bool running=false){valid=true;position=p;yaw=angle;running102=running;next=now+250;}
};
}
