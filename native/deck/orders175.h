#pragma once
#include <cstdint>

namespace deck175 {
// The native UI dispatcher retains command eligibility, construction and
// serialization. This preflight only establishes the borrowed land selection
// and reserves space before entering its ordinary queue writer.
template<class Read,class Current>
bool native_order_ready(uintptr_t base,uintptr_t root,uintptr_t environment,
                        Read read,Current current,bool allow_groups=false){
    if(!current()||!root||!environment||read(root+0x110)!=environment||
       read(root+0x5b8)!=environment)return false;
    const auto selected=read(environment+0x2826c);
    const auto owner=selected?read(selected+0x4a8):0;
    const auto group=owner?owner+0xb0:0;
    const auto queue=read(environment+0x28048);
    if(!group||!queue||read(root+0x5b4)!=owner||read(group)!=base+0x15c4da0)return false;
    const auto units=read(group+0x14),groups=read(group+8);
    if(units>512||groups>512||units+groups==0||units+groups>512)return false;
    if(units&&(read(group+0x10)<units||!read(group+0x18)))return false;
    if(groups&&(!allow_groups||read(group+4)<groups||!read(group+0xc)))return false;
    if(read(group+0x20)||read(group+0x2c))return false;
    const auto ui_entries=read(group+0x3c);
    if(!allow_groups&&(ui_entries||read(group+0x34)))return false;
    if(ui_entries&&(ui_entries>512||read(group+0x38)<ui_entries||!read(group+0x40)))return false;
    // Both selected land vectors use one tagged 32-bit ID per entry.
    // Keep the conservative headroom before entering the native queue writer.
    constexpr uintptr_t capacity_bytes=0x5000;
    const auto before=read(queue+4+capacity_bytes),budget=128+8*(units+groups);
    if(before>capacity_bytes||budget>capacity_bytes-before||!current())return false;
    return true;
}
template<class Read,class Invoke>
bool dispatch_selection_order(uintptr_t root,uintptr_t environment,unsigned action,Read read,Invoke invoke){
    const auto queue=read(environment+0x28048),before=read(queue+4+0x5000);
    invoke(root,action,0.f);
    const auto after=read(queue+4+0x5000);
    return after>before&&after<=0x5000;
}

template<class Read,class Current,class Invoke>
bool native_halt(uintptr_t base,uintptr_t root,uintptr_t environment,
                 Read read,Current current,Invoke invoke){
    return native_order_ready(base,root,environment,read,current)&&
        dispatch_selection_order(root,environment,4,read,invoke);
}

template<class Read,class ReadByte,class FireEligible>
unsigned fire_mode_action(uintptr_t selection,bool skirmish,Read read,ReadByte read_byte,
                          FireEligible fire_eligible){
    constexpr unsigned max_units=512;
    unsigned total=0;
    bool has_eligible=false,all_active=true;
    auto check_units=[&](uintptr_t entries,uintptr_t count){
        if(count>max_units-total||(count&&!entries))return false;
        total+=unsigned(count);
        for(uintptr_t i=0;i<count;++i){
            const auto unit=read(entries+i*4);
            unsigned char eligible{},active{};
            if(!unit||!(skirmish?read_byte(unit+0x179,eligible):fire_eligible(unit,eligible))||
               eligible>1)return false;
            // The game hides Skirmish for mixed eligibility. Fire at Will
            // aggregates the state of the eligible units in the selection.
            if(!eligible){
                if(skirmish)return false;
                continue;
            }
            if(!read_byte(unit+(skirmish?0xeb2:0xeb0),active)||active>1)return false;
            has_eligible=true;
            all_active=all_active&&active!=0;
        }
        return true;
    };
    if(!check_units(read(selection+0x18),read(selection+0x14)))return 0;
    const auto groups=read(selection+0xc),count=read(selection+8);
    // Both native group member lists are used by 2fc880/2feac0.
    for(uintptr_t i=0;i<count;++i){
        const auto group=read(groups+i*4);
        if(!group||!check_units(read(group+0x24),read(group+0x20))||
           !check_units(read(group+0x64),read(group+0x60)))return 0;
    }
    if(!has_eligible)return 0;
    // Explicit stock button orders: enable/disable Skirmish or Fire at Will.
    return skirmish?(all_active?0x55:0x54):(all_active?3:2);
}

template<class Read,class ReadByte,class FireEligible,class Current,class Invoke>
bool native_fire_mode(uintptr_t base,uintptr_t root,uintptr_t environment,bool skirmish,
                      Read read,ReadByte read_byte,FireEligible fire_eligible,Current current,Invoke invoke){
    if(!native_order_ready(base,root,environment,read,current,true))return false;
    const unsigned action=fire_mode_action(read(root+0x5b4)+0xb0,skirmish,read,read_byte,fire_eligible);
    return action&&current()&&dispatch_selection_order(root,environment,action,read,invoke);
}
}
