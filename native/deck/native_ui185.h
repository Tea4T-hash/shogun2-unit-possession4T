#pragma once
#include <windows.h>
#include <algorithm>
#include <bcrypt.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>
#include "native185_generated.h"
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"
namespace deck185 {
inline bool native_admitted(uintptr_t image,bool keyboard_only=false,unsigned only_rva=0){
 BCRYPT_ALG_HANDLE algorithm{};if(!image||BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;bool ok=true;
 bool found=false;for(const auto& span:native185::spans){
  if(only_rva&&span.rva!=only_rva)continue;found=true;
  if(keyboard_only&&span.rva!=0xbd75f0)continue;
  if(!span.size||span.size>32768||span.rva>0x1d88000||span.size>0x1d88000-span.rva){ok=false;break;}
  std::vector<unsigned char> bytes(span.size);SIZE_T got{};
  if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(image+span.rva),bytes.data(),bytes.size(),&got)||got!=bytes.size()){ok=false;break;}
  for(unsigned i=0;i<span.count;++i){unsigned off=native185::relocations[span.start+i];if(off+4>bytes.size()){ok=false;break;}uint32_t v;memcpy(&v,bytes.data()+off,4);v-=uint32_t(image-0x10000000u);memcpy(bytes.data()+off,&v,4);}
  unsigned char digest[32]{};if(!ok||BCryptHash(algorithm,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0){ok=false;break;}
  const char* digits="0123456789abcdef";char text[65]{};for(unsigned i=0;i<32;++i){text[i*2]=digits[digest[i]>>4];text[i*2+1]=digits[digest[i]&15];}if(strcmp(text,span.sha)){ok=false;break;}
 }
 BCryptCloseAlgorithmProvider(algorithm,0);return ok&&found;
}
struct UiSize {unsigned width{},height{};};
inline bool ui_size_valid(UiSize s){return s.width>=640&&s.height>=360&&s.width<=32767&&s.height<=32767;}
// Original logical-UI viewport -> uniformly larger panels. 3D viewport and
// camera routines are untouched. Native docking/text wrapping/scrollbars still
// receive the same reduced logical size used by projection and inverse input.
inline constexpr double native_ui_scale185=1.25;
// A modal must fit as a COMPLETE native layout, including its fixed-size
// descendants. Never compress its frame independently of its contents.
// Updated only by the campaign UI thread; projection and inverse input read
// the same atomic limits. The normal map still uses the accepted +25% scale.
alignas(8) inline volatile LONG64 panel_extent191{};
inline void panel_extent191_set(unsigned width,unsigned height){
 InterlockedExchange64(&panel_extent191,LONG64((uint64_t(height)<<32)|width));
}
inline UiSize enlarge_ui(UiSize size){
 if(!ui_size_valid(size))return size;
 auto extent=uint64_t(InterlockedCompareExchange64(&panel_extent191,0,0));
 const UiSize normal{(size.width*4u+2u)/5u,(size.height*4u+2u)/5u};
 const double min_w=unsigned(extent),min_h=unsigned(extent>>32);
 if(min_w<=normal.width&&min_h<=normal.height)return normal;
 const double scale=std::max({.8,min_w/size.width,min_h/size.height});
 return {unsigned(std::ceil(size.width*scale)),unsigned(std::ceil(size.height*scale))};
}
using UiViewport=unsigned*(__thiscall*)(void*,unsigned*);
using CursorGetter=int(__cdecl*)(void*);
using PanelPlace187=void(__stdcall*)(void*,unsigned);
using Resize187=void(__thiscall*)(void*,float,float,bool,bool);
using GetTop=int(__cdecl*)(void*);using SetTop=void(__cdecl*)(void*,int);using PushNumber=void(__cdecl*)(void*,float);
struct NativeUi {
 uintptr_t base{};UiViewport current{},maximum{};CursorGetter cursor{};GetTop get_top{};SetTop set_top{};PushNumber push_number{};PanelPlace187 place{};bool ready{};
 void* targets[4]{};
};
inline NativeUi native_ui185;
// Fixed-size private status only; no files, logging or collection worker.
struct UiTrace187 {unsigned magic{0x37554944},version{1},bytes{sizeof(UiTrace187)};volatile LONG sequence{};unsigned renderer_path{},width{},height{},panels{},fitted{};float root_w{},root_h{},before_w{},before_h{},after_w{},after_h{},reserved{};};
static_assert(sizeof(UiTrace187)==64,"Bounded own UI status ABI");
inline UiTrace187 trace187;

inline thread_local UiSize current_ui185;
inline unsigned* __fastcall ui_current185(void* self,void*,unsigned* out){auto result=native_ui185.current(self,out);if(result==out&&out){auto s=enlarge_ui({out[0],out[1]});out[0]=s.width;out[1]=s.height;current_ui185=s;}return result;}
inline unsigned* __fastcall ui_maximum185(void* self,void*,unsigned* out){auto result=native_ui185.maximum(self,out);if(result==out&&out){auto s=enlarge_ui({out[0],out[1]});out[0]=s.width;out[1]=s.height;}return result;}
inline int __cdecl ui_cursor185(void* lua){
 const int before=native_ui185.get_top(lua);current_ui185={};int results=native_ui185.cursor(lua);auto s=current_ui185;
 // The original getter supplies x,y,w,h but floors its bounds at1280x1024.
 // Replace ONLY those last two numeric results, using the same viewport its
 // original coordinate conversion just queried. Preserve parser validity and
 // every original input/coordinate result. No retained lua_State or raw slots.
 if(results==4&&ui_size_valid(s)&&native_ui185.get_top(lua)==before+4){native_ui185.set_top(lua,-3);native_ui185.push_number(lua,float(s.width));native_ui185.push_number(lua,float(s.height));}
 return results;
}
inline bool ui_pointer185(uintptr_t at,uintptr_t& out);
inline UiSize active_ui185();
// Only geometry from current PanelManager members is admitted. No widget
// controller/script callbacks are synthesized, no child is reparented, and no
// retained widget pointer survives a frame. The snapshot is all-or-nothing.
struct Panel191 {uintptr_t at{};float width{},height{};};
inline bool read_ui191(uintptr_t at,void* out,SIZE_T bytes){SIZE_T got{};return at&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,bytes,&got)&&got==bytes;}
inline bool member191(uintptr_t at,uintptr_t root,Panel191& result){
 uintptr_t table{},state{},parent{};
 if(!at||at==root||!ui_pointer185(at,table)||(table!=native_ui185.base+0x16e2aac&&table!=native_ui185.base+0x1704cdc)||
    !ui_pointer185(at+0xb8,state)||!state||!ui_pointer185(at+0x94,parent))return false;
 uintptr_t ancestor=parent;
 for(unsigned n=0;ancestor&&ancestor!=root&&n<8;++n){uintptr_t next{};if(!ui_pointer185(ancestor+0x94,next)||next==ancestor)return false;ancestor=next;}
 if(ancestor!=root)return false;
 float d[2]{};if(!read_ui191(state+0x28,d,sizeof d)||!std::isfinite(d[0])||!std::isfinite(d[1])||d[0]<=0||d[1]<=0||d[0]>8192||d[1]>8192)return false;
 result={at,d[0],d[1]};return true;
}
inline bool panels191(uintptr_t root,std::vector<Panel191>& result){
 uintptr_t manager{};if(!ui_pointer185(native_ui185.base+0x1bc73e4,manager)||!manager)return false;
 for(unsigned offset:{0x30u,0x3cu}){
  unsigned count{};uintptr_t items{};
  if(!read_ui191(manager+offset,&count,4)||count>32||!ui_pointer185(manager+offset+4,items)||(count&&!items))return false;
  for(unsigned i=0;i<count;++i){uintptr_t at{};if(!ui_pointer185(items+i*4,at))return false;
   if(std::any_of(result.begin(),result.end(),[&](const Panel191& p){return p.at==at;}))continue;
   Panel191 p;if(!member191(at,root,p))return false;
   unsigned char visible{};if(!read_ui191(at+0xf5,&visible,1)||visible>1)return false;
   if(visible)result.push_back(p);
  }
 }
 return true;
}
inline bool root_size191(uintptr_t root,UiSize size){
 uintptr_t table{},state{};float before[2]{};
 if(!ui_pointer185(root,table)||(table!=native_ui185.base+0x16e2aac&&table!=native_ui185.base+0x1704cdc)||
    !ui_pointer185(root+0xb8,state)||!read_ui191(state+0x28,before,sizeof before))return false;
 if(before[0]==float(size.width)&&before[1]==float(size.height))return true;
 unsigned char saved[2]{};if(!read_ui191(root+0xed,saved,2)||saved[0]>1||saved[1]>1)return false;
 struct Restore{unsigned char* p;unsigned char x,y;~Restore(){p[0]=x;p[1]=y;}} flags{reinterpret_cast<unsigned char*>(root+0xed),saved[0],saved[1]};
 flags.p[0]=flags.p[1]=1;
 // Resize ONLY the root canvas. All native panel/frame/child dimensions and
 // fonts remain original; recursive resizing caused the displaced checkmark.
 reinterpret_cast<Resize187>(native_ui185.base+0xf98900)(reinterpret_cast<void*>(root),float(size.width),float(size.height),false,false);
 float after[2]{};return read_ui191(state+0x28,after,sizeof after)&&after[0]==float(size.width)&&after[1]==float(size.height);
}
inline bool sync_panels191(bool modal){
 if(!native_ui185.ready)return false;
 uintptr_t root{};if(!ui_pointer185(native_ui185.base+0x1bbe364,root)||!root)return false;
 std::vector<Panel191> panels;if(modal&&!panels191(root,panels))return false;
 unsigned required_w=0,required_h=0;
 for(const auto& p:panels){required_w=std::max(required_w,unsigned(std::ceil(p.width))+32);required_h=std::max(required_h,unsigned(std::ceil(p.height))+32);}
 auto previous=InterlockedCompareExchange64(&panel_extent191,0,0);
 panel_extent191_set(required_w,required_h);auto size=active_ui185();
 if(!ui_size_valid(size)||!root_size191(root,size)){InterlockedExchange64(&panel_extent191,previous);return false;}
 // OpenPanel's no-placement descriptor (used by Clan) never visits1096060.
 // Fit AFTER stock initialization on the root frame callback, covering both
 // that path and normal opens/tab changes. Native MoveTo keeps hitboxes and
 // visuals together; no extra Initialise/OnSelect or synthetic clicks.
 for(const auto& p:panels){
  Panel191 current;if(!member191(p.at,root,current)||current.width!=p.width||current.height!=p.height)return false;
  float xy[2]{};reinterpret_cast<float*(__thiscall*)(void*,float*)>(native_ui185.base+0xf955b0)(reinterpret_cast<void*>(p.at),xy);
  if(!std::isfinite(xy[0])||!std::isfinite(xy[1]))return false;
  const float left=16,top=16,right=float(size.width)-p.width-16,bottom=float(size.height)-p.height-16;
  float target[2]{std::clamp(xy[0],left,std::max(left,right)),std::clamp(xy[1],top,std::max(top,bottom))};
  if(target[0]!=xy[0]||target[1]!=xy[1])reinterpret_cast<void(__thiscall*)(void*,const float*)>(native_ui185.base+0xf920a0)(reinterpret_cast<void*>(p.at),target);
 }
 InterlockedIncrement(&trace187.sequence);trace187.panels=unsigned(panels.size());trace187.fitted=unsigned(panels.size());trace187.before_w=float(required_w);trace187.before_h=float(required_h);trace187.after_w=float(size.width);trace187.after_h=float(size.height);trace187.reserved=0;MemoryBarrier();InterlockedIncrement(&trace187.sequence);
 return true;
}
inline void __stdcall ui_place187(void* panel,unsigned kind){
 // Preserve the native placement transaction unchanged. Whole-window fitting
 // runs later, after callbacks finish creating the actual panel contents.
 native_ui185.place(panel,kind);
}
inline bool install_ui185(uintptr_t base){
 if(native_ui185.ready)return native_ui185.base==base;
 if(!native_admitted(base))return false;auto init=MH_Initialize();if(init!=MH_OK&&init!=MH_ERROR_ALREADY_INITIALIZED)return false;
 native_ui185.base=base;native_ui185.get_top=reinterpret_cast<GetTop>(base+0xea2d60);native_ui185.set_top=reinterpret_cast<SetTop>(base+0xea37c0);native_ui185.push_number=reinterpret_cast<PushNumber>(base+0xea32b0);
 const unsigned rvas[]={0x13f7c00,0x13f7d20,0xf77f70,0x1096060};void* callbacks[]={reinterpret_cast<void*>(&ui_current185),reinterpret_cast<void*>(&ui_maximum185),reinterpret_cast<void*>(&ui_cursor185),reinterpret_cast<void*>(&ui_place187)};
 void** originals[]={reinterpret_cast<void**>(&native_ui185.current),reinterpret_cast<void**>(&native_ui185.maximum),reinterpret_cast<void**>(&native_ui185.cursor),reinterpret_cast<void**>(&native_ui185.place)};
 unsigned made=0;for(;made<4;++made){auto at=reinterpret_cast<void*>(base+rvas[made]);if(MH_CreateHook(at,callbacks[made],originals[made])!=MH_OK)break;native_ui185.targets[made]=at;}
 if(made!=4){while(made)MH_RemoveHook(native_ui185.targets[--made]);return false;}
 // MinHook queues one atomic thread-suspended enable operation for our hooks.
 // Do not enable, disable, remove or chain another component's hook list.
 bool queued=true;for(auto at:native_ui185.targets)if(MH_QueueEnableHook(at)!=MH_OK)queued=false;
 if(queued&&MH_ApplyQueued()==MH_OK){native_ui185.ready=true;return true;}
 // Keep trampolines pinned even on a partial enable failure. Restore only our
 // targets; never uninitialize a MinHook instance with another hook in it.
 for(auto at:native_ui185.targets)MH_QueueDisableHook(at);
 MH_ApplyQueued();for(auto at:native_ui185.targets)MH_DisableHook(at);
 return false;
}
inline bool ui_pointer185(uintptr_t at,uintptr_t& out){SIZE_T got{};return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&out,sizeof out,&got)&&got==sizeof out;}
inline UiSize active_ui185(){
 if(!native_ui185.ready)return {};uintptr_t root{},renderer{},table{};
 if(!ui_pointer185(native_ui185.base+0x1bbe364,root)||!root||!ui_pointer185(root+8,renderer))return {};
 // Exact original f9eb50 provider fallback; neither scene pointer is retained.
 const unsigned path=renderer?1:2;
 if(!renderer){uintptr_t provider{};if(!ui_pointer185(root+4,provider)||!provider||!ui_pointer185(provider,renderer))return {};}
 if(!renderer||!ui_pointer185(renderer,table)||table!=native_ui185.base+0x1754574)return {};
 unsigned dims[2]{};ui_current185(reinterpret_cast<void*>(renderer),nullptr,dims);
 float root_size[2]{};uintptr_t state{};SIZE_T got{};if(ui_pointer185(root+0xb8,state)&&state)ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(state+0x28),root_size,sizeof root_size,&got);
 InterlockedIncrement(&trace187.sequence);trace187.renderer_path=path;trace187.width=dims[0];trace187.height=dims[1];trace187.root_w=root_size[0];trace187.root_h=root_size[1];MemoryBarrier();InterlockedIncrement(&trace187.sequence);
 return {dims[0],dims[1]};
}
} // deck185
