#include "playable_runtime.cpp"
static unsigned prepares212,updates212,releases212,targets212,contacts212,avoids212;
static uint32_t __fastcall prepare212(void*,void*){++prepares212;return 0xaabb0000;}
static uint32_t __fastcall update212(void*,void*){++updates212;return 0xccdd0000;}
static void __fastcall release212(void*,void*,void*){++releases212;}
static uint32_t __fastcall target212(void*,void*,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t){++targets212;return 0x11220000;}
static uint32_t __fastcall contact212(void*,void*){++contacts212;return 0x33440000;}
static uint32_t __fastcall avoid212(void*,void*,void*){++avoids212;return 0x55660001;}
int main(){void* allocation=nullptr;try{
 allocation=VirtualAlloc(nullptr,0x1bc9000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);require(allocation,"Fixture allocation");engine=uint32_t(allocation);
 Status status{};state=&status;enabled=1;coordinator=1;
 std::vector<uint8_t> p(0x700),e(0x700),pu(0x1000),eu(0x1000),w(0x110),f(0x110),c(0x54),root(0x122),env(0x28300),manager(0x900),camera(0x200);
 auto at=[](auto& v){return uint32_t(v.data());};auto put=[](uint32_t a,auto v){memcpy(reinterpret_cast<void*>(a),&v,sizeof v);};
 put(engine+0x1bc8180,at(root));put(at(root)+0x110,at(env));put(at(env)+0x281fc,at(manager));put(at(manager)+0x8e0,at(camera));put(at(camera)+0x188,at(p));
 unsigned serial=11;for(auto entry:{std::pair<uint32_t,uint32_t>{at(p),at(pu)},{at(e),at(eu)}}){
  auto a=entry.first,u=entry.second;put(a,engine+0x15dfec8);put(a+0xf0,uint8_t(1));put(a+0x1c8,serial);put(a+0x214,u);put(u+4,serial);put(a+0x44,serial);put(a+0x4c,1.f);put(a+0x688,at(w));put(a+0x28,at(env));++serial;
 }
 require(actor_identity(reader(),at(p),at(w),engine,committed_player)&&actor_identity(reader(),at(e),at(w),engine,committed_enemy),"Fixture actor identity");
 observation::PossessionPrefix lease{};lease.magic=0x31504353;lease.version=46;lease.pid=GetCurrentProcessId();lease.ready=1;lease.mode=2;lease.actor_id=11;lease.unit_id=11;lease.lease_generation=7;lease.heartbeat=GetTickCount64();possession=&lease;
 auto generation=lifetimes.get(at(w));delivery_address=LONG(at(w));pending_address=LONG(at(w));
#ifndef MELEE212_LEGACY
 InterlockedExchange64(&delivery_lease212,7);InterlockedExchange64(&delivery_generation212,LONG64(generation));
#endif
 put(at(w),engine+0x15e1460);put(at(w)+4,at(env));put(at(w)+0xc,uint8_t(1));put(at(w)+0x38,2u);put(at(w)+0x30,at(p));put(at(w)+0x34,at(e));
 put(at(f)+0x24,2u);put(at(f)+0x10,at(p));put(at(f)+0x14,at(e));put(at(c),engine+0x15e1510);put(at(c)+0x24,1u);put(at(c)+0x10,at(e));put(at(c)+0x4c,at(p));
 native_prepare=reinterpret_cast<NativeWitnessCall>(&prepare212);native_update=reinterpret_cast<NativeWitnessCall>(&update212);native_release_actor=reinterpret_cast<retarget::ReleaseActor>(&release212);native_target=reinterpret_cast<NativeTarget>(&target212);native_contact_prepare=native_contact_update=reinterpret_cast<NativeWitnessCall>(&contact212);native_avoid_pair=reinterpret_cast<NativeAvoidPair>(&avoid212);
 require(fps_actor()==at(p),"Fixture current possession");protected_update=at(w);
 require(dispatch(at(f),1,false)==1&&!prepares212,"Active pair lost protection");
 release_actor_hook(f.data(),nullptr,p.data());require(!releases212,"Active reservation release was stolen");
 require(contact_prepare_hook(c.data(),nullptr)==1&&!contacts212,"Active contact reservation was stolen");
 require(avoid_pair_hook(p.data(),nullptr,e.data())==0&&!avoids212,"Active pair avoidance changed");
 // Returning to RTS leaves a still-live prior pair. No numeric saved reservation
 // may suppress original AI contact, target assignment, cleanup or avoidance.
 lease.mode=0;
 auto result=dispatch(at(f),1,false);release_actor_hook(f.data(),nullptr,p.data());auto contact=contact_prepare_hook(c.data(),nullptr);auto target=target_hook(f.data(),nullptr,at(p),2,3,4,5,6,7,8,9,10,11,12,13);auto avoid=avoid_pair_hook(p.data(),nullptr,e.data());
#ifdef MELEE212_LEGACY
 require(result==1&&contact==1&&target==0&&avoid==0&&!prepares212&&!releases212&&!targets212&&!contacts212&&!avoids212,"Baseline did not reproduce RTS reservation interference");
 std::cout<<"REPRODUCED212: after return to RTS, previous encounter still vetoes AI preparation, contact, target assignment, cleanup and avoidance. All five original ports suppressed; no game process used.\n";
#else
 require(result==0xaabb0000&&contact==0x33440000&&target==0x11220000&&avoid==0x55660001&&prepares212==1&&releases212==1&&targets212==1&&contacts212==1&&avoids212==1,"RTS encounter still vetoed by a prior possession reservation");
 require(dispatch(at(f),1,true)==0xccdd0000&&updates212==1,"RTS native update not forwarded once");
 // Exercise the negative gate itself across release, stale heartbeats, lease
 // replacement and native witness address reuse; none may retain suppression.
 lease.mode=2;lease.heartbeat=GetTickCount64();require(live_reservation212(pair_reservation()),"Fresh reservation rejected");
 lease.release=1;require(!live_reservation212(pair_reservation()),"Released lease retains reservation");lease.release=0;
 lease.stop=1;require(!live_reservation212(pair_reservation()),"Stopped lease retains reservation");lease.stop=0;
 lease.heartbeat=GetTickCount64()-1001;require(!live_reservation212(pair_reservation()),"Stale heartbeat retains reservation");lease.heartbeat=GetTickCount64();
 ++lease.lease_generation;require(!live_reservation212(pair_reservation()),"New possession inherits previous reservation");--lease.lease_generation;
 lifetimes.get(at(w),true);require(!live_reservation212(pair_reservation()),"Reused native encounter inherits reservation");
 InterlockedExchange64(&delivery_generation212,LONG64(lifetimes.get(at(w))));require(live_reservation212(pair_reservation()),"Current encounter generation rejected");
 lifetimes.removed(at(w),uint64_t(delivery_generation212));require(!live_reservation212(pair_reservation()),"Removed encounter retains reservation");
 // Repeat RTS attack/cleanup callbacks with surviving former participants:
 // no head count or new timeout disables native combat as casualties mount.
 lease.mode=0;
 for(unsigned count:{240u,12u,2u,1u})for(unsigned i=0;i<1000;++i){
  put(at(pu)+0x2bc,count);put(at(eu)+0x2bc,count);
  require(dispatch(at(f),i+1,false)==0xaabb0000&&dispatch(at(f),i+1,true)==0xccdd0000,"RTS crowd encounter blocked");
  require(contact_prepare_hook(c.data(),nullptr)==0x33440000&&contact_update_hook(c.data(),nullptr)==0x33440000,"RTS crowd contact blocked");
  release_actor_hook(f.data(),nullptr,p.data());
 }
 require(prepares212==4001&&updates212==4001&&releases212==4001&&contacts212==8001,"RTS callbacks failed exactly-once forwarding");
 require(active_callbacks==0&&!owns_coordinator,"Hook scope did not drain");
 std::cout<<"PASS melee212: active QTE protection retained; RTS prepare/update/contact/target/release/avoid return and exactly-once forwarding restored. Local actors and original-port sentinels only; live battle causation not proven.\n";
#endif
 VirtualFree(allocation,0,MEM_RELEASE);return 0;
}catch(const std::exception&x){std::cerr<<x.what()<<'\n';if(allocation)VirtualFree(allocation,0,MEM_RELEASE);return 1;}}
