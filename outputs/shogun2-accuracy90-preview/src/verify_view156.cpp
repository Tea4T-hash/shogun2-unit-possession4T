// Actual adapter paths and ORIGINAL134fad0/134fe50. Scene geometry, native
// projection publication and UI tree are explicit providers, not a live battle.
#define main retained_view153_main156
#include "verify_native_commands100.cpp"
#undef main
static void __fastcall world156(void*,void*,void*,const void*,unsigned mode,unsigned sorted,unsigned mask,unsigned){require(!mode&&sorted==1&&mask==0x303,"Camera queried non-world geometry");}
static void __cdecl sort156(void*,void*,unsigned,unsigned){}
static void __cdecl free156(void*){throw std::runtime_error("Empty world query unexpectedly allocated");}
static unsigned death_publications156,retired_queries156;
static void __fastcall publish156(void*,void*){++death_publications156;}
static bool __fastcall retired156(void*,void*,float,float){++retired_queries156;return false;}
int main(){SetUnhandledExceptionFilter(fault100);try{
 NativeFixture100 f;using P=NativeFixture100;auto cam=P::ptr(f.camera);
 DWORD old{};VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old);
 f.jump(0x36e8f0,reinterpret_cast<void*>(world156));f.jump(0x101a80,reinterpret_cast<void*>(sort156));f.jump(0xf04d0,reinterpret_cast<void*>(free156));
 f.jump(0x134f1d0,reinterpret_cast<void*>(publish156));original_matrix=reinterpret_cast<Matrix>(base+0x134fad0);
 original_camera_update104=reinterpret_cast<CameraUpdate104>(retired156);
 view_reset152();view_prepare152();third_person152=1;view_distance152=5;
 state->yaw=1.25f;state->pitch=.31f;orbit_button154(true);orbit_mouse154(.65f,-.13f);
 auto render=[&](float body){
   state->heartbeat=GetTickCount64();float affine[12]={std::cos(body),0,-std::sin(body),5,0,1,0,2,std::sin(body),0,std::cos(body),7};
   original_matrix(reinterpret_cast<void*>(cam),affine);remember_native_eye78(cam);adjust_eye(cam);
   float pivot[3];memcpy(pivot,reinterpret_cast<void*>(cam+8),12);view_camera152(cam);
   float d=0;for(unsigned i=0;i<3;++i){float x=field<float>(cam,8+i*4)-pivot[i];d+=x*x;}
   require(std::abs(std::sqrt(d)-5*std::sqrt(1.f+.25f*.25f+.08f*.08f))<.001f,"QTE reset third-person distance");
   require(std::abs(field<float>(cam,0x64)-std::sin(state->yaw+.65f)*std::cos(state->pitch-.13f))<.001f,"New QTE reset/oriented the orbit view");
 };
 for(unsigned serial=1;serial<=100;++serial){
   melee60::Request r;r.pid=GetCurrentProcessId();r.lease=possession_lease.generation;r.player=state->actor_id;r.player_unit=state->unit_id;
   r.opponent=123;r.opponent_unit=8;r.serial=serial;r.heartbeat=GetTickCount64();r.phase=melee60::Facing;
   require(PossessionMeleeCommitment63(&r)==63,"QTE setup failed");
   float yaw=state->yaw,pitch=state->pitch;combat_entry_locked126=1;render(float(serial)*.83f);
   require(state->yaw==yaw&&state->pitch==pitch&&orbit_held154,"Pair entry changed saved manual view");
   r.phase=melee60::NativeCombat;PossessionMeleeCommitment63(&r);render(-float(serial)*.47f);
   melee60::Request off;PossessionMeleeCommitment63(&off);render(.9f);
 }
 orbit_button154(false);combat_entry_locked126=1;
 melee60::Request r;r.pid=GetCurrentProcessId();r.lease=possession_lease.generation;r.player=state->actor_id;r.player_unit=state->unit_id;
 r.opponent=123;r.opponent_unit=8;r.serial=101;r.heartbeat=GetTickCount64();r.phase=melee60::Facing;PossessionMeleeCommitment63(&r);
 float yaw=state->yaw;mouse_look101(40,0,0,.016f);require(std::abs(state->yaw-yaw-.1f)<.00001f,"Third-person QTE mouse was locked by first-person entry");
 third_person152=0;combat_entry_locked126=1;yaw=state->yaw;mouse_look101(40,0,0,.016f);require(state->yaw==yaw,"First-person entry lock changed");
 third_person152=1;combat_entry_locked126=0;melee60::Request off;PossessionMeleeCommitment63(&off);
 deathpose::Eye native{{5,2,7,5,2,8},{0,1,0}};ModelEye sample{possession_lease.generation,state->actor_id,state->unit_id,GetTickCount64(),native};
 publish_model_eye(native_eye_channel78,sample);begin_death_view();require(death_third_person156,"Death lost selected third-person mode");
 // Retire actor and holder after the numeric handoff. This path must not query
 // either, nor call the native roster update which would select another man.
 field<uintptr_t>(P::ptr(f.holder),8)=0;field<unsigned>(possessed,0x1c8)=99;
 yaw=state->yaw;float pitch=state->pitch;
 for(unsigned i=0;i<100;++i){sample.tick=GetTickCount64();sample.eye.view[0]=float(i)*.02f;publish_model_eye(model_eye_channel,sample);
   require(camera_update104(reinterpret_cast<void*>(cam),nullptr,.1f,.5f),"Death update returned failure to native manager");
   require(state->yaw==yaw&&state->pitch==pitch&&third_person152,"Death changed third-person heading");
   float expected=sample.eye.view[0]-std::sin(yaw)*std::cos(pitch)*5+std::cos(yaw)*1.25f-std::sin(yaw)*std::sin(pitch)*.4f;
   require(std::abs(field<float>(cam,8)-expected)<.001f,"Death lost third-person pivot/distance");}
 require(!retired_queries156&&death_publications156==100,"Death queried retired native roster or missed native publication");
 input_window=CreateWindowExW(0,L"STATIC",L"fixture156",WS_POPUP,0,0,1,1,nullptr,nullptr,nullptr,nullptr);
 eyelid_top=CreateWindowExW(0,L"STATIC",L"fixture156 lid",WS_POPUP,0,0,1,1,nullptr,nullptr,nullptr,nullptr);
 require(input_window&&eyelid_top,"Death overlay fixture windows failed");ShowWindow(eyelid_top,SW_SHOWNOACTIVATE);update_eyelids(true);
 require(!IsWindowVisible(eyelid_top),"Third-person death showed first-person eyelids");DestroyWindow(eyelid_top);DestroyWindow(input_window);eyelid_top=input_window=nullptr;
 reset_death_view();view_reset152();
 std::cout<<"PASS: 100 new QTE pairs and exits retain third-person world look/orbit/distance; mouse responsive, first-person entry lock retained; 100 numeric death updates preserve third person after corpse retirement, omit eyelids and publish native matrix without roster access. Explicit world/UI/projection providers.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
