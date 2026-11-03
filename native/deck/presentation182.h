#pragma once
#include <cstdint>
namespace deck182 {
// At most one uncached decode per poll, three attempts per slot and two-second
// retry spacing. Reuse the menu's shared cached artwork during battle handoff.
struct Presentation {
 unsigned next{},attempts[30]{};uint64_t retry[30]{},wake{};bool complete{};
 template<class Ready,class Copy>void poll(uint64_t now,Ready ready,Copy copy){
  if(complete||now<wake)return;
  bool all_ready=true;uint64_t earliest=~uint64_t(0);
  for(unsigned seen=0;seen<30;++seen){
   unsigned slot=next;next=(next+1)%30;
   if(ready(slot))continue;all_ready=false;
   if(attempts[slot]>=3)continue;
   if(now<retry[slot]){if(retry[slot]<earliest)earliest=retry[slot];continue;}
   ++attempts[slot];retry[slot]=now+2000;copy(slot);return;
  }
  complete=all_ready;wake=earliest;
 }
};
}
