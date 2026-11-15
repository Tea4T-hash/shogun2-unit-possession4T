// Direct production branches against synthetic objects in THIS process only.
#include "adapter.cpp"
static unsigned intents,cancels;static uintptr_t intent_actor,cancel_camera;static float intent_x,intent_z;static unsigned intent_heading;
static void __fastcall fixture_intent(void* actor,void*,const float* xz,unsigned heading){
    ++intents;intent_actor=reinterpret_cast<uintptr_t>(actor);intent_x=xz[0];intent_z=xz[1];intent_heading=heading;
}
static void __fastcall fixture_cancel(void* camera,void*){++cancels;cancel_camera=reinterpret_cast<uintptr_t>(camera);}
int main(){void* memory=nullptr;try{
    Status status{};state=&status;enabled=1;
    memory=VirtualAlloc(nullptr,0x1bc9000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);require(memory!=nullptr,"Fixture allocation");base=reinterpret_cast<uintptr_t>(memory);
    auto code=reinterpret_cast<unsigned char*>(base+0x3936f0);code[0]=0xe9;
    int32_t delta=int32_t(reinterpret_cast<uintptr_t>(&fixture_intent)-(base+0x3936f0+5));memcpy(code+1,&delta,4);
    DWORD prior=0;require(VirtualProtect(reinterpret_cast<void*>((base+0x3936f0)&~uintptr_t(4095)),4096,PAGE_EXECUTE_READ,&prior)!=0,"Fixture executable code");
    FlushInstructionCache(GetCurrentProcess(),code,5);
    std::vector<unsigned char> root(0x120),env(0x28300),common(0x100),phase(0x40),manager(0x900),cam(0x200),view(0x40),holder(0x20),config(0x100),actor(0x700),enemy(0x700),player_unit(0x1000),enemy_unit(0x1000);
    auto ptr=[](auto& bytes){return reinterpret_cast<uintptr_t>(bytes.data());};
    possessed=ptr(actor);environment=ptr(env);unit=ptr(player_unit);auto enemy_actor=ptr(enemy);
    field<uintptr_t>(base,0x1bc8180)=ptr(root);field<uintptr_t>(ptr(root),0x110)=environment;
    field<uintptr_t>(environment,8)=ptr(common);field<uintptr_t>(ptr(common),0xb4)=ptr(phase);field<unsigned>(ptr(phase),0x14)=6;
    field<uintptr_t>(environment,0x281fc)=ptr(manager);field<uintptr_t>(ptr(manager),0x8e0)=ptr(cam);field<uintptr_t>(ptr(manager),0x8fc)=possessed;
    field<uintptr_t>(ptr(cam),0)=base+0x15d22e8;field<uintptr_t>(ptr(cam),0x180)=ptr(manager);field<uintptr_t>(ptr(cam),0x184)=ptr(view);field<uintptr_t>(ptr(cam),0x188)=possessed;
    field<uintptr_t>(ptr(holder),8)=possessed;field<unsigned>(unit,4)=7;field<unsigned>(ptr(enemy_unit),4)=8;
    for(auto a:{possessed,enemy_actor}){field<uintptr_t>(a,0)=base+0x15dfec8;field<uintptr_t>(a,0x20c)=ptr(config);field<unsigned char>(a,0xf0)=1;field<float>(a,0x48)=3;field<float>(a,0x50)=4;}
    field<uintptr_t>(possessed,0x214)=unit;field<uintptr_t>(enemy_actor,0x214)=ptr(enemy_unit);field<unsigned>(possessed,0x1c8)=0;field<unsigned>(enemy_actor,0x1c8)=9;
    field<float>(ptr(config),0x6c)=1.4f;field<float>(base,0x15b2448)=.02f;
    PossessionIdentity identity{environment,ptr(common),ptr(phase),ptr(manager),ptr(cam),ptr(view),ptr(holder),possessed,unit,ptr(config),0,7};
    require(possession_lease.acquire(identity),"Fixture lease");state->mode=POSSESSED;state->actor_id=0;state->unit_id=7;state->lease_generation=possession_lease.generation;state->heartbeat=GetTickCount64();
    melee60::Request r;r.pid=GetCurrentProcessId();r.lease=state->lease_generation;r.player=0;r.player_unit=7;r.opponent=9;r.opponent_unit=8;r.serial=1;r.heartbeat=GetTickCount64();r.phase=melee60::Facing;r.opponent_position[0]=10;r.player_position[0]=0;
    state->aiming=state->fire_request=state->weapon_phase=state->aim_pose=1;
    require(PossessionMeleeCommitment63(&r)==63&&state->aiming==0&&state->fire_request==0&&state->weapon_phase==0&&state->aim_pose==0,"Export rejected valid request");
    require(apply_melee_motion(possessed)&&intents==1&&intent_actor==possessed&&intent_x==3&&intent_z==4&&intent_heading>0,"Player hold/face intent or ABI");
    require(apply_melee_motion(enemy_actor)&&intents==2&&intent_actor==enemy_actor,"Opponent did not hold/face");
    auto calls=intents;r.phase=melee60::NativeCombat;PossessionMeleeCommitment63(&r);
    require(apply_melee_motion(possessed)&&apply_melee_motion(enemy_actor)&&intents==calls,"Native combat must receive no player intent");
    field<uintptr_t>(possessed,0x688)=1;require(apply_melee_motion(possessed)&&intents==calls,"Unavailable camera observation changed native combat control");field<uintptr_t>(possessed,0x688)=0;
    field<unsigned>(ptr(enemy_unit),0xf7c)=1;require(apply_melee_motion(enemy_actor)&&intents==calls,"Ranged melee pair bypassed commitment or injected movement during native playback");field<unsigned>(ptr(enemy_unit),0xf7c)=0;
    field<unsigned>(enemy_actor,0x1c8)=10;require(!apply_melee_motion(enemy_actor),"Foreign/reused actor entered pair steering");field<unsigned>(enemy_actor,0x1c8)=9;
    // Stale Facing packets must not rewind native animation root motion.
    std::vector<unsigned char> witness(0x100);auto w=ptr(witness);
    field<unsigned char>(w,0xc)=field<unsigned char>(w,0xd1)=1;field<unsigned>(w,0x38)=2;
    field<uintptr_t>(w,0x30)=possessed;field<uintptr_t>(w,0x34)=enemy_actor;
    field<uintptr_t>(possessed,0x688)=field<uintptr_t>(enemy_actor,0x688)=w;
    r.phase=melee60::Facing;r.player_anchor[0]=3;r.player_anchor[2]=4;r.opponent_anchor[0]=3;r.opponent_anchor[2]=4;PossessionMeleeCommitment63(&r);
    for(auto a:{possessed,enemy_actor}){field<float>(a,0x48)=9;field<float>(a,0x50)=11;
        require(!hold_melee_motion(a)&&apply_melee_motion(a)&&field<float>(a,0x48)==9&&field<float>(a,0x50)==11&&intents==calls,"Stale Facing restored pre-animation root or injected intent");}
    melee60::Request clear;PossessionMeleeCommitment63(&clear);
    require(finish_melee_handoff78(possessed)&&melee_handoff78.pending&&intents==calls,"Live native playback was stolen at handoff");
    field<unsigned char>(w,0xd1)=0;require(finish_melee_handoff78(possessed)&&melee_handoff78.pending,"Active native alignment was stolen at handoff");
    field<uintptr_t>(possessed,0x688)=field<uintptr_t>(enemy_actor,0x688)=0;field<unsigned char>(possessed,0xa1)=1;
    field<float>(possessed,0x8c)=17;field<float>(possessed,0xd8)=-31;
    require(finish_melee_handoff78(possessed)&&!melee_handoff78.pending&&intents==calls+1&&intent_x==9&&intent_z==11,
        "Handoff did not adopt the exact current native endpoint");
    require(!melee_handoff78.take_resume(possessed,possession_lease.generation+1,field<unsigned>(possessed,0x1c8),field<unsigned>(unit,4)),
        "Melee resume leaked to a foreign lease");
    require(field<float>(possessed,0x48)==9&&field<float>(possessed,0x50)==11&&field<float>(possessed,0x8c)==0&&field<float>(possessed,0xd8)==0&&!finish_melee_handoff78(possessed),
        "Handoff rewound position, kept residual motion or replayed twice");
    // Native motion completion can publish a fresh speed after cleanup.
    field<float>(possessed,0x1ac)=field<float>(possessed,0x94)=12;
    field<float>(possessed,0x160)=4;field<float>(possessed,0x104)=2;
    field<float>(base,0x15de874)=-.02f;
    const bool resume=melee_handoff78.take_resume(possessed,possession_lease.generation,field<unsigned>(possessed,0x1c8),field<unsigned>(unit,4));
    float speed=4;
    require(resume&&apply_native_speed_step(possessed,{0,1,0},true,.02f,1,speed,resume)&&std::abs(speed-.08f)<.001f,
        "Native melee momentum accelerated the first manual step");
    require(!melee_handoff78.take_resume(possessed,possession_lease.generation,field<unsigned>(possessed,0x1c8),field<unsigned>(unit,4)),
        "Melee resume applied twice");
    r.heartbeat=GetTickCount64()-501;PossessionMeleeCommitment63(&r);require(!apply_melee_motion(possessed),"Expired channel retained steering");
    r.heartbeat=GetTickCount64();r.lease++;PossessionMeleeCommitment63(&r);require(!apply_melee_motion(possessed),"Different possession lease retained steering");
    melee60::Request off;PossessionMeleeCommitment63(&off);
    deathpose::Eye native_eye{{3,2,4,3,2,5},{0,1,0}};
    memcpy(reinterpret_cast<void*>(ptr(cam)+8),native_eye.view,sizeof native_eye.view);memcpy(reinterpret_cast<void*>(ptr(cam)+0x58),native_eye.up,sizeof native_eye.up);
    ModelEye native_sample;native_sample.lease=possession_lease.generation;native_sample.actor=0;native_sample.unit=7;native_sample.tick=GetTickCount64();native_sample.eye=native_eye;
    publish_model_eye(native_eye_channel78,native_sample);field<float>(ptr(cam),0x1c)=3;remember_owned_eye(ptr(cam));
    field<unsigned char>(possessed,0xf0)=0;state->input=FORWARD;state->aiming=1;state->fire_request=1;state->pending_order=ORDER_ADVANCE;state->follow=1;
    require(!followed(possessed,true,false),"Dead soldier could be acquired");
    require(followed(possessed,false)&&dead_view(),"Known corpse lost FPS lease");
    require(memcmp(death_eye,native_eye.view,sizeof death_eye)==0,"Death retained free-look backwards instead of native face direction");
    require(!state->input&&!state->aiming&&!state->fire_request&&!state->pending_order&&!state->follow,"Death did not cancel controls/orders");
    auto start=death_started;require(followed(possessed,false)&&death_started==start,"Death timer restarted");
    field<unsigned>(possessed,0x1c8)=1;require(followed(possessed,false),"Confirmed world snapshot still depends on corpse serial");field<unsigned>(possessed,0x1c8)=0;
    field<uintptr_t>(ptr(holder),8)=enemy_actor;require(followed(possessed,false),"Confirmed world snapshot still depends on roster holder");field<uintptr_t>(ptr(holder),8)=possessed;
    field<uintptr_t>(ptr(root),0x110)=0;require(!followed(possessed,false),"Ended battle retained corpse");field<uintptr_t>(ptr(root),0x110)=environment;
    InterlockedExchange64(&death_started,LONG64(GetTickCount64()-15001));require(followed(possessed,false),"Window dispatch grace lost corpse before RTS return");
    InterlockedExchange64(&death_started,LONG64(GetTickCount64()-16001));require(!followed(possessed,false),"Stalled window retained corpse indefinitely");
    InterlockedExchange(&death_return_pending,1);original_cancel=reinterpret_cast<Cancel>(&fixture_cancel);end_death_on_window();
    require(cancels==1&&cancel_camera==ptr(cam)&&!possessed&&!possession_lease.active&&!dead_view()&&state->mode==IDLE,"Death return did not cancel the CAMERA and restore RTS ownership");
    VirtualFree(memory,0,MEM_RELEASE);std::cout<<"PASS: production commitment export, player/opponent native intent ABI, hold/face, native combat handoff, ranged/identity/freshness guards; known-corpse lease, disabled death controls, identity/battle retirement, 15s dispatch deadline, bounded fallback and correct camera cancellation. No live game modified.\n";return 0;
}catch(const std::exception& e){if(memory)VirtualFree(memory,0,MEM_RELEASE);std::cerr<<e.what()<<'\n';return 1;}}


