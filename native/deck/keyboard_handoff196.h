#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"
namespace deck196 {
// One verified original poll site, two sequential first-party owners. The
// trampoline remains allocated while disabled, so calls in flight can finish.
// Never chain or overwrite another hook, and never unload either component.
class PollOwnership {
 void* target_{};unsigned char original_[19]{},owned_[19]{};bool active_{};
 bool matches(const unsigned char* expected)const{
  unsigned char current[19]{};SIZE_T n{};
  return target_&&ReadProcessMemory(GetCurrentProcess(),target_,current,sizeof current,&n)&&n==sizeof current&&!memcmp(current,expected,sizeof current);
 }
public:
 bool prepare(void* target,void* callback){
  if(target_||!target||!callback)return false;SIZE_T n{};
  if(!ReadProcessMemory(GetCurrentProcess(),target,original_,sizeof original_,&n)||n!=sizeof original_)return false;
  // The adapter's existing signature/engine admission runs before this call.
  // These instructions are long enough for MinHook's ordinary five-byte jump.
  const unsigned char prefix[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe};
  if(memcmp(original_,prefix,sizeof prefix)||original_[12]!=0xbf||original_[17]!=0xf3||original_[18]!=0xa5)return false;
  memcpy(owned_,original_,sizeof owned_);owned_[0]=0xe9;
  uint32_t relative=uint32_t(reinterpret_cast<uintptr_t>(callback)-reinterpret_cast<uintptr_t>(target)-5);
  memcpy(owned_+1,&relative,4);target_=target;return true;
 }
 bool adopt_enabled(){if(!matches(owned_))return false;active_=true;return true;}
 bool release(){
  if(!active_)return true;
  if(!matches(owned_)||MH_DisableHook(target_)!=MH_OK||!matches(original_))return false;
  active_=false;return true;
 }
 bool acquire(){
  if(active_)return matches(owned_);
  if(!matches(original_)||MH_EnableHook(target_)!=MH_OK||!matches(owned_))return false;
  active_=true;return true;
 }
 bool active()const{return active_;}
};
}
