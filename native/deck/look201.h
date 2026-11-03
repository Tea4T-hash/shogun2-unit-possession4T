#pragma once
#include <windows.h>
#include <algorithm>
#include <cmath>
namespace deck201 {
struct Look {float x{},y{};};
// UI-thread controller samples are float mouse-equivalent displacements.
// Retain both axes under one lock until the existing camera-input consumer
// takes them. Whole-count conversion causes visible stop/jump steps at ADS.
// No acceleration, extrapolation, filter tail or change to stick sensitivity.
class LookQueue {
 SRWLOCK lock_=SRWLOCK_INIT;double x_{},y_{};
public:
 void clear(){AcquireSRWLockExclusive(&lock_);x_=y_=0;ReleaseSRWLockExclusive(&lock_);}
 void add(float x,float y){
  if(!std::isfinite(x)||!std::isfinite(y)){clear();return;}
  AcquireSRWLockExclusive(&lock_);
  // Match the existing per-sample400-count safety bound. A bounded batch
  // cannot grow forever if the consumer stops; stale/lifecycle paths clear it.
  x_=std::clamp(x_+double(std::clamp(x,-400.f,400.f)),-12800.,12800.);
  y_=std::clamp(y_+double(std::clamp(y,-400.f,400.f)),-12800.,12800.);
  ReleaseSRWLockExclusive(&lock_);
 }
 Look take(){AcquireSRWLockExclusive(&lock_);Look value{float(x_),float(y_)};x_=y_=0;ReleaseSRWLockExclusive(&lock_);return value;}
};
}
