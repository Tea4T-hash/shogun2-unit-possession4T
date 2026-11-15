// Execute the original BattleTimerControl selector and original pause/resume
// in owned mapped images. Only simulation-command, presentation object, speed
// observation and UI rendering endpoints are providers; no game entry runs.
#define wmain old_ui218
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/native214.h"
static float observed218=1.f;static unsigned commands218{},ui218{};static uintptr_t object218{};static uintptr_t empty218[2]{};
static float __fastcall speed218(void*,void*){return observed218;}
static void __fastcall consume218(void*,void*,float speed,unsigned char flags){assert(flags==0);observed218=speed;++commands218;}
static void __fastcall ui218update(void* self,void*,unsigned mode){assert(reinterpret_cast<uintptr_t>(self)==object218);assert(*reinterpret_cast<unsigned*>(object218+0x38)==mode);++ui218;}
static uintptr_t __cdecl presentation218(){return reinterpret_cast<uintptr_t>(empty218);}
static unsigned __fastcall unlocked218(void*,void*){return 0;}
int wmain(int argc,wchar_t** argv){assert(argc==3);unsigned cases=0;
 for(int i=1;i<argc;++i){HMODULE mapped=LoadLibraryExW(argv[i],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(mapped);auto b=reinterpret_cast<uintptr_t>(mapped);native214::admitted_image=0;assert(native214::admit(b));
  {std::vector<unsigned char> root(0xe00),env(0x28400),widget(0x328),controller(0x60),manager(0x30),session(0x50);uintptr_t vt[20]{};
   auto r=reinterpret_cast<uintptr_t>(root.data()),e=reinterpret_cast<uintptr_t>(env.data()),w=reinterpret_cast<uintptr_t>(widget.data()),c=reinterpret_cast<uintptr_t>(controller.data()),m=reinterpret_cast<uintptr_t>(manager.data()),s=reinterpret_cast<uintptr_t>(session.data());object218=c;
   auto put=[](uintptr_t at,uintptr_t v){memcpy(reinterpret_cast<void*>(at),&v,4);};put(r+0x110,e);put(r+0xd18,w);put(w+0x318,c);put(c,b+0x170b5c0);put(c+4,w);put(c+0x44,r);put(c+0x38,2);controller[0x40]=controller[0x41]=1;put(e+0x2804c,m);put(m+0x20,s);put(s,reinterpret_cast<uintptr_t>(vt));vt[0x44/4]=reinterpret_cast<uintptr_t>(&unlocked218);
   uintptr_t panel=0;Patch185 globals(b+0x1bc8194,&r,4),input_root(b+0x1bc8180,&r,4),modal(b+0x1bc73e4,&panel,4);
   auto gs=jump185(b+0x321880,reinterpret_cast<void*>(&speed218)),sc=jump185(b+0x10e58a0,reinterpret_cast<void*>(&consume218)),uc=jump185(b+0x10e1a60,reinterpret_cast<void*>(&ui218update)),pc=jump185(b+0x31e6e0,reinterpret_cast<void*>(&presentation218));
   Patch185 getter(b+0x321880,gs.data(),5),consumer(b+0x10e58a0,sc.data(),5),ui(b+0x10e1a60,uc.data(),5),presentation(b+0x31e6e0,pc.data(),5);
   const unsigned char selector[]={0x83,0xec,0x28};assert(!memcmp(reinterpret_cast<void*>(b+0x10e3380),selector,sizeof selector));
   native214::BattleSpeed speed;observed218=1;commands218=ui218=0;
   bool first=speed.apply(b,r,e,false);std::cout<<"native controller first result="<<first<<" speed="<<observed218<<" mode="<<*reinterpret_cast<unsigned*>(c+0x38)<<" commands="<<commands218<<" ui="<<ui218<<std::endl;assert(first&&observed218==.4f&&*reinterpret_cast<unsigned*>(c+0x38)==1);++cases;
   assert(speed.apply(b,r,e,false)&&observed218==1.f&&*reinterpret_cast<unsigned*>(c+0x38)==2);++cases;
   assert(speed.apply(b,r,e,false)&&observed218==.4f);++cases;
   assert(speed.apply(b,r,e,true)&&observed218==0.f&&*reinterpret_cast<unsigned*>(c+0x3c)==1);++cases;
   assert(speed.apply(b,r,e,true)&&observed218==.4f);++cases;
   assert(speed.apply(b,r,e,false)&&observed218==1.f);++cases;
   assert(speed.apply(b,r,e,true)&&observed218==0.f);assert(speed.apply(b,r,e,true)&&observed218==1.f);cases+=2;
   unsigned before=commands218;controller[0x40]=0;assert(!speed.apply(b,r,e,false));controller[0x40]=1;controller[0x41]=0;assert(!speed.apply(b,r,e,false));controller[0x41]=1;cases+=2;
   put(c+0x44,r+4);assert(!speed.apply(b,r,e,false));put(c+0x44,r);put(c+4,w+4);assert(!speed.apply(b,r,e,false));put(c+4,w);put(c,b+0x1709cc8);assert(!speed.apply(b,r,e,false));put(c,b+0x170b5c0);cases+=3;
   env[0x28254]=1;assert(!speed.apply(b,r,e,false));env[0x28254]=0;env[0x281e8]=1;assert(!speed.apply(b,r,e,false));env[0x281e8]=0;assert(commands218==before);cases+=2;
   // Native eligibility refusal is observable: no fake success/queue claim.
   controller[0x40]=0;reinterpret_cast<void(__thiscall*)(void*,unsigned)>(b+0x10e3380)(controller.data(),1);assert(commands218==before);controller[0x40]=1;++cases;
   assert(ui218==8&&commands218==8);assert(!memcmp(reinterpret_cast<void*>(b+0x10e3380),selector,sizeof selector));++cases;
  }assert(FreeLibrary(mapped));
 }
 std::cout<<"PASS speed218: "<<cases<<" original controller selector/pause/resume/identity/eligibility cases; simulation and presentation providers only; no gameplay\n";
}
