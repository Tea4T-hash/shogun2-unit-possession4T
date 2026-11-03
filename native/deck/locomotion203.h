#pragma once
// 366a10 computes the native motion driver AFTER367600. Update the original
// pose classifier only after that result exists, only for our completed intent.
static UnitTick130 original_motion_complete203;
static __declspec(thread) uintptr_t motion_completed203;
static __declspec(thread) unsigned motion_gait203;
// These are stock standing-idle categories, resolved from the pinned engine's
// animation-name table and owned-game fragments. Never cancel arbitrary clips.
static bool locomotion_idle204(unsigned clip){
    return (clip>=141&&clip<=157)||(clip>=205&&clip<=209)||
        (clip>=212&&clip<=216)||(clip>=219&&clip<=223)||(clip>=226&&clip<=230);
}
static bool locomotion_ordinary203(uintptr_t actor){
    // The horse integrates movement, then its original update attaches the rider.
    // A mounted rider's pose must never be classified as infantry locomotion.
    if(actor!=locomotion_actor()||!member_owned130(actor))return false;
    if(possession_lease.identity.mount){
        const auto rider_mode=field<unsigned>(possessed,0x1f0);
        if(native_mounted_jump140(actor)||(rider_mode!=29&&rider_mode!=30)||
           field<uintptr_t>(possessed,0x688)||field<uintptr_t>(possessed,0x474)||
           field<unsigned char>(possessed,0x488)||field<unsigned char>(possessed,0x489))return false;
    }
    return
        field<unsigned>(actor,0x1f0)<=7&&!field<uintptr_t>(actor,0x18)&&
        !field<uintptr_t>(actor,0x688)&&!field<unsigned char>(actor,0x488)&&!field<unsigned char>(actor,0x489)&&
        !wall_subject97(actor)&&!native_airborne_subject99(actor)&&!native_recovery103(actor)&&
        !charge_input_locked146()&&!charge_native_task146(actor)&&!state->aiming&&!state->aim_pose&&!state->weapon_phase;
}
static void locomotion_mark203(uintptr_t actor){
    __try{if(locomotion_ordinary203(actor)){
        motion_completed203=actor;
        motion_gait203=native_sprint_requested133(unsigned(state->input))&&reinterpret_cast<UnitRunAllowed>(base+0x15cb80)(reinterpret_cast<void*>(unit))?1u:0u;
    }}__except(EXCEPTION_EXECUTE_HANDLER){motion_completed203=0;}
}
static void __fastcall motion_complete203(void* object,void*){
    if(!possession_lease.active||reinterpret_cast<uintptr_t>(object)!=locomotion_actor()){original_motion_complete203(object);return;}
    const auto saved=motion_completed203;const auto saved_gait=motion_gait203;motion_completed203=0;
    InterlockedIncrement(&motion_callback_depth);
    __try{
        original_motion_complete203(object);
        const auto actor=reinterpret_cast<uintptr_t>(object);
        __try{if(motion_completed203==actor&&locomotion_ordinary203(actor)){
            const float speed=field<float>(actor,0x1ac);
            if(!std::isfinite(speed)||speed<0)return;
            const bool moving=speed>=.01f;
            auto clip=field<uintptr_t>(actor,0x474);
            if(clip){
                // A moving soldier can still receive formation standing idles.
                // Use the same interruptibility check/cleanup as native movement
                // (3de710 ->3dd9b0); do not clear clip pointers by hand.
                const unsigned id=field<unsigned>(clip,0);
                auto db=field<uintptr_t>(actor,0x23c);
                auto clips=db?field<uintptr_t>(db,0xc):0;
                if(!moving||!locomotion_idle204(id)||!clips||
                   field<unsigned>(actor,0x1d4)!=id||field<uintptr_t>(clips,id*4)!=clip)return;
                reinterpret_cast<UnitTick130>(base+0x3dd9b0)(object);
                if(field<uintptr_t>(actor,0x474))return;
            }
            // 3dd9d0 also requires planner intent+150, which can be zero after
            // integration while our soldier is STILL moving. Use its original
            // two pose routines with the completed physical motion instead.
            // No velocity, intent, position, state or animation clock is written.
            field<unsigned>(actor,0x244)=motion_gait203;
            reinterpret_cast<UnitTick130>(base+0x3e4850)(object);
            if(moving)reinterpret_cast<UnitTick130>(base+0x3e44f0)(object);
        }}__except(EXCEPTION_EXECUTE_HANDLER){}
    }__finally{motion_completed203=saved;motion_gait203=saved_gait;InterlockedDecrement(&motion_callback_depth);}
}
static void create_locomotion203(){
    const unsigned char signature[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57,0x80,0xbe,0x99,1,0,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x366a10),signature,sizeof signature),"Native motion completion differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x366a10),reinterpret_cast<void*>(&motion_complete203),reinterpret_cast<void**>(&original_motion_complete203))==MH_OK,"Native motion completion hook failed");
}
