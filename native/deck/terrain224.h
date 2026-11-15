#pragma once
#include "native214.h"
#include <array>
#include <cstring>
namespace terrain224 {
struct Point {float p[3];uintptr_t surface;unsigned id;};
static_assert(sizeof(Point)==20,"Original terrain point ABI");
// Fresh Ghidra125500 constructs a 0x40-byte request.37b480 reads its stride,
// radius, constraints, search bound and surface kind; a float[2] is NOT this ABI.
// Use the game's point-query constructor (5-unit stride, kind3), retain native
// placement checks, and bound a fine nudge to one search ring. No failed search
// may manufacture an order from37b430's fallback point.
inline bool locate(uintptr_t image,uintptr_t manager,const float* xz,Point& point){
 if(!image||!manager||!xz||!native214::admit(image,8))return false;
 for(unsigned i=0;i<2;++i)if(!std::isfinite(xz[i])||std::fabs(xz[i])>100000)return false;
 alignas(16) std::array<unsigned char,0x40> query{};
 __try{
  reinterpret_cast<void*(__thiscall*)(void*,const float*,float,unsigned)>(image+0x125500)(query.data(),xz,5.f,3);
  const float maximum=5.f;memcpy(query.data()+0x24,&maximum,4);
  Point result{};
  if(!(reinterpret_cast<unsigned(__thiscall*)(void*,const void*,void*,unsigned)>(image+0x37b480)(reinterpret_cast<void*>(manager),query.data(),&result,1)&255u))return false;
  for(float v:result.p)if(!std::isfinite(v)||std::fabs(v)>100000)return false;
  point=result;return true;
 }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
}
