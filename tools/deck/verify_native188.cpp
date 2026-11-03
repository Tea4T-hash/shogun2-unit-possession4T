// Actual admitted native bodies in inert owned images. No engine entry, game
// worker, Steam API or OS input is run. Only dependent providers are mocked.
#define wmain unused_native185_main
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/native_modifiers188.h"
#include "../../native/deck/campaign_keys185.h"
#include "../../native/deck/presentation188.h"
#include "../../native/deck/help_input188.h"
#include "../../native/deck/menu178.h"
static deck188::Modifiers* modifier188;
static SHORT WINAPI original188(int){return 1;}
static SHORT WINAPI foreign188(int){return 2;}
static SHORT WINAPI replacement188(int key){return modifier188->merge(key,1,1000,reinterpret_cast<HWND>(7));}
static unsigned char manager188[0x50]{},component188[0x50]{};static unsigned opens188{},inits188{};
static void* __cdecl manager_get188(){return manager188;}
static void* __fastcall string188(void* self,void*,const char* value){memcpy(self,&value,4);return self;}
static void __fastcall destruct188(void*,void*){}
static void __fastcall panel_open188(void* manager,void*,const char** key,unsigned a,unsigned b){assert(manager==manager188&&!strcmp(*key,"clan")&&!a&&!b);++opens188;}
static void* __fastcall root_find188(void*,void*,const char** key){assert(!strcmp(*key,"clan"));return component188;}
static uintptr_t __fastcall initialize188(void* self,void*,const char* key,unsigned a,unsigned b,unsigned c){assert(self==component188+0x18&&!strcmp(key,"Initialise")&&!a&&!b&&!c);++inits188;return 0;}
static void __fastcall wrap188(void*,void*,uintptr_t value){assert(!value);}
static uintptr_t image_base188;
static LONG WINAPI fault188(EXCEPTION_POINTERS* p){fprintf(stderr,"Fixture fault %08lx at image RVA %lx\n",p->ExceptionRecord->ExceptionCode,unsigned(p->ContextRecord->Eip-image_base188));fflush(stderr);return EXCEPTION_EXECUTE_HANDLER;}
struct Menu188 {unsigned left{},right{},moves{};void release_owned(bool,bool){}void reset_cursor(){}bool pointer(float,float){++moves;return true;}bool scroll(float){return true;}bool button(bool primary,bool pressed){if(pressed)(primary?left:right)++;return true;}};
int wmain(int argc,wchar_t** argv){assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned cases=0;SetUnhandledExceptionFilter(&fault188);
 static_assert(sizeof(deck188::Modifiers::Status)==56,"SSH modifier ABI");static_assert(sizeof(deck188::Presentation)==48,"SSH presentation ABI");
 for(int n=1;n<argc;++n){HMODULE image=LoadLibraryExW(argv[n],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);image_base188=base;assert(deck185::native_admitted(base,false,0x122fec0));
  auto slot=deck188::Modifiers::find(base);assert(slot==reinterpret_cast<void**>(base+0x15a1558));auto raw=*slot;
  {void* physical=reinterpret_cast<void*>(&original188);Patch185 imported(reinterpret_cast<uintptr_t>(slot),&physical,4);deck188::Modifiers mods;modifier188=&mods;assert(mods.install(base,&replacement188,reinterpret_cast<void*>(&original188))&&*slot==reinterpret_cast<void*>(&replacement188));++cases;
   auto call=reinterpret_cast<deck188::AsyncKey>(*slot);mods.publish(true,false,reinterpret_cast<HWND>(7),1000);assert(USHORT(call(VK_LCONTROL))==0x8001&&call('A')==1);++cases;
   assert(mods.merge(VK_CONTROL,1,1101,reinterpret_cast<HWND>(7))==1&&mods.merge(VK_CONTROL,1,1000,reinterpret_cast<HWND>(8))==1);++cases;
   mods.clear();assert(call(VK_CONTROL)==1&&USHORT(mods.merge(VK_CONTROL,SHORT(0x8001),1000,reinterpret_cast<HWND>(7)))==0x8001);++cases;
   for(unsigned i=0;i<1000;++i){mods.publish(i%2,!(i%2),reinterpret_cast<HWND>(7),1000);assert(USHORT(call(VK_CONTROL))==(i%2?0x8001:1)&&USHORT(call(VK_LSHIFT))==(i%2?1:0x8001));}++cases;
   assert(mods.remove(&replacement188)&&*slot==physical);++cases;
   deck188::Modifiers refused;void* alien=reinterpret_cast<void*>(&foreign188);{Patch185 occupied(reinterpret_cast<uintptr_t>(slot),&alien,4);assert(!refused.install(base,&replacement188,reinterpret_cast<void*>(&original188))&&*slot==alien);++cases;}
  }assert(*slot==raw);
  // The ORIGINAL Clan button opens the panel and invokes its Initialise entry.
  auto mgr=jump185(base+0x10ae9e0,reinterpret_cast<void*>(&manager_get188));auto str=jump185(base+0xecfa0,reinterpret_cast<void*>(&string188));auto dtor=jump185(base+0xa0b00,reinterpret_cast<void*>(&destruct188));auto open=jump185(base+0x10b2310,reinterpret_cast<void*>(&panel_open188));auto find=jump185(base+0xf8cf70,reinterpret_cast<void*>(&root_find188));auto init=jump185(base+0x127a130,reinterpret_cast<void*>(&initialize188));auto wrap=jump185(base+0x9f9b0,reinterpret_cast<void*>(&wrap188));auto drop=jump185(base+0x1d0dd0,reinterpret_cast<void*>(&destruct188));
  {Patch185 p0(base+0x10ae9e0,mgr.data(),5),p1(base+0xecfa0,str.data(),5),p2(base+0xa0b00,dtor.data(),5),p3(base+0x10b2310,open.data(),5),p4(base+0xf8cf70,find.data(),5),p5(base+0x127a130,init.data(),5),p6(base+0x9f9b0,wrap.data(),5),p7(base+0x1d0dd0,drop.data(),5);
   std::cerr<<"Native188 Clan call\n";auto original=reinterpret_cast<void(__stdcall*)(unsigned)>(base+0x122fec0);opens188=inits188=0;original(0);assert(opens188==1&&inits188==1);++cases;
   std::cerr<<"Native188 modal call\n";using Modal=bool(__thiscall*)(void*);auto modal=reinterpret_cast<Modal>(base+0x1096240);for(unsigned flags=0;flags<4;++flags){unsigned a=flags&1,b=flags&2;memcpy(manager188+0x30,&a,4);memcpy(manager188+0x3c,&b,4);assert(modal(manager188)==(flags!=0));++cases;}
  }assert(deck185::native_admitted(base));assert(FreeLibrary(image));
 }
 deck185::CampaignKeys keys;unsigned char current[256]{};unsigned action=0;
 keys.put(deck183::Clan,1000,1);assert(keys.apply(current,true,true,1000,1,&action)&&action==deck183::Clan&&!current[0x26]);++cases;
 keys.put(deck183::Finance|deck183::Back,1010,1);assert(keys.count==1);keys.apply(current,true,true,1010,1);assert(keys.apply(current,true,true,1011,1)&&current[1]==0x80&&!keys.count);++cases;
 deck188::presentation188.publish(3,true,1,2);deck188::Presentation read;assert(deck188::presentation188.read(read)&&read.help&&read.page==1&&read.hud_stage==2);assert(deck188::read_presentation188(GetCurrentProcessId(),read));++cases;
 deck174::Config config;deck178::Menu178 menu;Menu188 host;bool back=false;deck174::Frame f;f.connected=f.focused=f.native_bindings=true;f.source=deck174::Source::Simulation;f.active_buttons=deck174::all_buttons;f.epoch=1;f.sample_us=1000000;f.sequence=1;
 menu.sample(deck188::menu_frame188(f),f.sample_us,config,host,back);f.sequence++;f.sample_us+=8000;f.right_trigger=1;f.left_stick={1,1};f.buttons=deck174::bit(deck174::Button::Up);assert(menu.sample(deck188::menu_frame188(f),f.sample_us,config,host,back)&&host.left==1&&!host.right&&!host.moves);++cases;
 f.sequence++;f.sample_us+=8000;f.right_trigger=0;menu.sample(deck188::menu_frame188(f),f.sample_us,config,host,back);f.sequence++;f.sample_us+=8000;f.left_trigger=1;assert(menu.sample(deck188::menu_frame188(f),f.sample_us,config,host,back)&&host.left==1&&host.right==1&&!host.moves);++cases;
 std::cout<<"PASS native188: "<<cases<<" real stock/Bugfix import/Clan RET4/Initialise/modal/Back/shared-presentation checks; 2000 modifier alternations; dependent providers mocked; all patched windows restored; no game or Steam startup\n";
}
