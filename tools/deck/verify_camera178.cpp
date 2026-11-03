// Executes original owned native camera routines in an isolated mapping.
// Config/vtable providers are explicit synthetic substitutes; no DllMain,
// Steam, renderer, world, game process, real camera or gameplay is initialized.
#include "../../native/deck/native_guard175.h"
#include "../../native/deck/cursor178.h"
#include "../../native/deck/shoulder_zoom195.h"
#include <cassert>
#include <iostream>
#include <array>
static uintptr_t image_base178;
static double yaw178;
static float __fastcall config178(void* value,void*){
    auto rva=reinterpret_cast<uintptr_t>(value)-image_base178;
    if(rva==0x192d318||rva==0x192ca70)return 20.f;
    if(rva==0x192cc00)return 20.f;
    return 1.f;
}
static float __fastcall minimum178(void*,void*){return 0;}
static float __fastcall maximum178(void*,void*){return 100;}
static void __fastcall refresh178(void*,void*){}
static void __fastcall one_float178(void*,void*,float){}
static void __fastcall rotate178(void*,void*,float value){yaw178+=value;}
struct Camera178 {
    std::array<uintptr_t,100> table{};
    alignas(16) std::array<unsigned char,1024> data{};
    void set(unsigned offset,float value){memcpy(data.data()+offset,&value,4);}
    float get(unsigned offset){float value;memcpy(&value,data.data()+offset,4);return value;}
    Camera178(){
        table.fill(reinterpret_cast<uintptr_t>(&refresh178));
        table[0x54/4]=reinterpret_cast<uintptr_t>(&one_float178);
        table[0x60/4]=image_base178+0x2f76e0;
        table[0x64/4]=image_base178+0x2f7a50;
        table[0xac/4]=reinterpret_cast<uintptr_t>(&rotate178);
        table[0x144/4]=reinterpret_cast<uintptr_t>(&minimum178);
        table[0x148/4]=reinterpret_cast<uintptr_t>(&maximum178);
        auto pointer=reinterpret_cast<uintptr_t>(table.data());memcpy(data.data(),&pointer,4);
        set(0x78,1.f); // original forward basis [1c..1e]
        set(0x180,10.f);set(0x24,1.f);set(0x30,1.f);
    }
};
int wmain(int argc,wchar_t** argv){
    assert(argc==2);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);_set_error_mode(_OUT_TO_STDERR);
    HMODULE mapped=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(mapped);
    image_base178=reinterpret_cast<uintptr_t>(mapped);assert(deck175::native_guards(image_base178));
    auto target=reinterpret_cast<unsigned char*>(image_base178+0xa0b60);unsigned char saved[5];memcpy(saved,target,5);DWORD protection{};
    assert(VirtualProtect(target,5,PAGE_EXECUTE_READWRITE,&protection));target[0]=0xe9;
    uint32_t jump=uint32_t(reinterpret_cast<uintptr_t>(&config178)-reinterpret_cast<uintptr_t>(target)-5);memcpy(target+1,&jump,4);
    FlushInstructionCache(GetCurrentProcess(),target,5);DWORD ignored{};assert(VirtualProtect(target,5,protection,&ignored));
    // Original RET 8: frame time plus a second stack argument; the latter is
    // reused as scratch storage by this routine. Keep the native stack ABI.
    using Tick=bool(__thiscall*)(void*,float,float);using Step=void(__thiscall*)(void*,float);
    auto tick=reinterpret_cast<Tick>(image_base178+0x325c50);
    auto move=reinterpret_cast<Step>(image_base178+0x1362030);
    auto turn=reinterpret_cast<Step>(image_base178+0x3248f0);
    unsigned cases=0;double reference_yaw=0;
    for(int fps:{30,60,120,240})for(int hz:{30,60,125,250}){
        Camera178 camera;float native=1.f/fps;tick(camera.data.data(),native,0);assert(camera.get(0x140)==native);
        yaw178=0;
        for(int i=0;i<hz;++i){float scale;assert(deck178::camera_ticks(1.f/hz,native,scale));move(camera.data.data(),scale);turn(camera.data.data(),scale);}
        assert(std::fabs(camera.get(0x10)-20.f)<.001f);
        if(!reference_yaw)reference_yaw=yaw178;else assert(std::fabs(yaw178-reference_yaw)<.0001);
        ++cases;
    }
    unsigned zoom_cases=0;
    for(int fps:{30,60,120,240})for(int hz:{30,60,125,250})for(int direction:{-1,1}){
        Camera178 camera;camera.set(0x5c,1.f);camera.set(0x180,50.f);
        float native=1.f/fps;tick(camera.data.data(),native,0);
        auto zoom=reinterpret_cast<Step>(image_base178+(direction>0?0x1362230:0x1361fd0));
        float previous=0;
        for(int i=0;i<hz;++i){float scale;assert(deck178::camera_ticks(1.f/hz,native,scale));zoom(camera.data.data(),scale);
            float next=camera.get(0xc);assert(direction>0?next>previous:next<previous);previous=next;}
        assert(std::fabs(camera.get(0xc)-direction*20.f)<.003f);++zoom_cases;
    }
    std::cout<<"PASS battle194 continuous native zoom: "<<zoom_cases<<" rates/directions; every update moves, original native distance/rate providers; no wheel events\n";
    unsigned ramp_cases=0;
    for(int fps:{30,60,120,240})for(int hz:{30,60,125,250})for(int direction:{-1,1}){
        Camera178 camera;camera.set(0x5c,1.f);camera.set(0x180,50.f);
        float native=1.f/fps;tick(camera.data.data(),native,0);deck195::ShoulderZoom ramp;
        auto zoom=reinterpret_cast<Step>(image_base178+(direction>0?0x1362230:0x1361fd0));
        float previous=0;
        for(int i=0;i<hz;++i){float scale;assert(deck178::camera_ticks(1.f/hz,native,scale));
            zoom(camera.data.data(),scale*ramp.step(1,1.f/hz));
            float next=camera.get(0xc);assert(direction>0?next>previous:next<previous);previous=next;}
        assert(std::fabs(camera.get(0xc)-direction*12.4f)<.003f);++ramp_cases;
    }
    std::cout<<"PASS zoom195 native ramp: "<<ramp_cases<<" rate/direction combinations;450ms ramp,80percent peak, every update moves\n";
    Camera178 old_camera;tick(old_camera.data.data(),1.f/60,0);yaw178=0;
    for(int i=0;i<125;++i){move(old_camera.data.data(),2.5f*4.f/125);turn(old_camera.data.data(),2.5f/125);}
    assert(old_camera.get(0x10)<4.f&&old_camera.get(0x10)>3.f&&std::fabs(yaw178)<std::fabs(reference_yaw)*.05);
    assert(VirtualProtect(target,5,PAGE_EXECUTE_READWRITE,&protection));memcpy(target,saved,5);FlushInstructionCache(GetCurrentProcess(),target,5);assert(VirtualProtect(target,5,protection,&ignored));
    assert(deck175::native_guards(image_base178));FreeLibrary(mapped);
    std::cout<<"PASS camera178 original native setter/move/turn: "<<cases<<" rate combinations; old slowdown reproduced; synthetic providers only\n";
}
