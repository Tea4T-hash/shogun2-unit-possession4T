// Original native compass lifecycle and native move construction/serialization
// in owned mapped images. Terrain admission/point materialization, allocation
// and final UI dispatch are providers; original terrain query/search executes. No game startup, live unit or controller is used.
#define wmain old_ui_formation218
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/formation218.h"
#include "terrain_fixture224.h"
static uintptr_t root218{},queue218{};static unsigned calls218{},action218{};static float amount218{};
static void __fastcall dispatch218(void* root,void*,unsigned action,float amount){assert(reinterpret_cast<uintptr_t>(root)==root218);++calls218;action218=action;amount218=amount;*reinterpret_cast<unsigned*>(queue218+4+0x5000)+=16;}
static void* __fastcall copy218(void* out,void*,const void* source){memcpy(out,source,12);return out;}
static void __fastcall free218(void*,void*){}
static unsigned __fastcall run218(const void*,void*){return 0;}
struct Point218{float x,y,z;uintptr_t surface;unsigned id;};
static void put218(uintptr_t p,uintptr_t v){memcpy(reinterpret_cast<void*>(p),&v,4);}
static void scalar218(uintptr_t p,float v){memcpy(reinterpret_cast<void*>(p),&v,4);}
int wmain(int argc,wchar_t** argv){assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned cases=0;
 for(int input=1;input<argc;++input){auto image=LoadLibraryExW(argv[input],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto b=reinterpret_cast<uintptr_t>(image);native214::admitted_image=0;assert(native214::admit(b));
 {std::vector<unsigned char> root(0xe00),env(0x28400),ui(0x328),widget(0x328),ctrl(0x60),owner(0x200),selected(0x500),queue(0x5010),common(0x100),manager(0x200),unit1(0xb00),unit2(0xb00),pose1(0x50),pose2(0x50);uintptr_t children[1]{},units[2]{};unsigned char registry[28]{};
 auto r=reinterpret_cast<uintptr_t>(root.data()),e=reinterpret_cast<uintptr_t>(env.data()),u=reinterpret_cast<uintptr_t>(ui.data()),w=reinterpret_cast<uintptr_t>(widget.data()),c=reinterpret_cast<uintptr_t>(ctrl.data()),o=reinterpret_cast<uintptr_t>(owner.data()),s=reinterpret_cast<uintptr_t>(selected.data()),q=reinterpret_cast<uintptr_t>(queue.data()),co=reinterpret_cast<uintptr_t>(common.data()),m=reinterpret_cast<uintptr_t>(manager.data()),a=reinterpret_cast<uintptr_t>(unit1.data()),z=reinterpret_cast<uintptr_t>(unit2.data()),p1=reinterpret_cast<uintptr_t>(pose1.data()),p2=reinterpret_cast<uintptr_t>(pose2.data());root218=r;queue218=q;children[0]=w;units[0]=a;units[1]=z;
 put218(r+0x110,e);put218(r+0x5b8,e);put218(r+0x5b4,o);put218(e+0x2826c,s);put218(s+0x4a8,o);put218(e+0x28048,q);put218(e+8,co);put218(co+0xb8,m);put218(m,e);
 put218(u,b+0x1704cdc);put218(u+0x9c,1);put218(u+0xa0,reinterpret_cast<uintptr_t>(children));put218(w,b+0x1704cdc);put218(w+0x94,u);put218(w+0x318,c);put218(c,b+0x1709cc8);put218(c+4,w);put218(c+0x40,165);ctrl[0x50]=1;
 put218(o+0xb0,b+0x15c4da0);put218(o+0xc0,2);put218(o+0xc4,2);put218(o+0xc8,reinterpret_cast<uintptr_t>(units));put218(a,b+0x15c4da0);put218(z,b+0x15c4da0);put218(a+4,11);put218(z+4,22);put218(a+0x440,p1);put218(z+0x440,p2);Point218 origin1{10,0,20,0,0},origin2{100,0,200,0,0};memcpy(pose1.data()+0x10,&origin1,20);memcpy(pose2.data()+0x10,&origin2,20);unsigned short head=16384;memcpy(pose2.data()+0x28,&head,2);
 unsigned key=0x12345678,n=1;put218(reinterpret_cast<uintptr_t>(registry),key);uintptr_t reg=reinterpret_cast<uintptr_t>(registry);float trig[2]{0,1},trig2[2]{1,0};
 Patch185 rootGlobal(b+0x1bc8180,&r,4),uiGlobal(b+0x1bbe364,&u,4),keyGlobal(b+0x192d014,&key,4),registryGlobal(b+0x18d9000,&reg,4),countGlobal(b+0x18d9004,&n,4),angle0(b+0x1bd9560,trig,8),angle90(b+0x1bd9560+16384*8,trig2,8);
 auto d=jump185(b+0x10dfb80,reinterpret_cast<void*>(&dispatch218)),v=jump185(b+0x10ac90,reinterpret_cast<void*>(&copy218)),f=jump185(b+0x1d10f0,reinterpret_cast<void*>(&free218)),run=jump185(b+0x4596f0,reinterpret_cast<void*>(&run218));
 Patch185 dispatch(b+0x10dfb80,d.data(),5),vectors(b+0x10ac90,v.data(),5),release(b+0x1d10f0,f.data(),5),running(b+0x4596f0,run.data(),5);
 fixture224::Terrain terrain(b,co);
 formation218::FormationPad pad;auto current=[]{return true;};auto used=[&](){return *reinterpret_cast<unsigned*>(q+4+0x5000);};auto reset=[&](){pad.cancel();put218(q+4+0x5000,0);calls218=0;fixture224::probes=fixture224::allow_after=0;fixture224::outside=false;fixture224::successful_probes=UINT_MAX;};
 auto begin_native=[&](unsigned button){reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned)>(b+0x10dc0c0)(c,button);};
 auto release_native=[&](){reinterpret_cast<void(__thiscall*)(uintptr_t)>(b+0x10f85f0)(c);};
 for(auto direction:std::vector<deck174::Vec2>{{0,1},{0,-1},{1,0},{-1,0}}){reset();begin_native(direction.x?(direction.x<0?6:7):(direction.y<0?3:2));
  assert(*reinterpret_cast<unsigned*>(c+0x40)!=165);++cases;
  auto mode=direction.x?(direction.x<0?17u:18u):14u;assert(*reinterpret_cast<unsigned*>(c+0x40)==mode);assert((*reinterpret_cast<float*>(c+0x48)<0)==(direction.x<0||direction.y<0));++cases;
  assert(calls218==0&&used()==0);++cases;
  // Original UI elapsed-time update, with its original acceleration; no custom
  // timer or repeated native movement dispatch is substituted.
  scalar218(e+0x2829c,1.f);reinterpret_cast<void(__thiscall*)(void*)>(b+(direction.x?0x11037c0:0x1102480))(reinterpret_cast<void*>(c));assert(std::fabs(*reinterpret_cast<float*>(c+0x48))>=1.f);++cases;
  root[0x598]=1;release_native();assert(calls218==1&&action218==mode&&used()==16);++cases;
 }
 // Match native release when the ghost was never drawn.
 reset();deck216::FineIntent in;begin_native(7);release_native();assert(calls218==0&&used()==0);++cases;
 // A native formation group is eligible for the same release-only cardinal path.
 reset();alignas(4) unsigned char group_only[0x80]{};auto group_address=reinterpret_cast<uintptr_t>(group_only);
 put218(group_address,b+0x15c4da0);put218(group_address+0x7c,31);
 put218(o+0xb4,1);put218(o+0xb8,1);put218(o+0xbc,reinterpret_cast<uintptr_t>(&group_address));put218(o+0xc4,0);
 put218(o+0xe8,1);put218(o+0xec,1);put218(o+0xf0,reinterpret_cast<uintptr_t>(&group_address));
 begin_native(2);assert(calls218==0);root[0x598]=1;
 release_native();assert(calls218==1&&action218==14);++cases;
 put218(o+0xb4,0);put218(o+0xb8,0);put218(o+0xbc,0);put218(o+0xc4,2);put218(o+0xec,0);
 // Exact-axis presses keep their original native arrow and preview distance.
 for(float distance:{2.f,12.f})for(auto direction:std::vector<deck174::Vec2>{{0,1},{0,-1},{1,0},{-1,0}}) {
  reset();
  const bool side=std::fabs(direction.x)>std::fabs(direction.y);
  const bool negative=(side?direction.x:direction.y)<0;
  begin_native(side?(negative?6:7):(negative?3:2));
  const unsigned expected=side?(negative?17:18):14;
  assert(*reinterpret_cast<unsigned*>(c+0x40)==expected&&!calls218&&used()==0);++cases;
  scalar218(c+0x48,negative?-distance:distance);root[0x598]=1;
  release_native();assert(calls218==1&&action218==expected&&amount218==(negative?-distance:distance));++cases;
 }
 for(auto offset:{0x20u,0x2cu}) {
  reset();put218(o+0xb0+offset,1);in={};in.began=in.active=true;in.move={0,1};
  assert(!pad.sample(b,r,e,in,current)&&used()==0&&calls218==0);
  put218(o+0xb0+offset,0);++cases;
 }

 }
 assert(FreeLibrary(image));
 }
 std::cout<<"PASS formation218: "<<cases<<" original compass press/release and selection cases; final UI dispatch is a provider; no gameplay\n";
}
