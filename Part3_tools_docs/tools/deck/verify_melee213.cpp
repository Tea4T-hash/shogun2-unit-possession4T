// Original candidate registration, QTE list reduction and encounter destruction.
#include "../../native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp"
static unsigned releases213;static uint32_t fail_write213;
static bool write213(void* context,uint32_t address,const void* value,size_t size){if(address==fail_write213)return false;return write_local(context,address,value,size);}
static void release213(void*,uint32_t witness,uint32_t actor){++releases213;native_release_actor(reinterpret_cast<void*>(witness),reinterpret_cast<void*>(actor));}
int wmain(int argc,wchar_t**argv){HMODULE owned=nullptr;try{
 require(argc==2,"Owned engine path required");auto hash=hash_file(argv[1]);require(hash=="022fae2e2da92b59b8b1b8ee52887003c624e3c8eca060b209f5d792a7f874f5"||hash=="c4d91d74ce9ff8338cf61d5b021b03a2776830177a590fcac205509a42239545","Owned image hash");
 owned=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);require(owned,"Owned image mapped without initialization");engine=uint32_t(owned);
 std::vector<uint8_t> witness(0x140),manager(0x100),env(0x40),common(0x200),rider(0x700);uint32_t seat{};std::vector<std::vector<uint8_t>> actors(5,std::vector<uint8_t>(0x700)),units(2,std::vector<uint8_t>(0x1000));uint32_t busy[4]{};
 auto at=[](auto&v){return uint32_t(v.data());};auto put=[](uint32_t a,auto v){memcpy(reinterpret_cast<void*>(a),&v,sizeof v);};
 auto w=at(witness),mg=at(manager);put(at(env)+8,at(common));put(at(common)+0x9c,0x12340000u);put(mg+0x88,uint32_t(busy));
 for(unsigned i=0;i<5;++i){auto a=at(actors[i]),u=at(units[i&1]);put(a,engine+0x15dfec8);put(a+0xf0,uint8_t(1));put(a+0x1c8,i+1);put(a+0x2c,i+1);put(a+0x214,u);put(u+4,(i&1)+1);put(a+0x44,i&1);put(a+0x4c,1.f);}
 using Add=bool(__thiscall*)(void*,uint32_t);using Retire=void(__thiscall*)(void*);using Eligible=bool(__thiscall*)(void*,uint32_t);
 auto add=reinterpret_cast<Add>(engine+0x3c0b10);auto retire=reinterpret_cast<Retire>(engine+0x3bf170);auto eligible=reinterpret_cast<Eligible>(engine+0x3f4f30);native_release_actor=reinterpret_cast<retarget::ReleaseActor>(engine+0x3e1930);
 unsigned cases=0;
 for(bool mounted:{false,true})for(unsigned count:{3u,4u,5u})for(unsigned mode=0;mode<5;++mode){
  auto horse=at(actors[0]),human=at(rider);put(horse,engine+(mounted?0x15e8524:0x15dfec8));
  if(mounted){seat=human;put(horse+0x6cc,1u);put(horse+0x6d0,uint32_t(&seat));put(human,engine+0x15dfec8);put(human+0xf0,uint8_t(1));put(human+0x210,horse);put(human+0x214,at(units[0]));put(human+0x1c8,99u);put(human+0x4c,1.f);}
  std::fill(witness.begin(),witness.end(),uint8_t(0));for(auto&x:busy)x=0;put(w+4,at(env));put(w+8,mg);put(w+0x28,5u);fail_write213=0;releases213=0;
  for(unsigned i=0;i<count;++i){put(at(actors[i])+0x688,0u);require(eligible(manager.data(),at(actors[i])),"Native actor initially ineligible");require(add(witness.data(),at(actors[i])),"Original native candidate registration refused");require(!eligible(manager.data(),at(actors[i])),"Original candidate did not reserve busy bit");}
  NativeActor p,e;require(combat_identity(reader(),at(actors[0]),engine,p)&&combat_identity(reader(),at(actors[1]),engine,e),"Selected pair identity");
  if(mode){
   if(mode==3)fail_write213=w+0x24;if(mode==4)fail_write213=w+0x10;
   auto result=strictmelee::retain_selected(reader(),write213,nullptr,engine,w,p,e,mode==2?nullptr:release213);
   require(result==(mode==1),"Unexpected retention result");
   require(*reinterpret_cast<uint32_t*>(w+0x24)==(mode==1?2u:count),"Native candidate count not transactional");
   require(releases213==(mode==1?count-2:0),"Excluded releases not exactly once");
   for(unsigned i=0;i<count;++i){
    require(eligible(manager.data(),at(actors[i]))==(mode==1&&i>=2),"Selected reservation lost or excluded reservation leaked");
    if(mode!=1)require(*reinterpret_cast<uint32_t*>(w+0x10+i*4)==at(actors[i]),"Failed retention changed original list");
   }
  }
  retire(witness.data());
  for(unsigned i=0;i<count;++i)require(eligible(manager.data(),at(actors[i])),"Excluded candidate remains locked after native destruction");
  ++cases;
 }
 FreeLibrary(owned);
 std::cout<<"PASS native213: "<<cases<<" original-engine foot/mounted candidate/retirement cases; selected pair retained; excluded reservations cleaned natively; missing cleanup and failed writes preserve full native list; all actors regain original melee eligibility. No game initialization or live process.\n";
 return 0;
}catch(const std::exception&x){std::cerr<<x.what()<<'\n';if(owned)FreeLibrary(owned);return 1;}}
