#include "playable_runtime.cpp"
static unsigned native_releases,native_prepares;
static void __fastcall fake_release(void*,void*,void*){++native_releases;}
static uint32_t __fastcall fake_prepare(void*,void*){++native_prepares;return 0xabcd007f;}
static unsigned contact_prepares,contact_updates,avoid_calls;
static uint32_t __fastcall fake_contact_prepare(void*,void*){++contact_prepares;return 0x24680000;}
static uint32_t __fastcall fake_contact_update(void*,void*){++contact_updates;return 0x13570001;}
static uint32_t __fastcall fake_avoid(void*,void*,void*){++avoid_calls;return 0x12340001;}
static unsigned target_calls;
static uint32_t __fastcall fake_target(void*,void*,uint32_t,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g,uint32_t h,uint32_t i,uint32_t j,uint32_t k,uint32_t l,uint32_t m){
    require(b==2&&c==3&&d==4&&e==5&&f==6&&g==7&&h==8&&i==9&&j==10&&k==11&&l==12&&m==13,"Target raw stack arguments changed");++target_calls;return 0x12345678;
}
static unsigned alignment_calls,playback_updates;
static uint32_t __fastcall align_pair(void* witness,void*){
    auto address=uint32_t(witness);
    require(waiting_for_alignment(address),"Continuation ran outside native alignment");
    if(++alignment_calls==2){
        retarget::at<uint8_t>(address,0xc)=1;
        retarget::at<uint8_t>(address,0xd0)=0;
        retarget::at<uint32_t>(address,0x98)=0x2000;
        for(auto participant:{committed_player,committed_enemy})retarget::at<uint32_t>(participant.address,0x688)=address;
    }
    return 0;
}
static uint32_t __fastcall advance_pair(void* witness,void*){
    require(!waiting_for_alignment(uint32_t(witness)),"Animation advanced before native alignment");
    ++playback_updates;return 0;
}
int main(){try{
    auto mapped212=VirtualAlloc(nullptr,0x1bc9000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);require(mapped212,"Reservation engine fixture");
    Status status{};state=&status;engine=uint32_t(mapped212);enabled=1;coordinator=1;
    std::vector<unsigned char> p(0x700),e(0x700),other(0x700),pu(0x1000),eu(0x1000),ou(0x1000),held(0x110),foreign(0x110),unrelated(0x110);
    auto ptr=[](auto& bytes){return uint32_t(bytes.data());};auto w=ptr(held),f=ptr(foreign),u=ptr(unrelated);
    unsigned serial=1;for(auto pair:{std::pair<uint32_t,uint32_t>{ptr(p),ptr(pu)},{ptr(e),ptr(eu)},{ptr(other),ptr(ou)}}){
        auto a=pair.first,unit=pair.second;retarget::at<uint32_t>(a,0)=engine+0x15dfec8;retarget::at<uint8_t>(a,0xf0)=1;retarget::at<uint32_t>(a,0x688)=w;
        retarget::at<uint32_t>(a,0x214)=unit;retarget::at<uint32_t>(a,0x1c8)=serial;retarget::at<uint32_t>(unit,4)=serial;retarget::at<uint32_t>(a,0x44)=serial;retarget::at<float>(a,0x4c)=1.f;++serial;
    }
    require(actor_identity(reader(),ptr(p),w,engine,committed_player)&&actor_identity(reader(),ptr(e),w,engine,committed_enemy),"Fixture identity");
    std::vector<unsigned char> root212(0x122),env212(0x28300),manager212(0x900),camera212(0x200);
    retarget::at<uint32_t>(engine,0x1bc8180)=ptr(root212);retarget::at<uint32_t>(ptr(root212),0x110)=ptr(env212);
    retarget::at<uint32_t>(ptr(env212),0x281fc)=ptr(manager212);retarget::at<uint32_t>(ptr(manager212),0x8e0)=ptr(camera212);retarget::at<uint32_t>(ptr(camera212),0x188)=ptr(p);
    observation::PossessionPrefix lease212{};lease212.magic=0x31504353;lease212.version=46;lease212.pid=GetCurrentProcessId();lease212.ready=1;lease212.mode=2;lease212.actor_id=1;lease212.unit_id=1;lease212.lease_generation=1;lease212.heartbeat=GetTickCount64();possession=&lease212;
    InterlockedExchange64(&delivery_lease212,1);InterlockedExchange64(&delivery_generation212,LONG64(lifetimes.get(w)));
    delivery_address=LONG(w);pending_address=LONG(w);native_release_actor=reinterpret_cast<retarget::ReleaseActor>(fake_release);native_prepare=reinterpret_cast<NativeWitnessCall>(fake_prepare);
    retarget::at<uint32_t>(f,0x24)=2;retarget::at<uint32_t>(f,0x10)=ptr(p);retarget::at<uint32_t>(f,0x14)=ptr(other);
    retarget::at<uint32_t>(u,0x24)=1;retarget::at<uint32_t>(u,0x10)=ptr(other);
    require(dispatch(f,1,false)==1&&!native_prepares&&conflicts_blocked==1,"Competing encounter reached original preparation");
    require(dispatch(u,2,false)==0xabcd007f&&native_prepares==1,"Unrelated encounter not forwarded exactly once");
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());release_actor_hook(reinterpret_cast<void*>(f),nullptr,e.data());
    require(native_releases==0&&foreign_releases_preserved==2,"Competing removal cleared the owned pair");
    native_target=reinterpret_cast<NativeTarget>(fake_target);
    require(target_hook(reinterpret_cast<void*>(f),nullptr,ptr(p),2,3,4,5,6,7,8,9,10,11,12,13)==0&&!target_calls,"In-flight foreign activation stole reservation");
    WitnessKey authorized212{w,uint64_t(delivery_generation212),{1,1,1,2,1,2}};
    require(authorize(authorized212,1,Result::Success,committed_player,committed_enemy),"Active reservation authorization fixture");
    authorization.sealed=true;
    uintptr_t before{},after{};__asm mov before,esp
    for(unsigned n=0;n<1000;++n)require(target_hook(reinterpret_cast<void*>(w),nullptr,ptr(p),2,3,4,5,6,7,8,9,10,11,12,13)==0x12345678,"Owned target assignment changed return");
    __asm mov after,esp
    require(before==after&&target_calls==1000,"Target RET0x34 ABI or exactly-once forwarding failed");
    // Contact witnesses have a distinct 0x54-byte layout. Their target at
    // +0x4c must not activate a replacement pair, even after a foreign backlink.
    std::vector<unsigned char> contact(0x54),other_contact(0x54);
    auto c=ptr(contact),oc=ptr(other_contact);
    retarget::at<uint32_t>(c,0)=retarget::at<uint32_t>(oc,0)=engine+0x15e1510;
    retarget::at<uint32_t>(c,0x24)=retarget::at<uint32_t>(oc,0x24)=1;
    retarget::at<uint32_t>(c,0x10)=retarget::at<uint32_t>(oc,0x10)=ptr(other);
    retarget::at<uint32_t>(c,0x4c)=ptr(p);retarget::at<uint32_t>(oc,0x4c)=ptr(other);
    native_contact_prepare=reinterpret_cast<NativeWitnessCall>(fake_contact_prepare);
    native_contact_update=reinterpret_cast<NativeWitnessCall>(fake_contact_update);
    native_avoid_pair=reinterpret_cast<NativeAvoidPair>(fake_avoid);
    retarget::at<uint32_t>(ptr(p),0x688)=f;
    require(contact_prepare_hook(contact.data(),nullptr)==1&&contact_update_hook(contact.data(),nullptr)==1&&
        !contact_prepares&&!contact_updates&&contacts_blocked==2,"Contact target stole the committed pair");
    require(contact_prepare_hook(other_contact.data(),nullptr)==0x24680000&&contact_update_hook(other_contact.data(),nullptr)==0x13570001&&
        contact_prepares==1&&contact_updates==1,"Unrelated contact did not forward its full native return exactly once");
    protected_update=w;retarget::at<uint8_t>(w,0xd1)=0;
    uintptr_t avoid_before{},avoid_after{};__asm mov avoid_before,esp
    for(unsigned n=0;n<1000;++n){
        require(avoid_pair_hook(p.data(),nullptr,e.data())==0,"Preplay locomotion cancelled established pair");
        require(avoid_pair_hook(e.data(),nullptr,p.data())==0,"Reversed pair lost reservation");
    }
    __asm mov avoid_after,esp
    require(avoid_before==avoid_after&&!avoid_calls&&preplay_avoidance_blocked==2000,"Avoidance RET4 ABI or reservation gate failed");
    require(avoid_pair_hook(p.data(),nullptr,other.data())==0x12340001&&avoid_calls==1,"Unrelated avoidance altered");
    protected_update=0;require(avoid_pair_hook(p.data(),nullptr,e.data())==0x12340001&&avoid_calls==2,"Non-witness locomotion altered");
    protected_update=w;retarget::at<uint8_t>(w,0xd1)=1;
    require(avoid_pair_hook(p.data(),nullptr,e.data())==0x12340001&&avoid_calls==3,"Playing native pair altered");
    retarget::at<uint8_t>(w,0xd1)=0;retarget::at<uint8_t>(ptr(p),0xf0)=0;
    require(avoid_pair_hook(p.data(),nullptr,e.data())==0x12340001&&avoid_calls==4,"Projectile-dead actor incorrectly protected");
    retarget::at<uint8_t>(ptr(p),0xf0)=1;retarget::at<uint32_t>(ptr(p),0x688)=w;
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,other.data());release_actor_hook(reinterpret_cast<void*>(w),nullptr,p.data());
    require(native_releases==2,"Unrelated or owned native cleanup blocked");
    retarget::at<uint32_t>(ptr(p),0x688)=f;release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());
    require(native_releases==2,"Competing backlink erased immutable reservation");
    retarget::at<uint32_t>(ptr(p),0x1c8)=999;release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());
    require(native_releases==3,"Recycled actor retained old reservation");
    retarget::at<uint32_t>(ptr(p),0x1c8)=committed_player.serial;
    delivery_address=0;release_actor_hook(reinterpret_cast<void*>(f),nullptr,e.data());require(native_releases==4,"Ended reservation retained protection");
    require(active_callbacks==0,"Release callback did not drain");
    // A mounted native witness owns the horse, but other engine paths may
    // address its rider. Protect only a proven reciprocal passenger alias.
    std::vector<unsigned char> horse(0x700),passengers(4);
    auto mount=ptr(horse),rider=ptr(p);
    retarget::at<uint32_t>(mount,0)=actor_table(engine,Horse);
    retarget::at<uint8_t>(mount,0xf0)=1;
    retarget::at<uint32_t>(mount,0x214)=ptr(pu);
    retarget::at<uint32_t>(mount,0x1c8)=983;
    retarget::at<uint32_t>(mount,0x44)=1;
    retarget::at<float>(mount,0x4c)=1.f;
    retarget::at<uint32_t>(mount,0x688)=w;
    retarget::at<uint32_t>(mount,0x28)=0x1234;
    retarget::at<uint32_t>(mount,0x6cc)=1;
    retarget::at<uint32_t>(mount,0x6d0)=ptr(passengers);
    retarget::at<uint32_t>(ptr(passengers),0)=rider;
    retarget::at<uint32_t>(rider,0x688)=w;
    retarget::at<uint32_t>(rider,0x28)=0x1234;
    retarget::at<uint32_t>(rider,0x210)=mount;
    require(actor_identity(reader(),mount,w,engine,committed_player),"Mounted reservation identity");
    delivery_address=LONG(w);pending_address=LONG(w);
    unsigned releases=native_releases,prepares=native_prepares;
    require(dispatch(f,1,false)==1&&native_prepares==prepares,"Rider alias was stolen by a foreign encounter");
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,horse.data());
    require(native_releases==releases,"Mounted pair or rider alias unregistered by foreign removal");
    retarget::at<uint32_t>(rider,0x210)=0;
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());
    require(native_releases==++releases,"Detached rider incorrectly retained protection");
    retarget::at<uint32_t>(rider,0x210)=mount;
    retarget::at<uint32_t>(mount,0x6cc)=2;
    release_actor_hook(reinterpret_cast<void*>(f),nullptr,p.data());
    require(native_releases==++releases,"Multiple passengers incorrectly admitted");
    retarget::at<uint32_t>(mount,0x6cc)=1;
    release_actor_hook(reinterpret_cast<void*>(w),nullptr,horse.data());
    require(native_releases==++releases&&active_callbacks==0,"Owned mounted cleanup blocked");
    // A verdict retains the same witness, including when its native
    // continuation chooses a different clip variant for that same verdict.
    retarget::at<uint32_t>(ptr(p),0x210)=0;
    retarget::at<uint32_t>(ptr(p),0x688)=0;
    retarget::at<uint32_t>(ptr(e),0x688)=0;
    require(combat_identity(reader(),ptr(p),engine,committed_player),"On-foot identity after dismount");
    retarget::at<uint32_t>(w,0x30)=ptr(p);retarget::at<uint32_t>(w,0x34)=ptr(e);retarget::at<uint32_t>(w,0x38)=2;
    retarget::at<uint8_t>(w,0xc)=0;retarget::at<uint8_t>(w,0xd0)=1;
    retarget::at<uint32_t>(w,0x98)=0x1000;retarget::at<NativeVerdict>(w,0x80)={0,ptr(p),ptr(e)};
    require(authorize(authorized212,2,Result::Success,committed_player,committed_enemy),"Pending verdict authorization");
    require(seal_authorization(w,authorized212.generation,{0,ptr(p),ptr(e)}),"Pending native verdict was not sealed");
    native_prepare=reinterpret_cast<NativeWitnessCall>(align_pair);
    native_update=reinterpret_cast<NativeWitnessCall>(advance_pair);
    owns_coordinator=true;
    for(unsigned i=1;i<=2;++i){
        lease212.heartbeat=GetTickCount64();
        require(controlled(w,authorized212.generation,true)==0&&alignment_calls==i&&!playback_updates,
                "A pending verdict was discarded or allowed to play early");
        require(dispatch(f,1,false)==1,"Crowd encounter replaced an aligning verdict");
    }
    require(controlled(w,authorized212.generation,true)==0&&playback_updates==1,"Exact pair did not play after alignment");
    owns_coordinator=false;
    VirtualFree(mapped212,0,MEM_RELEASE);
    std::cout<<"PASS: production conflict dispatch and release wrappers; reserved pair cannot be stolen or unregistered by foreign witness removal; unrelated, owned, replaced and ended-reservation cleanup forward exactly once. Local synthetic actors only.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
