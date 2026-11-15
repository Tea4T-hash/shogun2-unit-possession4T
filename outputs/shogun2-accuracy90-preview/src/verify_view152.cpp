#define main retained_commands152
#include "verify_native_commands100.cpp"
#undef main
static unsigned refresh152,world_queries152;static bool obstruct152;static std::array<unsigned char,28> hits152;
static void __fastcall refresh_view152(void*,void*){++refresh152;}
static void __fastcall world_view152(void*,void*,void* out,const void* input,unsigned mode,unsigned sorted,unsigned mask,unsigned){
    require(!mode&&sorted==1&&mask==0x303,"Third person queried actors or changed native world ABI");++world_queries152;
    auto segment=static_cast<const float*>(input);auto v=static_cast<unsigned*>(out);
    if(obstruct152){v[0]=v[1]=1;v[2]=unsigned(hits152.data());for(unsigned i=0;i<3;++i)field<float>(uintptr_t(hits152.data()),4+i*4)=(segment[1+i]+segment[4+i])*.5f;}
}
static void __cdecl sort_view152(void*,void*,unsigned,unsigned){}
static void __cdecl free_view152(void* p){require(p==hits152.data(),"Camera freed foreign query");}
int main(){SetUnhandledExceptionFilter(fault100);try{
    NativeFixture100 f;using P=NativeFixture100;state->heartbeat=GetTickCount64();auto cam=P::ptr(f.camera);
    DWORD old{};VirtualProtect(f.image,0x1580000,PAGE_EXECUTE_READWRITE,&old);
    f.jump(unsigned(field<uintptr_t>(base+0x15d22e8,0xf4)-base),reinterpret_cast<void*>(refresh_view152));
    f.jump(0x36e8f0,reinterpret_cast<void*>(world_view152));f.jump(0x101a80,reinterpret_cast<void*>(sort_view152));f.jump(0xf04d0,reinterpret_cast<void*>(free_view152));
    std::uint8_t current[256]{},previous[256]{};view_reset152();
    current[0x19]=0x80;view_key152(current,previous,true,true);view_prepare152();require(third_person152&&!current[0x19],"P did not toggle/mask");
    for(unsigned i=0;i<30;++i){current[0x19]=0x80;view_key152(current,previous,true,true);view_prepare152();require(third_person152,"Held P repeated toggle");}
    auto native=[&](){float a[6]={0,2,0,0,2,1},up[3]={0,1,0};memcpy(reinterpret_cast<void*>(cam+8),a,24);memcpy(reinterpret_cast<void*>(cam+0x58),up,12);};
    native();obstruct152=false;view_distance152=3;view_camera152(cam);require(std::abs(field<float>(cam,0x10)+3)<.001f&&refresh152==1&&world_queries152==1,"Third person did not move the real camera via original world query");
    capture_aim_frame(cam);ViewRay ray{};require(read_aim_frame(ray)&&ray.origin[2]==-3&&ray.direction[2]==1,"Commands did not capture third-person view");
    native();obstruct152=true;view_camera152(cam);require(std::abs(field<float>(cam,0x10)+3.f*(.5f-.25f/(3.f*std::sqrt(1.f+.25f*.25f+.08f*.08f))))<.001f,"Camera did not pull in before obstruction");
    view_wheel152(120);view_prepare152();require(view_distance152==2.5f,"Wheel distance direction/step");for(unsigned i=0;i<20;++i){view_wheel152(12000);view_prepare152();}require(view_distance152==1,"Zoom minimum");
    for(unsigned i=0;i<20;++i){view_wheel152(-12000);view_prepare152();}require(view_distance152==12,"Zoom maximum");
    view_key152(current,previous,true,true);current[0x19]=0x80;view_key152(current,previous,true,true);view_prepare152();require(!third_person152,"Second P did not restore first person");
    native();view_camera152(cam);require(field<float>(cam,0x10)==0,"First person changed by offset");
    current[0x19]=0x80;view_key152(current,previous,false,true);require(!current[0x19],"Held P leaked into RTS");current[0x19]=0;view_key152(current,previous,false,true);current[0x19]=0x80;view_key152(current,previous,false,true);require(current[0x19]==0x80,"Fresh RTS P was consumed");
    for(float heading:{-3.13f,-1.f,0.f,1.f,3.13f})for(float elevation:{-1.f,0.f,1.f})for(float y:{-9.f,0.f,9.f})for(float p:{-9.f,0.f,9.f}){
        deathpose::Eye e{{4,10,2,4+std::sin(heading)*std::cos(elevation),10+std::sin(elevation),2+std::cos(heading)*std::cos(elevation)},
            {-std::sin(heading)*std::sin(elevation),std::cos(elevation),-std::cos(heading)*std::sin(elevation)}},out{};
        require(view152::eye(e,y,p,out),"Bounded native face basis rejected");float fwd[3],next[3];for(unsigned i=0;i<3;++i){fwd[i]=e.view[i+3]-e.view[i];next[i]=out.view[i+3]-out.view[i];}deathpose::normalize(fwd);deathpose::normalize(next);
        require(!memcmp(e.view,out.view,12)&&deathpose::dot(fwd,next)>=std::cos(view152::yaw_limit)*std::cos(view152::pitch_limit)-.001f,"Eye escaped face or changed origin");
    }
    #include "../../../../tools/deck/verify_framing203.h"
    view_reset152();require(!third_person152&&!eye_yaw152&&!eye_pitch152,"Exit retained third-person/eye state");
    std::cout<<"PASS: P ownership/held handoff, wheel bounds, actual native world-query camera/obstruction and targeting, first-person restoration, current-face eye limits. World geometry/allocation and camera refresh are explicit providers.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
