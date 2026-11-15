#pragma once
#include <windows.h>
#include <cstdint>

namespace deck226 {
// Native environment construction 2470f0 stores BattleSpeechController at
// +2827c. 181ef0 admits phase 2, and 2e5700 identifies its cinematic flag
// at +a4. Audio-only speech is allowed to continue during deployment.
template<class Read>uintptr_t active_speech(uintptr_t image,Read read){
    if(!image)return 0;
    auto root=read(image+0x1bc8180);
    if(!root||read(root)!=image+0x170d284)return 0;
    auto environment=read(root+0x110);
    if(!environment)return 0;
    auto common=read(environment+8);
    if(!common)return 0;
    auto phase=read(common+0xb4);
    if(!phase||read(phase+0x14)!=2)return 0;
    auto speech=read(environment+0x2827c);
    if(!speech||read(speech)!=image+0x15d3c38||read(speech+0x1c)!=environment||
       (read(speech+0xa4)&0xff)!=1)return 0;
    return speech;
}
inline uintptr_t active_speech(uintptr_t image){
    __try{
        return active_speech(image,[](uintptr_t at){return *reinterpret_cast<const uint32_t*>(at);});
    }__except(EXCEPTION_EXECUTE_HANDLER){return 0;}
}
}
