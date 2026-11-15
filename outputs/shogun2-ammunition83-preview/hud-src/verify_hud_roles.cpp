// Production measurement, reading only this fixture's own synthetic memory.
#define wmain imported_hud_entry
#include "hud.cpp"
#undef wmain

template<class T>static void put_role(uintptr_t p,unsigned off,T value){memcpy(reinterpret_cast<void*>(p+off),&value,sizeof value);}
int main(){try{
    auto image=VirtualAlloc(nullptr,0x2000000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    require(image!=nullptr,"fixture allocation");base=reinterpret_cast<uintptr_t>(image);pid=GetCurrentProcessId();process=GetCurrentProcess();
    auto root=base+0x1000,env=base+0x2000,common=base+0x3000,phase=base+0x4000,manager=base+0x7000,camera=base+0x8000;
    auto actor=base+0x9000,unit=base+0xa000,def=base+0xc000,obj=base+0xc100,army=base+0xd000,army_obj=base+0xe000,roster=base+0xe100;
    put_role(base,0x1bc8180,root);put_role(root,0x110,env);put_role(env,8,common);put_role(common,0xb4,phase);put_role(phase,0x14,5u);
    put_role(env,0x281fc,manager);put_role(manager,0x8e0,camera);put_role(camera,0,base+0x15d22e8);put_role(camera,0x188,actor);
    put_role(actor,0x1c8,19u);put_role(actor,0x214,unit);put_role(actor,0xf0,static_cast<unsigned char>(1));
    put_role(unit,4,20u);put_role(unit,0x2bc,120u);put_role(unit,0xcbc,120u);put_role(unit,0x1c,def);put_role(def,0,obj);
    static const wchar_t unit_name[]=L"Inf_Missile_Bow_Ashigaru";put_role(obj,8,reinterpret_cast<uintptr_t>(unit_name));
    put_role(unit,0x2a4,army);put_role(army,0,base+0x15bc860);put_role(army,0x114,1u);put_role(army,0x118,roster);put_role(roster,0,unit);
    put_role(army,0xf4,army_obj);static const wchar_t clan[]=L"oda";put_role(army_obj,8,reinterpret_cast<uintptr_t>(clan));
    Status telemetry{};shared=&telemetry;telemetry.magic=POC_MAGIC;telemetry.version=46;telemetry.size=sizeof telemetry;telemetry.pid=pid;
    telemetry.ready=1;telemetry.mode=POSSESSED;telemetry.actor_id=19;telemetry.unit_id=20;telemetry.lease_generation=1;
    const unsigned tables[]={0x15dfec8,0x15dbd68,0x15dbfb4,0x15dc590,0x15dc1a8,0x15dc39c,0x15dcf54,0x15dc978,0x15dcb6c,0x15dcd60};
    unsigned checks=0;
    auto measured=[&](){telemetry.heartbeat=GetTickCount64();try{return measure();}catch(...){return false;}};
    for(unsigned t:tables){
        put_role(actor,0,base+t);require(actor152::human(base,base+t)&&measured(),"supported role HUD");++checks;
        put_role(actor,0x1c8,18u);require(!measured(),"stale actor serial rejected");put_role(actor,0x1c8,19u);++checks;
        put_role(actor,0xf0,static_cast<unsigned char>(0));require(!measured(),"dead actor rejected");put_role(actor,0xf0,static_cast<unsigned char>(1));++checks;
        put_role(unit,4,21u);require(!measured(),"foreign unit rejected");put_role(unit,4,20u);++checks;
        put_role(army,0x114,0u);require(!measured(),"invalid ownership rejected");put_role(army,0x114,1u);++checks;
        telemetry.mode=0;require(!measured(),"unpossessed actor rejected");telemetry.mode=POSSESSED;++checks;
    }
    for(unsigned t:{0u,0x15e8524u,0x15df000u,0x15ffffffu}){put_role(actor,0,base+t);require(!measured(),"unsupported class rejected");++checks;}
    put_role(actor,0,base+tables[0]);put_role(phase,0x14,4u);require(!measured(),"nonbattle phase rejected");++checks;
    put_role(phase,0x14,5u);telemetry.pid=pid+1;require(!measured(),"foreign process rejected");++checks;
    telemetry.pid=pid;
    // Real production measurement must retain custom units without stock art,
    // while every role/identity/liveness/ownership guard still applies.
    static const wchar_t custom169[]=L"Armored_Addon_Unit_169";
    require(hud80::unit_art(custom169)==nullptr,"Fixture accidentally uses stock artwork");
    require(hud80::class_name(hud80::unit_art(custom169)).empty(),"Missing custom unit type must stay blank");++checks;
    put_role(obj,8,reinterpret_cast<uintptr_t>(custom169));
    for(unsigned t:tables){
        put_role(actor,0,base+t);require(measured(),"Custom unit lost its HUD");++checks;
        require(!display.unit&&display.native_unit_key==custom169&&hud80::display_name169(display)==L"Armored Addon Unit 169","Custom unit identity or readable label differs");++checks;
        put_role(actor,0x1c8,18u);require(!measured(),"Custom stale actor accepted");put_role(actor,0x1c8,19u);++checks;
        put_role(actor,0xf0,static_cast<unsigned char>(0));require(!measured(),"Custom dead actor accepted");put_role(actor,0xf0,static_cast<unsigned char>(1));++checks;
        put_role(unit,4,21u);require(!measured(),"Custom foreign unit accepted");put_role(unit,4,20u);++checks;
        put_role(army,0x114,0u);require(!measured(),"Custom invalid ownership accepted");put_role(army,0x114,1u);++checks;
        telemetry.mode=0;require(!measured(),"Custom unpossessed actor accepted");telemetry.mode=POSSESSED;++checks;
    }
    static const wchar_t custom_clan169[]=L"Addon_Clan_169";
    put_role(army_obj,8,reinterpret_cast<uintptr_t>(custom_clan169));
    require(measured()&&!display.owner,"Custom faction without stock mon hid HUD");++checks;
    put_role(army_obj,8,reinterpret_cast<uintptr_t>(clan));
    put_role(obj,8,reinterpret_cast<uintptr_t>(unit_name));
    require(measured()&&display.unit&&hud80::display_name169(display)==display.unit->name,"Stock presentation changed after custom unit");++checks;
    // Exercise the production preloader, then measure the same card after
    // removing its input. It must already be cached before possession.
    wchar_t temp[MAX_PATH]{}, scratch[MAX_PATH]{};
    require(GetTempPathW(MAX_PATH,temp)>0&&GetTempFileNameW(temp,L"s2c",0,scratch)!=0&&DeleteFileW(scratch),"Card fixture scratch");
    auto folder=std::filesystem::path(scratch);
    auto art=folder/L"data/ui/units/info/roster_card.tga";
    std::filesystem::create_directories(art.parent_path());
    {
        unsigned char picture[]={0,0,2,0,0,0,0,0,0,0,0,0,1,0,1,0,32,32,0,0,73,255};
        std::ofstream f(art,std::ios::binary);
        require(bool(f.write(reinterpret_cast<const char*>(picture),sizeof picture)),"Card fixture write");
    }
    cards217.configure(folder);
    auto alliance=base+0xf000,alliance_list=base+0xf100,armies=base+0xf200,alias=base+0xf300;
    static const wchar_t portrait[]=L"roster_card";
    put_role(phase,0x88,1u);put_role(phase,0x8c,alliance_list);put_role(alliance_list,0,alliance);
    put_role(alliance,0x20,1u);put_role(alliance,0x24,armies);put_role(armies,0,army);
    put_role(army,0x270,static_cast<unsigned char>(1));
    put_role(def,0x54,alias);put_role(alias,8,reinterpret_cast<uintptr_t>(portrait));
    put_role(obj,8,reinterpret_cast<uintptr_t>(custom169));
    telemetry.mode=0;
    put_role(phase,0x14,3u);preload_cards();
    require(card_preload.next==1&&card_preload.units.size()==1&&!measured(),"Deployment did not warm cards before possession");++checks;
    require(std::filesystem::remove(art),"Card fixture cleanup");
    for(auto dir=art.parent_path();dir!=folder;dir=dir.parent_path())require(std::filesystem::remove(dir),"Card fixture directory cleanup");
    require(std::filesystem::remove(folder),"Card fixture root cleanup");
    put_role(phase,0x14,5u);telemetry.mode=POSSESSED;
    require(measured()&&display.native_card217&&display.native_card217->pixels[2]==73,"Possession did not display preloaded custom card");++checks;
    put_role(phase,0x14,4u);preload_cards();
    require(card_preload.units.empty(),"Previous roster persisted outside battle");++checks;
    VirtualFree(image,0,MEM_RELEASE);printf("PASS production HUD measurement: 10 roles, %u checks\n",checks);return 0;
}catch(const std::exception& e){fprintf(stderr,"%s\n",e.what());return 1;}}
