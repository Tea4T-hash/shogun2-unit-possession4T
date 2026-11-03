#include "identity.h"
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "steering.h"
#include "personal_input199.h"
#include "mounted_look85.h"
#include "look_pitch96.h"
#include "animation_look104.h"
#include "possession_entry125.h"
#include "combat_focus125.h"
#include "view152.h"
#include "../../../common/startup_view160.h"
#include "../../../common/startup_gate168.h"
#include "../../../common/component_error168.h"
#include "possession_lease.h"
#include "control_metrics.h"
#include "camera_entry_gate.h"
#include "command_input100.h"
#include "legacy_v34.h"
#include "../vendor/include/MinHook.h"
#include "native_overlay106.h"
#include "miles_zero123.h"
#include "../../../common/runtime_storage166.h"
#include <intrin.h>
static_assert(sizeof(void*)==4,"Build this adapter for x86");
static HMODULE self_module;
static uintptr_t base;
static Status* state;
static HANDLE mapping;
static volatile LONG enabled;
static uintptr_t possessed,environment,unit;
static PossessionLease possession_lease;
static volatile LONG charge_input_generation146;
static volatile LONG charge_physical_input148;
static bool charge_input_locked146(){return enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED&&state->lease_generation&&unsigned(InterlockedCompareExchange(&charge_input_generation146,0,0))==state->lease_generation&&GetTickCount64()-state->heartbeat<=500;}
static bool charge_member146(uintptr_t actor);
static bool charge_native_task146(uintptr_t actor);
static bool proximity_managed147(uintptr_t actor);
static bool proximity_deny147(uintptr_t squad,uintptr_t actor);
static uintptr_t proximity_squad147(uintptr_t actor);
static bool formation_owned129(uintptr_t actor);
static uintptr_t charge_squad147(uintptr_t actor);
static void create_native_charge147();
static void charge_boundary146(uintptr_t native_unit,bool after);

static bool overlay_key106(const overlay106::Frame& frame,unsigned layer){
    auto key=frame.key;
    // Retained cancellation/result feedback is immutable numeric UI, never an
    // ownership permission. It may survive the matching possession's release.
    if(layer==overlay106::QTE&&frame.active==2&&state&&enabled&&state->ready&&!state->stop&&
       key.lease==state->lease_generation&&key.actor==state->actor_id&&key.unit==state->unit_id)return true;
    if(!state||!enabled||!state->ready||state->stop||state->release||state->mode!=POSSESSED||
       GetTickCount64()-state->heartbeat>500||!possession_lease.active)return false;
    auto identity=possession_lease.identity;unsigned generation=possession_lease.generation;
    return key.lease==generation&&key.actor==identity.actor_id&&key.unit==identity.unit_id&&
        key.lease==state->lease_generation&&key.actor==state->actor_id&&key.unit==state->unit_id&&
        possession_lease.active&&generation==possession_lease.generation&&state->mode==POSSESSED;
}
// Numeric heading published only at validated native boundaries. The mouse
// worker never dereferences a horse pointer, including during battle changes.
static volatile LONG mounted_heading85=-1;
static volatile LONG entry_movement125,mount_camera_locked154;
static SRWLOCK entry_lock125=SRWLOCK_INIT;
static look125::Focus combat_focus125;
static volatile LONG combat_entry_locked126;
static SRWLOCK combat_pair_lock126=SRWLOCK_INIT;
static look126::Pair combat_pair126;
static void combat_pair_reset126(){AcquireSRWLockExclusive(&combat_pair_lock126);combat_pair126={};ReleaseSRWLockExclusive(&combat_pair_lock126);}
static bool combat_pair_ready126(const melee60::Request& request){
    AcquireSRWLockShared(&combat_pair_lock126);auto pair=combat_pair126;ReleaseSRWLockShared(&combat_pair_lock126);
    return pair.locked(request,GetTickCount64());
}
// Worker consumes numeric intent only; all native actor reads stay at acquisition.
static unsigned entry_input125(unsigned physical,bool focused){
    AcquireSRWLockExclusive(&entry_lock125);
    // Check the mode inside the same lock as acquisition: an older ARMED
    // input poll must not erase the just-published movement handoff.
    if(!focused||!state||state->mode!=POSSESSED||state->release||state->stop||
       (physical&(FORWARD|BACKWARD|LEFT|RIGHT)))InterlockedExchange(&entry_movement125,0);
    unsigned result=physical|unsigned(InterlockedCompareExchange(&entry_movement125,0,0));
    ReleaseSRWLockExclusive(&entry_lock125);return result;
}
static HWND input_window;
static WNDPROC native_window_proc;
#include "raw_mouse.h"
#include "melee_control.h"
#include "equipment_input81.h"
static void weapon_mouse143(UINT message,bool active);
#include "native_keyboard84.h"
static constexpr UINT RESTORE_INTERFACE_MESSAGE=WM_APP+0x539;
static constexpr UINT RETURN_RTS_MESSAGE90=WM_APP+0x53b;
static void request_rts_restore90();
static void restore_rts_on_window90(uint32_t generation=0);
static constexpr UINT ENTER_SOLDIER_MESSAGE=WM_APP+0x53a;
static_assert(COMMAND_CANCEL_MESSAGE100!=RETURN_RTS_MESSAGE90&&COMMAND_CANCEL_MESSAGE100!=ENTER_SOLDIER_MESSAGE&&COMMAND_CANCEL_MESSAGE100!=RESTORE_INTERFACE_MESSAGE,"Private command/window messages must remain distinct");
static __declspec(thread) LONG camera_callback_depth;
static volatile LONG motion_callback_depth,entry_pending,entry_attempted;
static ClimbRequest199 climb_request199;
static void command_reset100();
static void command_native_cancel100();
static bool command_native_signatures100(uintptr_t);
static bool command_draw_permitted100(void*);
static void command_draw_started100(void*);
static bool command_release_managed100(void*);
static void command_on_shot100();
using CameraInput=uintptr_t(__thiscall*)(void*,uintptr_t,uintptr_t);
static CameraInput original_camera_input;
static void* camera_input_target;

static void interface_restore();
static void relinquish();
static void enter_selected_soldier();
static void climb_adopt98(uintptr_t actor);
static void climb_observe98(uintptr_t actor);
static bool climb_pending153(uintptr_t actor);
static bool climb_independent200(uintptr_t actor);
static void request_possession(){
    if(!startup168::can_possess())return;
    if(!enabled||!state||state->stop||state->release||state->mode==POSSESSED||state->mode==ARMED)return;
    InterlockedExchange(&entry_attempted,0);
    InterlockedExchange(&state->acquire,1);InterlockedExchange(&state->mode,ARMED);
}
// ---------------------------------------------------------------------------------------------
// Player key layout (custom build). Applied before every other key handler while possessed.
//   Up arrow    = Move / Attack (was Q)      Down arrow  = Hold Here (was Z)
//   Left arrow  = Follow Me (was C)          Right arrow = Stop (was E)
//   F = select ability (was T)               G = use ability (unchanged)
//   E = interact, in priority order: dismount -> climb wall -> mount
// The old Q, Z, C, T, M and F-climb keys do nothing while possessed.
// ---------------------------------------------------------------------------------------------
static int interact_next201; // 0 = climb next, 1 = mount next (when on foot)
static void interact201(){
    if(!state||charge_input_locked146())return;
    const LONG lease=LONG(state->lease_generation);
    if(possession_lease.identity.mount){InterlockedExchange(&mount_request154,lease);interact_next201=0;return;} // 1. dismount
    // 2. interact with object: the game has no other usable object interaction for a single soldier
    if(interact_next201==0){climb_request199.set(state->lease_generation,GetTickCount64());interact_next201=1;return;} // 3. climb
    InterlockedExchange(&mount_request154,lease);interact_next201=0; // 4. mount
}
static bool remap_keys201(UINT message,WPARAM& w,LPARAM l){
    if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED)return false;
    const bool key=message==WM_KEYDOWN||message==WM_KEYUP||message==WM_SYSKEYDOWN||message==WM_SYSKEYUP;
    const bool chr=message==WM_CHAR||message==WM_SYSCHAR;
    if(!key&&!chr)return false;
    unsigned k=unsigned(w);if(chr&&k>='a'&&k<='z')k-=32;
    if(chr){switch(k){case 'Q':case 'Z':case 'C':case 'T':case 'E':case 'F':case 'M':return true;}return false;}
    switch(k){
    // Unit orders: F1 move/attack, F2 stop, F3 hold here, F4 follow me.
    case VK_F1:w='Q';return false;
    case VK_F2:w='E';return false;
    case VK_F3:w='Z';return false;
    case VK_F4:w='C';return false;
    case 'Q':case 'Z':case 'C':case 'T':case 'M':case 'F':return true; // retired (F is read directly as the ability key)
    case 'E':if(message==WM_KEYDOWN&&!(l&(1L<<30)))interact201();return true;
    }
    return false;
}
static LRESULT CALLBACK weapon_window_proc(HWND window,UINT message,WPARAM w,LPARAM l){
    if(remap_keys201(message,w,l))return 0;
    if(charge_input_locked146()){
        // Drain command key releases without dispatching an order. Keys held
        // before native commitment must not remain latched after it ends.
        if(message==WM_KEYUP||message==WM_SYSKEYUP)command_key_held100&=~command100::key_bit(unsigned(w));
        if(message==WM_LBUTTONDOWN||message==WM_LBUTTONUP||message==WM_LBUTTONDBLCLK||message==WM_RBUTTONDOWN||message==WM_RBUTTONUP||message==WM_RBUTTONDBLCLK)return 0;
        if(message==WM_KEYDOWN||message==WM_KEYUP||message==WM_SYSKEYDOWN||message==WM_SYSKEYUP||message==WM_CHAR||message==WM_SYSCHAR){
            auto key=(w>='a'&&w<='z')?w-32:w;
            switch(key){case 'W':case 'A':case 'S':case 'D':case 'Q':case 'E':case 'Z':case 'C':case 'R':case 'F':case 'T':case 'G':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case VK_SHIFT:case VK_CONTROL:case VK_TAB:return 0;}
        }
    }
    if(command_window100(window,message,w,l))return 0;
    if((message==WM_KEYDOWN||message==WM_KEYUP||message==WM_SYSKEYDOWN||message==WM_SYSKEYUP||message==WM_CHAR||message==WM_SYSCHAR)&&keyboard_window_owned84(unsigned((message==WM_CHAR||message==WM_SYSCHAR)&&w>='a'&&w<='z'?w-32:w)))return 0;
    if(message==RETURN_RTS_MESSAGE90){restore_rts_on_window90(uint32_t(w));return 0;}
    if(message==RESTORE_INTERFACE_MESSAGE){command_native_cancel100();interface_restore();if(death_return_pending)end_death_on_window();return 0;}
    if(message==ENTER_SOLDIER_MESSAGE){
        if(InterlockedExchange(&entry_pending,0)){InterlockedExchange(&entry_attempted,1);enter_selected_soldier();}
        return 0;
    }
    if(message==WM_KEYDOWN&&w==VK_F9&&!(l&(1L<<30))){request_possession();return 0;}
    if(state&&message==WM_KEYDOWN&&(w==VK_F10||w==VK_ESCAPE||w==VK_INSERT)){
        command_native_cancel100();InterlockedExchange(&state->release,1);interface_restore();
    }
    if(state&&(message==WM_KILLFOCUS||(message==WM_ACTIVATEAPP&&!w))){
        command_native_cancel100();InterlockedExchange(&state->release,1);interface_restore();
    }
    if(enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED){
        if(third_person152&&(message==WM_MBUTTONDOWN||message==WM_MBUTTONUP||message==WM_MBUTTONDBLCLK)){orbit_button154(message!=WM_MBUTTONUP);return 0;}
        if(message==WM_MOUSEWHEEL&&third_person152){view_wheel152(GET_WHEEL_DELTA_WPARAM(w));return 0;}
        // Climbing is requested through E (interact201).
        // Consume owned movement/order keys so they do not also become Total War
        // camera or unit commands. Release/menu keys continue to the native window.
        if(message==WM_KEYDOWN||message==WM_KEYUP||message==WM_SYSKEYDOWN||message==WM_SYSKEYUP){
            switch(w){case 'W':case 'A':case 'S':case 'D':case 'Q':case 'E':case 'Z':case 'C':case 'R':case 'F':case 'T':case 'G':
                case VK_SHIFT:case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:case VK_TAB:
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case VK_F5:case VK_F6:case VK_F7:case VK_F9:return 0;}
        }
        if(message==WM_INPUT&&raw_mouse_active&&read_raw_mouse(l))return DefWindowProcW(window,message,w,l);
        if(message==WM_SETCURSOR&&!command_placing100){SetCursor(nullptr);++state->cursor_suppressed;return TRUE;}
        if(command_placing100&&(message==WM_RBUTTONDOWN||message==WM_LBUTTONDOWN||message==WM_LBUTTONUP||message==WM_RBUTTONUP))return 0;
        if(message==WM_RBUTTONDOWN||message==WM_RBUTTONUP||message==WM_RBUTTONDBLCLK){
            melee60::Request active;if(dead_view()||melee_request(active))return 0;
            InterlockedExchange(&state->aiming,message==WM_RBUTTONUP?0:1);return 0;
        }
        if(message==WM_LBUTTONDOWN||message==WM_LBUTTONUP||message==WM_LBUTTONDBLCLK){
            melee60::Request active;if(dead_view()||melee_request(active))return 0;
            ++state->weapon_mouse_captured;
            weapon_mouse143(message,(GetAsyncKeyState(VK_RBUTTON)&0x8000)&&
                equipment_ranged_selected81()&&!command_placing100&&!state->weapon_phase&&
                !state->fire_ticket&&state->weapon_reload_remaining<=0);
            return 0;
        }
    }
    return native_window_proc?CallWindowProcW(native_window_proc,window,message,w,l):DefWindowProcW(window,message,w,l);
}
static void capture_weapon_mouse(HWND window){
    if(input_window)return;
    DWORD pid=0;GetWindowThreadProcessId(window,&pid);if(pid!=GetCurrentProcessId())return;
    auto prior=reinterpret_cast<WNDPROC>(GetWindowLongPtrW(window,GWLP_WNDPROC));if(!prior)return;
    native_window_proc=prior;input_window=window;SetLastError(0);
    if(!SetWindowLongPtrW(window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(&weapon_window_proc))&&GetLastError()){input_window=nullptr;native_window_proc=nullptr;}
}
using Motion=void(__thiscall*)(void*);
static Motion original;
static void* target;
static void* cancel_target;
static void* toggle_target;
static void* matrix_target;
static void* projectile_camera_target;

using Cancel=void(__thiscall*)(void*);
using Toggle=void(__thiscall*)(void*,unsigned);
static Cancel original_cancel;
static Toggle original_toggle;
static Cancel original_projectile_camera;
using Matrix=void(__thiscall*)(void*,const void*);
static Matrix original_matrix;
static const unsigned char signature[16]={0x55,0x8b,0xec,0x83,0xe4,0xf0,0x83,0xec,0x34,0x53,0x56,0x8b,0xf1,0x57,0x8b,0x06};
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
#include "native_wall97.h"
#include "native_subject.h"
#include "dismount143.h"
#include "native_recovery103.h"
static volatile LONG recovery_blocks103,recovery_active103;
static volatile LONG animation_look_active104,animation_look_updates104;
static bool recovery_prepare103(uintptr_t actor){
    bool blocked=native_recovery103(actor);InterlockedExchange(&recovery_active103,blocked?1:0);
    if(!blocked)return false;
    // Do not zero native velocity/impulse or pin the actor. The original motion
    // integrator, collision and animation root movement still execute once.
    InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);
    state->weapon_phase=0;state->aim_pose=0;
    state->requested_speed=state->effective_speed=0;InterlockedIncrement(&recovery_blocks103);return true;
}

static bool weapon_mounted(){return possession_lease.active&&possession_lease.identity.mount!=0;}
static bool native_routing(uintptr_t actor_unit);
// After proven death, the camera consumes only numeric head poses captured by
// the native renderer; it never polls a retired corpse/roster from this thread.
static bool death_context(uintptr_t camera=0){
    if(!dead_view()||!possession_lease.active||!death_eye_valid||death_elapsed()>=5500)return false;
    const auto& old=possession_lease.identity;
    __try{auto root=field<uintptr_t>(base,0x1bc8180);
        if(!root||field<uintptr_t>(root,0x110)!=old.environment)return false;
        auto common=field<uintptr_t>(old.environment,8);if(common!=old.common||field<uintptr_t>(common,0xb4)!=old.phase)return false;
        auto phase_id=field<unsigned>(old.phase,0x14);if(phase_id<5||phase_id>8)return false;
        return (!camera||camera==old.camera)&&field<uintptr_t>(old.environment,0x281fc)==old.manager&&
            field<uintptr_t>(old.manager,0x8e0)==old.camera&&field<uintptr_t>(old.camera,0)==base+0x15d22e8&&
            field<uintptr_t>(old.camera,0x180)==old.manager;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static volatile LONG routing_empty_lease154,routing_empty_holds154;
static __declspec(thread) unsigned follow_failure93;
static volatile LONG release_stage93,release_confirmed93,release_recovered93,release_flags93;
static __declspec(align(8)) volatile LONG64 release_tick93;
static bool reject_follow93(unsigned stage){follow_failure93=stage;return false;}
struct Release103 {unsigned sequence,stage,lease,actor,unit,serial,mode,clip,count,capacity,morale,routing,guard,index,cam_index,view_count;uintptr_t expected,cam_actor,manager_actor,selected,owner,group_vt,members,first;uint64_t tick;};
static SRWLOCK release_lock103=SRWLOCK_INIT;
static Release103 releases103[8];static unsigned release_sequence103;
static void release_snapshot103(){
    Release103 x{};x.stage=follow_failure93;x.lease=possession_lease.generation;x.actor=state->actor_id;x.unit=state->unit_id;x.tick=GetTickCount64();x.expected=possessed;
    __try{const auto& i=possession_lease.identity;x.cam_actor=field<uintptr_t>(i.camera,0x188);x.manager_actor=field<uintptr_t>(i.manager,0x8fc);
        x.serial=field<unsigned>(possessed,0x1c8);x.mode=field<unsigned>(possessed,0x1f0);x.clip=field<unsigned>(possessed,0x1d4);
        x.morale=field<unsigned>(i.unit,0xce8);x.routing=field<unsigned>(i.unit,0xcec);x.guard=field<unsigned char>(i.unit,0xeb1);
        x.index=field<unsigned>(possessed,0x218);x.view_count=field<unsigned>(i.view,0x18);if(x.cam_actor)x.cam_index=field<unsigned>(x.cam_actor,0x218);
        x.selected=field<uintptr_t>(i.environment,0x2826c);if(x.selected)x.owner=field<uintptr_t>(x.selected,0x4a8);
        if(x.owner){auto g=x.owner+0xb0;x.group_vt=field<uintptr_t>(g,0);x.count=field<unsigned>(g,0x14);x.capacity=field<unsigned>(g,0x10);x.members=field<uintptr_t>(g,0x18);if(x.members&&x.count&&x.capacity)x.first=field<uintptr_t>(x.members,0);}
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    AcquireSRWLockExclusive(&release_lock103);x.sequence=++release_sequence103;releases103[(x.sequence-1)%8]=x;ReleaseSRWLockExclusive(&release_lock103);
}
static void release_log103(std::ostream& log){
    static unsigned last;Release103 copy[8]{};unsigned end;
    AcquireSRWLockShared(&release_lock103);end=release_sequence103;memcpy(copy,releases103,sizeof copy);ReleaseSRWLockShared(&release_lock103);
    for(unsigned n=end>8&&last<end-8?end-8:last;n<end;++n){const auto& x=copy[n%8];
        log<<"{\"event\":\"identity_release103\",\"sequence\":"<<x.sequence<<",\"tick_ms\":"<<x.tick<<",\"stage\":"<<x.stage<<",\"lease\":"<<x.lease<<",\"actor\":"<<x.actor<<",\"unit\":"<<x.unit<<",\"serial\":"<<x.serial<<",\"mode\":"<<x.mode<<",\"clip\":"<<x.clip
           <<",\"expected_actor\":"<<x.expected<<",\"camera_actor\":"<<x.cam_actor<<",\"manager_actor\":"<<x.manager_actor<<",\"selected\":"<<x.selected<<",\"owner\":"<<x.owner<<",\"group_vtable\":"<<x.group_vt<<",\"count\":"<<x.count<<",\"capacity\":"<<x.capacity<<",\"members\":"<<x.members<<",\"first_unit\":"<<x.first
           <<",\"roster104\":1,\"morale104\":"<<x.morale<<",\"routing_state104\":"<<x.routing<<",\"native_guard104\":"<<x.guard<<",\"logical_index104\":"<<x.index<<",\"camera_index104\":"<<x.cam_index<<",\"roster_count104\":"<<x.view_count<<"}\n";
    }last=end;
}
static void note_follow_release93(){
    release_snapshot103();
    InterlockedExchange(&release_stage93,LONG(follow_failure93));InterlockedIncrement(&release_confirmed93);InterlockedExchange64(&release_tick93,LONG64(GetTickCount64()));
    __try{LONG flags=field<unsigned char>(possessed,0xf0)|(field<unsigned char>(possessed,0xa1)<<8)|(field<unsigned char>(possessed,0xa2)<<16)|(field<unsigned char>(possessed,0xa3)<<24);InterlockedExchange(&release_flags93,flags);}__except(EXCEPTION_EXECUTE_HANDLER){InterlockedExchange(&release_flags93,-1);}
}
// Called only on the game thread, immediately before its native motion integrator.
static bool followed(uintptr_t actor,bool acquire,bool commit=true) {
    __try {
        if(!acquire&&dead_view())return death_context();
        uintptr_t root=field<uintptr_t>(base,0x1bc8180);
        if(!root)return reject_follow93(1);
        uintptr_t env=field<uintptr_t>(root,0x110);if(!env)return reject_follow93(2);
        uintptr_t common=field<uintptr_t>(env,8);if(!common)return reject_follow93(3);
        uintptr_t phase=field<uintptr_t>(common,0xb4);if(!phase)return reject_follow93(4);
        unsigned phase_id=field<unsigned>(phase,0x14);if(phase_id<5||phase_id>8)return reject_follow93(5);
        uintptr_t manager=field<uintptr_t>(env,0x281fc);if(!manager)return reject_follow93(6);
        uintptr_t cam=field<uintptr_t>(manager,0x8e0);if(!cam)return reject_follow93(7);
        if(field<uintptr_t>(cam,0)!=base+0x15d22e8 || field<uintptr_t>(cam,0x180)!=manager)return reject_follow93(8);
        if(!actor152::human(base,field<uintptr_t>(actor,0)))return reject_follow93(9);
        if(!acquire&&actor==possessed&&possession_lease.active&&field<unsigned char>(actor,0xf0)==0){
            const auto& old=possession_lease.identity;
            if(env!=old.environment||common!=old.common||phase!=old.phase||manager!=old.manager||cam!=old.camera||
               field<unsigned>(actor,0x1c8)!=old.actor_id)return reject_follow93(10);
            begin_death_view();return death_context(cam);
        }
        if(field<uintptr_t>(cam,0x188)!=actor||field<uintptr_t>(manager,0x8fc)!=actor)return reject_follow93(11);
        melee60::Request active_commitment;bool committed=melee_request(active_commitment)||
            (possession_lease.active&&melee_handoff78.matches(locomotion_actor(),possession_lease.generation,
                possession_lease.identity.mount?possession_lease.identity.mount_id:possession_lease.identity.actor_id,possession_lease.identity.unit_id));
        if(field<unsigned char>(actor,0xf0)!=1||(field<uintptr_t>(actor,0x18)&&!wall_subject97(actor)))return reject_follow93(12);
        uintptr_t view=field<uintptr_t>(cam,0x184);if(!view)return reject_follow93(13);
        unsigned count=field<unsigned>(view,0x18);if(count==0||count>1000)return reject_follow93(14);
        uintptr_t array=field<uintptr_t>(view,0x1c);if(!array)return reject_follow93(15);
        unsigned idx=count;uintptr_t actor_holder=0;
        for(unsigned i=0;i<count;++i){uintptr_t holder=field<uintptr_t>(array,i*4);if(holder&&field<uintptr_t>(holder,8)==actor){idx=i;actor_holder=holder;break;}}
        if(idx==count)return reject_follow93(16);
        uintptr_t actor_unit=field<uintptr_t>(actor,0x214);if(!actor_unit)return reject_follow93(17);
        // Morale belongs to the unit; it does not retire a living player lease.
        // Exactly one selected unit must own the followed native soldier. This
        // also rejects an old eye-level camera after an overhead selection change.
        uintptr_t selected=field<uintptr_t>(env,0x2826c);if(!selected)return reject_follow93(19);
        uintptr_t owner=field<uintptr_t>(selected,0x4a8);if(!owner)return reject_follow93(20);
        uintptr_t group=owner+0xb0;
        if(field<uintptr_t>(group,0)!=base+0x15c4da0||field<unsigned>(group,0x14)>field<unsigned>(group,0x10))return reject_follow93(21);
        unsigned selected_count154=field<unsigned>(group,0x14);
        bool retained_route154=!acquire&&possession_lease.active&&actor==possession_lease.identity.actor&&
            actor_unit==possession_lease.identity.unit&&field<unsigned>(actor,0x1c8)==possession_lease.identity.actor_id&&
            field<unsigned>(actor_unit,4)==possession_lease.identity.unit_id&&env==possession_lease.identity.environment;
        // Native routing clears the selection (recorded stage21, count0). The
        // current camera/roster/serial/lease still prove this SAME living human.
        // A nonempty foreign selection and every acquisition remain strict.
        if(!selected_count154&&retained_route154&&(native_routing(actor_unit)||
            unsigned(InterlockedCompareExchange(&routing_empty_lease154,0,0))==possession_lease.generation)){
            if(commit)InterlockedExchange(&routing_empty_lease154,LONG(possession_lease.generation));
            InterlockedIncrement(&routing_empty_holds154);
        }else{
            if(selected_count154!=1)return reject_follow93(21);
            uintptr_t members=field<uintptr_t>(group,0x18);if(!members||field<uintptr_t>(members,0)!=actor_unit)return reject_follow93(22);
        }
        PossessionIdentity candidate{env,common,phase,manager,cam,view,actor_holder,actor,actor_unit,
            field<uintptr_t>(actor,0x20c),field<unsigned>(actor,0x1c8),field<unsigned>(actor_unit,4)};
        if(!acquire&&commit&&camera_callback_depth&&GetCurrentThreadId()==state->window_thread)dismount_observe143(candidate);
        NativeSubject motion;if(!native_subject(actor,actor_unit,view,committed,motion)){
            // During a proved native dismount, retain the SAME live human's
            // camera only. Every manual-motion/weapon path yields below.
            if(!acquire&&dismount_pending143(candidate))return true;
            return reject_follow93(23);
        }
        if(motion.actor){
            if(field<uintptr_t>(actor,0x28)!=env||field<uintptr_t>(motion.actor,0x28)!=env)return reject_follow93(24);
            candidate.mount=motion.actor;candidate.mount_holder=motion.holder;
            candidate.mount_config=motion.config;candidate.mount_id=motion.serial;
        }
        if(!acquire){
            if(possession_lease.owns(candidate))return true;
            if(dismount_finished143(candidate)){
                // Only the current camera/window callback may publish a new
                // subject generation. Guard-only reads cannot mutate a lease.
                if(commit&&camera_callback_depth&&GetCurrentThreadId()==state->window_thread){
                    if(!possession_lease.acquire(candidate))return reject_follow93(27);
                    state->actor_index=idx;state->lease_generation=possession_lease.generation;view_lease152=possession_lease.generation;mount_completed154_now();
                    InterlockedExchange(&dismount_cleanup143,1);InterlockedIncrement(&dismount_transitions143);
                }
                return true;
            }
            if(dismount_pending143(candidate))return true;
            return reject_follow93(29);
        }
        if(!candidate.config||!candidate.unit_id)return reject_follow93(25);
        // Finish an already-playing stock exchange before taking its actor.
        // Rewinding it would risk duplicating a casualty already committed.
        uintptr_t existing=field<uintptr_t>(actor,0x688);
        if(existing&&field<unsigned char>(existing,0xd1)!=0)return reject_follow93(26);
        if(!commit)return true;
        if(!possession_lease.acquire(candidate))return reject_follow93(27);
        environment=env;unit=actor_unit;state->actor_index=idx;state->unit_id=candidate.unit_id;
        state->actor_id=candidate.actor_id;state->lease_generation=possession_lease.generation;
        return true;
    } __except(EXCEPTION_EXECUTE_HANDLER){return reject_follow93(28);}
}
static bool confirm_follow93(uintptr_t actor){
    if(followed(actor,false))return true;
    // No stale identity or grace-period bypass: the second pass must fully prove
    // the original lease again before any player control is allowed.
    if(followed(actor,false)){InterlockedIncrement(&release_recovered93);return true;}
    return false;
}
// Commit on the validated window boundary; do not wait for a ground-motion callback.
static bool adopt_possession97(uintptr_t actor){
    if(!state||!state->acquire||possessed||!followed(actor,true))return false;
    possessed=actor;auto locomotion=possession_lease.identity.mount?possession_lease.identity.mount:actor;
    unsigned heading=field<unsigned short>(locomotion,0x64);
    state->yaw=entry125::facing(heading);state->pitch=0;eye_reset152();combat_focus125.reset();InterlockedExchange(&combat_entry_locked126,0);combat_pair_reset126();
    InterlockedExchange(&mounted_heading85,possession_lease.identity.mount?LONG(heading):-1);
    unsigned carry=entry125::carry(field<unsigned>(locomotion,0x1f0),field<unsigned>(locomotion,0x244),
        field<float>(locomotion,0x8c),field<float>(locomotion,0x94));
    AcquireSRWLockExclusive(&entry_lock125);
    if(unsigned(state->input)&(FORWARD|BACKWARD|LEFT|RIGHT))carry=0;
    InterlockedExchange(&entry_movement125,LONG(carry));InterlockedOr(&state->input,LONG(carry));
    InterlockedExchange(&state->acquire,0);InterlockedExchange(&state->mode,POSSESSED);
    ReleaseSRWLockExclusive(&entry_lock125);climb_adopt98(actor);return true;
}
#include "camera_choice97.h"
// Native camera action handler: ECX=input object, two stack arguments, RET8.
// Observe only its real environment backlink; forward both arguments and EAX.
static bool camera_owned(uintptr_t);
static volatile LONG camera_input_blocks103;
static uintptr_t __fastcall camera_input(void* object,void*,uintptr_t first,uintptr_t second){
    __try {
        uintptr_t root=field<uintptr_t>(base,0x1bc8180);
        uintptr_t env=root?field<uintptr_t>(root,0x110):0;
        if(env&&field<uintptr_t>(reinterpret_cast<uintptr_t>(object),0x1c)==env)
            {state->camera_input_thread=GetCurrentThreadId();
                auto manager=field<uintptr_t>(env,0x281fc),camera=manager?field<uintptr_t>(manager,0x8e0):0;
                if(camera&&state->mode==POSSESSED&&!dead_view()&&camera_owned(camera)){InterlockedIncrement(&camera_input_blocks103);return 0;}
            }
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    ++camera_callback_depth;
    uintptr_t result=0;
    __try {result=original_camera_input(object,first,second);}
    __finally {--camera_callback_depth;}
    return result;
}
static bool camera_owned(uintptr_t camera) {
    __try {
        if(!enabled || state->stop || state->release || GetTickCount64()-state->heartbeat>500
            || (!state->acquire&&!possessed))return false;
        if((GetAsyncKeyState(VK_INSERT)&0x8000)||(GetAsyncKeyState(VK_F10)&0x8000)
            ||(GetAsyncKeyState(VK_ESCAPE)&0x8000))return false;
        if(dead_view())return death_context(camera);
        if(field<uintptr_t>(camera,0)!=base+0x15d22e8)return false;
        if(possessed&&possession_lease.active&&camera==possession_lease.identity.camera&&
           actor152::human(base,field<uintptr_t>(possessed,0))&&field<unsigned>(possessed,0x1c8)==possession_lease.identity.actor_id&&
           field<unsigned char>(possessed,0xf0)==0)return followed(possessed,false,false);
        uintptr_t actor=field<uintptr_t>(camera,0x188);
        if(!actor&&possessed&&possession_lease.active&&camera==possession_lease.identity.camera)actor=possessed;
        if(!actor)return false;
        if(possessed&&actor!=possessed)return false;
        return followed(actor,!possessed,false);
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
#include "native_orders.h"
#include "native_movement.h"
#include "native_speed_modifiers.h"
#include "native_terrain.h"
#include "native_fatigue.h"
static bool melee_motion_identity(uintptr_t actor,const melee60::Request& request,bool& player,bool& opponent,uintptr_t& rider){
    player=opponent=false;rider=actor;
    auto vtable=field<uintptr_t>(actor,0);
    if(vtable==base+0x15e8524){
        auto horse_unit=field<uintptr_t>(actor,0x214);if(!horse_unit)return false;
        auto horse_serial=field<unsigned>(actor,0x1c8);
        auto count=field<unsigned>(actor,0x6cc),links=field<uintptr_t>(actor,0x6d0);
        if(!links||count!=1)return false;
        unsigned matched=0;rider=0;
        for(unsigned i=0;i<count;++i){auto candidate=field<uintptr_t>(links,i*4);
            if(!candidate||!actor152::human(base,field<uintptr_t>(candidate,0))||field<uintptr_t>(candidate,0x210)!=actor||
               field<uintptr_t>(candidate,0x214)!=field<uintptr_t>(actor,0x214)||field<unsigned char>(candidate,0xf0)!=1)continue;
            auto u=field<uintptr_t>(candidate,0x214),serial=field<unsigned>(candidate,0x1c8),id=field<unsigned>(u,4);
            bool p=(request.player_kind==1?horse_serial:serial)==request.player&&id==request.player_unit,
                 e=(request.opponent_kind==1?horse_serial:serial)==request.opponent&&id==request.opponent_unit;
            if(p||e){rider=candidate;player=p;opponent=e;++matched;}
        }
        if(matched!=1)return false;
        if(player&&(actor!=possession_lease.identity.mount||field<unsigned>(actor,0x1c8)!=possession_lease.identity.mount_id))return false;
    }else if(actor152::human(base,vtable)){
        auto u=field<uintptr_t>(actor,0x214);if(!u)return false;
        auto serial=field<unsigned>(actor,0x1c8),id=field<unsigned>(u,4);
        player=request.player_kind==0&&serial==request.player&&id==request.player_unit;
        opponent=request.opponent_kind==0&&serial==request.opponent&&id==request.opponent_unit;
    }else return false;
    return (player!=opponent)&&field<unsigned char>(actor,0xf0)==1&&!field<uintptr_t>(rider,0x21c);
}
static bool native_melee_active78(uintptr_t actor,bool playing_only){
    auto witness=field<uintptr_t>(actor,0x688);if(!witness)return false;
    return field<unsigned char>(witness,0xc)==1&&field<unsigned>(witness,0x38)==2&&
        (field<uintptr_t>(witness,0x30)==actor||field<uintptr_t>(witness,0x34)==actor)&&
        (!playing_only||field<unsigned char>(witness,0xd1)!=0);
}
static void observe_combat_pair126(uintptr_t actor,const melee60::Request& request,bool player){
    __try{
    look126::Pose sample{};sample.x=field<float>(actor,0x48);sample.z=field<float>(actor,0x50);
    sample.bearing=field<unsigned short>(actor,0x64);sample.tick=GetTickCount64();
    sample.animation=field<uintptr_t>(actor,0x688);sample.playing=native_melee_active78(actor,true);sample.valid=true;
    AcquireSRWLockExclusive(&combat_pair_lock126);combat_pair126.observe(request,player,sample);ReleaseSRWLockExclusive(&combat_pair_lock126);
    }__except(EXCEPTION_EXECUTE_HANDLER){} // Observation never changes native combat control.
}
static void observe_melee_handoff78(uintptr_t actor,const melee60::Request& request,bool player){
    observe_combat_pair126(actor,request,player);
    auto& tracking=player?melee_handoff78:melee_opponent_motion78;
    if(!player||actor==locomotion_actor())tracking.observe(actor,possession_lease.generation,
        field<unsigned>(actor,0x1c8),field<unsigned>(field<uintptr_t>(actor,0x214),4),request.serial,
        request.phase==melee60::NativeCombat||native_melee_active78(actor,true));
}
// Called only on the owned motion callback after native playback/cleanup. Use
// the CURRENT native root, never an admitted anchor or a cached render pose.
static bool finish_melee_handoff78(uintptr_t actor){
    if(!possession_lease.active||!melee_handoff78.matches(actor,possession_lease.generation,
        field<unsigned>(actor,0x1c8),field<unsigned>(field<uintptr_t>(actor,0x214),4)))return false;
    if(native_melee_active78(actor,false)||!root_motion_subject(actor,false))return true;
    float position[2]={field<float>(actor,0x48),field<float>(actor,0x50)};
    for(unsigned i=0;i<3;++i){float value=field<float>(actor,0x48+i*4);if(!std::isfinite(value)||std::abs(value)>100000)return true;melee_handoff_root78[i]=value;}
    // Clear only residual translation/acceleration; the actor's position,
    // spatial memberships, native animation and casualty state are untouched.
    for(unsigned offset:{0x70u,0x74u,0x78u,0x7cu,0x8cu,0x90u,0x94u,0xd8u,0xdcu,0xe0u})field<float>(actor,offset)=0;
    field<unsigned short>(actor,0xe4)=0;
    reinterpret_cast<NativeIntent>(base+0x3936f0)(reinterpret_cast<void*>(actor),position,field<unsigned short>(actor,0x64));
    melee_handoff78.reset();InterlockedIncrement(&melee_handoff_updates78);return true;
}
static bool apply_melee_motion(uintptr_t actor){
    melee60::Request request;if(!melee_request(request))return false;
    __try{
        bool player,opponent;uintptr_t rider;if(!melee_motion_identity(actor,request,player,opponent,rider))return false;
        observe_melee_handoff78(actor,request,player);
        // An already-started stock horse jump owns motion even while its
        // matched pair is aligning. Never replace that launch/arc with a step.
        if(native_mounted_jump140(actor))return true;
        uintptr_t actor_unit=field<uintptr_t>(actor,0x214);if(!actor_unit)return false;
        // After original combat preparation, native targets own BOTH actors.
        // No WASD intent, facing override, or animation flags are applied then.
        if(field<unsigned char>(actor,0xf0)!=1)return false;
        if((player?melee_handoff78:melee_opponent_motion78).native_playback)return true;
        float dt=field<float>(base,0x15b2448);if(!std::isfinite(dt)||dt<.001f||dt>.2f)return true;
        uintptr_t config=field<uintptr_t>(actor,0x20c);if(!config)return false;
        float turn_rate=field<float>(config,0x6c);if(!std::isfinite(turn_rate)||turn_rate<0||turn_rate>100)return false;
        float x=field<float>(actor,0x48),z=field<float>(actor,0x50);
        if(request.phase==melee60::Aligning){
            auto witness=field<uintptr_t>(actor,0x688);if(!witness||field<unsigned char>(witness,0xc)!=1)return true;
            if(field<unsigned char>(witness,0xd1)==1)return true; // Native root motion owns playback immediately.
            auto a=field<uintptr_t>(witness,0x30),b=field<uintptr_t>(witness,0x34);
            if(field<unsigned>(witness,0x38)!=2||(a!=actor&&b!=actor))return true;
            auto other_actor=a==actor?b:a;auto other_kind=player?request.opponent_kind:request.player_kind;
            if(!other_actor||!actor152::kind(base,field<uintptr_t>(other_actor,0),other_kind)||field<uintptr_t>(other_actor,0x688)!=witness)return true;
            auto other_unit=field<uintptr_t>(other_actor,0x214);if(!other_unit)return true;
            if(field<unsigned>(other_actor,0x1c8)!=(player?request.opponent:request.player)||field<unsigned>(other_unit,4)!=(player?request.opponent_unit:request.player_unit))return true;
            // Read the targets registered by ORIGINAL preparation on this actor,
            // rather than following formation orders or pulling the pair apart.
            float tx=field<float>(actor,0x69c),tz=field<float>(actor,0x6a4),distance=std::hypot(tx-x,tz-z);
            float speed=reinterpret_cast<UnitSpeed>(base+0x18b470)(reinterpret_cast<void*>(actor_unit),0);
            float maximum_distance=(request.player_kind||request.opponent_kind)?30.f:8.f;
            if(!std::isfinite(distance)||distance>maximum_distance||!std::isfinite(speed)||speed<0||speed>100)return true;
            float step=std::fmin(distance,speed*dt),factor=distance>.0001f?step/distance:0;
            float target[2]={x+(tx-x)*factor,z+(tz-z)*factor};
            auto bearing=field<unsigned short>(actor,0x64);float desired=field<unsigned short>(actor,0x6a8)*6.28318530718f/65536.f;
            unsigned heading=static_cast<unsigned short>(bearing+face_view(bearing,desired,dt,turn_rate));
            for(unsigned offset:{0x70u,0x74u,0x78u,0x7cu,0xd8u,0xdcu,0xe0u})field<float>(actor,offset)=0;
            reinterpret_cast<NativeIntent>(base+0x3936f0)(reinterpret_cast<void*>(actor),target,heading);
            return true; // Original integration/collisions/terrain still execute.
        }
        const float* other=player?request.opponent_position:request.player_position;
        float desired=melee60::facing(x,z,other[0],other[2]);unsigned bearing=field<unsigned short>(actor,0x64);
        float position[2]={x,z};unsigned heading=static_cast<unsigned short>(bearing+face_view(bearing,desired,dt,turn_rate));
        reinterpret_cast<NativeIntent>(base+0x3936f0)(reinterpret_cast<void*>(actor),position,heading);
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// A registered QTE pair is kinematic only while answering the prompt. Leave
// positions intact, so their existing native spatial memberships remain valid.
// The outer native actor update still owns skeleton/render bookkeeping. No
// integration (including collision acceleration) may translate this pair here.
static volatile LONG melee_hold_updates;
static bool hold_melee_motion(uintptr_t actor){
    melee60::Request r;
    if(!enabled||!state||GetTickCount64()-state->heartbeat>500||!melee_request(r)||r.phase!=melee60::Facing)return false;
    __try{
        bool player,enemy;uintptr_t rider;if(!melee_motion_identity(actor,r,player,enemy,rider)||field<uintptr_t>(actor,0x18))return false;
        observe_melee_handoff78(actor,r,player);
        // A queued Facing publication may lag the engine's playback flag.
        // Never write the old anchor over live animation root motion.
        if((player?melee_handoff78:melee_opponent_motion78).native_playback){InterlockedIncrement(&melee_playback_hold_bypasses78);return false;}
        const auto* anchor=player?r.player_anchor:r.opponent_anchor;
        bool native_anchor=(player?r.player_kind:r.opponent_kind)==1;
        float offset_x=native_anchor?0:field<float>(rider,0x48)-field<float>(actor,0x48),offset_z=native_anchor?0:field<float>(rider,0x50)-field<float>(actor,0x50);
        if(!std::isfinite(offset_x)||!std::isfinite(offset_z)||std::hypot(offset_x,offset_z)>4)return false;
        float x=anchor[0]-offset_x,z=anchor[2]-offset_z;
        float drift=std::hypot(field<float>(actor,0x48)-x,field<float>(actor,0x50)-z);
        if(!std::isfinite(drift)||drift>8)return false;
        // Pin the admitted horizontal roots as well as suppressing integration.
        // Formation updates can otherwise move a participant outside this hook.
        field<float>(actor,0x48)=x;field<float>(actor,0x50)=z;
        float dt=field<float>(base,0x15b2448);auto config=field<uintptr_t>(actor,0x20c);
        float rate=config?field<float>(config,0x6c):0;
        // Invalid or large frame deltas suppress facing only, never the hold.
        if(config&&std::isfinite(rate)&&rate>=0&&rate<=100&&std::isfinite(dt)&&dt>=.001f&&dt<=.2f){
            const auto* other=player?r.opponent_position:r.player_position;
            auto bearing=field<unsigned short>(actor,0x64);
            float desired=melee60::facing(field<float>(actor,0x48),field<float>(actor,0x50),other[0],other[2]);
            field<unsigned short>(actor,0x64)=static_cast<unsigned short>(bearing+face_view(bearing,desired,dt,rate));
        }
        // Discard pending translation/acceleration, rather than accumulating
        // it for a lurch when native combat resumes. Do not change casualties,
        // animation states, collision ownership or the actor's position.
        for(unsigned offset:{0x70u,0x74u,0x78u,0x7cu,0x8cu,0x90u,0x94u,0xd8u,0xdcu,0xe0u})field<float>(actor,offset)=0;
        field<unsigned short>(actor,0xe4)=0;
        InterlockedIncrement(&melee_hold_updates);return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool save_rts_view90(uintptr_t env,uintptr_t manager);
static void entry_failed(unsigned error){
    request_rts_restore90();
    state->acquisition_error=error;InterlockedExchange(&state->acquire,0);
    if(!possessed)InterlockedExchange(&state->mode,IDLE);
}
#include "camera_pick150.h"
static void enter_selected_soldier(){
    if(!state||!camera_entry_live(enabled!=0,state->stop!=0,state->release!=0,
        GetTickCount64()-state->heartbeat,state->mode))return;
    DWORD pid=0,window_thread=GetWindowThreadProcessId(input_window,&pid);
    if(pid!=GetCurrentProcessId()||!camera_entry_thread(GetCurrentThreadId(),window_thread,
        camera_entry_owner(window_thread,state->camera_callback_thread,state->camera_input_thread),camera_callback_depth)){entry_failed(1);return;}
    __try {
        uintptr_t root=field<uintptr_t>(base,0x1bc8180);
        uintptr_t env=root?field<uintptr_t>(root,0x110):0;
        uintptr_t common=env?field<uintptr_t>(env,8):0;
        uintptr_t phase=common?field<uintptr_t>(common,0xb4):0;
        unsigned phase_id=phase?field<unsigned>(phase,0x14):0;
        if(phase_id<5||phase_id>8){entry_failed(2);return;}
        uintptr_t selected=field<uintptr_t>(env,0x2826c);
        uintptr_t owner=selected?field<uintptr_t>(selected,0x4a8):0;
        uintptr_t group=owner?owner+0xb0:0;
        if(!group||field<uintptr_t>(group,0)!=base+0x15c4da0||
            field<unsigned>(group,0x14)!=1||field<unsigned>(group,0x10)<1){entry_failed(3);return;}
        uintptr_t members=field<uintptr_t>(group,0x18);
        uintptr_t selected_unit=members?field<uintptr_t>(members,0):0;
        if(!selected_unit){entry_failed(4);return;}
        uintptr_t manager=field<uintptr_t>(env,0x281fc);
        if(!manager||field<uintptr_t>(manager,0)!=base+0x15d26fc){entry_failed(6);return;}
        uintptr_t view=field<uintptr_t>(manager,0x8f8);
        if(!view){entry_failed(5);return;}
        uintptr_t camera=field<uintptr_t>(manager,0x8e0);
        if(!camera){entry_failed(7);return;}
        if(field<uintptr_t>(camera,0)==base+0x15d22e8){
            uintptr_t actor=field<uintptr_t>(camera,0x188);
            if(actor&&adopt_possession97(actor)){state->acquisition_error=0;return;}
            entry_failed(8);return;
        }
        // Classic overhead camera is the current +8cc slot, not +8d4.
        // Verified live and by the game's return-to-overhead RVA2d5fc0.
        if(camera!=field<uintptr_t>(manager,0x8cc)||field<uintptr_t>(camera,0)!=base+0x15d1a74){entry_failed(9);return;}
        unsigned count=field<unsigned>(view,0x18);
        uintptr_t array=field<uintptr_t>(view,0x1c);
        if(!array||!count||count>1000){entry_failed(10);return;}
        auto center_actor150=camera_center_actor150(env,phase,camera);
        bool live=false;Choice97 choice{};int choice_score98=-1;
        for(unsigned i=0;i<count;++i){
            uintptr_t holder=field<uintptr_t>(array,i*4),actor=holder?field<uintptr_t>(holder,8):0;
            // Validate the selected view by actor backlinks without indexing a
            // global view array whose capacity has not yet been recovered.
            // A real view also contains noncombat special models. Those do not
            // disqualify its eligible regular soldiers. Every actor must still
            // belong to this one selected unit, and only a valid live regular
            // infantry actor may satisfy the candidate check below or the
            // post-construction followed() identity check.
            if(actor&&field<uintptr_t>(actor,0x214)!=selected_unit){entry_failed(15);return;}
            NativeSubject motion;if(actor&&native_subject(actor,selected_unit,view,false,motion)){
                auto witness=field<uintptr_t>(actor,0x688);if(witness&&field<unsigned char>(witness,0xd1))continue;
                live=true;int score98=wall_subject97(actor)?2:0;if(actor==field<uintptr_t>(manager,0x8fc))++score98;
                if(center_actor150&&(actor==center_actor150||motion.actor==center_actor150))score98+=4;
                if(score98>choice_score98){choice_score98=score98;choice={env,phase,manager,view,selected_unit,holder,actor,field<unsigned>(actor,0x1c8),field<unsigned>(actor,0x218)};}
            }
        }
        if(!live){entry_failed(11);return;}
        // Window-message boundary on the observed camera/window owner thread,
        // outside this thread's borrowed camera and native input callbacks.
        // The original native function constructs and selects the soldier camera.
        if(!save_rts_view90(env,manager)){entry_failed(16);return;}
        choice97=choice;
        __try{original_toggle(reinterpret_cast<void*>(manager),1);}
        __finally{choice97={};}
        camera=field<uintptr_t>(manager,0x8e0);
        uintptr_t actor=camera?field<uintptr_t>(camera,0x188):0;
        if(camera&&field<uintptr_t>(camera,0)==base+0x15d22e8&&actor&&adopt_possession97(actor)){
            ++state->camera_acquisitions;state->acquisition_error=0;return;
        }
        entry_failed(12);
        if(camera&&camera==field<uintptr_t>(manager,0x8e4)&&field<uintptr_t>(camera,0)==base+0x15d22e8)
            original_toggle(reinterpret_cast<void*>(manager),0);
    }__except(EXCEPTION_EXECUTE_HANDLER){entry_failed(13);}
}
#include "friendly_fire109.h"
#include "native_bow.h"
static void pose_manual_origin(float* origin);
static bool posed_arrow_origin(float* origin);
#include "weapon_animation.h"
#include "formation_arrival129.h"
#include "unit_detach130.h"
#include "equipment_native81.h"
#include "mount154.h"
// Revision40 attachment predicate was not reached for the display arrow.
#include "mounted_render_clock.h"
#include "climb_control97.h"
#include "soldier_pose.h"
#include "third_person152.h"
static void equipment_render_reset81(){
    AcquireSRWLockExclusive(&pose_lock);mounted_transition71.reset();mounted_held_palette.reset();
    pose_muzzle_transform_valid=false;pose_carbine_valid=false;ReleaseSRWLockExclusive(&pose_lock);
}
#include "first_person_interface.h"
#include "rts_camera_native90.h"
#include "native_commands100.h"
#include "native_charge139.h"
static void __fastcall camera_cancel(void* camera,void*) {
    uintptr_t caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
    InterlockedIncrement(reinterpret_cast<volatile LONG*>(&state->camera_cancels));
    state->cancel_caller=caller>=base&&caller<base+0x1d88000?unsigned(caller-base):0;
    if(camera_owned(reinterpret_cast<uintptr_t>(camera))){InterlockedIncrement(reinterpret_cast<volatile LONG*>(&state->camera_suppressed));return;}
    original_cancel(camera);
}
static bool retain_context(uintptr_t manager) {
    __try {return camera_owned(field<uintptr_t>(manager,0x8e0));}__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// Native projectile-camera activation takes no stack arguments.
static void __fastcall projectile_camera(void* manager,void*) {
    __try {
        if(state->mode==POSSESSED&&retain_context(reinterpret_cast<uintptr_t>(manager))){
            field<uint32_t>(reinterpret_cast<uintptr_t>(manager),0xab4)=0xffffffffu;
            field<float>(reinterpret_cast<uintptr_t>(manager),0xab8)=0;
            ++state->projectile_camera_suppressed;return;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){state->error=23;}
    original_projectile_camera(manager);
}
static void __fastcall camera_toggle(void* manager,void*,unsigned active) {
    if(!active && retain_context(reinterpret_cast<uintptr_t>(manager))){InterlockedIncrement(reinterpret_cast<volatile LONG*>(&state->camera_suppressed));return;}
    original_toggle(manager,active);
}
static void remember_native_eye78(uintptr_t camera){
    __try{
    if(!state||state->mode!=POSSESSED||dead_view()||!camera_owned(camera))return;
    ModelEye sample;sample.lease=possession_lease.generation;sample.actor=possession_lease.identity.actor_id;sample.unit=possession_lease.identity.unit_id;sample.tick=GetTickCount64();
    memcpy(sample.eye.view,reinterpret_cast<void*>(camera+8),sizeof sample.eye.view);
    memcpy(sample.eye.up,reinterpret_cast<void*>(camera+0x58),sizeof sample.eye.up);
    float yaw{},pitch{};if(animationview::angles(sample.eye,yaw,pitch))publish_model_eye(native_eye_channel78,sample);
    }__except(EXCEPTION_EXECUTE_HANDLER){}
}
static void adjust_eye(uintptr_t camera) {
    __try {
        animation_view_active78=false;
        if(state->mode!=POSSESSED||!camera_owned(camera))return;
        if(dead_view()&&death_eye_valid){
            update_death_model_view();
            for(unsigned i=0;i<6;++i)field<float>(camera,8+i*4)=death_eye[i];
            for(unsigned i=0;i<3;++i)field<float>(camera,0x58+i*4)=death_up[i];
            if(death_third_person156){
                deathpose::Eye eye;memcpy(eye.view,death_eye,24);memcpy(eye.up,death_up,12);
                if(!view_facing156(eye,state->yaw,state->pitch))return;
                memcpy(reinterpret_cast<void*>(camera+8),eye.view,24);memcpy(reinterpret_cast<void*>(camera+0x58),eye.up,12);
            }
            auto refresh=reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4));
            refresh(reinterpret_cast<void*>(camera));return;
        }
        InterlockedExchange(&mount_camera_locked154,0);
        if(dismount_native_motion143()){
            deathpose::Eye native_mount_eye154;memcpy(native_mount_eye154.view,reinterpret_cast<void*>(camera+8),24);memcpy(native_mount_eye154.up,reinterpret_cast<void*>(camera+0x58),12);
            float mount_yaw154{},mount_pitch154{};
            if(animationview::angles(native_mount_eye154,mount_yaw154,mount_pitch154)){
                state->yaw=mount_yaw154;state->pitch=mount_pitch154;
                memcpy(animation_view_up78,native_mount_eye154.up,12);animation_view_active78=true;
                InterlockedExchange(&mount_camera_locked154,1);
            }
            remember_owned_eye(camera);return;
        }
        float yaw=state->yaw,pitch=state->pitch;
        melee60::Request committed;bool commitment=melee_request(committed);
        bool paired156=commitment||(possession_lease.active&&melee_handoff78.matches(locomotion_actor(),possession_lease.generation,
            possession_lease.identity.mount?possession_lease.identity.mount_id:possession_lease.identity.actor_id,possession_lease.identity.unit_id));
        if(commitment)InterlockedExchange(&entry_movement125,0);
        if(paired156&&!third_person152){
            deathpose::Eye native_eye;
            memcpy(native_eye.view,reinterpret_cast<void*>(camera+8),sizeof native_eye.view);
            memcpy(native_eye.up,reinterpret_cast<void*>(camera+0x58),sizeof native_eye.up);
            if(!animationview::angles(native_eye,yaw,pitch))return;
            deathpose::Eye free_eye;
            if(commitment){
                InterlockedExchange(&entry_movement125,0);
                if(combat_focus125.begin(committed)){
                    eye_reset152();
                    InterlockedExchange(&combat_entry_locked126,1);
                    InterlockedExchange(&raw_mouse_x,0);InterlockedExchange(&raw_mouse_y,0);
                }
            }
            if(combat_focus125.valid&&!combat_focus125.unlocked){
                // The untouched native eye follows the facing/engagement animation.
                // Capture that exact view at unlock; do not snap or run a separate turn.
                state->yaw=yaw;state->pitch=pitch;
                bool locked=commitment?combat_pair_ready126(committed):
                    (melee_handoff78.native_playback&&melee_handoff78.encounter==combat_focus125.serial);
                if(combat_focus125.release(locked,state->yaw)){
                    InterlockedExchange(&raw_mouse_x,0);InterlockedExchange(&raw_mouse_y,0);
                }
            }
            if(combat_focus125.valid){
                if(!combat_focus125.unlocked)free_eye=native_eye;
                else if(!eye_apply152(native_eye,free_eye))return;
            }else if(!eye_apply152(native_eye,free_eye))return;
            if(!animationview::angles(free_eye,yaw,pitch))return;
            state->yaw=yaw;state->pitch=pitch;InterlockedExchange(&combat_entry_locked126,combat_focus125.valid&&!combat_focus125.unlocked?1:0);animation_view_active78=true;InterlockedExchange(&animation_look_active104,1);InterlockedIncrement(&animation_look_updates104);
            memcpy(reinterpret_cast<void*>(camera+8),free_eye.view,sizeof free_eye.view);
            memcpy(reinterpret_cast<void*>(camera+0x58),free_eye.up,sizeof free_eye.up);
            native_eye=free_eye;
            auto refresh=reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4));
            refresh(reinterpret_cast<void*>(camera));
            memcpy(animation_view_up78,native_eye.up,sizeof animation_view_up78);
            state->native_eye_y=state->raised_eye_y=field<float>(camera,0xc);
            remember_owned_eye(camera);InterlockedIncrement(&animation_view_updates78);return;
        }
        InterlockedExchange(&combat_entry_locked126,0);
        InterlockedExchange(&animation_look_active104,0);
        if(!std::isfinite(yaw)||!std::isfinite(pitch))return;
        if(possession_lease.identity.mount&&!(third_person152&&paired156)){
            unsigned bearing=field<unsigned short>(possession_lease.identity.mount,0x64);
            InterlockedExchange(&mounted_heading85,LONG(bearing));
            yaw=mountedlook85::constrain(yaw,bearing);state->yaw=yaw;
        }
        state->native_eye_y=field<float>(camera,0xc);
        field<float>(camera,0xc)+=0.0254f;state->raised_eye_y=field<float>(camera,0xc);
        float cp=std::cos(pitch);
        field<float>(camera,0x14)=field<float>(camera,8)+std::sin(yaw)*cp;
        field<float>(camera,0x18)=field<float>(camera,0xc)+std::sin(pitch);
        field<float>(camera,0x1c)=field<float>(camera,0x10)+std::cos(yaw)*cp;
        // Refresh the verified native direction cache through this camera's vtable.
        auto refresh=reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4));
        refresh(reinterpret_cast<void*>(camera));
        remember_owned_eye(camera);
    }__except(EXCEPTION_EXECUTE_HANDLER){InterlockedExchange(&state->error,22);}
}
static void __fastcall camera_matrix(void* camera,void*,const void* matrix) {
    state->camera_callback_thread=GetCurrentThreadId();
    ++camera_callback_depth;
    // The renderer still evaluates cameras while paused; retire stale ownership
    // here rather than waiting for another native motion update.
    __try {
        if(InterlockedExchange(&state->release,0)){relinquish();InterlockedExchange(&state->acquire,0);}
        if(possessed&&!confirm_follow93(possessed)){note_follow_release93();++state->identity_releases;relinquish();}
        if(InterlockedExchange(&dismount_cleanup143,0)){
            auto selected_ability143=ability_selected141;
            charge_reset139();equipment_reset81();ability_lease141=state->lease_generation;ability_selected141=selected_ability143;mounted_draw={};weapon_actor=weapon_context=0;
            InterlockedExchange(&state->fire_ticket,0);InterlockedExchange(&state->fire_request,0);
            InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);state->aim_pose=0;
            InterlockedExchange(&mounted_heading85,-1);InterlockedExchange(&entry_movement125,0);
            combat_focus125.reset();combat_pair_reset126();command_reset100();
        }
        mount_prepare154(reinterpret_cast<uintptr_t>(camera));view_prepare152();original_matrix(camera,matrix);if(possessed&&!dead_view())climb_observe98(possessed);remember_native_eye78(reinterpret_cast<uintptr_t>(camera));adjust_eye(reinterpret_cast<uintptr_t>(camera));view_camera152(reinterpret_cast<uintptr_t>(camera));interface_update(reinterpret_cast<uintptr_t>(camera));
        if(!dead_view()){
            capture_aim_frame(reinterpret_cast<uintptr_t>(camera));
            process_orders(reinterpret_cast<uintptr_t>(camera));
            if(!possessed||(!wall_subject97(possessed)&&!native_airborne_subject99(possessed))){
            process_equipment_commands81(reinterpret_cast<uintptr_t>(camera));

            process_bow(reinterpret_cast<uintptr_t>(camera));
            }
        }
    }__finally {--camera_callback_depth;}
}
static bool death_camera_update156(uintptr_t camera){
    if(!camera_owned(camera)||!dead_view())return false;
    ++camera_callback_depth;
    __try{
        adjust_eye(camera);view_camera152(camera);interface_update(camera);
        // Same native matrix publication as325ca0, using the retained numeric
        // eye rather than querying a retired corpse or the unit's next soldier.
        reinterpret_cast<Cancel>(base+0x134f1d0)(reinterpret_cast<void*>(camera));
        return death_context(camera);
    }__finally{--camera_callback_depth;}
}
static void relinquish(){InterlockedExchange(&mount_camera_locked154,0);InterlockedExchange(&mount_request154,0);InterlockedExchange(&routing_empty_lease154,0);view_reset152();InterlockedExchange(&dismount_cleanup143,0);charge_reset139();InterlockedExchange(&entry_movement125,0);combat_focus125.reset();combat_pair_reset126();InterlockedExchange(&combat_entry_locked126,0);InterlockedExchange(&animation_look_active104,0);command_reset100();climb_reset97();request_rts_restore90();next_accuracy90=0;publish_accuracy90(nullptr,0,0);InterlockedExchange(&mounted_heading85,-1);equipment_reset81();reset_death_view();InterlockedExchange(&manual_arrow_released,0);state->arrow_visual_hidden=0;possession_lease.release();InterlockedExchange(&state->input,0);InterlockedExchange(&state->aiming,0);state->aim_pose=0;InterlockedExchange(&state->weapon_phase,WEAPON_IDLE);possessed=0;environment=0;unit=0;InterlockedExchange(&state->follow,0);InterlockedExchange(&state->pending_order,0);follow_target=false;InterlockedExchange(&state->fire_request,0);InterlockedExchange(&state->mode,IDLE);}
static void end_death_on_window(){
    if(!InterlockedExchange(&death_return_pending,0))return;
    __try{uintptr_t manager=possession_lease.identity.manager;uintptr_t env=environment;
        uintptr_t camera=possession_lease.identity.camera;
        bool same=env&&manager&&camera&&field<uintptr_t>(env,0x281fc)==manager&&
            field<uintptr_t>(manager,0x8e0)==camera&&field<uintptr_t>(camera,0)==base+0x15d22e8&&
            field<uintptr_t>(camera,0x180)==manager;
        relinquish();if(same)original_cancel(reinterpret_cast<void*>(camera));
    }__except(EXCEPTION_EXECUTE_HANDLER){relinquish();}
}
static bool prepare(uintptr_t actor) {
    __try {
        if(!enabled || state->stop || GetTickCount64()-state->heartbeat>500){relinquish();return false;}
        if(InterlockedExchange(&state->release,0)){relinquish();InterlockedExchange(&state->acquire,0);}
        if(state->acquire&&!possessed)adopt_possession97(actor);
        if((actor==possessed||actor==possession_lease.identity.mount)&&dismount_native_motion143())return false; // Original dismount owns movement/animation.
        // A charge remains on rails, but original encounter registration can
        // now require its existing paired alignment before native playback.
        // Do not let the approach-only yield skip that proved handoff.
        if(apply_melee_motion(actor)){if(actor==locomotion_actor()){if(!charge_input_locked146())charge_reset139();InterlockedExchange(&entry_movement125,0);}return false;}
        if(charge_motion146(actor)){InterlockedExchange(&entry_movement125,0);return false;}
        if(actor!=locomotion_actor())return false;
        if(!confirm_follow93(possessed)){note_follow_release93();++state->identity_releases;relinquish();return false;}
        if(dead_view())return false;
        // Native horse jumping owns translation/animation through completion.
        // Manual WASD must not overwrite the stock launch, arc or landing.
        if(native_mounted_jump140(actor)){charge_reset139();InterlockedExchange(&entry_movement125,0);return false;}
        if(recovery_prepare103(actor)){charge_reset139();InterlockedExchange(&entry_movement125,0);return false;}
        if(climb_prepare97(actor)){charge_reset139();InterlockedExchange(&entry_movement125,0);return false;}
        if(finish_melee_handoff78(actor)){charge_reset139();InterlockedExchange(&entry_movement125,0);return false;}
        // The human request owns mounted combat too. During that commitment
        // WASD cannot override a horse's native matched-combat movement.
        melee60::Request mount_commitment;
        if(possession_lease.identity.mount&&melee_request(mount_commitment)){InterlockedExchange(&entry_movement125,0);return false;}
        unsigned bearing=field<unsigned short>(actor,0x64);
        if(possession_lease.identity.mount){
            InterlockedExchange(&mounted_heading85,LONG(bearing));
            state->yaw=mountedlook85::constrain(state->yaw,bearing);
        }
        float dt=field<float>(base,0x15b2448);
        // A zero simulation delta pauses control without losing the possession.
        if(std::isfinite(dt)&&dt>=0&&dt<0.001f)return false;
        if(!std::isfinite(dt)||dt<0||dt>0.2f){relinquish();return false;}
        float yaw=state->yaw;
        if(!std::isfinite(yaw)){relinquish();return false;}
        bool crouch_locked=equipment_prepare_motion81();
        if(crouch_locked)InterlockedExchange(&entry_movement125,0);
        unsigned movement_input=equipment81::movement_input(unsigned(state->input),crouch_locked);
        float speed=0,turn_rate=0;
        if(!read_native_movement(movement_input,speed,turn_rate)){state->error=24;relinquish();return false;}
        bool mounted=possession_lease.identity.mount!=0;
        Intent direction=mounted?steering_mount(movement_input,bearing,dt,1/dt,turn_rate):
            steering_view(movement_input,yaw,1,1);
        bool requested=direction.x!=0||direction.z!=0;
        if(!requested){
            float vx=field<float>(actor,0x8c),vz=field<float>(actor,0x94);
            if(!std::isfinite(vx)||!std::isfinite(vz)){relinquish();return false;}
            float magnitude=std::hypot(vx,vz);
            if(magnitude>0){direction.x=vx/magnitude;direction.z=vz/magnitude;}
        }
        float modifier=1;
        if(!read_native_speed_modifiers(actor,movement_input,speed,modifier)){
            ++state->speed_modifier_errors;state->error=98;relinquish();return false;
        }
        speed*=modifier;
        if(!apply_native_terrain(actor,direction,dt,speed)){
            ++state->terrain_errors;state->error=97;relinquish();return false;
        }
        float rapid_ceiling136=cap_rapid_pace136(movement_input,speed);
        if(!apply_native_speed_step(actor,direction,requested,dt,modifier*state->terrain_speed_factor,speed)){
            ++state->speed_modifier_errors;state->error=99;relinquish();return false;
        }
        if(rapid_ceiling136>=0){speed=std::min(speed,rapid_ceiling136);state->effective_speed=speed;}
        if(crouch_locked){
            // A kneeling firing posture has no walking locomotion. Stop owned
            // horizontal intent and residual velocity; retain vertical physics.
            for(unsigned offset:{0x70u,0x78u,0x8cu,0x94u,0xd8u,0xe0u})field<float>(actor,offset)=0;
            speed=0;state->effective_speed=state->requested_speed=0;
        }
        Intent intent={direction.x*speed*dt,direction.z*speed*dt,0};
        intent.turn=mounted?direction.turn:face_view(bearing,yaw,dt,turn_rate);
        InterlockedIncrement(&state->sequence);
        for(unsigned i=0;i<3;++i)state->before[i]=field<float>(actor,0x48+i*4);
        if(!issue_native_intent(actor,intent,bearing)){
            ++state->native_intent_errors;InterlockedIncrement(&state->sequence);relinquish();return false;
        }
        state->bearing=bearing;return true;
    } __except(EXCEPTION_EXECUTE_HANDLER){if(state->sequence&1)InterlockedIncrement(&state->sequence);relinquish();InterlockedExchange(&state->error,20);return false;}
}
static void finish(uintptr_t actor) {
    __try {
        if(possession_lease.identity.mount)InterlockedExchange(&mounted_heading85,LONG(field<unsigned short>(actor,0x64)));
        for(unsigned i=0;i<3;++i){state->after[i]=field<float>(actor,0x48+i*4);state->velocity[i]=field<float>(actor,0x8c+i*4);}
        // The caller computes this native animation driver after the integrator;
        // telemetry here observes its previous completed update.
        state->motion_animation_speed=field<float>(actor,0x1ac);
        ++state->controlled_updates;++state->frame;state->last_motion_tick=GetTickCount64();
        InterlockedIncrement(&state->sequence);
    } __except(EXCEPTION_EXECUTE_HANDLER){if(state->sequence&1)InterlockedIncrement(&state->sequence);relinquish();InterlockedExchange(&state->error,21);}
}
static void __fastcall detour(void* actor,void*) {
    // Record the thread that actually invokes the original native integrator,
    // for telemetry only. This worker thread does not establish camera affinity.
    DWORD current=GetCurrentThreadId();
    if(!state->native_motion_thread)InterlockedCompareExchange(
        reinterpret_cast<volatile LONG*>(&state->native_motion_thread),LONG(current),0);
    InterlockedIncrement(&motion_callback_depth);
    __try {
        if(!charge_input_locked146()&&hold_melee_motion(reinterpret_cast<uintptr_t>(actor))){
            InterlockedIncrement(reinterpret_cast<volatile LONG*>(&state->other_updates));return;
        }
        bool controlled=prepare(reinterpret_cast<uintptr_t>(actor));
        original(actor);
        if(controlled){finish(reinterpret_cast<uintptr_t>(actor));if(!possession_lease.identity.mount){refresh_accuracy90(reinterpret_cast<uintptr_t>(actor));weapon_tick(reinterpret_cast<uintptr_t>(actor));}}
        else InterlockedIncrement(reinterpret_cast<volatile LONG*>(&state->other_updates));
        if(possession_lease.active&&possession_lease.identity.mount&&reinterpret_cast<uintptr_t>(actor)==possessed&&
           followed(possessed,false)&&!dead_view()){refresh_accuracy90(possessed);weapon_tick(possessed);}
    }__finally {InterlockedDecrement(&motion_callback_depth);}
}
static bool down(int vk){return (GetAsyncKeyState(vk)&0x8000)!=0;}
static void snapshot_log(std::ostream& log) {
    log<<"{\"event\":\"friendly_fire109\",\"tick_ms\":"<<GetTickCount64()<<",\"hits\":"<<friendly_hits109<<",\"errors\":"<<friendly_errors109<<"}\n";
    nativeoverlay106::log(log);
    command_log100(log);
    charge_log139(log);
    proximity_log147(log);
    log<<"{\"event\":\"native_input143\",\"tick_ms\":"<<GetTickCount64()<<",\"dismount_transitions\":"<<dismount_transitions143<<",\"native_bow_audio145\":1,\"ability_sound_requests\":"<<ability_sound_played143<<",\"audio_errors\":"<<audio_errors143<<"}\n";
    {static LONG prior=-1,failed=-1;LONG blocked=keyboard_blocked84,errors=keyboard_failures84;
     if(blocked!=prior||errors!=failed){prior=blocked;failed=errors;
      log<<"{\"event\":\"native_keyboard84\",\"tick_ms\":"<<GetTickCount64()
       <<",\"polls\":"<<keyboard_polls84<<",\"blocked_polls\":"<<blocked
       <<",\"held_mask\":"<<keyboard_held84<<",\"last_mask\":"<<keyboard_last_mask84
       <<",\"native_failed_polls\":"<<errors<<"}\n";}}
    {static LONG previous=-1;auto count=melee_handoff_updates78;
     if(count!=previous){previous=count;log<<"{\"event\":\"animation_handoff78\",\"tick_ms\":"<<GetTickCount64()
        <<",\"handoffs\":"<<count<<",\"native_view_updates\":"<<animation_view_updates78<<",\"playback_anchor_bypasses\":"<<melee_playback_hold_bypasses78
        <<",\"root\":["<<melee_handoff_root78[0]<<','<<melee_handoff_root78[1]<<','<<melee_handoff_root78[2]<<"]}\n";}}
    log<<"{\"event\":\"control154\",\"tick_ms\":"<<GetTickCount64()<<",\"routing_empty_holds\":"<<routing_empty_holds154<<",\"mount_orders\":"<<mount_orders154<<",\"mount_unavailable\":"<<mount_unavailable154<<"}\n";
    release_log103(log);log_mounted_pose_diagnostic(log);log_mounted_sequence69(log);log_mounted_render70(log);log_weapon_observation82(log);log_mounted_pose_audit87(log);
    {static LONG seen=-1;LONG blocks=mounted_carbine_predraw_blocks72;
      if(blocks!=seen){seen=blocks;
        log<<"{\"event\":\"mounted_carbine_gate72\",\"tick_ms\":"<<GetTickCount64()
           <<",\"predraw_blocks\":"<<blocks<<"}\n";}}
    {static LONG prior_updates=-1,prior_rejections=-1;LONG u=mounted_transition_updates71,r=mounted_transition_rejections71;
      if(u!=prior_updates||r!=prior_rejections){prior_updates=u;prior_rejections=r;
        log<<"{\"event\":\"mounted_transition71\",\"tick_ms\":"<<GetTickCount64()<<",\"updates\":"<<u
           <<",\"blended_updates\":"<<mounted_transition_blends71<<",\"rejections\":"<<r<<"}\n";}}
    { static LONG held_seen=-1,fail_seen=-1;LONG h=mounted_palette_holds,f=mounted_palette_failures;
      if(h!=held_seen||f!=fail_seen){held_seen=h;fail_seen=f;
        log<<"{\"event\":\"mounted_palette69\",\"tick_ms\":"<<GetTickCount64()<<",\"holds\":"<<h<<",\"failures\":"<<f<<"}\n";}}

    Status s{};bool ok=false;
    for(int i=0;i<4;++i){LONG seq=state->sequence;if(seq&1)continue;MemoryBarrier();memcpy(&s,state,sizeof(s));MemoryBarrier();if(seq==state->sequence){ok=true;break;}}
    if(!ok)return;
    log<<"{\"event\":\"control_metrics\",\"tick_ms\":"<<GetTickCount64()<<",\"mode\":"<<s.mode;
    control_metrics(log,s);accuracy_log90(log);climb_log97(log);rapid_pace_log136(log);
    log<<",\"context103\":1,\"camera_cycle_blocks103\":"<<camera_cycle_blocks103<<",\"camera_input_blocks103\":"<<camera_input_blocks103<<",\"cursor_self_hits103\":"<<cursor_self_hits103<<",\"cursor_queries103\":"<<cursor_queries103<<",\"cursor_errors103\":"<<cursor_errors103<<",\"command_target_samples103\":"<<command_target_samples103<<",\"command_target_kind103\":"<<command_target_kind103<<",\"command_target_unit103\":"<<command_target_unit103<<",\"recovery103\":1,\"recovery_blocks103\":"<<recovery_blocks103<<",\"recovery_active103\":"<<recovery_active103;
    log<<",\"ranged_barrier96\":1,\"lower_barrier_bypasses96\":"<<lower_barrier_bypasses96<<",\"idle_barrier_bypasses96\":"<<idle_barrier_bypasses96<<",\"look_pitch96\":1,\"ranged_barrier95\":1,\"animation_barrier_bypasses95\":"<<animation_barrier_bypasses95<<",\"same_unit_ai_draws95\":"<<same_unit_ai_draws95<<",\"same_unit_ai_releases95\":"<<same_unit_ai_releases95;
    log<<",\"release_stage93\":"<<release_stage93<<",\"release_confirmed93\":"<<release_confirmed93<<",\"release_recovered93\":"<<release_recovered93<<",\"release_tick93\":"<<InterlockedCompareExchange64(&release_tick93,0,0)<<",\"release_flags93\":"<<release_flags93;
    log<<"}\n";
    log<<"{\"event\":\"death_view\",\"tick_ms\":"<<GetTickCount64()<<",\"active\":"<<(dead_view()?"true":"false")
        <<",\"elapsed_ms\":"<<death_elapsed()<<",\"eyelid_fraction\":"<<melee60::eyelid_fraction(death_elapsed())
        <<",\"model_tracking\":"<<(death_model_valid?"true":"false")
        <<",\"eye\":["<<death_eye[0]<<','<<death_eye[1]<<','<<death_eye[2]<<"]"
        <<",\"up\":["<<death_up[0]<<','<<death_up[1]<<','<<death_up[2]<<"]"
        <<",\"lease\":"<<s.lease_generation<<",\"actor_id\":"<<s.actor_id<<",\"unit_id\":"<<s.unit_id<<"}\n";
    log<<"{\"tick_ms\":"<<GetTickCount64()<<",\"mode\":"<<s.mode<<",\"input\":"<<s.input<<",\"frame\":"<<s.frame<<",\"unit_id\":"<<s.unit_id<<",\"actor_index\":"<<s.actor_index
        <<",\"before\":["<<s.before[0]<<','<<s.before[1]<<','<<s.before[2]<<"],\"after\":["<<s.after[0]<<','<<s.after[1]<<','<<s.after[2]
        <<"],\"velocity\":["<<s.velocity[0]<<','<<s.velocity[1]<<','<<s.velocity[2]<<"],\"bearing\":"<<s.bearing<<",\"yaw\":"<<s.yaw<<",\"pitch\":"<<s.pitch<<",\"native_eye_y\":"<<s.native_eye_y<<",\"raised_eye_y\":"<<s.raised_eye_y<<",\"mouse_events\":"<<s.mouse_events<<",\"controlled_updates\":"<<s.controlled_updates<<",\"other_updates\":"<<s.other_updates<<",\"camera_cancels\":"<<s.camera_cancels<<",\"camera_suppressed\":"<<s.camera_suppressed<<",\"cancel_caller_rva\":"<<s.cancel_caller<<",\"orders_sent\":"<<s.orders_sent<<",\"last_order\":"<<s.last_order<<",\"order_error\":"<<s.order_error<<",\"follow\":"<<s.follow<<",\"packet_size\":"<<s.packet_size<<",\"queue_before\":"<<s.queue_before<<",\"queue_after\":"<<s.queue_after<<",\"window_thread\":"<<s.window_thread<<",\"camera_thread\":"<<s.camera_thread<<",\"order_target\":["<<s.order_target_x<<','<<s.order_target_z<<"],\"formation\":["<<s.formation_x<<','<<s.formation_z<<"],\"formation_count\":"<<s.formation_count<<",\"projectile_camera_suppressed\":"<<s.projectile_camera_suppressed<<",\"weapon_phase\":"<<s.weapon_phase<<",\"weapon_record\":"<<s.weapon_record<<",\"weapon_animation\":"<<s.weapon_animation<<",\"weapon_mouse_captured\":"<<s.weapon_mouse_captured<<",\"aiming\":"<<s.aiming<<",\"aim_pose\":"<<s.aim_pose<<",\"aim_blend\":"<<s.aim_blend<<",\"hip_fov\":"<<s.hip_fov<<",\"view_fov\":"<<s.view_fov<<",\"hud_hidden\":"<<s.hud_hidden<<",\"crosshair_visible\":"<<s.crosshair_visible<<",\"pose_muzzle_updates\":"<<s.pose_muzzle_updates<<",\"pose_weapon_record\":"<<s.pose_weapon_record<<",\"pose_native_clip\":"<<s.pose_native_clip<<",\"pose_native_bias\":"<<s.pose_native_bias<<",\"bow_origin_aligned\":"<<s.bow_origin_aligned<<",\"bow_origin_correction\":"<<s.bow_origin_correction<<",\"shot_camera_origin\":["<<s.shot_camera_origin[0]<<','<<s.shot_camera_origin[1]<<','<<s.shot_camera_origin[2]<<']'<<",\"pose_updates\":"<<s.pose_updates<<",\"pose_aim_updates\":"<<s.pose_aim_updates<<",\"pose_restores\":"<<s.pose_restores<<",\"pose_error\":"<<s.pose_error<<",\"pose_bones\":"<<s.pose_bones<<",\"pose_pitch\":"<<s.pose_pitch<<",\"camera_pitch\":"<<s.camera_pitch<<",\"native_matrix_override\":"<<s.native_matrix_override<<",\"raw_mouse_active\":"<<s.raw_mouse_active<<",\"raw_mouse_error\":"<<s.raw_mouse_error<<",\"raw_mouse_events\":"<<s.raw_mouse_events<<",\"raw_aim_events\":"<<s.raw_aim_events<<",\"raw_y_total\":"<<s.raw_y_total<<",\"raw_aim_y_total\":"<<s.raw_aim_y_total<<",\"interface_error\":"<<s.interface_error<<",\"interface_restores\":"<<s.interface_restores<<",\"cursor_suppressed\":"<<s.cursor_suppressed<<",\"aim_clip\":"<<s.aim_clip<<",\"draws_started\":"<<s.draws_started<<",\"reloads_started\":"<<s.reloads_started<<",\"ai_shots_suppressed\":"<<s.ai_shots_suppressed<<",\"weapon_release_time\":"<<s.weapon_release_time<<",\"weapon_reload_remaining\":"<<s.weapon_reload_remaining<<",\"shots_queued\":"<<s.shots_queued<<",\"shots_spawned\":"<<s.shots_spawned<<",\"shots_blocked\":"<<s.shots_blocked<<",\"shots_canceled\":"<<s.shots_canceled<<",\"spread_removed\":"<<s.spread_removed<<",\"direct_fire\":"<<s.direct_fire<<",\"launch_delta\":"<<s.launch_delta<<",\"shot_origin\":["<<s.shot_origin[0]<<','<<s.shot_origin[1]<<','<<s.shot_origin[2]<<']'<<",\"shot_velocity\":["<<s.shot_velocity[0]<<','<<s.shot_velocity[1]<<','<<s.shot_velocity[2]<<']'<<",\"shot_view_direction\":["<<s.shot_view_direction[0]<<','<<s.shot_view_direction[1]<<','<<s.shot_view_direction[2]<<']'<<",\"shot_error\":"<<s.shot_error<<",\"fire_ticket\":"<<s.fire_ticket<<",\"shot_packet_size\":"<<s.shot_packet_size<<",\"shot_thread\":"<<s.shot_thread<<",\"ammo_before\":"<<s.ammo_before<<",\"ammo_after\":"<<s.ammo_after<<",\"shot_target\":["<<s.shot_target[0]<<','<<s.shot_target[1]<<','<<s.shot_target[2]<<"],\"error\":"<<s.error<<"}\n";log.flush();
}
static void mouse_look101(int dx,int dy,unsigned input,float elapsed){
    if(!third_person152&&InterlockedCompareExchange(&combat_entry_locked126,0,0))return; // First-person native pair facing owns the initial view.
    melee60::Request committed;
    if(!dead_view()&&std::abs(dx)<500&&std::abs(dy)<500){
        float turn=float(bool(input&TURN_RIGHT))-float(bool(input&TURN_LEFT));
        if(possession_lease.identity.mount)turn=0; // Mounted keyboard steering does not rotate mouse look.
        float sensitivity=0.0025f*(1.0f-state->aim_blend*0.55f);
        if(orbit_mouse154(float(dx)*sensitivity,-float(dy)*sensitivity)){if(dx||dy)++state->mouse_events;return;}
        if(InterlockedCompareExchange(&mount_camera_locked154,0,0))return;
        LONG heading=InterlockedCompareExchange(&mounted_heading85,0,0);
        if(third_person152&&(melee_request(committed)||InterlockedCompareExchange(&animation_look_active104,0,0)))state->yaw=look104::mouse_yaw(state->yaw,float(dx)*sensitivity);
        else if(melee_request(committed)||InterlockedCompareExchange(&animation_look_active104,0,0)){eye_mouse152(float(dx)*sensitivity,-float(dy)*sensitivity);state->yaw=look104::mouse_yaw(state->yaw,float(dx)*sensitivity);}
        else state->yaw=mountedlook85::mouse_yaw(state->yaw,float(dx)*sensitivity+turn*1.256637f*elapsed,
            possession_lease.identity.mount!=0&&heading>=0,unsigned(heading));
        state->pitch=look96::mouse_pitch(state->pitch,dy,sensitivity);
        if(dx||dy)++state->mouse_events;
    }
}

static DWORD WINAPI worker(void*) {
    storage166::DiscardLog log;
    try {
        view_default160(startup160::third_person());
        wchar_t name[80];mapping_name(name,GetCurrentProcessId());
        mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Status),name);
        require(mapping && GetLastError()!=ERROR_ALREADY_EXISTS,"Prototype already loaded; restart game to reload");
        state=static_cast<Status*>(MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(Status)));require(state!=nullptr,"Control mapping failed");
        memset(state,0,sizeof(Status));state->magic=POC_MAGIC;state->version=POC_VERSION;state->size=sizeof(Status);state->pid=GetCurrentProcessId();
        wchar_t dll_path[MAX_PATH];require(GetModuleFileNameW(self_module,dll_path,MAX_PATH)>0,"DLL path failed");
        equipment_initialize_mapping81();accuracy_initialize90();friendly_initialize109();
        Module engine=find_module(GetCurrentProcessId());base=engine.base;
        const auto engine_sha170=hash_file(engine.path);
        require(engine.size==0x1d88000,"Unsupported engine address layout");
        const auto binding172=engine172::admit(engine.path,engine_sha170);engine172::live_hooks(base,1,binding172);
        require(accuracy_signatures90(),"Native accuracy/projection/camera signatures differ");
        require(native_bow_signatures(base),"Native bow signatures differ");
        require(native_movement_signatures(base),"Native movement signatures differ");
        require(native_speed_modifier_signatures(base),"Native surface/fatigue/speed-step signatures differ");
        require(native_terrain_signatures(base),"Native terrain signatures or constants differ");
        require(equipment_signatures81(base),"Native equipment signatures differ");
        require(command_native_signatures100(base),"Native command100 signatures differ");
        require(native_order_signatures(base),"Native order signatures or serializer tables differ");
        target=reinterpret_cast<void*>(base+0x367600);
        require(memcmp(target,signature,sizeof(signature))==0,"Motion signature changed");
        cancel_target=reinterpret_cast<void*>(base+0x321310);toggle_target=reinterpret_cast<void*>(base+0x319dd0);
        const unsigned char cancel_sig[8]={0xc6,0x81,0x3c,0x01,0x00,0x00,0x01,0xc3};
        const unsigned char toggle_sig[8]={0x80,0x7c,0x24,0x04,0x00,0x56,0x8b,0xf1};
        matrix_target=reinterpret_cast<void*>(base+0x134fad0);
        const unsigned char matrix_sig[8]={0x83,0xec,0x0c,0x56,0x8b,0x74,0x24,0x14};
        require(memcmp(cancel_target,cancel_sig,8)==0&&memcmp(toggle_target,toggle_sig,8)==0,"Camera signatures differ");
        require(memcmp(matrix_target,matrix_sig,8)==0,"Camera matrix signature differs");
        memcpy(state->original_code,target,16);
        require(MH_Initialize()==MH_OK,"MinHook initialization failed");
        require(MH_CreateHook(target,reinterpret_cast<void*>(&detour),reinterpret_cast<void**>(&original))==MH_OK,"Motion hook creation failed");
        require(MH_CreateHook(cancel_target,reinterpret_cast<void*>(&camera_cancel),reinterpret_cast<void**>(&original_cancel))==MH_OK,"Camera cancel hook failed");
        require(MH_CreateHook(toggle_target,reinterpret_cast<void*>(&camera_toggle),reinterpret_cast<void**>(&original_toggle))==MH_OK,"Camera toggle hook failed");
        require(MH_CreateHook(matrix_target,reinterpret_cast<void*>(&camera_matrix),reinterpret_cast<void**>(&original_matrix))==MH_OK,"Camera matrix hook failed");
        camera_input_target=reinterpret_cast<void*>(base+0x2fdcf0);
        const unsigned char input_sig[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57,0x8b,0x46,0x1c};
        require(!memcmp(camera_input_target,input_sig,sizeof input_sig),"Camera input signature differs");
        require(MH_CreateHook(camera_input_target,reinterpret_cast<void*>(&camera_input),reinterpret_cast<void**>(&original_camera_input))==MH_OK,"Camera input hook failed");
        create_keyboard_hook84();
        require(MH_CreateHook(reinterpret_cast<void*>(base+0x2ac0b0),reinterpret_cast<void*>(&bow_dispatch),reinterpret_cast<void**>(&original_bow_dispatch))==MH_OK,"Bow dispatch hook failed");
        projectile_camera_target=reinterpret_cast<void*>(base+0x31f660);
        const unsigned char projectile_camera_sig[8]={0x56,0x8b,0xf1,0x57,0x8d,0xbe,0xec,0x08};
        require(memcmp(projectile_camera_target,projectile_camera_sig,8)==0,"Projectile camera signature differs");
        require(MH_CreateHook(projectile_camera_target,reinterpret_cast<void*>(&projectile_camera),reinterpret_cast<void**>(&original_projectile_camera))==MH_OK,"Projectile camera hook failed");
        charge_mapping146();create_weapon_hooks();create_formation_arrival129();create_unit_detach130();create_mounted_render_hooks70();create_siege_hooks97();create_context_hooks103();

        create_dispersion_hook();create_friendly_hook109();create_follow_voice125();
        create_interface_hooks();
        create_pose_hook();
        create_fatigue_hooks();
        nativeoverlay106::install(&overlay_key106);
        // When replacing the exact stopped revision in this same process, retain
        // its observed native camera thread ID. No native object pointers or
        // ownership transfer; entry still requires the current window owner and
        // revalidates every live camera, selection and actor backlink.
        wchar_t prior_name[80];swprintf_s(prior_name,80,L"Local\\Shogun2PossessionPOC34-%lu",GetCurrentProcessId());
        Handle prior_map(OpenFileMappingW(FILE_MAP_READ,FALSE,prior_name));
        if(prior_map.h){
            auto prior_state=static_cast<LegacyStatus34*>(MapViewOfFile(prior_map.h,FILE_MAP_READ,0,0,sizeof(LegacyStatus34)));
            if(prior_state){
                if(prior_state->magic==LEGACY_MAGIC34&&prior_state->version==34&&prior_state->size==sizeof(LegacyStatus34)&&
                    prior_state->pid==GetCurrentProcessId()&&prior_state->stop&&prior_state->mode==DISABLED&&
                    !prior_state->ready&&!prior_state->error&&!prior_state->fire_ticket&&!prior_state->fire_guard&&
                    prior_state->camera_callback_thread&&prior_state->camera_callback_thread==prior_state->window_thread)
                    state->camera_callback_thread=prior_state->camera_callback_thread;
                UnmapViewOfFile(prior_state);
            }
        }
        state->heartbeat=GetTickCount64();InterlockedExchange(&enabled,1);
        require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Hook enable failed");
        const auto audio_guard123=mileszero123::install();
        log<<"{\"event\":\"miles_zero123\",\"status\":"<<audio_guard123<<"}\n";log.flush();
        InterlockedExchange(&state->ready,1);
        log<<"{\"event\":\"hook_ready\",\"pid\":"<<GetCurrentProcessId()<<",\"sha256\":\""<<engine_sha170<<"\",\"motion_rva\":\"0x367600\"}\n";log.flush();
        bool f9=false,f10=false,was_focus=false,mouse_captured=false,left_mouse=false;uint64_t next=0,last_mouse=GetTickCount64();
        // Release builds remain available until explicit disable or game exit.
        while(!state->stop) {
            DWORD foreground_pid=0;DWORD foreground_thread=GetWindowThreadProcessId(GetForegroundWindow(),&foreground_pid);bool focus=foreground_pid==GetCurrentProcessId();
            state->heartbeat=GetTickCount64();unsigned input=0;
            bool charge_locked146=charge_input_locked146();
            if(!focus||!down(VK_MBUTTON)||!third_person152)orbit_button154(false);
            equipment_sample_input81(focus&&!charge_locked146);ability_sample141(focus&&!charge_locked146);
            if(!focus)bow_sample142(down(VK_LBUTTON),false);
            if(focus){
                capture_weapon_mouse(GetForegroundWindow());
                bool click=down(VK_LBUTTON);
                melee60::Request active_melee;bool melee_locked=melee_request(active_melee);
                bool firing142=!charge_locked146&&state->mode==POSSESSED&&!dead_view()&&!melee_locked&&!command_placing100&&down(VK_RBUTTON)&&equipment_ranged_selected81();
                bow_sample142(click,firing142&&!state->weapon_phase&&!state->fire_ticket&&state->weapon_reload_remaining<=0);
                if(click&&!left_mouse&&firing142&&bow_family142==0)InterlockedExchange(&state->fire_request,1);
                left_mouse=click;
                InterlockedExchange(&state->aiming,!charge_locked146&&state->mode==POSSESSED&&!dead_view()&&!melee_locked&&!command_placing100&&down(VK_RBUTTON)?1:0);
                state->window_thread=foreground_thread;
                if(down('W'))input|=FORWARD;if(down('S'))input|=BACKWARD;if(down('A'))input|=LEFT;if(down('D'))input|=RIGHT;
                if(down(VK_SHIFT))input|=RUN;
                bool a=down(VK_F9),b=down(VK_F10);
                if(a&&!f9)request_possession();
                if(b&&!f10)InterlockedExchange(&state->release,1);
                if(down(VK_ESCAPE))InterlockedExchange(&state->release,1);
                f9=a;f10=b;
                if(state->acquire&&state->mode==ARMED&&!entry_attempted&&input_window&&
                    camera_entry_owner(foreground_thread,state->camera_callback_thread,state->camera_input_thread)&&
                    !InterlockedCompareExchange(&entry_pending,1,0)){
                    if(PostMessageW(input_window,ENTER_SOLDIER_MESSAGE,0,0))++state->acquisition_posts;
                    else {InterlockedExchange(&entry_pending,0);entry_failed(14);}
                }
            }else {if(was_focus)InterlockedExchange(&state->release,1);f9=f10=false;}
            if(!focus){left_mouse=false;InterlockedExchange(&state->aiming,0);}
            if((state->mode!=POSSESSED||state->stop||state->release||!focus)&&interface_active)restore_interface_on_window();
            InterlockedExchange(&charge_physical_input148,LONG(charge_intent199(input,down(VK_LBUTTON),down(VK_RBUTTON),focus&&state->mode==POSSESSED&&!state->release&&!dead_view())));
            was_focus=focus;input=entry_input125(input,focus&&!dead_view()&&!charge_locked146);if(charge_locked146)input=0;
            InterlockedExchange(&state->input,dead_view()?0:LONG(input));
            uint64_t now=GetTickCount64();float elapsed=std::min(float(now-last_mouse)*0.001f,0.05f);last_mouse=now;
            bool command_resume100=InterlockedExchange(&command_recenter100,0)!=0;
            if(command_resume100){mouse_captured=false;InterlockedExchange(&raw_mouse_x,0);InterlockedExchange(&raw_mouse_y,0);}
            if(focus&&state->mode==POSSESSED&&!state->release&&!state->stop){
                HWND window=GetForegroundWindow();RECT client{};POINT center{},cursor{};
                if(GetClientRect(window,&client)&&client.right>200&&client.bottom>200){
                    center={client.right/2,client.bottom/2};
                    if(ClientToScreen(window,&center)&&GetCursorPos(&cursor)){
                        int dx=command_resume100?0:raw_mouse_active?int(InterlockedExchange(&raw_mouse_x,0)):(mouse_captured?cursor.x-center.x:0);
                        int dy=command_resume100?0:raw_mouse_active?int(InterlockedExchange(&raw_mouse_y,0)):(mouse_captured?cursor.y-center.y:0);
                        // In-game input capture is active only for this possessed game window.
                        if(SetCursorPos(center.x,center.y))mouse_captured=true;
                        mouse_look101(dx,dy,input,elapsed);
                    }
                }
            }else mouse_captured=false;
            update_eyelids(focus);
            if(dead_view()&&death_elapsed()>=4500&&!InterlockedCompareExchange(&death_return_pending,1,0)){
                if(!input_window||!PostMessageW(input_window,RESTORE_INTERFACE_MESSAGE,0,0)){
                    InterlockedExchange(&death_return_pending,0);InterlockedExchange(&state->release,1);
                }
            }
            if(GetTickCount64()>=next){
                // This reset used to be hidden inside the logging function.
                // Keep the HUD maintenance even with all disk logging removed.
                if(state&&state->mode!=POSSESSED)climb_publish97(climb97::Off);
                if(log.enabled())snapshot_log(log);
                else{AcquireSRWLockExclusive(&weapon_observation_lock82);weapon_observation_read82=weapon_observation_write82;ReleaseSRWLockExclusive(&weapon_observation_lock82);}
                next=GetTickCount64()+250;
            }Sleep(8);
        }
        InterlockedExchange(&state->release,1);restore_interface_on_window();
        InterlockedExchange(&enabled,0);InterlockedExchange(&state->input,0);
        nativeoverlay106::stop();
        if(eyelid_top)DestroyWindow(eyelid_top);if(eyelid_bottom)DestroyWindow(eyelid_bottom);
        if(input_window&&reinterpret_cast<WNDPROC>(GetWindowLongPtrW(input_window,GWLP_WNDPROC))==&weapon_window_proc)
            SetWindowLongPtrW(input_window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(native_window_proc));
        // Disable gameplay control first. A queued manual shot must be dropped,
        // never forwarded as an ordinary ammo-free projectile after shutdown.
        for(unsigned i=0;i<20&&state->fire_ticket;++i)Sleep(50);
        bool guarded=state->fire_ticket!=0;
        MH_STATUS result=MH_OK;
        if(!mileszero123::disable())result=MH_UNKNOWN;
        if(MH_DisableHook(keyboard_poll_target84)!=MH_OK)result=MH_UNKNOWN;
        for(unsigned rva:{0x2fc450u,0x2f79a0u,0x10ee5b0u,0x325ca0u,0x321d70u,0x3e5170u})if(MH_DisableHook(reinterpret_cast<void*>(base+rva))!=MH_OK)result=MH_UNKNOWN;
        for(void* p:{reinterpret_cast<void*>(base+0xf9c360),reinterpret_cast<void*>(base+0xf9bf20),reinterpret_cast<void*>(base+0x122f740),cursor_target})if(MH_DisableHook(p)!=MH_OK)result=MH_UNKNOWN;
        for(void* p:{reinterpret_cast<void*>(base+0x2fcb10),reinterpret_cast<void*>(base+0x341560),reinterpret_cast<void*>(base+0x2d6210),reinterpret_cast<void*>(base+0x340d30),reinterpret_cast<void*>(base+0x3945c0),reinterpret_cast<void*>(base+0x3cf280),reinterpret_cast<void*>(base+0x3ccd00),reinterpret_cast<void*>(base+0x473f80),target,cancel_target,toggle_target,matrix_target,camera_input_target,projectile_camera_target,reinterpret_cast<void*>(base+0x3ca920),reinterpret_cast<void*>(base+0x3e8b50),reinterpret_cast<void*>(base+0x3f0370),reinterpret_cast<void*>(base+0x3f03d0),reinterpret_cast<void*>(base+0x3e3f50),reinterpret_cast<void*>(base+0x3d3630),reinterpret_cast<void*>(base+0x3d4320),reinterpret_cast<void*>(base+0x3e1a20),reinterpret_cast<void*>(base+0x4272e0),reinterpret_cast<void*>(base+0x4c56d0)})if(MH_DisableHook(p)!=MH_OK)result=MH_UNKNOWN;
        if(!guarded){if(MH_DisableHook(reinterpret_cast<void*>(base+0x2ac0b0))!=MH_OK)result=MH_UNKNOWN;}
        for(void* p:{reinterpret_cast<void*>(base+0x3f06e0),reinterpret_cast<void*>(base+0x3c73e0)})if(MH_DisableHook(p)!=MH_OK)result=MH_UNKNOWN;
        for(void* p:{reinterpret_cast<void*>(base+0x3d3ce0),reinterpret_cast<void*>(base+0x3d33f0),reinterpret_cast<void*>(base+0x3d3d40),reinterpret_cast<void*>(base+0x3d3de0),reinterpret_cast<void*>(base+0x3d3e90),reinterpret_cast<void*>(base+0x3d3e60),reinterpret_cast<void*>(base+0x3d3fc0),reinterpret_cast<void*>(base+0x3cfa60),reinterpret_cast<void*>(base+0x3d41d0),reinterpret_cast<void*>(base+0x3cbe40)})if(MH_DisableHook(p)!=MH_OK)result=MH_UNKNOWN;
        // Pending native rejoin retains its read-only combat compatibility.
        if(disable_control_hooks130()!=MH_OK)result=MH_UNKNOWN;
        state->fire_guard=guarded?1:0;state->error=result==MH_OK?0:30;
        memcpy(state->restored_code,target,16);bool restored=memcmp(state->restored_code,state->original_code,16)==0;
        bool camera_restored=memcmp(cancel_target,cancel_sig,8)==0&&memcmp(toggle_target,toggle_sig,8)==0&&memcmp(matrix_target,matrix_sig,8)==0;
        state->mode=DISABLED;state->ready=0;if(memcmp(projectile_camera_target,projectile_camera_sig,8)!=0)camera_restored=false;
        if(!camera_restored||!restored)state->error=31;
        const unsigned char skin_sig[]={0x83,0xec,0x0c,0x56,0x8b,0x74,0x24,0x14};
        if(memcmp(reinterpret_cast<void*>(base+0x4c56d0),skin_sig,8)!=0)state->error=31;
        if(!native_fatigue_signatures(base))state->error=31;
        log<<"{\"event\":\"disabled\",\"reason\":\"explicit_stop\",\"tick_ms\":"<<GetTickCount64()<<",\"bytes_restored\":"<<(restored?"true":"false")<<",\"camera_bytes_restored\":"<<(camera_restored?"true":"false")<<",\"hook_status\":"<<int(result)<<"}\n";log.flush();
        charge_reset139();charge146::publish(charge_shared146,{});
        // Keep DLL, mapping, and trampoline resident until process exit: no unload race.
    }catch(const std::exception& ex){startup167::remember(ex.what());nativeoverlay106::stop();if(state){state->error=1;state->ready=0;}if(log){log<<"{\"event\":\"initialization_failed\",\"reason\":\""<<ex.what()<<"\"}\n";log.flush();}}
    return 0;
}
BOOL WINAPI DllMain(HINSTANCE instance,DWORD reason,LPVOID) {
    if(reason==DLL_PROCESS_ATTACH){self_module=instance;DisableThreadLibraryCalls(instance);HANDLE t=CreateThread(nullptr,0,worker,nullptr,0,nullptr);if(t)CloseHandle(t);else return FALSE;}
    return TRUE;
}

