#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace deck202 {
struct Key {
 uint32_t lease{},actor{},unit{};
 bool valid()const{return lease&&unit;} // Native actor serial0 is valid.
 bool operator==(const Key& r)const{return lease==r.lease&&actor==r.actor&&unit==r.unit;}
};
struct Motion {float x{},y{};};
// A stick is a turn rate, not a sequence of mouse displacements. Publish the
// newest valid rate; integrate once over elapsed owned-camera time. There is
// no smoothing tail and no replay of motion accumulated during a stalled UI.
class LookRate {
 SRWLOCK lock_=SRWLOCK_INIT;
 Key key_{};uint64_t epoch_{},sample_{},frame_{};uint32_t stale_{100000};Motion rate_{};
 void reset(){key_={};epoch_=sample_=frame_=0;rate_={};}
public:
 void clear(){AcquireSRWLockExclusive(&lock_);reset();ReleaseSRWLockExclusive(&lock_);}
 void publish(Key key,uint64_t epoch,uint64_t sample,Motion rate,uint32_t stale){
  AcquireSRWLockExclusive(&lock_);
  if(!key.valid()||!epoch||!sample||stale<20000||stale>250000||!std::isfinite(rate.x)||!std::isfinite(rate.y)||std::fabs(rate.x)>16000||std::fabs(rate.y)>16000){reset();}
  else {
   if(!(key==key_)||epoch!=epoch_||sample<sample_)frame_=0;
   key_=key;epoch_=epoch;sample_=sample;rate_=rate;stale_=stale;
  }
  ReleaseSRWLockExclusive(&lock_);
 }
 Motion advance(Key key,uint64_t now,bool allowed=true){
  AcquireSRWLockExclusive(&lock_);Motion result{};
  if(!allowed||!key.valid()||!(key==key_)||!sample_||now<sample_||now-sample_>stale_){frame_=0;}
  else {
   if(frame_&&now>=frame_){
    // Same50ms pause bound as the previous controller sample. Long pauses
    // discard excess time; they never produce a delayed catch-up rotation.
    float dt=float(std::min<uint64_t>(now-frame_,50000))*1e-6f;
    result={rate_.x*dt,rate_.y*dt};
   }
   frame_=now;
  }
  ReleaseSRWLockExclusive(&lock_);return result;
 }
};
}
