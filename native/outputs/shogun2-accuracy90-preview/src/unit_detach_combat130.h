#pragma once
// Detached actors retain personal native combat tasks. These consumers need
// position/facing values, not renewed squad participation or a fake controller.
static thread_local uintptr_t detached_task_actor130;
static thread_local unsigned detached_position130[5];
using NativePose130=uintptr_t(__thiscall*)(void*);
using NativePair130=void(__thiscall*)(void*,uintptr_t,uintptr_t,unsigned);
using NativeNearby130=bool(__thiscall*)(void*);
static UnitTick130 original_personal_idle130,original_personal_move130,original_personal_advance137;
static NativePose130 original_squad_position130;
static NativePair130 original_squad_pair130;
static NativeNearby130 original_squad_nearby130;
static void* original_heading_idle130;static void* original_heading_move130;
static uintptr_t heading_idle_resume130,heading_move_resume130;
static bool detached_actor_live130(uintptr_t actor){
    if(!actor||!detached_count130||!detached_buckets130[detached_bucket130(actor)])return false;
    InterlockedIncrement(&detached_proofs130);
    bool result=false,locked=!member_sync_active130;
    if(locked)AcquireSRWLockShared(&member_lock130);
    __try{__try{
        for(const auto& r:detached130)if(r.actor==actor){
            auto root=field<uintptr_t>(base,0x1bc8180),env=root?field<uintptr_t>(root,0x110):0;
            auto common=env?field<uintptr_t>(env,8):0,phase=common?field<uintptr_t>(common,0xb4):0;
            auto id=phase?field<unsigned>(phase,0x14):0;
            result=env==r.environment&&phase==r.phase&&id>=5&&id<=8&&
                field<uintptr_t>(actor,0x214)==r.unit&&field<unsigned>(r.unit,4)==r.unit_id&&
                field<uintptr_t>(r.unit,0x14)==env&&unit_member130(r.unit,actor)&&
                field<unsigned>(actor,0x1c8)==r.actor_id&&field<uintptr_t>(actor,0x20c)==r.config&&
                field<unsigned char>(actor,0xf0)==1&&!field<uintptr_t>(actor,0x3b4);break;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){result=false;}}
    __finally{if(locked)ReleaseSRWLockShared(&member_lock130);}
    return result;
}
static void personal_task130(void* task,UnitTick130 original){
    uintptr_t previous=detached_task_actor130;
    auto actor=field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4);
    // A personal wall approach, traversal and landing own this soldier.
    // Formation idle/move tasks must not replace its goal with its old slot.
    if(climb_independent200(actor))return;
    detached_task_actor130=detached_actor_live130(actor)?actor:0;
    __try{original(task);}__finally{detached_task_actor130=previous;}
}
static void __fastcall personal_idle130(void* task,void*){personal_task130(task,original_personal_idle130);}
static void __fastcall personal_move130(void* task,void*){personal_task130(task,original_personal_move130);}
// Native advance task also reads squad+74c before applying personal motion.
static void __fastcall personal_advance137(void* task,void*){personal_task130(task,original_personal_advance137);}
static uintptr_t __fastcall squad_position130(void* context,void*){
    auto caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
    if(reinterpret_cast<uintptr_t>(context)==0x628&&(caller==base+0x3edc90||caller==base+0x3ee73d)&&detached_actor_live130(detached_task_actor130)){
        // Native 399f80 reads an absolute position value with discriminator+10=0.
        for(unsigned i=0;i<3;++i)detached_position130[i]=field<unsigned>(detached_task_actor130,0x48+i*4);
        detached_position130[3]=detached_position130[4]=0;
        return reinterpret_cast<uintptr_t>(detached_position130);
    }
    return original_squad_position130(context);
}
static unsigned __cdecl detached_heading130(uintptr_t actor){
    return detached_task_actor130==actor&&detached_actor_live130(actor)?0x10000u|field<unsigned short>(actor,0x64):0;
}
static void __fastcall squad_pair130(void* context,void*,uintptr_t actor,uintptr_t other,unsigned mode){
    if(reinterpret_cast<uintptr_t>(context)==0x74c&&detached_task_actor130==actor&&detached_actor_live130(actor))return;
    original_squad_pair130(context,actor,other,mode);
}
static bool __fastcall squad_nearby130(void* context,void*){
    if(reinterpret_cast<uintptr_t>(context)==0x74c&&detached_actor_live130(detached_task_actor130))return false;
    return original_squad_nearby130(context);
}
static __declspec(naked) void heading_idle130(){
    __asm{
        pushfd
        test eax,eax
        jnz native_existing
        pushad
        sub esp,128
        movdqu [esp+0],xmm0
        movdqu [esp+16],xmm1
        movdqu [esp+32],xmm2
        movdqu [esp+48],xmm3
        movdqu [esp+64],xmm4
        movdqu [esp+80],xmm5
        movdqu [esp+96],xmm6
        movdqu [esp+112],xmm7
        push edx
        call detached_heading130
        add esp,4
        test eax,eax
        jz unchanged
        and eax,0xffff
        mov [esp+156],eax
        movdqu xmm0,[esp+0]
        movdqu xmm1,[esp+16]
        movdqu xmm2,[esp+32]
        movdqu xmm3,[esp+48]
        movdqu xmm4,[esp+64]
        movdqu xmm5,[esp+80]
        movdqu xmm6,[esp+96]
        movdqu xmm7,[esp+112]
        add esp,128
        popad
        popfd
        jmp dword ptr [heading_idle_resume130]
    unchanged:
        movdqu xmm0,[esp+0]
        movdqu xmm1,[esp+16]
        movdqu xmm2,[esp+32]
        movdqu xmm3,[esp+48]
        movdqu xmm4,[esp+64]
        movdqu xmm5,[esp+80]
        movdqu xmm6,[esp+96]
        movdqu xmm7,[esp+112]
        add esp,128
        popad
        popfd
        jmp dword ptr [original_heading_idle130]
    native_existing:
        popfd
        jmp dword ptr [original_heading_idle130]
    }
}
static __declspec(naked) void heading_move130(){
    __asm{
        pushfd
        test eax,eax
        jnz native_existing
        pushad
        sub esp,128
        movdqu [esp+0],xmm0
        movdqu [esp+16],xmm1
        movdqu [esp+32],xmm2
        movdqu [esp+48],xmm3
        movdqu [esp+64],xmm4
        movdqu [esp+80],xmm5
        movdqu [esp+96],xmm6
        movdqu [esp+112],xmm7
        push edx
        call detached_heading130
        add esp,4
        test eax,eax
        jz unchanged
        and eax,0xffff
        mov [esp+156],eax
        movdqu xmm0,[esp+0]
        movdqu xmm1,[esp+16]
        movdqu xmm2,[esp+32]
        movdqu xmm3,[esp+48]
        movdqu xmm4,[esp+64]
        movdqu xmm5,[esp+80]
        movdqu xmm6,[esp+96]
        movdqu xmm7,[esp+112]
        add esp,128
        popad
        popfd
        jmp dword ptr [heading_move_resume130]
    unchanged:
        movdqu xmm0,[esp+0]
        movdqu xmm1,[esp+16]
        movdqu xmm2,[esp+32]
        movdqu xmm3,[esp+48]
        movdqu xmm4,[esp+64]
        movdqu xmm5,[esp+80]
        movdqu xmm6,[esp+96]
        movdqu xmm7,[esp+112]
        add esp,128
        popad
        popfd
        jmp dword ptr [original_heading_move130]
    native_existing:
        popfd
        jmp dword ptr [original_heading_move130]
    }
}
static void create_detach_combat130(){
    struct Site {unsigned rva;void* detour;void** original;};
    const unsigned char heading[]={0x66,0x8b,0x80,0x4c,6,0,0};
    require(!memcmp(reinterpret_cast<void*>(base+0x3edcba),heading,7)&&!memcmp(reinterpret_cast<void*>(base+0x3ee767),heading,7),"Detached heading consumers differ");
    const unsigned char pose[]={0x8d,0x81,0xb4,0x0,0x0,0x0,0xc3};
    const unsigned char nearby[]={0x83,0x79,0x10,0x0,0xf,0x95,0xc0,0xc3,0xcc,0xcc,0xcc,0xcc};
    require(!memcmp(reinterpret_cast<void*>(base+0x15d5d0),pose,sizeof pose)&&!memcmp(reinterpret_cast<void*>(base+0x17bee0),nearby,sizeof nearby),"Detached position/coordination readers differ");
    const unsigned char idle_task[]={0x83,0xec,0x3c,0x53,0x8b,0xd9,0x56,0x57,0x89,0x5c,0x24,0xc};
    require(!memcmp(reinterpret_cast<void*>(base+0x3ed950),idle_task,sizeof idle_task),"Detached native task signature differs");
    const unsigned char move_task[]={0x83,0xec,0x4c,0x53,0x8b,0xd9,0x56,0x57,0x89,0x5c,0x24,0xc};
    require(!memcmp(reinterpret_cast<void*>(base+0x3ee240),move_task,sizeof move_task),"Detached native task signature differs");
    const unsigned char advance[]={0x83,0xec,0x54,0xf3,0x0f,0x10,0x25};
    const unsigned char advance_tail[]={0x0f,0x57,0xc0,0x53,0x56};
    require(!memcmp(reinterpret_cast<void*>(base+0x3eba00),advance,sizeof advance)&&
        field<uintptr_t>(base+0x3eba00,7)==base+0x15b183c&&!memcmp(reinterpret_cast<void*>(base+0x3eba0b),advance_tail,sizeof advance_tail),"Detached native advance task differs");
    const unsigned char pair[]={0x83,0xec,0x10,0x56,0x8b,0x74,0x24,0x1c,0x57,0x8b,0xf9,0x83};
    require(!memcmp(reinterpret_cast<void*>(base+0x14ad80),pair,sizeof pair),"Detached native task signature differs");
    heading_idle_resume130=base+0x3edcc1;heading_move_resume130=base+0x3ee76e;
    Site sites[]={
        {0x3eba00,reinterpret_cast<void*>(&personal_advance137),reinterpret_cast<void**>(&original_personal_advance137)},
        {0x3ed950,reinterpret_cast<void*>(&personal_idle130),reinterpret_cast<void**>(&original_personal_idle130)},
        {0x3ee240,reinterpret_cast<void*>(&personal_move130),reinterpret_cast<void**>(&original_personal_move130)},
        {0x15d5d0,reinterpret_cast<void*>(&squad_position130),reinterpret_cast<void**>(&original_squad_position130)},
        {0x14ad80,reinterpret_cast<void*>(&squad_pair130),reinterpret_cast<void**>(&original_squad_pair130)},
        {0x17bee0,reinterpret_cast<void*>(&squad_nearby130),reinterpret_cast<void**>(&original_squad_nearby130)},
        {0x3edcba,reinterpret_cast<void*>(&heading_idle130),&original_heading_idle130},
        {0x3ee767,reinterpret_cast<void*>(&heading_move130),&original_heading_move130}};
    for(auto& s:sites)require(MH_CreateHook(reinterpret_cast<void*>(base+s.rva),s.detour,s.original)==MH_OK,"Detached personal-combat compatibility hook failed");
}
