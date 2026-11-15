#define main retained_movement131_main
#include "verify_native_commands100.cpp"
#undef main
int main(){SetUnhandledExceptionFilter(&fault100);try{
    NativeFixture100 f;using P=NativeFixture100;
    std::vector<unsigned char> squad(0x2800),aggregate(0x100);uintptr_t members[1]={possessed};auto s=P::ptr(squad);
    field<unsigned>(s,0x18)=1;field<uintptr_t>(s,0x1c)=reinterpret_cast<uintptr_t>(members);field<uintptr_t>(s,0x510)=unit;
    using NativeSpeed=float(__thiscall*)(void*,unsigned,unsigned);auto native=reinterpret_cast<NativeSpeed>(base+0x18b310);
    // A config mismatch is not permission to control another native subject.
    field<uintptr_t>(unit,0xf9c)=P::ptr(aggregate);field<float>(P::ptr(aggregate),0x1c)=19;field<float>(P::ptr(aggregate),0x20)=29;
    float rejected=0,rejected_turn=0;state->heartbeat=GetTickCount64();require(!read_native_movement(RUN,rejected,rejected_turn),"Mismatched movement config gained control");
    field<uintptr_t>(unit,0xf9c)=P::ptr(f.config);
    field<float>(P::ptr(f.config),0x6c)=2;
    unsigned cases=0;
    for(float walk:{.8f,1.4f,3.f})for(float run:{2.f,4.f,8.f})for(float factor:{0.f,.5f,1.f,1.5f,2.f,3.f})
    for(float global_walk:{.5f,1.f,1.25f})for(float global_run:{.5f,1.f,1.25f})for(bool allowed:{false,true})for(unsigned input:{0u,unsigned(RUN)}){
        run_allowed102=allowed;field<float>(P::ptr(f.config),0x1c)=walk;field<float>(P::ptr(f.config),0x20)=run;
        field<float>(unit,0x22c)=factor;field<float>(base,0x18d1c80+0x5c)=global_walk;field<float>(base,0x18d1ce0+0x5c)=global_run;
        unsigned gait=(input&RUN)&&allowed?1:0;float expected=native(squad.data(),gait,1),observed=0,turn=0;
        state->heartbeat=GetTickCount64();bool valid131=read_native_movement(input,observed,turn);
        if(!valid131||observed!=expected||turn!=2)std::cerr<<"case "<<cases<<" valid "<<valid131<<" input "<<input<<" expected "<<expected<<" observed "<<observed<<" turn "<<turn<<" fail "<<follow_failure93<<"\n";
        require(valid131&&observed==expected&&turn==2,"Possessed rate differs from original native entity/unit/global speed calculation");++cases;
    }
    // Rapid-Advance-like unit multiplier transitions must be linear, including
    // expiration; no ability-name heuristic, division or second multiplication.
    field<float>(P::ptr(f.config),0x20)=4;field<float>(base,0x18d1ce0+0x5c)=.75f;run_allowed102=true;
    for(float factor:{1.f,2.f,1.5f,1.f,0.f}){field<float>(unit,0x22c)=factor;float observed=0,turn=0;
        require(read_native_movement(RUN,observed,turn)&&observed==3*factor,"Live modifier was duplicated or retained after expiration");}
    // The actual planner uses18b310 with its last argument zero for
    // the main formation. Use the stock function as the pace oracle.
    uintptr_t squads[]={s};field<unsigned>(unit,0xb70)=1;
    field<uintptr_t>(unit,0xb74)=reinterpret_cast<uintptr_t>(squads);
    field<uintptr_t>(unit,0xb9c)=s;
    field<uintptr_t>(s,0)=base+0x15bb21c;field<uintptr_t>(s,4)=environment;
    members[0]=P::ptr(f.peer);field<uintptr_t>(members[0],0x3b4)=s;
    field<float>(unit,0x22c)=1.1f;
    for(float unit_pace:{.65f,1.f,1.25f})for(float formation_pace:{.75f,1.f,1.1f})for(unsigned input:{0u,unsigned(RUN)}){
        field<float>(unit,0xbb0)=unit_pace;field<float>(s,0x2774)=formation_pace;
        float speed=0,turn_rate=0;
        state->heartbeat=GetTickCount64();
        require(read_native_movement(input,speed,turn_rate)&&speed==native(squad.data(),input?1:0,0)&&turn_rate==2,
            "Possessed pace skipped the original formation's live pace");
    }
    require(personal_travel_input91(FORWARD,true,false,false)==(FORWARD|RUN)&&
        personal_travel_input91(LEFT,true,false,false)==(LEFT|RUN)&&
        personal_travel_input91(FORWARD,false,false,false)==FORWARD&&
        personal_travel_input91(FORWARD,true,true,false)==FORWARD&&
        personal_travel_input91(FORWARD|WALK_OVERRIDE,true,false,false)==(FORWARD|WALK_OVERRIDE)&&
        personal_travel_input91(LEFT,true,false,true)==LEFT&&
        personal_travel_input91(FORWARD|BACKWARD,true,false,false)==(FORWARD|BACKWARD),
        "Follow Me gait must agree for control and animation");
    std::vector<unsigned char> runners[4];
    uintptr_t roster[5]{possessed};
    for(unsigned i=0;i<4;++i){
        runners[i].resize(0x700);
        const auto peer=roster[i+1]=P::ptr(runners[i]);
        field<uintptr_t>(peer,0)=base+0x15dfec8;field<uintptr_t>(peer,0x214)=unit;
        field<uintptr_t>(peer,0x28)=environment;field<uintptr_t>(peer,0x20c)=P::ptr(f.config);
        field<unsigned char>(peer,0xf0)=1;field<unsigned>(peer,0x1c8)=100+i;
        field<uintptr_t>(peer,0x3b4)=s;field<unsigned>(peer,0x244)=1;
        field<float>(peer,0x8c)=float(i+5);field<float>(peer,0x48)=float(i);
    }
    field<float>(s,0x670)=10.f;
    const auto old_count=field<unsigned>(unit,0x2bc);
    const auto old_roster=field<uintptr_t>(unit,0x2c0);
    field<unsigned>(unit,0x2bc)=5;field<uintptr_t>(unit,0x2c0)=reinterpret_cast<uintptr_t>(roster);
    field<unsigned>(roster[4],0x244)=2; // charge is not ordinary formation running
    const auto x=field<float>(possessed,0x48),z=field<float>(possessed,0x50);
    field<float>(possessed,0x48)=field<float>(possessed,0x50)=0.f;
    float matched=3.f;
    match_formation_run91(possessed,RUN,{1.f,0.f,0},matched);
    require(matched==6.f,"Nearby native runners must supply their measured median pace");
    state->follow=1;matched=3.f;
    match_formation_run91(possessed,RUN,{1.f,0.f,0},matched);
    require(matched==3.f,"Follow Me used its own slower followers as a speed reference");
    state->follow=0;field<float>(roster[1],0x8c)=-5.f;
    match_formation_run91(possessed,RUN,{1.f,0.f,0},matched);
    require(matched==3.f,"Counterflow/charge joined the running pace sample");
    field<float>(roster[1],0x8c)=5.f;field<float>(possessed,0x48)=50.f;
    match_formation_run91(possessed,RUN,{1.f,0.f,0},matched);
    require(matched==3.f,"Distant unit changed independent sprint speed");
    field<float>(possessed,0x48)=x;field<float>(possessed,0x50)=z;
    field<unsigned>(unit,0x2bc)=old_count;field<uintptr_t>(unit,0x2c0)=old_roster;
    std::cout<<"PASS formation pace91: local matching runners, excluded charge/counterflow, and independent Follow Me gait.\n";
    for(float invalid:{0.f,-1.f,NAN}){
        field<float>(s,0x2774)=invalid;float speed=0,turn_rate=0;
        require(read_native_movement(RUN,speed,turn_rate)&&speed==native(squad.data(),1,1),
            "Invalid formation pace blocked independent movement");
    }
    field<float>(s,0x2774)=.75f;field<unsigned>(unit,0xb70)=0;float separate=0,turn_rate=0;
    require(read_native_movement(RUN,separate,turn_rate)&&separate==native(squad.data(),1,1),
        "Unregistered formation affected ordinary movement");
    std::cout<<"PASS formation pace72: original main-squad walk/run, live order multipliers and independent fallback.\n";
    field<float>(unit,0x22c)=NAN;float observed=0,turn=0;require(!read_native_movement(RUN,observed,turn),"Invalid speed modifier accepted");
    std::cout<<"PASS: "<<cases<<" exact original native18b310 comparisons: current soldier config, global walk/run scaling, native run restriction, zero/buff/debuff and expiry; mismatched config rejects control. Synthetic squad and run-predicate provider; real Rapid Advance/collision/root-motion behavior needs live comparison.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
