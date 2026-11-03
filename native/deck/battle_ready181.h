#pragma once
#include "handoff181.h"
namespace deck181 {
template<class Read>bool battle_ready(uintptr_t base,Read read){
 const auto root=read(base+0x1bc8180);if(!root)return false;
 const auto env=read(root+0x110);if(!env)return false;
 const auto common=read(env+8);if(!common)return false;
 const auto phase=read(common+0xb4);if(!phase)return false;
 const auto id=read(phase+0x14);if(id>=5&&id<=8)return true;
 // Phase3 was observed in the owner's troops-visible deployment on 2026-10-31.
 // Its selected-soldier view is NULL: camera control must not require that view.
 // Unknown phases remain refused even if stale battlefield pointers survive.
 if(id!=3)return false;
 if(read(root)!=base+0x170d284||read(root+0x18)!=base+0x170d290||read(root+0x10c)!=base+0x170d304)return false;
 const auto manager=read(env+0x281fc);if(!manager||read(manager)!=base+0x15d26fc)return false;
 const auto camera=read(manager+0x8e0);
 return camera&&camera==read(manager+0x8cc)&&read(camera)==base+0x15d1a74;
}
}
