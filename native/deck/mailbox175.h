#pragma once
#include "input174.h"

namespace deck175 {
// Same-process numeric samples only. The runtime supplies its short SRW lock;
// native game calls never run while that lock is held. An overflow requires a
// neutral reacquisition, rather than replaying an old press without its release.
struct Mailbox {
    static constexpr unsigned capacity=32;
    std::array<deck174::Frame,capacity> frames{};
    unsigned read{},count{};bool discontinuity{};
    uint64_t last_sequence{},last_epoch{};
    bool put(const deck174::Frame& frame) {
        if(frame.epoch==last_epoch&&frame.sequence<=last_sequence)return false;
        if(frame.epoch!=last_epoch){read=count=0;discontinuity=true;}
        last_epoch=frame.epoch;last_sequence=frame.sequence;
        if(count==capacity){read=count=0;discontinuity=true;}
        frames[(read+count)%capacity]=frame;++count;return true;
    }
    void invalidate(){read=count=0;last_sequence=last_epoch=0;discontinuity=true;}
    unsigned take(std::array<deck174::Frame,capacity>& out,bool& reset) {
        reset=discontinuity;discontinuity=false;unsigned copied=count;
        for(unsigned i=0;i<copied;++i)out[i]=frames[(read+i)%capacity];
        read=count=0;return copied;
    }
};
}
