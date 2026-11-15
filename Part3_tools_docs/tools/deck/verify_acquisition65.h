#pragma once
static void verify_ranged65(uintptr_t formation,uintptr_t independent,uintptr_t player_target,uintptr_t native_target){
    uintptr_t targets65[8]{},list65[]={8,0,reinterpret_cast<uintptr_t>(targets65)};
    for(const auto call:ranged_sites65){
        require(ranged_builder65(base+call+5)&&!ranged_builder65(base+call)&&
                !ranged_builder65(base+call+6),"Ranged enumeration escaped its exact callers");
        list65[1]=0;
        original_ranged_group65(reinterpret_cast<void*>(formation),list65);
        require(list65[1]==1&&targets65[0]==native_target,"Native primary target list differs");
        append_ranged_group65(formation,list65);
        require(list65[1]==2&&targets65[1]==player_target,"Independent soldier was omitted from native target list");
        list65[1]=0;
        append_ranged_group65(independent,list65);
        require(!list65[1],"Unrelated target enumeration appended the player");
    }
    std::cout<<"PASS combat65: original ranged list and detached soldier at all three admitted builders\n";
}
