// Owned offline DLL: separate MinHook instance, like the gameplay adapter.
#include "../../native/deck/keyboard_handoff196.h"
using Poll=bool(__cdecl*)();
static deck196::PollOwnership owner;static Poll original;static void* target;static unsigned calls;
static bool __cdecl poll(){++calls;return original();}
extern "C" __declspec(dllexport) bool __cdecl owner_create(void* at){
 target=at;return owner.prepare(at,reinterpret_cast<void*>(&poll))&&MH_Initialize()==MH_OK&&
 MH_CreateHook(at,reinterpret_cast<void*>(&poll),reinterpret_cast<void**>(&original))==MH_OK&&
 MH_EnableHook(at)==MH_OK&&owner.adopt_enabled();
}
extern "C" __declspec(dllexport) bool __cdecl owner_acquire(){return owner.acquire();}
extern "C" __declspec(dllexport) bool __cdecl owner_release(){return owner.release();}
extern "C" __declspec(dllexport) bool __cdecl owner_destroy(){return owner.release()&&MH_RemoveHook(target)==MH_OK&&MH_Uninitialize()==MH_OK;}
extern "C" __declspec(dllexport) unsigned __cdecl owner_calls(){return calls;}
