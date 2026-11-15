#define main retained_role_commands157
#include "verify_native_commands100.cpp"
#undef main
static uintptr_t projectile157;static unsigned tasks157;
static void __fastcall task157(void*,void*,const uintptr_t* context){projectile157=context[0];++tasks157;}
int main(){SetUnhandledExceptionFilter(fault100);try{
    NativeFixture100 f;using P=NativeFixture100;auto a=possessed,v=P::ptr(f.view);
    std::vector<unsigned char> config(0x100),actions(0x20),root(0x40),defs(0x1100),personal(0x100),normal(0x100);
    field<uintptr_t>(P::ptr(actions),0)=P::ptr(root);field<uintptr_t>(P::ptr(root),0x28)=P::ptr(defs);
    field<uintptr_t>(a,0x23c)=P::ptr(actions);field<uintptr_t>(a,0x6cc)=P::ptr(personal);
    field<unsigned>(P::ptr(personal),0)=3;field<unsigned>(P::ptr(personal),0x60)=150;field<float>(P::ptr(personal),0x6c)=50;
    field<unsigned>(unit,0xf7c)=1;field<uintptr_t>(unit,0xf1c)=P::ptr(normal);field<unsigned>(P::ptr(normal),0)=60;
    field<unsigned>(P::ptr(normal),0x60)=100;field<float>(P::ptr(normal),0x6c)=200;
    DWORD old{};VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old);f.jump(0x3d95a0,reinterpret_cast<void*>(task157));
    unsigned poses=0;
    for(unsigned role:{0x15dbd68u,0x15dbfb4u,0x15dc590u,0x15dc1a8u,0x15dc39cu,0x15dcf54u,0x15dc978u,0x15dcb6cu,0x15dcd60u}){
        field<uintptr_t>(a,0)=base+role;field<uintptr_t>(a,0x20c)=P::ptr(config);state->heartbeat=GetTickCount64();
        possession_lease.release();require(followed(a,true),"Native special soldier failed acquisition");state->lease_generation=possession_lease.generation;
        NativeSubject subject;Choice97 choice{environment,P::ptr(f.phase),P::ptr(f.manager),v,unit,P::ptr(f.holder),a,field<unsigned>(a,0x1c8),field<unsigned>(a,0x218)};
        field<uintptr_t>(P::ptr(f.manager),0x8f8)=v;
        // Exercise production acquisition/camera guards across the native posture
        // domain, including the old special-config >=18 rejection. Root flags
        // remain authoritative; this does not make each posture player-movable.
        for(unsigned posture=0;posture<=0x5c;++posture){
            field<unsigned>(a,0x1f0)=posture;
            require(native_subject(a,unit,v,false,subject)&&followed(a,false)&&choice_valid97(v,choice),"Special config posture incorrectly rejects camera/lease");++poses;
        }
        field<unsigned>(P::ptr(defs),0xc44)=1;require(bow_record(unit)==P::ptr(personal),"Special soldier inherited formation projectile");
        uintptr_t context[17]{};context[0]=P::ptr(normal);
        reinterpret_cast<void(__thiscall*)(void*,const void*)>(base+0x391b50)(reinterpret_cast<void*>(a),context);
        require(projectile157==P::ptr(personal),"Original391b50 disagrees with actual actor projectile");
        field<unsigned>(P::ptr(defs),0xc44)=0;require(!bow_record(unit),"Non-shooting special role inherited peer weapon");
        field<unsigned>(P::ptr(defs),0xc44)=1;field<uintptr_t>(a,0x6cc)=0;
        require(bow_record(unit)==unit_projectile152(unit),"Native no-override fallback changed");field<uintptr_t>(a,0x6cc)=P::ptr(personal);
        for(unsigned offset:{0xa3u,0xa4u}){field<unsigned char>(a,offset)=1;require(!native_subject(a,unit,v,false,subject),"Root ownership flag bypassed");field<unsigned char>(a,offset)=0;}
        field<unsigned char>(a,0xa2)=1;require(native_subject(a,unit,v,false,subject),"Retained native airborne role rejected");field<unsigned char>(a,0xa2)=0;
        field<uintptr_t>(a,0x18)=unit;require(!native_subject(a,unit,v,false,subject),"Attached actor admitted as free locomotion");field<uintptr_t>(a,0x18)=0;
        field<unsigned char>(a,0xf0)=0;require(!native_subject(a,unit,v,false,subject),"Dead role acquired");field<unsigned char>(a,0xf0)=1;
        field<uintptr_t>(a,0x214)=0;require(!native_subject(a,unit,v,false,subject),"Foreign role acquired");field<uintptr_t>(a,0x214)=unit;
        field<unsigned>(a,0x1c8)=1;require(!followed(a,false),"Role serial reuse accepted");field<unsigned>(a,0x1c8)=0;
    }
    NativeSubject subject;
    for(unsigned role:{0x15e65f4u,0x15e8524u,0x15d1124u,0x15d0ed8u,0x15d1370u,0x15d04c8u,0x15dc784u,0x15dbb1cu,0x15dcf58u}){
        field<uintptr_t>(a,0)=base+role;require(!native_subject(a,unit,v,false,subject),"Non-soldier/intermediate/unknown role acquired");
    }
    field<uintptr_t>(a,0)=base+0x15dfec8;require(!native_subject(a,unit,v,false,subject),"Regular soldier accepted foreign config");
    require(tasks157==9&&poses==837,"Native special-role coverage incomplete");

    // The original getter reads the live override through unit+ed4. Supply
    // the Guard's original rifle and suppression projectile as fixture data.
    field<uintptr_t>(a,0)=base+0x15dfec8;
    field<unsigned>(unit,0xf7c)=15;
    field<float>(unit,0xf78)=10;
    std::vector<unsigned char> special(0x100),ability(0x80),entry(0x282c);
    auto set_rifle=[](uintptr_t record,const wchar_t* key){
        field<unsigned>(record,0)=254;
        field<unsigned>(record,4)=unsigned(std::wcslen(key));
        field<unsigned>(record,8)=128;
        field<uintptr_t>(record,0xc)=reinterpret_cast<uintptr_t>(key);
        field<unsigned>(record,0x60)=125;
        field<float>(record,0x6c)=200;
    };
    const auto standard=P::ptr(normal),special_record=P::ptr(special),definition=P::ptr(ability),live=P::ptr(entry),manager=unit+0xed4;
    set_rifle(standard,L"boshin_breech_loading_rifle");
    set_rifle(special_record,L"boshin_rifle_suppressive");
    field<uintptr_t>(manager,0)=base+0x15cfeec;
    field<uintptr_t>(manager,8)=unit;
    field<unsigned>(manager,0x24)=field<unsigned>(manager,0x28)=1;
    field<uintptr_t>(manager,0x2c)=live;
    field<uintptr_t>(live,4)=definition;
    field<unsigned>(definition,0)=61;
    field<uintptr_t>(definition,0x28)=special_record;
    auto override_record=special_record;
    require(bow_record(unit)==standard,"Ordinary Guard rifle selection");
    field<uintptr_t>(manager,0x40)=reinterpret_cast<uintptr_t>(&override_record);
    field<unsigned char>(live,0x18)=1;
    require(bow_record(unit)==standard&&unit_projectile152(unit)==special_record,
        "Supported suppression override leaked into the personal rifle or altered formation fire");
    require(field<uintptr_t>(manager,0x40)==reinterpret_cast<uintptr_t>(&override_record)&&field<float>(unit,0xf78)==10,
        "Personal rifle resolution wrote native override or ammunition");
    // Exercise an active override that fails the personal projectile validator.
    field<float>(special_record,0x6c)=0;
    require(bow_record(unit)==standard&&!unit_projectile152(unit),
        "Unusable suppression override disabled the verified personal rifle or rewrote formation selection");
    field<unsigned>(definition,0)=64;
    require(!bow_record(unit),"Foreign ability granted a rifle fallback");
    field<unsigned>(definition,0)=61;
    field<unsigned char>(live,0x18)=0;
    require(!bow_record(unit),"Inactive suppression ability granted a rifle fallback");
    field<unsigned char>(live,0x18)=1;
    field<unsigned>(unit,0xf7c)=0;
    require(!bow_record(unit),"A unit without an ammunition pool gained a rifle");
    field<unsigned>(unit,0xf7c)=15;
    field<uintptr_t>(manager,0x40)=0;
    require(bow_record(unit)==standard&&field<float>(unit,0xf78)==10,
        "Disabling suppression did not restore rifle selection or changed native ammunition");
    std::cout<<"PASS: original Guard rifle/active override selection, scoped suppression fallback, inactive/foreign/zero-pool rejection and return to normal. Fixture projectile metadata.\n";
    std::cout<<"PASS: nine native special soldier roles through acquisition/current camera choice, 837 posture cases, original391b50 actual projectile/no-shoot/fallback; strict root/life/unit/serial/config/non-soldier guards. Native scene and animation providers remain offline boundaries.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
