#include "../../../common/fixture_paths.h"
#include "adapter.cpp"
static unsigned forwarded_index97,forwarded_actor97,path_requests153,task_forwards153;
static uintptr_t portal_surface153;
static unsigned path_goal153[5];
static float portal_height155;static unsigned native_lookup155;
static std::array<unsigned char,1024> scene155{},cache155{};
static unsigned __fastcall native_cell155(void*,void*,const float*,unsigned){++native_lookup155;return 7;}
// World-height/navigation provider only: original portal397e40/80 and
// original359070 construct the location, retaining its real native layout.
static uintptr_t __fastcall portal_world155(void*,void*,void* out,const float* xz,uintptr_t surface,unsigned flags){
 require(flags==3,"Portal changed native query flags");float point[3]={xz[0],portal_height155,xz[1]};
 reinterpret_cast<void(__thiscall*)(void*,const float*,uintptr_t)>(base+0x359070)(out,point,surface);return uintptr_t(out);
}
static void path_permission200(uintptr_t actor,const unsigned* goal);
static void __fastcall path_plan153(void* object,void*){
 auto actor=reinterpret_cast<uintptr_t>(object);++path_requests153;memcpy(path_goal153,reinterpret_cast<void*>(actor+0x420),20);
 // Execute the ORIGINAL request builder and cached-route validation that
 // faulted in22472. Async solve/terrain remain explicit providers, not success.
 unsigned request[18]{};auto scene=uintptr_t(scene155.data());
 field<uintptr_t>(scene,0)=scene+0x100;field<uintptr_t>(scene+0x100,8)=scene+0x200;field<uintptr_t>(scene+0x200,0xbc)=scene+0x380;
 field<float>(scene,0x7c)=field<float>(scene,0x80)=-100000;field<float>(scene,0x84)=field<float>(scene,0x88)=100000;
 reinterpret_cast<void(__thiscall*)(void*,uintptr_t,unsigned,const void*,const void*,float,unsigned,unsigned,unsigned,unsigned)>(base+0x35fdf0)(request,scene,1,path_goal153,path_goal153,1.f,0,0,0,3);
 cache155[0x20]=cache155[0x21]=cache155[0x76]=1;field<unsigned>(uintptr_t(cache155.data()),0x70)=0xdeadbeef;
 unsigned cache_slot[4]={0,0,0,unsigned(uintptr_t(cache155.data()))};
 require(!reinterpret_cast<unsigned(__thiscall*)(void*,const void*,const void*)>(base+0x378c70)(reinterpret_cast<void*>(scene),cache_slot,request),"Fresh fixture path unexpectedly reused route");
 path_permission200(actor,path_goal153);
 field<unsigned>(actor,0x3bc)=4;
}
// Execute the original actor request builder against an isolated navigation
// provider. No unit-level wall order is created for the player or peers.
static unsigned request_cases200,down_entry_refusals201;static bool request_hooked200;
static void path_permission200(uintptr_t actor,const unsigned* goal){
 auto env=field<uintptr_t>(actor,0x28),common=field<uintptr_t>(env,8),owner=field<uintptr_t>(actor,0x214);
 auto scene=common;field<uintptr_t>(scene,0)=scene+0x100;field<uintptr_t>(scene+0x100,8)=scene+0x200;field<uintptr_t>(scene+0x200,0xbc)=scene+0x380;
 field<float>(scene,0x7c)=field<float>(scene,0x80)=-100000;field<float>(scene,0x84)=field<float>(scene,0x88)=100000;
 if(!request_hooked200)original_climb_path_request200=reinterpret_cast<ClimbPathRequest200>(base+0x3ccd00);
 for(unsigned unit_climb=0;unit_climb<2;++unit_climb){
  field<unsigned char>(owner,0xfac)=static_cast<unsigned char>(unit_climb);
  unsigned original[18]{},personal[18]{};
  original_climb_path_request200(reinterpret_cast<void*>(actor),original,goal,0,3,0);
  require(field<unsigned char>(uintptr_t(original),0x35)==unit_climb,"Original actor wall permission did not follow unit state");
  require(field<uintptr_t>(uintptr_t(original),0x40)==(unit_climb?owner+0xfa8:0),"Original actor wall list differs");
  if(request_hooked200)reinterpret_cast<ClimbPathRequest200>(base+0x3ccd00)(reinterpret_cast<void*>(actor),personal,goal,0,3,0);
  else climb_path_request200(reinterpret_cast<void*>(actor),nullptr,personal,goal,0,3,0);
  require(field<unsigned char>(uintptr_t(personal),0x35)==1&&!field<uintptr_t>(uintptr_t(personal),0x40),"Personal route still requires unit wall order");
  // Original371b60 has a SECOND unit-derived portal gate at request+34.
  // Native world enabled/surface checks remain in force after admission.
  unsigned char edge[0x60]{},start[0x40]{},end[0x40]{};auto ed=uintptr_t(edge),st=uintptr_t(start),en=uintptr_t(end);
  field<uintptr_t>(ed,0x10)=climb_entry153.pipe;field<uintptr_t>(ed,0x14)=st;field<uintptr_t>(ed,0x18)=en;field<unsigned>(ed,8)=1;
  field<uintptr_t>(en,0xc)=base+0x15e7fd0;field<unsigned char>(en,0x10)=1;
  using Edge200=bool(__cdecl*)(uintptr_t,unsigned,unsigned,unsigned,unsigned,unsigned,float,void*);
  auto edge_allowed=reinterpret_cast<Edge200>(base+0x371b60);
  require(!edge_allowed(ed,1,0,field<unsigned char>(uintptr_t(original),0x34),0,3,1.f,nullptr),"Unordered native unit unexpectedly permitted pipe traversal");
  require(edge_allowed(ed,1,0,field<unsigned char>(uintptr_t(personal),0x34),0,3,1.f,nullptr),"Personal route still fails original native portal admission");
  field<unsigned char>(climb_entry153.pipe,4)=0;
  require(!edge_allowed(ed,1,0,field<unsigned char>(uintptr_t(personal),0x34),0,3,1.f,nullptr),"Personal climb waived disabled native wall");field<unsigned char>(climb_entry153.pipe,4)=1;
  field<unsigned char>(en,0x10)=0;require(!edge_allowed(ed,1,0,1,0,3,1.f,nullptr),"Personal climb waived disabled target surface");field<unsigned char>(en,0x10)=1;
  field<uintptr_t>(en,0x20)=1;require(!edge_allowed(ed,1,0,1,0,3,1.f,nullptr),"Personal climb waived native region restriction");
  require(reinterpret_cast<bool(__thiscall*)(void*)>(base+0x372bd0)(reinterpret_cast<void*>(climb_entry153.pipe)),"Original native castle-kind classification differs");
  field<unsigned char>(uintptr_t(personal),0x34)=field<unsigned char>(uintptr_t(original),0x34);
  field<unsigned char>(uintptr_t(personal),0x35)=static_cast<unsigned char>(unit_climb);field<uintptr_t>(uintptr_t(personal),0x40)=unit_climb?owner+0xfa8:0;
  require(!memcmp(original,personal,sizeof original),"Personal climb altered geometry or other native path policy");
  auto saved=climb_entry153.generation;++climb_entry153.generation;
  if(request_hooked200)reinterpret_cast<ClimbPathRequest200>(base+0x3ccd00)(reinterpret_cast<void*>(actor),personal,goal,0,3,0);
  else climb_path_request200(reinterpret_cast<void*>(actor),nullptr,personal,goal,0,3,0);
  require(!memcmp(original,personal,sizeof original),"Stale possession granted personal climb permission");climb_entry153.generation=saved;
  ++request_cases200;
 }
 field<unsigned char>(owner,0xfac)=0;field<uintptr_t>(env,8)=common;
}
static uintptr_t __fastcall surface_portal153(void*,void*){return portal_surface153;}
static void __fastcall surface_transform153(void*,void*){} // Explicit identity surface transform provider.
static void __fastcall formation_task153(void*,void*){++task_forwards153;}

static unsigned __fastcall fake_index97(void*,void*){++forwarded_index97;return 999;}
static uintptr_t __fastcall fake_actor97(void*,void*,unsigned){++forwarded_actor97;return 0;}
static LONG WINAPI fault_siege155(EXCEPTION_POINTERS* p){std::cerr<<"siege155 fault "<<std::hex<<p->ExceptionRecord->ExceptionCode<<" at "<<uintptr_t(p->ExceptionRecord->ExceptionAddress)-base<<" address "<<p->ExceptionRecord->ExceptionInformation[1]<<"\n";return EXCEPTION_EXECUTE_HANDLER;}
static void request_lifetime199(){
 ClimbRequest199 r;r.set(3,1000);require(r.take(3,1001)&&!r.take(3,1001),"Climb request was lost or replayed");
 r.set(3,1000);require(!r.take(4,1001)&&!r.take(3,1001),"Climb request crossed possession lease");
 r.set(3,1000);require(!r.take(3,1251)&&!r.take(3,1252),"Rejected climb remained queued");
 r.set(3,1000);r.clear();require(!r.take(3,1001),"Released possession retained climb request");
 r.set(3,1000);require(!r.take(3,999),"Reversed clock accepted climb request");
}
int main(){request_lifetime199();SetUnhandledExceptionFilter(fault_siege155);unsigned char* mapped=nullptr;try{
 std::wstring path=fixture_paths::engine();require(hash_file(path)==EXPECTED_HASH,"Exact native fixture image");
 std::ifstream in(path,std::ios::binary);std::vector<unsigned char> image((std::istreambuf_iterator<char>(in)),{});auto dos=reinterpret_cast<IMAGE_DOS_HEADER*>(image.data());auto nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(image.data()+dos->e_lfanew);
 mapped=static_cast<unsigned char*>(VirtualAlloc(nullptr,nt->OptionalHeader.SizeOfImage,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));require(mapped!=nullptr,"Native fixture allocation");memcpy(mapped,image.data(),nt->OptionalHeader.SizeOfHeaders);
 auto sections=IMAGE_FIRST_SECTION(nt);for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i){auto&s=sections[i];if(s.SizeOfRawData){require(s.PointerToRawData+s.SizeOfRawData<=image.size(),"Native section bounds");memcpy(mapped+s.VirtualAddress,image.data()+s.PointerToRawData,s.SizeOfRawData);}}
 uint32_t delta=uint32_t(reinterpret_cast<uintptr_t>(mapped)-nt->OptionalHeader.ImageBase);auto reloc=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];unsigned offset=0;
 while(offset<reloc.Size){auto block=reinterpret_cast<IMAGE_BASE_RELOCATION*>(mapped+reloc.VirtualAddress+offset);require(block->SizeOfBlock>=8&&offset+block->SizeOfBlock<=reloc.Size,"Relocation bounds");auto entries=reinterpret_cast<uint16_t*>(block+1);
  for(unsigned i=0;i<(block->SizeOfBlock-8)/2;++i)if(entries[i]>>12==IMAGE_REL_BASED_HIGHLOW)*reinterpret_cast<uint32_t*>(mapped+block->VirtualAddress+(entries[i]&4095))+=delta;offset+=block->SizeOfBlock;}
 DWORD old=0;require(VirtualProtect(mapped,nt->OptionalHeader.SizeOfImage,PAGE_EXECUTE_READWRITE,&old)!=0,"Read-only native fixture image");FlushInstructionCache(GetCurrentProcess(),mapped,nt->OptionalHeader.SizeOfImage);

 base=reinterpret_cast<uintptr_t>(mapped);
 const unsigned char routing[]={0x31,0xc0,0xc3};memcpy(mapped+0x1821e0,routing,sizeof routing);
 climb97::Status publication;publication.pid=77;publication.lease=5;publication.actor=1203;publication.unit=11;publication.tick=1000;publication.mode=climb97::Climbing;
 require(climb97::valid(publication,77,5,1203,11,1000)&&!climb97::valid(publication,77,6,1203,11,1000)&&!climb97::valid(publication,77,5,1204,11,1000)&&!climb97::valid(publication,78,5,1203,11,1000)&&!climb97::valid(publication,77,5,1203,11,1501)&&!climb97::valid(publication,77,5,1203,11,999),"Numeric wall prompt stale/battle/lease identity guards");
 Status status{};state=&status;enabled=1;state->heartbeat=GetTickCount64();state->acquire=1;state->mode=ARMED;
 std::vector<unsigned char> root(0x120),env(0x28300),common(0x400),phase(0x400),manager(0x910),cam(0x200),view(0x90),holder(0x210),other_holder(0x210),selected(0x500),owner(0x120),config(0x100),actor(0x700),dead_actor(0x700),unitbuf(0x1000),pipebuf(0x100),source(0x100),definition(0x80);
 auto ptr=[](auto& v){return reinterpret_cast<uintptr_t>(v.data());};auto a=ptr(actor),u=ptr(unitbuf),v=ptr(view),pipe=ptr(pipebuf),def=ptr(definition),ph=ptr(phase),en=ptr(env);
 unsigned char nodes[]={0x0b,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xde,0xca,0x1f,0x43,0x89,0x13,0xfe,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xf1,0xd9,0x31,0x43,0x3f,0xa2,0xe7,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xde,0xca,0x1f,0x43,0xa8,0x18,0xfe,0xc2,0x5a,0x1c,0x58,0xc2,0xf1,0xd9,0x31,0x43,0x3f,0xa2,0xe7,0xc2,0x44,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xf1,0xd9,0x31,0x43,0x3f,0xa2,0xe7,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x50,0xca,0x35,0x43,0xc9,0xa8,0xe7,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xf1,0xd9,0x31,0x43,0x3f,0xa2,0xe7,0xc2,0x5a,0x1c,0x58,0xc2,0x50,0xca,0x35,0x43,0xc9,0xa8,0xe7,0xc2,0x44,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x50,0xca,0x35,0x43,0xc9,0xa8,0xe7,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x1b,0xcd,0x35,0x43,0xe7,0x53,0xe4,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x50,0xca,0x35,0x43,0xc9,0xa8,0xe7,0xc2,0x5a,0x1c,0x58,0xc2,0x1b,0xcd,0x35,0x43,0xe7,0x53,0xe4,0xc2,0x44,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00,0x0b,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x1b,0xcd,0x35,0x43,0xe7,0x53,0xe4,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0xcd,0xcc,0x34,0x43,0x00,0x00,0xe2,0xc2,0x00,0x00,0x00,0x00,0x5a,0x1c,0x58,0xc2,0x1b,0xcd,0x35,0x43,0xe7,0x53,0xe4,0xc2,0x5a,0x1c,0x58,0xc2,0xcd,0xcc,0x34,0x43,0x00,0x00,0xe2,0xc2,0x45,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00};std::array<uintptr_t,2> active_crest98{1067,ptr(config)};std::array<uintptr_t,2> holders{ptr(other_holder),ptr(holder)};std::array<uintptr_t,1> units{u},pipes{pipe};
 field<uintptr_t>(base,0x1bc8180)=ptr(root);field<uintptr_t>(ptr(root),0x110)=en;field<uintptr_t>(en,8)=ptr(common);field<uintptr_t>(ptr(common),0xb4)=ph;field<unsigned>(ph,0x14)=6;
 field<uintptr_t>(en,0x281fc)=ptr(manager);field<uintptr_t>(ptr(manager),0)=base+0x15d26fc;field<uintptr_t>(ptr(manager),0x8e0)=ptr(cam);field<uintptr_t>(ptr(manager),0x8f8)=v;field<uintptr_t>(ptr(manager),0x8fc)=a;
 field<uintptr_t>(ptr(cam),0)=base+0x15d22e8;field<uintptr_t>(ptr(cam),0x180)=ptr(manager);field<uintptr_t>(ptr(cam),0x184)=v;field<uintptr_t>(ptr(cam),0x188)=a;field<float>(ptr(cam),0x6c)=1;
 field<uintptr_t>(v,0x6c)=en;field<unsigned>(v,0x18)=2;field<uintptr_t>(v,0x1c)=reinterpret_cast<uintptr_t>(holders.data());field<uintptr_t>(ptr(holder),8)=a;field<uintptr_t>(ptr(other_holder),8)=ptr(dead_actor);
 field<uintptr_t>(en,0x2826c)=ptr(selected);field<uintptr_t>(ptr(selected),0x4a8)=ptr(owner);auto group=ptr(owner)+0xb0;field<uintptr_t>(group,0)=base+0x15c4da0;field<unsigned>(group,0x10)=field<unsigned>(group,0x14)=1;field<uintptr_t>(group,0x18)=reinterpret_cast<uintptr_t>(units.data());
 for(auto subject:{a,ptr(dead_actor)}){field<uintptr_t>(subject,0)=base+0x15dfec8;field<uintptr_t>(subject,0x214)=u;field<uintptr_t>(subject,0x20c)=ptr(config);field<uintptr_t>(subject,0x28)=en;}
 field<unsigned char>(a,0xf0)=field<unsigned char>(a,0xa1)=1;field<unsigned>(a,0x1c8)=1203;field<unsigned>(a,0x218)=17;field<unsigned>(u,4)=11;field<uintptr_t>(u,0xf9c)=ptr(config);
 field<unsigned>(ph,0x2fc)=field<unsigned>(ph,0x300)=1;field<uintptr_t>(ph,0x304)=reinterpret_cast<uintptr_t>(pipes.data());
 field<uintptr_t>(pipe,0)=base+0x15e7fd0;field<unsigned char>(pipe,4)=1;field<unsigned>(pipe,0x10)=0x854;field<uintptr_t>(pipe,0x14)=ptr(manager);field<uintptr_t>(pipe,0x30)=ptr(source);field<uintptr_t>(pipe,0x34)=def;field<unsigned>(pipe,0x40)=11;field<uintptr_t>(ptr(source),0x30)=pipe;field<unsigned>(ptr(source),0x40)=11;
 field<uintptr_t>(def,0)=base+0x15dd88c;field<float>(base+0x1a4d3a0,0x5c)=.6f;field<float>(base+0x1a4d2e0,0x5c)=2.25f;
 field<unsigned>(def,0x10)=field<unsigned>(def,0x14)=4;field<uintptr_t>(def,0x18)=reinterpret_cast<uintptr_t>(nodes);
 Wall97 wall;require(read_wall97(a,pipe,wall),"Recorded native wall path proof");
 original_pipe_advance97=reinterpret_cast<PipeAdvance97>(base+0x3945c0);original_pipe_mode97=reinterpret_cast<PipeMode97>(base+0x3cf280);original_view_index97=reinterpret_cast<ViewIndex97>(&fake_index97);original_view_actor97=reinterpret_cast<ViewActor97>(&fake_actor97);
 require(adopt_possession97(a)&&state->mode==POSSESSED&&!state->acquire&&possessed==a,"F9 must adopt immediately without motion callback");
 Choice97 choice{en,ph,ptr(manager),v,u,ptr(holder),a,1203,17};choice97=choice;
 require(view_index_hook97(reinterpret_cast<void*>(v),nullptr)==17,"F9 chooses eligible live actor after dead first member");
 require(view_actor_hook97(reinterpret_cast<void*>(v),nullptr,17)==a,"Scoped native logical-index lookup resolves current holder");
 choice97={};require(view_actor_hook97(reinterpret_cast<void*>(v),nullptr,18)==0&&forwarded_actor97==1,"Foreign index forwarded");
 field<unsigned>(a,0x1c8)=1204;require(!followed(a,false)&&!choice_valid97(v,choice),"ABA actor serial rejected");field<unsigned>(a,0x1c8)=1203;

 // Original portal getter, location constructor, parameter/order and the
 // recorded cached-path validation execute. Only terrain/async solve is supplied.
 const unsigned patched153[]={0x3df3f0,0x1656b0,0x3f6290};unsigned char saved153[3][8];
 auto patch153=[&](unsigned rv,void* fn){auto code=mapped+rv;code[0]=0xe9;unsigned rel=unsigned(reinterpret_cast<uintptr_t>(fn)-(base+rv)-5);memcpy(code+1,&rel,4);};
 for(unsigned i=0;i<3;++i)memcpy(saved153[i],mapped+patched153[i],8);
 patch153(0x3df3f0,reinterpret_cast<void*>(path_plan153));patch153(0x1656b0,reinterpret_cast<void*>(portal_world155));patch153(0x3f6290,reinterpret_cast<void*>(native_cell155));
 std::vector<unsigned char> bottom153(64),top153(64),surface153(128);portal_surface153=ptr(surface153);
 field<uintptr_t>(ptr(surface153),0)=base+0x15e7f00;field<uintptr_t>(base+0x15e7f00,0x10)=base+0x95740;field<uintptr_t>(ptr(surface153),0x44)=42;
 field<uintptr_t>(pipe,0x28)=ptr(bottom153);field<uintptr_t>(pipe,0x2c)=ptr(top153);
 std::array<unsigned char,sizeof nodes> native_nodes153{};memcpy(native_nodes153.data(),nodes,sizeof nodes);
 field<float>(ptr(config),0x1c)=1.3f;field<float>(ptr(config),0x20)=3;field<float>(ptr(config),0x6c)=2;
 field<float>(u,0x22c)=1;field<float>(base+0x18d1c80,0x5c)=1;field<float>(base+0x18d1ce0,0x5c)=1;
 // Explicit registry fixture here; native group creation/transfer is exercised
 // separately by verify_proximity147 using original1668e0 and member lifecycles.
 std::vector<unsigned char> definition200(0x100);field<uintptr_t>(u,0x2c)=ptr(definition200);
 std::vector<unsigned char> independent200(0x600);auto ip200=ptr(independent200);uintptr_t registry200[]={ip200},members200[]={a};
 field<uintptr_t>(ip200,0)=base+0x15bb21c;field<uintptr_t>(ip200,4)=en;field<uintptr_t>(ip200,0x510)=u;field<unsigned>(ip200,8)=2;field<unsigned>(ip200,0x18)=1;field<uintptr_t>(ip200,0x1c)=uintptr_t(members200);
 field<uintptr_t>(u,0x14)=en;field<unsigned>(u,0xb70)=1;field<uintptr_t>(u,0xb74)=uintptr_t(registry200);field<unsigned>(u,0x384)=1;field<uintptr_t>(u,0x388)=uintptr_t(members200);field<uintptr_t>(a,0x3b4)=ip200;
 proximity_records147[0]={en,ph,u,a,ptr(config),0,ip200,field<unsigned>(u,4),field<unsigned>(a,0x1c8),possession_lease.generation,0,false};
 field<float>(base,0x15b2448)=1.f/60;
 for(unsigned wall_kind200:{11u,12u,13u,14u})for(auto role153:{0x15dfec8u,0x15dbd68u,0x15dbfb4u})for(float rotate153:{0.f,1.570796327f,3.141592654f,-1.570796327f})for(bool descending153:{false,true})for(bool ground154:{false,true}){
  field<unsigned>(pipe,0x40)=field<unsigned>(ptr(source),0x40)=wall_kind200;
  portal_surface153=ground154?0:ptr(surface153);
  memcpy(nodes,native_nodes153.data(),sizeof nodes);
  for(unsigned i=0;i<4;++i)for(unsigned off:{0x24u,0x30u}){auto node=uintptr_t(nodes)+i*72;float x=field<float>(node,off),z=field<float>(node,off+8);field<float>(node,off)=x*std::cos(rotate153)+z*std::sin(rotate153);field<float>(node,off+8)=-x*std::sin(rotate153)+z*std::cos(rotate153);}
  auto near153=uintptr_t(nodes)+(descending153?3:0)*72;unsigned near_off153=descending153?0x30:0x24;
  for(unsigned side155=0;side155<2;++side155){auto pp155=side155?ptr(top153):ptr(bottom153);auto nn155=uintptr_t(nodes)+(side155?3:0)*72;unsigned oo155=side155?0x30:0x24;
   field<uintptr_t>(pp155,0)=base+(ground154?0x15de28c:0x15de2ac);field<uintptr_t>(pp155,4)=ptr(env);
   field<float>(pp155,8)=field<float>(nn155,oo155);field<float>(pp155,0x10)=field<float>(nn155,oo155+8);field<uintptr_t>(pp155,0x18)=portal_surface153;
  }
  portal_height155=field<float>(uintptr_t(nodes)+(descending153?0:3)*72,descending153?0x28:0x34);
  field<uintptr_t>(a,0)=base+role153;field<uintptr_t>(a,0x18)=0;field<unsigned>(a,0x1f0)=0;field<unsigned char>(a,0xa1)=1;
  for(unsigned k=0;k<3;++k)field<float>(a,0x48+k*4)=field<float>(near153,near_off153+k*4);
  field<float>(a,0x48)+=std::cos(rotate153);field<float>(a,0x50)-=std::sin(rotate153);
  field<unsigned short>(a,0x64)=20000;state->yaw=std::atan2(-std::cos(rotate153),std::sin(rotate153));state->input=FORWARD|RUN;state->heartbeat=GetTickCount64();
  float before153[3];memcpy(before153,reinterpret_cast<void*>(a+0x48),12);auto heading153=field<unsigned short>(a,0x64);auto requests153=path_requests153;
  climb_reset97();climb_adopt98(a);state->aiming=1;climb_request199.set(possession_lease.generation,GetTickCount64());
  require(!climb_prepare97(a)&&!climb_align152&&!keyboard_wall98&&climb_error154==6,"Blocked climb did not reject without locking movement");
  state->aiming=0;require(!climb_prepare97(a)&&!climb_align152&&path_requests153==requests153,"Rejected climb executed later without another press");
  climb_reset97();climb_adopt98(a);climb_lease97=0;
  if(descending153){
   require(!climb_prepare97(a)&&!climb_near97&&!climb_align152&&!keyboard_wall98,"Top of wall still offered climb-down");
   climb_request199.set(possession_lease.generation,GetTickCount64());
   require(!climb_prepare97(a)&&!climb_align152&&!keyboard_wall98&&path_requests153==requests153,"F at crest began a descent or locked controls");
   Wall97 down201;require(read_wall97(a,pipe,down201)&&!climb_begin153(a,down201,true)&&!climb_align152&&path_requests153==requests153,"Direct top-entry request was accepted");
   ++down_entry_refusals201;continue;
  }
  climb_request199.set(possession_lease.generation,GetTickCount64());require(climb_prepare97(a),"F failed to enter native approach");
  require(path_goal153[3]==portal_surface153&&path_goal153[4]==0,"Bearing pose was passed as navigation location");
  require(path_requests153==requests153+1&&climb_align152&&!field<uintptr_t>(a,0x18),"F bypassed the native route/setup or bound before native arrival");
  require(!memcmp(before153,reinterpret_cast<void*>(a+0x48),12)&&field<unsigned short>(a,0x64)==heading153,"F directly teleported or forced body bearing");
  auto far153=uintptr_t(nodes)+(descending153?0:3)*72;unsigned far_off153=descending153?0x24:0x30;
  for(unsigned k=0;k<3;++k){float value;memcpy(&value,path_goal153+k,4);require(std::abs(value-field<float>(far153,far_off153+k*4))<.001f,"Native route targets the wrong wall endpoint");}
  require(field<float>(a,0x438)==1.3f&&field<unsigned>(a,0x448)==2,"Stock path speed/default restrictions changed");
  for(unsigned frame153=0;frame153<100;++frame153){state->heartbeat=GetTickCount64();require(climb_prepare97(a),"Native approach lost without input");}
  state->aiming=1;state->fire_request=1;require(climb_prepare97(a)&&climb_align152&&!state->aiming&&!state->fire_request,"Ready/fire interrupted owned native approach");
  require(path_requests153==requests153+1&&keyboard_wall98,"Held approach rebuilt routes or accepted manual movement");
  unsigned task153[2]={0,unsigned(a)},other153[2]={0,unsigned(ptr(dead_actor))};auto forwarded153=task_forwards153;
  personal_task130(task153,reinterpret_cast<UnitTick130>(formation_task153));personal_task130(other153,reinterpret_cast<UnitTick130>(formation_task153));
  require(task_forwards153==forwarded153+1,"Wall approach failed to retain its goal or blocked a peer task");
  if(role153==0x15dfec8u&&rotate153==0.f&&!descending153&&!ground154){
   auto repeat153=[&](){state->heartbeat=GetTickCount64();climb_request199.set(possession_lease.generation,GetTickCount64());require(climb_prepare97(a)&&climb_align152,"Fresh F did not restart a failed native route");};
   // A paused pending route stays valid. A motionless admitted route is
   // cancelled through ORIGINAL3df380 after bounded simulation time.
   for(unsigned i=0;i<200;++i){state->heartbeat=GetTickCount64();require(climb_prepare97(a),"Paused route spent simulation budget");}
   for(unsigned i=0;i<125&&climb_align152;++i){++field<unsigned>(ptr(common),0x50);state->heartbeat=GetTickCount64();climb_prepare97(a);}
   require(!climb_align152&&!keyboard_wall98&&!field<unsigned>(a,0x3bc)&&climb_error154==10,"Stalled route retained native or keyboard movement lock");repeat153();
   climb_request199.set(possession_lease.generation,GetTickCount64());require(!climb_prepare97(a)&&!keyboard_wall98&&!field<unsigned>(a,0x3bc),"Second F did not cancel owned approach");repeat153();
   field<unsigned>(a,0x3bc)=7;require(!climb_prepare97(a)&&!climb_align152,"Native route failure retained manual-input lock");repeat153();
   field<unsigned>(a,0x420)^=1;auto external153=field<unsigned>(a,0x420);
   require(!climb_prepare97(a)&&!climb_align152&&field<unsigned>(a,0x420)==external153,"External native goal was overwritten instead of releasing ownership");repeat153();
   pipes[0]=0;require(!climb_prepare97(a)&&!climb_align152,"Removed wall retained pending approach");pipes[0]=pipe;repeat153();
  }
  if(role153==0x15dfec8u&&rotate153==0.f&&!descending153&&!ground154){
   unsigned char request_before200[16];memcpy(request_before200,mapped+0x3ccd00,16);
   require(MH_Initialize()==MH_OK,"Request hook initialize");
   require(MH_CreateHook(reinterpret_cast<void*>(base+0x3ccd00),reinterpret_cast<void*>(climb_path_request200),reinterpret_cast<void**>(&original_climb_path_request200))==MH_OK&&MH_EnableHook(reinterpret_cast<void*>(base+0x3ccd00))==MH_OK,"Request hook enable");
   request_hooked200=true;path_permission200(a,path_goal153);request_hooked200=false;
   require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK&&MH_Uninitialize()==MH_OK&&!memcmp(request_before200,mapped+0x3ccd00,16),"Request hook did not restore exact original bytes");
  }
  field<uintptr_t>(a,0x18)=pipe;field<float>(a,0x20)=descending153?1.f:0.f;field<unsigned>(a,0x1f0)=descending153?69:68;
  require(climb_prepare97(a)&&!climb_align152&&wall_subject97(a),"Real native attachment did not restore W/S traversal");
 }
 // Native path vectors may contain 1..3 segments (the old hard-coded4
 // was a single recorded wall, not a native minimum). Registry/type/clip and
 // finite geometry checks still apply for these interior/elevated entries.
 memcpy(nodes,native_nodes153.data(),sizeof nodes);
 for(unsigned segments154:{1u,2u,3u}){
  field<unsigned>(def,0x14)=segments154;Wall97 short154;require(read_wall97(a,pipe,short154)&&short154.count==segments154,"Short native wall resource omitted from discovery");
 }
 field<unsigned>(def,0x14)=4;
 memcpy(nodes,native_nodes153.data(),sizeof nodes);field<uintptr_t>(a,0)=base+0x15dfec8;
 for(unsigned i=0;i<3;++i)memcpy(mapped+patched153[i],saved153[i],8);
 field<uintptr_t>(pipe,0x28)=field<uintptr_t>(pipe,0x2c)=0;climb_reset97();
 field<uintptr_t>(a,0x18)=pipe;field<unsigned>(a,0x1f0)=68;require(followed(a,false),"Same lease survives native climb attachment");
 using Point=uintptr_t(__thiscall*)(void*,float*,float);using Project=float(__thiscall*)(void*,const float*);auto point=reinterpret_cast<Point>(base+0x39a2e0);auto project=reinterpret_cast<Project>(base+0x38cc00);float length=reinterpret_cast<float(__thiscall*)(void*)>(base+0x3931f0)(reinterpret_cast<void*>(def));require(length>20&&length<40,"Original native captured wall length");
 unsigned cases=0;
 for(float p:{0.f,.001f,.1f,.4f,.7f,.9f,.999f,1.f})for(float step:{.001f,.01f,.1f})for(unsigned input:{0u,1u,2u,3u}){
   state->heartbeat=GetTickCount64();field<float>(base,0x15b2448)=1.f/60;state->input=input;field<unsigned>(a,0x1f0)=68;
   float position[4]{};point(reinterpret_cast<void*>(def),position,p);for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=position[k];field<float>(a,0xd8+k*4)=0;}
   field<float>(a,0x20)=field<float>(a,0x24)=p;
   ++field<unsigned>(ptr(common),0x50);pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,step);
   float budget=(input==2?1.f:.6f)*field<float>(base,0x15b2448);float expected=std::clamp(p+(input==1?budget/length:input==2?-budget/length:0),0.f,1.f);
   require(std::abs(field<float>(a,0x20)-expected)<.00001f,"Native W/S/hold progress including top/bottom");
   float targetpoint[4]{};point(reinterpret_cast<void*>(def),targetpoint,expected);
   for(unsigned k=0;k<3;++k)require(std::abs(field<float>(a,0xd8+k*4)-(targetpoint[k]-position[k]))<.0002f,"Native path intent XYZ, actor root untouched");
   require(std::abs(project(reinterpret_cast<void*>(def),position)-p)<.00001f,"Exact native projection/evaluation round trip");++cases;
 }
 for(unsigned input:{1u,2u}){
   state->heartbeat=GetTickCount64();state->input=input;float position[4]{};point(reinterpret_cast<void*>(def),position,.2f);
   for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=position[k];field<float>(a,0xd8+k*4)=0;}field<float>(a,0xd8)=.1f;
   ++field<unsigned>(ptr(common),0x50);pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,0);
   require(std::abs(field<float>(a,0x20)-(.2f+(input==1?.6f/60/length:-1.f/60/length)))<.00001f,"Native rate replaces stale pending root stride in both directions");
   field<float>(base,0x15b2448)=0;for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=position[k];field<float>(a,0xd8+k*4)=0;}field<float>(a,0xd8)=.1f;
   ++field<unsigned>(ptr(common),0x50);pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,.1f);require(std::abs(field<float>(a,0x20)-.2f)<.00001f,"Actual pause freezes owned traversal");field<float>(base,0x15b2448)=1.f/60;
 }

 unsigned rate_cases98=0;for(bool fast:{false,true})for(float normal:{.3f,.6f,.9f})for(float segment:{.5f,1.f,1.5f})for(float dt:{.01f,.05f,.1f,.2f})for(unsigned direction98:{1u,2u}){
   field<unsigned char>(u,0x294)=fast?1:0;field<float>(base+(fast?0x1a4d2e0:0x1a4d3a0),0x5c)=normal;for(unsigned n=0;n<4;++n)field<float>(reinterpret_cast<uintptr_t>(nodes)+n*72,0x40)=segment;
   state->input=direction98;state->heartbeat=GetTickCount64();field<float>(base,0x15b2448)=dt;float position[4]{};point(reinterpret_cast<void*>(def),position,.2f);
   for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=position[k];field<float>(a,0xd8+k*4)=3;}field<float>(a,0x20)=.2f;
   ++field<unsigned>(ptr(common),0x50);pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,100);
   require(std::abs(field<float>(a,0x20)-(.2f+(direction98==1?normal:-1.f)*segment*dt/length))<.00001f,"Live native parameter/unit capability/segment/tick rate rather than ground pending or argument");
   float progress98=field<float>(a,0x20),intent98[3];for(unsigned k=0;k<3;++k)intent98[k]=field<float>(a,0xd8+k*4);auto steps98=climb_steps97;
   for(unsigned repeat=0;repeat<10;++repeat)pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,100);
   require(field<float>(a,0x20)==progress98&&climb_steps97==steps98,"Duplicate callbacks within one simulation tick do not consume another climb budget");for(unsigned k=0;k<3;++k)require(intent98[k]==field<float>(a,0xd8+k*4),"Duplicate callbacks preserve one pending native step");++rate_cases98;
 }
 field<unsigned char>(u,0x294)=0;field<float>(base+0x1a4d3a0,0x5c)=.6f;field<float>(base+0x1a4d2e0,0x5c)=2.25f;field<float>(base,0x15b2448)=1.f/60;
 for(unsigned n=0;n<4;++n)field<float>(reinterpret_cast<uintptr_t>(nodes)+n*72,0x40)=1;
 field<unsigned>(a,0x1f0)=8;climb_adopt98(a);require(keyboard_wall98&&climb_prepare97(a),"Current registered attachment wins during old ground-pose transition; no F prompt");field<unsigned>(a,0x1f0)=68;
 state->release=1;float peer_position[4]{};point(reinterpret_cast<void*>(def),peer_position,.2f);for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=peer_position[k];field<float>(a,0xd8+k*4)=0;}
 auto previous_steps=climb_steps97;pipe_advance_hook97(reinterpret_cast<void*>(a+0x14),nullptr,pipe,.1f);require(climb_steps97==previous_steps&&std::abs(field<float>(a,0x20)-(.2f+.1f/length))<.00001f,"Released ownership forwards native AI progress exactly");state->release=0;
 // Native crest selector and native downward state, without replacing its RNG/playback.
 for(unsigned kind=11;kind<=14;++kind){field<unsigned>(pipe,0x40)=field<unsigned>(ptr(source),0x40)=kind;field<float>(a,0x20)=.8f;state->input=1;
   require(pipe_mode_hook97(reinterpret_cast<void*>(a),nullptr,pipe)==72+kind-11,"Original native crest selection retained");state->input=2;require(pipe_mode_hook97(reinterpret_cast<void*>(a),nullptr,pipe)==69,"Reverse traversal avoids ascending crest event");}
 field<unsigned char>(u,0x294)=1;state->input=2;require(pipe_mode_hook97(reinterpret_cast<void*>(a),nullptr,pipe)==71,"Native fast descending capability retained at crest segment");field<unsigned char>(u,0x294)=0;
 field<unsigned>(pipe,0x40)=field<unsigned>(ptr(source),0x40)=11;field<uintptr_t>(a,0x18)=0;field<unsigned>(a,0x1f0)=72;field<unsigned>(a,0x1d4)=1067;field<uintptr_t>(a,0x474)=reinterpret_cast<uintptr_t>(active_crest98.data());field<unsigned char>(a,0xa1)=0;field<unsigned char>(a,0xa7)=1;
 require(followed(a,false),"Native crest root-motion lease retained");field<unsigned>(a,0x1d4)=55;active_crest98[0]=55;require(!followed(a,false),"Unrelated root-only state rejected");field<unsigned>(a,0x1d4)=1067;active_crest98[0]=1067;field<unsigned char>(a,0xa7)=0;require(followed(a,false),"Native detach before a7 publication retains exact active crest");field<unsigned>(a,0x1f0)=8;require(followed(a,false),"Active crest survives native requested ground mode");field<unsigned>(a,0x1f0)=72;
 field<unsigned char>(a,0xf0)=0;require(!wall_subject97(a),"Dead wall actor rejected");field<unsigned char>(a,0xf0)=1;
 field<uintptr_t>(a,0x474)=0;field<uintptr_t>(a,0x18)=pipe;field<unsigned>(a,0x1f0)=68;pipes[0]=0;require(!wall_subject97(a)&&!followed(a,false),"Removed resource rejected, cached index never grants ownership");pipes[0]=pipe;
 field<uintptr_t>(ptr(source),0x30)=0;require(!wall_subject97(a),"Resource identity reused rejected");field<uintptr_t>(ptr(source),0x30)=pipe;
 field<uintptr_t>(pipe,0x30)=0;require(wall_subject97(a)&&followed(a,false),"Actual live null construction source accepted through current registry");
 field<unsigned>(ph,0x14)=9;require(!followed(a,false),"Battle phase change rejected");field<unsigned>(ph,0x14)=6;
 climb_last_direction97=1;
 for(unsigned i=0;i<10000;++i){state->input=0;field<float>(a,0x20)=.2f;require(pipe_mode_hook97(reinterpret_cast<void*>(a),nullptr,pipe)==68,"Stable native up/hold mode and ABI");}
 // Actual MinHook/trampoline ABI and exact restoration, in this fixture only.
 const unsigned hook_rvas[]={0x2d6210,0x340d30,0x3945c0,0x3cf280,0x2fcb10,0x341560,0x325ca0,0x321d70,0x3ccd00};unsigned char before[9][16];for(unsigned i=0;i<9;++i)memcpy(before[i],mapped+hook_rvas[i],16);
 require(MH_Initialize()==MH_OK,"Fixture hook initialize");create_siege_hooks97();require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Fixture hook enable");
 state->heartbeat=GetTickCount64();field<float>(a,0x20)=.2f;state->input=1;
 require(reinterpret_cast<ViewIndex97>(base+0x2d6210)(reinterpret_cast<void*>(v))==0,"Ordinary native first-holder index was changed by possession");
 ++camera_scope104;
 require(reinterpret_cast<ViewIndex97>(base+0x2d6210)(reinterpret_cast<void*>(v))==17,"Actual native view trampoline scoped index");
 require(reinterpret_cast<ViewActor97>(base+0x340d30)(reinterpret_cast<void*>(v),17)==a,"Actual native view trampoline actor ABI");
 --camera_scope104;
 for(unsigned input:{0u,1u,2u,3u})for(unsigned i=0;i<1000;++i){
   state->input=input;float position[4]{};point(reinterpret_cast<void*>(def),position,.2f);for(unsigned k=0;k<3;++k){field<float>(a,0x48+k*4)=position[k];field<float>(a,0xd8+k*4)=0;}
   ++field<unsigned>(ptr(common),0x50);reinterpret_cast<PipeAdvance97>(base+0x3945c0)(reinterpret_cast<void*>(a+0x14),pipe,.01f);
   require(std::abs(field<float>(a,0x20)-(.2f+(input==1?.6f/60/length:input==2?-1.f/60/length:0)))<.00001f,"Actual traversal trampoline RET8");
 }
 require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK,"Fixture hook disable");for(unsigned i=0;i<9;++i)require(!memcmp(before[i],mapped+hook_rvas[i],16),"All nine roster/traversal/request hook bytes restored");require(MH_Uninitialize()==MH_OK,"Fixture hook teardown");
 if(climb_shared97)UnmapViewOfFile(climb_shared97);if(climb_mapping97)CloseHandle(climb_mapping97);climb_shared97=nullptr;climb_mapping97=nullptr;
 require(down_entry_refusals201==96,"Missing top-entry refusal coverage");std::cout<<"PASS wall201: "<<down_entry_refusals201<<" top-entry offers/requests refused without input lock or route mutation\n";VirtualFree(mapped,0,MEM_RELEASE);mapped=nullptr;std::cout<<"PASS wall200: "<<request_cases200<<" original per-actor request cases, all four castle wall kinds, scoped native trampoline, unit order independent permission, stalled/manual cancellation; siege98 production immediate F9 adoption, eligible logical-index choice, strict wall/crest lease, original native geometry/stride/projection, "<<cases<<" W/S/hold endpoint cases, four native crests, 10000 mode calls, 4000 actual hooked traversal ABI calls, "<<rate_cases98<<" native parameter/segment/dt cases with "<<(rate_cases98*10)<<" duplicate callbacks, nine exact hook restorations, stale/resource/phase/death guards. Synthetic world; no game initialization.\n";return 0;
}catch(const std::exception& e){if(mapped)VirtualFree(mapped,0,MEM_RELEASE);std::cerr<<e.what()<<'\n';return 1;}}
