#pragma once
#include <windows.h>
#include <cstdint>
namespace deck189 {
// The original UI selection consumer reads cached Ctrl at1bbe84f, populated
// by f92ae0 from the DirectInput buffer via bd12a0. Both exact native bodies
// are covered by binding175 guards. Scope only the game's input transaction;
// preserve physical key bits and let the original routine refresh its caches.
struct SelectionModifiers {
 uintptr_t base{};unsigned char control{},shift{};
 SelectionModifiers(uintptr_t image,WPARAM flags):base(image){
  auto keys=reinterpret_cast<unsigned char*>(base+0x1b30f58);
  control=keys[0x1d];shift=keys[0x2a];
  if(flags&MK_CONTROL)keys[0x1d]|=0x80;if(flags&MK_SHIFT)keys[0x2a]|=0x80;
  reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(base+0xf92ae0)(0,0);
 }
 ~SelectionModifiers(){auto keys=reinterpret_cast<unsigned char*>(base+0x1b30f58);keys[0x1d]=control;keys[0x2a]=shift;reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(base+0xf92ae0)(0,0);}
 SelectionModifiers(const SelectionModifiers&)=delete;SelectionModifiers& operator=(const SelectionModifiers&)=delete;
};
}
