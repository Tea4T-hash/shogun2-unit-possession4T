#define wmain previous185_main
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/selection189.h"
#include "../../native/deck/campaign_keys185.h"
#include "../../native/deck/help_input188.h"
#undef assert
#define assert(e) do{if(!(e)){fprintf(stderr,"selection189 assertion line %u: %s\n",unsigned(__LINE__),#e);fflush(stderr);ExitProcess(3);}}while(0)
static uintptr_t fault_base189;
static LONG WINAPI fault189(EXCEPTION_POINTERS* p){fprintf(stderr,"selection189 fixture fault %08lx at image RVA %08lx\n",p->ExceptionRecord->ExceptionCode,unsigned(p->ContextRecord->Eip-fault_base189));fflush(stderr);return EXCEPTION_EXECUTE_HANDLER;}
static unsigned __fastcall identity189(void*,void*){return 42;}
static void* __cdecl no_allocation189(unsigned){return nullptr;}
static HWND WINAPI foreground189(){return reinterpret_cast<HWND>(7);}
static void** import189(uintptr_t base,const char* wanted){auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);auto d=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];auto row=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+d.VirtualAddress);for(;row->Name;++row){auto original=reinterpret_cast<IMAGE_THUNK_DATA*>(base+row->OriginalFirstThunk),slot=reinterpret_cast<IMAGE_THUNK_DATA*>(base+row->FirstThunk);for(;original->u1.AddressOfData;++original,++slot)if(!IMAGE_SNAP_BY_ORDINAL(original->u1.Ordinal)){auto name=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+original->u1.AddressOfData);if(!strcmp(reinterpret_cast<const char*>(name->Name),wanted))return reinterpret_cast<void**>(&slot->u1.Function);}}return nullptr;}
int wmain(int argc,wchar_t** argv){assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);SetUnhandledExceptionFilter(&fault189);unsigned cases=0;for(int i=1;i<argc;++i){HMODULE image=LoadLibraryExW(argv[i],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);fault_base189=base;auto slot=import189(base,"GetForegroundWindow");assert(slot);void* fn=reinterpret_cast<void*>(&foreground189);Patch185 imported(reinterpret_cast<uintptr_t>(slot),&fn,4);HWND window=reinterpret_cast<HWND>(7);Patch185 owner(base+0x1b30f48,&window,4);fprintf(stderr,"loaded/mapped/imported fixture %d\n",i);auto keys=reinterpret_cast<unsigned char*>(base+0x1b30f58);unsigned char empty[256]{};Patch185 keyboard(base+0x1b30f58,empty,256);unsigned char zero=0;Patch185 cached(base+0x1bbe84f,&zero,1);using Query=unsigned char(__stdcall*)(unsigned);using Cached=unsigned char(__cdecl*)();auto query=reinterpret_cast<Query>(base+0xbd12a0);auto ctrl=reinterpret_cast<Cached>(base+0xf90330);
 fprintf(stderr,"state patches complete\n");
 // Reproduce the old mismatch: DirectInput Ctrl down, UI selection Ctrl off.
 fprintf(stderr,"query test\n");keys[0x1d]=0x80;assert(query(0x1c)&&!ctrl());++cases;keys[0x1d]=0;
 {deck189::SelectionModifiers owned(base,MK_CONTROL);assert(query(0x1c)&&ctrl());++cases;}assert(!query(0x1c)&&!ctrl()&&!keys[0x1d]);++cases;
 keys[0x9d]=0x80;{deck189::SelectionModifiers owned(base,0);assert(query(0x69)&&ctrl());++cases;}assert(keys[0x9d]==0x80&&ctrl());++cases;keys[0x9d]=0;
 // The actual battlefield event constructor writes modifier bytes consumed
 // by RTS selection/orders; keep its real modifier queries, mock only the
 // unrelated entity-ID and allocation providers.
 {auto jump=jump185(base+0xf7440,reinterpret_cast<void*>(&no_allocation189));Patch185 alloc(base+0xf7440,jump.data(),5);uintptr_t table[2]{0,reinterpret_cast<uintptr_t>(&identity189)},object=reinterpret_cast<uintptr_t>(table);unsigned char event[16]{};
  using Constructor=void*(__thiscall*)(void*,unsigned,unsigned,void*,unsigned,unsigned);auto construct=reinterpret_cast<Constructor>(base+0x2538e0);
  {deck189::SelectionModifiers owned(base,MK_CONTROL);construct(event,1,0,&object,0,0);assert(event[0]==1&&event[3]==1&&event[2]==0);++cases;}
  {deck189::SelectionModifiers owned(base,MK_SHIFT);construct(event,1,0,&object,0,0);assert(event[2]==1&&event[3]==0);++cases;}
  {deck189::SelectionModifiers owned(base,MK_SHIFT|MK_CONTROL);construct(event,1,0,&object,0,0);assert(event[2]==1&&event[3]==1);++cases;}
  memset(event,0,sizeof event);construct(event,1,0,&object,0,0);assert(event[2]==0&&event[3]==0);++cases;
 }
 // Campaign poll holds the same DI modifiers and refreshes the actual UI
 // producer; verify the native cached consumer, not just WM modifier flags.
 {deck185::CampaignKeys campaign;auto refresh=reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(base+0xf92ae0);
  for(unsigned flags:{unsigned(MK_CONTROL),unsigned(MK_SHIFT),unsigned(MK_CONTROL|MK_SHIFT)}){
   memset(keys,0,256);campaign.hold(flags,1000,7);campaign.apply(keys,true,true,1001,7);refresh(0,0);
   assert(bool(ctrl())==bool(flags&MK_CONTROL)&&bool(keys[0x2a]&0x80)==bool(flags&MK_SHIFT));++cases;
  }
  memset(keys,0,256);campaign.clear();campaign.apply(keys,true,true,1002,7);refresh(0,0);assert(!ctrl()&&!keys[0x2a]);++cases;
 }
 keys[0x1d]=0x80;{deck189::SelectionModifiers owned(base,MK_CONTROL|MK_SHIFT);assert(ctrl()&&keys[0x2a]==0x80);++cases;}assert(keys[0x1d]==0x80&&!keys[0x2a]);++cases;
 }deck174::Frame f;f.buttons=deck174::bit(deck174::Button::L1)|deck174::bit(deck174::Button::RightPadTouch)|deck174::bit(deck174::Button::RightPadClick);f.right_pad={.3f,.7f};auto modal=deck188::campaign_menu_frame189(f,true);assert(!(modal.buttons&deck174::bit(deck174::Button::RightPadTouch))&&(modal.buttons&deck174::bit(deck174::Button::LeftPadTouch))&&modal.left_pad.y==.7f);auto map=deck188::campaign_menu_frame189(f,false);assert(map.buttons&deck174::bit(deck174::Button::RightPadTouch));std::cout<<"PASS selection189: "<<cases<<" original native Ctrl producer/query/UI-cache/battlefield-event cases on stock+Bugfix, plus right-pad modal mode shift; no game or Steam.\n";}
