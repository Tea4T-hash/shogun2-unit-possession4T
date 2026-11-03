static float fixture_vx203{},fixture_vz203{};static bool fixture_mark203{},fixture_zero_intent204{};
static bool __fastcall can_run203(void*,void*){return true;}
static bool __fastcall special_gait203(void*,void*){return false;}
static void __fastcall integration203(void* value,void*){
 auto a=reinterpret_cast<uintptr_t>(value);float target[]={field<float>(a,0x48)+fixture_vx203*.04f,field<float>(a,0x50)+fixture_vz203*.04f};
 reinterpret_cast<NativeIntent>(base+0x3936f0)(value,target,0);
 field<float>(a,0x8c)=fixture_vx203;field<float>(a,0x94)=fixture_vz203;
 if(fixture_zero_intent204)field<float>(a,0x150)=0;
 if(fixture_mark203)locomotion_mark203(a);
}
