#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <vector>
#include "zoom181_generated.h"
namespace zoom181 {
inline constexpr unsigned relative_rva=0x904c10;
inline constexpr float maximum_pending=14.4f; // two native wheel amounts, never a replay backlog
inline bool enqueue(float& pending,float wheel_fraction){
 if(!std::isfinite(pending)||!std::isfinite(wheel_fraction)||std::fabs(wheel_fraction)>2)return false;
 // Native cam_up negates its amount, and one native wheel notch is7.2.
 const float amount=-wheel_fraction*4.f*7.2f;
 if(amount&&pending*amount<0)pending=0;
 pending=std::clamp(pending+amount,-maximum_pending,maximum_pending);return true;
}
inline bool valid_amount(float amount){return std::isfinite(amount)&&std::fabs(amount)<=maximum_pending;}
inline bool admitted(uintptr_t image){
 BCRYPT_ALG_HANDLE algorithm{};if(!image||BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;
 bool ok=true;
 for(const auto& span:zoom181_binding::spans){
  if(!span.size||span.size>4096||span.rva>0x1d88000||span.size>0x1d88000-span.rva){ok=false;break;}
  std::vector<unsigned char> bytes(span.size);SIZE_T got{};
  if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(image+span.rva),bytes.data(),bytes.size(),&got)||got!=bytes.size()){ok=false;break;}
  for(unsigned i=0;i<span.reloc_count;++i){unsigned offset=zoom181_binding::relocations[span.reloc_start+i];if(offset+4>bytes.size()){ok=false;break;}uint32_t value;memcpy(&value,bytes.data()+offset,4);value-=uint32_t(image-0x10000000u);memcpy(bytes.data()+offset,&value,4);}
  unsigned char digest[32]{};if(!ok||BCryptHash(algorithm,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0){ok=false;break;}
  const char* digits="0123456789abcdef";char text[65]{};for(unsigned i=0;i<32;++i){text[i*2]=digits[digest[i]>>4];text[i*2+1]=digits[digest[i]&15];}
  if(strcmp(text,span.sha)){ok=false;break;}
 }
 BCryptCloseAlgorithmProvider(algorithm,0);return ok;
}
}
