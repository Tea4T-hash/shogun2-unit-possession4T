// Isolated owned IMAGE math only. No DllMain, game entry, graphics/Steam or
// actual controller. Native viewport providers and render-device state mocked.
#define NOMINMAX
#include "../../native/deck/native_ui185.h"
#include <array>
#include <cassert>
#include <iostream>
struct Point185{float x,y;};
static unsigned width185=1280,height185=800;static unsigned display185[12]{};
static void __fastcall physical185(void*,void*,int* out){out[0]=out[1]=0;out[2]=int(width185);out[3]=int(height185);}
static bool __fastcall mode185(void*,void*){return false;}
static unsigned* __fastcall display_info185(void*,void*){display185[4]=display185[8]=width185;display185[5]=display185[9]=height185;return display185;}
struct Patch185 {unsigned char* at{};std::vector<unsigned char> saved;
 Patch185(uintptr_t address,const void* bytes,unsigned size):at(reinterpret_cast<unsigned char*>(address)),saved(size){DWORD old{},unused{};memcpy(saved.data(),at,size);assert(VirtualProtect(at,size,PAGE_EXECUTE_READWRITE,&old));memcpy(at,bytes,size);FlushInstructionCache(GetCurrentProcess(),at,size);assert(VirtualProtect(at,size,old,&unused));}
 ~Patch185(){DWORD old{},unused{};assert(VirtualProtect(at,saved.size(),PAGE_EXECUTE_READWRITE,&old));memcpy(at,saved.data(),saved.size());FlushInstructionCache(GetCurrentProcess(),at,saved.size());assert(VirtualProtect(at,saved.size(),old,&unused));}
};
static std::array<unsigned char,5> jump185(uintptr_t address,void* target){std::array<unsigned char,5>b{0xe9};uint32_t d=uint32_t(reinterpret_cast<uintptr_t>(target)-address-5);memcpy(b.data()+1,&d,4);return b;}
static unsigned expected_logical185(unsigned w,unsigned h,bool horizontal){if(h<1024){double s=1024./h;w=unsigned(std::lround(w*s));h=1024;}return horizontal?w:h;}
int wmain(int argc,wchar_t** argv){assert(argc==3);unsigned viewport_cases=0,transforms=0;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 for(int input=1;input<argc;++input){
  HMODULE image=LoadLibraryExW(argv[input],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);assert(deck185::native_admitted(base));
  auto patch=jump185(base+0x13f7b40,reinterpret_cast<void*>(&physical185));
  // Scope installation is checked on exact owned image BEFORE mocked providers.
  assert(deck185::install_ui185(base));assert(!deck185::native_admitted(base));
  {Patch185 physical(base+0x13f7b40,patch.data(),5);uintptr_t flags=reinterpret_cast<uintptr_t>(&mode185),display=reinterpret_cast<uintptr_t>(&display_info185);
   Patch185 mode(base+0x1754574+0x248,&flags,4),device(base+0x1754574+0x30c,&display,4);
   uintptr_t table=base+0x1754574;std::array<unsigned char,32> renderer{};memcpy(renderer.data(),&table,4);
   std::array<unsigned char,0x328> root{};uintptr_t render=reinterpret_cast<uintptr_t>(renderer.data());memcpy(root.data()+8,&render,4);uintptr_t context=reinterpret_cast<uintptr_t>(root.data());Patch185 global(base+0x1bbe364,&context,4);
   using View=unsigned*(__thiscall*)(void*,unsigned*);using Transform=Point185*(__thiscall*)(void*,Point185*,float,float);
   auto view=reinterpret_cast<View>(base+0x13f7c00);auto ndc=reinterpret_cast<Transform>(base+0x13e35d0);auto inverse=reinterpret_cast<Transform>(base+0x14121a0);
   for(auto dimensions:{std::pair<unsigned,unsigned>{1280,800},{1280,720},{1920,1080},{2560,1440}}){
    width185=dimensions.first;height185=dimensions.second;unsigned dims[2]{};assert(view(renderer.data(),dims)==dims);
    assert(dims[0]==unsigned(std::lround(expected_logical185(width185,height185,true)/deck185::native_ui_scale185))&&dims[1]==unsigned(std::lround(expected_logical185(width185,height185,false)/deck185::native_ui_scale185)));++viewport_cases;unsigned maximum[2]{};auto maxview=reinterpret_cast<View>(base+0x13f7d20);assert(maxview(renderer.data(),maximum)==maximum&&maximum[0]==dims[0]&&maximum[1]==dims[1]);
    auto active=deck185::active_ui185();assert(active.width==dims[0]&&active.height==dims[1]);
    uintptr_t provider=render,provider_at=reinterpret_cast<uintptr_t>(&provider),zero=0;memcpy(root.data()+4,&provider_at,4);memcpy(root.data()+8,&zero,4);
    auto original_renderer=reinterpret_cast<uintptr_t(__thiscall*)(void*)>(base+0xf9eb50);assert(original_renderer(root.data())==render);
    auto fallback=deck185::active_ui185();assert(fallback.width==dims[0]&&fallback.height==dims[1]);memcpy(root.data()+8,&render,4);
    for(unsigned y=0;y<=16;++y)for(unsigned x=0;x<=32;++x){float px=float(x)/32,py=float(y)/16;Point185 logical{},projected{};
     assert(inverse(renderer.data(),&logical,px,py)==&logical);assert(fabs(logical.x-px*dims[0])<.002&&fabs(logical.y-py*dims[1])<.002);
     // Native UI dimension->clip-space conversion, same logical viewport as
     // native inverse pointer mapping. No formula-only surrogate of the call.
     ndc(renderer.data(),&projected,logical.x,logical.y);assert(fabs(projected.x-2*px)<.00001&&fabs(projected.y+2*py)<.00001);++transforms;
    }
   }
  }
  // Restore all OWNED detours; fixtures may unload, production bridge is pinned.
  for(auto target:deck185::native_ui185.targets){assert(MH_DisableHook(target)==MH_OK);assert(MH_RemoveHook(target)==MH_OK);}
  deck185::native_ui185={};assert(deck185::native_admitted(base));assert(FreeLibrary(image));
 }
 std::cout<<"PASS native UI185: "<<viewport_cases<<" native logical viewports and "<<transforms<<" actual native projection/inverse points; exact stock+Bugfix guards/ABI/rollback. Providers mocked; no game startup.\n";return 0;
}
