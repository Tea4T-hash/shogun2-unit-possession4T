// Execute original native perception/filter/sort/eligibility and charge
// dispatch. Grid cells, allocator, collision scene and movement path services
// are explicit fixture providers; this is not a complete rendered battle.
#define main retained_commands148
#include "verify_native_commands100.cpp"
#undef main
#include "native_group_fixture148.h"
static unsigned cell_coordinates148[2]{};
static bool blocked_scene148;
static unsigned cell_calls148,collision_calls148;
static void __fastcall cells148(void*,void*,const float*,float,unsigned,unsigned,void* out){
    auto result=static_cast<unsigned*>(out);result[0]=result[1]=1;result[2]=reinterpret_cast<unsigned>(cell_coordinates148);++cell_calls148;
}
static bool __fastcall collision148(void*,void*,uintptr_t,uintptr_t,unsigned){++collision_calls148;return blocked_scene148;}
static void __fastcall unused_path148(void*,void*,const void*,const void*){}
static void __fastcall unused_direct148(void*,void*,const void*,float,unsigned){}
static void __fastcall unused_point148(void*,void*){}
static float __fastcall alignment_speed149(void*,void*,unsigned){return 2.f;}
static void* __fastcall contact_words148(void*,void*,unsigned bytes,unsigned){return words147((bytes+3)/4,0);}
static unsigned native_bits148[8]{};
static LONG CALLBACK first_fault148(EXCEPTION_POINTERS* e){
    if(e->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){auto c=e->ContextRecord;
        std::cerr<<"Native scene provider fault rva="<<std::hex<<c->Eip-base<<" address="<<e->ExceptionRecord->ExceptionInformation[1]<<" ecx="<<c->Ecx<<" eax="<<c->Eax<<std::dec<<"\n";}
    return EXCEPTION_CONTINUE_SEARCH;
}
int main(){SetUnhandledExceptionFilter(&fault147);try{
    AddVectoredExceptionHandler(1,&first_fault148);
    NativeFixture100 f;using P=NativeFixture100;
    std::vector<unsigned char> regular_pool(0x2800),independent_pool(0x2800),width(0x40),terrain(0x100),grid(0x40),manager(0x200),side(0x20),enemy_side(0x20),enemy_unit(0x1800),unit_record(0x100);
    uintptr_t regular[8]{},independent[8]{},grid_roots[1]{};
    uintptr_t definition_vt[]={reinterpret_cast<uintptr_t>(&definition147)},ability_vt[]={0,reinterpret_cast<uintptr_t>(&ability_probe147)};
    auto actor=possessed,peer=P::ptr(f.peer),eu=P::ptr(enemy_unit),controller=P::ptr(manager),ip=P::ptr(independent_pool);
    field<uintptr_t>(P::ptr(f.phase),0x2c)=P::ptr(regular_pool);
    field<uintptr_t>(unit,0x10)=reinterpret_cast<uintptr_t>(definition_vt);field<uintptr_t>(unit,0x14)=environment;field<uintptr_t>(unit,0x440)=P::ptr(width);
    field<uintptr_t>(unit,0xed4)=reinterpret_cast<uintptr_t>(ability_vt);
    field<unsigned>(unit,0xb88)=field<unsigned>(unit,0xb6c)=8;field<uintptr_t>(unit,0xb90)=reinterpret_cast<uintptr_t>(regular);field<uintptr_t>(unit,0xb74)=reinterpret_cast<uintptr_t>(independent);field<uintptr_t>(unit,0xb98)=field<uintptr_t>(unit,0xb7c)=environment;
    field<uintptr_t>(P::ptr(f.point_manager),4)=P::ptr(terrain);
    DWORD old{};require(VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old)!=0,"Personal charge providers");
    f.jump(0x8c2b0,reinterpret_cast<void*>(&words147));f.jump(0x8c2d0,reinterpret_cast<void*>(&words147));f.jump(0xf04d0,reinterpret_cast<void*>(&free_words147));
    f.jump(0x9e9d0,reinterpret_cast<void*>(&words147)); // ushort visited-ID allocation, conservatively sized provider.
    f.jump(0x3f6290,reinterpret_cast<void*>(&no_terrain_cell147));
    f.jump(0x3d8070,reinterpret_cast<void*>(&cells148));f.jump(0x3f5600,reinterpret_cast<void*>(&collision148));
    f.jump(0x3df290,reinterpret_cast<void*>(&unused_path148));f.jump(0x3de710,reinterpret_cast<void*>(&unused_direct148));f.jump(0x370b40,reinterpret_cast<void*>(&unused_point148));
    f.jump(0x3ddc60,reinterpret_cast<void*>(&contact_words148));
    f.jump(0x18b470,reinterpret_cast<void*>(&alignment_speed149)); // Native unit walk pace is the explicit alignment provider.
    VirtualProtect(f.image,0x1580000,old,&old);FlushInstructionCache(GetCurrentProcess(),f.image,0x1580000);
    using Factory=uintptr_t(__thiscall*)(void*,unsigned);
    auto formation=reinterpret_cast<Factory>(base+0x166d60)(reinterpret_cast<void*>(unit),0);field<uintptr_t>(unit,0xb9c)=formation;
    field<uintptr_t>(P::ptr(f.phase),0x2c)=ip;
    auto add=reinterpret_cast<MemberInsert130>(base+0x14bc50);add(reinterpret_cast<void*>(formation),actor,0);add(reinterpret_cast<void*>(formation),peer,0);
    uintptr_t own_roster[]={actor,peer};field<unsigned>(unit,0x384)=2;field<uintptr_t>(unit,0x388)=reinterpret_cast<uintptr_t>(own_roster);
    field<float>(peer,0x48)=100;field<float>(actor,0x94)=5;field<unsigned>(actor,0x34)=3;field<float>(actor,0x54)=2;field<uintptr_t>(actor,0x24c)=actor;field<uintptr_t>(actor,0x2ec)=environment;
    field<uintptr_t>(actor,0x44)=field<uintptr_t>(peer,0x44)=P::ptr(side);field<unsigned>(P::ptr(side),0x10)=0;field<unsigned>(P::ptr(enemy_side),0x10)=1;
    field<float>(P::ptr(f.config),0x2c)=7;
    field<uintptr_t>(P::ptr(f.common),0xcc)=controller;field<uintptr_t>(controller,4)=environment;field<uintptr_t>(controller,0x88)=reinterpret_cast<uintptr_t>(native_bits148);
    field<unsigned>(P::ptr(f.common),0x50)=100;
    field<float>(base,0x1a4db40+0x5c)=12;field<float>(base,0x1a4cfe0+0x5c)=2;field<float>(base,0x1a4d960+0x5c)=3;
    field<unsigned>(eu,4)=99;field<uintptr_t>(eu,0x14)=environment;field<uintptr_t>(eu,0x10)=reinterpret_cast<uintptr_t>(definition_vt);
    field<uintptr_t>(unit,0x2c)=field<uintptr_t>(eu,0x2c)=P::ptr(unit_record); // Native ordinary infantry category0.
    std::vector<unsigned char> enemy(0x700),farther(0x700),behind(0x700),dead(0x700),high(0x700),nodes(6*24);
    uintptr_t enemies[]={P::ptr(enemy),P::ptr(farther),P::ptr(behind),P::ptr(dead),P::ptr(high)},spatial[]={peer,enemies[1],enemies[2],enemies[3],enemies[4],enemies[0]};
    field<unsigned>(eu,0x384)=5;field<uintptr_t>(eu,0x388)=reinterpret_cast<uintptr_t>(enemies);
    for(unsigned i=0;i<5;++i){auto a=enemies[i];memcpy(reinterpret_cast<void*>(a),f.peer.data(),f.peer.size());field<uintptr_t>(a,0x214)=eu;field<uintptr_t>(a,0x44)=P::ptr(enemy_side);field<unsigned>(a,0x1c8)=88+i;field<unsigned>(a,0x34)=3;field<float>(a,0x54)=2;field<float>(a,0x48)=0;field<float>(a,0x50)=i==0?8.f:i==1?10.f:i==2?-4.f:2.f;}
    field<unsigned char>(enemies[3],0xf0)=0;field<float>(enemies[4],0x4c)=50;
    field<unsigned>(peer,0x34)=3;field<float>(peer,0x54)=2;
    for(unsigned i=0;i<6;++i){auto node=P::ptr(nodes)+24*i;field<unsigned>(node,0)=8;field<unsigned short>(node,4)=unsigned short(10+i);field<uintptr_t>(node,0xc)=spatial[i];field<uintptr_t>(node,0x14)=i<5?node+24:0;}
    grid_roots[0]=P::ptr(nodes);field<uintptr_t>(P::ptr(grid),0)=reinterpret_cast<uintptr_t>(grid_roots);field<uintptr_t>(P::ptr(f.common),0xa8)=P::ptr(grid);
    equipment_ranged81=0;equipment_slot81=equipment81::Melee;state->heartbeat=GetTickCount64();state->input=FORWARD|RUN;
    require(MH_Initialize()==MH_OK,"Personal charge hooks init");create_unit_detach130();require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Personal charge hooks enable");charge_mapping146();
    sync_members130(unit);require(proximity_squad147(actor)==ip&&field<uintptr_t>(actor,0x3b4)==ip,"Far actor did not have independent native participation");
    // Reverse the native perception query: an enemy with NO preassigned target
    // must enumerate the separated player although the player's formation is far away.
    auto seen_actor=field<uintptr_t>(P::ptr(nodes),0xc);auto seen_next=field<uintptr_t>(P::ptr(nodes),0x14);
    field<uintptr_t>(P::ptr(nodes),0xc)=actor;field<uintptr_t>(P::ptr(nodes),0x14)=0;
    auto foe_z=field<float>(enemies[0],0x50);auto foe_bearing=field<unsigned short>(enemies[0],0x64);
    field<float>(enemies[0],0x50)=4;field<unsigned short>(enemies[0],0x64)=32768;
    ChargeList148 observed{};
    reinterpret_cast<ChargeQuery148>(base+0x3d1fa0)(reinterpret_cast<void*>(enemies[0]),&observed,6.f,0x71c,1);
    require(observed.count==1&&field<uintptr_t>(observed.data,0)==actor,"Native enemy perception lost separated player");
    reinterpret_cast<void(__cdecl*)(void*)>(base+0xf04d0)(reinterpret_cast<void*>(observed.data));
    field<uintptr_t>(P::ptr(nodes),0xc)=seen_actor;field<uintptr_t>(P::ptr(nodes),0x14)=seen_next;
    field<float>(enemies[0],0x50)=foe_z;field<unsigned short>(enemies[0],0x64)=foe_bearing;cell_calls148=collision_calls148=0;
    charge_reset139();charge_boundary146(unit,false);require(!charge_input_locked146()&&!cell_calls148,"Entry movement carry created a charge without actual W+Shift");
    for(unsigned input:{unsigned(FORWARD|RUN),unsigned(FORWARD),unsigned(RUN),unsigned(FORWARD|RUN|BACKWARD),unsigned(FORWARD|RUN|LEFT)}){charge_physical_input148=LONG(input);charge_boundary146(unit,false);require(!charge_input_locked146()&&!cell_calls148,"Invalid physical input queried/started charge");}
    for(auto value:{charge_intent199(FORWARD|RUN,false,false,true),charge_intent199(FORWARD|RUN,true,true,true),charge_intent199(FORWARD|RUN,true,false,false)}){
        charge_physical_input148=LONG(value);charge_probe_tick148=0;charge_boundary146(unit,false);
        require(!charge_input_locked146()&&!cell_calls148,"Unowned, aim or sprint-only input requested personal charge");
    }
    charge_physical_input148=charge_intent199(FORWARD|RUN,true,false,true);
    auto before_combat_queries=charge_queries148;
    for(unsigned posture:{8u,9u,10u,11u,29u}){field<unsigned>(actor,0x1f0)=posture;charge_probe_tick148=0;charge_boundary146(unit,false);require(!charge_input_locked146()&&charge_queries148==before_combat_queries,"Native fighting/defending posture was reset by a new charge");}
    field<unsigned>(actor,0x1f0)=0;
    field<float>(enemies[0],0x50)=2;charge_probe_tick148=0;charge_boundary146(unit,false);require(!charge_input_locked146()&&charge_near_reject149,"Melee-distance opponent was skipped to charge a farther man");field<float>(enemies[0],0x50)=8;
    field<float>(base,0x1a4d960+0x5c)=9;charge_probe_tick148=0;charge_boundary146(unit,false);require(!charge_input_locked146(),"Charge separation was hard-coded instead of using live native property");field<float>(base,0x1a4d960+0x5c)=3;
    blocked_scene148=true;charge_probe_tick148=0;charge_boundary146(unit,false);
    if(!cell_calls148||!collision_calls148)std::cerr<<"Probe stage cells="<<cell_calls148<<" collision="<<collision_calls148<<" candidates="<<charge_candidates148<<" errors="<<charge_errors139<<" reject="<<charge_reject148<<" admission="<<charge_admission146(actor)<<" speed="<<field<float>(actor,0x94)<<" flags="<<field<unsigned>(actor,0x34)<<" scratch_count="<<field<unsigned>(base+0x179a1b4,0xc)<<"\n";
    require(!charge_input_locked146()&&cell_calls148&&collision_calls148,"Native collision eligibility rejection was bypassed");
    auto cached_blocks=blocks147.size();
    for(unsigned i=0;i<100;++i){charge_probe_tick148=0;charge_boundary146(unit,false);require(!charge_input_locked146(),"Repeated rejected scene created a charge");}
    require(blocks147.size()==cached_blocks,"Repeated native perception accumulated live allocations");
    state->follow=1;blocked_scene148=false;charge_probe_tick148=0;charge_boundary146(unit,false);require(state->follow==1,"Personal charge cancelled Follow Me");
    if(!charge_input_locked146()||!charge_native_task146(actor))std::cerr<<"Start stage cells="<<cell_calls148<<" candidates="<<charge_candidates148<<" eligible="<<charge_eligibility148<<" starts="<<charge_starts139<<" returns="<<charge_returns146<<" errors="<<charge_errors139<<" reject="<<charge_reject148<<" target="<<charge_control146.proof.target<<" task="<<std::hex<<field<uintptr_t>(actor,0x250)-base<<" present="<<unsigned(field<unsigned char>(actor,0x2e8))<<" done="<<unsigned(field<unsigned char>(actor,0x258))<<std::dec<<"\n";
    require(charge_input_locked146()&&charge_native_task146(actor)&&field<uintptr_t>(actor+0x250,0x1c)==enemies[0],"Native perception/eligibility did not choose nearest valid forward enemy");
    require(charge_control146.squad==ip&&field<uintptr_t>(actor,0x3b4)==ip&&field<uintptr_t>(actor+0x250,0x20)==eu&&!field<unsigned>(unit,0x9ec),"Personal charge depended on whole-unit order or rejoined distant formation");
    charge146::Value value{};require(charge146::read(charge_shared146,value)&&charge146::matches(value,GetCurrentProcessId(),state->lease_generation,state->actor_id,state->unit_id,field<unsigned>(actor,0x1c8),0,GetTickCount64()),"Personal charge missing current QTE observation");
    auto before_actor=f.actor;auto intents=state->native_intent_calls;state->heartbeat=GetTickCount64();require(!prepare(actor)&&f.actor==before_actor&&state->native_intent_calls==intents,"Manual integrator overwrote native personal charge");
    // Actual original contact queue and12-byte constructor, only allocation is
    // supplied. A failed queue is not accepted as a native charge handoff.
    reinterpret_cast<ChargeContact148>(base+0x3c0bd0)(reinterpret_cast<void*>(controller),actor,enemies[0]);
    require(charge_pose_wait149==1&&!field<unsigned>(controller,0x14),"Task presence was accepted as native charge pose at contact");
    reinterpret_cast<ChargeContact148>(base+0x3c0bd0)(reinterpret_cast<void*>(controller),enemies[0],actor);
    require(charge_contacts148==1&&field<unsigned>(controller,0x14)==1,"Charge pose wait blocked an incoming native attack/defence");
    reinterpret_cast<UnitTick130>(base+0x3e6d50)(reinterpret_cast<void*>(actor+0x250));
    field<float>(actor,0x1ac)=5;field<float>(actor,0x150)=1;
    reinterpret_cast<UnitTick130>(base+0x3dd9d0)(reinterpret_cast<void*>(actor));
    require(field<unsigned>(actor,0x244)==2&&field<unsigned>(actor,0x1d0)==16,"Original entry/motion did not classify a real charge");
    reinterpret_cast<ChargeContact148>(base+0x3c0bd0)(reinterpret_cast<void*>(controller),actor,enemies[0]);
    require(charge_contacts148==2&&charge_control146.queued&&field<unsigned>(controller,0x14)==2,"Original incoming/outgoing contact queue did not establish handoff");
    field<unsigned char>(actor,0x258)=1;charge_boundary146(unit,true);require(charge_input_locked146(),"Charge task completion lost queued native encounter to QTE");
    field<unsigned>(P::ptr(f.common),0x50)=103;charge_boundary146(unit,true);require(!charge_input_locked146(),"Processed/expired contact kept charge control forever");
    charge_probe_tick148=0;charge_boundary146(unit,false);require(charge_input_locked146(),"Fresh charge after expired queue did not start");
    // Reproduce148: its early on-rails return skipped the existing registered
    // native pair-alignment path. Targets below are original-preparation fields;
    // this fixture supplies their placement, not an animation or casualty.
    std::vector<unsigned char> pair149(0x200);auto w149=P::ptr(pair149);
    field<unsigned char>(w149,0xc)=1;field<uintptr_t>(w149,4)=environment;field<unsigned>(w149,0x38)=2;field<uintptr_t>(w149,0x30)=actor;field<uintptr_t>(w149,0x34)=enemies[0];field<unsigned>(w149,0x78)=2;field<uintptr_t>(w149,0x58)=actor;field<unsigned>(w149,0x5c)=16;
    field<uintptr_t>(actor,0x688)=field<uintptr_t>(enemies[0],0x688)=w149;field<float>(actor,0x69c)=1;field<float>(actor,0x6a4)=1;
    melee60::Request r149;r149.pid=GetCurrentProcessId();r149.lease=state->lease_generation;r149.serial=149;r149.player=state->actor_id;r149.player_unit=state->unit_id;r149.opponent=field<unsigned>(enemies[0],0x1c8);r149.opponent_unit=99;r149.phase=melee60::Aligning;r149.heartbeat=GetTickCount64();
    require(PossessionMeleeCommitment63(&r149)==63,"Native charge pair handoff rejected");
    field<float>(actor,0xd8)=field<float>(actor,0xe0)=0;state->heartbeat=GetTickCount64();
    require(!prepare(actor)&&(field<float>(actor,0xd8)>0||field<float>(actor,0xe0)>0),"On-rails charge skipped registered native pair alignment");
    charge_boundary146(unit,true);require(charge_pose_entries149>=1&&charge_pair_plan149==16,"Actual native charge classification/plan observation missing");
    field<unsigned char>(w149,0xd1)=1;field<float>(actor,0xd8)=7;state->heartbeat=GetTickCount64();require(!prepare(actor)&&field<float>(actor,0xd8)==7,"Native charge playback received an alignment override");
    melee60::Request off149;PossessionMeleeCommitment63(&off149);melee_handoff78.reset();melee_opponent_motion78.reset();field<uintptr_t>(actor,0x688)=field<uintptr_t>(enemies[0],0x688)=0;
    charge_boundary146(unit,true);require(!charge_input_locked146(),"Finished native pair kept charge control forever");
    charge_physical_input148=0;
    // Current native target death/removal does not manufacture a replacement.
    field<unsigned char>(enemies[0],0xf0)=0;field<unsigned char>(enemies[1],0xf0)=0;charge_probe_tick148=0;charge_physical_input148=charge_intent199(FORWARD|RUN,true,false,true);charge_boundary146(unit,false);require(!charge_input_locked146(),"No valid forward enemy still created a charge");
    charge_physical_input148=0;
    state->mode=IDLE;sync_members130(unit);
    require(!proximity_count147&&!detached_count130,"Foot charge exit retained membership ownership");
    auto remove=reinterpret_cast<MemberRemove130>(base+0x1a80a0);remove(reinterpret_cast<void*>(formation),actor);remove(reinterpret_cast<void*>(formation),peer);
    std::vector<unsigned char> horse(0x700),peer_horse(0x700),horse_holder(0x210),peer_horse_holder(0x210),mount_config(0x100);
    auto h=P::ptr(horse),ph=P::ptr(peer_horse),mc=P::ptr(mount_config);
    uintptr_t riders[]={actor,peer},holders[]={P::ptr(f.holder),P::ptr(f.peer_holder),P::ptr(horse_holder),P::ptr(peer_horse_holder)};
    field<uintptr_t>(unit,0xf9c)=mc;
    for(unsigned i=0;i<2;++i){auto a=i?ph:h;field<uintptr_t>(a,0)=base+0x15e8524;field<uintptr_t>(a,0x214)=unit;field<uintptr_t>(a,0x20c)=mc;field<uintptr_t>(a,0x28)=field<uintptr_t>(a,0x2ec)=environment;field<uintptr_t>(a,0x24c)=a;field<unsigned char>(a,0xf0)=field<unsigned char>(a,0xa1)=1;field<unsigned>(a,0x1c8)=44+i;field<unsigned>(a,0x6cc)=1;field<uintptr_t>(a,0x6d0)=reinterpret_cast<uintptr_t>(riders+i);field<uintptr_t>(riders[i],0x210)=a;field<uintptr_t>(a,0x44)=P::ptr(side);field<unsigned>(a,0x34)=3;field<float>(a,0x54)=2;field<float>(a,0x48)=float(i*100);add(reinterpret_cast<void*>(formation),a,0);}
    field<float>(mc,0x2c)=9;field<float>(mc,0x1c)=2;field<float>(mc,0x20)=7;field<float>(mc,0x6c)=1;field<float>(h,0x94)=7;
    field<uintptr_t>(P::ptr(horse_holder),8)=h;field<uintptr_t>(P::ptr(peer_horse_holder),8)=ph;field<unsigned>(P::ptr(f.view),0x18)=4;field<uintptr_t>(P::ptr(f.view),0x1c)=reinterpret_cast<uintptr_t>(holders);
    field<unsigned>(actor,0x1f0)=29;state->mode=POSSESSED;require(followed(actor,true),"Fresh independent mounted charge lease");state->lease_generation=possession_lease.generation;state->heartbeat=GetTickCount64();
    field<unsigned char>(enemies[0],0xf0)=field<unsigned char>(enemies[1],0xf0)=1;
    charge_reset139();charge_physical_input148=charge_intent199(FORWARD|RUN,true,false,true);charge_probe_tick148=0;charge_boundary146(unit,false);
    require(charge_input_locked146()&&charge_native_task146(h)&&field<uintptr_t>(h+0x250,0x1c)==enemies[0]&&field<uintptr_t>(actor,0x250)==base+0x15e1b34,"Independent mounted trigger lost real nearest target or original rider propagation");
    require(field<uintptr_t>(h,0x3b4)!=formation&&!field<uintptr_t>(actor,0x3b4)&&!field<unsigned>(unit,0x9ec),"Mounted charge required unit order or duplicated rider formation membership");
    auto mounted_before=horse;state->heartbeat=GetTickCount64();require(!prepare(h)&&horse==mounted_before,"Manual movement overwrote independent native horse charge");
    auto prior_queries=charge_queries148;for(unsigned i=0;i<1000;++i)charge_boundary146(unit,false);require(charge_queries148==prior_queries,"Committed charge continuously reprobed the crowd");
    charge_physical_input148=0;state->mode=IDLE;sync_members130(unit);charge_reset139();charge_boundary146(unit,true);
    require(!charge_input_locked146()&&!proximity_count147&&!detached_count130&&!charge_errors139&&!proximity_errors147,"Mounted exit retained charge control/membership or concealed a native fault");
    MH_DisableHook(MH_ALL_HOOKS);MH_Uninitialize();
    // Engine spatial-query containers cache allocations for process lifetime.
    // This private fixture owns its allocator; retire every remaining fixture
    // block once all native operations have finished, before freeing the image.
    while(!blocks147.empty())free_words147(*blocks147.begin());
    std::cout<<"PASS: deliberate primary + forward sprint, preserved Follow Me, native enemy perception of independent player; original native perception/height/allegiance/FOV/eligibility and nearest reduction choose real forward target; behind/dead/high/occluded cases; repeated queries retain no extra allocations; no unit order or forced kill; original human/horse/rider dispatch,1000 committed ticks without reprobe, input yield, confirmed native contact-to-QTE handoff and exit restoration. Grid/collision/path/allocation services supplied; rendered outcomes remain live validation.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
