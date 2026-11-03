#pragma once
// Local input intent only; these bits are never added to the shared movement ABI.
constexpr unsigned ChargePrimary199=1u<<16;
inline unsigned charge_intent199(unsigned movement,bool primary,bool aiming,bool owned){
    return owned?(movement|((primary&&!aiming)?ChargePrimary199:0u)):0u;
}
struct ClimbRequest199 {
    SRWLOCK lock=SRWLOCK_INIT;unsigned generation{};uint64_t tick{};
    void set(unsigned lease,uint64_t now){AcquireSRWLockExclusive(&lock);generation=lease;tick=now;ReleaseSRWLockExclusive(&lock);}
    void clear(){set(0,0);}
    bool take(unsigned lease,uint64_t now){AcquireSRWLockExclusive(&lock);bool result=generation&&generation==lease&&now>=tick&&now-tick<=250;generation=0;tick=0;ReleaseSRWLockExclusive(&lock);return result;}
};
