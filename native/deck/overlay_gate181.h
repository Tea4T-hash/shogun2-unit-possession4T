#pragma once
#include "battle_ready181.h"
#include "presentation188.h"
static bool deck_help_gate188(){deck188::Presentation presentation;return enabled&&state&&state->ready&&!state->stop&&GetTickCount64()-state->heartbeat<500&&deck188::read_presentation188(GetCurrentProcessId(),presentation)&&presentation.help!=0;}
static bool deck_header_gate186(){
 if(!enabled||!state||!state->ready||state->stop||state->release||GetTickCount64()-state->heartbeat>500)return false;
 deck181::Sample input;if(!deck181::read_sample(GetCurrentProcessId(),input)||(input.context!=unsigned(deck174::Context::RTS)&&input.context!=unsigned(deck174::Context::Menu)))return false;
 __try{
  auto root=*reinterpret_cast<const uintptr_t*>(base+0x1bc8180);if(!root||*reinterpret_cast<const uintptr_t*>(root)!=base+0x170d284||*reinterpret_cast<const unsigned char*>(root+0xca0))return false;
  auto env=*reinterpret_cast<const uintptr_t*>(root+0x110);if(!env)return false;
  auto common=*reinterpret_cast<const uintptr_t*>(env+8);if(!common)return false;auto phase=*reinterpret_cast<const uintptr_t*>(common+0xb4);if(!phase)return false;
  if(!deck181::battle_ready(base,[](uintptr_t at){return *reinterpret_cast<const uintptr_t*>(at);}))return false;
  auto manager=*reinterpret_cast<const uintptr_t*>(env+0x281fc);if(!manager)return false;auto camera=*reinterpret_cast<const uintptr_t*>(manager+0x8e0);if(!camera)return false;
  if(input.context==unsigned(deck174::Context::RTS))return state->mode!=POSSESSED&&camera==*reinterpret_cast<const uintptr_t*>(manager+0x8cc)&&*reinterpret_cast<const uintptr_t*>(camera)==base+0x15d1a74;
  return true; // Numeric controls header, never a gameplay/actor permission.
 }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
