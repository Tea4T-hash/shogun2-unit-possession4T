// Isolated native engine fixture. No game process, external input or entry point.
static unsigned neighbourhood207_count;
static uintptr_t neighbourhood207_target,neighbourhood207_next;
static __declspec(naked) void record_neighbour207(){__asm{
 inc dword ptr [neighbourhood207_count]
 mov dword ptr [neighbourhood207_target],edi
 jmp dword ptr [neighbourhood207_next]
}}
static __declspec(naked) void end_neighbours207(){__asm{
 pop edi
 pop esi
 pop ebx
 add esp,0x38
 ret
}}
static unsigned __fastcall fixture_id208(void* context,void*){return reinterpret_cast<uintptr_t>(context)==unit+0x10?0:1;}
static unsigned __fastcall fixture_width207(void*,void*,float,unsigned count){return count;}
static void verify_acquisition207(NativeFixture100& f,uintptr_t formation,uintptr_t independent,uintptr_t actor){
 using P=NativeFixture100;
 std::vector<unsigned char> observer_bytes(0x700),enemy_bytes(0x1800),group_bytes(0x2800),ours(0x80),theirs(0x80),side_owner(0x120),record(0x100),projectile(0x100),projectile_type(0x40),weather(0x80),phase_object(0x40),phase_links(0x40),phase_type(0x40),context_bytes(0x40),candidates(0x2804),cone_bytes(0x80);
 auto observer=P::ptr(observer_bytes),enemy=P::ptr(enemy_bytes),group=P::ptr(group_bytes),side=P::ptr(theirs),context=P::ptr(context_bytes);
 auto saved_unit=f.actor_unit;auto saved_actor_side=field<uintptr_t>(actor,0x44);
 unsigned char saved_formation[0xd0],saved_independent[0xd0];memcpy(saved_formation,reinterpret_cast<void*>(formation+0x628),sizeof saved_formation);memcpy(saved_independent,reinterpret_cast<void*>(independent+0x628),sizeof saved_independent);
 auto old_weather=field<uintptr_t>(P::ptr(f.common),0x5c),old_phase=field<uintptr_t>(P::ptr(f.phase),0x1c4);
 uintptr_t members[]={observer},registered[]={group},visible[]={unit};
 field<uintptr_t>(side,0)=environment;field<uintptr_t>(actor,0x44)=P::ptr(ours);field<unsigned>(P::ptr(ours),0x10)=0;field<unsigned>(side,0x10)=1;field<unsigned>(side,0x40)=1;field<uintptr_t>(side,0x44)=reinterpret_cast<uintptr_t>(visible);
 field<uintptr_t>(observer,0x44)=side;field<uintptr_t>(observer,0x28)=environment;field<uintptr_t>(observer,0x214)=enemy;field<unsigned char>(observer,0xf0)=1;
 field<uintptr_t>(enemy,0x14)=environment;field<uintptr_t>(enemy,0xb9c)=group;field<unsigned>(enemy,0xb8c)=1;field<uintptr_t>(enemy,0xb90)=reinterpret_cast<uintptr_t>(registered);
 field<uintptr_t>(enemy,0x2a4)=P::ptr(side_owner);field<uintptr_t>(P::ptr(side_owner),0xfc)=side;
 field<uintptr_t>(group,0)=base+0x15bb21c;field<uintptr_t>(group,4)=environment;field<uintptr_t>(group,0x510)=enemy;field<unsigned>(group,0x18)=1;field<uintptr_t>(group,0x1c)=reinterpret_cast<uintptr_t>(members);
 field<uintptr_t>(context,4)=enemy;
 uintptr_t width_vt[]={0,0,0,0,reinterpret_cast<uintptr_t>(&fixture_width207)},width_object[]={reinterpret_cast<uintptr_t>(width_vt)};
 field<uintptr_t>(group,0x624)=reinterpret_cast<uintptr_t>(width_object);
 field<uintptr_t>(enemy,0x2c)=P::ptr(record);field<uintptr_t>(enemy,0xf1c)=P::ptr(projectile);field<uintptr_t>(P::ptr(projectile),0x24)=P::ptr(projectile_type);field<unsigned>(P::ptr(projectile),0x60)=100;field<float>(enemy,0xf78)=100;field<unsigned char>(enemy,0x150)=1;
 uintptr_t ability[]={0,reinterpret_cast<uintptr_t>(&ability_probe147)};field<uintptr_t>(enemy,0xed4)=reinterpret_cast<uintptr_t>(ability);
 field<uintptr_t>(P::ptr(f.common),0x5c)=P::ptr(weather);field<uintptr_t>(P::ptr(f.phase),0x1c4)=P::ptr(phase_object);field<uintptr_t>(P::ptr(phase_object),0xc)=P::ptr(phase_links);field<uintptr_t>(P::ptr(phase_links),0x10)=P::ptr(phase_type);
 field<uintptr_t>(unit,0x14)=environment;field<unsigned>(unit,0xb60)=1;field<unsigned>(unit,0x384)=2;field<unsigned char>(unit,0xeb5)=0;field<uintptr_t>(unit,0x2c)=P::ptr(record);
 auto geometry=[&](uintptr_t g,float x,float z,float radius=1.f){
  memset(reinterpret_cast<void*>(g+0x628),0,0xd0);float rect[]={x,z,0,2,2,1,0,0,1};memcpy(reinterpret_cast<void*>(g+0x644),rect,sizeof rect);field<float>(g,0x670)=radius;field<float>(g,0x6dc)=x;field<float>(g,0x6e4)=z;
 };
 geometry(group,0,0);geometry(formation,215,0);geometry(independent,40,0);
 state->heartbeat=GetTickCount64();acquisition207={enemy,0,0,1};
 require(independent207(unit,formation)==independent,"207 exact registered independent fixture refused");
 using Radial=bool(__stdcall*)(uintptr_t,uintptr_t,float,unsigned);auto radial=reinterpret_cast<Radial>(base+0x459b40);
 using Score=uintptr_t(__cdecl*)(uintptr_t,void*,void*,float,float);auto score=reinterpret_cast<Score>(base+0x1752d0);
 using Cone=void(__stdcall*)(uintptr_t,void*,void*,unsigned);auto query=reinterpret_cast<Cone>(base+0x48d6a0);
 auto buffer=P::ptr(candidates);uintptr_t current[4]{};
 // Run actual native range/scoring code, not substituted yes/no gates.
 require(MH_DisableHook(reinterpret_cast<void*>(base+0x459b4d))==MH_OK,"Disable207 radial");
 require(!radial(group+0x628,unit,100,1),"207 old radial omission not reproduced");
 require(MH_EnableHook(reinterpret_cast<void*>(base+0x459b4d))==MH_OK,"Enable207 radial");
 unsigned calls=0;
 for(unsigned debug:{0u,1u})for(unsigned include:{0u,1u})for(float range:{10.f,30.f,50.f,100.f,250.f}){
  field<unsigned char>(base,0x1a65a90+0x5c)=static_cast<unsigned char>(debug);acquisition207={enemy,0,0,1};
  unsigned before{},after{};__asm mov before,esp
  bool accepted=radial(group+0x628,unit,range,include);
  __asm mov after,esp
  require(before==after&&accepted==(range>=50)&&!acquisition207.probe,"207 native radial result/ABI/probe differs");++calls;
 }
 field<unsigned char>(base,0x1a65a90+0x5c)=0;
 acquisition207={enemy,0,0,1};require(radial(group+0x628,unit,100,1),"207 owned radial target missing");
 field<uintptr_t>(buffer,0)=unit;field<unsigned>(buffer,0x2800)=1;
 require(MH_DisableHook(reinterpret_cast<void*>(base+0x175360))==MH_OK,"Disable207 score");require(score(enemy,candidates.data(),current,100,0)==0,"207 old score omission not reproduced");
 require(MH_EnableHook(reinterpret_cast<void*>(base+0x175360))==MH_OK,"Enable207 score");require(score(enemy,candidates.data(),current,100,0)==unit,"207 actual native scoring omitted independent target");
 field<unsigned char>(unit,0x3a2)=1;require(score(enemy,candidates.data(),current,100,0)==0,"207 native target eligibility bypassed");field<unsigned char>(unit,0x3a2)=0;
 field<float>(enemy,0xf78)=0;require(score(enemy,candidates.data(),current,100,0)==0,"207 native ammo/range gate bypassed");field<float>(enemy,0xf78)=100;
 require(score(enemy,candidates.data(),current,10,0)==0,"207 nominal range widened");
 // Native cone construction and exact body; points face +X then -X.
 for(unsigned i=0;i<65536;++i){double a=double(i)*6.283185307179586/65536;field<float>(base,0x1bd9560+i*8)=float(std::sin(a));field<float>(base,0x1bd9564+i*8)=float(std::cos(a));}
 using MakeCone=void(__thiscall*)(void*,const float*,const float*,float,float,unsigned short);
 float origin[]={0,0};
 for(float direction:{1.f,-1.f}){
  float forward[]={direction,0};reinterpret_cast<MakeCone>(base+0x417840)(cone_bytes.data(),origin,forward,2.f,100.f,16384);
  acquisition207={enemy,0,0,1};field<unsigned>(buffer,0x2800)=0;query(side,cone_bytes.data(),candidates.data(),0);
  require(field<unsigned>(buffer,0x2800)==unsigned(direction>0),"207 native cone widened/lost candidate");
  if(direction>0){require(field<uintptr_t>(buffer,0)==unit&&acquisition207.chosen==0,"207 cone target identity differs");require(score(enemy,candidates.data(),current,100,0)==unit,"207 cone to native score handoff failed");query(side,cone_bytes.data(),candidates.data(),1);require(field<unsigned>(buffer,0x2800)==1,"207 native append-unique duplicated target");}
 }

 #include "verify_acquisition208.h"
 // Complete original ranged search entry: its FAW gate, cone, native scorer
 // and unit result all execute. Only native world data is supplied by fixture.
 std::vector<unsigned char> search_context(0x40),enemy_config(0x100),contact_object(0x80);
 auto sc=P::ptr(search_context);field<uintptr_t>(sc,0xc)=environment;field<uintptr_t>(sc,0x10)=enemy;
 field<unsigned char>(enemy,0xeb0)=1;field<uintptr_t>(enemy,0xf9c)=P::ptr(enemy_config);field<float>(P::ptr(enemy_config),0x60)=90;
 field<unsigned short>(group,0x64c)=16384;uintptr_t result[3]{};acquisition207={};
 for(auto rv:{0x48d6a0u,0x48d860u,0x1752d0u})require(MH_DisableHook(reinterpret_cast<void*>(base+rv))==MH_OK,"Disable208 shared scope");
 original_search207(search_context.data(),result);require(result[0]==0,"207 original full ranged omission not reproduced");
 for(auto rv:{0x48d6a0u,0x48d860u,0x1752d0u})require(MH_EnableHook(reinterpret_cast<void*>(base+rv))==MH_OK,"Enable208 shared scope");
 auto search=reinterpret_cast<AcquisitionSearch207>(base+0x16aa00);auto depth=motion_callback_depth;
 require(search(search_context.data(),result)==reinterpret_cast<uintptr_t>(result)&&result[0]==unit&&!result[1]&&!result[2]&&!acquisition207.kind&&motion_callback_depth==depth,"207 complete native ranged search/scope/ABI failed");
 field<unsigned char>(enemy,0xeb0)=0;search(search_context.data(),result);require(!result[0]&&!acquisition207.kind,"207 bypassed native fire-at-will gate");field<unsigned char>(enemy,0xeb0)=1;
 // Actual native contact gate must inspect the same selected group.
 field<unsigned>(P::ptr(record),0x1c)=1;field<uintptr_t>(enemy,0x2ac)=P::ptr(contact_object);field<uintptr_t>(unit,0x2ac)=P::ptr(contact_object);field<unsigned>(P::ptr(contact_object),0x10)=1;
 acquisition207={enemy,independent,0,1};field<uintptr_t>(buffer,0)=unit;field<unsigned>(buffer,0x2800)=1;
 require(score(enemy,candidates.data(),current,100,0)==unit,"207 contact-capable observer failed native score");
 auto saved_contacts=field<unsigned>(independent,0x75c),saved_contact_list=field<uintptr_t>(independent,0x760);uintptr_t contacts[]={enemy,0,0};
 field<unsigned>(independent,0x75c)=1;field<uintptr_t>(independent,0x760)=reinterpret_cast<uintptr_t>(contacts);
 require(score(enemy,candidates.data(),current,100,0)==0,"207 native friendly-contact exclusion bypassed");field<unsigned>(independent,0x75c)=saved_contacts;field<uintptr_t>(independent,0x760)=saved_contact_list;
 field<unsigned>(P::ptr(record),0x1c)=0;field<uintptr_t>(enemy,0x2ac)=0;acquisition207={};

 // Ownership, alliance-list, lifetime and ordinary target behavior.
 for(unsigned which=0;which<7;++which){
  acquisition207={enemy,0,0,1};
  if(which==0)state->release=1;if(which==1)field<unsigned>(side,0x10)=0;if(which==2)field<unsigned>(side,0x40)=0;
  if(which==3)++field<unsigned>(possessed,0x1c8);if(which==4)field<unsigned char>(actor,0xf0)=0;if(which==5)acquisition207.observer=unit;if(which==6)acquisition207.kind=0;
  require(!radial(group+0x628,unit,100,1),"207 ownership/visibility/lifetime bypassed");
  state->release=0;field<unsigned>(side,0x10)=1;field<unsigned>(side,0x40)=1;if(which==3)--field<unsigned>(possessed,0x1c8);field<unsigned char>(actor,0xf0)=1;
 }
 geometry(formation,10,0);acquisition207={enemy,0,0,1};require(radial(group+0x628,unit,100,1)&&!acquisition207.chosen,"207 nearer formation displaced");
 geometry(formation,215,0,200);geometry(independent,120,0);acquisition207={enemy,0,0,1};require(radial(group+0x628,unit,100,1)&&!acquisition207.chosen,"207 rejected independent lost valid large formation");
 geometry(formation,215,0);geometry(independent,40,0);
 // Execute the ORIGINAL melee function through the candidate distance branch.
 // Fixture stops before target-record allocation/AI orders; these are not simulated.
 unsigned char entry[5],finish[5];memcpy(entry,reinterpret_cast<void*>(base+0x1b888e),5);memcpy(finish,reinterpret_cast<void*>(base+0x1b89bb),5);DWORD previous{};
 require(VirtualProtect(reinterpret_cast<void*>(base+0x1b8000),0x1000,PAGE_EXECUTE_READWRITE,&previous)!=0,"207 fixture protection");
 neighbourhood207_next=base+0x1b89a3;f.jump(0x1b888e,reinterpret_cast<void*>(&record_neighbour207));f.jump(0x1b89bb,reinterpret_cast<void*>(&end_neighbours207));FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(base+0x1b8000),0x1000);
 auto neighbours=reinterpret_cast<AcquisitionNeighbours207>(base+0x1b8740);
 require(MH_DisableHook(reinterpret_cast<void*>(base+0x1b8830))==MH_OK,"Disable207 neighbourhood");neighbourhood207_count=0;acquisition207={};neighbours(context_bytes.data());require(!neighbourhood207_count&&!acquisition207.kind,"207 old melee omission not reproduced");
 require(MH_EnableHook(reinterpret_cast<void*>(base+0x1b8830))==MH_OK,"Enable207 neighbourhood");neighbourhood207_count=0;neighbours(context_bytes.data());require(neighbourhood207_count==1&&neighbourhood207_target==unit&&!acquisition207.kind,"207 native melee candidate/scope restoration failed");
 geometry(independent,120,0);neighbourhood207_count=0;neighbours(context_bytes.data());require(!neighbourhood207_count,"207 native melee distance widened");
 geometry(formation,20,0);neighbourhood207_count=0;neighbours(context_bytes.data());require(neighbourhood207_count==1,"207 ordinary melee formation lost");
 memcpy(reinterpret_cast<void*>(base+0x1b888e),entry,5);memcpy(reinterpret_cast<void*>(base+0x1b89bb),finish,5);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(base+0x1b8000),0x1000);DWORD ignored{};VirtualProtect(reinterpret_cast<void*>(base+0x1b8000),0x1000,previous,&ignored);
 memcpy(f.actor_unit.data(),saved_unit.data(),saved_unit.size());field<uintptr_t>(actor,0x44)=saved_actor_side;
 memcpy(reinterpret_cast<void*>(formation+0x628),saved_formation,sizeof saved_formation);memcpy(reinterpret_cast<void*>(independent+0x628),saved_independent,sizeof saved_independent);
 field<uintptr_t>(P::ptr(f.common),0x5c)=old_weather;field<uintptr_t>(P::ptr(f.phase),0x1c4)=old_phase;acquisition207={};
 require(field<uintptr_t>(unit,0xb9c)==formation&&field<uintptr_t>(actor,0x214)==unit&&field<uintptr_t>(actor,0x3b4)==independent,"207 native ownership changed");
 std::cout<<"PASS acquisition207: "<<calls<<" original radial branches; complete original ranged search, contact/scoring and melee discovery; ordinary formations, range, ammo, native eligibility, visible-enemy list and lease guards preserved. Mounted="<<(actor!=possessed)<<"; downstream strategic orders/LOS/shots remain live checks.\n";
}
