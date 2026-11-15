#pragma once
// Preserve native melee readiness without replacing ordinary locomotion.
// Test138 leaves native modes 0/4 alone after the user rejected their combat gait.
// Readiness changes retain native state/animation handling and never force a clip.
namespace meleedisplay137 {
inline bool ready_request(unsigned mode,bool mounted){return mounted?mode==29:(mode==8||mode==9);}
inline unsigned ready(bool mounted){return mounted?30u:10u;}
}
static bool melee_presentation_owned137(uintptr_t actor){
    __try {
        if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||actor!=possessed||
           equipment_slot81!=equipment81::Melee||equipment_personal_override214||!field<unsigned char>(unit,0xead)||
           field<unsigned char>(actor,0xf0)!=1||native_airborne_subject99(actor)||!followed(actor,false)||
           !personal_weapon_mode(field<unsigned>(actor,0x1f0),weapon_mounted()))return false;
        // Unlike firing eligibility, presentation must survive empty ammunition.
        auto record=reinterpret_cast<uintptr_t>(reinterpret_cast<NativeGet>(base+0x14ad40)(reinterpret_cast<void*>(unit)));
        if(native_personal_kind(record)==PersonalWeaponKind::Unsupported)return false;
        if(!weapon_mounted()&&!valid_weapon_clip(actor,109))return false;
        // Attached traversal and matched native combat own their authored state.
        return !field<uintptr_t>(actor,0x18)&&!field<uintptr_t>(actor,0x688);
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static unsigned melee_presentation_mode137(uintptr_t actor,unsigned mode,unsigned attached){
    if(!attached&&meleedisplay137::ready_request(mode,weapon_mounted())&&melee_presentation_owned137(actor))
        return meleedisplay137::ready(weapon_mounted());
    return mode;
}
