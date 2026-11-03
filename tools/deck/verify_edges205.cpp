// Execute both actual option-query/CMP/JZ sequences in an owned mapped engine.
// No DllMain, game startup, graphics, Steam API or live process is involved.
#define NOMINMAX
#include "../../native/deck/camera_edges205.h"
#include <array>
#include <cassert>
#include <iostream>
#include <vector>
struct Patch {
 uintptr_t at;std::vector<unsigned char> bytes;
 Patch(uintptr_t p,const void* data,size_t n):at(p),bytes(n){memcpy(bytes.data(),reinterpret_cast<void*>(p),n);write(data,n);}
 void write(const void* p,size_t n){DWORD old{},unused{};assert(VirtualProtect(reinterpret_cast<void*>(at),n,PAGE_EXECUTE_READWRITE,&old));memcpy(reinterpret_cast<void*>(at),p,n);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(at),n);assert(VirtualProtect(reinterpret_cast<void*>(at),n,old,&unused));}
 ~Patch(){write(bytes.data(),bytes.size());}
};
static std::array<unsigned char,5> jump(uintptr_t at,void* to){std::array<unsigned char,5>b{0xe9};auto d=uint32_t(reinterpret_cast<uintptr_t>(to)-at-5);memcpy(b.data()+1,&d,4);return b;}
static unsigned calls{};static unsigned char preference=1;
static unsigned char* __fastcall get_option(void*,void*,unsigned key){assert(key==0x48);++calls;return &preference;}
static uintptr_t entry{};
__declspec(naked) static unsigned invoke(void* owner){__asm{
 mov ecx,[esp+4]
 mov eax,[ecx]
 push 48h
 jmp dword ptr [entry]
}}
__declspec(naked) static void allowed(){__asm{mov eax,1
 ret}}
__declspec(naked) static void blocked(){__asm{xor eax,eax
 ret}}
static unsigned checked(void* owner){unsigned result{},before{},after{},saved_b{},saved_s{},saved_d{};
 __asm {
 mov before,esp
 mov ebx,12345678h
 mov esi,23456789h
 mov edi,3456789ah
 push owner
 call invoke
 add esp,4
 mov result,eax
 mov after,esp
 mov saved_b,ebx
 mov saved_s,esi
 mov saved_d,edi
 }
 assert(before==after&&saved_b==0x12345678&&saved_s==0x23456789&&saved_d==0x3456789a);return result;
}
static unsigned exercise(uintptr_t base,bool suppressed){
 unsigned count=0;std::array<uintptr_t,8> table{};table[7]=reinterpret_cast<uintptr_t>(&get_option);uintptr_t registry=reinterpret_cast<uintptr_t>(table.data());
 const unsigned sites[]={0x30481b,0x8fb626},pass[]={0x304829,0x8fb634},skip[]={0x304a95,0x8fb7f9};
 for(unsigned i=0;i<2;++i){
  auto a=jump(base+pass[i],reinterpret_cast<void*>(&allowed)),b=jump(base+skip[i],reinterpret_cast<void*>(&blocked));
  Patch tail1(base+pass[i],a.data(),5),tail0(base+skip[i],b.data(),5);entry=base+sites[i];
  for(unsigned value=0;value<2;++value)for(auto pos:{std::pair<float,float>{0,0},{.5f,0},{1,0},{0,.5f},{.5f,.5f},{1,.5f},{0,1},{.5f,1},{1,1}}){
   preference=static_cast<unsigned char>(value);unsigned previous=calls;std::array<unsigned char,40> cursor{};memcpy(cursor.data(),&pos.first,4);memcpy(cursor.data()+4,&pos.second,4);cursor[0x15]=1;
   Patch native_cursor(base+0x17acf84,cursor.data(),cursor.size());
   auto result=checked(&registry);assert(result==(suppressed?0:value));assert(calls==previous+(suppressed?0:1));assert(preference==value);
   assert(!memcmp(reinterpret_cast<void*>(base+0x17acf84),cursor.data(),cursor.size()));++count;
  }
 }
 // The ordinary settings getter, including writes through its returned pointer,
 // is untouched. Suppression is confined to the two camera consumers.
 preference=1;assert(get_option(&registry,nullptr,0x48)==&preference&&preference==1);
 return count;
}
int wmain(int argc,wchar_t** argv){assert(argc==3);unsigned cases=0;SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 for(int which=1;which<argc;++which){
  auto image=LoadLibraryExW(argv[which],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);deck205::Edges edge;
  assert(edge.unchanged(base));assert(edge.install(0,false)&&!edge.base&&!edge.ready);
  cases+=exercise(base,false);
  // Either occupied or changed site refuses before modifying its partner.
  for(auto at:{base+0x30481b,base+0x8fb626}){unsigned char foreign=0xcc;Patch patch(at,&foreign,1);assert(!edge.install(base,true)&&!edge.base);}
  assert(edge.unchanged(base));assert(edge.install(base,true)&&edge.ready);assert(edge.install(base,true));
  cases+=exercise(base,true);
  assert(edge.disable()&&edge.unchanged(base));cases+=exercise(base,false);
  for(auto p:edge.targets)assert(MH_RemoveHook(reinterpret_cast<void*>(p))==MH_OK);
  assert(FreeLibrary(image));
 }
 std::cout<<"PASS edges205: "<<cases<<" original native campaign/battle option branches; stock+Bugfix, Deck suppression, desktop/rollback, ABI, cursor/preference preservation and occupied-site refusal. No live game.\n";
}
