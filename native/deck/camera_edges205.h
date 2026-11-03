#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"

// Only the two original camera consumers of ui_mouse_scroll (option 0x48).
// The option, cursor record and saved preferences are never written. These
// sites normally call bool*(this, unsigned) and clean four argument bytes.
// Resume at the original CMP/JZ with a private false value. All subsequent
// cursor hit testing, wheel bookkeeping and direct stick camera calls survive.
// Installed by the process-pinned bridge only after Valve selects Deck mode.
namespace deck205 {
inline const unsigned char disabled=0;
inline uintptr_t battle_resume{},campaign_resume{};
__declspec(naked) inline void battle_edge(){__asm{
 lea esp,[esp+4]
 mov eax,offset disabled
 jmp dword ptr [battle_resume]
}}
__declspec(naked) inline void campaign_edge(){__asm{
 lea esp,[esp+4]
 mov eax,offset disabled
 jmp dword ptr [campaign_resume]
}}
inline constexpr unsigned char battle_window[]={
 0x6a,0x48,0xf3,0x0f,0x10,0x40,0x04,0x8b,0x46,0x40,0xf3,0x0f,0x11,0x44,0x24,0x28,
 0x8b,0x80,0x4c,0x80,0x02,0x00,0x8b,0x48,0x0c,0x8b,0x01,0x8b,0x40,0x1c,0xff,0xd0,
 0x80,0x38,0x00,0x0f,0x84,0x6c,0x02,0x00,0x00};
inline constexpr unsigned char campaign_window[]={
 0x8b,0x46,0x40,0x6a,0x48,0x8b,0x40,0x10,0x8b,0x48,0x0c,0x8b,0x01,0x8b,0x40,0x1c,
 0xff,0xd0,0x80,0x38,0x00,0x0f,0x84,0xc5,0x01,0x00,0x00};
inline bool equal(uintptr_t address,const void* bytes,size_t size){
 unsigned char copy[64]{};SIZE_T got{};
 return size<=sizeof copy&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),copy,size,&got)&&got==size&&!memcmp(copy,bytes,size);
}
struct Edges {
 uintptr_t base{},targets[2]{};void* originals[2]{};unsigned char owned[2][5]{};bool ready{};
 bool unchanged(uintptr_t image)const{
  return equal(image+0x304800,battle_window,sizeof battle_window)&&equal(image+0x8fb619,campaign_window,sizeof campaign_window);
 }
 bool disable(){
  bool ok=true;
  for(unsigned i=0;i<2;++i)if(targets[i]){
   if(equal(targets[i],owned[i],5)){
    if(MH_DisableHook(reinterpret_cast<void*>(targets[i]))!=MH_OK)ok=false;
   }else{const unsigned char entry[]={0x8b,0x40,0x1c,0xff,0xd0};if(!equal(targets[i],entry,5))ok=false;}
  }
  if(ok)ready=false;return ok;
 }
 bool install(uintptr_t image,bool deck){
  if(!deck)return true; // Desktop: no memory reads, hooks or settings changes.
  if(ready)return base==image&&equal(targets[0],owned[0],5)&&equal(targets[1],owned[1],5);
  if(base||!image||!unchanged(image))return false;
  auto status=MH_Initialize();if(status!=MH_OK&&status!=MH_ERROR_ALREADY_INITIALIZED)return false;
  base=image;battle_resume=image+0x304820;campaign_resume=image+0x8fb62b;
  const unsigned rva[]={0x30481b,0x8fb626};void* callbacks[]={reinterpret_cast<void*>(&battle_edge),reinterpret_cast<void*>(&campaign_edge)};
  unsigned made=0;
  for(;made<2;++made){auto target=image+rva[made];if(MH_CreateHook(reinterpret_cast<void*>(target),callbacks[made],originals+made)!=MH_OK)break;
   targets[made]=target;owned[made][0]=0xe9;uint32_t delta=uint32_t(reinterpret_cast<uintptr_t>(callbacks[made])-target-5);memcpy(owned[made]+1,&delta,4);}
  if(made!=2){while(made)MH_RemoveHook(reinterpret_cast<void*>(targets[--made]));base=targets[0]=targets[1]=0;return false;}
  bool queued=true;for(auto target:targets)if(MH_QueueEnableHook(reinterpret_cast<void*>(target))!=MH_OK)queued=false;
  if(queued&&MH_ApplyQueued()==MH_OK&&equal(targets[0],owned[0],5)&&equal(targets[1],owned[1],5)){ready=true;return true;}
  // Keep any created trampolines pinned on failure, just like the bridge.
  for(auto target:targets)MH_QueueDisableHook(reinterpret_cast<void*>(target));MH_ApplyQueued();disable();return false;
 }
};
inline Edges edges;
}
