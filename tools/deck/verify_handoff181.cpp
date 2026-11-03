#include "../../native/deck/handoff181.h"
#include "../../native/deck/battle_ready181.h"
#include "../../native/outputs/shogun2-accuracy90-preview/src/camera_entry_gate.h"
#include <cassert>
#include <iostream>
#include <map>
int main(){unsigned checks=0;
 assert(!camera_entry_owner(12,0,0)&&deck181::entry_owner(12,0,0,12)==12);++checks;
 assert(!deck181::entry_owner(12,0,0,13)&&!deck181::entry_owner(0,0,0,12));++checks;
 assert(!deck181::entry_owner(12,13,0,12)&&!deck181::entry_owner(12,0,13,12));++checks;
 assert(deck181::entry_owner(12,12,0,0)==12&&deck181::entry_owner(12,0,12,0)==12);++checks;
 assert(camera_entry_thread(12,12,deck181::entry_owner(12,0,0,12),0)&&!camera_entry_thread(13,12,12,0)&&!camera_entry_thread(12,12,12,1));++checks;
 deck181::PendingEntry pending;assert(!pending.expired(true,1000)&&!pending.expired(true,2499)&&pending.expired(true,2500));++checks;
 assert(!pending.expired(false,2501)&&!pending.began&&!pending.expired(true,4000)&&!pending.expired(true,1000));++checks;
 constexpr uintptr_t b=0x10000000;
 std::map<uintptr_t,uintptr_t> m={{b+0x1bc8180,0x20000000},{0x20000000,b+0x170d284},{0x20000018,b+0x170d290},{0x2000010c,b+0x170d304},{0x20000110,0x21000000},{0x21000008,0x22000000},{0x220000b4,0x23000000},{0x23000014,3},{0x210281fc,0x24000000},{0x24000000,b+0x15d26fc},{0x240008e0,0x25000000},{0x240008cc,0x25000000},{0x25000000,b+0x15d1a74},{0x240008f8,0x26000000},{0x2102826c,0x27000000}};
 auto read=[&](uintptr_t a){auto i=m.find(a);return i==m.end()?uintptr_t(0):i->second;};
 assert(deck181::battle_ready(b,read));++checks; // Observed native deployment phase3.
 m[0x25000000]=b+0x15d22e8;assert(!deck181::battle_ready(b,read));m[0x25000000]=b+0x15d1a74;++checks;
 for(auto at:{uintptr_t(0x20000000),uintptr_t(0x20000018),uintptr_t(0x2000010c),uintptr_t(0x24000000)}){auto old=m[at];m[at]=0;assert(!deck181::battle_ready(b,read));m[at]=old;}++checks;
 m[0x240008f8]=0;m[0x2102826c]=0;assert(deck181::battle_ready(b,read));++checks;
 for(auto phase:{0,1,2,4,9,99}){m[0x23000014]=phase;assert(!deck181::battle_ready(b,read));}++checks;
 for(auto phase:{5,6,7,8}){m[0x23000014]=phase;assert(deck181::battle_ready(b,read));}++checks;
 std::cout<<"PASS "<<checks<<" handoff181 owner/timeout/initialized-deployment checks; no game execution\n";
}
