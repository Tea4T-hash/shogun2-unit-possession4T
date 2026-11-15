#pragma once
#include "ownership177.h"
namespace deck222 {
// Bridge-local barrier: the loader and menu worker are in this same module.
// Keep it independent of the adapter's later battle request, which may reset
// while its initial window/context is not yet eligible. No hook is chained.
class StartupHandoff {
 volatile LONG requested_{};
public:
 bool active()const{return InterlockedCompareExchange(const_cast<volatile LONG*>(&requested_),0,0)!=0;}
 bool request(){return InterlockedCompareExchange(&requested_,1,0)==0;}
 void release(){InterlockedExchange(&requested_,0);}
 template<class Busy,class Tick,class Wait>bool wait(Busy busy,Tick tick,Wait pause,unsigned timeout=3000){
  const auto began=tick();
  while(busy()){
   const auto now=tick();if(now<began||now-began>=timeout)return false;
   pause();
  }return true;
 }
};
inline StartupHandoff& startup(){static StartupHandoff value;return value;}
class StartupGuard {
 bool held_{};
public:
 StartupGuard()=default;StartupGuard(const StartupGuard&)=delete;StartupGuard& operator=(const StartupGuard&)=delete;
 bool acquire(bool deck){
  if(!deck)return true;if(held_||!startup().request())return false;held_=true;
  return startup().wait([](){return deck177::busy(deck177::Owner::Frontend);},[](){return GetTickCount64();},[](){Sleep(8);});
 }
 void release(){if(held_){startup().release();held_=false;}}
 ~StartupGuard(){release();}
};
}
