// Original registered SQUAD factory/lifecycle probe. No live game access.
#define main retained_commands147
#include "verify_native_commands100.cpp"
#undef main
#include "native_group_fixture148.h"
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
 require(MH_Initialize()==MH_OK,"Proximity hook init");create_unit_detach130();create_ranged_position128();create_formation_arrival129();require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Proximity hooks enable");
 state->heartbeat=GetTickCount64();sync_members130(unit);
 require(proximity_count147==1&&proximity_managed147(possessed)&&!detached_count130&&field<uintptr_t>(possessed,0x3b4)==formation,"Nearby soldier did not remain in native formation");
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
 // Original foot target enumeration includes the independent player, and
 // original single-member geometry follows the physical actor's position.
 unsigned native_targets[8]{};uintptr_t targets[4]{};native_targets[2]=4;native_targets[4]=reinterpret_cast<uintptr_t>(targets);
 using Enumerate=void(__thiscall*)(void*,void*);reinterpret_cast<Enumerate>(base+0x3c1c10)(reinterpret_cast<void*>(possessed),native_targets);
 require(native_targets[3]==1&&targets[0]==possessed,"Original individual target enumeration lost independent player");
 unsigned sample[7]{},samples[]={1,1,reinterpret_cast<uintptr_t>(sample)};using Sample=void(__thiscall*)(void*,uintptr_t);
 reinterpret_cast<Sample>(base+0x126df0)(sample,possessed);field<float>(base,0x1bd9564)=1;
 using Bounds=void(__thiscall*)(void*,const void*);reinterpret_cast<Bounds>(base+0x159d70)(reinterpret_cast<void*>(ip+0x628),samples);
 require(field<float>(ip,0x644)==field<float>(possessed,0x48)&&field<float>(ip,0x648)==field<float>(possessed,0x50),"Original independent target bounds stayed at formation position");
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
 field<float>(h,0x48)=1;proximity_find147(h)->checked=0;state->heartbeat=GetTickCount64();sync_members130(unit);charge_assignment147(f,formation,h,ph,true);
 state->mode=IDLE;sync_members130(unit);require(!proximity_count147&&!detached_count130&&field<uintptr_t>(h,0x3b4)==formation&&!field<unsigned>(unit,0xb70),"Mounted exit did not restore and retire native groups");
 require(!proximity_errors147&&!member_errors130&&!charge_errors139,"Proximity concealed a native exception");
 MH_DisableHook(MH_ALL_HOOKS);MH_Uninitialize();
 // Retire the test's original regular group too, through its native lifecycle.
 for(auto a:{possessed,P::ptr(f.peer),h,ph})if(field<uintptr_t>(a,0x3b4)==formation)remove(reinterpret_cast<void*>(formation),a);
 require(reinterpret_cast<Retire>(base+0x169130)(reinterpret_cast<void*>(unit+0xb80),formation)&&blocks147.empty(),"Final fixture lifecycle leaked native allocations");
 std::cout<<"PASS: original native kind2/3 group lifecycle; foot/mounted proximity joins/leaves,4m join/8m leave and1000 cycles; no own-position boundary feedback; same native unit roster/stats; native actor target resolver/enumerator and single-member bounds; nearby formation firing excludes only player; peer waits remain; exact exit/pool retirement. Enemy strategic acquisition and rendered shots/charges remain live checks.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
