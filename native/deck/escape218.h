#pragma once
#include <windows.h>
#include <cstdint>
namespace deck218 {
// One native DirectInput Escape pulse, consumed on the game's own poll thread.
// Battle scripts may own Escape; the original game dispatches it normally.
// Window messages alone do not populate the game's cached DI key array.
class Escape {
 SRWLOCK lock_=SRWLOCK_INIT;HWND window_{};uint64_t tick_{};uintptr_t scope_{};
public:
 bool request(HWND window,uint64_t now,uintptr_t scope=0){
  DWORD pid{};if(!window||!IsWindow(window)||!GetWindowThreadProcessId(window,&pid)||pid!=GetCurrentProcessId()||GetForegroundWindow()!=window)return false;
  AcquireSRWLockExclusive(&lock_);window_=window;tick_=now;scope_=scope;ReleaseSRWLockExclusive(&lock_);return true;
 }
 void clear(){AcquireSRWLockExclusive(&lock_);window_=nullptr;tick_=0;ReleaseSRWLockExclusive(&lock_);}
 bool apply(unsigned char* keys,bool valid,HWND focused,uint64_t now,uintptr_t scope=0){
  AcquireSRWLockExclusive(&lock_);bool live=window_&&focused==window_&&scope==scope_&&now>=tick_&&now-tick_<=250;
  bool consumed=live&&valid&&keys;
  if(consumed)keys[0x01]|=0x80;
  if(consumed||!live){window_=nullptr;tick_=0;}
  ReleaseSRWLockExclusive(&lock_);return consumed;
 }
};
}
