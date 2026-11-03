#pragma once
#include "view152.h"
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "pose_geometry.h"
#include "self_visibility100.h"
#include "seated_weapon_geometry.h"
#include "first_person_presentation101.h"
static volatile LONG infantry_eye_updates101,infantry_eye_rejects101;
#include "arrow_render_geometry.h"
#include "mounted_hold_geometry.h"
#include "mounted_settle_geometry88.h"
#include "decoded_rig_profiles89.h"
static volatile LONG rig_layout89,rig_rejections89;
#include "mounted_transition_geometry.h"
#include "mounted_muzzle_calibration.h"
static mountedge::Cache mounted_transition71;
static volatile LONG mounted_transition_updates71,mounted_transition_blends71,mounted_transition_rejections71;
static volatile LONG mounted_carbine_predraw_blocks72;
#include "mounted_firearm_geometry.h"
static heldpose::Cache mounted_held_palette;
static heldpose::Acquisition88 mounted_acquisition88;
static volatile LONG mounted_acquisition_waits88;
static volatile LONG mounted_palette_holds,mounted_palette_failures;
using SkinReady=void(__thiscall*)(void*,void*);
static SkinReady original_skin_ready;
static uintptr_t pose_holder,pose_result,pose_array;
static unsigned pose_count;
static float pose_before[256*12],pose_after[256*12];
static bool pose_saved;
static uintptr_t pose_binding100,pose_asset100;
// Bounded numeric snapshots only. The logger never dereferences game objects.
struct MountedPoseAudit87 {uint64_t tick;unsigned lease,draw,frame,phase,held,bones[7],sim_tick88,acquired88,blend_ready88,settle_matches88,layout89,rejections89;float native[84],visible[84],world[12];};
static MountedPoseAudit87 mounted_pose_audit87[32];
static unsigned mounted_pose_sequence87;static uint64_t mounted_pose_audit_tick87;

static SRWLOCK pose_lock=SRWLOCK_INIT;
static uintptr_t pose_actor,pose_environment;
static uint32_t pose_actor_id,pose_unit_id;
static uintptr_t pose_unit,pose_config;
static uint64_t pose_tick;
static float pose_world[12],pose_pivot[3],pose_arrow_world[3];
static bool pose_arrow_valid;
static bool pose_carbine_valid;
static float pose_carbine_world[3];
static float pose_muzzle_transform[12];
static bool pose_muzzle_transform_valid;
static bool pose_infantry_pitch101;
static bool carbine_socket_profile(uintptr_t actor,uintptr_t binding,unsigned clip){
    if(clip!=758)return false;
    if(!mounted_carbine_native_profile(actor))return false;
    // The live native binding record has identity Weapon1 bind translation,
    // scale and quaternion. Reject retargeted/modded binds instead of guessing.
    auto bone=field<uintptr_t>(binding,0x90);if(!bone||field<unsigned>(bone,4)!=1)return false;
    for(unsigned i=0;i<3;++i){float t=field<float>(bone,8+i*4),q=field<float>(bone,24+i*4);
        if(!std::isfinite(t)||!std::isfinite(q)||std::abs(t)>.0001f||std::abs(q)>.0001f)return false;}
    float scale=field<float>(bone,20),w=field<float>(bone,36);
    return std::isfinite(scale)&&std::isfinite(w)&&std::abs(scale-1)<.001f&&std::abs(w-1)<.001f;
}
static bool mounted_spine(uintptr_t asset,unsigned n,const int32_t* parents,unsigned& spine){
    auto hierarchy=field<uintptr_t>(asset,0x40);if(!hierarchy)return false;
    unsigned hips=n;spine=n;
    for(unsigned i=0;i<n;++i){auto record=hierarchy+i*20;auto length=field<unsigned>(record,0),text=field<uintptr_t>(record,12);
        if(!text||length>128)return false;
        if(length==5&&!memcmp(reinterpret_cast<void*>(text),L"Spine",10))spine=i;
        if(length==4&&!memcmp(reinterpret_cast<void*>(text),L"Hips",8))hips=i;
    }
    return spine<n&&hips<n&&parents[spine]==int32_t(hips)&&parents[hips]<0;
}
static uintptr_t death_pose_holder,death_pose_binding,death_pose_asset;
static uint32_t death_pose_lease;
static deathpose::Calibration death_pose_calibration;
static bool death_pose_calibrated;
static void capture_model_eye(uintptr_t holder,uintptr_t result){
    if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||!possession_lease.active)return;
    __try{
        const auto& id=possession_lease.identity;
        auto root=field<uintptr_t>(base,0x1bc8180);if(!root||field<uintptr_t>(root,0x110)!=id.environment)return;
        auto linked=field<uintptr_t>(holder,8),binding=field<uintptr_t>(holder,0x1e4);
        const bool dying=dead_view()||(linked==id.actor&&field<unsigned char>(linked,0xf0)==0);
        if(!binding)return;auto asset=field<uintptr_t>(binding,0x1c);if(!asset)return;
        if(dying){
            if(!death_pose_calibrated||death_pose_lease!=possession_lease.generation||holder!=death_pose_holder||binding!=death_pose_binding||asset!=death_pose_asset||death_elapsed()>=4500)return;
            if(linked&&linked!=id.actor)return;
        }else if(linked!=id.actor)return;
        // A surviving link must still identify the same actor. A cleared link
        // is allowed only on the previously captured holder after proven death.
        if(linked&&(!actor152::human(base,field<uintptr_t>(linked,0))||field<uint32_t>(linked,0x1c8)!=id.actor_id||field<uintptr_t>(linked,0x214)!=id.unit))return;
        auto n=field<unsigned>(result,4);auto array=field<uintptr_t>(result,8),bone=field<uintptr_t>(binding,0x9c);
        if(!array||!bone||!n||n>256||n!=field<unsigned>(asset,0x3c))return;
        auto index=field<unsigned>(bone,4);if(index>=n)return;
        const float* matrices=reinterpret_cast<float*>(array);
        if(pose_saved&&holder==pose_holder&&result==pose_result&&array==pose_array&&n==pose_count&&(!pose_binding100||(binding==pose_binding100&&asset==pose_asset100))&&memcmp(matrices,pose_after,n*48)==0)matrices=pose_before;
        deathpose::Frame head;if(!deathpose::frame(reinterpret_cast<float*>(holder+0x24),matrices+index*12,head))return;
        // Calibrate the face from the untouched native eye, before free mouse
        // look replaces its direction. Death must not retain a backward glance.
        if(!dying){ModelEye owned;if(!read_model_eye(native_eye_channel78,owned)||!leased_model_eye(owned)||GetTickCount64()-owned.tick>250||!deathpose::calibrate(head,owned.eye,death_pose_calibration))return;
            death_pose_holder=holder;death_pose_binding=binding;death_pose_asset=asset;death_pose_lease=possession_lease.generation;death_pose_calibrated=true;}
        ModelEye sample;sample.lease=possession_lease.generation;sample.actor=id.actor_id;sample.unit=id.unit_id;sample.tick=GetTickCount64();
        if(!deathpose::follow(head,death_pose_calibration,sample.eye))return;
        ModelEye prior;if(dying&&read_model_eye(model_eye_channel,prior)&&leased_model_eye(prior)){
            float delta[3];for(unsigned i=0;i<3;++i)delta[i]=sample.eye.view[i]-prior.eye.view[i];
            if(deathpose::dot(delta,delta)>64)return; // Reject replacement/teleport, not a normal fall.
        }
        publish_model_eye(model_eye_channel,sample);
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}
static void restore_soldier_pose(){
    AcquireSRWLockExclusive(&pose_lock);
    __try {
        uintptr_t root=field<uintptr_t>(base,0x1bc8180);
        if(pose_saved&&root&&field<uintptr_t>(root,0x110)==pose_environment&&pose_holder&&field<uintptr_t>(pose_holder,8)==pose_actor&&
           field<unsigned>(pose_actor,0x1c8)==pose_actor_id&&field<uintptr_t>(pose_actor,0x214)==pose_unit&&
           field<unsigned>(pose_unit,4)==pose_unit_id&&field<uintptr_t>(pose_actor,0x20c)==pose_config&&
           (!pose_binding100||(field<uintptr_t>(pose_holder,0x1e4)==pose_binding100&&field<uintptr_t>(pose_binding100,0x1c)==pose_asset100))&&
           field<uintptr_t>(field<uintptr_t>(pose_holder,0x14),0x14)==pose_result&&
           field<uintptr_t>(pose_result,8)==pose_array&&field<unsigned>(pose_result,4)==pose_count&&
           memcmp(reinterpret_cast<void*>(pose_array),pose_after,pose_count*48)==0){
            memcpy(reinterpret_cast<void*>(pose_array),pose_before,pose_count*48);++state->pose_restores;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){state->pose_error=3;}
    pose_saved=false;pose_arrow_valid=false;pose_carbine_valid=false;pose_muzzle_transform_valid=false;pose_infantry_pitch101=false;mounted_held_palette.reset();mounted_acquisition88.reset();mounted_transition71.reset();
    ReleaseSRWLockExclusive(&pose_lock);
}
static void apply_soldier_pose(uintptr_t holder,uintptr_t result){
    if(third_person152||!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||!possessed)return;
    __try {
        bool mounted=possession_lease.identity.mount!=0;bool climbing97=!mounted&&wall_subject97(possessed);
        if(field<unsigned char>(possessed,0xf0)!=1||field<uintptr_t>(holder,8)!=possessed||!followed(possessed,false)||
           (!mounted&&!climbing97&&field<unsigned>(possessed,0x1f0)>=18))return;
        pose_infantry_pitch101=false;
        melee60::Request commitment;bool combat_pose96=melee_request(commitment);if(mounted&&combat_pose96){mounted_transition71.reset();return;} // Native paired animation owns the complete rig.
        uintptr_t binding=field<uintptr_t>(holder,0x1e4);if(!binding)return;
        uintptr_t asset=field<uintptr_t>(binding,0x1c);if(!asset)return;
        unsigned n=field<unsigned>(result,4);uintptr_t array=field<uintptr_t>(result,8);
        if(!n||n>256||n!=field<unsigned>(asset,0x3c)||!array)return;
        if(pose_saved&&holder==pose_holder&&result==pose_result&&array==pose_array&&n==pose_count&&
            (!pose_binding100||(binding==pose_binding100&&asset==pose_asset100))&&
            memcmp(reinterpret_cast<void*>(array),pose_after,n*48)==0)memcpy(reinterpret_cast<void*>(array),pose_before,n*48);
        int32_t parents[256];uintptr_t hierarchy=field<uintptr_t>(asset,0x40);
        for(unsigned i=0;i<n;++i)parents[i]=field<int32_t>(hierarchy,i*20+16);
        const unsigned offsets[7]={0xa8,0xa4,0x9c,0xa0,0x90,0x94,0x98};unsigned indices[7];
        for(unsigned i=0;i<7;++i){uintptr_t bone=field<uintptr_t>(binding,offsets[i]);if(!bone)return;indices[i]=field<unsigned>(bone,4);if(indices[i]>=n)return;}
        uintptr_t projectile=bow_record(unit);if(!mounted&&!projectile&&!climbing97)return;
        unsigned record_id=projectile?field<unsigned>(projectile,0):0,native_clip=field<unsigned>(possessed,0x1d4);
        float pitch=weapon_pose_pitch(is_bow_projectile(projectile)?3:record_id,native_clip,state->pitch);if(!std::isfinite(pitch))return;
        // Identify the verified rifle from its catalog even while lowered.
        // The current actor clip can still describe the previous draw.
        bool carbine_profile=equipment_slot81==equipment81::Ranged&&mounted&&projectile&&!is_bow_projectile(projectile)&&carbine_socket_profile(possessed,binding,MOUNTED_FORWARD_LEVEL);
        // This callback hands model-space matrices to native attachments and skinning.
        // Camera evaluation uses a separate temporary array and is left intact.
        unsigned mounted_spine_index=0;
        if(mounted){
            rig89::Node nodes[256]{};wchar_t names[256][64]{};
            bool valid=n<=48;
            for(unsigned i=0;valid&&i<n;++i){auto record=hierarchy+i*20;
                unsigned length=field<unsigned>(record,0);auto text=field<uintptr_t>(record,12);
                if(!text||!length||length>=64){valid=false;break;}
                memcpy(names[i],reinterpret_cast<void*>(text),length*sizeof(wchar_t));
                nodes[i]={names[i],length,parents[i]};
            }
            auto layout=valid?rig89::classify(nodes,n):rig89::Layout::Unknown;
            InterlockedExchange(&rig_layout89,LONG(layout));
            if(!rig89::sockets(layout,indices)){
                InterlockedIncrement(&rig_rejections89);mounted_held_palette.reset();mounted_acquisition88.reset();
                pose_arrow_valid=pose_carbine_valid=pose_muzzle_transform_valid=false;return;
            }
            mounted_spine_index=7; // Profile proves Hips -> Spine and all named sockets.
        }
        memcpy(pose_before,reinterpret_cast<void*>(array),n*48);
        if(mounted&&mounted_clock_held(possessed)){
            heldpose::Key key{possessed,asset,binding,state->lease_generation,state->draws_started};
            if(mounted_bow_native_profile85(possessed)){
                bool paused=field<float>(base,0x15b2448)<.001f;
                auto result88=heldpose::apply_settled88(mounted_held_palette,mounted_acquisition88,key,
                    reinterpret_cast<float*>(array),n,parents,mounted_spine_index,indices+4,
                    mounted_bow_blend_ready88(),mounted_blend_tick88(),paused);
                if(result88==heldpose::SettleResult::Applied)InterlockedIncrement(&mounted_palette_holds);
                else if(result88==heldpose::SettleResult::Waiting)InterlockedIncrement(&mounted_acquisition_waits88);
                else InterlockedIncrement(&mounted_palette_failures);
            }else {
                mounted_acquisition88.reset();
                if(heldpose::apply(mounted_held_palette,key,reinterpret_cast<float*>(array),n,parents,mounted_spine_index,indices+4))InterlockedIncrement(&mounted_palette_holds);
                else InterlockedIncrement(&mounted_palette_failures);
            }
        }else {mounted_held_palette.reset();mounted_acquisition88.reset();}
        for(unsigned k=0;k<3;++k)pose_pivot[k]=(field<float>(array,indices[0]*48+12+k*16)+field<float>(array,indices[1]*48+12+k*16))*0.5f;
        memcpy(pose_world,reinterpret_cast<void*>(holder+0x24),48);
        bool aiming=equipment_ranged_selected81()&&state->aiming&&(GetAsyncKeyState(VK_RBUTTON)&0x8000)&&state->weapon_phase!=WEAPON_RELOAD;
        if(carbine_profile&&aiming&&!mounted_carbine_pose_aim(possessed,aiming)){
            aiming=false;InterlockedIncrement(&mounted_carbine_predraw_blocks72);
        }
        // Native lowering/reload still animates the arms. Keep the same look-space
        // correction when RMB goes up, rather than removing it on that input edge.
        bool infantry_look96=!mounted&&!climbing97&&equipment_ranged_selected81()&&!combat_pose96;
        if(infantry_look96){
            ViewRay view101{};
            if(!read_aim_frame(view101,true)||!firstperson101::eye_in_model(pose_world,view101.origin,pose_pivot)){
                InterlockedIncrement(&infantry_eye_rejects101);return;
            }
            InterlockedIncrement(&infantry_eye_updates101);
        }
        pose_muzzle_transform_valid=false;pose_carbine_valid=false;
        if(mounted){
            unsigned spine=mounted_spine_index;
            // Native rider assets already position the pelvis, legs and head.
            // Apply only the established upper-body look correction.
            float native_yaw=std::atan2(pose_world[2],pose_world[10]);
            float relative=std::remainder(state->yaw-native_yaw,6.2831853071795864769f);
            float rotation[12]={1,0,0,0,0,1,0,0,0,0,1,0};
            auto rendered=reinterpret_cast<float*>(array);float pivot[3]={rendered[spine*12+3],rendered[spine*12+7],rendered[spine*12+11]};
            bool firearm_aim=projectile&&!is_bow_projectile(projectile)&&aiming;
            // Rifle idle used whole-torso pitch; rifle aim used arm pitch.
            // That snapped torso/neck at RMB despite smooth arm transitions.
            // Keep the verified carbine's torso/head look convention identical
            // throughout idle/draw/reload. Only its arm/equipment lift toggles.
            bool firearm_pose=firearm_aim||carbine_profile;
            float gun_raise=firearm_aim?MOUNTED_GUN_RAISE:0.f;
            if(firearm_pose){
                if(!mounted_firearm_transform(rendered,pose_world,spine,indices[0],indices[1],relative,pitch,rotation,gun_raise)){
                    memcpy(reinterpret_cast<void*>(array),pose_before,n*48);mounted_held_palette.reset();return;
                }
            }else {pitch_matrix(rotation,pivot,std::clamp(pitch,-.65f,.65f));yaw_matrix(rotation,pivot,std::clamp(relative,-mountedlook85::half_arc,mountedlook85::half_arc));}
            float inverse_world[12],to_world[12];
            if(affine_pose_inverse(pose_world,inverse_world)){
                affine_product(pose_world,rotation,to_world);
                affine_product(to_world,inverse_world,pose_muzzle_transform);pose_muzzle_transform_valid=true;
            }
            bool pose_ok=firearm_pose?
                pose_mounted_firearm(rendered,n,parents,spine,indices[0],indices[1],indices[2],indices[3],indices+4,pose_world,relative,pitch,gun_raise):
                pose_rider_matrices(rendered,n,parents,spine,indices[0],indices[1],indices[2],indices[3],indices+4,relative,pitch);
            if(!pose_ok){
                // Cache correction is provisional until the entire owned pose
                // passes validation. Restore native bytes on rejection.
                memcpy(reinterpret_cast<void*>(array),pose_before,n*48);
                mounted_held_palette.reset();pose_muzzle_transform_valid=false;return;
            }
        }else if(!pose_owned_matrices(reinterpret_cast<float*>(array),n,parents,indices[0],indices[1],indices[2],indices[3],indices+4,pitch,infantry_look96,infantry_look96?pose_pivot:nullptr))return;
        if(carbine_profile&&aiming){
            ViewRay ray{};if(read_aim_frame(ray))align_mounted_carbine(reinterpret_cast<float*>(array),n,parents,indices[0],indices[1],indices+4,pose_world,ray.direction);
        }
        if(mounted&&projectile&&!state->weapon_phase){
            mountedge::Key key{possessed,holder,binding,asset,state->lease_generation};
            float step=field<float>(base,0x15b2448);
            if(mountedge::apply(mounted_transition71,key,reinterpret_cast<float*>(array),n,parents,0,indices[0],indices[1],indices+4,
                               aiming,GetTickCount64(),!std::isfinite(step)||step<.001f)){
                InterlockedIncrement(&mounted_transition_updates71);
                if(mounted_transition71.blending)InterlockedIncrement(&mounted_transition_blends71);
            }else InterlockedIncrement(&mounted_transition_rejections71);
        }else mounted_transition71.reset();
        // Mounted bows previously launched 30--53 cm off the crosshair ray.
        // Correct the final visible rig, not only the projectile origin. Keep
        // infantry and the holstered sword unchanged. Native release/reload
        // animation and collision handling remain native.
        if(mounted&&aiming&&is_bow_projectile(projectile)&&native_clip==MOUNTED_FORWARD_LEVEL&&
           (mounted_clock_held(possessed)||state->weapon_phase==WEAPON_DRAW)){
            ViewRay ray{};float correction=0;
            if(read_aim_frame(ray))mountedshot87::align_bow(reinterpret_cast<float*>(array),n,parents,indices[0],indices[1],indices+4,pose_world,ray,correction);
        }
        // Compute projectile sockets from the final visible transition pose.
        if(carbine_profile&&aiming)
            pose_carbine_valid=calibrated_carbine_muzzle_world(reinterpret_cast<float*>(array)+indices[4]*12,pose_world,pose_carbine_world);
        // Capture the same full-size posed arrow attachment used for rendering.
        // Native3e0b10 is a clip metadata point, not this live weapon socket.
        // Save it before the post-shot visual basis reduction; do not pitch twice.
        pose_arrow_valid=is_bow_projectile(projectile)&&indices[6]==3&&parents[3]<0&&
            arrow_attachment_world(reinterpret_cast<float*>(array)+3*12,pose_world,pose_arrow_world);
        // Native mesh1 rigid_equip_arrow_lod1..4 all bind bone3 (Weapon3).
        // All Yumi/extended Yumi meshes bind bone1; the sword binds bone2.
        // Suppress only the display arrow in this actor's owned skin result.
        bool hide_arrow=is_bow_projectile(projectile)&&possessed==weapon_actor&&
            InterlockedCompareExchange(&manual_arrow_released,0,0)!=0&&
            (state->weapon_phase==WEAPON_FIRED||state->weapon_phase==WEAPON_RELOAD);
        state->arrow_visual_hidden=0;
        if(hide_arrow&&indices[4]==1&&indices[5]==2&&indices[6]==3&&parents[3]<0&&
           hide_display_arrow(reinterpret_cast<float*>(array),n,indices[6])){
            ++state->arrow_visual_updates;state->arrow_visual_hidden=1;state->arrow_visual_bone=indices[6];
        }
        pose_holder=holder;pose_result=result;pose_array=array;pose_count=n;pose_binding100=binding;pose_asset100=asset;
        memcpy(pose_after,reinterpret_cast<void*>(array),n*48);pose_saved=true;
        if(mounted&&is_bow_projectile(projectile)&&GetTickCount64()-mounted_pose_audit_tick87>=50){
            mounted_pose_audit_tick87=GetTickCount64();auto& sample=mounted_pose_audit87[mounted_pose_sequence87++%32];
            sample.tick=mounted_pose_audit_tick87;sample.lease=state->lease_generation;sample.draw=state->draws_started;
            sample.frame=field<unsigned>(possessed,0x1d8);sample.phase=state->weapon_phase;sample.held=mounted_clock_held(possessed)?1:0;
            sample.layout89=unsigned(rig_layout89);sample.rejections89=unsigned(rig_rejections89);
            sample.sim_tick88=mounted_blend_tick88();sample.acquired88=mounted_held_palette.valid?1:0;
            sample.blend_ready88=mounted_bow_blend_ready88()?1:0;sample.settle_matches88=mounted_acquisition88.matches;
            memcpy(sample.world,pose_world,48);
            for(unsigned i=0;i<7;++i){sample.bones[i]=indices[i];memcpy(sample.native+i*12,pose_before+indices[i]*12,48);memcpy(sample.visible+i*12,pose_after+indices[i]*12,48);}
        }

        pose_actor=possessed;pose_environment=environment;pose_tick=GetTickCount64();
        pose_actor_id=possession_lease.identity.actor_id;pose_unit=unit;pose_unit_id=possession_lease.identity.unit_id;
        pose_config=possession_lease.identity.config;pose_infantry_pitch101=infantry_look96;
        ++state->pose_updates;if(aiming)++state->pose_aim_updates;state->pose_pitch=(mounted||infantry_look96)?pitch:0;state->pose_bones=n;
        state->pose_weapon_record=record_id;state->pose_native_clip=native_clip;state->pose_native_bias=state->pitch-pitch;
    }__except(EXCEPTION_EXECUTE_HANDLER){state->pose_error=1;}
}
static volatile LONG self_visibility_updates100,self_visibility_rejects100,self_visibility_restores100;
struct SelfSkin100 {
    uintptr_t holder{},result{},array{},binding{},asset{};unsigned lease{},count{},head{},neck{},left{},right{},weapons[3]{};
    int32_t parents[256]{};float native[256*12]{};
};
static bool prepare_self_skin100(uintptr_t holder,uintptr_t result,SelfSkin100& skin){
    __try{
        // Retire only our exact previous presentation, including when native
        // animation evaluation reuses its palette. Never undo a fresh or foreign
        // palette, a new actor serial, a new binding or a changed model count.
        auto root=field<uintptr_t>(base,0x1bc8180);
        if(pose_saved&&holder==pose_holder&&result==pose_result&&root&&field<uintptr_t>(root,0x110)==pose_environment&&
           field<uintptr_t>(holder,8)==pose_actor&&field<unsigned>(pose_actor,0x1c8)==pose_actor_id&&field<uintptr_t>(pose_actor,0x214)==pose_unit&&
           field<unsigned>(pose_unit,4)==pose_unit_id&&field<uintptr_t>(pose_actor,0x20c)==pose_config&&
           (!pose_binding100||(field<uintptr_t>(pose_holder,0x1e4)==pose_binding100&&field<uintptr_t>(pose_binding100,0x1c)==pose_asset100))&&field<unsigned>(result,4)==pose_count&&
           field<uintptr_t>(result,8)==pose_array&&pose_count>0&&pose_count<=256&&memcmp(reinterpret_cast<void*>(pose_array),pose_after,pose_count*48)==0){
            memcpy(reinterpret_cast<void*>(pose_array),pose_before,pose_count*48);pose_saved=false;InterlockedIncrement(&self_visibility_restores100);
        }
        if(third_person152||!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||!possession_lease.active||!possessed||
           holder!=possession_lease.identity.holder||field<uintptr_t>(holder,8)!=possessed||!followed(possessed,false))return false;
        if(field<unsigned char>(possessed,0xf0)!=1)return false; // Native death rig remains unchanged.
        skin.lease=possession_lease.generation;skin.holder=holder;skin.result=result;skin.binding=field<uintptr_t>(holder,0x1e4);if(!skin.binding)return false;
        skin.asset=field<uintptr_t>(skin.binding,0x1c);skin.count=field<unsigned>(result,4);skin.array=field<uintptr_t>(result,8);
        if(!skin.asset||!skin.array||!skin.count||skin.count>256||skin.count!=field<unsigned>(skin.asset,0x3c))return false;
        auto hierarchy=field<uintptr_t>(skin.asset,0x40);if(!hierarchy)return false;
        const unsigned offsets[7]={0x9c,0xa0,0xa8,0xa4,0x90,0x94,0x98};unsigned indices[7]{};
        for(unsigned i=0;i<7;++i){auto bone=field<uintptr_t>(skin.binding,offsets[i]);if(!bone)return false;indices[i]=field<unsigned>(bone,4);if(indices[i]>=skin.count)return false;}
        skin.head=indices[0];skin.neck=indices[1];skin.left=indices[2];skin.right=indices[3];memcpy(skin.weapons,indices+4,sizeof skin.weapons);
        if(skin.head==skin.left||skin.head==skin.right||skin.left==skin.right)return false;
        for(unsigned i=0;i<skin.count;++i)skin.parents[i]=field<int32_t>(hierarchy,i*20+16);
        memcpy(skin.native,reinterpret_cast<void*>(skin.array),skin.count*48);
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool current_self_skin100(const SelfSkin100& skin){
    __try{return enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED&&possession_lease.active&&skin.lease==possession_lease.generation&&
        skin.holder==possession_lease.identity.holder&&field<uintptr_t>(skin.holder,8)==possessed&&field<uintptr_t>(skin.holder,0x1e4)==skin.binding&&
        field<uintptr_t>(skin.binding,0x1c)==skin.asset&&field<uintptr_t>(skin.result,8)==skin.array&&field<unsigned>(skin.result,4)==skin.count&&
        !dead_view()&&field<unsigned char>(possessed,0xf0)==1&&followed(possessed,false);}
    __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static void finish_self_skin100(const SelfSkin100& skin){
    __try{
        if(!current_self_skin100(skin))return;
        if(!visibility100::hide(reinterpret_cast<float*>(skin.array),skin.native,skin.count,skin.parents,skin.head,skin.neck,skin.left,skin.right,skin.weapons)){
            InterlockedIncrement(&self_visibility_rejects100);return;
        }
        // One restoration record covers weapon corrections and self visibility.
        // Existing muzzle/arrow snapshots stay independent of the hidden head.
        pose_holder=skin.holder;pose_result=skin.result;pose_array=skin.array;pose_count=skin.count;pose_binding100=skin.binding;pose_asset100=skin.asset;
        memcpy(pose_before,skin.native,skin.count*48);memcpy(pose_after,reinterpret_cast<void*>(skin.array),skin.count*48);pose_saved=true;
        pose_actor=possessed;pose_environment=environment;pose_actor_id=possession_lease.identity.actor_id;
        pose_unit=unit;pose_unit_id=possession_lease.identity.unit_id;pose_config=possession_lease.identity.config;pose_tick=GetTickCount64();
        InterlockedIncrement(&self_visibility_updates100);
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedIncrement(&self_visibility_rejects100);}
}
static void native_skin_cache101(void* holder,void* result,const SelfSkin100& skin){
    // The original method retains the live palette pointer and caches raw head
    // position plus the shoulder axis. Presentation must not feed back into
    // native camera/animation calculations. All three sockets are proven unique.
    const unsigned sockets[3]={skin.head,skin.left,skin.right};float presented[3][3]{};
    for(unsigned socket=0;socket<3;++socket)for(unsigned row=0;row<3;++row){
        auto offset=sockets[socket]*12+row*4+3;auto palette=reinterpret_cast<float*>(skin.array);
        presented[socket][row]=palette[offset];palette[offset]=skin.native[offset];
    }
    __try{original_skin_ready(holder,result);}
    __finally{for(unsigned socket=0;socket<3;++socket)for(unsigned row=0;row<3;++row)
        reinterpret_cast<float*>(skin.array)[sockets[socket]*12+row*4+3]=presented[socket][row];}
}
static void __fastcall skin_ready_hook(void* holder,void*,void* result){
    AcquireSRWLockExclusive(&pose_lock);
    __try{
        SelfSkin100 skin{};bool owned=prepare_self_skin100(reinterpret_cast<uintptr_t>(holder),reinterpret_cast<uintptr_t>(result),skin);
        if(owned){pose_arrow_valid=false;pose_carbine_valid=false;pose_muzzle_transform_valid=false;pose_infantry_pitch101=false;}
        capture_model_eye(reinterpret_cast<uintptr_t>(holder),reinterpret_cast<uintptr_t>(result));
        apply_soldier_pose(reinterpret_cast<uintptr_t>(holder),reinterpret_cast<uintptr_t>(result));
        if(owned&&current_self_skin100(skin)){
            native_skin_cache101(holder,result,skin);
            finish_self_skin100(skin);
        }else original_skin_ready(holder,result);
    }__finally{ReleaseSRWLockExclusive(&pose_lock);}
}
static void pose_manual_origin(float* origin){
    if(third_person152)return;
    AcquireSRWLockShared(&pose_lock);
    __try {
    if(pose_saved&&pose_actor==possessed&&pose_environment==environment&&pose_unit==unit&&
       pose_actor_id==possession_lease.identity.actor_id&&pose_unit_id==possession_lease.identity.unit_id&&
       GetTickCount64()-pose_tick<=250&&std::isfinite(state->pitch)&&followed(possessed,false)){
        uintptr_t record=bow_record(unit);
        if(record&&weapon_mounted()&&pose_carbine_valid){
            bool valid=true;for(unsigned r=0;r<3;++r)if(!std::isfinite(pose_carbine_world[r])||std::abs(pose_carbine_world[r]-field<float>(possessed,0x48+r*4))>4)valid=false;
            if(valid){memcpy(origin,pose_carbine_world,sizeof pose_carbine_world);++state->pose_muzzle_updates;}
        }else if(record&&weapon_mounted()&&pose_muzzle_transform_valid){
            float next[3];for(unsigned r=0;r<3;++r){next[r]=pose_muzzle_transform[r*4+3];for(unsigned c=0;c<3;++c)next[r]+=pose_muzzle_transform[r*4+c]*origin[c];}
            bool valid=true;for(unsigned r=0;r<3;++r)if(!std::isfinite(next[r])||std::abs(next[r]-field<float>(possessed,0x48+r*4))>4)valid=false;
            if(valid){memcpy(origin,next,sizeof next);++state->pose_muzzle_updates;}
        }else if(record&&pose_infantry_pitch101){float pitch=weapon_pose_pitch(is_bow_projectile(record)?3:field<unsigned>(record,0),weapon_clip,state->pitch);
            if(pitch_world_point(origin,pose_pivot,pose_world,pitch))++state->pose_muzzle_updates;}
    }
    }__finally{ReleaseSRWLockShared(&pose_lock);}
}
static bool posed_arrow_origin(float* origin){
    bool valid=false;AcquireSRWLockShared(&pose_lock);
    __try {
        if(pose_saved&&pose_arrow_valid&&pose_actor==possessed&&pose_environment==environment&&
           pose_actor_id==possession_lease.identity.actor_id&&pose_unit_id==possession_lease.identity.unit_id&&
           GetTickCount64()-pose_tick<=250&&followed(possessed,false)){
            valid=true;
            for(unsigned i=0;i<3;++i)if(!std::isfinite(pose_arrow_world[i])||
                std::abs(pose_arrow_world[i]-field<float>(possessed,0x48+i*4))>4)valid=false;
            if(valid)memcpy(origin,pose_arrow_world,sizeof pose_arrow_world);
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){valid=false;}
    ReleaseSRWLockShared(&pose_lock);return valid;
}
static void create_pose_hook(){
    const unsigned char skin_signature[]={0x83,0xec,0x0c,0x56,0x8b,0x74,0x24,0x14};
    require(memcmp(reinterpret_cast<void*>(base+0x4c56d0),skin_signature,8)==0,"Skin callback signature differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x4c56d0),reinterpret_cast<void*>(&skin_ready_hook),reinterpret_cast<void**>(&original_skin_ready))==MH_OK,"Skin callback hook failed");
}

static void log_mounted_pose_audit87(std::ostream& log){
    static unsigned drained=0;MountedPoseAudit87 samples[32];unsigned count=0;
    AcquireSRWLockShared(&pose_lock);unsigned end=mounted_pose_sequence87,start=end-drained>32?end-32:drained;
    for(unsigned i=start;i<end;++i)samples[count++]=mounted_pose_audit87[i%32];drained=end;
    ReleaseSRWLockShared(&pose_lock);
    for(unsigned i=0;i<count;++i){const auto& s=samples[i];log<<"{\"event\":\"mounted_pose87\",\"tick_ms\":"<<s.tick<<",\"lease\":"<<s.lease<<",\"draw\":"<<s.draw<<",\"frame\":"<<s.frame<<",\"phase\":"<<s.phase<<",\"held\":"<<s.held;
        log<<",\"rig_layout89\":"<<s.layout89<<",\"rig_rejections89\":"<<s.rejections89;
        log<<",\"sim_tick88\":"<<s.sim_tick88<<",\"acquired88\":"<<s.acquired88<<",\"blend_ready88\":"<<s.blend_ready88<<",\"settle_matches88\":"<<s.settle_matches88;
        auto list=[&](const char* name,const auto* values,unsigned n){log<<",\""<<name<<"\":[";for(unsigned j=0;j<n;++j){if(j)log<<',';log<<values[j];}log<<']';};
        list("bones",s.bones,7);list("native",s.native,84);list("visible",s.visible,84);list("world",s.world,12);log<<"}\n";
    }
}
