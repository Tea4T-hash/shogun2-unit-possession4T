// Execute ONLY reviewed zoom wrapper bodies in an owned inert mapped image.
// Their scene/clamp providers are explicit mocks; no engine entry, game, hooks,
// Steam, controller, live process or on-disk image is executed/modified.
#include "../../native/deck/zoom181.h"
#include <array>
#include <cassert>
#include <iostream>
#include <fstream>
static void* expected_camera;static void* expected_manager;static unsigned updates,notifies;static int clamp_flag,scene_flag;
static float* __fastcall position(void* self,void*,float* out){assert(self==expected_camera);out[0]=1;out[1]=2;out[2]=3;return out;}
static void __fastcall update(void* self,void*,int clamp,int scene){assert(self==expected_camera);++updates;clamp_flag=clamp;scene_flag=scene;}
static void __fastcall notify(void* self,void*,void* camera,const float* before){assert(self==expected_manager&&camera==expected_camera&&before[0]==1&&before[1]==2&&before[2]==3);++notifies;}
struct Patch {
 unsigned char* at{};std::array<unsigned char,5> saved{};
 Patch(uintptr_t address,void* replacement):at(reinterpret_cast<unsigned char*>(address)){DWORD old{};memcpy(saved.data(),at,5);assert(VirtualProtect(at,5,PAGE_EXECUTE_READWRITE,&old));at[0]=0xe9;uint32_t delta=uint32_t(reinterpret_cast<uintptr_t>(replacement)-(address+5));memcpy(at+1,&delta,4);FlushInstructionCache(GetCurrentProcess(),at,5);DWORD unused{};assert(VirtualProtect(at,5,old,&unused));}
 ~Patch(){DWORD old{};assert(VirtualProtect(at,5,PAGE_EXECUTE_READWRITE,&old));memcpy(at,saved.data(),5);FlushInstructionCache(GetCurrentProcess(),at,5);DWORD unused{};assert(VirtualProtect(at,5,old,&unused));}
};
static void put(void* where,unsigned offset,uintptr_t value){memcpy(static_cast<unsigned char*>(where)+offset,&value,4);}
static float& value(void* camera){return *reinterpret_cast<float*>(static_cast<unsigned char*>(camera)+0x13c);}
int wmain(int argc,wchar_t** argv){
 assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned calls=0;
 for(int input=1;input<argc;++input){
  HMODULE image=LoadLibraryExW(argv[input],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);uintptr_t base=reinterpret_cast<uintptr_t>(image);assert(zoom181::admitted(base));
  {
   Patch lower(base+0x86d3a0,reinterpret_cast<void*>(&update));Patch scene(base+0x697d80,reinterpret_cast<void*>(&notify));assert(!zoom181::admitted(base));
   std::array<unsigned char,0x160> camera{};std::array<uintptr_t,0x140/4> vtable{};std::array<unsigned char,16> context{};std::array<unsigned char,0x14e4> root{};int manager=1;
   expected_camera=camera.data();expected_manager=&manager;vtable[0x34/4]=reinterpret_cast<uintptr_t>(&position);put(camera.data(),0,reinterpret_cast<uintptr_t>(vtable.data()));put(camera.data(),0x138,reinterpret_cast<uintptr_t>(context.data()));put(context.data(),8,reinterpret_cast<uintptr_t>(root.data()));put(root.data(),0x14e0,reinterpret_cast<uintptr_t>(&manager));
   using Relative=void(__thiscall*)(void*,float);auto relative=reinterpret_cast<Relative>(base+zoom181::relative_rva);
   using Absolute=void(__thiscall*)(void*,float,int,int);auto absolute=reinterpret_cast<Absolute>(base+0x904b50);
   for(unsigned i=0;i<1000;++i){value(camera.data())=.5f;float amount=i&1?.03125f:-.03125f;unsigned before=updates;relative(camera.data(),amount);assert(std::fabs(value(camera.data())-(.5f+amount*.01f))<1e-7f&&updates==before+1&&clamp_flag==1&&scene_flag==0);++calls;}
   value(camera.data())=.5f;relative(camera.data(),.001f);assert(value(camera.data())>.5f&&value(camera.data())<.50002f);++calls;
   absolute(camera.data(),.47265625f,1,0);assert(value(camera.data())==.47265625f&&clamp_flag==1&&scene_flag==0);++calls;
   assert(updates==notifies);
  }
  assert(zoom181::admitted(base));assert(FreeLibrary(image));
 }
 for(unsigned hz:{30u,60u,125u,250u}){float pending=0,total=0;unsigned nonzero=0;for(unsigned i=0;i<hz;++i){assert(zoom181::enqueue(pending,2.f/float(hz)));total+=pending;if(pending)++nonzero;pending=0;}assert(std::fabs(total+57.6f)<.001f&&nonzero==hz);}
 float pending=0;for(unsigned i=0;i<10000;++i)assert(zoom181::enqueue(pending,.2f));assert(pending==-zoom181::maximum_pending);assert(zoom181::enqueue(pending,-.01f)&&pending>0&&pending<1);float preserved=pending;assert(!zoom181::enqueue(pending,NAN)&&pending==preserved);assert(!zoom181::valid_amount(INFINITY));
 std::cout<<"PASS smooth zoom181: "<<calls<<" actual native float/ABI calls; stock+Bugfix guards/relocation/restoration;250Hz fractional delivery and bounded reversal. Lower native scene/clamp are explicit providers; no game executed.\n";
}
