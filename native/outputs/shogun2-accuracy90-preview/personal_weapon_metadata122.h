#pragma once
#include "personal_weapon_catalog.h"
#include "personal_weapon_profile.h"
#include "equipment_protocol81.h"

static bool projectile_type_is122(uintptr_t name,const char* expected,unsigned size){
    const unsigned length=field<unsigned>(name,0),capacity=field<unsigned>(name,4);
    const auto data=field<uintptr_t>(name,8);
    if(data<65536||length!=size||capacity<length||capacity>1048576)return false;
    const bool wide=field<unsigned char>(data,1)==0;
    for(unsigned i=0;i<size;++i){
        const unsigned c=wide?field<wchar_t>(data,i*sizeof(wchar_t)):field<unsigned char>(data,i);
        if(c!=static_cast<unsigned char>(expected[i]))return false;
    }
    return true;
}
static bool resolved_shot_is124(uintptr_t projectile,const char* key,unsigned length){
    // c8a600 resolves the shot enum into +24. c8adc0 copies the
    // shot's key to offset zero of the resolved record. Projectile
    // +18 is the optional model string, not the shot type.
    const auto shot=field<uintptr_t>(projectile,0x24);
    return shot>=65536&&projectile_type_is122(shot,key,length);
}
static LauncherSight123 portable_launcher_sight124(uintptr_t record){
    // Native hand-held launchers use the artillery projectile category too.
    // Personal control still requires the soldier's equipped record and clips.
    if(field<unsigned>(record,0x10)!=1||field<unsigned>(record,0x4c)!=1)
        return LauncherSight123::None;
    const auto missile=record+0x28;
    if(projectile_type_is122(missile,"rocket",6)&&resolved_shot_is124(record,"rocket",6))
        return LauncherSight123::Rocket;
    if(projectile_type_is122(missile,"chakram",7)&&resolved_shot_is124(record,"percussive_shell",16))
        return LauncherSight123::Mortar;
    return LauncherSight123::None;
}
static LauncherSight123 launcher_sight123(uintptr_t record){
    __try {
        if(!record)return LauncherSight123::None;
        if(projectile_type_is122(record+4,"shogun_handmortar",17))return LauncherSight123::Mortar;
        if(projectile_type_is122(record+4,"shogun_rocket",13))return LauncherSight123::Rocket;
        return portable_launcher_sight124(record);
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    return LauncherSight123::None;
}
static PersonalWeaponKind native_personal_kind(uintptr_t record,bool* from_type=nullptr){
    if(from_type)*from_type=false;
    __try {
        if(!record)return PersonalWeaponKind::Unsupported;
        const unsigned id=field<unsigned>(record,0),range=field<unsigned>(record,0x60);
        const float speed=field<float>(record,0x6c);
        const unsigned length=field<unsigned>(record,4),capacity=field<unsigned>(record,8);
        const auto text=field<uintptr_t>(record,0xc);
        if(text>=65536&&length&&length<=128&&capacity>=length&&capacity<=1048576){
            // Keep the verified exceptions, including stock hand mortars and rockets.
            const auto known=field<unsigned char>(text,1)==0?
                personal_key_kind(reinterpret_cast<const wchar_t*>(text),length,capacity,range,speed):
                personal_ascii_key_kind(reinterpret_cast<const char*>(text),length,capacity,range,speed);
            if(known!=PersonalWeaponKind::Unsupported)return known;
        }
        // The original c8a600 constructor stores category at +10, a
        // resolved shot type at +24, and the declared missile type at +28.
        // A pack can change the projectile key, row index, velocity or range.
        // The loaded type still says what the soldier is actually using.
        const auto category=field<unsigned>(record,0x10);
        if(field<uintptr_t>(record,0x24)>=65536&&range&&range<=10000&&
           std::isfinite(speed)&&speed>0&&speed<=10000){
            if(category!=0){
                if(portable_launcher_sight124(record)==LauncherSight123::None)return PersonalWeaponKind::Unsupported;
                if(from_type)*from_type=true;
                return PersonalWeaponKind::Thrown;
            }
            const auto missile=record+0x28;
            // Native 41c760 registers every pellet in a fragment volley.
            // Require the loaded shot type as well as the missile category;
            // impact fragments and artillery shrapnel are not infantry guns.
            const auto count=field<unsigned>(record,0x4c);
            const bool fire_lance=projectile_type_is122(missile,"shrapnel",8)&&
                resolved_shot_is124(record,"fragment",8)&&count>1&&count<=128;
            const auto kind=projectile_type_is122(missile,"arrow",5)?PersonalWeaponKind::Bow:
                (projectile_type_is122(missile,"bullet",6)||fire_lance)?PersonalWeaponKind::Firearm:
                projectile_type_is122(missile,"grenade",7)?PersonalWeaponKind::Thrown:PersonalWeaponKind::Unsupported;
            if(from_type)*from_type=kind!=PersonalWeaponKind::Unsupported;
            return kind;
        }
        // Keep the original numeric profiles when type metadata is absent.
        if(!field<uintptr_t>(record,0x24)&&!length&&personal_projectile_supported(id,range,speed,false))
            return id==3?PersonalWeaponKind::Bow:PersonalWeaponKind::Firearm;
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    return PersonalWeaponKind::Unsupported;
}
