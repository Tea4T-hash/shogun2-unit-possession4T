#include "../../shogun2-possession-cleanup-v46/src/identity.h"
#include "hud_state89.h"
#include "../../../common/unit_activity215.h"
static void formation215(){
 using namespace unitactivity215;
 std::array<unsigned char,0x1000> unit{};
 std::array<unsigned char,0x700> player{},peer{},horse{},enemy{},animation{};
 std::array<unsigned char,0x100> witness{};
 auto ptr=[](auto& v){return reinterpret_cast<uintptr_t>(v.data());};auto put=[](uintptr_t at,auto v){memcpy(reinterpret_cast<void*>(at),&v,sizeof v);};
 const uintptr_t image=0x10000000u,env=7,u=ptr(unit),a=ptr(player),b=ptr(peer);uintptr_t roster[]{a,b};
 put(u+4,99u);put(u+0x2bc,2u);put(u+0x2c0,reinterpret_cast<uintptr_t>(roster));
 for(auto soldier:{a,b}){put(soldier,image+0x15dfec8);put(soldier+0x214,u);put(soldier+0x28,env);put(soldier+0x1c8,17u);put(soldier+0xf0,uint8_t(1));}
 auto reader=[](uintptr_t at,void* out,size_t n){SIZE_T got{};return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,n,&got)&&got==n;};Snapshot activity;
 put(a+0x8c,50.f);put(a+0x688,uintptr_t(1));put(a+0x1f0,14u);
 require(sample(reader,image,env,u,99,a,activity)&&activity.peers==1&&activity.activity==Activity::Holding,"Player movement/combat leaked into unit HUD");
 put(b+0x8c,2.f);require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Walking,"Peer walking missing");
 put(b+0x244,1u);require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Running,"Peer running missing");
 put(b+0x8c,0.f);
 const auto w=ptr(witness),e=ptr(enemy);
 put(e,image+0x15dfec8);put(e+0x28,env);put(e+0x214,u+4);put(e+0x44,2u);
 put(w+4,env);put(w+0xc,uint8_t(1));put(w+0x38,2u);put(w+0x30,b);put(w+0x34,e);
 put(b+0x688,w);put(e+0x688,w);
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Holding,"Reserved opponent counted as melee");
 put(w+0xd1,uint8_t(1));
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Melee,"Playing native pair missing");
 put(e+0x688,uintptr_t(0));
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Holding,"Stale pair counted as melee");
 put(e+0x688,w);put(w+0xd0,uint8_t(1));
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Holding,"Finished pair counted as melee");
 put(b+0x688,uintptr_t(0));
 put(b+0x1f0,14u);put(b+0x474,ptr(animation));put(b+0x1d4,126u);put(ptr(animation),126u);
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Firing,"Peer ranged cycle missing");put(ptr(animation),125u);
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Holding,"Stale ranged clip counted");
 put(b+0x210,ptr(horse));put(ptr(horse),image+soldier_roles::horse_table);put(ptr(horse)+0x214,u);put(ptr(horse)+0x28,env);put(ptr(horse)+0xf0,uint8_t(1));put(ptr(horse)+0x8c,5.f);put(ptr(horse)+0x244,1u);
 require(sample(reader,image,env,u,99,a,activity)&&activity.activity==Activity::Running,"Mounted peer running missing");
 require(wcscmp(icon(Activity::Running),icon(Activity::Walking))!=0&&wcscmp(icon(Activity::Melee),icon(Activity::Routing))!=0,"Activity symbols are ambiguous");
 put(b+0x214,u+4);require(!sample(reader,image,env,u,99,a,activity),"Foreign unit member accepted");put(b+0x214,u);put(u+0x2bc,2001u);require(!sample(reader,image,env,u,99,a,activity),"Unbounded roster accepted");
 require(ammunition(10,20)==.5f&&ammunition(0,20)==0&&ammunition(25,20)==1&&ammunition(10,0)==0&&ammunition(NAN,20)==0,"Native ammo meter bounds");
 std::cout<<"PASS HUD215: formation-only activity, player exclusion, ranged/melee/mounted peers, stale/foreign/roster guards and native-capacity meter\n";
}
int main(){try{using namespace hudstate89;formation215();
 Input s{true,true,true,false,true,1,0,1,1,0,20,0};
 auto a=resolve(s);require(a.fire_prompt&&a.reticle==Reticle::Drop,"Ready bow missing drop reticle");
 for(unsigned kind:{1u,3u})for(unsigned icon:{1u,3u,4u,5u,7u,8u,9u}){s.kind=kind;s.icon=icon;require(resolve(s).reticle==Reticle::Drop,"Nonfirearm projectile lost descending marks");}
 s.kind=2;require(resolve(s).reticle==Reticle::Firearm,"Firearm changed reticle");
 s.pending=true;require(!resolve(s).fire_prompt&&resolve(s).reticle==Reticle::None,"Pending switch looked confirmed");s.pending=false;
 s.ammunition=0;require(!resolve(s).fire_prompt,"Empty ammo looked ready");s.ammunition=20;
 s.phase=4;require(!resolve(s).fire_prompt,"Reload looked ready");s.phase=0;
 s.aim_pose=2;require(!resolve(s).fire_prompt,"Drawing looked drawn");s.aim_pose=1;
 s.aiming=false;require(!resolve(s).fire_prompt,"Lowered weapon looked raised");s.aiming=true;
 s.ammo_known=false;require(!resolve(s).fire_prompt&&resolve(s).reticle==Reticle::Firearm,
  "Known personal firearm lost its aim marker when unit ammo was unavailable");s.ammo_known=true;
 s.equipment=false;require(resolve(s).reticle==Reticle::None,"Foreign/stale equipment had a reticle");
 for(unsigned i=1;i<4;++i)require(drop_y[i]>drop_y[i-1]&&drop_width[i]<drop_width[i-1],"Descending reticle geometry changed");
 std::cout<<"PASS: native weapon kinds, nonfirearm descending reticles, firearm retention, pending/empty/reload/draw/lower/unavailable presentation.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
