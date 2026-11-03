#pragma once
// Included inside menu181 after the existing arity, string and viewport helpers.
// Read-only native-context health. Never reset the game's parser validity flag.
namespace context181 {
struct Trace {
 uint32_t magic{0x31545843},version{1},bytes{sizeof(Trace)};
 volatile LONG sequence{};uint32_t calls{},valid{},invalid{},missing{},thread{};
 char first_invalid[96]{},first_bad_result[96]{},last_tag[96]{};
};
static Trace trace;
static SRWLOCK trace_lock=SRWLOCK_INIT;
using Lookup=void*(__cdecl*)(void*);
using PushPointer=void(__cdecl*)(void*,void*);
inline HMODULE image(){
#if defined(S2UP165_LUA_FIXTURE) && !defined(S2UP_FRONTEND181_STANDALONE)
 static HMODULE value=LoadLibraryExW((game165+L"\\Empire.Retail.dll").c_str(),nullptr,DONT_RESOLVE_DLL_REFERENCES);return value;
#else
 return GetModuleHandleW(L"Empire.Retail.dll");
#endif
}
inline Lookup bind(){
 auto module=image();if(!module)return nullptr;auto base=reinterpret_cast<uintptr_t>(module);
 // Verified original 127a210: balanced lookup[lightuserdata(lua_State*)].
 const unsigned char expected[]={0x56,0x8b,0x74,0x24,0x08,0x57,0x68,0,0,0,0,0x56,0xe8,0xaf,0x90,0xc2,0xff};
 unsigned char actual[sizeof expected]{};SIZE_T count{};
 if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(base+0x127a210),actual,sizeof actual,&count)||count!=sizeof actual)return nullptr;
 uintptr_t text{};memcpy(&text,actual+7,4);if(text!=base+0x172cb74)return nullptr;
 memset(actual+7,0,4);if(memcmp(actual,expected,sizeof expected))return nullptr;
 return reinterpret_cast<Lookup>(base+0x127a210);
}
inline int health(void* lua,Lookup lookup){
 if(!lua||!lookup)return -1;void* pointer=lookup(lua);if(!pointer)return -1;
 struct Header {void* state;unsigned char valid;};Header header{};SIZE_T size{};
 if(!ReadProcessMemory(GetCurrentProcess(),pointer,&header,sizeof header,&size)||size!=sizeof header||header.state!=lua||header.valid>1)return -1;
 return header.valid?1:0;
}
inline bool remember(const std::string& tag,int value){
 AcquireSRWLockExclusive(&trace_lock);InterlockedIncrement(&trace.sequence);
 ++trace.calls;trace.thread=GetCurrentThreadId();
 if(value>0)++trace.valid;else if(!value)++trace.invalid;else ++trace.missing;
 auto copy=[&](char* out){strncpy_s(out,96,tag.c_str(),_TRUNCATE);};
 copy(trace.last_tag);if(value<=0&&!trace.first_invalid[0])copy(trace.first_invalid);
 if(tag.compare(0,4,"bad:")==0&&!trace.first_bad_result[0])copy(trace.first_bad_result);
 InterlockedIncrement(&trace.sequence);ReleaseSRWLockExclusive(&trace_lock);return value>0;
}
inline bool observe(void* lua,const std::string& tag){
 if(tag.empty()||tag.size()>95)return false;
#ifdef S2UP165_LUA_FIXTURE
 // The generic float-Lua host has l_gt at72; the game's native VM uses68.
 // Full helper execution is tested separately with an explicit owned facade.
 return remember(tag,1);
#else
 return remember(tag,health(lua,bind()));
#endif
}
}
