#pragma once
#include <cstdint>
namespace deck181 {
// A Deck entry may use the already verified owning-window dispatch. Existing
// camera observations, if present, must still agree; never invent observations.
inline uint32_t entry_owner(uint32_t window,uint32_t matrix,uint32_t input,uint32_t verified_dispatch){
 if(!window||(matrix&&matrix!=window)||(input&&input!=window))return 0;
 return matrix?matrix:input?input:verified_dispatch==window?window:0;
}
struct PendingEntry {
 uint64_t began{};
 bool expired(bool armed,uint64_t now){
  if(!armed){began=0;return false;}
  if(!began||now<began){began=now;return false;}
  return now-began>=1500;
 }
};
// Deployment is identified by the fully initialized native battlefield graph,
// rather than guessing its phase enum. Menu/campaign cameras cannot satisfy it.
inline bool battle_camera(bool phase,bool root,bool environment,bool manager,bool rts,bool view,bool selection){
 return phase&&root&&environment&&manager&&rts&&view&&selection;
}
}
