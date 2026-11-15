// Isolated mapped-engine regression; never initializes or starts a game.
#include "../../native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp"
int wmain(int argc,wchar_t**argv){HMODULE owned=nullptr;try{
 require(argc==2,"Pass isolated owned engine file");auto sha=hash_file(argv[1]);require(sha=="022fae2e2da92b59b8b1b8ee52887003c624e3c8eca060b209f5d792a7f874f5"||sha=="c4d91d74ce9ff8338cf61d5b021b03a2776830177a590fcac205509a42239545","Owned engine fixture hash");
 owned=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);require(owned,"Mapped owned image without initialization");engine=uint32_t(owned);
 Status status{};state=&status;enabled=1;
 std::vector<uint8_t> p(0x700),e(0x700),pu(0x1000),eu(0x1000),held(0x110),foreign(0x110),manager(0x100);uint32_t busy[4]{};
 auto at=[](auto&v){return uint32_t(v.data());};auto put=[](uint32_t a,auto v){memcpy(reinterpret_cast<void*>(a),&v,sizeof v);};
 unsigned id=1;for(auto entry:{std::pair<uint32_t,uint32_t>{at(p),at(pu)},{at(e),at(eu)}}){auto a=entry.first;put(a,engine+0x15dfec8);put(a+0xf0,uint8_t(1));put(a+0x1c8,id);put(a+0x2c,id);put(a+0x214,entry.second);put(entry.second+4,id);put(a+0x44,id);put(a+0x4c,1.f);++id;}
 put(at(foreign)+8,at(manager));put(at(manager)+0x88,uint32_t(busy));
 committed_player={at(p),1,at(pu),1,1,1.f,Human};committed_enemy={at(e),2,at(eu),2,2,1.f,Human};delivery_address=LONG(at(held));possession=nullptr;
 native_release_actor=reinterpret_cast<retarget::ReleaseActor>(engine+0x3e1930);
 using Eligible=bool(__thiscall*)(void*,uint32_t);auto eligible=reinterpret_cast<Eligible>(engine+0x3f4f30);
 unsigned count=0;
 for(unsigned survivors:{240u,12u,2u,1u})for(auto a:{at(p),at(e)}){
  put(at(pu)+0x2bc,survivors);put(at(eu)+0x2bc,survivors);put(a+0x688,at(held));put(a+0x1e8,at(held));auto index=*reinterpret_cast<uint32_t*>(a+0x2c);busy[index>>5]|=1u<<(index&31);
  require(!eligible(manager.data(),a),"Original eligibility did not reject native busy bit");
  release_actor_hook(foreign.data(),nullptr,reinterpret_cast<void*>(a));
  require(eligible(manager.data(),a)&&!*reinterpret_cast<uint32_t*>(a+0x688)&&!*reinterpret_cast<uint32_t*>(a+0x1e8)&&!(busy[index>>5]&(1u<<(index&31))),"Original native release did not restore melee eligibility");

  ++count;
 }
 require(active_callbacks==0,"Release callback scope did not drain");FreeLibrary(owned);
 std::cout<<"PASS native212: "<<count<<" original-engine cases clear busy bits/backlinks via unmodified native release and restore unmodified melee eligibility. No engine initialization or live process.\n";

 return 0;
}catch(const std::exception&x){std::cerr<<x.what()<<'\n';if(owned)FreeLibrary(owned);return 1;}}
