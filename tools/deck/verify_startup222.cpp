#define NOMINMAX
#include "../../native/deck/native_input185.h"
#include "../../native/deck/startup_handoff222.h"
#include <cassert>
#include <iostream>
static deck185::NativeInput frontend;static unsigned polls;static bool __cdecl campaign(){++polls;return frontend.original();}
using Create=bool(__cdecl*)(uintptr_t);static Create create;static uintptr_t image;static HANDLE finished;static bool created;
static DWORD WINAPI loader(void*){deck222::StartupGuard guard;created=guard.acquire(true)&&create(image);SetEvent(finished);return 0;}
int wmain(int argc,wchar_t**argv){assert(argc==2);unsigned cases=0;auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x1d88000,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE));assert(memory);image=uintptr_t(memory);auto at=memory+0xbd75f0;auto current=memory+0x1b30f58;auto previous=memory+0x1b31058;
unsigned char stub[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe,0,0,0,0,0xbf,0,0,0,0,0xf3,0xa5,0x5f,0x5e,0xb0,1,0xc3};uintptr_t cur=uintptr_t(current),prev=uintptr_t(previous);memcpy(stub+8,&cur,4);memcpy(stub+13,&prev,4);memcpy(at,stub,sizeof stub);FlushInstructionCache(GetCurrentProcess(),at,sizeof stub);
assert(frontend.create(image,&campaign)&&frontend.enable());auto dll=LoadLibraryExW(argv[1],nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);assert(dll);create=reinterpret_cast<Create>(GetProcAddress(dll,"startup_create"));auto error=reinterpret_cast<const char*(__cdecl*)()>(GetProcAddress(dll,"startup_error"));auto destroy=reinterpret_cast<bool(__cdecl*)()>(GetProcAddress(dll,"startup_destroy"));assert(create&&error&&destroy);
// Reproduce the actual reported startup error while the bridge still owns it.
auto failed=create(image);std::cerr<<"Reproduced startup: returned="<<failed<<" error="<<error()<<"\n";assert(!failed&&strstr(error(),"Native keyboard poll signature differs"));++cases;assert(at[0]==0xe9);++cases;
deck177::Lease front;assert(front.acquire(deck177::Owner::Frontend));finished=CreateEventW(nullptr,TRUE,FALSE,nullptr);assert(finished);auto thread=CreateThread(nullptr,0,loader,nullptr,0,nullptr);assert(thread);auto deadline=GetTickCount64()+3000;while(!deck222::startup().active()&&GetTickCount64()<deadline)Sleep(1);assert(deck222::startup().active()&&WaitForSingleObject(finished,0)==WAIT_TIMEOUT);++cases;
// Same production ordering as frontend worker: exact original poll restore
// first, then owner lease release. Loader may not validate early.
assert(frontend.disable()&&!memcmp(at,stub,sizeof stub));front.release();assert(WaitForSingleObject(finished,3000)==WAIT_OBJECT_0&&created);++cases;assert(WaitForSingleObject(thread,1000)==WAIT_OBJECT_0&&!deck222::startup().active());++cases;CloseHandle(thread);CloseHandle(finished);
using Poll=bool(__cdecl*)();auto native=reinterpret_cast<Poll>(at);auto n=polls;assert(native()&&polls==n);++cases;assert(!frontend.enable());++cases;assert(destroy()&&!memcmp(at,stub,sizeof stub));++cases;
for(unsigned i=0;i<30;++i){assert(frontend.enable());{deck222::StartupGuard g;assert(g.acquire(true)&&deck222::startup().active());assert(frontend.disable());assert(create(image));assert(native());assert(destroy());}assert(!deck222::startup().active()&&!memcmp(at,stub,sizeof stub));++cases;}
// PC is a pure bypass; nested request and deadline fail closed.
{deck222::StartupGuard pc;assert(pc.acquire(false)&&!deck222::startup().active());++cases;}
deck222::StartupHandoff isolated;uint64_t tick=0;assert(isolated.request()&&!isolated.request());++cases;assert(!isolated.wait([](){return true;},[&](){return tick;},[&](){tick+=8;},24));++cases;assert(isolated.active());isolated.release();assert(!isolated.active());++cases;
assert(MH_RemoveHook(at)==MH_OK&&MH_Uninitialize()==MH_OK);assert(!memcmp(at,stub,sizeof stub));FreeLibrary(dll);VirtualFree(memory,0,MEM_RELEASE);
std::cout<<"PASS startup222: "<<cases<<" actual production keyboard-startup/two-DLL handoff cases; exact reported error reproduced before correction;30 roundtrips; device COM tail is provider; no game or live input\n";
}
