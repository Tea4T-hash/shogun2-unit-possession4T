// Included inside the existing native fixture, after cone geometry is ready.
// Reproduce177e30's direct shared-query -> scoring calls with NO16aa00 scope.
{
 float forward[]={1,0};reinterpret_cast<MakeCone>(base+0x417840)(cone_bytes.data(),origin,forward,2.f,100.f,16384);
 acquisition207={};field<unsigned>(buffer,0x2800)=0;
 original_cone208(side,cone_bytes.data(),candidates.data(),0);
 require(!field<unsigned>(buffer,0x2800),"208 direct cone omission not reproduced");
 field<uintptr_t>(buffer,0)=unit;field<unsigned>(buffer,0x2800)=1;
 require(original_score208(enemy,candidates.data(),current,100,0)==0,"208 direct scoring omission not reproduced");
 auto shared_radial=reinterpret_cast<AcquisitionRadial208>(base+0x48d860);
 unsigned checks=0;
 for(unsigned nested:{0u,1u})for(unsigned mode:{0u,1u}){
  Acquisition207 sentinel={enemy,independent,independent,nested?2u:0u,side};acquisition207=sentinel;
  auto depth208=motion_callback_depth;unsigned before208{},after208{};
  field<unsigned>(buffer,0x2800)=0;
  __asm mov before208,esp
  if(mode)require(shared_radial(candidates.data(),enemy,100,1)==buffer,"208 shared radial return differs");
  else query(side,cone_bytes.data(),candidates.data(),0);
  auto selected208=score(enemy,candidates.data(),current,100,0);
  __asm mov after208,esp
  require(before208==after208&&selected208==unit&&field<unsigned>(buffer,0x2800)==1,"208 direct candidate/score ABI failure");
  require(!memcmp(&sentinel,&acquisition207,sizeof sentinel)&&motion_callback_depth==depth208,"208 nested scope leaked");
  ++checks;
 }
 // The shared cone entry has only a side parameter: explicitly test its own
 // environment/hostility/lease guards and exact ordinary-formation fallback.
 for(unsigned which=0;which<8;++which){
  acquisition207={};field<unsigned>(buffer,0x2800)=0;
  if(which==0)state->release=1;if(which==1)field<unsigned>(side,0x10)=0;
  if(which==2)field<unsigned>(side,0x40)=0;if(which==3)++field<unsigned>(possessed,0x1c8);
  if(which==4)field<unsigned char>(actor,0xf0)=0;if(which==5)field<uintptr_t>(side,0)=0;
  if(which==6)field<unsigned>(side,0x10)=32;if(which==7)field<unsigned>(independent,0x18)=2;
  query(side,cone_bytes.data(),candidates.data(),0);
  require(!field<unsigned>(buffer,0x2800)&&!acquisition207.kind,"208 cone ownership/side guard bypassed");
  state->release=0;field<unsigned>(side,0x10)=1;field<unsigned>(side,0x40)=1;field<uintptr_t>(side,0)=environment;
  if(which==3)--field<unsigned>(possessed,0x1c8);field<unsigned char>(actor,0xf0)=1;field<unsigned>(independent,0x18)=1;++checks;
 }
 // Rejected candidates never replace a valid parent; own unit fields stay exact.
 geometry(formation,10,0);acquisition207={};field<unsigned>(buffer,0x2800)=0;
 query(side,cone_bytes.data(),candidates.data(),0);require(field<unsigned>(buffer,0x2800)==1&&score(enemy,candidates.data(),current,100,0)==unit,"208 ordinary formation lost");++checks;
 geometry(formation,215,0);geometry(independent,120,0);field<unsigned>(buffer,0x2800)=0;
 query(side,cone_bytes.data(),candidates.data(),0);require(!field<unsigned>(buffer,0x2800),"208 native cone range widened");++checks;
 geometry(independent,40,0);acquisition207={};
 require(field<uintptr_t>(unit,0xb9c)==formation&&field<uintptr_t>(actor,0x3b4)==independent,"208 changed ownership");
 // Exercise the real unit-pair cache with cold AND already-cached parent
 // distances, both parameter orders, movement and failed ownership guards.
 uintptr_t id_vtable[]={reinterpret_cast<uintptr_t>(&fixture_id208)};
 auto saved_id=field<uintptr_t>(unit,0x10);field<uintptr_t>(unit,0x10)=reinterpret_cast<uintptr_t>(id_vtable);field<uintptr_t>(enemy,0x10)=reinterpret_cast<uintptr_t>(id_vtable);
 float distances[4]={-1,-1,-1,-1};uintptr_t cache[]={0,reinterpret_cast<uintptr_t>(distances),2};
 auto native_distance=reinterpret_cast<AcquisitionDistance208>(base+0x47efc0);
 using Squared208=float(__thiscall*)(void*,void*);
 auto squared208=reinterpret_cast<Squared208>(base+0x1aaed0);
 unsigned distance_checks=0;
 for(unsigned reverse:{0u,1u})for(float x:{40.f,80.f,250.f}){
  geometry(independent,x,0);distances[2]=-1;
  auto a=reverse?unit:enemy,b=reverse?enemy:unit;
  float parent_distance=original_distance208(cache,a,b);
  float expected=reverse?parent_distance:std::min(parent_distance,std::sqrt(squared208(reinterpret_cast<void*>(group+0x644),reinterpret_cast<void*>(independent+0x644))));
  require(parent_distance>200,"208 primary-only distance omission not reproduced");
  for(bool warm:{false,true}){
   distances[2]=warm?parent_distance:-1;acquisition207={};auto depth208=motion_callback_depth;
   unsigned before208{},after208{};__asm mov before208,esp
   float actual=native_distance(cache,a,b);
   __asm mov after208,esp
   require(before208==after208&&std::fabs(actual-expected)<.001f&&!acquisition207.kind&&motion_callback_depth==depth208,"208 native unit-distance result/ABI/scope failed");
   require(distances[2]==parent_distance,"208 polluted shared parent distance cache");++distance_checks;
  }
 }
 geometry(independent,40,0);
 for(unsigned which=0;which<5;++which){
  if(which==0)state->release=1;if(which==1)field<unsigned>(side,0x10)=0;if(which==2)field<unsigned>(side,0x40)=0;
  if(which==3)++field<unsigned>(possessed,0x1c8);if(which==4)field<unsigned char>(actor,0xf0)=0;
  require(native_distance(cache,enemy,unit)==distances[2],"208 unit-distance ownership/visibility guard bypassed");
  state->release=0;field<unsigned>(side,0x10)=1;field<unsigned>(side,0x40)=1;if(which==3)--field<unsigned>(possessed,0x1c8);field<unsigned char>(actor,0xf0)=1;++distance_checks;
 }
 require(native_distance(cache,enemy,enemy)==0&&native_distance(cache,unit,unit)==0,"208 native self-distance changed");++distance_checks;
 field<uintptr_t>(unit,0x10)=saved_id;
 std::cout<<"PASS acquisition208 distance: "<<distance_checks<<" original native cached-distance cases; hostile observer uses foot/horse geometry; parent observer/cache/guards/ABI retained.\n";
 std::cout<<"PASS acquisition208: "<<checks<<" direct shared cone/radial/scoring checks without normal-search scope; original omission reproduced; nested scopes/ABI/guards preserved. Mounted="<<(actor!=possessed)<<".\n";
}
