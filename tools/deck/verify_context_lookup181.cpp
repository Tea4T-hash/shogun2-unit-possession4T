// Actual native helpers in an inert owned image; never an engine entry point.
// The external test host is float Lua5.1.5 (104-byte state, globals at72).
// The admitted game's VM stores globals at68. Use an explicit owned facade;
// never reinterpret an arbitrary host state as a fully ABI-compatible game VM.
#define WIN32_LEAN_AND_MEAN
#define S2UP165_LUA_FIXTURE 1
#include <windows.h>
#include <cstdint>
#include <string>
#include <cstring>
static std::wstring game165;
namespace menu181 {
#include "../../native/deck/context181.h"
}
struct Context181 {void* state;unsigned char valid;unsigned char padding[19];};
static Context181 context;
static unsigned char state181[128]{},node181[0xa4]{};
static int top181(void* L){auto p=static_cast<unsigned char*>(L);return int((*reinterpret_cast<uintptr_t*>(p+8)-*reinterpret_cast<uintptr_t*>(p+12))/8);}
static void* facade181(void* L){
 auto p=static_cast<unsigned char*>(L);
 // Validate the host's global TValue tag before any native table operation.
 if(*reinterpret_cast<unsigned*>(p+76)!=5)return nullptr;
 memset(state181,0,sizeof state181);memcpy(state181,L,104);memcpy(state181+68,p+72,8);return state181;
}
extern "C" __declspec(dllexport) int __cdecl seed181(void* L){
 if(top181(L)!=1||!facade181(L))return 0;
 wchar_t path[32768]{};auto n=GetEnvironmentVariableW(L"S2UP165_FIXTURE_GAME",path,32768);if(!n||n>=32768)return 0;game165=path;
 auto b=reinterpret_cast<uintptr_t>(menu181::context181::image());if(!b)return 0;
 const unsigned char bytes[]={0x8b,0x54,0x24,0x04,0x8b,0x44,0x24,0x08,0x8b,0x4a,0x08,0x89,0x01,0xc7,0x41,0x04,0x02,0,0,0,0x83,0x42,0x08,0x08,0xc3};
 if(memcmp(reinterpret_cast<void*>(b+0xea3230),bytes,sizeof bytes))return 0;
 context={state181,1,{}};*reinterpret_cast<unsigned*>(node181+0x9c)=3;
 auto push=reinterpret_cast<menu181::context181::PushPointer>(b+0xea3230);push(L,state181);push(L,&context);return 2;
}
extern "C" __declspec(dllexport) int __cdecl health181(void* L){
 if(top181(L)!=1)return 0;auto facade=facade181(L);if(!facade)return 0;
 return menu181::context181::health(facade,menu181::context181::bind())>0?1:0;
}
extern "C" __declspec(dllexport) int __cdecl invalid181(void* L){if(top181(L)!=1)return 0;context.valid=0;return 1;}
extern "C" __declspec(dllexport) int __cdecl restore181(void* L){if(top181(L)!=1)return 0;context.valid=1;return 1;}
extern "C" __declspec(dllexport) int __cdecl child_count181(void* L){
 if(top181(L)!=1)return 0;auto facade=facade181(L);if(!facade)return 0;
 auto b=reinterpret_cast<uintptr_t>(menu181::context181::image());if(!b)return 0;
 auto p=reinterpret_cast<unsigned char*>(b+0xf73910);
 const unsigned char bytes[]={0x56,0x8b,0xf1,0x83,0x3e,0,0x74,0x23,0xff,0x74,0x24,0x08,0xe8,0xef,0x68,0x30,0};
 if(memcmp(p,bytes,sizeof bytes))return 0;
 void* node=node181;using Call=int(__thiscall*)(void*,void*);
 auto count=reinterpret_cast<Call>(p)(&node,facade);memcpy(static_cast<unsigned char*>(L)+8,state181+8,4);return count;
}

static unsigned string_error_calls183;static int string_expected183,string_actual183;
static void __cdecl string_error183(void*,int expected,int actual){++string_error_calls183;string_expected183=expected;string_actual183=actual;}
extern "C" __declspec(dllexport) int __cdecl reject_plain_find183(void* L){
 if(top181(L)!=1)return 0;auto facade=facade181(L);if(!facade)return 0;
 auto b=reinterpret_cast<uintptr_t>(menu181::context181::image());if(!b)return 0;
 // Signature/pinned failure tail from indexed original1277430. Its only
 // diagnostic-report CALL is redirected in this owned inert mapping, never
 // a real game process; actual native type query and validity write execute.
 const unsigned char prefix[]={0x83,0xec,0x0c,0x56,0x8b,0x74,0x24,0x14,0x80,0x7e,0x04,0x00,0x74,0x68,0x6a,0xff};
 const unsigned char tail[]={0xe8,0xe1,0xa4,0x06,0x00,0x83,0xc4,0x14,0xc6,0x46,0x04,0x00};
 auto entry=reinterpret_cast<unsigned char*>(b+0x1277430);auto call=reinterpret_cast<unsigned char*>(b+0x127749a);
 if(memcmp(entry,prefix,sizeof prefix)||memcmp(call,tail,sizeof tail))return 0;
 DWORD prior{};if(!VirtualProtect(call,5,PAGE_EXECUTE_READWRITE,&prior))return 0;
 unsigned char saved[5]{};memcpy(saved,call,5);int relative=int(reinterpret_cast<uintptr_t>(&string_error183)-reinterpret_cast<uintptr_t>(call+5));memcpy(call+1,&relative,4);FlushInstructionCache(GetCurrentProcess(),call,5);
 Context181 value{facade,1,{}};unsigned char unused[24]{};using Read=void*(__cdecl*)(void*,void*);
 string_error_calls183=0;string_expected183=string_actual183=0;auto original_top=top181(facade);
 reinterpret_cast<Read>(entry)(&value,unused);
 memcpy(call,saved,5);FlushInstructionCache(GetCurrentProcess(),call,5);DWORD ignored{};VirtualProtect(call,5,prior,&ignored);
 return value.valid==0&&string_error_calls183==1&&string_expected183==4&&string_actual183==1&&top181(facade)==original_top?1:0;
}
