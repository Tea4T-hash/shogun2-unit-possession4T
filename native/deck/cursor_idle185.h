#pragma once
#include <cstdint>
namespace deck185 {
// Visibility belongs to the game/OS. Never change ShowCursor's global counter.
struct CursorIdle {
 uint64_t last{};
 void activity(uint64_t now){last=now;}
 bool ready(uint64_t now,bool visible){if(!last){last=now;return false;}if(now<last){last=now;return false;}return !visible&&now-last>=1500;}
};
}
