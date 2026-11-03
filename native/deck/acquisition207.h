#pragma once
// Native candidate discovery must see the existing, registered one-member
// group as well as its parent formation. No unit/roster pointer is rewritten.
struct Acquisition207 {uintptr_t observer{},chosen{},probe{};unsigned kind{};uintptr_t side{};};
static thread_local Acquisition207 acquisition207;
using AcquisitionSearch207=uintptr_t(__thiscall*)(void*,void*);
using AcquisitionNeighbours207=void(__thiscall*)(void*);
static AcquisitionSearch207 original_search207;
static AcquisitionNeighbours207 original_neighbours207;
static uintptr_t __fastcall search207(void* context,void*,void* result){
    const auto saved=acquisition207;acquisition207={};
    InterlockedIncrement(&motion_callback_depth);
    __try{
        __try{auto p=reinterpret_cast<uintptr_t>(context);
            if(field<uintptr_t>(p,0xc)==environment)acquisition207={field<uintptr_t>(p,0x10),0,0,1};
        }__except(EXCEPTION_EXECUTE_HANDLER){}
        return original_search207(context,result);
    }__finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
static void __fastcall neighbours207(void* context,void*){
    const auto saved=acquisition207;acquisition207={};
    InterlockedIncrement(&motion_callback_depth);
    __try{
        __try{acquisition207={field<uintptr_t>(reinterpret_cast<uintptr_t>(context),4),0,0,2};}__except(EXCEPTION_EXECUTE_HANDLER){}
        original_neighbours207(context);
    }__finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
static uintptr_t independent207(uintptr_t target,uintptr_t primary){
    if(!acquisition207.kind||!target||target!=unit||acquisition207.observer==target)return 0;
    auto actor=locomotion_actor();
    if(!member_owned130(actor)||!proximity_independent200(actor)||field<uintptr_t>(target,0xb9c)!=primary||!squad_registered130(primary,target,environment))return 0;
    auto independent=field<uintptr_t>(actor,0x3b4),observer=acquisition207.observer;
    if(independent==primary||field<unsigned>(independent,0x18)!=1||squad_matches130(independent,actor)!=1)return 0;
    uintptr_t side=acquisition207.side;
    if(acquisition207.kind==3){
        // Shared cone queries carry a side, not an observing unit. The exact
        // visible-enemy list below remains authoritative for hostility.
        auto own_side=field<uintptr_t>(actor,0x44);
        if(!side||!own_side||field<uintptr_t>(side,0)!=environment||
           field<unsigned>(side,0x10)>31||field<unsigned>(own_side,0x10)>31||
           field<unsigned>(side,0x10)==field<unsigned>(own_side,0x10))return 0;
    }else{
        if(!observer||field<uintptr_t>(observer,0x14)!=environment)return 0;
        auto group=field<uintptr_t>(observer,0xb9c);
        if(!squad_registered130(group,observer,environment))return 0;
        auto count=field<unsigned>(group,0x18),members=field<uintptr_t>(group,0x1c);
        if(!count||count>1000||!members)return 0;
        auto member=field<uintptr_t>(members,0);
        if(!acquisition_enemy203(member,actor))return 0;
        side=field<uintptr_t>(member,0x44);
    }
    // Native visible-enemy list also respects alliance/visibility decisions.
    // Different numerical side IDs alone do not establish hostility.
    auto n=field<unsigned>(side,0x40),list=field<uintptr_t>(side,0x44);
    if(n>2048||!list)return 0;
    for(unsigned i=0;i<n;++i)if(field<uintptr_t>(list,i*4)==target)return independent;
    return 0;
}
static bool closer207(uintptr_t primary,uintptr_t independent,uintptr_t cone=0){
    float points[3][3]{};using Point=float*(__thiscall*)(void*,float*);
    if(cone){points[0][0]=field<float>(cone,0);points[0][2]=field<float>(cone,4);}
    else{
        auto observer=field<uintptr_t>(acquisition207.observer,0xb9c);
        reinterpret_cast<Point>(base+0x399f80)(reinterpret_cast<void*>(observer+0x6dc),points[0]);
    }
    unsigned i=1;for(auto group:{primary,independent})
        reinterpret_cast<Point>(base+0x399f80)(reinterpret_cast<void*>(group+0x6dc),points[i++]);
    for(auto& p:points)for(float v:p)if(!std::isfinite(v)||std::fabs(v)>1000000)return false;
    float old=0,now=0;for(unsigned axis:{0u,2u}){
        float a=points[1][axis]-points[0][axis],b=points[2][axis]-points[0][axis];old+=a*a;now+=b*b;
    }
    return now<old; // Native formation wins ties; its accepted targets survive.
}
static uintptr_t __cdecl cone_group207(uintptr_t target,uintptr_t primary,uintptr_t cone){
    __try{
        if((acquisition207.kind!=1&&acquisition207.kind!=3)||target!=unit)return primary;
        acquisition207.chosen=0;auto independent=independent207(target,primary);
        if(!independent||!cone||!closer207(primary,independent,cone))return primary;
        float p[3]{};using Point=float*(__thiscall*)(void*,float*);
        reinterpret_cast<Point>(base+0x399f80)(reinterpret_cast<void*>(independent+0x6dc),p);
        float dx=p[0]-field<float>(cone,0),dz=p[2]-field<float>(cone,4);
        float radius=field<float>(cone,0x18)+field<float>(independent,0x670);
        if(!std::isfinite(radius)||radius<0||!(dx*dx+dz*dz<radius*radius))return primary;
        // Original48d6a0's exact circle broad phase and native cone intersection.
        using Intersects=bool(__thiscall*)(void*,const void*);
        if(!reinterpret_cast<Intersects>(base+0x4589a0)(reinterpret_cast<void*>(cone),reinterpret_cast<void*>(independent+0x644)))return primary;
        acquisition207.chosen=independent;return independent;
    }__except(EXCEPTION_EXECUTE_HANDLER){return primary;}
}
static uintptr_t __cdecl radial_group207(uintptr_t target,uintptr_t primary,uintptr_t observer_geometry,float range,unsigned include_radius){
    __try{
        if(acquisition207.kind!=1||target!=unit||observer_geometry!=field<uintptr_t>(acquisition207.observer,0xb9c)+0x628)return primary;
        auto independent=independent207(target,primary);if(!independent)return primary;
        if(acquisition207.probe==independent)return independent;
        acquisition207.chosen=0;if(!closer207(primary,independent)||!std::isfinite(range)||range<0)return primary;
        // Re-enter the ORIGINAL bounded range routine with only its local group
        // register changed. The probe flag prevents recursion; no fake unit.
        const auto saved=acquisition207.probe;bool accepted=false;acquisition207.probe=independent;
        __try{using Range=bool(__stdcall*)(uintptr_t,uintptr_t,float,unsigned);
            accepted=reinterpret_cast<Range>(base+0x459b40)(observer_geometry,target,range,include_radius);
        }__finally{acquisition207.probe=saved;}
        if(!accepted)return primary;acquisition207.chosen=independent;return independent;
    }__except(EXCEPTION_EXECUTE_HANDLER){return primary;}
}
static uintptr_t __cdecl score_group207(uintptr_t target,uintptr_t observer,float nominal_range){
    auto primary=field<uintptr_t>(target,0xb9c);
    __try{if((acquisition207.kind==1||acquisition207.kind==4)&&observer==acquisition207.observer){
        auto independent=independent207(target,primary);
        if(independent&&((acquisition207.kind==4&&closer207(primary,independent))||
                         (acquisition207.kind==1&&independent==acquisition207.chosen))){
            // Preserve the formation if this smaller group fails either native
            // scoring range gate. Native scoring repeats both on the chosen group.
            using Range=float(__thiscall*)(void*);
            using Adjust=float(__thiscall*)(void*,float,void*);
            using Within=bool(__thiscall*)(void*,void*,float);
            auto geometry=field<uintptr_t>(observer,0xb9c)+0x628;
            float range=reinterpret_cast<Range>(base+0x18a590)(reinterpret_cast<void*>(observer));
            float adjusted=reinterpret_cast<Adjust>(base+0x15ae80)(reinterpret_cast<void*>(geometry),range,reinterpret_cast<void*>(independent+0x628));
            auto within=reinterpret_cast<Within>(base+0x180cd0);
            if(std::isfinite(adjusted)&&std::isfinite(nominal_range)&&adjusted>=0&&nominal_range>=0&&
               within(reinterpret_cast<void*>(geometry+0x1c),reinterpret_cast<void*>(independent+0x644),adjusted)&&
               within(reinterpret_cast<void*>(geometry+0x1c),reinterpret_cast<void*>(independent+0x644),nominal_range))return independent;
        }
    }}__except(EXCEPTION_EXECUTE_HANDLER){}
    return primary;
}
static uintptr_t __cdecl neighbour_group207(uintptr_t target){
    auto primary=field<uintptr_t>(target,0xb9c);
    __try{if(acquisition207.kind==2){auto independent=independent207(target,primary);
        if(independent&&closer207(primary,independent)){
            auto group=field<uintptr_t>(acquisition207.observer,0xb9c);
            float a[3]{},b[3]{};using Point=float*(__thiscall*)(void*,float*);
            auto point=reinterpret_cast<Point>(base+0x399f80);
            point(reinterpret_cast<void*>(group+0x6dc),a);point(reinterpret_cast<void*>(independent+0x6dc),b);
            float dx=a[0]-b[0],dz=a[2]-b[2];
            // Exact 1b8740 neighbourhood radius (native constant = 100).
            float radius=field<float>(group,0x670)+field<float>(base,0x15b3680)+field<float>(independent,0x670);
            if(std::isfinite(radius)&&radius>=0&&dx*dx+dz*dz<radius*radius)return independent;
        }
    }}__except(EXCEPTION_EXECUTE_HANDLER){}
    return primary;
}

static uintptr_t cone207_resume;
static __declspec(naked) void cone207(){__asm{
 mov esi,dword ptr [edx+0xb9c]
 pushfd
 cmp edx,dword ptr [unit]
 jne ordinary207
 pushad
 mov ebp,esp
 sub esp,528
 and esp,-16
 fxsave [esp]
 push dword ptr [ebp+76]
 push dword ptr [ebp+4]
 push dword ptr [ebp+20]
 call cone_group207
 add esp,12
 mov dword ptr [ebp+4],eax
 fxrstor [esp]
 mov esp,ebp
 popad
 popfd
 jmp dword ptr [cone207_resume]
ordinary207:
 popfd
 jmp dword ptr [cone207_resume]
}}

static uintptr_t radial207_resume;
static __declspec(naked) void radial207(){__asm{
 mov esi,dword ptr [eax+0xb9c]
 pushfd
 cmp eax,dword ptr [unit]
 jne ordinary207
 pushad
 mov ebp,esp
 sub esp,528
 and esp,-16
 fxsave [esp]
 push dword ptr [ebp+68]
 push dword ptr [ebp+64]
 push dword ptr [ebp+56]
 push dword ptr [ebp+4]
 push dword ptr [ebp+28]
 call radial_group207
 add esp,20
 mov dword ptr [ebp+4],eax
 fxrstor [esp]
 mov esp,ebp
 popad
 popfd
 jmp dword ptr [radial207_resume]
ordinary207:
 popfd
 jmp dword ptr [radial207_resume]
}}

static uintptr_t score207_resume;
static __declspec(naked) void score207(){__asm{
 pushfd
 cmp esi,dword ptr [unit]
 jne ordinary207
 pushad
 mov ebp,esp
 sub esp,528
 and esp,-16
 fxsave [esp]
 push dword ptr [ebp+148]
 push dword ptr [ebp]
 push dword ptr [ebp+4]
 call score_group207
 add esp,12
 mov dword ptr [ebp+0],eax
 fxrstor [esp]
 mov esp,ebp
 popad
 popfd
 jmp dword ptr [score207_resume]
ordinary207:
 popfd
 mov edi,dword ptr [esi+0xb9c]
 jmp dword ptr [score207_resume]
}}

static uintptr_t score_contact207_resume;
static __declspec(naked) void score_contact207(){__asm{
 pushfd
 cmp esi,dword ptr [unit]
 jne ordinary207
 pushad
 mov ebp,esp
 sub esp,528
 and esp,-16
 fxsave [esp]
 push dword ptr [ebp+148]
 push dword ptr [ebp]
 push dword ptr [ebp+4]
 call score_group207
 add esp,12
 mov dword ptr [ebp+24],eax
 fxrstor [esp]
 mov esp,ebp
 popad
 popfd
 jmp dword ptr [score_contact207_resume]
ordinary207:
 popfd
 mov ecx,dword ptr [esi+0xb9c]
 jmp dword ptr [score_contact207_resume]
}}

static uintptr_t neighbour207_resume;
static __declspec(naked) void neighbour207(){__asm{
 pushfd
 cmp edi,dword ptr [unit]
 jne ordinary207
 pushad
 mov ebp,esp
 sub esp,528
 and esp,-16
 fxsave [esp]
 push dword ptr [ebp]
 call neighbour_group207
 add esp,4
 mov dword ptr [ebp+24],eax
 fxrstor [esp]
 mov esp,ebp
 popad
 popfd
 jmp dword ptr [neighbour207_resume]
ordinary207:
 popfd
 mov ecx,dword ptr [edi+0xb9c]
 jmp dword ptr [neighbour207_resume]
}}

#include "acquisition208.h"
static void create_acquisition207(){
    create_acquisition208();
    const unsigned char search[]={0xb8,0x90,0x50,0,0};
    const unsigned char neighbours[]={0x83,0xec,0x38,0x53,0x56,0x8b,0xf1};
    require(!memcmp(reinterpret_cast<void*>(base+0x16aa00),search,sizeof search)&&!memcmp(reinterpret_cast<void*>(base+0x1b8740),neighbours,sizeof neighbours),"Native acquisition entry differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x16aa00),reinterpret_cast<void*>(&search207),reinterpret_cast<void**>(&original_search207))==MH_OK,"Native ranged acquisition hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1b8740),reinterpret_cast<void*>(&neighbours207),reinterpret_cast<void**>(&original_neighbours207))==MH_OK,"Native melee neighbourhood hook failed");
    {const unsigned char bytes[]={0x8b,0xb2,0x9c,0x0b,0x00,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x48d6f3),bytes,sizeof bytes),"Native cone207 group load differs");
    cone207_resume=base+0x48d6f9;void* trampoline=nullptr;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x48d6f3),reinterpret_cast<void*>(&cone207),&trampoline)==MH_OK,"Native cone207 group hook failed");}
    {const unsigned char bytes[]={0x8b,0xb0,0x9c,0x0b,0x00,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x459b4d),bytes,sizeof bytes),"Native radial207 group load differs");
    radial207_resume=base+0x459b53;void* trampoline=nullptr;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x459b4d),reinterpret_cast<void*>(&radial207),&trampoline)==MH_OK,"Native radial207 group hook failed");}
    {const unsigned char bytes[]={0x8b,0xbe,0x9c,0x0b,0x00,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x175360),bytes,sizeof bytes),"Native score207 group load differs");
    score207_resume=base+0x175366;void* trampoline=nullptr;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x175360),reinterpret_cast<void*>(&score207),&trampoline)==MH_OK,"Native score207 group hook failed");}
    {const unsigned char bytes[]={0x8b,0x8e,0x9c,0x0b,0x00,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x175346),bytes,sizeof bytes),"Native score_contact207 group load differs");
    score_contact207_resume=base+0x17534c;void* trampoline=nullptr;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x175346),reinterpret_cast<void*>(&score_contact207),&trampoline)==MH_OK,"Native score_contact207 group hook failed");}
    {const unsigned char bytes[]={0x8b,0x8f,0x9c,0x0b,0x00,0x00};
    require(!memcmp(reinterpret_cast<void*>(base+0x1b8830),bytes,sizeof bytes),"Native neighbour207 group load differs");
    neighbour207_resume=base+0x1b8836;void* trampoline=nullptr;
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1b8830),reinterpret_cast<void*>(&neighbour207),&trampoline)==MH_OK,"Native neighbour207 group hook failed");}
}
