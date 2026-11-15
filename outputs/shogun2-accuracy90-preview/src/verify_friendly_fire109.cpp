// Real retail473f80/40f130 bytes and production detour/ownership/registry.
// Damage resolution is an explicit provider substitution; no battle is launched.
#define main retained_commands109_main
#include "verify_native_commands100.cpp"
#undef main
static unsigned forwarded109;static uintptr_t expected_victim109,expected_projectile109;
static bool expect_victim_scope=true;
static uintptr_t __fastcall damage109(void*,void*,uintptr_t victim,void*,uintptr_t source,uintptr_t direction,uintptr_t projectile,uint32_t third){
    require(victim==expected_victim109&&projectile==expected_projectile109&&direction==projectile+0x98&&third==0xabcdef01,"Native impact arguments changed");
    require(field<uintptr_t>(source,0)==field<uintptr_t>(projectile,0x1c)&&field<uintptr_t>(source,4)==projectile&&field<unsigned>(source,8)==0x4a3,"Native hit source changed");
    require(projectile_impact_victim==(expect_victim_scope?victim:0),"Native projectile damage lost its victim scope");
    ++forwarded109;return 0x12345678;
}
int main(){SetUnhandledExceptionFilter(&fault100);try{
    NativeFixture100 f;using P=NativeFixture100;
    state->ready=1;state->heartbeat=GetTickCount64();
    std::vector<unsigned char> side(32),ally_side(32),other_unit(0x1800),pool(0x50),projectile(0x100),foreign(0x100);
    auto p=P::ptr(projectile),peer=P::ptr(f.peer),s=P::ptr(side),as=P::ptr(ally_side);
    field<unsigned>(s,0x10)=field<unsigned>(as,0x10)=1;
    field<uintptr_t>(possessed,0x44)=s;field<uintptr_t>(peer,0x44)=as;field<uintptr_t>(peer,0x214)=P::ptr(other_unit);field<unsigned>(P::ptr(other_unit),4)=8;
    field<uintptr_t>(p,0x1c)=possessed;field<unsigned>(p,4)=42;
    field<uintptr_t>(P::ptr(foreign),0x1c)=peer;field<unsigned>(P::ptr(foreign),4)=43;
    uintptr_t entries[]={P::ptr(foreign),p};field<unsigned>(P::ptr(pool),0x10)=2;field<uintptr_t>(P::ptr(pool),0x14)=reinterpret_cast<uintptr_t>(entries);
    friendly109::Status published{};published.magic=friendly109::magic;published.version=109;published.size=sizeof published;published.pid=GetCurrentProcessId();friendly_status109=&published;
    auto current=possession_lease.identity;auto generation=possession_lease.generation;PossessionIdentity proof;
    require(friendly_owned109(proof)&&same_soldier(proof,current),"Valid ownership rejected");
    friendly_tag109(P::ptr(pool),0,current,generation);friendly109::Tag saved;
    require(!friendly_registry109.find(P::ptr(foreign),43,generation,saved)&&friendly_registry109.find(p,42,generation,saved),"AI projectile was attributed to player");
    DWORD old{};require(VirtualProtect(reinterpret_cast<void*>(base+0x4282d0),16,PAGE_EXECUTE_READWRITE,&old)!=0,"Damage fixture protection");
    f.jump(0x4282d0,reinterpret_cast<void*>(&damage109));VirtualProtect(reinterpret_cast<void*>(base+0x4282d0),16,old,&old);FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(base+0x4282d0),16);
    require(MH_Initialize()==MH_OK,"Impact fixture hook init");create_friendly_hook109();require(MH_EnableHook(reinterpret_cast<void*>(base+0x473f80))==MH_OK,"Impact fixture hook enable");
    auto impact=reinterpret_cast<FriendlyImpact109>(base+0x473f80);expected_victim109=peer;expected_projectile109=p;
    for(unsigned i=0;i<200;++i){unsigned before{},after{};state->heartbeat=GetTickCount64();
        __asm mov before,esp
        auto result=impact(f.common.data(),peer,p,0xabcdef01,0x3f400000);
        require(!projectile_impact_victim,"Projectile victim scope leaked beyond the hit");
        __asm mov after,esp
        require(before==after&&result==0x12345678,"Impact RET10/EAX forwarding differs");
    }
    require(forwarded109==200&&published.event==1&&published.victim==12&&published.victim_unit==8&&published.projectile==42,"Allied contact not once-only or native forwarding suppressed");
    auto tag=[&](unsigned id){friendly_registry109={};field<unsigned>(p,4)=id;friendly_tag109(P::ptr(pool),1,current,generation);};
    auto hit=[&](){state->heartbeat=GetTickCount64();auto before=forwarded109;require(impact(f.common.data(),peer,p,0xabcdef01,0x3f400000)==0x12345678&&forwarded109==before+1,"Foreign/invalid hit did not forward");};
    tag(44);field<unsigned>(as,0x10)=2;hit();require(published.event==1,"Enemy hit triggered shame");field<unsigned>(as,0x10)=1;
    tag(45);field<unsigned>(p,4)=46;hit();require(published.event==1,"Recycled projectile address gained attribution");
    tag(47);++state->actor_id;hit();require(published.event==1,"Stale actor gained attribution");--state->actor_id;
    tag(48);++possession_lease.generation;hit();require(published.event==1,"Old possession gained attribution");--possession_lease.generation;
    tag(49);field<unsigned char>(peer,0xf0)=0;hit();require(published.event==1,"Dead target triggered shame");field<unsigned char>(peer,0xf0)=1;
    tag(50);state->mode=IDLE;hit();require(published.event==1,"RTS gained attribution");state->mode=POSSESSED;
    tag(51);field<uintptr_t>(P::ptr(f.manager),0x8fc)=peer;hit();require(published.event==1,"Lost camera holder gained attribution");field<uintptr_t>(P::ptr(f.manager),0x8fc)=possessed;
    tag(52);field<uintptr_t>(p,0x1c)=peer;hit();require(published.event==1,"AI shot with copied projectile id gained attribution");field<uintptr_t>(p,0x1c)=possessed;
    tag(53);hit();require(published.event==2&&friendly_errors109==0,"Second valid personal allied hit rejected");
    require(MH_DisableHook(reinterpret_cast<void*>(base+0x473f80))==MH_OK,"Impact hook disable");
    expect_victim_scope=false;auto count=published.event;hit();require(published.event==count,"Disabled observer still published");
    require(MH_Uninitialize()==MH_OK,"Impact hook cleanup");friendly_status109=nullptr;
    std::cout<<"PASS: original retail impact/source bytes, 200 exact ECX/RET10/EAX forwards, personal-only attribution, allied battle-team filtering, recycled ids/lease/holder/death/RTS rejection and once-only contacts. Damage provider substituted.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
