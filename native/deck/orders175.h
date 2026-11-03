#pragma once
#include <cstdint>

namespace deck175 {
// The native UI dispatcher retains command eligibility, construction and
// serialization. This preflight only establishes the borrowed land selection
// and reserves space before entering its ordinary queue writer.
template<class Read,class Current,class Invoke>
bool native_halt(uintptr_t base,uintptr_t root,uintptr_t environment,
                 Read read,Current current,Invoke invoke){
    if(!current()||!root||!environment||read(root+0x110)!=environment||
       read(root+0x5b8)!=environment)return false;
    const auto selected=read(environment+0x2826c);
    const auto owner=selected?read(selected+0x4a8):0;
    const auto group=owner?owner+0xb0:0;
    const auto queue=read(environment+0x28048);
    if(!group||!queue||read(root+0x5b4)!=owner||read(group)!=base+0x15c4da0)return false;
    const auto count=read(group+0x14),capacity=read(group+0x10);
    if(!count||count>512||capacity<count||!read(group+0x18))return false;
    if(read(group+8)||read(group+0x20)||read(group+0x2c)||read(group+0x34)||read(group+0x3c))return false;
    // Verified serializer342930: four tagged32-bit category counts, one tagged
    //32-bit ID per land unit, and one tagged32-bit flag ID. Context1cc200 adds
    //7 bytes.128+8*n deliberately exceeds that32+5*n payload and leaves margin
    // for framing. Reject a full/corrupt queue before native direct writes.
    constexpr uintptr_t capacity_bytes=0x5000;
    const auto before=read(queue+4+capacity_bytes),budget=128+8*count;
    if(before>capacity_bytes||budget>capacity_bytes-before||!current())return false;
    invoke(root,4,0.f); //10dfb80 ->10f97d0 case4, native Halt.
    const auto after=read(queue+4+capacity_bytes);
    return after>before&&after<=capacity_bytes;
}
}
