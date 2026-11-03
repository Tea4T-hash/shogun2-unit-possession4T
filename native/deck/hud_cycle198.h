#pragma once
#include <windows.h>
#include <cstdint>
#define S2UP_HUD_CYCLE198 1
// One visibility state for keyboard K and controller View. No controller/device
// dependency: the existing HUD visibility mapping carries the resolved result.
namespace hudcycle198 {
static SRWLOCK lock=SRWLOCK_INIT;
static uintptr_t root_id{};
static unsigned lease_id{},stage{};
static bool active{};
static volatile LONG held{};
inline unsigned observe(uintptr_t root,unsigned lease,bool possessed,unsigned native_hidden){
    AcquireSRWLockExclusive(&lock);
    if(!possessed){active=false;root_id=0;lease_id=stage=0;}
    else if(!active||root_id!=root||lease_id!=lease||unsigned(stage!=0)!=native_hidden){
        active=true;root_id=root;lease_id=lease;stage=native_hidden?1u:0u;
    }
    unsigned result=possessed?stage:0;ReleaseSRWLockExclusive(&lock);return result;
}
inline void commit(uintptr_t root,unsigned lease,unsigned value){
    AcquireSRWLockExclusive(&lock);active=true;root_id=root;lease_id=lease;stage=value;
    ReleaseSRWLockExclusive(&lock);
}
inline unsigned hidden(uintptr_t root,unsigned lease,bool possessed,unsigned native_hidden){
    unsigned value=observe(root,lease,possessed,native_hidden);
    return possessed?unsigned(value==2):native_hidden;
}
inline void filter(uint8_t* current,uint8_t* previous,bool owned,bool valid){
    if(!valid)return;
    bool down=(current[0x25]&0x80)!=0;
    bool drain=InterlockedCompareExchange(&held,0,0)!=0;
    if(owned||drain)current[0x25]=previous[0x25]=0;
    InterlockedExchange(&held,down&&(owned||drain));
}
inline bool window_owned(bool possessed){return possessed||InterlockedCompareExchange(&held,0,0)!=0;}
}
