// Actual production keyboard hook startup, in a separate MinHook DLL.
#include "../../native/outputs/shogun2-accuracy90-preview/src/identity.h"
#include "../../native/outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"
static uintptr_t base;static volatile LONG enabled;
enum {POSSESSED=2};struct TestStatus{LONG mode{},stop{},release{};unsigned lease_generation{1};};static TestStatus status;static TestStatus*state=&status;
static bool charge_input_locked146(){return false;}
#include "../../native/outputs/shogun2-accuracy90-preview/src/native_keyboard84.h"
static std::string last;
extern "C" __declspec(dllexport) bool __cdecl startup_create(uintptr_t image){
 base=image;last.clear();try{auto n=MH_Initialize();require(n==MH_OK||n==MH_ERROR_ALREADY_INITIALIZED,"Fixture hook init");create_keyboard_hook84();require(MH_EnableHook(keyboard_poll_target84)==MH_OK,"Fixture hook enable");return true;}
 catch(const std::exception&e){last=e.what();hud_visibility_close148();return false;}
}
extern "C" __declspec(dllexport) const char* __cdecl startup_error(){return last.c_str();}
extern "C" __declspec(dllexport) bool __cdecl startup_destroy(){auto ok=MH_DisableHook(keyboard_poll_target84)==MH_OK&&MH_RemoveHook(keyboard_poll_target84)==MH_OK&&MH_Uninitialize()==MH_OK;hud_visibility_close148();return ok;}
