// Production camera sequence with ORIGINAL134fad0/134fe50, not a hand-built
// orthogonal camera. Geometry/allocation remain explicit fixture providers.
#define main retained_commands_main153
#include "verify_native_commands100.cpp"
#undef main
static unsigned world_queries152;
static bool obstruct152;
static std::array<unsigned char,28> hits153;
static void __fastcall world_view152(void*,void*,void* out,const void* input,unsigned mode,unsigned sorted,unsigned mask,unsigned){
    require(!mode&&sorted==1&&mask==0x303,"Third person world ABI differs");++world_queries152;
    auto segment=static_cast<const float*>(input);auto v=static_cast<unsigned*>(out);
    if(obstruct152){v[0]=v[1]=1;v[2]=unsigned(hits153.data());for(unsigned i=0;i<3;++i)field<float>(uintptr_t(hits153.data()),4+i*4)=(segment[1+i]+segment[4+i])*.5f;}
}
static void __cdecl sort_view152(void*,void*,unsigned,unsigned){}
static void __cdecl free_view152(void* p){require(p==hits153.data(),"Camera freed foreign query");}
int main(){SetUnhandledExceptionFilter(fault100);try{
    NativeFixture100 f;using P=NativeFixture100;auto cam=P::ptr(f.camera);
    DWORD old{};VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old);
    f.jump(0x36e8f0,reinterpret_cast<void*>(world_view152));
    f.jump(0x101a80,reinterpret_cast<void*>(sort_view152));
    f.jump(0xf04d0,reinterpret_cast<void*>(free_view152));
    original_matrix=reinterpret_cast<Matrix>(base+0x134fad0);
    view_reset152();view_prepare152();InterlockedExchange(&third_person152,1);view_distance152=3;
    unsigned cases=0;
    for(float heading:{-2.8f,-1.f,0.f,.7f,2.8f})for(float pitch:{-.8f,-.21f,0.f,.25f,.8f})for(float x:{0.f,5.f,12.f}){
        state->heartbeat=GetTickCount64();state->yaw=heading;state->pitch=pitch;
        float affine[12]={1,0,0,x,0,1,0,2,0,0,1,7};
        view_prepare152();original_matrix(reinterpret_cast<void*>(cam),affine);adjust_eye(cam);
        float pivot[3];memcpy(pivot,reinterpret_cast<void*>(cam+8),12);
        obstruct152=false;view_camera152(cam);
        float d=0;for(unsigned k=0;k<3;++k){float v=field<float>(cam,8+k*4)-pivot[k];d+=v*v;}
        require(third_person152&&std::abs(std::sqrt(d)-3*std::sqrt(1.f+.25f*.25f+.08f*.08f))<.001f,"Normal pitched camera lost its full shoulder segment");
        capture_aim_frame(cam);ViewRay ray{};require(read_aim_frame(ray),"Third-person displayed aim missing");
        for(unsigned k=0;k<3;++k)require(std::abs(ray.origin[k]-field<float>(cam,8+k*4))<.001f,"Aim retained first-person eye");
        require(std::abs(field<float>(cam,0x68)-std::sin(pitch))<.001f,"Original native direction refresh was not retained");++cases;
    }
    require(world_queries152==cases,"Missing or repeated per-view geometry query");
    for(float heading154:{-1.f,0.f,2.f}){
        state->yaw=heading154;state->pitch=.1f;combat_entry_locked126=0;animation_look_active104=0;
        float affine154[12]={1,0,0,5,0,1,0,2,0,0,1,7};
        auto before154=steering_view(FORWARD|LEFT,state->yaw,1.f,1.f);
        require(weapon_window_proc(nullptr,WM_MBUTTONDOWN,0,0)==0,"MMB was not owned by third person");
        mouse_look101(320,-100,FORWARD|LEFT,.016f);
        require(state->yaw==heading154&&state->pitch==.1f,"Orbit mouse redirected soldier/movement/eye pitch");
        auto after154=steering_view(FORWARD|LEFT,state->yaw,1.f,1.f);require(before154.x==after154.x&&before154.z==after154.z,"WASD follows orbit instead of saved movement facing");
        original_matrix(reinterpret_cast<void*>(cam),affine154);adjust_eye(cam);float pivot154[3];memcpy(pivot154,reinterpret_cast<void*>(cam+8),12);view_camera152(cam);
        float distance154=0;for(unsigned i=0;i<3;++i){float d=field<float>(cam,8+i*4)-pivot154[i];distance154+=d*d;}
        require(std::abs(std::sqrt(distance154)-3*std::sqrt(1.f+.25f*.25f+.08f*.08f))<.001f,"Orbit lost soldier pivot/shoulder segment");
        require(std::abs(field<float>(cam,0x64)-std::sin(heading154+.8f)*std::cos(.35f))<.001f,"Orbit did not swivel rendered native camera");
        weapon_window_proc(nullptr,WM_MBUTTONUP,0,0);require(!orbit_held154&&orbit_yaw154==0&&orbit_pitch154==0,"MMB release retained orbit input");
    }
    view_reset152();std::cout<<"PASS: "<<cases<<" production stationary/walking pitched-camera cases through ORIGINAL134fad0 and134fe50; P persists and displayed targeting follows. Geometry/allocation providers explicit.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
