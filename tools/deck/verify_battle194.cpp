// Original native group decision and modifier cache in an owned, inert image.
// No game, Steam, live inputs, selection objects or runtime hooks are started.
#define NOMINMAX
#include "../../native/deck/native_guard175.h"
#include "../../native/deck/channel181.h"
#include <array>
#include <cassert>
#include <iostream>
#include <fstream>
static unsigned cases{},assigned{},group_index{},selection_count{};
#define CHECK(e) do{if(!(e)){std::cerr<<"battle194 assertion line "<<__LINE__<<": "<<#e<<"\n";ExitProcess(3);}++cases;}while(0)
struct Patch {
 unsigned char* at;std::vector<unsigned char> saved;
 Patch(uintptr_t address,const void* bytes,unsigned n):at(reinterpret_cast<unsigned char*>(address)),saved(n){DWORD old{},unused{};memcpy(saved.data(),at,n);CHECK(VirtualProtect(at,n,PAGE_EXECUTE_READWRITE,&old));memcpy(at,bytes,n);FlushInstructionCache(GetCurrentProcess(),at,n);CHECK(VirtualProtect(at,n,old,&unused));}
 ~Patch(){DWORD old{},unused{};VirtualProtect(at,saved.size(),PAGE_EXECUTE_READWRITE,&old);memcpy(at,saved.data(),saved.size());FlushInstructionCache(GetCurrentProcess(),at,saved.size());VirtualProtect(at,saved.size(),old,&unused);}
};
static std::array<unsigned char,5> jump(uintptr_t at,void* fn){std::array<unsigned char,5> b{0xe9};uint32_t d=uint32_t(uintptr_t(fn)-at-5);memcpy(b.data()+1,&d,4);return b;}
static HWND WINAPI foreground(){return reinterpret_cast<HWND>(7);}
static void** imported(uintptr_t base,const char* wanted){auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(base);auto nt=reinterpret_cast<IMAGE_NT_HEADERS*>(base+dos->e_lfanew);auto row=reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);for(;row->Name;++row){auto name=reinterpret_cast<IMAGE_THUNK_DATA*>(base+row->OriginalFirstThunk),slot=reinterpret_cast<IMAGE_THUNK_DATA*>(base+row->FirstThunk);for(;name->u1.AddressOfData;++name,++slot)if(!IMAGE_SNAP_BY_ORDINAL(name->u1.Ordinal)){auto n=reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base+name->u1.AddressOfData);if(!strcmp(reinterpret_cast<const char*>(n->Name),wanted))return reinterpret_cast<void**>(&slot->u1.Function);}}return nullptr;}
static void __fastcall create_group(void* object,void*,unsigned index){++assigned;group_index=index;selection_count=reinterpret_cast<unsigned*>(object)[10];}
static void __fastcall refresh_group(void*,void*,unsigned){}
static deck174::Frame frame(unsigned n,uint64_t time,uint64_t buttons=0){deck174::Frame f;f.sequence=n;f.epoch=1;f.sample_us=time;f.buttons=buttons;f.source=deck174::Source::Simulation;f.active_buttons=deck174::all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
int wmain(int argc,wchar_t** argv){
 CHECK(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 using namespace deck174;
 for(int file=1;file<argc;++file){HMODULE mapped=LoadLibraryExW(argv[file],nullptr,DONT_RESOLVE_DLL_REFERENCES);CHECK(mapped);auto base=uintptr_t(mapped);CHECK(deck175::native_guards(base));{
 auto slot=imported(base,"GetForegroundWindow");CHECK(slot);void* fg=reinterpret_cast<void*>(&foreground);Patch imp(uintptr_t(slot),&fg,4);HWND owner=reinterpret_cast<HWND>(7);Patch window(base+0x1b30f48,&owner,4);
 unsigned char zero[256]{};Patch keymem(base+0x1b30f58,zero,256),cache(base+0x1bbe84e,zero,5);
 int groups[10];for(int& n:groups)n=-1;Patch slots(base+0x17b8954,groups,sizeof groups);
 unsigned char ret=0xc3;Patch selection_provider(base+0xf97e80,&ret,1);
 auto j=jump(base+0xf89a40,reinterpret_cast<void*>(&create_group));Patch allocator(base+0xf89a40,j.data(),5);j=jump(base+0xf887e0,reinterpret_cast<void*>(&refresh_group));Patch notifier(base+0xf887e0,j.data(),5);
 std::array<unsigned,32> selection{};selection[10]=3; // several selected units, no existing groups
 auto keys=reinterpret_cast<unsigned char*>(base+0x1b30f58);auto cached=reinterpret_cast<unsigned char*>(base+0x1bbe84f);
 auto native_group=reinterpret_cast<void(__thiscall*)(void*,unsigned)>(base+0xf91e40);
 auto native_refresh=reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(base+0xf92ae0);
 const Button buttons[]={Button::Up,Button::Down,Button::Left,Button::Right};
 for(unsigned index=0;index<4;++index){
  deck181::Controls controls;auto f=frame(1,1000000);controls.sample(f,Context::RTS,f.sample_us);
  f=frame(2,1008000,bit(buttons[index]));CHECK(!controls.sample(f,Context::RTS,f.sample_us).shortcuts);
  // Realistic input cadence through the full550ms hold; no stale-frame bypass.
  deck181::Output out;for(unsigned n=3;n<=70;++n){f=frame(n,1008000+uint64_t(n-2)*8000,bit(buttons[index]));out=controls.sample(f,Context::RTS,f.sample_us);CHECK(!out.shortcuts);}
  f=frame(72,1560000,bit(buttons[index]));out=controls.sample(f,Context::RTS,f.sample_us);CHECK(out.shortcuts==(1u<<(deck183::Assign1+index)));
  deck181::ShortcutQueue183 queue;CHECK(queue.put(deck183::Assign1+index,1560));deck183::Chord chord;CHECK(queue.take(1561,chord)&&chord.scan==index+2&&chord.control);
  memset(keys,0,256);native_refresh(0,0);assigned=0;
  // The previous merge left native's separate modifier cache unchanged: its
  // actual group handler takes recall and silently does nothing for no group.
  keys[chord.scan]|=0x80;keys[0x1d]|=0x80;CHECK(!*cached);native_group(selection.data(),index);CHECK(assigned==0);
  memset(keys,0,256);deck181::Sample s;deck181::merge_rts_keys194(keys,s,&chord,base);CHECK(*cached&&keys[index+2]);
  native_group(selection.data(),index);CHECK(assigned==1&&group_index==index&&selection_count==3);
  memset(keys,0,256);deck181::merge_rts_keys194(keys,s,nullptr,base);CHECK(!*cached&&!keys[index+2]);
  CHECK(!queue.take(1562,chord));f=frame(73,1568000,bit(buttons[index]));CHECK(!controls.sample(f,Context::RTS,f.sample_us).shortcuts);f=frame(74,1576000);CHECK(!controls.sample(f,Context::RTS,f.sample_us).shortcuts);
 }
 // Physical right Ctrl survives the merge/release, with the original cache.
 memset(keys,0,256);keys[0x9d]=0x80;deck181::Sample s;deck181::merge_rts_keys194(keys,s,nullptr,base);CHECK(*cached&&keys[0x9d]==0x80);
 memset(keys,0,256);deck181::merge_rts_keys194(keys,s,nullptr,base);CHECK(!*cached);
 }
 CHECK(deck175::native_guards(base));CHECK(FreeLibrary(mapped));}
 // Actual public button policy + key merge: A holds Space, never SelectAll;
 // L4+R4 emits Ctrl+A once and does not carry its own Shift into that chord.
 deck181::Controls c;auto f=frame(1,1000000);c.sample(f,Context::RTS,f.sample_us);unsigned char keys[256]{};
 for(unsigned i=2;i<12;++i){f=frame(i,1000000+i*8000,bit(Button::A));auto o=c.sample(f,Context::RTS,f.sample_us);CHECK(o.held(deck181::Space)&&!o.shortcuts);deck181::Sample s;std::copy(o.keys.begin(),o.keys.end(),s.keys);memset(keys,0,256);deck181::merge_rts_keys194(keys,s,nullptr,0);CHECK(keys[0x39]&&!keys[0x1e]);}
 f=frame(12,1100000);CHECK(!c.sample(f,Context::RTS,f.sample_us).held(deck181::Space));
 f=frame(13,1108000,bit(Button::L4));c.sample(f,Context::RTS,f.sample_us);f=frame(14,1116000,bit(Button::L4)|bit(Button::R4));auto o=c.sample(f,Context::RTS,f.sample_us);CHECK(o.shortcuts==(1u<<deck183::SelectAll));
 deck181::Sample s;std::copy(o.keys.begin(),o.keys.end(),s.keys);memset(keys,0,256);deck181::merge_rts_keys194(keys,s,&deck183::chords[deck183::SelectAll],0);CHECK(keys[0x1e]&&keys[0x1d]&&!keys[0x2a]);
 f=frame(15,1124000,bit(Button::L4)|bit(Button::R4));CHECK(!c.sample(f,Context::RTS,f.sample_us).shortcuts);
 f=frame(16,1132000);c.sample(f,Context::RTS,f.sample_us);f=frame(17,1140000,bit(Button::R4));CHECK(c.sample(f,Context::RTS,f.sample_us).shortcuts==(1u<<deck183::ToggleMelee));
 std::cout<<"PASS battle194: "<<cases<<" group modifier/actual native assignment and battle binding checks; selection allocation/notification providers mocked; no game or inputs\n";
}
