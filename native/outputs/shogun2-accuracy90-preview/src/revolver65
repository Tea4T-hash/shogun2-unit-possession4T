#pragma once
#include <algorithm>
#include <cstdint>
#include <cwchar>

// The game's native magazine is actor+394. The reload scheduler takes the
// capacity only when the cylinder is empty; the current count remains native.
static bool revolver_record65(uintptr_t record){
    if(!record)return false;
    const auto length=field<unsigned>(record,4),capacity=field<unsigned>(record,8);
    const auto text=field<uintptr_t>(record,0xc);
    if(text<65536u||capacity<length||capacity>1048576)return false;
    for(const auto* key:{L"boshin_revolver",L"boshin_officer_revolver"}){
        if(length!=std::wcslen(key))continue;
        const bool wide=field<unsigned char>(text,1)==0;
        bool match=true;
        for(unsigned i=0;i<length;++i){
            const wchar_t c=wide?field<wchar_t>(text,i*2):field<unsigned char>(text,i);
            if(c!=key[i]){match=false;break;}
        }
        if(match)return true;
    }
    return false;
}
using NativeReloadSchedule65=void(__thiscall*)(void*,float,unsigned);
static void schedule_personal_reload65(NativeReloadSchedule65 native,uintptr_t actor,uintptr_t record,float duration){
    const bool revolver=revolver_record65(record);
    native(reinterpret_cast<void*>(actor),revolver?std::min(300.f,duration+3.f):duration,
           revolver?6u:field<unsigned>(record,0xa0));
}
static bool reload_after_recoil65(uintptr_t actor,uintptr_t record){
    return !revolver_record65(record)||field<unsigned>(actor,0x394)==0;
}
