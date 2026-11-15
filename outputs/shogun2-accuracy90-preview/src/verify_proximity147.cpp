// Original registered SQUAD factory/lifecycle probe. No live game access.
#define main retained_commands147
#include "verify_native_commands100.cpp"
#undef main
#include "native_group_fixture148.h"
static uintptr_t general_role_definition;
static unsigned ring_draw_calls, ring_flags;
static void* ring_renderer;
static uintptr_t ring_original;
static unsigned char ring_explicit_center;
static float ring_x, ring_z, ring_radius;

static void __fastcall fixture_ring_draw67(void* ring, void*, unsigned flags, void* renderer) {
    ++ring_draw_calls;
    ring_flags = flags;
    ring_renderer = renderer;
    ring_original = reinterpret_cast<uintptr_t>(ring);
    ring_explicit_center = field<unsigned char>(ring_original, 0x2c);
    ring_x = field<float>(ring_original, 0x30);
    ring_z = field<float>(ring_original, 0x34);
    ring_radius = field<float>(ring_original, 0x28);
}

static void verify_general_ring67(uintptr_t physical) {
    alignas(uintptr_t) unsigned char ring[0x40]{};
    const auto address = reinterpret_cast<uintptr_t>(ring);
    field<uintptr_t>(address, 0) = base + 0x15d352c;
    field<uintptr_t>(address, 0x1c) = unit;
    field<float>(address, 0x28) = 33.f;
    field<float>(address, 0x30) = -40.f;
    field<float>(address, 0x34) = -80.f;
    auto saved = original_general_ring67;
    original_general_ring67 = reinterpret_cast<GeneralRingDraw67>(&fixture_ring_draw67);
    ring_draw_calls = 0;
    draw_general_ring67(ring, nullptr, 1, &ring_draw_calls);
    require(ring_draw_calls == 1 && ring_original != address && ring_explicit_center == 1 &&
        ring_x == field<float>(physical, 0x48) && ring_z == field<float>(physical, 0x50) &&
        ring_radius == 33.f && ring_flags == 1 && ring_renderer == &ring_draw_calls,
        "Native ring did not get the possessed general center and original draw arguments");
    require(!field<unsigned char>(address, 0x2c) && field<float>(address, 0x30) == -40.f &&
        field<float>(address, 0x34) == -80.f, "Native ring or formation was modified");
    field<uintptr_t>(address, 0x1c) = unit + 4;
    draw_general_ring67(ring, nullptr, 0, &ring_draw_calls);
    require(ring_draw_calls == 2 && ring_original == address && !ring_explicit_center &&
        ring_x == -40.f && ring_z == -80.f && !ring_flags, "Another unit's ring was recentered");
    field<uintptr_t>(address, 0x1c) = unit;
    InterlockedExchange(&state->release, 1);
    draw_general_ring67(ring, nullptr, 1, &ring_draw_calls);
    InterlockedExchange(&state->release, 0);
    require(ring_draw_calls == 3 && ring_original == address && !ring_explicit_center,
        "Released general still moved the ring");
    auto actor_table = field<uintptr_t>(possessed, 0);
    field<uintptr_t>(possessed, 0) = base + 0x15dbfb4;
    draw_general_ring67(ring, nullptr, 1, &ring_draw_calls);
    field<uintptr_t>(possessed, 0) = actor_table;
    require(ring_draw_calls == 4 && ring_original == address && !ring_explicit_center,
        "Ordinary officer moved the general's ring");
    original_general_ring67 = saved;
}
static constexpr DWORD slot_fixture_end68=0xE0533238;
static float officer_slot_path68[3];
static unsigned officer_path_requests68;
static void __fastcall observe_slot_request68(void* order,void*,uintptr_t actor){
    require(actor==possessed,"Native path request targeted a different soldier");
    memcpy(officer_slot_path68,order,sizeof officer_slot_path68);
    ++officer_path_requests68;
    RaiseException(slot_fixture_end68,0,0,nullptr);
}
static void __fastcall finish_slot_probe68(void*,void*){RaiseException(slot_fixture_end68,0,0,nullptr);}
static unsigned __fastcall slot_land68(void*,void*){return 0;}
static bool native_slot_probe68(uintptr_t actor){
    using NativeActorUpdate=void(__thiscall*)(void*,unsigned);
    __try{
        reinterpret_cast<NativeActorUpdate>(base+0x3dea90)(reinterpret_cast<void*>(actor),0);
    }__except(GetExceptionCode()==slot_fixture_end68?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH){
        return true;
    }
    return false;
}
static unsigned officer_task_releases;
static void __fastcall officer_task_release(void*,void*,unsigned destroy){
    require(destroy==1,"Officer task reset did not use native destruction mode");
    ++officer_task_releases;
}

static uintptr_t __fastcall morale_unit_id67(void* subobject, void*) {
    return field<unsigned>(reinterpret_cast<uintptr_t>(subobject) - 0x10, 4);
}
static bool __fastcall morale_ability67(void*, void*, unsigned effect) { return effect == 0x2b; }
static float __fastcall morale_radius67(void*, void*, unsigned effect) {
    require(effect == 0x2b, "Native morale fixture called the wrong ability");
    return 40.f;
}
static uintptr_t __fastcall reform_fixture_unit70(void* reference,void*){
    return reinterpret_cast<uintptr_t>(reference)-0xc;
}

static uintptr_t morale_node67[5];
static unsigned morale_allocations67;
static void* __cdecl morale_allocate67(unsigned count, unsigned options) {
    require(count == 1 && !options, "Unexpected native morale effect allocation");
    ++morale_allocations67;
    return morale_node67;
}

static void verify_general_morale67(NativeFixture100& f, uintptr_t physical) {
    using P = NativeFixture100;
    std::vector<unsigned char> receiver(0x1800), shape(0x700), side(0x28), army(0x11c), general(0x250), receiver_army(0x100);
    auto r = P::ptr(receiver), rs = P::ptr(shape);
    auto ga = P::ptr(general), pa = field<uintptr_t>(unit, 0xb9c);
    uintptr_t army_list[] = {P::ptr(army)}, own_units[] = {unit, r};
    uintptr_t ids[] = {reinterpret_cast<uintptr_t>(&morale_unit_id67)};
    uintptr_t ability[] = {0, reinterpret_cast<uintptr_t>(&morale_ability67), 0,
                           reinterpret_cast<uintptr_t>(&morale_radius67)};
    uintptr_t context[0x3c / sizeof(uintptr_t)]{}, sentinel[2]{};
    context[0] = environment;
    context[1] = r;
    const auto saved_actor_table = field<uintptr_t>(possessed, 0);
    const auto saved_id = field<uintptr_t>(unit, 0x10);
    const auto saved_general = field<uintptr_t>(unit, 0x2a4);
    const auto saved_abilities = field<uintptr_t>(unit, 0xed4);
    const auto saved_active = field<unsigned>(unit, 0xb60);
    const auto saved_men = field<unsigned>(unit, 0x384);
    const auto saved_flag = field<unsigned char>(unit, 0x158);
    const auto saved_definition = field<uintptr_t>(unit, 0x1c);
    unsigned char definition[0x200]{};
    field<unsigned>(reinterpret_cast<uintptr_t>(definition), 0x1b0) = 8;
    field<uintptr_t>(unit, 0x1c) = reinterpret_cast<uintptr_t>(definition);
    field<uintptr_t>(unit, 0x10) = field<uintptr_t>(r, 0x10) = reinterpret_cast<uintptr_t>(ids);
    field<uintptr_t>(unit, 0x2a4) = ga;
    field<uintptr_t>(ga, 0x24c) = unit;
    field<unsigned char>(unit, 0x158) = 1;
    field<uintptr_t>(unit, 0xed4) = reinterpret_cast<uintptr_t>(ability);
    field<unsigned>(unit, 0xb60) = 1;
    field<unsigned>(unit, 0x384) = 2;
    field<unsigned>(r, 4) = 19;
    field<uintptr_t>(r, 0x14) = environment;
    field<uintptr_t>(r, 0xb9c) = rs;
    field<uintptr_t>(r, 0x2a4) = P::ptr(receiver_army);
    field<uintptr_t>(P::ptr(receiver_army), 0xfc) = P::ptr(side);
    field<unsigned>(P::ptr(side), 0x20) = 1;
    field<uintptr_t>(P::ptr(side), 0x24) = reinterpret_cast<uintptr_t>(army_list);
    field<unsigned>(P::ptr(army), 0x114) = 2;
    field<uintptr_t>(P::ptr(army), 0x118) = reinterpret_cast<uintptr_t>(own_units);
    field<uintptr_t>(possessed, 0) = base + 0x15dbd68;
    const float saved_x = field<float>(physical, 0x48), saved_z = field<float>(physical, 0x50);
    unsigned char saved_shape[0xcc]{};
    memcpy(saved_shape, reinterpret_cast<void*>(pa + 0x628), sizeof saved_shape);
    const auto allocator = reinterpret_cast<unsigned char*>(base + 0x9ea30);
    unsigned char saved_allocator[5]{};
    memcpy(saved_allocator, allocator, sizeof saved_allocator);
    DWORD old{};
    require(VirtualProtect(allocator, sizeof saved_allocator, PAGE_EXECUTE_READWRITE, &old) != 0,
        "Native morale effect allocator fixture protection");
    f.jump(0x9ea30, reinterpret_cast<void*>(&morale_allocate67));
    FlushInstructionCache(GetCurrentProcess(), allocator, sizeof saved_allocator);

    auto geometry = [](uintptr_t formation, float x, float z) {
        field<float>(formation, 0x644) = field<float>(formation, 0x6dc) = x;
        field<float>(formation, 0x648) = field<float>(formation, 0x6e4) = z;
        field<float>(formation, 0x650) = field<float>(formation, 0x654) = 6.f;
        field<float>(formation, 0x658) = field<float>(formation, 0x664) = 1.f;
        field<float>(formation, 0x65c) = field<float>(formation, 0x660) = 0.f;
        field<float>(formation, 0x670) = 5.f;
        field<unsigned>(formation, 0x6ec) = 0;
    };
    auto phase = P::ptr(f.phase);
    float distances[20 * 20];
    std::fill_n(distances, std::size(distances), 1000.f);
    const auto cache = phase + 0x1b0;
    const auto saved_cache_ptr = field<uintptr_t>(cache, 4);
    const auto saved_cache_stride = field<unsigned>(cache, 8);
    field<uintptr_t>(cache, 4) = reinterpret_cast<uintptr_t>(distances);
    field<unsigned>(cache, 8) = 20;
    auto native = reinterpret_cast<GeneralMoraleScan67>(base + 0x1587d0);
    auto effect = [&](GeneralMoraleScan67 scan, unsigned expected_kind, unsigned min_bonus) {
        const auto stop = reinterpret_cast<uintptr_t>(sentinel);
        context[0x34 / 4] = context[0x38 / 4] = stop;
        sentinel[0] = sentinel[1] = 0;
        morale_allocations67 = 0;
        state->heartbeat = GetTickCount64();
        scan(context);
        if (!min_bonus) {
            require(!morale_allocations67 && context[0x34 / 4] == stop,
                "An out-of-range or ineligible unit received the general morale effect");
            return 0u;
        }
        require(morale_allocations67 == 1 && context[0x34 / 4] == reinterpret_cast<uintptr_t>(morale_node67) &&
            morale_node67[2] == expected_kind && morale_node67[4] >= min_bonus,
            "Original native scan did not write the expected general morale effect");
        return static_cast<unsigned>(morale_node67[4]);
    };

    geometry(pa, 1000.f, 1000.f);
    geometry(rs, 15.f, 20.f);
    field<float>(physical, 0x48) = 15.f;
    field<float>(physical, 0x50) = 20.f;
    verify_general_ring67(physical);
    effect(original_general_morale67, 0, 0);
    const unsigned inside = effect(native, 0xf, 8);
    field<float>(physical, 0x48) = 15.f;
    field<float>(physical, 0x50) = 64.f;
    require(effect(native, 0xf, 1) < inside, "Native general bonus did not fall with distance");
    field<float>(physical, 0x50) = 20.f;
    field<unsigned char>(ga, 0x1ec) = 1;
    effect(native, 2, 8);
    field<unsigned char>(ga, 0x1ec) = 0;
    field<unsigned>(unit, 0xb60) = 2;
    effect(native, 0, 0);
    field<unsigned>(unit, 0xb60) = 1;
    field<unsigned>(P::ptr(army), 0x114) = 0;
    effect(native, 0, 0);
    field<unsigned>(P::ptr(army), 0x114) = 2;

    geometry(pa, 15.f, 20.f);
    distances[19 * 20 + 7] = 0.f;
    field<float>(physical, 0x48) = 151.f;
    effect(original_general_morale67, 0xf, 8);
    effect(native, 0, 0);
    require(distances[19 * 20 + 7] == 0.f && field<float>(pa, 0x644) == 15.f &&
        !general_morale67.source, "General morale rewrote formation geometry or the shared cache");
    InterlockedExchange(&state->release, 1);
    effect(native, 0xf, 8);
    InterlockedExchange(&state->release, 0);

    memcpy(allocator, saved_allocator, sizeof saved_allocator);
    require(VirtualProtect(allocator, sizeof saved_allocator, old, &old) != 0, "Restore native morale allocation protection");
    FlushInstructionCache(GetCurrentProcess(), allocator, sizeof saved_allocator);
    memcpy(reinterpret_cast<void*>(pa + 0x628), saved_shape, sizeof saved_shape);
    field<float>(physical, 0x48) = saved_x;
    field<float>(physical, 0x50) = saved_z;
    field<uintptr_t>(possessed, 0) = saved_actor_table;
    field<uintptr_t>(cache, 4) = saved_cache_ptr;
    field<unsigned>(cache, 8) = saved_cache_stride;
    field<uintptr_t>(unit, 0x10) = saved_id;
    field<uintptr_t>(unit, 0x2a4) = saved_general;
    field<uintptr_t>(unit, 0xed4) = saved_abilities;
    field<unsigned>(unit, 0xb60) = saved_active;
    field<unsigned>(unit, 0x384) = saved_men;
    field<unsigned char>(unit, 0x158) = saved_flag;
    field<uintptr_t>(unit, 0x1c) = saved_definition;
    std::cout << "PASS combat67: original native general morale effect follows the actor and preserves the native falloff, Rally state and cache. Mounted=" << (physical != possessed) << '\n';
}

static uintptr_t __fastcall fixture_role_definition(void*,void*){return general_role_definition;}
static unsigned __fastcall fixture_general_rank(void*,void*){return 1;}
static void verify_general_projectile_guard(uintptr_t peer, bool detached) {
    unsigned char damage_group[0x70]{}, definition[0x200]{}, unit_data[0x200]{};
    uintptr_t role_table[0x1e4/4]{},members[]{possessed};
    const auto group=reinterpret_cast<uintptr_t>(damage_group);
    const auto table=base+0x15dbd68;
    const auto saved_actor_table=field<uintptr_t>(possessed,0);
    const auto saved_role_query=field<uintptr_t>(table,0xbc);
    const auto saved_unit_data=field<uintptr_t>(unit,0x1c);
    const auto saved_start=field<unsigned>(unit,0xcbc);
    const auto saved_remaining=field<unsigned>(unit,0x2bc);
    DWORD previous{}, ignored{};
    void* query=reinterpret_cast<void*>(table+0xbc);
    require(VirtualProtect(query,4,PAGE_READWRITE,&previous)!=0,"General rank fixture protection");
    field<uintptr_t>(table,0xbc)=reinterpret_cast<uintptr_t>(&fixture_role_definition);
    general_role_definition=reinterpret_cast<uintptr_t>(definition);
    field<uintptr_t>(general_role_definition,0)=reinterpret_cast<uintptr_t>(role_table);
    role_table[0x1e0/4]=reinterpret_cast<uintptr_t>(&fixture_general_rank);
    field<uintptr_t>(possessed,0)=table;
    field<uintptr_t>(unit,0x1c)=reinterpret_cast<uintptr_t>(unit_data);
    field<unsigned char>(reinterpret_cast<uintptr_t>(unit_data),0x1c2)=1;
    field<unsigned>(unit,0xcbc)=20;
    field<unsigned>(unit,0x2bc)=18;
    field<uintptr_t>(group,4)=possessed;
    field<unsigned>(group,0x68)=1;
    field<uintptr_t>(group,0x6c)=reinterpret_cast<uintptr_t>(members);
    state->ready=1;state->heartbeat=GetTickCount64();
    PossessionIdentity proof;
    require(friendly_owned109(proof)&&proximity_independent200(possessed)==detached,
        "General fixture does not prove the intended lease and formation");
    verify_general_ring67(possessed);
    using NativeDamage=bool(__thiscall*)(void*,unsigned,unsigned,unsigned);
    auto damage=reinterpret_cast<NativeDamage>(base+0x3a6140);
    auto hit=[&](uintptr_t victim,unsigned amount){
        projectile_impact_victim=victim;
        const bool lethal=damage(damage_group,amount,1,0);
        projectile_impact_victim=0;
        return lethal;
    };
    field<unsigned>(possessed,0x680)=5;
    require(!hit(possessed,1)&&field<unsigned>(possessed,0x680)==4,"General lost ordinary native health");
    for(auto victim:{uintptr_t(0),peer}){
        require(!hit(victim,10)&&field<unsigned>(possessed,0x680)==1,
            "Unrelated damage lost native bodyguard protection");
    }
    require(hit(possessed,10)==detached&&field<unsigned>(possessed,0x680)==(detached?0u:1u),
        "Lethal projectile did not respect detached general scope");
    require(reinterpret_cast<NativeLeaderRole>(base+0x3d8b00)(reinterpret_cast<void*>(possessed)),
        "Ordinary leader identity query was altered");
    field<uintptr_t>(possessed,0)=saved_actor_table;
    field<uintptr_t>(table,0xbc)=saved_role_query;
    require(VirtualProtect(query,4,previous,&ignored)!=0,"Restore general rank fixture protection");
    field<uintptr_t>(unit,0x1c)=saved_unit_data;
    field<unsigned>(unit,0xcbc)=saved_start;
    field<unsigned>(unit,0x2bc)=saved_remaining;
    field<unsigned>(possessed,0x680)=0;
}
#include "../../../../tools/deck/verify_acquisition207.h"
#include "../../../../tools/deck/verify_acquisition65.h"
static void charge_assignment147(NativeFixture100& f,uintptr_t formation,uintptr_t physical,uintptr_t peer,bool mounted){
 using P=NativeFixture100;
 std::vector<unsigned char> enemy(0x700),other(0x700),enemy_unit(0x1800),enemy_squad(0x2800),controller(0x40),parent(0xa0),peer_parent(0xa0);
 auto eu=P::ptr(enemy_unit),es=P::ptr(enemy_squad),one=P::ptr(enemy),two=P::ptr(other),ctl=P::ptr(controller);uintptr_t targets[]={one,two};
 for(unsigned i=0;i<2;++i){auto a=i?two:one;memcpy(reinterpret_cast<void*>(a),f.peer.data(),f.peer.size());field<uintptr_t>(a,0x214)=eu;field<unsigned>(a,0x1c8)=88+i;field<float>(a,0x48)=float(i*4);field<float>(a,0x50)=10;}
 field<unsigned>(eu,4)=99;field<uintptr_t>(eu,0x14)=environment;field<uintptr_t>(eu,0xb9c)=es;field<unsigned>(es,0x18)=2;field<uintptr_t>(es,0x1c)=reinterpret_cast<uintptr_t>(targets);
 for(auto s:{formation,es}){field<float>(s,0x6dc)=0;field<float>(s,0x6e4)=s==es?10.f:0.f;field<unsigned>(s,0x6ec)=0;}
 field<unsigned char>(P::ptr(parent),0x9c)=field<unsigned char>(P::ptr(peer_parent),0x9c)=1;
 for(unsigned i=0;i<2;++i){auto a=i?peer:physical;field<uintptr_t>(a,0x24c)=a;field<uintptr_t>(a,0x2ec)=environment;field<uintptr_t>(a,0x2f0)=a;field<uintptr_t>(a,0x2f4)=i?P::ptr(peer_parent):P::ptr(parent);field<float>(a,0x48)=float(i*4);field<float>(a,0x50)=0;field<unsigned char>(a,0x384)=0;}
 field<uintptr_t>(ctl,4)=formation;field<uintptr_t>(ctl,0x14)=environment;field<uintptr_t>(ctl,0x18)=eu;
 state->heartbeat=GetTickCount64();state->input=state->aiming=state->aim_pose=state->weapon_phase=0;equipment_ranged81=0;equipment_slot81=equipment81::Melee;
 proximity_find147(physical)->checked=0;sync_members130(unit);require(field<uintptr_t>(physical,0x3b4)==formation,"Charge fixture not merged near formation");
 field<unsigned>(P::ptr(f.common),0x50)=100;if(!charge_shared146)charge_mapping146();charge_reset139();
 reinterpret_cast<UnitTick130>(base+0x184160)(controller.data());
 require(!charge_input_locked146()&&!charge_formation147(physical,true)&&charge_formation147(peer,true),"Unit charge took control of player or lost native peer assignment");
 require(field<uintptr_t>(peer+0x2f8,0x20)==two,"Original174ec0 did not independently select the peer target");
 field<unsigned>(P::ptr(f.common),0x50)=112;
 reinterpret_cast<UnitTick130>(base+0x3a9dc0)(reinterpret_cast<void*>(peer+0x2f8));
 require(!charge_native_task146(physical)&&charge_native_task146(peer)&&field<uintptr_t>(peer+0x250,0x1c)==two,"Player inherited unit charge or native peer dispatch failed");
 if(mounted)require(field<uintptr_t>(P::ptr(f.peer),0x250)==base+0x15e1b34,"Original459ea0 lost peer rider propagation");

 // Real scheduled/personal tasks now have real destructors. Retire them before
 // the explicit parent-context providers above go out of fixture scope.
 unsigned idle[]={0,0};
 for(auto a:{physical,peer}){reinterpret_cast<UnitTick130>(base+0x3ca8f0)(reinterpret_cast<void*>(a+0x2f0));reinterpret_cast<ChargeOrder139>(base+0x3d8d80)(reinterpret_cast<void*>(a),idle);}
 if(mounted)for(auto a:{possessed,P::ptr(f.peer)})reinterpret_cast<ChargeOrder139>(base+0x3d8d80)(reinterpret_cast<void*>(a),idle);
}
int main(){SetUnhandledExceptionFilter(&fault147);try{
 NativeFixture100 f;using P=NativeFixture100;
 std::vector<unsigned char> pool(0x2800),width(0x40),terrain_definition(0x100);uintptr_t registered[8]{},definition_vt[1]{reinterpret_cast<uintptr_t>(&definition147)},ability_vt[2]{0,reinterpret_cast<uintptr_t>(&ability_probe147)};
 auto p=P::ptr(pool);field<uintptr_t>(P::ptr(f.phase),0x2c)=p;field<uintptr_t>(unit,0x10)=reinterpret_cast<uintptr_t>(definition_vt);
 field<uintptr_t>(unit,0x14)=environment;field<unsigned>(unit,0x384)=2;field<uintptr_t>(unit,0xed4)=reinterpret_cast<uintptr_t>(ability_vt);field<uintptr_t>(unit,0x440)=P::ptr(width);
 field<unsigned>(unit,0xb88)=8;field<uintptr_t>(unit,0xb90)=reinterpret_cast<uintptr_t>(registered);field<uintptr_t>(unit,0xb98)=environment;
 field<uintptr_t>(P::ptr(f.point_manager),4)=P::ptr(terrain_definition);
 DWORD old{};require(VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old)!=0,"Native primitive provider protection");
 f.jump(0x8c2b0,reinterpret_cast<void*>(&words147));f.jump(0x8c2d0,reinterpret_cast<void*>(&words147));f.jump(0xf04d0,reinterpret_cast<void*>(&free_words147));
 f.jump(0x3f6290,reinterpret_cast<void*>(&no_terrain_cell147)); // No regional policy override; native default remains.
 VirtualProtect(f.image,0x1580000,old,&old);FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
 using Factory=uintptr_t(__thiscall*)(void*,unsigned);
 auto created=reinterpret_cast<Factory>(base+0x166d60)(reinterpret_cast<void*>(unit),0);
 require(created==p&&squad_registered130(p,unit,environment)&&field<unsigned>(unit,0xb8c)==1&&registered[0]==p,"Original native subgroup factory/registration failed");
 auto add=reinterpret_cast<MemberInsert130>(base+0x14bc50);add(pool.data(),possessed,0);
 require(field<uintptr_t>(possessed,0x3b4)==p&&squad_matches130(p,possessed)==1,"Original subgroup did not own physical soldier");
 std::vector<unsigned char> target_context(0x40);field<uintptr_t>(P::ptr(target_context),0x24)=unit;field<uintptr_t>(P::ptr(target_context),0x34)=possessed;
 require(reinterpret_cast<NativePose130>(base+0x1679c0)(target_context.data())==p,"Original actor-target resolver ignored registered subgroup");
 reinterpret_cast<MemberRemove130>(base+0x1a80a0)(pool.data(),possessed);
 using Retire=bool(__thiscall*)(void*,uintptr_t);
 require(reinterpret_cast<Retire>(base+0x169130)(reinterpret_cast<void*>(unit+0xb80),p)&&!field<unsigned>(unit,0xb8c)&&field<uintptr_t>(P::ptr(f.phase),0x2c)==p,"Original subgroup retirement did not return native pool object");
 require(blocks147.empty(),"Native subgroup lifecycle leaked fixture allocations");
 // Build the same native regular/independent registries as a real unit. The
 // allocator/free list and engine ID/ability methods are explicit providers.
 auto formation=reinterpret_cast<Factory>(base+0x166d60)(reinterpret_cast<void*>(unit),0);field<uintptr_t>(unit,0xb9c)=formation;
 std::vector<unsigned char> independent_pool(0x2800),fire_state(0x170);auto ip=P::ptr(independent_pool);field<uintptr_t>(P::ptr(f.phase),0x2c)=ip;
 uintptr_t independent_registered[8]{},physical[]={possessed,P::ptr(f.peer)};
 field<unsigned>(unit,0xb6c)=8;field<uintptr_t>(unit,0xb74)=reinterpret_cast<uintptr_t>(independent_registered);field<uintptr_t>(unit,0xb7c)=environment;
 field<uintptr_t>(unit,0x388)=reinterpret_cast<uintptr_t>(physical);field<uintptr_t>(possessed,0x24c)=possessed;field<uintptr_t>(possessed,0x2ec)=environment;field<float>(P::ptr(f.peer),0x48)=2;
 add(reinterpret_cast<void*>(formation),possessed,0);add(reinterpret_cast<void*>(formation),P::ptr(f.peer),0);
 field<unsigned char>(P::ptr(f.peer),0x11f)=1;field<unsigned char>(possessed,0x11f)=0;field<unsigned char>(possessed,0x384)=1;field<unsigned char>(possessed,0x308)=0;
 field<uintptr_t>(P::ptr(fire_state),4)=formation;
 require(MH_Initialize()==MH_OK,"Proximity hook init");create_unit_detach130();create_ranged_position128();create_formation_arrival129();create_friendly_hook109();create_general_influence67();require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Proximity hooks enable");
 state->heartbeat=GetTickCount64();sync_members130(unit);
 require(proximity_count147==1&&proximity_managed147(possessed)&&!detached_count130&&field<uintptr_t>(possessed,0x3b4)==formation,"Nearby soldier did not remain in native formation");
 verify_general_projectile_guard(P::ptr(f.peer),false);
 auto arrival=reinterpret_cast<FormationArrival129>(base+0x14ef80),moving=reinterpret_cast<FormationArrival129>(base+0x1aef50);
 require(arrival(reinterpret_cast<void*>(formation))&&!moving(fire_state.data()),"Nearby manual player blocked native formation arrival/fire");
 // A peer's genuine wait must still block. No blanket fire-at-will waiver.
 field<unsigned char>(P::ptr(f.peer),0x384)=1;field<unsigned char>(P::ptr(f.peer),0x11f)=0;
 require(!arrival(reinterpret_cast<void*>(formation))&&moving(fire_state.data()),"Proximity waived an AI peer's real readiness");field<unsigned char>(P::ptr(f.peer),0x384)=0;field<unsigned char>(P::ptr(f.peer),0x11f)=1;
 auto set_distance=[&](float distance){field<float>(possessed,0x48)=field<float>(P::ptr(f.peer),0x48)+distance;state->heartbeat=GetTickCount64();auto record=proximity_find147(possessed);if(record)record->checked=0;sync_members130(unit);};
 // An explicit personal wall request must detach even inside join distance.
 // The ORIGINAL kind2 factory/transfer/retirement and actor target resolver run.
 auto created_before200=proximity_created147;
 climb_align152=true;climb_entry153.actor=possessed;climb_entry153.serial=field<unsigned>(possessed,0x1c8);climb_entry153.generation=possession_lease.generation;
 for(unsigned i=0;i<20;++i){state->heartbeat=GetTickCount64();sync_members130(unit);require(proximity_independent200(possessed)&&field<uintptr_t>(P::ptr(f.peer),0x3b4)==formation,"Personal wall climb rejoined nearby unit or moved a peer");}
 require(proximity_created147==created_before200+1&&field<uintptr_t>(unit,0xb9c)==formation&&field<uintptr_t>(possessed,0x214)==unit&&physical[0]==possessed,"Personal wall climb replaced unit ownership or accumulated groups");
 climb_cancel200(possessed);set_distance(2);require(field<uintptr_t>(possessed,0x3b4)==formation&&!field<unsigned>(unit,0xb70),"Cancelled personal climb did not restore ordinary proximity");
 auto saved_config=f.config;auto original_owner=field<uintptr_t>(possessed,0x214);set_distance(20);
 require(proximity_squad147(possessed)==ip&&field<uintptr_t>(possessed,0x3b4)==ip&&squad_matches130(formation,possessed)==0&&squad_matches130(ip,possessed)==1&&proximity_created147==created_before200+2,"Far player did not enter a real independent native group");
 require(field<unsigned>(ip,8)==2&&!field<unsigned char>(ip,0x515)&&field<uintptr_t>(unit,0xb9c)==formation&&field<uintptr_t>(possessed,0x214)==original_owner&&physical[0]==possessed&&physical[1]==P::ptr(f.peer)&&f.config==saved_config,"Proximity changed native unit ownership, roster, stats or main group");
 require(reinterpret_cast<NativePose130>(base+0x1679c0)(target_context.data())==ip,"Native selected-actor targeting fell back to distant formation");
 verify_general_projectile_guard(P::ptr(f.peer),true);
 verify_general_morale67(f,possessed);
 std::cout<<"PASS combat67: general ring uses the current actor center with native draw arguments and unchanged source object.\n";
 std::cout<<"PASS combat66: original native projectile damage retains bodyguard protection in formation and consumes the detached general's final hit point.\n";
 // Original foot target enumeration includes the independent player, and
 // original single-member geometry follows the physical actor's position.
 unsigned native_targets[8]{};uintptr_t targets[4]{};native_targets[2]=4;native_targets[4]=reinterpret_cast<uintptr_t>(targets);
 using Enumerate=void(__thiscall*)(void*,void*);reinterpret_cast<Enumerate>(base+0x3c1c10)(reinterpret_cast<void*>(possessed),native_targets);
 require(native_targets[3]==1&&targets[0]==possessed,"Original individual target enumeration lost independent player");
 unsigned sample[7]{},samples[]={1,1,reinterpret_cast<uintptr_t>(sample)};using Sample=void(__thiscall*)(void*,uintptr_t);
 reinterpret_cast<Sample>(base+0x126df0)(sample,possessed);field<float>(base,0x1bd9564)=1;
 using Bounds=void(__thiscall*)(void*,const void*);reinterpret_cast<Bounds>(base+0x159d70)(reinterpret_cast<void*>(ip+0x628),samples);
 require(field<float>(ip,0x644)==field<float>(possessed,0x48)&&field<float>(ip,0x648)==field<float>(possessed,0x50),"Original independent target bounds stayed at formation position");
 #include "../../../../tools/deck/verify_acquisition203.h"
 verify_acquisition207(f,formation,ip,possessed);
 verify_ranged65(formation,ip,possessed,P::ptr(f.peer));
 set_distance(6);require(field<uintptr_t>(possessed,0x3b4)==ip,"Outside join threshold oscillated into formation");
 set_distance(3);require(field<uintptr_t>(possessed,0x3b4)==formation,"Nearby player did not rejoin automatically");
 set_distance(6);require(field<uintptr_t>(possessed,0x3b4)==formation,"Inside leave threshold oscillated out of formation");
 for(unsigned i=0;i<1000;++i){set_distance(20);require(field<uintptr_t>(possessed,0x3b4)==ip,"Repeat leave lost independent group");set_distance(2);require(field<uintptr_t>(possessed,0x3b4)==formation,"Repeat join lost formation");}
 require(proximity_created147-proximity_retired147==0&&!field<unsigned>(unit,0xb70)&&field<unsigned>(unit,0xb8c)==1&&blocks147.size()<8,"Repeated transitions accumulated native groups or allocations");
 // Clear only the earlier readiness-predicate provider's synthetic task flag
 // before constructing and executing real native charge tasks.
 field<unsigned char>(possessed,0x384)=0;charge_assignment147(f,formation,possessed,P::ptr(f.peer),false);
 set_distance(20);auto transfer=reinterpret_cast<MemberInsert130>(base+0x1b33f0);transfer(reinterpret_cast<void*>(formation),possessed,1);
 require(field<uintptr_t>(possessed,0x3b4)==formation&&!field<unsigned>(ip,0x18),"Native regroup could not drain the independent group");
 proximity_find147(possessed)->checked=0;sync_members130(unit);
 require(field<uintptr_t>(possessed,0x3b4)==ip&&squad_matches130(ip,possessed)==1&&proximity_created147-proximity_retired147==1,"Native regroup lost player policy or accumulated an empty independent group");
 state->mode=IDLE;sync_members130(unit);
 require(!proximity_count147&&!detached_count130&&!field<unsigned>(unit,0xb70)&&field<uintptr_t>(possessed,0x3b4)==formation&&proximity_created147==proximity_retired147,"Exit did not restore membership and retire independent native group");
 // Run the actual native reload scheduler over two full cylinders.
 for(const auto* key:{L"boshin_revolver",L"boshin_officer_revolver"}){
     alignas(4) unsigned char record[0xa4]{},shooter[0x398]{};
     auto r=reinterpret_cast<uintptr_t>(record),a=reinterpret_cast<uintptr_t>(shooter);
     field<unsigned>(r,4)=field<unsigned>(r,8)=unsigned(wcslen(key));
     field<uintptr_t>(r,0xc)=reinterpret_cast<uintptr_t>(key);
     field<unsigned>(r,0xa0)=1;
     for(unsigned cylinder=0;cylinder<2;++cylinder){
         for(unsigned shot=1;shot<=6;++shot){
             schedule_personal_reload65(reinterpret_cast<NativeReloadSchedule65>(base+0x3ea970),a,r,2.f);
             require(field<unsigned>(a,0x394)==6-shot&&reload_after_recoil65(a,r)==(shot==6)&&
                     field<float>(a,0x390)==(shot==6?5.f:0.f),"Revolver cylinder or extended reload differs");
         }
         field<float>(a,0x390)=0.f;
     }
 }
 std::cout<<"PASS combat65: native two-cylinder revolver reload schedule\n";
 // A soldier still in the main group needs native formation state reset as
 // much as one that changed groups. Use the actual clear=1 transfer routine.
 field<float>(possessed,0x48)=field<float>(P::ptr(f.peer),0x48)+2.f;
 state->mode=POSSESSED;state->heartbeat=GetTickCount64();sync_members130(unit);
 require(proximity_managed147(possessed)&&field<uintptr_t>(possessed,0x3b4)==formation,"Near-return setup");
 field<unsigned>(possessed,0x674)=0;field<unsigned char>(possessed,0x385)=1;
 state->mode=IDLE;sync_members130(unit);
 require(!proximity_count147&&squad_matches130(formation,possessed)==1&&field<unsigned>(possessed,0x674)==2&&
         !field<unsigned char>(possessed,0x385),"Near release failed to reset native formation state");
 std::cout<<"PASS combat65: already-joined release resets native formation state\n";
 // Native transfer must clear a lingering personal task for the officer
 // both when he is already in the unit and when he returns from his own group.
 const auto normal_table=field<uintptr_t>(possessed,0);
 field<uintptr_t>(possessed,0)=base+0x15dbfb4;
 require(followed(possessed,true),"Officer fixture failed native possession validation");
 state->lease_generation=possession_lease.generation;
 const uintptr_t officer_task_vtable[]{reinterpret_cast<uintptr_t>(&officer_task_release)};
 // The native actor update executes its own branch, slot calculation and path
 // record constructor. Observe the request at its external path provider.
 const unsigned providers[]{0x381970,0x3efc70,0x3ea960};
 void* handlers[]{reinterpret_cast<void*>(&observe_slot_request68),
     reinterpret_cast<void*>(&finish_slot_probe68),reinterpret_cast<void*>(&slot_land68)};
 unsigned char provider_bytes[3][5]{};
 DWORD code_protection{};
 require(VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&code_protection)!=0,"Protect officer path fixture");
 for(unsigned i=0;i<3;++i){
     memcpy(provider_bytes[i],reinterpret_cast<void*>(base+providers[i]),5);
     f.jump(providers[i],handlers[i]);
 }
 FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
 for(bool separated:{false,true}){
     state->mode=POSSESSED;state->heartbeat=GetTickCount64();sync_members130(unit);
     set_distance(separated?20:2);
     require(field<uintptr_t>(possessed,0x3b4)==(separated?ip:formation),"Officer setup used the wrong group");
     const float slot_x=field<float>(P::ptr(f.peer),0x48),slot_z=field<float>(P::ptr(f.peer),0x50)+1.f;
     field<float>(formation,0x644)=slot_x;field<float>(formation,0x648)=slot_z;
     field<float>(formation,0x670)=5.f;
     field<float>(possessed,0x648)=slot_x;field<float>(possessed,0x64c)=0.f;field<float>(possessed,0x650)=slot_z;
     field<unsigned>(possessed,0x658)=0;
     field<unsigned char>(possessed,0x664)=field<unsigned char>(possessed,0x665)=1;
     const unsigned calls_before=officer_path_requests68;
     require(native_slot_probe68(possessed)&&officer_path_requests68==calls_before,
         "Stock consumed slot unexpectedly requested a path");
     field<uintptr_t>(possessed,0x2f8)=reinterpret_cast<uintptr_t>(officer_task_vtable);
     field<unsigned char>(possessed,0x384)=field<unsigned char>(possessed,0x385)=1;
     field<unsigned>(possessed,0x674)=0;
     state->mode=IDLE;sync_members130(unit);
     require(!proximity_count147&&squad_matches130(formation,possessed)==1&&field<unsigned>(possessed,0x674)==2&&
             !field<unsigned char>(possessed,0x384)&&!field<unsigned char>(possessed,0x385),
             "Officer release left a stale formation or task");
     require(!field<unsigned char>(possessed,0x664)&&!field<unsigned char>(possessed,0x665)&&
         native_slot_probe68(possessed)&&officer_path_requests68==calls_before+1&&
         officer_slot_path68[0]==slot_x&&officer_slot_path68[2]==slot_z,
         "Original officer update did not request a route to its assigned slot");
 }
 for(unsigned i=0;i<3;++i)memcpy(reinterpret_cast<void*>(base+providers[i]),provider_bytes[i],5);
 require(VirtualProtect(f.image,0x1580000,code_protection,&code_protection)!=0,"Restore officer path fixture protection");
 FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
 require(officer_task_releases==2,"Officer's old task was not released exactly once per exit");
 field<uintptr_t>(possessed,0)=normal_table;
 require(followed(possessed,true),"Regular soldier fixture restoration");
 state->lease_generation=possession_lease.generation;
 std::cout<<"PASS combat68: original actor update requests its native officer slot after both near and separated release.\n";
 // Real 4668f0, 187000, 19fac0, and the native move order constructor run.
 // Only the unit-reference resolver is supplied by this fixture. There is
 // no UI/event-bus provider, so a player-facing move would fail this test.
 const auto pose=P::ptr(width),common=P::ptr(f.common);
 uintptr_t move_reference[11]{};
 move_reference[10]=reinterpret_cast<uintptr_t>(&reform_fixture_unit70);
 field<uintptr_t>(unit,0xc)=reinterpret_cast<uintptr_t>(move_reference);
 field<uintptr_t>(unit,0x444)=unit;
 const ReformPoint70 saved_position{{-21.6743164f,126.855316f,-449.9990234f},0,0};
 memcpy(reinterpret_cast<void*>(pose+0x10),&saved_position,sizeof saved_position);
 field<float>(pose,0x24)=27.625f;
 field<unsigned short>(pose,0x28)=0x7f90;
 auto saved_pose=width;
 for(unsigned scenario=0;scenario<4;++scenario){
     state->mode=POSSESSED;state->release=0;
     require(followed(possessed,true),"Acquire fresh reform test identity");
     state->lease_generation=possession_lease.generation;
     set_distance(scenario==1?20.f:2.f);
     if(scenario==2)field<float>(possessed,0x48)=field<float>(P::ptr(f.peer),0x48)+20.f;
     field<unsigned>(unit,0x9ec)=scenario==3?1u:0u;
     field<unsigned>(unit,0x9f0)=field<unsigned>(unit,0x9f4)=0;
     const auto counter=field<unsigned>(common,0x54);
     if(!scenario){
         state->release=1;sync_members130(unit);
         require(proximity_find147(possessed)&&!field<unsigned>(unit,0x9ec),
                 "Reform ran before possession relinquished its lease");
     }
     state->release=0;state->mode=IDLE;possession_lease.release();sync_members130(unit);
     require(!proximity_count147&&squad_matches130(formation,possessed)==1&&width==saved_pose,
             "Release changed the native pose or failed to restore formation membership");
     if(!scenario){
         const auto order=unit+0x454;
         require(field<unsigned>(unit,0x9ec)==1&&field<unsigned>(common,0x54)==counter+1&&
             field<uintptr_t>(order,0)==base+0x15bb460&&
             !memcmp(reinterpret_cast<const void*>(order+0x24),&saved_position,sizeof saved_position)&&
             field<unsigned short>(order,0x4e)==0x7f90&&field<float>(order,0x54)==27.625f&&
             field<unsigned char>(order,0x58)==1&&!field<unsigned char>(order,0x59)&&
             field<unsigned char>(order,0x5a)==1,
             "Reform failed to enqueue one native walk with the exact current formation");
         sync_members130(unit);
         require(field<unsigned>(unit,0x9ec)==1&&field<unsigned>(common,0x54)==counter+1,
                 "A completed release queued another reform");
     }else{
         require(field<unsigned>(unit,0x9ec)==(scenario==3?1u:0u)&&field<unsigned>(common,0x54)==counter,
                 "Reform issued for a distant soldier, stale proximity or an occupied queue");
     }
 }
 field<unsigned>(unit,0x9ec)=field<unsigned>(unit,0x9f0)=field<unsigned>(unit,0x9f4)=0;
 field<uintptr_t>(unit,0xc)=field<uintptr_t>(unit,0x444)=0;
 state->mode=POSSESSED;require(followed(possessed,true),"Restore pre-reform fixture lease");
 state->lease_generation=possession_lease.generation;
 std::cout<<"PASS reform70: native silent same-pose walk queue on near release only; pending release, distant, stale-range and busy-queue guards.\n";

 // If every other soldier dies, native main-group selection can adopt the
 // independent player group. Exit must create a normal formation before
 // retiring it, and must never leave b9c pointing at a returned pool object.
 state->mode=POSSESSED;state->heartbeat=GetTickCount64();sync_members130(unit);set_distance(20);
 auto remove=reinterpret_cast<MemberRemove130>(base+0x1a80a0);remove(reinterpret_cast<void*>(formation),P::ptr(f.peer));field<unsigned char>(P::ptr(f.peer),0xf0)=0;
 reinterpret_cast<UnitTick130>(base+0x16b180)(reinterpret_cast<void*>(unit));require(field<uintptr_t>(unit,0xb9c)==ip,"Last-soldier native main selection fixture");
 require(reinterpret_cast<Retire>(base+0x169130)(reinterpret_cast<void*>(unit+0xb80),formation),"Last-soldier old formation retirement");
 state->mode=IDLE;sync_members130(unit);formation=field<uintptr_t>(unit,0xb9c);
 require(!proximity_count147&&!field<unsigned>(unit,0xb70)&&squad_registered130(formation,unit,environment)&&formation!=ip&&field<uintptr_t>(possessed,0x3b4)==formation,"Last-soldier exit retained a retired primary group");
 field<unsigned char>(P::ptr(f.peer),0xf0)=1;add(reinterpret_cast<void*>(formation),P::ptr(f.peer),0);
 // Native mounted primary groups own the horse. The actor-target resolver
 // must follow the current physical group when given the retained human ID.
 remove(reinterpret_cast<void*>(formation),possessed);remove(reinterpret_cast<void*>(formation),P::ptr(f.peer));
 std::vector<unsigned char> horse(0x700),peer_horse(0x700),holder(0x210),peer_holder(0x210),mount_config(0x100),record(0x100);
 auto h=P::ptr(horse),ph=P::ptr(peer_horse),mc=P::ptr(mount_config);uintptr_t riders[]={possessed,P::ptr(f.peer)},holders[]={P::ptr(f.holder),P::ptr(f.peer_holder),P::ptr(holder),P::ptr(peer_holder)};
 field<uintptr_t>(unit,0xf9c)=mc;field<uintptr_t>(unit,0x2c)=P::ptr(record);
 for(unsigned i=0;i<2;++i){auto a=i?ph:h;field<uintptr_t>(a,0)=base+0x15e8524;field<uintptr_t>(a,0x214)=unit;field<uintptr_t>(a,0x20c)=mc;field<uintptr_t>(a,0x28)=field<uintptr_t>(a,0x2ec)=environment;field<uintptr_t>(a,0x24c)=a;field<unsigned char>(a,0xf0)=field<unsigned char>(a,0xa1)=1;field<unsigned>(a,0x1c8)=44+i;field<unsigned>(a,0x6cc)=1;field<uintptr_t>(a,0x6d0)=reinterpret_cast<uintptr_t>(riders+i);field<uintptr_t>(riders[i],0x210)=a;field<float>(a,0x48)=float(i*2);add(reinterpret_cast<void*>(formation),a,0);}
 field<unsigned>(possessed,0x1f0)=29;field<uintptr_t>(P::ptr(holder),8)=h;field<uintptr_t>(P::ptr(peer_holder),8)=ph;field<unsigned>(P::ptr(f.view),0x18)=4;field<uintptr_t>(P::ptr(f.view),0x1c)=reinterpret_cast<uintptr_t>(holders);
 state->mode=POSSESSED;require(followed(possessed,true),"Fresh mounted proximity lease");state->lease_generation=possession_lease.generation;state->heartbeat=GetTickCount64();sync_members130(unit);
 require(proximity_managed147(h)&&field<uintptr_t>(h,0x3b4)==formation&&!field<uintptr_t>(possessed,0x3b4),"Nearby mounted ownership duplicated rider in horse formation");
 field<float>(h,0x48)=30;proximity_find147(h)->checked=0;state->heartbeat=GetTickCount64();sync_members130(unit);
 require(field<uintptr_t>(h,0x3b4)==ip&&squad_matches130(ip,h)==1&&!field<uintptr_t>(possessed,0x3b4),"Far mounted player did not preserve native horse/rider group roles");
 require(reinterpret_cast<NativePose130>(base+0x1679c0)(target_context.data())==ip,"Selected rider target lost actual independent horse group");
 verify_general_morale67(f,h);
 verify_acquisition207(f,formation,ip,h);
 verify_ranged65(formation,ip,possessed,P::ptr(f.peer));
 field<float>(h,0x48)=1;proximity_find147(h)->checked=0;state->heartbeat=GetTickCount64();sync_members130(unit);charge_assignment147(f,formation,h,ph,true);
 state->mode=IDLE;sync_members130(unit);require(!proximity_count147&&!detached_count130&&field<uintptr_t>(h,0x3b4)==formation&&!field<unsigned>(unit,0xb70),"Mounted exit did not restore and retire native groups");
 require(!proximity_errors147&&!member_errors130&&!charge_errors139,"Proximity concealed a native exception");
 MH_DisableHook(MH_ALL_HOOKS);MH_Uninitialize();
 // Retire the test's original regular group too, through its native lifecycle.
 for(auto a:{possessed,P::ptr(f.peer),h,ph})if(field<uintptr_t>(a,0x3b4)==formation)remove(reinterpret_cast<void*>(formation),a);
 require(reinterpret_cast<Retire>(base+0x169130)(reinterpret_cast<void*>(unit+0xb80),formation)&&blocks147.empty(),"Final fixture lifecycle leaked native allocations");
 std::cout<<"PASS: original native kind2/3 group lifecycle; foot/mounted proximity joins/leaves,4m join/8m leave and1000 cycles; no own-position boundary feedback; same native unit roster/stats; native actor target resolver/enumerator and single-member bounds; nearby formation firing excludes only player; peer waits remain; exact exit/pool retirement. Enemy strategic acquisition and rendered shots/charges remain live checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
