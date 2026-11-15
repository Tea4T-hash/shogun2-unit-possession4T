#include "adapter.cpp"
static uintptr_t fixture_projectile81;static bool fixture_kneel81=true;
static unsigned fixture_changes81,fixture_mode81;
static void* __fastcall fixture_projectile_get81(void*,void*){return reinterpret_cast<void*>(fixture_projectile81);}
static bool __fastcall fixture_ability81(void*,void*,unsigned id){return id==57&&fixture_kneel81;}
static void __fastcall fixture_stop81(void*,void*){}
static void __fastcall fixture_change81(void* actor,void*,unsigned mode,unsigned,unsigned){++fixture_changes81;fixture_mode81=mode;field<unsigned>(reinterpret_cast<uintptr_t>(actor),0x1f0)=mode;}
static void jump81(unsigned char* destination,const void* target){destination[0]=0xe9;auto relative=uint32_t(reinterpret_cast<uintptr_t>(target)-reinterpret_cast<uintptr_t>(destination)-5);memcpy(destination+1,&relative,4);}
int main(){void* memory=nullptr;try{
    memory=VirtualAlloc(nullptr,0x1d88000,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);require(memory!=nullptr,"Fixture allocation failed");base=reinterpret_cast<uintptr_t>(memory);
    jump81(reinterpret_cast<unsigned char*>(base+0x14ad40),&fixture_projectile_get81);
    jump81(reinterpret_cast<unsigned char*>(base+0x15c530),&fixture_ability81);
    jump81(reinterpret_cast<unsigned char*>(base+0x3ca920),&fixture_stop81);
    std::vector<unsigned char> actor_bytes(0x800),unit_bytes(0x1100),projectile_bytes(0x200),database_bytes(0x30),clip_bytes(8),asset_bytes(16);
    std::vector<uintptr_t> clips(0x4a3);possessed=reinterpret_cast<uintptr_t>(actor_bytes.data());unit=reinterpret_cast<uintptr_t>(unit_bytes.data());fixture_projectile81=reinterpret_cast<uintptr_t>(projectile_bytes.data());
    Status fixture{};state=&fixture;state->mode=POSSESSED;state->lease_generation=42;state->actor_id=923;state->unit_id=9;
    possession_lease.active=true;possession_lease.generation=42;possession_lease.identity.actor=possessed;possession_lease.identity.actor_id=923;possession_lease.identity.unit=unit;possession_lease.identity.unit_id=9;
    field<unsigned>(possessed,0x1c8)=923;field<unsigned char>(possessed,0xf0)=1;field<unsigned>(possessed,0x1f0)=8;field<unsigned>(unit,4)=9;
    field<float>(base,0x15b2448)=.04f;field<unsigned>(unit,0xf7c)=10;
    field<unsigned>(fixture_projectile81,0)=60;field<unsigned>(fixture_projectile81,0x60)=100;field<float>(fixture_projectile81,0x6c)=200;
    auto db=reinterpret_cast<uintptr_t>(database_bytes.data());field<uintptr_t>(possessed,0x23c)=db;field<uintptr_t>(db,0xc)=reinterpret_cast<uintptr_t>(clips.data());
    // Native valid_weapon_clip checks both index and a non-null asset.
    std::vector<unsigned char> records(4*8);unsigned ids[]={106,127,132,138};
    for(unsigned i=0;i<4;++i){auto r=reinterpret_cast<uintptr_t>(records.data()+i*8);field<unsigned>(r,0)=ids[i];field<uintptr_t>(r,4)=reinterpret_cast<uintptr_t>(asset_bytes.data());clips[ids[i]]=r;}
    original_change=reinterpret_cast<ChangeState>(&fixture_change81);
    equipment_requests81={42,0,0,true};require(equipment_prepare_motion81()&&equipment_crouch_supported81&&fixture_mode81==9,"Ctrl failed to select native kneeling posture");
    unsigned changes=fixture_changes81;state->weapon_phase=WEAPON_DRAW;equipment_requests81.ctrl=false;
    require(equipment_prepare_motion81()&&fixture_changes81==changes,"Ctrl release interrupted committed shot");
    state->weapon_phase=0;require(!equipment_prepare_motion81()&&fixture_mode81==8,"Ctrl release failed to stand");
    field<unsigned>(possessed,0x1f0)=9;require(!equipment_prepare_motion81()&&fixture_mode81==8,"Native unit kneeling left personal walking in a kneeling pose");
    fixture_kneel81=false;equipment_requests81.ctrl=true;changes=fixture_changes81;
    require(!equipment_prepare_motion81()&&!equipment_crouch_supported81&&fixture_changes81==changes,"Locked kneeling ability granted");fixture_kneel81=true;
    auto reload=clips[138];clips[138]=0;require(!equipment_prepare_motion81()&&!equipment_crouch_supported81,"Missing kneel reload accepted");clips[138]=reload;
    equipment_requests81.lease=41;require(!equipment_prepare_motion81(),"Old lease Ctrl request accepted");equipment_requests81.lease=42;
    melee_handoff78.observe(possessed,42,923,9,7);changes=fixture_changes81;
    require(!equipment_prepare_motion81()&&fixture_changes81==changes,"Crouching interrupted melee handoff");melee_handoff78.reset();
    require(equipment_prepare_motion81(),"Crouching did not resume at safe boundary");
    equipment_requests81.slot=equipment81::Melee;field<unsigned char>(unit,0xead)=1;equipment_mode_poll131(equipment_requests81,true);require(!equipment_prepare_motion81()&&equipment_slot81==equipment81::Melee&&fixture_mode81==10,"Melee switching failed to stand and equip native melee pose");
    equipment_requests81.ctrl=false;equipment_requests81.slot=equipment81::Ranged;field<unsigned char>(unit,0xead)=0;equipment_mode_poll131(equipment_requests81,true);require(!equipment_prepare_motion81()&&equipment_slot81==equipment81::Ranged&&fixture_mode81==8,"Ranged switching failed");
    equipment_requests81.slot=0;field<unsigned char>(unit,0xead)=1;
    require(!equipment_prepare_motion81()&&equipment_slot81==equipment81::Melee&&fixture_mode81==10,"Actual native melee mode without private request was ignored");
    field<unsigned char>(unit,0xead)=0;
    require(!equipment_prepare_motion81()&&equipment_slot81==equipment81::Ranged&&fixture_mode81==8,"Actual native ranged mode without private request was ignored");
    possession_lease.identity.mount=1;equipment_requests81.ctrl=true;changes=fixture_changes81;require(!equipment_prepare_motion81()&&!equipment_crouch_supported81&&fixture_changes81==changes,"Mounted soldier crouched");possession_lease.identity.mount=0;
    equipment_reset81();require(!equipment_crouch81&&!equipment_crouch_supported81&&!equipment_lease81&&!equipment_requests81.ctrl&&!equipment_requests81.slot&&!equipment_requests81.tabs,"Release retained equipment requests");
    // A custom pack can keep the actor's normal clips and replace projectile
    // IDs, keys, velocity and range. The in-memory constructor layout is the
    // source for these fixtures, not a guess based on any projectile's name.
    auto string=[&](uintptr_t dst,const auto* text,unsigned n){
        field<unsigned>(dst,0)=n;field<unsigned>(dst,4)=n;
        field<uintptr_t>(dst,8)=reinterpret_cast<uintptr_t>(text);
    };
    const auto record=fixture_projectile81;
    static const wchar_t custom[]=L"custom_mixed_unit_unknown";
    static const wchar_t arrow[]=L"arrow",gun[]=L"bullet",shell[]=L"cannon_ball";
    static const char bomb[]="grenade";
    std::array<unsigned char,64> shot_type{};
    field<unsigned>(record,0)=812;
    field<unsigned>(record,0x60)=207;
    field<float>(record,0x6c)=123.5f;
    string(record+4,custom,unsigned(std::wcslen(custom)));
    string(record+0x28,arrow,5);
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Unresolved custom descriptor accepted");
    field<uintptr_t>(record,0x24)=reinterpret_cast<uintptr_t>(shot_type.data());
    field<unsigned>(record,0x10)=1;
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Artillery accepted as personal weapon");
    field<unsigned>(record,0x10)=0;
    require(native_personal_kind(record)==PersonalWeaponKind::Bow&&bow_record(unit)==record,"Custom native bow rejected");
    ammo83::Item item{};
    ammunition_item83(record,item);
    require(item.icon==ammo83::Arrow,"Custom bow did not get its generic arrow icon");
    string(record+0x28,gun,6);
    require(native_personal_kind(record)==PersonalWeaponKind::Firearm,"Custom native firearm rejected");
    item={};ammunition_item83(record,item);
    require(item.icon==ammo83::Bullet,"Custom firearm did not get its generic bullet icon");
    // Two rows from the enabled Workshop packs, neither in the old key list.
    static const wchar_t matchlock[]=L"shogun_matchlock_long",pistoleers[]=L"pistoleers";
    string(record+4,matchlock,unsigned(std::wcslen(matchlock)));
    field<unsigned>(record,0x60)=225;field<float>(record,0x6c)=300;
    require(native_personal_kind(record)==PersonalWeaponKind::Firearm,"Ultimate Field Command long matchlock rejected");
    string(record+4,pistoleers,unsigned(std::wcslen(pistoleers)));
    field<unsigned>(record,0x60)=60;field<float>(record,0x6c)=200;
    require(native_personal_kind(record)==PersonalWeaponKind::Firearm,"Expanded Japan pistoleers rejected");
    static const wchar_t rocket[]=L"rocket",bohiya[]=L"shogun_bohiya",rocket_model[]=L"projectile_rocket";
    static const wchar_t mortar[]=L"shogun_handmortar",rockets[]=L"shogun_rocket";
    static const char fragment_model[]="projectile_fragment01";
    const auto shot=reinterpret_cast<uintptr_t>(shot_type.data());
    static const char fire_lance[]="ming_rocket_firelance",fragment[]="fragment",shrapnel[]="shrapnel",fougasse[]="fougasse";
    field<unsigned>(record,0x10)=1;field<unsigned>(record,0x60)=200;field<float>(record,0x6c)=45.f;
    field<unsigned>(record,0x4c)=1;
    string(record+4,bohiya,13);string(record+0x18,rocket_model,17);string(record+0x28,rocket,6);string(shot,rocket,6);
    bool typed=false;
    require(native_personal_kind(record,&typed)==PersonalWeaponKind::Thrown&&typed&&bow_record(unit)==record&&
            launcher_sight123(record)==LauncherSight123::Rocket,"UEFC Bo-Hiya did not equip its portable launcher");
    item={};ammunition_item83(record,item);require(item.icon==ammo83::Rocket,"Bo-Hiya ammunition was not recognized as rockets");
    string(record+4,custom,unsigned(std::wcslen(custom)));
    require(native_personal_kind(record)==PersonalWeaponKind::Thrown&&launcher_sight123(record)==LauncherSight123::Rocket,
            "Renamed portable launcher was not recognized");
    field<unsigned>(record,0x10)=3;
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported&&launcher_sight123(record)==LauncherSight123::None,
            "Naval-category launcher was admitted");
    field<unsigned>(record,0x10)=1;
    string(record+4,mortar,17);
    require(native_personal_kind(record)==PersonalWeaponKind::Thrown&&launcher_sight123(record)==LauncherSight123::Mortar,
            "Stock hand mortar lost its weapon family");
    string(record+4,rockets,13);
    require(native_personal_kind(record)==PersonalWeaponKind::Thrown&&launcher_sight123(record)==LauncherSight123::Rocket,
            "Stock fire rockets lost their weapon family");
    field<unsigned>(record,0x10)=0;field<unsigned>(record,0x60)=50;field<float>(record,0x6c)=50.f;
    string(record+4,fire_lance,sizeof fire_lance-1);string(record+0x18,fragment_model,sizeof fragment_model-1);
    string(record+0x28,shrapnel,8);string(shot,fragment,8);
    field<unsigned>(record,0x4c)=30;
    require(native_personal_kind(record,&typed)==PersonalWeaponKind::Firearm&&typed&&bow_record(unit)==record,
            "UEFC Korean Pirates did not equip their native musket-rig fire lance");
    item={};ammunition_item83(record,item);require(item.icon==ammo83::Bullet,"Fire lance inherited the rocket icon from its name");
    string(record+4,custom,unsigned(std::wcslen(custom)));
    require(native_personal_kind(record)==PersonalWeaponKind::Firearm,"Renamed fire lance was not recognized");
    field<unsigned>(record,0x10)=1;
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Crew artillery shrapnel admitted");
    field<unsigned>(record,0x10)=0;field<unsigned>(record,0x4c)=1;
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"A lone fragment was treated as a volley");
    field<unsigned>(record,0x4c)=30;string(shot,fougasse,8);
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Impact fragments admitted as a personal weapon");
    string(record+4,custom,unsigned(std::wcslen(custom)));
    string(record+0x28,bomb,7);
    require(native_personal_kind(record)==PersonalWeaponKind::Thrown,"Custom native grenade rejected");
    string(record+0x28,shell,11);
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Cannon shell treated as personal firearm");
    field<unsigned char>(unit,0xead)=0;
    field<unsigned>(possessed,0x1f0)=8;
    equipment_prepare_motion81();
    require(!equipment_personal152&&equipment_slot81==equipment81::Melee&&bow_family142<0,
            "Unknown initial record became an equipped ranged weapon");
    string(record+0x28,gun,6);
    state->weapon_phase=WEAPON_DRAW;
    equipment_prepare_motion81();
    require(!equipment_personal152&&equipment_slot81==equipment81::Melee,"Changed equipment interrupted a committed shot");
    state->weapon_phase=0;
    equipment_prepare_motion81();
    require(equipment_personal152&&equipment_slot81==equipment81::Ranged&&bow_family142==0&&equipment_ranged_selected81(),
            "Possession did not refresh to the newly recognized firearm");
    field<unsigned char>(unit,0xead)=1;state->aiming=1;
    equipment_prepare_motion81();
    require(equipment_personal_override214&&equipment_ranged_selected81(),
            "Custom firearm could not enter personal aim while its formation used melee");
    field<unsigned>(record+0x28,4)=2;
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Malformed missile type accepted");
    string(record+0x28,gun,6);
    field<float>(record,0x6c)=std::numeric_limits<float>::quiet_NaN();
    require(native_personal_kind(record)==PersonalWeaponKind::Unsupported,"Invalid custom velocity accepted");
    VirtualFree(memory,0,MEM_RELEASE);memory=nullptr;
    std::cout<<"PASS: production equipment boundary selects actual native kneel/stand/melee modes; shot/handoff/old lease/unlocked ability/missing clip/mounted guards; release clears state. Synthetic objects only, game untouched.\n";return 0;
}catch(const std::exception& e){if(memory)VirtualFree(memory,0,MEM_RELEASE);std::cerr<<e.what()<<'\n';return 1;}}
