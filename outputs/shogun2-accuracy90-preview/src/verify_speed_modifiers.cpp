#include "../../../common/fixture_paths.h"
#include "identity.h"
#include "movement_math.h"
#include <array>
static std::vector<unsigned char> code_at(const std::vector<unsigned char>& image,unsigned rva,unsigned size){
    auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(image.data());
    auto nt=reinterpret_cast<const IMAGE_NT_HEADERS32*>(image.data()+dos->e_lfanew);
    auto sections=IMAGE_FIRST_SECTION(nt);
    for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i){auto& s=sections[i];
        if(rva>=s.VirtualAddress&&rva+size<=s.VirtualAddress+s.SizeOfRawData){unsigned off=s.PointerToRawData+rva-s.VirtualAddress;
            require(off+size<=image.size(),"Native fixture bounds");return {image.begin()+off,image.begin()+off+size};}}
    throw std::runtime_error("Native fixture section absent");
}
static void relocate(unsigned char* bytes,unsigned size,unsigned original,uintptr_t replacement){
    unsigned count=0;
    for(unsigned i=0;i+4<=size;++i){unsigned address=0;memcpy(&address,bytes+i,4);
        if(address==original){memcpy(bytes+i,&replacement,4);++count;i+=3;}}
    require(count>0,"Native fixture relocation absent");
}
int main(){void* memory=nullptr;try{
    std::wstring path=fixture_paths::engine();
    require(hash_file(path)==EXPECTED_HASH,"Native movement fixture requires verified engine");
    std::ifstream stream(path,std::ios::binary);std::vector<unsigned char> image((std::istreambuf_iterator<char>(stream)),{});
    memory=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);require(memory!=nullptr,"Fixture allocation");
    auto bytes=static_cast<unsigned char*>(memory);
    auto speed=code_at(image,0x3aa718,0x55),grid=code_at(image,0x3aa4cd,0x7f);
    // ECX=synthetic actor, stack arg=target/previous/scaled acceleration.
    const unsigned char prefix[]={0x53,0x83,0xec,0x30,0x8b,0xd9,0x8b,0x44,0x24,0x38,
        0xf3,0x0f,0x10,0x18,0xf3,0x0f,0x10,0x50,4,0xf3,0x0f,0x10,0x60,8};
    memcpy(bytes,prefix,sizeof prefix);memcpy(bytes+sizeof prefix,speed.data(),speed.size());
    const unsigned char suffix[]={0xf3,0x0f,0x11,0x13,0x83,0xc4,0x30,0x5b,0xc2,4,0};
    memcpy(bytes+sizeof prefix+speed.size(),suffix,sizeof suffix);
    float timestep=.1f,negative=-.1f;memcpy(bytes+256,&timestep,4);memcpy(bytes+260,&negative,4);
    relocate(bytes+sizeof prefix,unsigned(speed.size()),0x115b2448,reinterpret_cast<uintptr_t>(bytes+256));
    relocate(bytes+sizeof prefix,unsigned(speed.size()),0x115de874,reinterpret_cast<uintptr_t>(bytes+260));
    // Execute the entire original terrain cell lookup, including native float
    // multiplication, x87 conversion, unsigned cap and grid-address calculation.
    const unsigned char grid_prefix[]={0x53,0x56,0x57,0x83,0xec,0x30,0x8b,0x54,0x24,0x40,
        0xf3,0x0f,0x10,2,0xf3,0x0f,0x10,0x72,4,0xf3,0x0f,0x11,0x74,0x24,0x20,
        0xc7,0x44,0x24,0x18,0,0,0,0xbf,0x89,0x4c,0x24,0x28,0x8b,0xc1};
    const unsigned char grid_suffix[]={0x83,0xc4,0x30,0x5f,0x5e,0x5b,0xc2,4,0};
    memcpy(bytes+512,grid_prefix,sizeof grid_prefix);memcpy(bytes+512+sizeof grid_prefix,grid.data(),grid.size());
    memcpy(bytes+512+sizeof grid_prefix+grid.size(),grid_suffix,sizeof grid_suffix);
    DWORD old=0;require(VirtualProtect(memory,4096,PAGE_EXECUTE_READ,&old)!=0,"Fixture protection");FlushInstructionCache(GetCurrentProcess(),memory,4096);
    std::array<unsigned char,0x108> actor{};
    using Step=void(__thiscall*)(void*,const float*);auto step=reinterpret_cast<Step>(bytes);
    unsigned comparisons=0;
    for(float previous:{0.0f,.5f,2.1f,4.25f,6.0f})for(float target:{0.0f,.2f,1.0f,2.1f,4.25f,6.0f})
        for(float accel:{0.0f,1.5f,3.0f,6.0f})for(float decel:{0.0f,3.0f,6.0f}){
            *reinterpret_cast<float*>(actor.data()+0x104)=decel;
            float input[3]={target,previous,accel};step(actor.data(),input);
            require(*reinterpret_cast<float*>(actor.data())==native_speed_step(previous,target,accel,decel,.1f,-.1f),"Native speed-step SSE differs");
            ++comparisons;
        }
    std::array<unsigned char,0xb0> common{},terrain{};std::array<unsigned,64> cells{};
    for(unsigned i=0;i<cells.size();++i)cells[i]=i;
    *reinterpret_cast<uintptr_t*>(common.data()+0xa4)=reinterpret_cast<uintptr_t>(terrain.data());
    *reinterpret_cast<uintptr_t*>(terrain.data())=reinterpret_cast<uintptr_t>(cells.data());
    *reinterpret_cast<unsigned*>(terrain.data()+8)=3;
    *reinterpret_cast<unsigned*>(terrain.data()+0x10)=3;
    *reinterpret_cast<unsigned*>(terrain.data()+0x14)=7;
    using Cell=unsigned(__thiscall*)(void*,const float*);auto cell=reinterpret_cast<Cell>(bytes+512);
    unsigned cell_comparisons=0;
    for(float scale:{.25f,1.0f,2.5f}){
        *reinterpret_cast<float*>(terrain.data()+0x24)=scale;
        for(int i=-4000;i<=4000;++i){float point[2]={float(i)/1000,float(-i)/1500};
            unsigned expected=(native_ground_cell(point[0],scale,3,7)<<3)+native_ground_cell(point[1],scale,3,7);
            require(cell(common.data(),point)==expected,"Native terrain cell lookup differs");++cell_comparisons;}
    }
    require(native_surface_column(true,1,false,false)==0xc&&native_surface_column(true,2,false,false)==0x10&&
        native_surface_column(false,0,true,false)==0x14&&native_surface_column(false,0,false,true)==0x18&&
        native_surface_column(true,0,true,true)==0,"Native surface column precedence");
    // Deliberate stop and walk/run transitions exercise braking rather than only
    // algebraic equality. A release must settle at zero in finite native steps.
    float moving=4.25f;unsigned ticks=0;
    while(moving>0&&ticks<20){moving=native_speed_step(moving,0,3,6,.1f,-.1f);++ticks;}
    require(moving==0&&ticks==8,"Native braking fails to stop");
    require(native_speed_step(0,4.25f,3,6,.1f,-.1f)==.3f,"Native start acceleration");
    // A direction change starts from the velocity in that direction.
    for(const auto direction:std::array<std::array<float,2>,4>{{{1,0},{0,1},{-1,0},{0,-1}}}){
        const auto x=direction[0],z=direction[1];
        require(directed_speed95(6.f,x*6.f,z*6.f,x,z)==6.f,"Aligned run speed lost");
        require(directed_speed95(6.f,-x*6.f,-z*6.f,x,z)==0.f,"Opposite run speed carried into reversal");
        require(directed_speed95(6.f,z*6.f,-x*6.f,x,z)==0.f,"Perpendicular run speed carried into strafe");
        require(native_speed_step(directed_speed95(6.f,-x*6.f,-z*6.f,x,z),2.f,3.f,6.f,.1f,-.1f)==.3f,
            "Reversal skipped native acceleration");
    }
    VirtualFree(memory,0,MEM_RELEASE);memory=nullptr;
    std::cout<<"PASS: "<<comparisons<<" original SSE speed-step comparisons and "<<cell_comparisons
        <<" original terrain-grid lookups; braking and category precedence verified; game untouched.\n";return 0;
}catch(const std::exception& e){if(memory)VirtualFree(memory,0,MEM_RELEASE);std::cerr<<e.what()<<'\n';return 1;}}
