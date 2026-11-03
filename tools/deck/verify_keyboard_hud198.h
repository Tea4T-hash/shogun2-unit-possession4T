// Included inside the actual native keyboard-hook fixture after ownership starts.
// The native device reader is substituted; the real poll hook and buffers run.
root151[0xca0]=0;physical[0x25]=0x80;
require(poll()&&!current[0x25]&&!previous[0x25]&&keyboard_window_owned84('K'),"Possession K leaked to stock toggle");
for(unsigned n=0;n<100;++n)require(poll()&&!current[0x25]&&!previous[0x25]&&!hud_visibility_shared148->hidden,"Held K repeated in poll");
// The dispatch fixture checks the real K/View native call; here check the
// subsequent keyboard poll cannot overwrite its independent mod-HUD result.
auto root198=reinterpret_cast<uintptr_t>(root151.data());
for(unsigned stage198:{1u,2u,0u}){
 root151[0xca0]=stage198?1:0;hudcycle198::commit(root198,status.lease_generation,stage198);
 require(poll()&&hud_visibility_shared148->hidden==unsigned(stage198==2),"HUD result lost on keyboard poll");
}
status.mode=0;require(poll()&&!current[0x25]&&!previous[0x25],"Held possession K leaked on RTS return");
physical[0x25]=0;require(poll()&&!keyboard_window_owned84('K'),"K release not drained");
physical[0x25]=0x80;root151[0xca0]=1;
require(poll()&&current[0x25]==0x80&&previous[0x25]==0&&hud_visibility_shared148->hidden,"Fresh RTS K lost native toggle");
status.mode=POSSESSED;++status.lease_generation;
require(poll()&&!current[0x25]&&!previous[0x25]&&!hud_visibility_shared148->hidden,"Entry with RTS hidden must keep possession HUD visible");
root151[0xca0]=1;hudcycle198::commit(root198,status.lease_generation,2);require(poll()&&hud_visibility_shared148->hidden,"Both-hidden stage missing");
++status.lease_generation;require(poll()&&!hud_visibility_shared148->hidden,"Old lease HUD preference leaked");
hudcycle198::commit(root198,status.lease_generation,2);root151[0xca0]=0;
require(poll()&&!hud_visibility_shared148->hidden,"External native show did not reconcile");
physical[0x25]=0;require(poll(),"K final release");
