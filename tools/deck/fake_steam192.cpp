// Owned fixture only. No Steam client, game or input is started.
#include "../../native/deck/device192.h"
#include <cstring>
static int mode;static unsigned calls;
struct Fake final:device192::Utils010 {
 void unused0()override{__debugbreak();}
 void unused1()override{__debugbreak();}
 void unused2()override{__debugbreak();}
 void unused3()override{__debugbreak();}
 void unused4()override{__debugbreak();}
 void unused5()override{__debugbreak();}
 void unused6()override{__debugbreak();}
 void unused7()override{__debugbreak();}
 void unused8()override{__debugbreak();}
 void unused9()override{__debugbreak();}
 void unused10()override{__debugbreak();}
 void unused11()override{__debugbreak();}
 void unused12()override{__debugbreak();}
 void unused13()override{__debugbreak();}
 void unused14()override{__debugbreak();}
 void unused15()override{__debugbreak();}
 void unused16()override{__debugbreak();}
 void unused17()override{__debugbreak();}
 void unused18()override{__debugbreak();}
 void unused19()override{__debugbreak();}
 void unused20()override{__debugbreak();}
 void unused21()override{__debugbreak();}
 void unused22()override{__debugbreak();}
 void unused23()override{__debugbreak();}
 void unused24()override{__debugbreak();}
 void unused25()override{__debugbreak();}
 void unused26()override{__debugbreak();}
 void unused27()override{__debugbreak();}
 void unused28()override{__debugbreak();}
 void unused29()override{__debugbreak();}
 void unused30()override{__debugbreak();}
 void unused31()override{__debugbreak();}
 void unused32()override{__debugbreak();}
 void unused33()override{__debugbreak();}
 bool IsSteamRunningOnSteamDeck()override{++calls;if(mode==5)RaiseException(0xe1920001,0,0,nullptr);return mode==1;}
} object;
static void* bad_table[35]{};static void** bad=bad_table;
extern "C" __declspec(dllexport) void __cdecl FixtureMode192(int value){mode=value;calls=0;}
extern "C" __declspec(dllexport) unsigned __cdecl FixtureCalls192(){return calls;}
extern "C" __declspec(dllexport) int __cdecl SteamAPI_GetHSteamPipe(){return mode==2?0:12;}
extern "C" __declspec(dllexport) void* __cdecl SteamClient(){return &object;}
extern "C" __declspec(dllexport) void* __cdecl SteamAPI_ISteamClient_GetISteamUtils(void* client,int pipe,const char* version){if(client!=&object||pipe!=12||strcmp(version,"SteamUtils010"))return nullptr;return mode==3?nullptr:mode==4?static_cast<void*>(&bad):static_cast<void*>(&object);}
