#include "identity.h"
#include "command_policy100.h"
#include "command_input100.h"
#include "command_protocol100.h"
#include "self_visibility100.h"
int main(){try{
    using namespace command100;
    require(bearing(0)==0&&bearing(tau*.25f)==16384,"Native bearing conversion differs");
    auto volley=std::make_unique<Volley>();ActorKey first{0x1000,0x2000,0},reuse{0x1000,0x2000,1};
    require(!volley->available(first),"Command fire available before player shot");
    for(unsigned cycle=0;cycle<1000;++cycle){
        volley->shot();require(volley->available(first)&&volley->consume(first)&&!volley->consume(first),"One native release per current actor/shot violated");
        require(volley->consume(reuse),"A current new serial was conflated with a retired actor");
    }
    volley->reset();require(!volley->available(first)&&volley->count==0,"Lease reset retained shot permission");volley->shot();
    for(unsigned i=0;i<3000;++i)require(volley->consume({uintptr_t(0x10000+i*16*Volley::capacity),uintptr_t(0x50000+i*16*Volley::capacity),0}),"Bounded collision table lost a current actor");
    require(volley->count==3000,"Volleys accumulated or dropped actors");volley->shot();require(volley->count==0,"Shot churn was not bounded");
    KeyFilter filter;std::uint8_t current[256]{},previous[256]{};
    for(unsigned scan:scans)current[scan]=previous[scan]=0x80;require(filter.apply(current,previous,true,true)==31,"New keys were not masked from native input");
    for(unsigned scan:scans)require(current[scan]==0&&previous[scan]==0,"Native key edge leaked");
    for(unsigned scan:scans)current[scan]=0x80;filter.apply(current,previous,false,true);for(unsigned scan:scans)require(current[scan]==0,"Held key leaked into RTS handoff");
    memset(current,0,sizeof current);filter.apply(current,previous,false,true);current[scans[0]]=0x80;filter.apply(current,previous,false,true);require(current[scans[0]]==0x80,"Released command key stayed masked in RTS");
    require(next_fire(FireMode::Hold)==FireMode::AtWill&&next_fire(FireMode::AtWill)==FireMode::OnCommand&&next_fire(FireMode::OnCommand)==FireMode::Hold,"Requested firing cycle differs");
    FollowTarget follow;Point p{10,0,20};require(follow.changed(p,1000),"Stationary initial follow not issued");follow.accepted(p,0,1000);
    for(unsigned i=0;i<3600;++i){
        float look=float(i)*.1f;require(!follow.changed(p,2000+i*16),"Mouse look restarted a stationary formation");
        require(std::abs(follow.heading({11,0,20},look)-tau*.25f)<1e-6f,"Travel heading inherited mouse look");
    }
    require(!follow.changed({10.5f,0,20},100000)&&follow.changed({11,0,20},100000),"Follow translation threshold differs");
    require(!follow.changed({NAN,0,20},100000),"Nonfinite follow target accepted");
    Tap102 tap;
    for(unsigned window:{100u,250u,500u,900u}){
        tap.arm(1000,7);require(tap.consume(1000+window,window,7),"Double tap lost system interval boundary");
        require(!tap.consume(1001+window,window,7),"Third tap inherited consumed pair");
        tap.arm(1000,7);require(!tap.consume(1001+window,window,7),"Late tap became run");
        tap.arm(1000,7);require(!tap.consume(1100,window,8),"New lease inherited double tap");
        tap.arm(1000,7);require(!tap.consume(999,window,7),"Backward clock became double tap");
        tap.arm(1000,7);tap.reset();require(!tap.consume(1001,window,7),"Focus/reset retained tap");
    }
    FollowPace102 pace;
    require(pace.update(8,false)&&pace.catchup,"Distant walking leader did not request native catch-up run");
    for(unsigned frame=0;frame<1000;++frame)require(pace.update(frame%2?2.1f:3.9f,false),"Catch-up boundary churn");
    require(!pace.update(2,false)&&!pace.catchup&&!pace.update(4,false),"Arrived formation did not return to walk");
    require(pace.update(1,true)&&!pace.catchup&&!pace.update(1,false),"Caught-up formation did not match leader gait");
    follow.reset();follow.accepted(p,1.2f,1000,false);
    require(!follow.changed(p,1249,true)&&follow.changed(p,1250,true),"Pace change lost cadence");
    require(follow.heading(p,2.4f)==1.2f&&follow.target({10.2f,0,20}).x==10,"Pace-only order rotated or drifted target");
    follow.accepted(p,1.2f,1250,true);require(!follow.changed(p,100000,true),"Unchanged run restarted native formation");
    int32_t parents[8]={-1,0,0,0,3,-1,-1,-1};unsigned weapons[3]={5,6,7};float native[96]{},visible[96]{};
    for(unsigned i=0;i<8;++i){native[i*12]=native[i*12+5]=native[i*12+10]=1;native[i*12+7]=1.6f;native[i*12+11]=.2f;}native[4*12+7]=1.8f;
    memcpy(visible,native,sizeof native);
    for(unsigned i=0;i<1000;++i)require(visibility100::hide(visible,native,8,parents,4,3,1,2,weapons),"Self visibility rejected valid native rig");
    for(unsigned i:{0u,1u,2u,5u,6u,7u})require(!memcmp(visible+i*12,native+i*12,48),"Self visibility changed body/arms/weapon");
    require(visible[3*12]==1.f&&visible[4*12]==.0001f,"Neck/head hiding accumulated scale");
    parents[1]=3;memcpy(visible,native,sizeof native);require(visibility100::hide(visible,native,8,parents,4,3,1,2,weapons)&&visible[1*12]==1&&visible[3*12]==1,"Neck branch swallowed an arm");
    parents[1]=0;parents[0]=4;float untouched[96];memcpy(untouched,visible,sizeof visible);require(!visibility100::hide(visible,native,8,parents,4,3,1,2,weapons)&&!memcmp(visible,untouched,sizeof visible),"Invalid hierarchy partially mutated");
    command100::Status sample{};sample.magic=magic;sample.version=100;sample.size=sizeof sample;sample.pid=1;sample.lease=2;sample.actor=3;sample.unit=4;sample.tick=100;
    require(valid(sample,1,2,3,4,100)&&!valid(sample,2,2,3,4,100)&&!valid(sample,1,3,3,4,100),"Numeric command ownership gate differs");sample.fire_mode=3;require(!valid(sample,1,2,3,4,100),"Unknown command enum reached HUD");
    sample.fire_mode=0;sample.feedback=unsigned(Feedback::SkirmishOff)+1;
    require(!valid(sample,1,2,3,4,100),"Unknown command acknowledgement reached HUD");
    std::cout<<"PASS: native bearing, 1000 shot epochs, 3000 colliding identities, held-key drain, follow cadence, 1000 idempotent visibility frames and numeric HUD guards\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
