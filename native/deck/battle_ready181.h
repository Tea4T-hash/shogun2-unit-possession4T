#pragma once
#include "handoff181.h"
namespace deck181 {
template<class Read>bool deployment_root(uintptr_t base,uintptr_t root,Read read){
 return root&&read(root)==base+0x170d284&&read(root+0x18)==base+0x170d290&&read(root+0x10c)==base+0x170d304;
}
template<class Read>bool in_deployment(uintptr_t base,Read read){
 const auto root=read(base+0x1bc8180);
 if(!deployment_root(base,root,read))return false;
 const auto env=read(root+0x110),common=env?read(env+8):0;
 const auto phase=common?read(common+0xb4):0;
 return phase&&read(phase+0x14)==3;
}
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
 if(!deployment_root(base,root,read))return false;
 const auto manager=read(env+0x281fc);if(!manager||read(manager)!=base+0x15d26fc)return false;
 const auto camera=read(manager+0x8e0);
 return camera&&camera==read(manager+0x8cc)&&read(camera)==base+0x15d1a74;
}
template<class Read>bool owns_possession_cursor(uintptr_t base,uintptr_t environment,uintptr_t camera,Read read){
 // The worker can survive the battle. An old POSSESSED flag is not proof
 // that a current native battle camera still owns the shared pointer.
 if(!environment||!camera)return false;
 const auto root=read(base+0x1bc8180);
 if(!root||read(root)!=base+0x170d284||read(root+0x110)!=environment)return false;
 const auto common=read(environment+8),phase=common?read(common+0xb4):0;
 const auto id=phase?read(phase+0x14):0;
 if(id<5||id>8)return false;
 const auto manager=read(environment+0x281fc);
 return manager&&read(manager)==base+0x15d26fc&&read(manager+0x8e0)==camera&&
        read(camera)==base+0x15d22e8&&read(camera+0x180)==manager;
}
}
