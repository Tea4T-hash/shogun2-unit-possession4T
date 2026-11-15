// Original retail ability lookup/unlock/cooldown, constructor and serializer.
// Native battle scheduler and effect application are not simulated; primitive
// writers, vector allocation, game phase and movement eligibility are providers.
#define main retained_command141_main
#include "verify_native_commands100.cpp"
#undef main
static bool __cdecl false141(){return false;}
static bool __fastcall stationary141(void*,void*){return false;}
static bool kneel_blocked101;
static bool __fastcall kneel_gate101(void*,void*){return kneel_blocked101;}
static bool __fastcall can_fire141(void*,void*){return true;}
int main(){SetUnhandledExceptionFilter(&fault100);try{
 NativeFixture100 f;using P=NativeFixture100;field<uintptr_t>(unit,0x14)=environment;
 auto patch=[&](unsigned address,void* fn){DWORD old{};require(VirtualProtect(reinterpret_cast<void*>(base+address),16,PAGE_EXECUTE_READWRITE,&old)!=0,"Patch fixture provider");f.jump(address,fn);VirtualProtect(reinterpret_cast<void*>(base+address),16,old,&old);};
 patch(0x459550,reinterpret_cast<void*>(&false141));patch(0x180760,reinterpret_cast<void*>(&stationary141));patch(0x180730,reinterpret_cast<void*>(&kneel_gate101));patch(0x15c4c0,reinterpret_cast<void*>(&can_fire141));
 unsigned char descriptors[7*28]{};memcpy(descriptors,f.descriptors,sizeof f.descriptors);unsigned key=0x141141;
 field<unsigned>(base,0x192ee34)=key;memcpy(descriptors+6*28,&key,4);descriptors[6*28+16]=3;field<uintptr_t>(base,0x18d9000)=reinterpret_cast<uintptr_t>(descriptors);field<unsigned>(base,0x18d9004)=7;
 std::vector<unsigned char> entries(4*0x282c),records(4*0x60);auto array=P::ptr(entries),defs=P::ptr(records),manager=unit+0xed4;
 field<uintptr_t>(manager,0)=base+0x15cfeec;field<uintptr_t>(manager,8)=unit;field<unsigned>(manager,0x24)=4;field<unsigned>(manager,0x28)=3;field<uintptr_t>(manager,0x2c)=array;
 unsigned ids[]={39,23,26,39};
 for(unsigned i=0;i<4;++i){auto record=defs+i*0x60;field<uintptr_t>(array+i*0x282c,4)=record;field<unsigned>(record,0)=ids[i];field<float>(record,8)=10;field<float>(record,0x10)=-1;if(i==1||i==2)field<uintptr_t>(record,0x28)=defs;}
 ability141::Status sample{};bool collected=ability_collect141(sample);std::cerr<<"Collected="<<collected<<" count="<<sample.count<<"\n";for(unsigned i=0;i<sample.count;++i)std::cerr<<sample.items[i].id<<":"<<sample.items[i].flags<<" ";std::cerr<<"\n";require(collected&&sample.count==3,"Native live ability list not collected");
 require(sample.items[0].id==39&&(sample.items[0].flags&ability141::Available),"Original native Rapid eligibility lost");
 field<float>(array,0xc)=-1;sample={};require(ability_collect141(sample)&&sample.count==3&&sample.items[0].cooldown==0&&(sample.items[0].flags&ability141::Available),"Native non-positive cooldown was hidden or blocked");
 field<float>(array,0xc)=12;sample={};require(ability_collect141(sample)&&sample.items[0].cooldown==12&&!(sample.items[0].flags&ability141::Available),"Cooldown bypassed native eligibility");field<float>(array,0xc)=0;
 field<unsigned>(manager,0x28)=4;sample={};require(ability_collect141(sample)&&sample.count==3,"Duplicate native ID duplicated selector");field<unsigned>(manager,0x28)=3;
 field<uintptr_t>(manager,8)=unit+4;sample={};require(!ability_collect141(sample),"Foreign manager accepted");field<uintptr_t>(manager,8)=unit;
 field<unsigned>(manager,0x28)=129;require(!ability_collect141(sample),"Unbounded ability list accepted");field<unsigned>(manager,0x28)=3;
 sample={};require(ability_collect141(sample),"Restored ability list refused");auto actor_before=f.actor;
 for(unsigned repeat=0;repeat<1000;++repeat){f.clear_queue();require(ability_send141(sample,0),"Native whole-unit ability command refused");require(packets100.size()==1&&packets100[0].integers[0]==39&&packets100[0].integers[1]==0&&packets100[0].bytes[0]==0,"Native ability ID/action/group flags changed");}
 require(f.actor==actor_before,"Ability selector modified soldier state directly");
 field<unsigned char>(array,0x18)=1;sample={};ability_collect141(sample);f.clear_queue();require(ability_send141(sample,0)&&packets100[0].integers[1]==1,"Active ability did not use native disable action");field<unsigned char>(array,0x18)=0;
 field<float>(array,0xc)=9;sample={};ability_collect141(sample);f.clear_queue();require(!ability_send141(sample,0)&&packets100.empty(),"Cooling ability queued");field<float>(array,0xc)=0;
 sample={};ability_collect141(sample);f.clear_queue();field<unsigned>(P::ptr(f.queue)+4,QUEUE_CAPACITY)=QUEUE_CAPACITY-1;require(!ability_send141(sample,0)&&field<unsigned>(P::ptr(f.queue)+4,QUEUE_CAPACITY)==QUEUE_CAPACITY-1,"Full queue partially published ability");
 ability141::Status shared{};ability_shared141=&shared;shared.magic=ability141::Magic;shared.version=141;shared.size=sizeof shared;shared.pid=GetCurrentProcessId();state->heartbeat=GetTickCount64();ability_reset141();f.clear_queue();
 ability_process141(P::ptr(f.camera));require(shared.count==3&&ability141::valid(shared,state->pid,state->lease_generation,state->actor_id,state->unit_id,GetTickCount64()),"Production ability publication invalid");
 ability_request141={state->lease_generation-1,2,true};ability_process141(P::ptr(f.camera));require(packets100.empty()&&shared.selected==0,"Old lease request applied");
 ability_request141={state->lease_generation,1,false};ability_process141(P::ptr(f.camera));require(shared.selected==1&&packets100.empty(),"T cycle activated ability");
 equipment_ranged81=1;equipment_slot81=equipment81::Ranged;field<unsigned>(possessed,0x1f0)=8;field<float>(base,0x15b2448)=.04f;
 field<unsigned char>(array+2*0x282c,0x18)=1;sample={};ability_collect141(sample);f.clear_queue();state->weapon_phase=WEAPON_DRAW;require(!ability_send141(sample,1)&&packets100.empty(),"Ammo changed inside committed draw");state->weapon_phase=0;
 sample={};ability_collect141(sample);f.clear_queue();require(ability_send141(sample,1)&&packets100.size()==2&&packets100[0].integers[0]==26&&packets100[0].integers[1]==1&&packets100[1].integers[0]==23&&packets100[1].integers[1]==0,"Ammo transaction did not disable old style before activating new");
 const auto kneel_record=defs+3*0x60;
 field<unsigned>(kneel_record,0)=57;
 field<unsigned>(manager,0x28)=4;sample={};
 require(ability_collect141(sample)&&sample.count==4&&sample.items[3].id==57&&
         (sample.items[3].flags&ability141::Available),"Native Crouch Fire absent from selector");
 f.clear_queue();
 require(ability_send141(sample,3)&&packets100.size()==1&&packets100[0].integers[0]==57&&
         packets100[0].integers[1]==0,"Crouch Fire did not use the native enable command");
 field<unsigned char>(array+3*0x282c,0x18)=1;sample={};
 require(ability_collect141(sample)&&(sample.items[3].flags&ability141::Active),"Native Crouch Fire active flag lost");
 f.clear_queue();
 require(ability_send141(sample,3)&&packets100.size()==1&&packets100[0].integers[1]==1,
         "Crouch Fire did not use the native disable command");
 kneel_blocked101=true;sample={};f.clear_queue();
 require(ability_collect141(sample)&&!(sample.items[3].flags&ability141::Available)&&
         !ability_send141(sample,3)&&packets100.empty(),"Native Crouch Fire eligibility bypassed");
 EquipmentRequests81 tab{};equipment_keys134(tab,4,true);require(!tab.tabs,"Tab still changed ammo");
 ability_reset141();require(!ability_request141.lease&&ability_selected141==~0u&&!ability_pending_until141,"Exit retained ability request");ability_shared141=nullptr;
 std::cout<<"PASS: production native ability list, original unlock/cooldown and whole-unit serialization;1000 activations, no actor writes, inactive/active/ammo transaction, full queue, stale lease, selector/publication/reset and removed Tab binding. Native effect scheduler/visuals require live testing.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
