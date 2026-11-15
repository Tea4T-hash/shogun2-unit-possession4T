#pragma once
#include "bow_draw142.h"
static SRWLOCK bow_input_lock142=SRWLOCK_INIT;
static bow142::Input bow_input142;
static bow142::Release bow_release142;
static volatile LONG bow_family142=-1;
struct BowTicket142 {bow142::Key key{};uintptr_t record{};float factor{1};uint64_t serial{};};
static BowTicket142 bow_ticket142;
static double bow_clock142(){LARGE_INTEGER now,frequency;QueryPerformanceCounter(&now);QueryPerformanceFrequency(&frequency);return double(now.QuadPart)/double(frequency.QuadPart);}
static bow142::Key bow_key142(){return state?bow142::Key{state->lease_generation,state->actor_id,state->unit_id}:bow142::Key{};}
static void bow_reset142(){
 AcquireSRWLockExclusive(&bow_input_lock142);bow_input142.cancel();bow_release142={};ReleaseSRWLockExclusive(&bow_input_lock142);
 InterlockedExchange(&bow_family142,-1);bow_ticket142={};
}
static void bow_sample142(bool click,bool active){
 bow142::Release release{};AcquireSRWLockExclusive(&bow_input_lock142);
 bool fire=bow_input142.sample(click,bow_family142==1,active,bow_key142(),bow_clock142(),release);
 if(fire)bow_release142=release;
 else if(!active)bow_release142={};
 ReleaseSRWLockExclusive(&bow_input_lock142);
 if(fire)InterlockedExchange(&state->fire_request,1);
}
static float bow_duration142(bool mounted){
 auto db=field<uintptr_t>(possessed,0x23c),array=db?field<uintptr_t>(db,0xc):0;
 auto clip=array?field<uintptr_t>(array,(mounted?758:126)*4):0,asset=clip?field<uintptr_t>(clip,4):0;
 // +90 is the first RELEASE EVENT, not asset duration. Foot raise126
 // has no release event. Original3f0370 advances +64 frames at the native
 // simulation step; mounted758 draws only up to its authored release marker.
 unsigned frames=asset?field<unsigned>(asset,0x64):0;float step=field<float>(base,0x15b2448);
 float duration=mounted&&asset&&field<unsigned char>(asset,0x94)==1?field<float>(asset,0x90):
     frames>=2&&frames<=30000&&std::isfinite(step)&&step>0&&step<=.25f?(frames-1)*step:0;
 return std::isfinite(duration)&&duration>=.05f&&duration<=30?duration:0.f;
}
static float bow_power142(bool mounted,double now){
 AcquireSRWLockShared(&bow_input_lock142);
 const double held=bow_input142.held_for(bow_key142(),now);
 ReleaseSRWLockShared(&bow_input_lock142);
 if(held<0)return 0.f;
 // The meter is optional. Unreadable metadata must not affect possession.
 __try {
  const float duration=bow_duration142(mounted);
  return duration>0?bow142::factor(held,duration):0.f;
 }__except(EXCEPTION_EXECUTE_HANDLER){return 0.f;}
}
// Called only after native ownership/ready/reload/queue guards. No actor or
// weapon-definition writes: bind one numeric power observation to this shot.
static bool bow_bind142(uintptr_t record,bool mounted){
 bow_ticket142={};if(!is_bow_projectile(record))return true;
 bow142::Release release;
 AcquireSRWLockExclusive(&bow_input_lock142);release=bow_release142;bow_release142={};ReleaseSRWLockExclusive(&bow_input_lock142);
 if(!release.serial||!bow142::same(release.key,bow_key142()))return false;
 const float duration=bow_duration142(mounted);
 if(duration<=0)return false;
 bow_ticket142={release.key,record,bow142::factor(release.seconds,duration),release.serial};return true;
}
static float bow_speed142(uintptr_t record,float native_speed){
 if(bow_ticket142.record==record&&bow142::same(bow_ticket142.key,bow_key142())&&std::isfinite(native_speed)&&native_speed>0)
  return native_speed*bow_ticket142.factor;
 return native_speed;
}
