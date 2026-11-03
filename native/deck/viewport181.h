#pragma once
// Included inside menu181. The engine UI wrapper's result-enable flag is not
// touched. This is the engine's already-guarded float Lua5.1 C API instead.
namespace viewport181 {
using PushNumber=void(__cdecl*)(void*,float);
inline bool valid(int width,int height,uint64_t tick,uint64_t now){return width>=640&&height>=360&&width<=32767&&height<=32767&&tick&&now>=tick&&now-tick<=250;}
inline PushNumber bind(){
#if defined(S2UP165_LUA_FIXTURE) && !defined(S2UP_FRONTEND181_STANDALONE)
 // Only the inert offline fixture maps the owned image without DllMain. The
 // selected pure Lua push function has no imports/global accesses; its exact
 // body is verified below before use with the matching float-Lua host.
 static HMODULE image=LoadLibraryExW((game165+L"\\Empire.Retail.dll").c_str(),nullptr,DONT_RESOLVE_DLL_REFERENCES);
#else
 HMODULE image=GetModuleHandleW(L"Empire.Retail.dll");
#endif
 if(!image)return nullptr;
 const unsigned char expected[]={0x8b,0x4c,0x24,0x04,0xf3,0x0f,0x10,0x44,0x24,0x08,0x8b,0x41,0x08,0xf3,0x0f,0x11,0x00,0xc7,0x40,0x04,0x03,0x00,0x00,0x00,0x83,0x41,0x08,0x08,0xc3};
 auto entry=reinterpret_cast<unsigned char*>(image)+0xea32b0;unsigned char actual[sizeof expected]{};SIZE_T count{};
 if(!ReadProcessMemory(GetCurrentProcess(),entry,actual,sizeof actual,&count)||count!=sizeof actual||memcmp(actual,expected,sizeof actual))return nullptr;
 return reinterpret_cast<PushNumber>(entry);
}
inline int push(void* lua,int width,int height,uint64_t tick,uint64_t now,PushNumber number){
 if(!lua||!number||!valid(width,height,tick,now))return 0;
 // A Lua C function receives LUA_MINSTACK spare slots (20 in this pinned ABI).
 // One incoming argument and two numeric results fit without stack growth.
 number(lua,float(width));number(lua,float(height));return 2;
}
inline int result(void* lua,int width,int height,uint64_t tick,uint64_t now){
 if(!valid(width,height,tick,now))return 0;return push(lua,width,height,tick,now,bind());
}
}
