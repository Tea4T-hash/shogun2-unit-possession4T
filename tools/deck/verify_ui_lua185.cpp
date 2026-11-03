#define NOMINMAX
#include <string>
#include "../../native/deck/native_ui185.h"
static HMODULE fixture185;
static bool bind185(){
 if(fixture185)return true;wchar_t game[32768]{};if(!GetEnvironmentVariableW(L"S2UP165_FIXTURE_GAME",game,32768))return false;
 std::wstring path=std::wstring(game)+L"\\Empire.Retail.dll";fixture185=LoadLibraryExW(path.c_str(),nullptr,DONT_RESOLVE_DLL_REFERENCES);if(!fixture185)return false;
 auto base=reinterpret_cast<uintptr_t>(fixture185);if(!deck185::native_admitted(base))return false;
 deck185::native_ui185.get_top=reinterpret_cast<deck185::GetTop>(base+0xea2d60);deck185::native_ui185.set_top=reinterpret_cast<deck185::SetTop>(base+0xea37c0);deck185::native_ui185.push_number=reinterpret_cast<deck185::PushNumber>(base+0xea32b0);return true;
}
static int __cdecl original185(void* lua){for(float n:{100.f,200.f,1280.f,1024.f})deck185::native_ui185.push_number(lua,n);deck185::current_ui185={1310,819};return 4;}
static int __cdecl empty185(void*){deck185::current_ui185={1310,819};return 0;}
static int __cdecl unconverted185(void* lua){for(float n:{100.f,200.f,1280.f,1024.f})deck185::native_ui185.push_number(lua,n);return 4;}
extern "C" __declspec(dllexport) int __cdecl s2up_ui_cursor_fixture185(void* lua){if(!bind185())return 0;deck185::native_ui185.cursor=&original185;return deck185::ui_cursor185(lua);}
extern "C" __declspec(dllexport) int __cdecl s2up_ui_empty_fixture185(void* lua){if(!bind185())return 0;deck185::native_ui185.cursor=&empty185;return deck185::ui_cursor185(lua);}
extern "C" __declspec(dllexport) int __cdecl s2up_ui_unconverted_fixture185(void* lua){if(!bind185())return 0;deck185::native_ui185.cursor=&unconverted185;return deck185::ui_cursor185(lua);}
