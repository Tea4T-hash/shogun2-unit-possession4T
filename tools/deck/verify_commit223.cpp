// Isolated original commit; allocator/registry providers only. Original selection-view callback retained.
#define wmain older_ui223
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/native214.h"
#include "../../native/deck/orders175.h"
#include <unordered_set>
static std::unordered_set<void*> allocations;static unsigned registry[5]{};static unsigned key223=0x12345678;
static void* __cdecl allocate223(unsigned bytes,void*){void* p=calloc(1,bytes);assert(p);allocations.insert(p);return p;}
static void __cdecl free223(void* p){if(p){assert(allocations.erase(p)==1);free(p);}}
static void* __cdecl registry223(unsigned key){assert(key>=key223&&key<=key223+3);return registry;}
// Unit weapon definition is the only provider in the capability predicate.
static unsigned __fastcall weapon223(uintptr_t unit,void*){
 return *reinterpret_cast<const unsigned char*>(unit+0x179)?0x15:0x25;
}
static void put223(uintptr_t p,uintptr_t v){memcpy(reinterpret_cast<void*>(p),&v,4);}
int wmain(int argc,wchar_t** argv){assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned cases=0,skirmish_cases=0,fire_cases=0;
for(int f=1;f<argc;++f){auto image=LoadLibraryExW(argv[f],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto b=reinterpret_cast<uintptr_t>(image);native214::admitted_image=0;assert(native214::admit(b));{
std::vector<unsigned char> root(0xe00),env(0x28400),ui(0x328),widget(0x328),ctrl(0x60),owner(0x200),selected(0x500),queue(0x5010),unit1(0x1000),unit2(0x1000);
auto r=reinterpret_cast<uintptr_t>(root.data()),e=reinterpret_cast<uintptr_t>(env.data()),u=reinterpret_cast<uintptr_t>(ui.data()),w=reinterpret_cast<uintptr_t>(widget.data()),c=reinterpret_cast<uintptr_t>(ctrl.data()),o=reinterpret_cast<uintptr_t>(owner.data()),s=reinterpret_cast<uintptr_t>(selected.data()),q=reinterpret_cast<uintptr_t>(queue.data()),a=reinterpret_cast<uintptr_t>(unit1.data()),z=reinterpret_cast<uintptr_t>(unit2.data());uintptr_t children[]{w},units[]{a,z};
put223(r+0x110,e);put223(r+0x5b8,e);put223(r+0x5b4,o);put223(r+0x10c,b+0x170d304);put223(e+0x2826c,s);put223(s+0x4a8,o);put223(e+0x28048,q);
put223(u,b+0x1704cdc);put223(u+0x9c,1);put223(u+0xa0,reinterpret_cast<uintptr_t>(children));put223(w,b+0x1704cdc);put223(w+0x94,u);put223(w+0x318,c);put223(c,b+0x1709cc8);put223(c+4,w);put223(c+0x40,165);ctrl[0x50]=1;
put223(o+0xb0,b+0x15c4da0);put223(o+0xc0,2);put223(o+0xc4,2);put223(o+0xc8,reinterpret_cast<uintptr_t>(units));put223(a,b+0x15c4da0);put223(z,b+0x15c4da0);put223(a+4,11);put223(z+4,22);
unsigned side_key=key223+1,skirmish_key=key223+2,fire_key=key223+3;
Patch185 sideGlobal(b+0x192f634,&side_key,4),skirmishGlobal(b+0x192d2cc,&skirmish_key,4),fireGlobal(b+0x192ca68,&fire_key,4);
Patch185 rootGlobal(b+0x1bc8180,&r,4),uiGlobal(b+0x1bbe364,&u,4),keyGlobal(b+0x192d7c0,&key223,4);
auto ap=jump185(b+0xf0510,reinterpret_cast<void*>(&allocate223)),fp=jump185(b+0xf04d0,reinterpret_cast<void*>(&free223)),rp=jump185(b+0x207490,reinterpret_cast<void*>(&registry223));
Patch185 allocator(b+0xf0510,ap.data(),5),deallocator(b+0xf04d0,fp.data(),5),reg(b+0x207490,rp.data(),5);
auto current=[]{return true;};
auto read=[](uintptr_t at){uintptr_t value{};native214::value(at,value);return value;};
auto press=[&](unsigned button){
 reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned)>(b+0x10dc0c0)(c,button);
};
auto release_order=[&]{
 const auto before=read(q+4+0x5000);
 if(deck175::native_order_ready(b,r,e,read,current,true))
  reinterpret_cast<void(__thiscall*)(uintptr_t)>(b+0x10f85f0)(c);
 else
  reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned)>(b+0x10dc060)(c,0);
 return read(q+4+0x5000)>before;
};
for(unsigned button:{2u,3u,6u,7u})for(int i=0;i<30;++i){
 press(button);root[0x598]=1;assert(release_order()&&allocations.empty());
 put223(q+4+0x5000,0);++cases;
}

// Reproduce the exact live selection counts: one native group, no loose units,
 // and one entry in the fifth UI selection vector. Use the original queue writer.
 std::vector<unsigned char> group(0x500);
 auto g=reinterpret_cast<uintptr_t>(group.data());
 uintptr_t group_list[]{g};
 put223(g,b+0x15c4da0);put223(g+0x7c,31);
 put223(g+0x20,1);put223(g+0x24,reinterpret_cast<uintptr_t>(units));put223(g+0x48,e);
 put223(o+0xb4,1);put223(o+0xb8,1);put223(o+0xbc,reinterpret_cast<uintptr_t>(group_list));
 put223(o+0xc4,0);put223(o+0xe8,1);put223(o+0xec,1);put223(o+0xf0,reinterpret_cast<uintptr_t>(group_list));
 for(unsigned button:{2u,3u,6u,7u}) {
  press(button);root[0x598]=1;
  assert(release_order()&&allocations.empty());
  put223(q+4+0x5000,0);++cases;
 }
 auto read_byte=[](uintptr_t at,unsigned char& value){return native214::value(at,value);};
 auto invoke=[&](uintptr_t root,unsigned action,float value){
  reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned,float)>(b+0x10dfb80)(root,action,value);
 };
 assert(native214::admit(b,128));
 auto weapon=jump185(b+0x14ad60,reinterpret_cast<void*>(&weapon223));
 Patch185 weapon_type(b+0x14ad60,weapon.data(),5);
 auto eligible=[&](uintptr_t unit,unsigned char& value){
  value=reinterpret_cast<unsigned char(__fastcall*)(uintptr_t)>(b+0x15c2e0)(unit);
  return true;
 };
 unit1[0x179]=unit2[0x179]=1;
 put223(a+0xf18,0x10);put223(z+0xf18,0x10);
 put223(g+0x60,1);put223(g+0x64,reinterpret_cast<uintptr_t>(units+1));
 for(bool grouped:{true,false}){
  if(!grouped){put223(o+0xb8,0);put223(o+0xc4,2);put223(o+0xec,0);}
  // Compare the old shortcut's mode 2 with the explicit button orders.
  auto check_fire=[&](unsigned mode){
   const unsigned length=read(q+4+0x5000);
   const unsigned descriptor=unsigned(queue[6])<<24|unsigned(queue[7])<<16|unsigned(queue[8])<<8|queue[9];
   assert(length>=10&&descriptor==fire_key&&allocations.empty());
   const auto at=4+length-4;
   assert(queue[at]==mode&&!queue[at+1]&&!queue[at+2]&&!queue[at+3]);
   put223(q+4+0x5000,0);++fire_cases;
  };
  invoke(r,1,0.f);check_fire(2);
  unit1[0xeb0]=unit2[0xeb0]=1;
  assert(deck175::native_fire_mode(b,r,e,false,read,read_byte,eligible,current,invoke));check_fire(1);
  unit1[0xeb0]=unit2[0xeb0]=0;
  assert(deck175::native_fire_mode(b,r,e,false,read,read_byte,eligible,current,invoke));check_fire(0);
  for(bool active:{false,true}){
   unit1[0xeb2]=1;unit2[0xeb2]=active;
   assert(deck175::native_fire_mode(b,r,e,true,read,read_byte,eligible,current,invoke)&&allocations.empty());
   const unsigned length=read(q+4+0x5000);
   const unsigned descriptor=unsigned(queue[6])<<24|unsigned(queue[7])<<16|unsigned(queue[8])<<8|queue[9];
   assert(length>=7&&descriptor==skirmish_key&&queue[4+length-1]==!active);
   put223(q+4+0x5000,0);++skirmish_cases;
  }
 }
 for(unsigned used:{0x4fe0u,0x4ff0u,0x5000u,0x5001u}){
 press(2);root[0x598]=1;put223(q+4+0x5000,used);
 auto before=queue;assert(!release_order());assert(queue==before&&allocations.empty()&&*reinterpret_cast<unsigned*>(ctrl.data()+0x40)==165);put223(q+4+0x5000,0);++cases;
}
}assert(FreeLibrary(image));}
printf("PASS original fire57: %u native old/off/on selection packets across two engine images\n",fire_cases);
printf("PASS original skirmish56: %u native on/off selection packets across two engine images\n",skirmish_cases);
printf("PASS original commit223: %u complete release/dispatcher/constructor/vector-copy/serializer/destructor cases; allocator and registry are providers; no gameplay\n",cases);
}
