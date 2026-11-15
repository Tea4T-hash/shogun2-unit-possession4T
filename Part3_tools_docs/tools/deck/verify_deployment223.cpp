// Read the actual original UI ABI from owned mapped images. No game startup.
#define wmain previous_ui223
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/battle_panels223.h"
static void put223(uintptr_t at,uintptr_t value){memcpy(reinterpret_cast<void*>(at),&value,4);}
int wmain(int argc,wchar_t** argv){assert(argc==3);unsigned cases=0;
 for(int i=1;i<argc;++i){auto image=LoadLibraryExW(argv[i],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto b=reinterpret_cast<uintptr_t>(image);native214::admitted_image=0;assert(native214::admit(b,1));
 {unsigned char manager[0x60]{},root[0x328]{},finish[0x328]{},pause[0x328]{};uintptr_t entries[2]{};
 auto m=reinterpret_cast<uintptr_t>(manager),r=reinterpret_cast<uintptr_t>(root),f=reinterpret_cast<uintptr_t>(finish),p=reinterpret_cast<uintptr_t>(pause);
 Patch185 mgr(b+0x1bc73e4,&m,4),ui(b+0x1bbe364,&r,4);
 auto setup=[&](unsigned char* widget,uintptr_t at,const char* name){put223(at,b+0x1704cdc);put223(at+0x18,b+0x1704d44);put223(at+0x88,unsigned(strlen(name)));put223(at+0x8c,unsigned(strlen(name)));put223(at+0x90,reinterpret_cast<uintptr_t>(name));put223(at+0x94,r);widget[0xf5]=1;};
 setup(finish,f,"finish_deployment");setup(pause,p,"battle_menu");
 assert(!deck223::battle_modal(b,true)&&!deck223::battle_modal(b,false));++cases;
 entries[0]=f;put223(m+0x30,1);put223(m+0x34,reinterpret_cast<uintptr_t>(entries));put223(m+0x3c,1);put223(m+0x40,reinterpret_cast<uintptr_t>(entries));
 assert(native214::modal(b)&&!deck223::battle_modal(b,true)&&deck223::battle_modal(b,false));++cases;
 entries[1]=p;put223(m+0x30,2);assert(deck223::battle_modal(b,true));++cases;put223(m+0x30,1);
 entries[0]=p;assert(deck223::battle_modal(b,true));++cases;entries[0]=f;
 setup(finish,f,"finish_deployment_extra");assert(deck223::battle_modal(b,true));++cases;setup(finish,f,"finish_deployment");
 put223(f+0x94,f);assert(deck223::battle_modal(b,true));++cases;put223(f+0x94,r);
 finish[0xf5]=0;assert(!deck223::battle_modal(b,true)&&deck223::battle_modal(b,false));++cases;finish[0xf5]=1;
 put223(f+0x18,0);assert(deck223::battle_modal(b,true));++cases;put223(f+0x18,b+0x1704d44);
 put223(f+0x88,4096);assert(deck223::battle_modal(b,true));++cases;put223(f+0x88,17);
 unsigned char changed=0x90;{Patch185 occupied(b+0xf8e840,&changed,1);assert(deck223::battle_modal(b,true));++cases;}
 put223(m+0x30,33);assert(deck223::battle_modal(b,true));++cases;put223(m+0x30,1);
 put223(m+0x34,0);assert(deck223::battle_modal(b,true));++cases;
 }assert(FreeLibrary(image));
 }
 std::cout<<"PASS deployment223: "<<cases<<" original ABI/read-only deployment exception and pause/unknown/corrupt-panel safeguards; no gameplay\n";
}
