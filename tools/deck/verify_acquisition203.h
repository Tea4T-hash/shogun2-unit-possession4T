{
 std::vector<unsigned char> observer203(0x700),enemy_unit203(0x1800),enemy_group203(0x2800),context203(0x40),ours203(0x20),theirs203(0x20);
 auto observer=P::ptr(observer203),eu=P::ptr(enemy_unit203),eg=P::ptr(enemy_group203),ctx=P::ptr(context203);
 uintptr_t observer_members[]={observer},enemy_registered[]={eg};auto old_side=field<uintptr_t>(possessed,0x44);
 field<uintptr_t>(possessed,0x44)=P::ptr(ours203);field<unsigned>(P::ptr(ours203),0x10)=0;field<unsigned>(P::ptr(theirs203),0x10)=1;
 field<uintptr_t>(observer,0x44)=P::ptr(theirs203);field<uintptr_t>(observer,0x28)=environment;field<uintptr_t>(observer,0x214)=eu;field<unsigned char>(observer,0xf0)=1;
 field<uintptr_t>(eu,0x14)=environment;field<unsigned>(eu,0xb8c)=1;field<uintptr_t>(eu,0xb90)=reinterpret_cast<uintptr_t>(enemy_registered);
 field<uintptr_t>(eg,0)=base+0x15bb21c;field<uintptr_t>(eg,4)=environment;field<uintptr_t>(eg,0x510)=eu;field<unsigned>(eg,0x18)=1;field<uintptr_t>(eg,0x1c)=reinterpret_cast<uintptr_t>(observer_members);
 field<float>(observer,0x48)=field<float>(possessed,0x48);field<float>(observer,0x50)=field<float>(possessed,0x50)-10;
 field<float>(eg,0x644)=field<float>(observer,0x48);field<float>(eg,0x648)=field<float>(observer,0x50);
 field<float>(formation,0x644)=field<float>(P::ptr(f.peer),0x48);field<float>(formation,0x648)=field<float>(P::ptr(f.peer),0x50);
 field<uintptr_t>(ctx,4)=eg;field<uintptr_t>(ctx,0x18)=field<uintptr_t>(ctx,0x24)=unit;float forward[]={0,1};
 require(original_charge_pick203(context203.data(),observer,forward,1)==0,"Original native selector omission not reproduced");
 auto pick=reinterpret_cast<ChargePick203>(base+0x174ec0);require(pick(context203.data(),observer,forward,1)==possessed,"Independent native charge target omitted");
 auto group=reinterpret_cast<TargetSquad147>(base+0x1679c0);require(original_target_squad147(context203.data())==formation&&group(context203.data())==ip,"Unit-only target remained distant formation");
 field<float>(eg,0x644)=field<float>(formation,0x644);field<float>(eg,0x648)=field<float>(formation,0x648);require(group(context203.data())==formation,"Nearer formation target replaced");
 field<float>(eg,0x644)=field<float>(observer,0x48);field<float>(eg,0x648)=field<float>(observer,0x50);
 field<unsigned>(P::ptr(theirs203),0x10)=0;require(pick(context203.data(),observer,forward,1)==0&&group(context203.data())==formation,"Friendly target substituted");field<unsigned>(P::ptr(theirs203),0x10)=1;
 field<float>(observer,0x48)+=5;require(pick(context203.data(),observer,forward,1)==original_charge_pick203(context203.data(),observer,forward,1),"Original corridor widened");field<float>(observer,0x48)-=5;
 field<uintptr_t>(ctx,0x34)=P::ptr(f.peer);require(group(context203.data())==formation,"Explicit other target replaced");field<uintptr_t>(ctx,0x34)=0;
 state->release=1;require(group(context203.data())==formation&&pick(context203.data(),observer,forward,1)==0,"Release kept acquisition override");state->release=0;
 require(field<uintptr_t>(unit,0xb9c)==formation&&field<uintptr_t>(possessed,0x214)==unit&&physical[0]==possessed,"Acquisition changed ownership/roster/main group");
 field<uintptr_t>(possessed,0x44)=old_side;
 std::cout<<"PASS acquisition203: actual native omission reproduced; charge and unit-only target resolve registered independent group, preserve friendlies/corridor/explicit target/release and ownership. Strategic AI goals not simulated.\n";
}
