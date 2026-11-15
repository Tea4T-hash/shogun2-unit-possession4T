#pragma once
#include <memory>
#include <intrin.h>
extern "C" unsigned long _tls_index;
__declspec(thread) volatile int tls224=0;
namespace fixture224 {
inline unsigned probes{},allow_after{},successful_probes=UINT_MAX;inline bool outside{},legacy{};inline HANDLE legacy_event{};
inline unsigned __fastcall admit(void*,void*,const unsigned char* query,const float* xz,unsigned,char* blocked,char* in_bounds){
 ++probes;if(legacy){if(probes==512)assert(SetEvent(legacy_event));*blocked=*in_bounds=1;return 0;}assert(probes<=32);float step{},radius{},maximum{};unsigned kind{};
 memcpy(&step,query+8,4);memcpy(&radius,query+12,4);memcpy(&maximum,query+0x24,4);memcpy(&kind,query+0x38,4);
 assert(step==5.f&&radius==0.f&&maximum==5.f&&kind==3);
 for(unsigned i=0x10;i<0x20;++i)assert(!query[i]);
 for(unsigned i=0x28;i<0x34;++i)assert(!query[i]);
 assert(query[0x34]==1&&!query[0x3c]&&!query[0x3d]&&!query[0x3e]);
 assert(std::isfinite(xz[0])&&std::isfinite(xz[1]));*blocked=1;*in_bounds=1;
 return outside?0:unsigned(probes>allow_after&&probes<=successful_probes);
}
inline void* __fastcall point(void*,void*,terrain224::Point* out,const float* xz,unsigned zero,unsigned kind,unsigned last){
 assert(!zero&&kind==3&&!last);*out={{xz[0],0,xz[1]},0,0};return out;
}
struct Terrain {
 std::vector<unsigned char> properties=std::vector<unsigned char>(0x3200);
 std::vector<std::unique_ptr<Patch185>> patches;
 Terrain(uintptr_t image,uintptr_t common){
  tls224=1;auto tls=reinterpret_cast<uintptr_t*>(__readfsdword(0x2c));assert(tls&&tls[_tls_index]);
  auto patch=[&](uintptr_t p,const void* v,unsigned n){patches.emplace_back(std::make_unique<Patch185>(p,v,n));};
  const int initialized=INT_MIN;patch(image+0x1a3c444,&initialized,4);patch(image+0x1cb06d0,&_tls_index,4);
  float bounds[]{-1000,-1000,1000,1000};memcpy(reinterpret_cast<void*>(common+0x6c),bounds,16);memcpy(reinterpret_cast<void*>(common+0x7c),bounds,16);
  auto prop=reinterpret_cast<uintptr_t>(properties.data());memcpy(reinterpret_cast<void*>(common+0xb4),&prop,4);
  auto a=jump185(image+0x38d350,reinterpret_cast<void*>(&admit)),p=jump185(image+0x37c010,reinterpret_cast<void*>(&point));
  patch(image+0x38d350,a.data(),5);patch(image+0x37c010,p.data(),5);
  probes=allow_after=0;successful_probes=UINT_MAX;outside=legacy=false;
 }
};
}
