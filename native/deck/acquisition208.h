#pragma once
// Some native combat controllers (177e30, for example) bypass16aa00 and call
// the shared discovery/scoring routines directly. Give each entry its own
// bounded scope; never rely on a previous query leaving TLS state behind.
using AcquisitionCone208=void(__stdcall*)(uintptr_t,void*,void*,unsigned);
using AcquisitionRadial208=uintptr_t(__stdcall*)(void*,uintptr_t,float,unsigned);
using AcquisitionScore208=uintptr_t(__cdecl*)(uintptr_t,void*,void*,float,float);
static AcquisitionCone208 original_cone208;
static AcquisitionRadial208 original_radial208;
static AcquisitionScore208 original_score208;
static void __stdcall cone_scope208(uintptr_t side,void* cone,void* candidates,unsigned append){
    if(!unit||!possessed){original_cone208(side,cone,candidates,append);return;}
    const auto saved=acquisition207;acquisition207={0,0,0,3,side};
    InterlockedIncrement(&motion_callback_depth);
    __try{original_cone208(side,cone,candidates,append);}
    __finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
static uintptr_t __stdcall radial_scope208(void* candidates,uintptr_t observer,float range,unsigned include_radius){
    if(!unit||!possessed)return original_radial208(candidates,observer,range,include_radius);
    const auto saved=acquisition207;acquisition207={observer,0,0,1};
    InterlockedIncrement(&motion_callback_depth);
    __try{return original_radial208(candidates,observer,range,include_radius);}
    __finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
static uintptr_t __cdecl score_scope208(uintptr_t observer,void* candidates,void* current,float range,float extra){
    if(!unit||!possessed)return original_score208(observer,candidates,current,range,extra);
    const auto saved=acquisition207;acquisition207={observer,0,0,4};
    InterlockedIncrement(&motion_callback_depth);
    __try{return original_score208(observer,candidates,current,range,extra);}
    __finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
// Unit-pair distances are cached from the two PRIMARY formations by47efc0.
// Return the nearer native rectangle distance for an owned separated member,
// without changing that shared cache, the parent centre or any AI order.
// Native callers pass observer first. Queries made BY the parent formation
// stay unchanged; its commands must not use the distant player as its centre.
using AcquisitionDistance208=float(__thiscall*)(void*,uintptr_t,uintptr_t);
static AcquisitionDistance208 original_distance208;
static float __fastcall distance208(void* cache,void*,uintptr_t first,uintptr_t second){
    if(!unit||!possessed||second!=unit||first==unit)
        return original_distance208(cache,first,second);
    const auto saved=acquisition207;acquisition207={first,0,0,4};
    InterlockedIncrement(&motion_callback_depth);
    __try{
        float ordinary=original_distance208(cache,first,second);
        __try{
            auto primary=field<uintptr_t>(unit,0xb9c),independent=independent207(unit,primary);
            if(!independent||!std::isfinite(ordinary)||ordinary<=0)return ordinary;
            auto observer=field<uintptr_t>(acquisition207.observer,0xb9c);
            using Squared=float(__thiscall*)(void*,void*);
            float squared=reinterpret_cast<Squared>(base+0x1aaed0)(reinterpret_cast<void*>(observer+0x644),reinterpret_cast<void*>(independent+0x644));
            if(!std::isfinite(squared)||squared<0)return ordinary;
            float distance=std::sqrt(squared);
            return distance<ordinary?distance:ordinary;
        }__except(EXCEPTION_EXECUTE_HANDLER){return ordinary;}
    }__finally{acquisition207=saved;InterlockedDecrement(&motion_callback_depth);}
}
static void create_acquisition208(){
    const unsigned char distance[]={0x53,0x8b,0x5c,0x24,0x08};
    require(!memcmp(reinterpret_cast<void*>(base+0x47efc0),distance,sizeof distance),"Native unit-distance entry differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x47efc0),reinterpret_cast<void*>(&distance208),reinterpret_cast<void**>(&original_distance208))==MH_OK,"Native unit-distance hook failed");
    // Whole instruction prefixes, exact in stock and reviewed Bugfix engines.
    const unsigned char cone[]={0x83,0xec,0x18,0x8b,0x44,0x24,0x1c};
    const unsigned char radial[]={0x51,0x53,0x56,0x57,0x8b,0x7c,0x24,0x14};
    const unsigned char score[]={0x83,0xec,0x58,0x8b,0x4c,0x24,0x60};
    require(!memcmp(reinterpret_cast<void*>(base+0x48d6a0),cone,sizeof cone)&&
            !memcmp(reinterpret_cast<void*>(base+0x48d860),radial,sizeof radial)&&
            !memcmp(reinterpret_cast<void*>(base+0x1752d0),score,sizeof score),"Shared native acquisition208 entry differs");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x48d6a0),reinterpret_cast<void*>(&cone_scope208),reinterpret_cast<void**>(&original_cone208))==MH_OK,"Shared native cone scope failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x48d860),reinterpret_cast<void*>(&radial_scope208),reinterpret_cast<void**>(&original_radial208))==MH_OK,"Shared native radial scope failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x1752d0),reinterpret_cast<void*>(&score_scope208),reinterpret_cast<void**>(&original_score208))==MH_OK,"Shared native scoring scope failed");
}
