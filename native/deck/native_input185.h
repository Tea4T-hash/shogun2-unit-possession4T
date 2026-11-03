#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"
namespace deck185 {
using Poll=bool(__cdecl*)();
struct NativeInput {
 Poll original{};void* target{};bool enabled{};unsigned char entry[19]{};
 bool create(uintptr_t base,Poll callback){
  if(target)return true;auto code=reinterpret_cast<unsigned char*>(base+0xbd75f0);
  const unsigned char prefix[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe};
  unsigned char bytes[19]{};SIZE_T got{};
  if(!ReadProcessMemory(GetCurrentProcess(),code,bytes,sizeof bytes,&got)||got!=sizeof bytes||memcmp(bytes,prefix,sizeof prefix)||
     *reinterpret_cast<uint32_t*>(bytes+8)!=base+0x1b30f58||bytes[12]!=0xbf||
     *reinterpret_cast<uint32_t*>(bytes+13)!=base+0x1b31058||bytes[17]!=0xf3||bytes[18]!=0xa5)return false;
  auto init=MH_Initialize();if(init!=MH_OK&&init!=MH_ERROR_ALREADY_INITIALIZED)return false;
  if(MH_CreateHook(code,reinterpret_cast<void*>(callback),reinterpret_cast<void**>(&original))!=MH_OK)return false;
  memcpy(entry,bytes,sizeof entry);target=code;return true;
 }
 bool enable(){if(enabled)return true;if(!target||memcmp(target,entry,sizeof entry))return false;if(MH_EnableHook(target)!=MH_OK)return false;enabled=true;return true;}
 bool disable(){if(!enabled)return true;if(MH_DisableHook(target)!=MH_OK)return false;enabled=false;return true;}
 // The bridge is process-pinned. Keep its disabled trampoline alive so a
 // native call already in flight can finish safely across scene handoff.
};
}
