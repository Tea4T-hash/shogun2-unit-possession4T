#pragma once
// Compatibility filename/helper retained. Test105 extends the old asset-specific
// sling exception to the owned infantry ready/draw/fire lifecycle. Recorded
// firearm failures use the same reset-mode return RVA3e93c8 at frame zero.
// Protect only its animation stop, never the rest of the native reset routine.
// Native clock completion, explicit stops, recovery, paired combat, ownership
// loss and other actors still forward. No timer, replay or synthetic release.
static bool protect_throw_stop83(uintptr_t actor,uintptr_t caller){
    __try {
        if(caller!=base+0x3e93c8||own_weapon_call||!weapon_owned(actor)||weapon_mounted()||
           !equipment_ranged_selected81()||!weapon_ready_held()||field<uintptr_t>(actor,0x688)||
           native_recovery103(actor))return false;
        unsigned mode=field<unsigned>(actor,0x1f0);if(mode!=14&&mode!=15)return false;
        auto animation=field<uintptr_t>(actor,0x474);if(!animation)return false;
        unsigned id=field<unsigned>(animation,0);
        if(id>=0x4a3||field<unsigned>(actor,0x1d4)!=id)return false;
        bool hold=!state->weapon_phase&&state->aim_pose&&
            (id==(mode==15?127u:124u)||(mode==14&&state->aim_pose==AIM_BOW_DRAWING&&id==126));
        bool firing=actor==weapon_actor&&weapon_clip==id&&
            (state->weapon_phase==WEAPON_DRAW||state->weapon_phase==WEAPON_FIRED);
        if(!hold&&!firing)return false;
        auto db=field<uintptr_t>(actor,0x23c),array=db?field<uintptr_t>(db,0xc):0;
        if(!array||field<uintptr_t>(array,id*4)!=animation)return false;
        auto active_asset=field<uintptr_t>(animation,4);
        if(!active_asset)return false;
        auto frames=field<unsigned>(active_asset,0x64);
        return frames&&frames<=30000&&field<unsigned>(actor,0x1d8)<frames;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
