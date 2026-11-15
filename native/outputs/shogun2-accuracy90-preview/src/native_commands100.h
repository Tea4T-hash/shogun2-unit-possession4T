#pragma once
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "command_protocol100.h"
#include "native_advance100.h"
#include "native_context103.h"
// Included after native orders, weapons, traversal and interface helpers.
using FacingConstructor100=void*(__thiscall*)(void*,const void*,const void*,unsigned,float,unsigned,unsigned);
using OrientationConstructor100=void*(__thiscall*)(void*,const void*,const void*,unsigned,unsigned,unsigned);
using UnitStateConstructor100=void*(__thiscall*)(void*,const void*,unsigned);
using SceneQuery100=void(__thiscall*)(void*,void*,const void*,unsigned,unsigned,unsigned,unsigned,unsigned);
using SurfacePoint100=void*(__thiscall*)(void*,void*,const float*);
using WalkableSurface100=bool(__thiscall*)(void*,uintptr_t,unsigned);
using NativeFree100=void(__cdecl*)(void*);
using MoveCell100=uintptr_t(__thiscall*)(void*,const float*,uintptr_t,unsigned);
struct MovePoint100 {float position[3]{};uintptr_t surface{};unsigned surface_id{};};
static_assert(sizeof(MovePoint100)==20,"Native movement point layout changed");
static command100::MoveMode command_mode100=command100::MoveMode::None;
static volatile LONG command_fire_mode100;
static command100::Feedback command_feedback100=command100::Feedback::None;
static uint64_t command_feedback_until100;
static unsigned command_generation100,command_error100;
static command100::FollowTarget command_follow100;
static volatile LONG follow_updates101;
static command100::FollowPace102 command_pace102;
static float command_gap102;
static volatile LONG command_follow_run102,command_hold_runs102,command_hold_walks102;
static volatile LONG command_morale104,command_routing104,command_guard104;
static SRWLOCK command_mapping_lock100=SRWLOCK_INIT;
static void command_reset_volley100();
static HANDLE command_mapping100;static command100::Status* command_shared100;
static SRWLOCK command_volley_lock100=SRWLOCK_INIT;
static command100::Volley command_volley100;
static volatile LONG command_peer_releases100,command_peer_blocks100;
static void command_feedback_set100(command100::Feedback feedback,unsigned error=0){command_feedback100=feedback;command_feedback_until100=GetTickCount64()+1800;command_error100=error;}
#include "follow_voice125.h"
static bool command_native_signatures100(uintptr_t image){
    const unsigned char construct[]={0x56,0x8b,0x74,0x24,8,0x57,0x8b,0xf9};
    for(unsigned address:{0x255680u,0x2557e0u,0x255140u})if(memcmp(reinterpret_cast<void*>(image+address),construct,sizeof construct))return false;
    return field<uintptr_t>(image+0x15cffe0,0)==image+0x2e6eb0&&field<uintptr_t>(image+0x15cffdc,0)==image+0x2e6f00&&
        field<uintptr_t>(image+0x15d002c,0)==image+0x2e6c40;
}
enum class UnitState100 {Fire,Guard,Skirmish};
static bool command_serialize100(const MovePoint100* destination,unsigned facing,float width,int fire_policy=-1,bool plain=false,UnitState100 unit_state=UnitState100::Fire,unsigned char* prepared=nullptr,unsigned* prepared_size=nullptr,bool running102=false){
    const bool boolean_order=unit_state!=UnitState100::Fire;
    alignas(16) unsigned char order[0x80]{},context[16]{},packet[QUEUE_CAPACITY+4]{};
    bool constructed=false,context_live=false,result=false;
    __try{
        if(!enabled||state->stop||state->release||state->mode!=POSSESSED||dead_view()||!followed(possessed,false))return false;
        auto group=selected_possessed_group();auto queue=field<uintptr_t>(environment,0x28048);
        unsigned descriptor_offset=unit_state==UnitState100::Skirmish?0x192d2ccu:unit_state==UnitState100::Guard?0x192e90cu:
            fire_policy>=0?0x192ca68u:plain?0x192e0d0u:width>0?0x192eaacu:0x192d014u;
        unsigned key=field<unsigned>(base,descriptor_offset),flags=0;
        if(!group||!queue||!descriptor_flags(key,flags)){command_error100=41;return false;}
        if(fire_policy>=0){
            if(fire_policy>1)return false;
            reinterpret_cast<UnitStateConstructor100>(base+(boolean_order?0x254e10:0x255140))(order,reinterpret_cast<void*>(group),unsigned(fire_policy));
        }else{
            if(!destination||!command100::finite({destination->position[0],destination->position[1],destination->position[2]})||
               !std::isfinite(width)||width<0||width>4096)return false;
            if(plain)reinterpret_cast<MoveConstructor>(base+0x255550)(order,reinterpret_cast<void*>(group),destination,unsigned(running102),0);
            else if(width>0)reinterpret_cast<FacingConstructor100>(base+0x2557e0)(order,reinterpret_cast<void*>(group),destination,facing,width,unsigned(running102),0);
            else reinterpret_cast<OrientationConstructor100>(base+0x255680)(order,reinterpret_cast<void*>(group),destination,facing,unsigned(running102),0);
        }
        constructed=true;auto expected=base+(boolean_order?0x15cffe8:fire_policy>=0?0x15d002c:plain?0x15cffd8:width>0?0x15cffdc:0x15cffe0);
        if(field<uintptr_t>(reinterpret_cast<uintptr_t>(order),0)!=expected){command_error100=47;}
        else{
            reinterpret_cast<ContextConstructor>(base+0x1cc200)(context,packet,key,flags);context_live=true;
            auto serialize=reinterpret_cast<SerializeOrder>(field<uintptr_t>(expected,0));
            if(serialize(order,context)){
                reinterpret_cast<NativeCleanup>(base+0x1d0d40)(context);context_live=false;
                unsigned length=field<unsigned>(reinterpret_cast<uintptr_t>(packet),QUEUE_CAPACITY);
                auto& used=field<unsigned>(queue+4,QUEUE_CAPACITY);state->queue_before=used;
                if(prepared&&prepared_size&&valid_packet(packet,length,key)){
                    memcpy(prepared,packet,length);*prepared_size=length;result=true;command_error100=0;
                }else if(!prepared&&append_packet(reinterpret_cast<unsigned char*>(queue+4),used,packet,length,key)){
                    state->queue_after=used;state->packet_size=length;++state->orders_sent;command_error100=0;state->order_error=0;result=true;
                    if(destination){state->order_target_x=destination->position[0];state->order_target_z=destination->position[2];}
                }else command_error100=48;
            }else command_error100=49;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){command_error100=60;}
    if(context_live){__try{reinterpret_cast<NativeCleanup>(base+0x1d0d40)(context);}__except(EXCEPTION_EXECUTE_HANDLER){command_error100=61;result=false;}}
    if(constructed){__try{reinterpret_cast<NativeCleanup>(base+0x1d10f0)(order+4);}__except(EXCEPTION_EXECUTE_HANDLER){command_error100=61;result=false;}}
    return result;
}
static bool command_hold_here100(const MovePoint100& point,unsigned facing,bool running102=false){
    alignas(16) unsigned char move[QUEUE_CAPACITY+4]{},guard[QUEUE_CAPACITY+4]{};unsigned move_size=0,guard_size=0;
    if(!command_serialize100(&point,facing,0,-1,false,UnitState100::Fire,move,&move_size,running102)||
       !command_serialize100(nullptr,0,0,1,false,UnitState100::Guard,guard,&guard_size))return false;
    auto queue=field<uintptr_t>(environment,0x28048);if(!queue)return false;
    auto& used=field<unsigned>(queue+4,QUEUE_CAPACITY);unsigned before=used,total=move_size+guard_size;
    if(before>QUEUE_CAPACITY||total>QUEUE_CAPACITY-before){command_error100=48;return false;}
    auto output=reinterpret_cast<unsigned char*>(queue+4);memcpy(output+before,move,move_size);memcpy(output+before+move_size,guard,guard_size);
    MemoryBarrier();InterlockedExchange(reinterpret_cast<volatile LONG*>(&used),LONG(before+total));
    state->queue_before=before;state->queue_after=used;state->packet_size=total;state->orders_sent+=2;state->order_error=0;
    state->last_order=ORDER_HOLD;state->order_target_x=point.position[0];state->order_target_z=point.position[2];return true;
}
static bool command_actor_point100(command100::Point p,MovePoint100& output){
    if(!command100::finite(p))return false;
    auto common=possession_lease.identity.common,manager=field<uintptr_t>(common,0xb8);
    if(!manager||field<uintptr_t>(manager,0)!=environment)return false;
    const float xz[2]={p.x,p.z};auto surface=field<uintptr_t>(possessed,0x10);
    reinterpret_cast<MoveParameters>(base+0x37c010)(reinterpret_cast<void*>(manager),&output,xz,unsigned(surface),1,0);
    return command100::finite({output.position[0],output.position[1],output.position[2]});
}
static bool command_pick100(const ViewRay& view,MovePoint100& output){
    struct Results {unsigned capacity{},count{};uintptr_t data{};} results;
    struct Segment {unsigned kind;float start[3],end[3];} segment{};
    bool returned=false,valid=false;
    __try{
        if(!possession_lease.active||!followed(possessed,false))return false;
        float norm=0;segment.kind=3;
        for(unsigned i=0;i<3;++i){
            if(!std::isfinite(view.origin[i])||!std::isfinite(view.direction[i])||std::abs(view.origin[i])>100000)return false;
            segment.start[i]=view.origin[i];segment.end[i]=view.origin[i]+view.direction[i]*4096.f;norm+=view.direction[i]*view.direction[i];
        }
        if(std::abs(norm-1.f)>.001f)return false;
        auto phase=possession_lease.identity.phase;
        // Native segment query and native sorting. Terrain and surface objects
        // only: an actor in front of the cursor must not become a move target.
        reinterpret_cast<SceneQuery100>(base+0x158970)(reinterpret_cast<void*>(phase),&results,&segment,0,1,0x303,0,0);returned=true;
        if(results.count&&results.count<=65536&&results.capacity>=results.count&&results.data){
            // First physical surface wins. Do not look through a rejected wall
            // and accidentally send the formation to terrain behind it.
            auto hit=results.data;auto object=field<uintptr_t>(hit,0);
            if(object){
                unsigned type=field<unsigned>(object,0);
                auto common=possession_lease.identity.common,manager=field<uintptr_t>(common,0xb8);
                const float xz[2]={field<float>(hit,4),field<float>(hit,12)};
                if(manager&&field<uintptr_t>(manager,0)==environment&&(type==1||type==2)&&
                   reinterpret_cast<MoveCell100>(base+0x3b2470)(reinterpret_cast<void*>(manager),xz,0,1)){
                    reinterpret_cast<MoveParameters>(base+0x37c010)(reinterpret_cast<void*>(manager),&output,xz,0,1,0);valid=true;
                }else if(type==0x100||type==0x200){
                    auto surface=field<uintptr_t>(object,12);auto navigation=surface?field<uintptr_t>(surface,0x44):0;
                    auto navigation_manager=field<uintptr_t>(common,0xbc);
                    if(surface&&navigation&&navigation_manager&&reinterpret_cast<WalkableSurface100>(base+0x3f52f0)(reinterpret_cast<void*>(navigation_manager),navigation,1)){
                        auto table=field<uintptr_t>(surface,0),function=table?field<uintptr_t>(table,0x34):0;
                        if(function>=base&&function<base+0x1580000){
                            reinterpret_cast<SurfacePoint100>(function)(reinterpret_cast<void*>(surface),&output,reinterpret_cast<const float*>(hit+4));valid=true;
                        }
                    }
                }
            }
        }
        valid=valid&&command100::finite({output.position[0],output.position[1],output.position[2]});
    }__except(EXCEPTION_EXECUTE_HANDLER){command_error100=62;valid=false;}
    if(returned&&results.data){__try{reinterpret_cast<NativeFree100>(base+0xf04d0)(reinterpret_cast<void*>(results.data));}__except(EXCEPTION_EXECUTE_HANDLER){command_error100=63;valid=false;}}
    return valid;
}
static bool command_frame_matches100(const command100::Frame& frame){
    return frame.lease==possession_lease.generation&&frame.actor==possession_lease.identity.actor_id&&frame.unit==possession_lease.identity.unit_id&&
        GetTickCount64()>=frame.tick&&GetTickCount64()-frame.tick<=250;
}
static void command_reset100(){
    command_voice_reset125();
    command_native_cancel100();command_input_reset100();command_follow100.reset();command_pace102.reset();command_gap102=0;command_follow_run102=0;command_generation100=0;
    command_mode100=command100::MoveMode::None;InterlockedExchange(&command_fire_mode100,0);
    command_reset_volley100();
    AcquireSRWLockExclusive(&command_mapping_lock100);
    if(command_shared100){InterlockedIncrement(&command_shared100->sequence);MemoryBarrier();command_shared100->lease=0;command_shared100->tick=0;MemoryBarrier();InterlockedIncrement(&command_shared100->sequence);}
    ReleaseSRWLockExclusive(&command_mapping_lock100);
}

struct CommandCycle100 {uintptr_t task{};unsigned draw_tick{},clip{};};
struct CommandPermit100 {command100::ActorKey actor{};CommandCycle100 cycle{};unsigned generation{};bool active{};};
static CommandPermit100 command_permits100[command100::Volley::capacity];
static void command_reset_volley100(){
    AcquireSRWLockExclusive(&command_volley_lock100);command_volley100.reset();memset(command_permits100,0,sizeof command_permits100);ReleaseSRWLockExclusive(&command_volley_lock100);
}
static bool command_peer100(uintptr_t subject,command100::ActorKey& key){
    if(!subject||subject==possessed||!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||
       !possession_lease.active||command_fire_mode100!=LONG(command100::FireMode::OnCommand)||!unit)return false;
    __try{
        if(!followed(possessed,false)||field<uintptr_t>(subject,0x214)!=unit||field<unsigned char>(subject,0xf0)!=1)return false;
        const auto& owner=possession_lease.identity;auto array=field<uintptr_t>(owner.view,0x1c);unsigned count=field<unsigned>(owner.view,0x18);
        if(!array||!count||count>1000)return false;
        for(unsigned i=0;i<count;++i){
            auto holder=field<uintptr_t>(array,i*4),human=holder?field<uintptr_t>(holder,8):0;
            if(!human||human==possessed)continue;
            if(human!=subject&&field<uintptr_t>(human,0x210)!=subject)continue;
            if(!actor152::human(base,field<uintptr_t>(human,0))||field<unsigned char>(human,0xf0)!=1||
               field<uintptr_t>(human,0x214)!=unit||field<uintptr_t>(human,0x28)!=owner.environment)return false;
            NativeSubject paired{};if(!native_subject(human,unit,owner.view,false,paired))return false;
            if(subject!=human&&subject!=paired.actor)return false;
            unsigned serial=field<unsigned>(human,0x1c8);
            if(field<uintptr_t>(holder,8)!=human||field<unsigned>(unit,4)!=owner.unit_id||field<uintptr_t>(human,0x20c)==0)return false;
            key={human,holder,serial};return true;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){}return false;
}
static bool command_draw_permitted100(void* task){
    command100::ActorKey key{};
    __try{if(!command_peer100(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4),key))return true;}
    __except(EXCEPTION_EXECUTE_HANDLER){return true;}
    AcquireSRWLockShared(&command_volley_lock100);bool allow=command_volley100.available(key);ReleaseSRWLockShared(&command_volley_lock100);
    if(!allow)InterlockedIncrement(&command_peer_blocks100);return allow;
}
static unsigned command_permit_slot100(const command100::ActorKey& key){
    unsigned i=command_volley100.slot(key);
    for(unsigned n=0;n<command100::Volley::capacity;++n,i=(i+1)&(command100::Volley::capacity-1))
        if(!command_permits100[i].actor.actor||command100::same(command_permits100[i].actor,key))return i;
    return command100::Volley::capacity;
}
static void command_draw_started100(void* task){
    command100::ActorKey key{};
    __try{if(!command_peer100(field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4),key))return;}
    __except(EXCEPTION_EXECUTE_HANDLER){return;}
    AcquireSRWLockExclusive(&command_volley_lock100);unsigned slot=command_permit_slot100(key);
    if(slot<command100::Volley::capacity){auto& permit=command_permits100[slot];permit.actor=key;permit.cycle={};permit.active=false;if(!++permit.generation)++permit.generation;}
    ReleaseSRWLockExclusive(&command_volley_lock100);
}
static bool command_release_managed100(void* task){
    command100::ActorKey key{};CommandCycle100 cycle{};unsigned before=0;
    __try{
        auto pointer=reinterpret_cast<uintptr_t>(task);
        if(!command_peer100(field<uintptr_t>(pointer,4),key))return false;
        cycle={pointer,field<unsigned>(pointer,0x78),field<unsigned>(pointer,0x88)};
        before=field<unsigned char>(pointer,0x6c)|(unsigned(field<unsigned char>(pointer,0x6d))<<8);
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
    AcquireSRWLockExclusive(&command_volley_lock100);
    unsigned slot=command_permit_slot100(key);bool admitted=false;unsigned generation=0;
    if(slot<command100::Volley::capacity){const auto& prior=command_permits100[slot];
        generation=prior.generation;admitted=prior.active&&prior.actor.actor&&prior.cycle.task==cycle.task&&prior.cycle.draw_tick==cycle.draw_tick&&prior.cycle.clip==cycle.clip;
    }
    bool available=command_volley100.available(key);ReleaseSRWLockExclusive(&command_volley_lock100);
    if(slot==command100::Volley::capacity||(!admitted&&!available)){acknowledge_skipped_ranged_release(task);InterlockedIncrement(&command_peer_blocks100);return true;}
    original_native_release(task);
    __try{
        auto pointer=reinterpret_cast<uintptr_t>(task);
        unsigned after=field<unsigned char>(pointer,0x6c)|(unsigned(field<unsigned char>(pointer,0x6d))<<8);
        if(admitted&&field<unsigned char>(pointer,0x6e)){
            AcquireSRWLockExclusive(&command_volley_lock100);slot=command_permit_slot100(key);
            if(slot<command100::Volley::capacity&&command_permits100[slot].generation==generation)command_permits100[slot].active=false;
            ReleaseSRWLockExclusive(&command_volley_lock100);
        }
        if(!admitted&&after!=before){
            command100::ActorKey current{};
            if(command_peer100(field<uintptr_t>(pointer,4),current)&&command100::same(key,current)){
                AcquireSRWLockExclusive(&command_volley_lock100);slot=command_permit_slot100(key);
                if(command_volley100.consume(key)&&slot<command100::Volley::capacity){auto& permit=command_permits100[slot];
                    if(permit.generation==generation){permit.actor=key;permit.cycle=cycle;permit.active=field<unsigned char>(pointer,0x6e)==0;}
                    InterlockedIncrement(&command_peer_releases100);}
                ReleaseSRWLockExclusive(&command_volley_lock100);
            }
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    return true;
}
static void command_on_shot100(){
    if(command_fire_mode100!=LONG(command100::FireMode::OnCommand)||!possession_lease.active||!followed(possessed,false))return;
    AcquireSRWLockExclusive(&command_volley_lock100);command_volley100.shot();ReleaseSRWLockExclusive(&command_volley_lock100);
}
static bool command_formation_gap102(uintptr_t camera,command100::Point leader,float& gap){
    auto view=field<uintptr_t>(camera,0x184);
    if(view!=possession_lease.identity.view)return false;
    auto count=field<unsigned>(view,0x18),array=field<uintptr_t>(view,0x1c);
    if(!array||!count||count>1000)return false;
    double x=0,z=0;unsigned peers=0;
    for(unsigned i=0;i<count;++i){
        auto holder=field<uintptr_t>(array,i*4),actor=holder?field<uintptr_t>(holder,8):0;
        if(!actor||actor==possessed||!actor152::human(base,field<uintptr_t>(actor,0))||field<unsigned char>(actor,0xf0)!=1||
           field<uintptr_t>(actor,0x214)!=unit||field<uintptr_t>(actor,0x28)!=environment)continue;
        command100::Point p{field<float>(actor,0x48),field<float>(actor,0x4c),field<float>(actor,0x50)};
        if(!command100::finite(p)||field<uintptr_t>(holder,8)!=actor)continue;
        x+=p.x;z+=p.z;++peers;
    }
    // A lone survivor needs no catch-up run. All identity guards still apply.
    gap=peers?float(std::hypot(x/peers-leader.x,z/peers-leader.z)):0;
    return std::isfinite(gap)&&followed(possessed,false)&&camera_owned(camera);
}
static void command_enqueue100(command100::Input input,command100::Pace pace=command100::Pace::Automatic){
    command100::Event event{};event.tick=GetTickCount64();event.input=input;event.pace=pace;
    AcquireSRWLockExclusive(&command_input_lock100);event.frame=command_frame100;
    command_q_tap102.reset();
    if(input==command100::Input::Hold&&pace!=command100::Pace::Automatic){
        event.running102=pace==command100::Pace::Run;
        command_z_tap102.reset();
    }else if(input==command100::Input::Hold){
        event.running102=command_z_tap102.consume(event.tick,GetDoubleClickTime(),event.frame.lease)&&
            event.frame.actor==command_z_anchor102.actor&&event.frame.unit==command_z_anchor102.unit;
        if(event.running102){auto current_tick=event.frame.tick;event.frame=command_z_anchor102;event.frame.tick=current_tick;}
        else {command_z_anchor102=event.frame;command_z_tap102.arm(event.tick,event.frame.lease);}
    }else command_z_tap102.reset();
    if(command_write100-command_read100>=32){command_read100=command_write100;command_q_tap102.reset();command_z_tap102.reset();InterlockedExchange(&command_input_overflow100,1);InterlockedExchange(&command_placing100,0);}
    else command_events100[command_write100++%32]=event;
    ReleaseSRWLockExclusive(&command_input_lock100);
}
static void command_window_wheel101(commander76::UnitOrder order){
    constexpr command100::Input commands[]={command100::Input::Follow,command100::Input::Stop,
        command100::Input::Hold,command100::Input::Fire};
    if(order.sector<0||order.sector>=4||!keyboard_fps84()||charge_input_locked146())return;
    command_native_cancel100();
    command_enqueue100(commands[order.sector],order.sector==2?
        (order.run?command100::Pace::Run:command100::Pace::Walk):command100::Pace::Automatic);
}
static bool command_window_fire62(command100::Input input){
    if(!keyboard_fps84()||charge_input_locked146())return false;
    command_enqueue100(input);
    return true;
}
static bool command_window100(HWND window,UINT message,WPARAM key,LPARAM flags){
    if(message==COMMAND_CANCEL_MESSAGE100){if(unsigned(key)==command_native_sequence100)command_native_cancel100();return true;}
    if(command_native_root100&&!command_placing100)command_native_cancel100();
    if(message==WM_KILLFOCUS||(message==WM_ACTIVATEAPP&&!key)){
        command_native_cancel100();command_input_reset100();command_cancel_fire62();command_key_held100=0;return false;
    }
    if(command_native_mouse100(window,message,key,flags))return true;
    if(message!=WM_KEYDOWN&&message!=WM_KEYUP&&message!=WM_SYSKEYDOWN&&message!=WM_SYSKEYUP)return false;
    unsigned bit=command100::key_bit(unsigned(key));if(!bit)return false;
    bool pressed=message==WM_KEYDOWN||message==WM_SYSKEYDOWN;
    bool owned=enabled&&state&&state->mode==POSSESSED&&!state->stop&&!state->release;
    bool drain=(command_key_held100&bit)!=0;
    if(!pressed)command_key_held100&=~bit;
    if(!owned){if(key=='R'&&!pressed)command_cancel_fire62();return drain;}
    if(pressed){if((flags&(1L<<30))||drain)return true;command_key_held100|=bit;}
    if(key=='R'){
        if(pressed){
            command_fire_press_tick62=GetTickCount64();
            command_fire_press_lease62=state->lease_generation;
        }else{
            if(drain&&command_fire_press_tick62&&command_fire_press_lease62==state->lease_generation){
                const bool held=GetTickCount64()-command_fire_press_tick62>=350;
                command_enqueue100(held?command100::Input::Skirmish:command100::Input::Fire);
            }
            command_cancel_fire62();
        }
        return true;
    }
    if(key=='Q'){
        if(pressed){AcquireSRWLockExclusive(&command_input_lock100);command_z_tap102.reset();ReleaseSRWLockExclusive(&command_input_lock100);}
        if(pressed){if(!command_native_begin100(window))command_feedback_set100(command100::Feedback::Unavailable);}
        else if(command_native_root100){if(command_native_end100(window)){command_mode100=command100::MoveMode::Advance;InterlockedExchange(&state->follow,0);command_follow100.reset();}}
        return true;
    }
    if(!pressed)return true;
    command_native_cancel100();
    command_enqueue100(key=='E'?command100::Input::Stop:key=='Z'?command100::Input::Hold:command100::Input::Follow);
    return true;
}
static void command_mapping_publish100(command100::Status sample){
    AcquireSRWLockExclusive(&command_mapping_lock100);
    if(!command_shared100){
        wchar_t name[120];swprintf_s(name,L"Local\\Shogun2PossessionUnitCommands100-%lu",GetCurrentProcessId());
        command_mapping100=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof sample,name);
        if(command_mapping100)command_shared100=static_cast<command100::Status*>(MapViewOfFile(command_mapping100,FILE_MAP_ALL_ACCESS,0,0,sizeof sample));
    }
    if(!command_shared100){ReleaseSRWLockExclusive(&command_mapping_lock100);return;}
    sample.magic=command100::magic;sample.version=100;sample.size=sizeof sample;sample.pid=GetCurrentProcessId();
    sample.lease=possession_lease.generation;sample.actor=possession_lease.identity.actor_id;sample.unit=possession_lease.identity.unit_id;
    sample.tick=GetTickCount64();sample.move_mode=unsigned(command_mode100);sample.fire_mode=unsigned(command_fire_mode100);
    sample.feedback=unsigned(command_feedback100);sample.feedback_until=command_feedback_until100;sample.error=command_error100;
    sample.placing=command_placing100?1u:0u;sample.releases=unsigned(command_peer_releases100);sample.blocked=unsigned(command_peer_blocks100);
    AcquireSRWLockShared(&command_volley_lock100);sample.volley=command_volley100.epoch;ReleaseSRWLockShared(&command_volley_lock100);
    LONG sequence=InterlockedIncrement(&command_shared100->sequence);sample.sequence=sequence;MemoryBarrier();memcpy(command_shared100,&sample,sizeof sample);MemoryBarrier();InterlockedIncrement(&command_shared100->sequence);
    ReleaseSRWLockExclusive(&command_mapping_lock100);
}
static void command_log100(std::ostream& log){
    command100::Status sample{};AcquireSRWLockShared(&command_mapping_lock100);
    if(command_shared100)memcpy(&sample,command_shared100,sizeof sample);ReleaseSRWLockShared(&command_mapping_lock100);
    log<<"{\"event\":\"commands100\",\"tick_ms\":"<<GetTickCount64()<<",\"protocol\":100,\"lease\":"<<sample.lease
       <<",\"actor_id\":"<<sample.actor<<",\"unit_id\":"<<sample.unit<<",\"move_mode\":"<<sample.move_mode<<",\"fire_mode\":"<<sample.fire_mode
       <<",\"placing\":"<<sample.placing<<",\"volley\":"<<sample.volley
       <<",\"peer_releases\":"<<command_peer_releases100<<",\"peer_blocks\":"<<command_peer_blocks100<<",\"error\":"<<sample.error
       <<",\"native_q_presses\":"<<command_native_presses100<<",\"native_q_moves\":"<<command_native_moves100<<",\"native_q_releases\":"<<command_native_releases100<<",\"native_q_cancels\":"<<command_native_cancels100
       <<",\"commands101\":1,\"q_aim_center101\":1,\"q_mouse_look101\":1,\"follow_translation101\":1,\"follow_updates101\":"<<follow_updates101
       <<",\"commands102\":1,\"follow_catchup102\":"<<(command_pace102.catchup?1:0)<<",\"follow_gap102\":"<<command_gap102<<",\"follow_run102\":"<<command_follow_run102
       <<",\"hold_walks102\":"<<command_hold_walks102<<",\"hold_runs102\":"<<command_hold_runs102<<",\"q_doubles102\":"<<command_native_doubles102
       <<",\"formation104\":1,\"camera_scope104\":1,\"camera_updates104\":"<<camera_updates104<<",\"holder_resolutions104\":"<<holder_resolutions104<<",\"roster_forwards104\":"<<roster_forwards104<<",\"selection_follow_blocks104\":"<<selection_follow_blocks104
       <<",\"native_guard104\":"<<command_guard104<<",\"morale104\":"<<command_morale104<<",\"routing_state104\":"<<command_routing104
       <<",\"animation_look104\":1,\"animation_look_updates104\":"<<animation_look_updates104<<",\"animation_look_active104\":"<<animation_look_active104
       <<",\"clip101\":1,\"native_near101\":"<<interface_clip101.native<<",\"presented_near101\":"<<interface_clip101.presented
       <<",\"infantry_eye_updates101\":"<<infantry_eye_updates101<<",\"infantry_eye_rejects101\":"<<infantry_eye_rejects101
       <<",\"self_visibility_updates\":"<<self_visibility_updates100<<",\"self_visibility_rejects\":"<<self_visibility_rejects100<<",\"self_visibility_restores\":"<<self_visibility_restores100<<"}\n";
}
static void process_orders(uintptr_t camera){
    if(!enabled||!state||state->stop||state->release||state->mode!=POSSESSED||dead_view()||!camera_owned(camera))return;
    if(GetCurrentThreadId()!=state->window_thread||InterlockedCompareExchange(&order_busy,1,0))return;
    __try{
        InterlockedExchange(&command_morale104,LONG(field<unsigned>(unit,0xce8)));InterlockedExchange(&command_routing104,LONG(field<unsigned>(unit,0xcec)));InterlockedExchange(&command_guard104,LONG(field<unsigned char>(unit,0xeb1)));
        if(command_generation100!=possession_lease.generation){
            command_reset100();command_generation100=possession_lease.generation;
            InterlockedExchange(&command_fire_mode100,field<unsigned char>(unit,0xeb0)?LONG(command100::FireMode::AtWill):LONG(command100::FireMode::Hold));
        }
        command100::Frame frame{};frame.position={field<float>(possessed,0x48),field<float>(possessed,0x4c),field<float>(possessed,0x50)};
        frame.yaw=state->yaw;
        frame.tick=GetTickCount64();frame.lease=possession_lease.generation;frame.actor=possession_lease.identity.actor_id;frame.unit=possession_lease.identity.unit_id;
        command_publish_frame100(frame);
        observe_formation(camera);
        melee60::Request commitment{};bool busy=charge_input_locked146()||melee_request(commitment)||wall_subject97(possessed)||native_airborne_subject99(possessed);
        if(busy&&command_native_root100)command_native_cancel100();
        commander_tick_orders101(!busy);
        if(InterlockedExchange(&command_input_overflow100,0)){command_feedback_set100(command100::Feedback::Unavailable,64);}
        command100::Event event{};
        while(command_pop100(event)){
            if(busy){InterlockedExchange(&command_placing100,0);command_feedback_set100(command100::Feedback::Busy);continue;}
            if(event.input!=command100::Input::Stop&&!command_frame_matches100(event.frame)){command_feedback_set100(command100::Feedback::Unavailable,65);continue;}
            command_voice_reset125();
            if(event.input==command100::Input::Stop){
                if(issue_native_order(ORDER_HOLD,frame.position.x,frame.position.z)){
                    command_mode100=command100::MoveMode::Stop;InterlockedExchange(&state->follow,0);command_follow100.reset();InterlockedExchange(&command_placing100,0);InterlockedExchange(&command_recenter100,1);command_feedback_set100(command100::Feedback::Stopped);
                    commander_ack_order102(1);
                }else command_feedback_set100(command100::Feedback::Unavailable,state->order_error);
            }else if(event.input==command100::Input::Hold){
                MovePoint100 point{};
                // Project the captured position through the same native surface
                // resolver as Follow Me. A rider origin or sloping ground need
                // not be within 30cm of the terrain ray intersection.
                bool projected=command_actor_point100(event.frame.position,point);if(!projected)command_error100=66;
                if(projected&&command_hold_here100(point,command100::bearing(event.frame.yaw),event.running102)){
                    InterlockedExchange(&command_placing100,0);InterlockedExchange(&command_recenter100,1);
                    if(event.running102)InterlockedIncrement(&command_hold_runs102);else InterlockedIncrement(&command_hold_walks102);
                    command_mode100=command100::MoveMode::Hold;InterlockedExchange(&state->follow,0);command_follow100.reset();command_feedback_set100(command100::Feedback::Holding);
                    commander_ack_order102(2,event.running102);
                }else command_feedback_set100(command100::Feedback::Unavailable,command_error100);
            }else if(event.input==command100::Input::Follow){
                InterlockedExchange(&command_placing100,0);InterlockedExchange(&command_recenter100,1);
                InterlockedExchange(&state->follow,1);command_follow100.reset();command_pace102.reset();command_mode100=command100::MoveMode::Follow;
                state->last_order=ORDER_FOLLOW;command_feedback_set100(command100::Feedback::Following);
                commander_ack_order102(0);
            }else if(event.input==command100::Input::Skirmish){
                const auto active=field<unsigned char>(unit,0xeb2);
                if(field<unsigned char>(unit,0x179)!=1||active>1){
                    command_feedback_set100(command100::Feedback::Unavailable);
                }else if(command_serialize100(nullptr,0,0,!active,false,UnitState100::Skirmish)){
                    command_feedback_set100(active?command100::Feedback::SkirmishOff:command100::Feedback::SkirmishOn);
                }else command_feedback_set100(command100::Feedback::Unavailable,command_error100);
            }else if(event.input==command100::Input::Fire){
                if(!bow_record(unit)){command_feedback_set100(command100::Feedback::Unavailable);continue;}
                auto next=command100::next_fire(command100::FireMode(command_fire_mode100));
                if(command_serialize100(nullptr,0,0,next==command100::FireMode::Hold?1:0)){
                    AcquireSRWLockExclusive(&command_volley_lock100);command_volley100.reset();memset(command_permits100,0,sizeof command_permits100);
                    InterlockedExchange(&command_fire_mode100,LONG(next));ReleaseSRWLockExclusive(&command_volley_lock100);
                    commander_ack_order102(3);
                    command_feedback_set100(next==command100::FireMode::Hold?command100::Feedback::FireHold:next==command100::FireMode::AtWill?command100::Feedback::FireAtWill:command100::Feedback::FireCommand);
                }else command_feedback_set100(command100::Feedback::Unavailable,command_error100);
            }
        }
        if(state->follow&&!busy&&!command_placing100){
            // A normal native formation centered on the leader forms beside and
            // around them, rather than chasing a point six metres behind.
            float initial=field<unsigned short>(possessed,0x64)*command100::tau/65536.f;
            if(frame.tick>=command_follow100.next){
                if(command_formation_gap102(camera,frame.position,command_gap102)){
                    // Match the same native walk/run selection as personal
                    // locomotion. Catch-up can request run even while stationary;
                    // the stock order consumer still applies native run permission.
                    const unsigned input=personal_travel_input91(
                        equipment81::movement_input(unsigned(state->input),equipment_crouch81!=0),
                        true,state->aiming||state->aim_pose,possession_lease.identity.mount!=0);
                    bool translating=bool(input&FORWARD)!=bool(input&BACKWARD)||(!possession_lease.identity.mount&&(bool(input&LEFT)!=bool(input&RIGHT)));
                    bool leader_running=native_sprint_requested133(input)&&translating&&
                        reinterpret_cast<UnitRunAllowed>(base+0x15cb80)(reinterpret_cast<void*>(unit));
                    const bool running=command_pace102.update(command_gap102,leader_running);
                    if(command_follow100.changed(frame.position,frame.tick,running)){
                        float heading=command_follow100.heading(frame.position,initial);
                        auto goal=command_follow100.target(frame.position);MovePoint100 destination{};
                        if(command_actor_point100(goal,destination)&&command_serialize100(&destination,command100::bearing(heading),0,-1,false,UnitState100::Fire,nullptr,nullptr,running)){
                            command_follow100.accepted(goal,heading,frame.tick,running);command_follow_run102=running?1:0;InterlockedIncrement(&follow_updates101);
                        }
                    }
                }
                command_follow100.next=frame.tick+250;
            }
        }
        command100::Status status{};
        command_mapping_publish100(status);
    }__except(EXCEPTION_EXECUTE_HANDLER){command_feedback_set100(command100::Feedback::Unavailable,60);InterlockedExchange(&command_placing100,0);}
    InterlockedExchange(&order_busy,0);
}
