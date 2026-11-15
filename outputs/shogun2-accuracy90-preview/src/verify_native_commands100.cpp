#include "../../../common/fixture_paths.h"
// Executes production adapter guards/handlers with bounded original native
// constructors, serializers, scene-query glue and skin cache code. Geometry,
// allocation/vector primitives and event providers are isolated fixture stubs.
#include "adapter.cpp"
#include <thread>
struct Packet100 {unsigned key{},integer_count{},byte_count{},short_count{},float_count{};unsigned integers[8]{},bytes[8]{},shorts[4]{};float floats[4]{};MovePoint100 point{};bool has_point{};};
struct Writer100 {unsigned char* packet;unsigned size,key,index;};
static std::vector<Packet100> packets100;
static bool run_allowed102=true;
static bool __fastcall native_run102(void*,void*){return run_allowed102;}
static unsigned group_cleanups100,context_cleanups100,sorts100,queries100,frees100,native_releases100,release_stage100;
static unsigned char native_cursor_hit100[60]{},native_cursor_copy100[60]{};static unsigned native_cursor_frees100;
static std::vector<UINT> native_messages100;static std::vector<unsigned> native_events100;
static void* __cdecl cursor_alloc100(unsigned size){require(size==60,"Unexpected fixture cursor allocation");return native_cursor_copy100;}
static void* __fastcall cursor_copy100(void* out,void*,const void* in){memcpy(out,in,60);return out;}
static void __cdecl cursor_free100(void* block,unsigned size){require(block==native_cursor_copy100&&size==60,"Foreign cursor cleanup");++native_cursor_frees100;}
static void __fastcall cursor_preview100(void*,void*,uintptr_t){}
static unsigned ui_restores151;
static void __fastcall native_ui151(void* root,void*,unsigned hidden,unsigned a,unsigned b,unsigned c){
    require(hidden==field<unsigned char>(reinterpret_cast<uintptr_t>(root),0xca0)&&a==1&&b==0&&c==1,"Native UI restore lost CURRENT K state/ABI");++ui_restores151;
}
static void __fastcall cursor_refresh100(void* cursor,void*,unsigned,unsigned,unsigned){field<uintptr_t>(reinterpret_cast<uintptr_t>(cursor),0x24)=reinterpret_cast<uintptr_t>(native_cursor_hit100);}
static LRESULT CALLBACK stock_window100(HWND window,UINT message,WPARAM flags,LPARAM point){
    native_messages100.push_back(message);if(message==WM_CANCELMODE)return 0;
    if(message==WM_RBUTTONDOWN||message==WM_RBUTTONUP||message==WM_RBUTTONDBLCLK)require((flags&(MK_SHIFT|MK_CONTROL))==command_native_modifiers100(),"Native Q lost Shift/Ctrl modifiers");
    RECT client{};GetClientRect(window,&client);
    require(LOWORD(point)==client.right/2&&HIWORD(point)==client.bottom/2,"Q target left the visible first-person aim point");
    unsigned type=reinterpret_cast<NativeMouseDecode100>(base+0xbd4a10)(window,message,flags,point,client.right,client.bottom,0);native_events100.push_back(type);
    if(type==5||type==4||type==12||type==7||type==6){auto root=field<uintptr_t>(base,0x1bc8180);reinterpret_cast<NativeCursorEvent100>(base+0x2f7350)(reinterpret_cast<void*>(root+0x10c),&type,0,0,1);}
    return 0;
}

static LRESULT CALLBACK modal_window214(HWND,UINT message,WPARAM,LPARAM){native_messages100.push_back(message);return 0;}
static void __fastcall modal_visible214(void* widget,void*,unsigned visible){field<unsigned char>(reinterpret_cast<uintptr_t>(widget),0xf5)=static_cast<unsigned char>(visible);}
static float scene_origin102[3]{};
static uintptr_t fixture_group100,fixture_world_one100,fixture_world_two100;
static unsigned scene_object100[4]{};static unsigned char scene_hits100[28]{};
static void append100(Writer100* writer,const void* bytes,unsigned count){
    require(writer->size+count<QUEUE_CAPACITY,"Fixture writer overflow");memcpy(writer->packet+writer->size,bytes,count);writer->size+=count;
    writer->packet[0]=static_cast<unsigned char>(writer->size);writer->packet[1]=static_cast<unsigned char>(writer->size>>8);memcpy(writer->packet+QUEUE_CAPACITY,&writer->size,4);
}
static void* __fastcall context100(void* memory,void*,void* packet,unsigned key,unsigned flags){
    auto writer=static_cast<Writer100*>(memory);*writer={static_cast<unsigned char*>(packet),7,key,unsigned(packets100.size())};
    Packet100 capture{};capture.key=key;packets100.push_back(capture);writer->packet[0]=7;writer->packet[1]=0;
    for(unsigned i=0;i<4;++i)writer->packet[2+i]=static_cast<unsigned char>(key>>(24-i*8));writer->packet[6]=static_cast<unsigned char>(flags);memcpy(writer->packet+QUEUE_CAPACITY,&writer->size,4);return memory;
}
static void __fastcall destroy_context100(void*,void*){++context_cleanups100;}
static void __fastcall destroy_group100(void* group,void*){require(*static_cast<uintptr_t*>(group)==base+0x15c4da0,"Destructor received wrong group");++group_cleanups100;}
static void* __fastcall copy_vector100(void* out,void*,const void* in){memcpy(out,in,12);return out;}
static void* __cdecl write_group100(void* context,const void* group){
    auto pointer=reinterpret_cast<uintptr_t>(group);require(field<uintptr_t>(pointer,0)==base+0x15c4da0&&field<unsigned>(pointer,0x14)==1,"Native serializer group identity differs");
    auto writer=static_cast<Writer100*>(context);unsigned id=7;append100(writer,&id,4);return context;
}
static void* __cdecl write_point100(void* context,const void* point){
    auto writer=static_cast<Writer100*>(context);auto& capture=packets100[writer->index];memcpy(&capture.point,point,20);capture.has_point=true;
    append100(writer,point,12);append100(writer,static_cast<const unsigned char*>(point)+16,4);return context;
}
static void* __cdecl write_uint100(void* context,unsigned value){auto writer=static_cast<Writer100*>(context);auto& c=packets100[writer->index];c.integers[c.integer_count++]=value;append100(writer,&value,4);return context;}
static void* __cdecl write_byte100(void* context,unsigned char value){auto writer=static_cast<Writer100*>(context);auto& c=packets100[writer->index];c.bytes[c.byte_count++]=value;append100(writer,&value,1);return context;}
static void* __cdecl write_short100(void* context,unsigned short value){auto writer=static_cast<Writer100*>(context);auto& c=packets100[writer->index];c.shorts[c.short_count++]=value;append100(writer,&value,2);return context;}
static void* __cdecl write_float100(void* context,float value){auto writer=static_cast<Writer100*>(context);auto& c=packets100[writer->index];c.floats[c.float_count++]=value;append100(writer,&value,4);return context;}
static uintptr_t __fastcall cell100(void*,void*,const float*,unsigned){return 1;}
static float __fastcall ground100(void*,void*,const float* xz){return xz[0]*.1f+xz[1]*.2f;}
static void __fastcall world_one100(void* world,void*,void* result,const void* shape,unsigned mode,unsigned sorted,unsigned mask,unsigned visible){
    require(reinterpret_cast<uintptr_t>(world)==fixture_world_one100&&mode==0&&sorted==1&&mask==0x303,"Native scene-query receiver/arguments differ");
    require(*static_cast<const unsigned*>(shape)==3,"Placement ray is not a native segment");(void)visible;
    auto ray=static_cast<const float*>(shape);memcpy(scene_origin102,ray+1,12);require(std::abs(std::hypot(std::hypot(ray[4]-ray[1],ray[5]-ray[2]),ray[6]-ray[3])-4096.f)<.01f,"Native ray extent differs");
    auto vector=static_cast<unsigned*>(result);vector[0]=vector[1]=1;vector[2]=reinterpret_cast<unsigned>(scene_hits100);++queries100;
}
static void __fastcall world_two100(void* world,void*,void*,const void*,unsigned,unsigned,unsigned,unsigned,unsigned){require(reinterpret_cast<uintptr_t>(world)==fixture_world_two100,"Native second scene provider differs");}
static void __cdecl sort100(void* start,void* end,unsigned count,unsigned mode){require(start==scene_hits100&&reinterpret_cast<uintptr_t>(end)-reinterpret_cast<uintptr_t>(start)==28&&count==1&&mode==1,"Native ray result stride/sort differs");++sorts100;}
static void __cdecl free100(void* block){require(block==scene_hits100,"Native query freed foreign memory");++frees100;}
static bool __fastcall gate100(void*,void*){return true;}
static void __fastcall release100(void* task,void*){
    ++native_releases100;auto bytes=static_cast<unsigned char*>(task);
    if(release_stage100==1)bytes[0x6c]=1;
    else if(release_stage100==2)bytes[0x6d]=bytes[0x6e]=1;
}
struct NativeFixture100 {
    void* image{};Status status{};
    std::vector<unsigned char> root=std::vector<unsigned char>(0x1800),env=std::vector<unsigned char>(0x28300),common=std::vector<unsigned char>(0x200),phase=std::vector<unsigned char>(0x2900),
        manager=std::vector<unsigned char>(0x910),point_manager=std::vector<unsigned char>(0x100),camera=std::vector<unsigned char>(0x200),view=std::vector<unsigned char>(0x40),
        holder=std::vector<unsigned char>(0x210),peer_holder=std::vector<unsigned char>(0x210),selected=std::vector<unsigned char>(0x500),owner=std::vector<unsigned char>(0x200),
        config=std::vector<unsigned char>(0x100),actor=std::vector<unsigned char>(0x700),peer=std::vector<unsigned char>(0x700),actor_unit=std::vector<unsigned char>(0x1800),
        queue=std::vector<unsigned char>(QUEUE_CAPACITY+8),world_one=std::vector<unsigned char>(32),world_two=std::vector<unsigned char>(32);
    std::array<uintptr_t,2> holders{};std::array<uintptr_t,1> units{};unsigned char descriptors[7*28]{};
    static uintptr_t ptr(std::vector<unsigned char>& v){return reinterpret_cast<uintptr_t>(v.data());}
    void jump(unsigned address,void* destination){auto code=reinterpret_cast<unsigned char*>(base+address);code[0]=0xe9;unsigned delta=unsigned(reinterpret_cast<uintptr_t>(destination)-base-address-5);memcpy(code+1,&delta,4);}
    NativeFixture100(){
        const std::wstring path=fixture_paths::engine();require(hash_file(path)==EXPECTED_HASH,"Exact engine required");
        std::ifstream file(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});
        auto dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(bytes.data());auto nt=reinterpret_cast<const IMAGE_NT_HEADERS32*>(bytes.data()+dos->e_lfanew);
        image=VirtualAlloc(nullptr,nt->OptionalHeader.SizeOfImage,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);require(image!=nullptr,"Private native fixture allocation");base=reinterpret_cast<uintptr_t>(image);
        memcpy(image,bytes.data(),nt->OptionalHeader.SizeOfHeaders);auto sections=IMAGE_FIRST_SECTION(nt);
        for(unsigned i=0;i<nt->FileHeader.NumberOfSections;++i){const auto& section=sections[i];if(section.SizeOfRawData)memcpy(reinterpret_cast<void*>(base+section.VirtualAddress),bytes.data()+section.PointerToRawData,section.SizeOfRawData);}
        auto reloc=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];unsigned cursor=0;
        while(cursor<reloc.Size){auto block=reinterpret_cast<const IMAGE_BASE_RELOCATION*>(base+reloc.VirtualAddress+cursor);require(block->SizeOfBlock>=8,"Fixture relocation bounds");
            auto entries=reinterpret_cast<const unsigned short*>(block+1);unsigned count=(block->SizeOfBlock-8)/2;
            for(unsigned i=0;i<count;++i)if((entries[i]>>12)==IMAGE_REL_BASED_HIGHLOW)field<unsigned>(base,block->VirtualAddress+(entries[i]&4095))+=unsigned(base-nt->OptionalHeader.ImageBase);
            cursor+=block->SizeOfBlock;
        }
        // Validate pristine mapped native ranges before this fixture replaces
        // serializer/terrain endpoints with explicit providers.
        require(native214::admit(base),"Pristine QoL native dependencies differ in fixture");
        jump(0x15cb80,reinterpret_cast<void*>(&native_run102));
        jump(0x10f1060,reinterpret_cast<void*>(&native_ui151)); // Native UI tree is an explicit provider.
        field<float>(base,0x18d1c80+0x5c)=1;field<float>(base,0x18d1ce0+0x5c)=1;
        jump(0x10ac90,reinterpret_cast<void*>(&copy_vector100));jump(0x1cc200,reinterpret_cast<void*>(&context100));jump(0x1d0d40,reinterpret_cast<void*>(&destroy_context100));jump(0x1d10f0,reinterpret_cast<void*>(&destroy_group100));
        jump(0x342930,reinterpret_cast<void*>(&write_group100));jump(0x285990,reinterpret_cast<void*>(&write_point100));jump(0x2859e0,reinterpret_cast<void*>(&write_uint100));jump(0x285a00,reinterpret_cast<void*>(&write_uint100));
        jump(0x285d00,reinterpret_cast<void*>(&write_byte100));jump(0x285ce0,reinterpret_cast<void*>(&write_short100));jump(0x285a20,reinterpret_cast<void*>(&write_float100));
        jump(0x3f6290,reinterpret_cast<void*>(&cell100));jump(0x3ea8a0,reinterpret_cast<void*>(&ground100));jump(0x36e8f0,reinterpret_cast<void*>(&world_one100));jump(0x36e7b0,reinterpret_cast<void*>(&world_two100));jump(0x101a80,reinterpret_cast<void*>(&sort100));jump(0xf04d0,reinterpret_cast<void*>(&free100));
        jump(0x1101da0,reinterpret_cast<void*>(&cursor_preview100));jump(0x335260,reinterpret_cast<void*>(&cursor_refresh100));jump(0xf7440,reinterpret_cast<void*>(&cursor_alloc100));jump(0x252bf0,reinterpret_cast<void*>(&cursor_copy100));jump(0x14fbc77,reinterpret_cast<void*>(&cursor_free100));
        field<unsigned>(reinterpret_cast<uintptr_t>(native_cursor_hit100),4)=1;field<unsigned char>(base,0x1b3116b)=1;
        const unsigned char routing[]={0x31,0xc0,0xc3};memcpy(reinterpret_cast<void*>(base+0x1821e0),routing,sizeof routing);
        DWORD previous{};require(VirtualProtect(image,0x1580000,PAGE_EXECUTE_READ,&previous)!=0,"Fixture code protection");FlushInstructionCache(GetCurrentProcess(),image,0x1580000);
        state=&status;enabled=1;possessed=ptr(actor);environment=ptr(env);unit=ptr(actor_unit);state->mode=POSSESSED;state->pid=GetCurrentProcessId();state->window_thread=GetCurrentThreadId();
        field<uintptr_t>(ptr(root),0)=base+0x170d284;field<uintptr_t>(ptr(root),0x18)=base+0x170d290;field<uintptr_t>(ptr(root),0x10c)=base+0x170d304;
        field<uintptr_t>(base,0x1bc8180)=ptr(root);field<uintptr_t>(ptr(root),0x110)=environment;field<uintptr_t>(environment,8)=ptr(common);
        field<uintptr_t>(ptr(common),0xb4)=ptr(phase);field<uintptr_t>(ptr(phase),0)=environment;field<unsigned>(ptr(phase),0x14)=6;
        field<uintptr_t>(environment,0x281fc)=ptr(manager);field<uintptr_t>(ptr(manager),0x8e0)=ptr(camera);field<uintptr_t>(ptr(manager),0x8fc)=possessed;
        field<uintptr_t>(ptr(common),0xb8)=ptr(point_manager);field<uintptr_t>(ptr(point_manager),0)=environment;
        field<uintptr_t>(ptr(common),0xc0)=ptr(world_one);field<uintptr_t>(ptr(common),0xc4)=ptr(world_two);fixture_world_one100=ptr(world_one);fixture_world_two100=ptr(world_two);
        field<uintptr_t>(ptr(camera),0)=base+0x15d22e8;field<uintptr_t>(ptr(camera),0x180)=ptr(manager);field<uintptr_t>(ptr(camera),0x188)=possessed;field<uintptr_t>(ptr(camera),0x184)=ptr(view);
        field<unsigned>(ptr(view),0x18)=2;holders={ptr(holder),ptr(peer_holder)};field<uintptr_t>(ptr(view),0x1c)=reinterpret_cast<uintptr_t>(holders.data());field<uintptr_t>(ptr(holder),8)=possessed;field<uintptr_t>(ptr(peer_holder),8)=ptr(peer);
        field<uintptr_t>(environment,0x2826c)=ptr(selected);field<uintptr_t>(ptr(selected),0x4a8)=ptr(owner);fixture_group100=ptr(owner)+0xb0;
        units={unit};field<uintptr_t>(fixture_group100,0)=base+0x15c4da0;field<unsigned>(fixture_group100,0x10)=field<unsigned>(fixture_group100,0x14)=1;field<uintptr_t>(fixture_group100,0x18)=reinterpret_cast<uintptr_t>(units.data());
        for(auto a:{possessed,ptr(peer)}){field<uintptr_t>(a,0)=base+0x15dfec8;field<unsigned char>(a,0xf0)=field<unsigned char>(a,0xa1)=1;field<uintptr_t>(a,0x214)=unit;field<uintptr_t>(a,0x28)=environment;field<uintptr_t>(a,0x20c)=ptr(config);}
        field<unsigned>(ptr(peer),0x1c8)=12;field<unsigned>(unit,4)=7;field<uintptr_t>(unit,0xf9c)=ptr(config);field<unsigned char>(unit,0xeb0)=1;
        require(followed(possessed,true),"Production current lease fixture invalid");state->lease_generation=possession_lease.generation;state->actor_id=0;state->unit_id=7;
        unsigned offsets[]={0x192e0d0,0x192da04,0x192d014,0x192eaac,0x192ca68,0x192e90c,0x192d2cc};
        for(unsigned i=0;i<7;++i){unsigned key=0x10203001+i;field<unsigned>(base,offsets[i])=key;memcpy(descriptors+i*28,&key,4);descriptors[i*28+16]=3;}
        field<uintptr_t>(base,0x18d9000)=reinterpret_cast<uintptr_t>(descriptors);field<unsigned>(base,0x18d9004)=7;field<uintptr_t>(environment,0x28048)=ptr(queue);
        scene_object100[0]=1;field<uintptr_t>(reinterpret_cast<uintptr_t>(scene_hits100),0)=reinterpret_cast<uintptr_t>(scene_object100);field<float>(reinterpret_cast<uintptr_t>(scene_hits100),4)=10;field<float>(reinterpret_cast<uintptr_t>(scene_hits100),8)=0;field<float>(reinterpret_cast<uintptr_t>(scene_hits100),12)=20;
        original_draw_gate=reinterpret_cast<RangedGate>(&gate100);original_native_release=reinterpret_cast<RangedAction>(&release100);original_skin_ready=reinterpret_cast<SkinReady>(base+0x4c56d0);
        command_generation100=possession_lease.generation;command_fire_mode100=LONG(command100::FireMode::AtWill);
    }
    ~NativeFixture100(){enabled=0;command_reset100();if(command_shared100){UnmapViewOfFile(command_shared100);command_shared100=nullptr;}if(command_mapping100){CloseHandle(command_mapping100);command_mapping100=nullptr;}state=nullptr;possession_lease.release();possessed=environment=unit=0;if(image)VirtualFree(image,0,MEM_RELEASE);base=0;}
    void clear_queue(){field<unsigned>(ptr(queue)+4,QUEUE_CAPACITY)=0;packets100.clear();}
};

static LONG WINAPI fault100(EXCEPTION_POINTERS* exception){std::cerr<<"Fixture fault code="<<std::hex<<exception->ExceptionRecord->ExceptionCode<<" rva="<<(reinterpret_cast<uintptr_t>(exception->ExceptionRecord->ExceptionAddress)-base)<<'\n';return EXCEPTION_EXECUTE_HANDLER;}
int main(){SetUnhandledExceptionFilter(&fault100);try{
    NativeFixture100 fixture;using P=NativeFixture100;
    require(followed(possessed,false)&&selected_possessed_group()==fixture_group100,"Actual production ownership/group guard failed");
    require(command_native_signatures100(base),"Exact native command signatures differ");
    MovePoint100 destination{{10,3,20},0,0};unsigned before{},after{},calls=0;
    for(unsigned facing:{0u,16384u,32768u,65535u})for(float width:{0.f,1.f,25.f,80.f})for(unsigned repeat=0;repeat<100;++repeat){
        fixture.clear_queue();__asm mov before,esp
        require(command_serialize100(&destination,facing,width),"Native facing/width packet refused");
        __asm mov after,esp
        require(before==after,"Native formation ABI stack imbalance");const auto& packet=packets100.back();
        require(packet.has_point&&packet.point.position[0]==10&&packet.point.position[1]==3&&packet.point.position[2]==20&&packet.short_count==1&&packet.shorts[0]==facing,"Native formation position/heading differs");
        if(width>0)require(packet.float_count==1&&packet.floats[0]==width&&packet.integer_count==2&&packet.integers[0]==0&&packet.integers[1]==9&&packet.byte_count==2,"Native width/move-speed/queue fields differ");
        else require(packet.float_count==0&&packet.byte_count==2&&packet.bytes[0]==0&&packet.bytes[1]==0,"Native preserved-width orientation flags differ");
        ++calls;
    }
    for(bool run:{false,true})for(unsigned repeat=0;repeat<100;++repeat){
        fixture.clear_queue();unsigned stack_before{},stack_after{};__asm mov stack_before,esp
        require(command_serialize100(&destination,1234,0,-1,false,UnitState100::Fire,nullptr,nullptr,run),"Native gait packet refused");
        __asm mov stack_after,esp
        require(stack_before==stack_after&&packets100.back().bytes[0]==unsigned(run)&&packets100.back().bytes[1]==0,"Original move serializer changed run or queue flag/ABI");
        fixture.clear_queue();require(command_hold_here100(destination,1234,run),"Native Hold Here gait refused");
        require(packets100[0].bytes[0]==unsigned(run)&&packets100[0].bytes[1]==0&&packets100[1].bytes[0]==1,"Hold gait lost guard or changed queue semantics");
    }
    fixture.clear_queue();require(command_serialize100(&destination,0,0,-1,true),"Native point move refused");require(packets100.back().short_count==0&&packets100.back().float_count==0,"Point move changed native facing or width");
    for(unsigned mode:{0u,1u}){fixture.clear_queue();unsigned char native_before=field<unsigned char>(unit,0xeb0);require(command_serialize100(nullptr,0,0,int(mode)),"Native fire policy packet refused");
        require(packets100.back().integer_count==1&&packets100.back().integers[0]==mode&&field<unsigned char>(unit,0xeb0)==native_before,"Fire policy bypassed native dispatch or changed enum");}
    state->heartbeat=GetTickCount64();
    require(camera_owned(P::ptr(fixture.camera)),"Skirmish fixture lacks a current camera lease");
    for(bool deck:{false,true})for(unsigned active:{0u,1u}){
        process_orders(P::ptr(fixture.camera));
        fixture.clear_queue();field<unsigned char>(unit,0x179)=1;field<unsigned char>(unit,0xeb2)=active;
        if(deck)require(command_window_fire62(command100::Input::Skirmish),"Possession L5 order refused");
        else {
            require(command_window100(nullptr,WM_KEYDOWN,'R',0),"Skirmish press refused");
            command_fire_press_tick62=GetTickCount64()-400;
            require(command_window100(nullptr,WM_KEYDOWN,'R',1L<<30)&&!command_write100,"Skirmish repeated while held");
            require(command_window100(nullptr,WM_KEYUP,'R',0),"Skirmish release refused");
        }
        process_orders(P::ptr(fixture.camera));
        require(packets100.size()==1&&packets100[0].key==0x10203007&&packets100[0].byte_count==1&&
            packets100[0].bytes[0]==!active&&field<unsigned char>(unit,0xeb2)==active,"Skirmish used the wrong packet/state");
        auto feedback=active?command100::Feedback::SkirmishOff:command100::Feedback::SkirmishOn;
        require(command_shared100&&command_shared100->feedback==unsigned(feedback)&&
            command100::valid(*command_shared100,state->pid,state->lease_generation,state->actor_id,state->unit_id,GetTickCount64())&&
            command_shared100->fire_mode==unsigned(command100::FireMode::AtWill),"Skirmish acknowledgement invalidated the fire mode in the HUD");
        command_input_reset100();
    }
    process_orders(P::ptr(fixture.camera));
    fixture.clear_queue();field<unsigned char>(unit,0x179)=0;
    require(command_window100(nullptr,WM_KEYDOWN,'R',0),"R press refused");
    command_fire_press_tick62=GetTickCount64()-400;
    command_window100(nullptr,WM_KEYUP,'R',0);process_orders(P::ptr(fixture.camera));
    require(packets100.empty(),"Ineligible unit received Skirmish");
    command_input_reset100();
    command100::Event fire62{};
    command_window100(nullptr,WM_KEYDOWN,'R',0);command_window100(nullptr,WM_KEYUP,'R',0);
    require(command_pop100(fire62)&&fire62.input==command100::Input::Fire&&!command_pop100(fire62),"R tap did not enqueue exactly one fire order");
    command_window100(nullptr,WM_KEYDOWN,'R',0);command_window100(nullptr,WM_KILLFOCUS,0,0);command_window100(nullptr,WM_KEYUP,'R',0);
    require(!command_pop100(fire62),"Focus loss committed an R order");
    std::cout<<"PASS possession62: R tap/hold, two native skirmish packets and cancellation\n";
    std::cout<<"PASS possession63: both Skirmish acknowledgements preserve the HUD sample\n";
    std::cout<<"PASS possession64: Deck Skirmish uses the native on/off packets and HUD acknowledgements\n";
    fixture.clear_queue();require(command_hold_here100(destination,16384),"Hold + native guard transaction refused");
    require(packets100.size()==2&&packets100[0].shorts[0]==16384&&packets100[1].byte_count==1&&packets100[1].bytes[0]==1,"Native guard-mode packet wrong");
    unsigned committed=field<unsigned>(P::ptr(fixture.queue)+4,QUEUE_CAPACITY);require(committed>0,"Hold transaction never published");
    fixture.clear_queue();field<unsigned>(P::ptr(fixture.queue)+4,QUEUE_CAPACITY)=QUEUE_CAPACITY-1;
    require(!command_hold_here100(destination,0)&&field<unsigned>(P::ptr(fixture.queue)+4,QUEUE_CAPACITY)==QUEUE_CAPACITY-1,"Full queue published a partial hold/guard order");
    fixture.clear_queue();unsigned contexts=context_cleanups100,groups=group_cleanups100;
    field<unsigned>(fixture_group100,0x14)=2;require(!command_serialize100(&destination,0,1)&&context_cleanups100==contexts&&group_cleanups100==groups,"Foreign/multiple selection constructed an order");field<unsigned>(fixture_group100,0x14)=1;
    unsigned serial=field<unsigned>(possessed,0x1c8);field<unsigned>(possessed,0x1c8)=serial+1;
    require(!command_serialize100(&destination,0,1),"Reused owned actor serial issued a native command");field<unsigned>(possessed,0x1c8)=serial;
    ViewRay ray{{0,2,0},{0,-1,0}};MovePoint100 picked{};
    require(command_pick100(ray,picked)&&picked.position[0]==10&&picked.position[1]==5&&picked.position[2]==20,"Native scene pick did not retain native terrain position");
    require(queries100==1&&sorts100==1&&frees100==1,"Native query vector/sort cleanup differs");
    scene_object100[0]=8;require(!command_pick100(ray,picked)&&frees100==2,"Actor hit became a move target or native result leaked");scene_object100[0]=1;
    // Actual production same-unit peer authorization, including original forwarding.
    command_fire_mode100=LONG(command100::FireMode::OnCommand);command_volley100.reset();memset(command_permits100,0,sizeof command_permits100);
    unsigned char task[0x90]{};field<uintptr_t>(reinterpret_cast<uintptr_t>(task),4)=P::ptr(fixture.peer);field<unsigned>(reinterpret_cast<uintptr_t>(task),0x78)=100;field<unsigned>(reinterpret_cast<uintptr_t>(task),0x88)=130;
    command100::ActorKey peer_key{};require(command_peer100(P::ptr(fixture.peer),peer_key),"Valid peer did not pass full production ownership proof");
    command_draw_started100(task);require(!weapon_draw_gate(task,nullptr),"Peer was ready before a successful player shot");command_on_shot100();require(weapon_draw_gate(task,nullptr),"Player shot did not release native draw wait");
    release_stage100=0;unsigned native_before=native_releases100;require(command_release_managed100(task)&&native_releases100==native_before+1,"Pre-marker native release not forwarded");
    require(command_volley100.available(peer_key),"Permission consumed before native release marker");
    release_stage100=1;require(command_release_managed100(task)&&!command_volley100.available(peer_key),"First actual marker did not consume one volley");
    release_stage100=2;native_before=native_releases100;require(command_release_managed100(task)&&native_releases100==native_before+1&&task[0x6e]==1,"Second native weapon event was cut short");
    // Reuse the same native task, timestamp and clip: the draw hook's actual
    // generation must still revoke the preceding draw's permission.
    task[0x6c]=task[0x6d]=task[0x6e]=0;command_draw_started100(task);native_before=native_releases100;
    require(command_release_managed100(task)&&native_releases100==native_before&&task[0x6e]==1,"Reused task inherited prior draw authorization");
    task[0x6c]=task[0x6d]=task[0x6e]=0;command_draw_started100(task);command_on_shot100();release_stage100=1;require(command_release_managed100(task)&&task[0x6c],"Second successful player shot did not authorize the next draw");
    command_fire_mode100=LONG(command100::FireMode::AtWill);require(weapon_draw_gate(task,nullptr)&&!command_release_managed100(task),"Fire-at-will native forwarding changed");
    command_fire_mode100=LONG(command100::FireMode::OnCommand);field<uintptr_t>(P::ptr(fixture.peer),0x214)=unit+16;
    require(weapon_draw_gate(task,nullptr)&&!command_release_managed100(task),"Foreign unit firing was intercepted");field<uintptr_t>(P::ptr(fixture.peer),0x214)=unit;
    // Exercise the actual native skin pointer/cache routine around the new
    // visibility pass. No camera/physics cache may receive the collapsed head.
    std::vector<unsigned char> binding(0xc0),asset(0x50),hierarchy(8*20),bones(7*8),render_object(0x20),result(12);
    float palette[96]{},raw[96]{};int32_t parents[8]={-1,0,0,0,3,-1,-1,-1};unsigned indices[7]={4,3,1,2,5,6,7},offsets[7]={0x9c,0xa0,0xa8,0xa4,0x90,0x94,0x98};
    for(unsigned i=0;i<8;++i){palette[i*12]=palette[i*12+5]=palette[i*12+10]=1;palette[i*12+7]=1.6f;field<int32_t>(P::ptr(hierarchy),i*20+16)=parents[i];}palette[4*12+7]=1.8f;memcpy(raw,palette,sizeof raw);
    for(unsigned i=0;i<7;++i){field<unsigned>(P::ptr(bones),i*8+4)=indices[i];field<uintptr_t>(P::ptr(binding),offsets[i])=P::ptr(bones)+i*8;}
    field<uintptr_t>(P::ptr(binding),0x1c)=P::ptr(asset);field<unsigned>(P::ptr(asset),0x3c)=8;field<uintptr_t>(P::ptr(asset),0x40)=P::ptr(hierarchy);
    field<uintptr_t>(P::ptr(fixture.holder),0x1e4)=P::ptr(binding);field<uintptr_t>(P::ptr(fixture.holder),0x14)=P::ptr(render_object);
    field<unsigned>(P::ptr(result),4)=8;field<uintptr_t>(P::ptr(result),8)=reinterpret_cast<uintptr_t>(palette);
    for(unsigned mode:{0u,8u,14u,18u,30u})for(unsigned frame=0;frame<200;++frame){
        field<unsigned>(possessed,0x1f0)=mode;skin_ready_hook(fixture.holder.data(),nullptr,result.data());
        require(std::abs(field<float>(P::ptr(fixture.holder),0x1b0)-1.8f)<1e-6f,"Native cached head position followed the collapsed head");
        require(palette[3*12]==1.f&&palette[4*12]==.0001f,"Self visibility missing or accumulated on non-ranged/paired states");
        for(unsigned bone:{0u,1u,2u,5u,6u,7u})require(!memcmp(palette+bone*12,raw+bone*12,48),"Body/arms/weapon altered by self visibility");
    }
    restore_soldier_pose();require(!memcmp(palette,raw,sizeof raw),"Exact previous self palette was not restored on exit");
    InterlockedExchange(&third_person152,1);skin_ready_hook(fixture.holder.data(),nullptr,result.data());require(!memcmp(palette,raw,sizeof raw),"Third person hid/distorted own full model");InterlockedExchange(&third_person152,0);
    SelfSkin100 cache101{};require(prepare_self_skin100(P::ptr(fixture.holder),P::ptr(result),cache101),"Raw cache snapshot");
    palette[cache101.head*12+7]+=2;palette[cache101.left*12+11]+=3;palette[cache101.right*12+3]-=4;
    float presented101[96];memcpy(presented101,palette,sizeof palette);
    native_skin_cache101(fixture.holder.data(),result.data(),cache101);
    require(std::abs(field<float>(P::ptr(fixture.holder),0x1b0)-raw[cache101.head*12+7])<1e-6f&&
        std::abs(field<float>(P::ptr(fixture.holder),0x1b8)-(raw[cache101.left*12+11]-raw[cache101.right*12+11]))<1e-6f&&
        std::abs(field<float>(P::ptr(fixture.holder),0x1c0)-(raw[cache101.right*12+3]-raw[cache101.left*12+3]))<1e-6f,"Presented arms/head leaked into original native camera caches");
    require(!memcmp(palette,presented101,sizeof palette),"Temporary native-cache capture lost presented socket translations");
    memcpy(palette,raw,sizeof raw);
    require(field<uintptr_t>(P::ptr(render_object),0x14)==P::ptr(result),"Native renderer did not retain its actual palette");
    restore_soldier_pose();require(!memcmp(palette,raw,sizeof raw),"First-person exit did not restore exact native model matrices");
    SelfSkin100 prepared{};require(prepare_self_skin100(P::ptr(fixture.holder),P::ptr(result),prepared)&&current_self_skin100(prepared),"Current self-skin snapshot rejected");
    field<uintptr_t>(P::ptr(binding),0x1c)=P::ptr(asset)+4;require(!current_self_skin100(prepared),"Changed model asset reused a self-skin snapshot");finish_self_skin100(prepared);require(!memcmp(palette,raw,sizeof raw),"Retired model snapshot changed its palette");field<uintptr_t>(P::ptr(binding),0x1c)=P::ptr(asset);
    // Native skin calls for another actor still see their original full model.
    field<uintptr_t>(P::ptr(fixture.peer_holder),0x1e4)=P::ptr(binding);field<uintptr_t>(P::ptr(fixture.peer_holder),0x14)=P::ptr(render_object);
    skin_ready_hook(fixture.peer_holder.data(),nullptr,result.data());require(!memcmp(palette,raw,sizeof raw),"Foreign model was hidden");
    // Q is the stock RMB message path. Run the actual native mouse decoder and
    // cursor press/release cleanup; the window host, world refresh, allocator and
    // snapshot-copy primitives are fixture substitutions. No custom Q packet.
    HWND window=CreateWindowExW(0,L"STATIC",L"Command100 fixture",WS_POPUP,0,0,1280,720,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);require(window!=nullptr,"Hidden input fixture window");
    input_window=window;native_window_proc=&stock_window100;state->heartbeat=GetTickCount64();
    // Reproduce blanket possession UI suppression, then native modal ownership.
    require(native214::admit(base),"QoL native dependencies differ in fixture");
    {unsigned char manager214[0x60]{},widget214[0x328]{};auto manager_at214=reinterpret_cast<uintptr_t>(manager214),widget_at214=reinterpret_cast<uintptr_t>(widget214);
     auto prior_manager214=field<uintptr_t>(base,0x1bc73e4);field<uintptr_t>(base,0x1bc73e4)=manager_at214;
     auto prior_visibility214=original_ui_visible;original_ui_visible=reinterpret_cast<UIVisible>(&modal_visible214);interface_active=1;saved_widget_count=1;saved_widgets[0]={widget_at214,1,0,true};
     ui_visible_hook(widget214,nullptr,1);require(!widget214[0xf5],"Possession native HUD unexpectedly exposed");
     *reinterpret_cast<unsigned*>(manager214+0x30)=1;ui_visible_hook(widget214,nullptr,1);
     require(widget214[0xf5]&&!keyboard_fps84(),"Native pause/results still hidden or keyboard captured");
     auto prior_window214=native_window_proc;native_window_proc=&modal_window214;auto mode214=state->mode;
     weapon_window_proc(window,WM_LBUTTONDOWN,0,0);weapon_window_proc(window,WM_KEYDOWN,VK_ESCAPE,0);
     require(native_messages100.size()==2&&native_messages100[0]==WM_LBUTTONDOWN&&native_messages100[1]==WM_KEYDOWN&&state->mode==mode214&&!state->release,"Native menu input did not pass through while possessed");
     native_window_proc=prior_window214;memset(manager214,0,sizeof manager214);
     for(auto key:{'3','4'}){
         require(speed_key214(window,WM_KEYDOWN,key,0)&&speed_key214(window,WM_KEYDOWN,key,1L<<30)&&
             speed_key214(window,WM_KEYUP,key,0)&&speed_key214(window,WM_CHAR,key,0),"Possession speed key not owned");
         require(!speed_key214(window,WM_SYSKEYDOWN,key,0),"Alt chord owned by speed control");
     }
     require(!speed_key214(window,WM_SYSKEYDOWN,VK_MENU,0)&&!speed_key214(window,WM_KEYDOWN,'2',0),"Unrelated key captured as battle speed");
     *reinterpret_cast<unsigned*>(manager214+0x30)=1;
     require(!speed_key214(window,WM_KEYDOWN,'3',0)&&!speed_key214(window,WM_KEYDOWN,'4',0),"Speed key captured in native menu");
     memset(manager214,0,sizeof manager214);
     state->mode=0;
     require(!speed_key214(window,WM_KEYDOWN,'3',0)&&!speed_key214(window,WM_KEYDOWN,'4',0),"Speed key captured in RTS");
     state->mode=mode214;saved_widgets[0].suppress=false;ui_visible_hook(widget214,nullptr,1);require(widget214[0xf5],"New native results widget suppressed outside modal");
     interface_active=0;saved_widget_count=0;original_ui_visible=prior_visibility214;field<uintptr_t>(base,0x1bc73e4)=prior_manager214;native_messages100.clear();
     std::cout<<"PASS modal214: native pause/results visibility, mouse/keyboard passthrough and retained possession; owning-window provider only\n";
    }
    auto cursor=P::ptr(fixture.root)+0x10c;unsigned packets_before=unsigned(packets100.size());
    require(command_window100(window,WM_KEYDOWN,'Q',0)&&command_window100(window,WM_KEYDOWN,'Q',1L<<30),"Q press/repeat capture differs");
    require(command_placing100&&native_messages100.size()==1&&native_messages100[0]==WM_RBUTTONDOWN&&native_events100[0]==5&&field<unsigned char>(cursor,0x5b)==1,"Q did not enter exact native RMB/cursor state");
    // Use the exact render-camera unprojection and native drag-width pipeline
    // with a flat fixture ground provider. Width is never computed by production.
    auto aim_ground101=[](float yaw,float pitch){
        std::array<unsigned char,0x180> render{};auto a=reinterpret_cast<uintptr_t>(render.data());
        field<float>(a,0x60)=0;field<float>(a,0x64)=2;field<float>(a,0x68)=0;
        field<float>(a,0x15c)=std::sin(yaw)*std::cos(pitch);field<float>(a,0x160)=std::sin(pitch);field<float>(a,0x164)=std::cos(yaw)*std::cos(pitch);
        using Unproject=float*(__thiscall*)(void*,float*,float,float,float);float point[3]{};
        reinterpret_cast<Unproject>(base+0x13660f0)(render.data(),point,0,0,1);
        float distance=-2/(point[1]-2);return MovePoint100{{point[0]*distance,0,point[2]*distance},0,0};
    };
    state->yaw=-.2f;state->pitch=-.3f;auto start101=aim_ground101(state->yaw,state->pitch);
    mounted_heading85=0;float old_yaw=state->yaw,old_pitch=state->pitch;mouse_look101(160,-20,0,.008f);
    auto end101=aim_ground101(state->yaw,state->pitch);
    auto held101=field<uintptr_t>(cursor,0x28),current101=field<uintptr_t>(cursor,0x24);
    field<uintptr_t>(held101,0xc)=reinterpret_cast<uintptr_t>(&start101);field<uintptr_t>(current101,0xc)=reinterpret_cast<uintptr_t>(&end101);
    using NativeWidth101=float(__thiscall*)(void*,unsigned,float,float);
    float width101=reinterpret_cast<NativeWidth101>(base+0x2acfa0)(reinterpret_cast<void*>(cursor),0,0,4096);
    require(end101.position[0]>start101.position[0]&&std::abs(width101-std::hypot(end101.position[0]-start101.position[0],end101.position[2]-start101.position[2]))<.0001f,"Original native left/right drag width disagrees with aimed endpoints");
    field<uintptr_t>(held101,0xc)=field<uintptr_t>(current101,0xc)=0;
    require(state->yaw!=old_yaw&&state->pitch!=old_pitch&&command_placing100,"Q froze or exited first-person mouse aim");
    require(command_window100(window,WM_MOUSEMOVE,0,MAKELPARAM(64,96))&&field<unsigned char>(base,0x17acf95)==1,"Q drag lost the native right-button bit");
    require(command_window100(window,WM_KEYUP,'Q',0)&&!command_placing100&&native_messages100.back()==WM_RBUTTONUP&&native_events100.back()==4&&field<unsigned char>(cursor,0x5b)==0&&field<unsigned char>(base,0x17acf95)==0,"Q release did not use native event4 cleanup");
    command100::Event event{};require(!command_pop100(event)&&packets100.size()==packets_before,"Q constructed a custom movement packet");
    require(command_window100(window,WM_KEYDOWN,'Q',0),"Second Q press");auto release_count=command_native_releases100;
    require(!command_window100(window,WM_KILLFOCUS,0,0)&&!command_placing100&&command_native_releases100==release_count&&field<unsigned char>(cursor,0x5b)==0&&field<unsigned char>(base,0x17acf95)==0,"Focus loss committed or retained native RMB gesture");
    require(command_native_presses100==2&&command_native_moves100==1&&command_native_releases100==1&&command_native_cancels100==1&&native_cursor_frees100==2,"Native Q lifecycle count differs");
    require(command_window100(window,WM_KEYDOWN,'Q',0),"Third Q press after focus cleanup");state->release=1;command_native_cancel100();require(!command_placing100&&field<unsigned char>(cursor,0x5b)==0,"Ownership release retained native placement");state->release=0;command_key_held100=0;
    require(command_window100(window,WM_KEYDOWN,'Q',0),"Q before asynchronous release");auto messages_before=native_messages100.size();
    std::thread release_worker([]{command_native_cancel100();});release_worker.join();
    require(native_messages100.size()==messages_before&&!command_placing100&&command_native_root100,"Native UI cleanup ran on the worker thread");
    command_window100(window,COMMAND_CANCEL_MESSAGE100,command_native_sequence100,0);require(!command_native_root100&&field<unsigned char>(cursor,0x5b)==0,"Posted window-thread cleanup did not finish");command_key_held100=0;
    field<uintptr_t>(P::ptr(fixture.root),0)=0;require(command_window100(window,WM_KEYDOWN,'Q',0)&&!command_placing100,"Foreign UI root admitted stock input");field<uintptr_t>(P::ptr(fixture.root),0)=base+0x170d284;
    auto characters_before=native_messages100.size();for(auto key:{'q','e','z','c','r'})weapon_window_proc(window,WM_CHAR,key,0);
    require(native_messages100.size()==characters_before&&!keyboard_window_owned84(VK_F2),"Command characters leaked or native F2 was remapped");
    require(!command_window100(window,RETURN_RTS_MESSAGE90,12345,0),"Q cleanup consumed the saved RTS-return message");
    // Execute the actual camera-boundary follow service with stationary actor
    // translation and changing body/view angles. Only the initial native order
    // is permitted; a subsequent real translation must issue a travel heading.
    fixture.clear_queue();command_key_held100=0;command_follow100.reset();state->follow=1;
    field<float>(possessed,0x48)=10;field<float>(possessed,0x4c)=3;field<float>(possessed,0x50)=20;
    state->heartbeat=GetTickCount64();process_orders(P::ptr(fixture.camera));
    require(packets100.size()==1,"Stationary Follow Me failed to form up initially");
    for(unsigned i=0;i<600;++i){
        command_follow100.next=0;field<unsigned short>(possessed,0x64)=static_cast<unsigned short>(i*109);
        state->yaw=float(i)*.1f;state->heartbeat=GetTickCount64();process_orders(P::ptr(fixture.camera));
        require(packets100.size()==1,"Mouse/body facing restarted stationary native formation");
    }
    field<float>(possessed,0x48)=11;command_follow100.next=0;state->heartbeat=GetTickCount64();process_orders(P::ptr(fixture.camera));
    require(packets100.size()==2&&packets100.back().shorts[0]==16384,"Follow did not use actual travel direction");
    state->follow=0;command_follow100.reset();
    // Actual follow service, original move constructors/serializer, synthetic
    // current peer positions; native AI path completion is not simulated.
    auto service102=[&](){command_follow100.next=0;state->heartbeat=GetTickCount64();process_orders(P::ptr(fixture.camera));};
    field<float>(P::ptr(fixture.peer),0x48)=11;field<float>(P::ptr(fixture.peer),0x50)=20;
    state->follow=1;command_pace102.reset();fixture.clear_queue();state->input=FORWARD;service102();
    require(packets100.size()==1&&packets100.back().bytes[0]==1,"Follow Me did not default to the native running order");
    state->input=FORWARD|RUN;service102();require(packets100.size()==1,"Holding sprint restarted the same Follow Me run");
    for(unsigned i=0;i<600;++i){state->yaw=float(i);service102();require(packets100.size()==1,"Stationary gaze churned running formation");}
    run_allowed102=false;service102();require(packets100.size()==2&&packets100.back().bytes[0]==0,"Native denied player run was not matched");run_allowed102=true;
    state->input=FORWARD;service102();require(packets100.size()==3&&packets100.back().bytes[0]==1,"Restored run eligibility did not restore Follow Me running");
    state->input=0;
    field<float>(P::ptr(fixture.peer),0x48)=1;service102();require(packets100.size()==3&&command_pace102.catchup,"Lagging formation did not keep running to catch up");
    field<float>(P::ptr(fixture.peer),0x48)=8;service102();require(packets100.size()==3,"Catch-up hysteresis restarted order inside band");
    field<float>(P::ptr(fixture.peer),0x48)=10;service102();require(packets100.size()==4&&packets100.back().bytes[0]==0&&!command_pace102.catchup,"Arrived formation failed to stop running");
    state->input=FORWARD;service102();require(packets100.size()==5&&packets100.back().bytes[0]==1,"Follow Me did not resume running when the leader moved");
    state->input=RUN;service102();require(packets100.size()==6&&packets100.back().bytes[0]==0,"Standing with Shift triggered running");
    state->input=FORWARD|BACKWARD|RUN;service102();require(packets100.size()==6,"Opposed movement inputs requested run");
    state->input=LEFT|RIGHT|RUN;service102();require(packets100.size()==6,"Opposed strafe inputs requested run");
    equipment_crouch81=1;state->input=FORWARD|RUN;service102();require(packets100.size()==6,"Crouch-locked player requested matching run");equipment_crouch81=0;state->input=0;
    // Foreign/dead peers cannot pull the formation into catch-up.
    field<float>(P::ptr(fixture.peer),0x48)=-500;field<uintptr_t>(P::ptr(fixture.peer),0x214)=unit+4;service102();require(packets100.size()==6,"Foreign unit affected catch-up");field<uintptr_t>(P::ptr(fixture.peer),0x214)=unit;
    field<unsigned char>(P::ptr(fixture.peer),0xf0)=0;service102();require(packets100.size()==6,"Dead actor affected catch-up");field<unsigned char>(P::ptr(fixture.peer),0xf0)=1;
    state->follow=0;state->input=0;command_follow100.reset();command_pace102.reset();
    field<float>(P::ptr(fixture.peer),0x48)=field<float>(possessed,0x48);
    field<float>(P::ptr(fixture.peer),0x50)=field<float>(possessed,0x50);
    fixture.clear_queue();state->follow=1;state->input=FORWARD|WALK_OVERRIDE;service102();
    require(packets100.size()==1&&packets100.back().bytes[0]==0,"Explicit Deck walk did not override Follow Me run");
    state->input=FORWARD|RUN;service102();
    require(packets100.size()==2&&packets100.back().bytes[0]==1,"Toggling sprint back on did not update the native Follow Me order");
    state->follow=0;state->input=0;command_follow100.reset();command_pace102.reset();
    // Q forwards a true stock double-click (7 -> 6), not two single releases.
    command_key_held100=0;command_q_tap102.reset();native_events100.clear();
    require(command_window100(window,WM_KEYDOWN,'Q',0)&&command_window100(window,WM_KEYUP,'Q',0),"Single Q tap failed");
    require(command_window100(window,WM_KEYDOWN,'Q',0)&&command_window100(window,WM_KEYUP,'Q',0),"Double Q tap failed");
    require(native_events100.size()==4&&native_events100[0]==5&&native_events100[1]==4&&native_events100[2]==7&&native_events100[3]==6,"Q did not preserve actual native contextual run events");
    require(field<unsigned char>(base,0x1b31169)==0&&!command_placing100,"Native double-click flag leaked after Q release");
    command_window100(window,WM_KEYDOWN,'Q',0);command_window100(window,WM_KEYUP,'Q',0);require(native_events100[native_events100.size()-2]==5,"Third Q incorrectly inherited double");
    command_q_tap102.reset();command_window100(window,WM_KEYDOWN,'Q',0);command_window100(window,WM_KILLFOCUS,0,0);require(!command_q_tap102.armed,"Focus cancellation retained double tap");command_key_held100=0;
    // Hold Here uses the original current-position surface projection, not the
    // old crosshair ray. Check the serialized terrain height and unchanged query
    // count as well as the double-tap anchor.
    // First Z is immediate walk. Second is run to the same captured anchor and
    // facing, even if the player moved/looked between taps. Repeat cannot upgrade.
    const auto hold_queries144=queries100;
    fixture.clear_queue();command_z_tap102.reset();field<float>(possessed,0x48)=10;field<float>(possessed,0x4c)=5;state->yaw=.25f;service102();
    require(command_window100(window,WM_KEYDOWN,'Z',0),"Z single press");process_orders(P::ptr(fixture.camera));
    require(packets100.size()==2&&packets100[0].bytes[0]==0,"Single Z did not immediately walk with guard");
    require(command_window100(window,WM_KEYDOWN,'Z',1L<<30),"Z repeat");process_orders(P::ptr(fixture.camera));require(packets100.size()==2,"Held Z autorepeat issued run");
    command_window100(window,WM_KEYUP,'Z',0);field<float>(possessed,0x48)=14;state->yaw=1.5f;service102();
    command_window100(window,WM_KEYDOWN,'Z',0);process_orders(P::ptr(fixture.camera));
    require(packets100.size()==4&&packets100[2].bytes[0]==1&&packets100[2].shorts[0]==command100::bearing(.25f)&&packets100[2].point.position[0]==10&&packets100[2].point.position[1]==10*.1f+packets100[2].point.position[2]*.2f&&queries100==hold_queries144,"Double Z lost original Hold Here anchor/facing or run flag");
    command_window100(window,WM_KEYUP,'Z',0);command_window100(window,WM_KILLFOCUS,0,0);command_key_held100=0;
    for(bool run:{false,true}){
        fixture.clear_queue();service102();
        command_window_wheel101({2,run});process_orders(P::ptr(fixture.camera));
        require(packets100.size()==2&&packets100[0].bytes[0]==unsigned(run),
                "Wheel Hold Here lost its native walk/run flag");
        require(commander_input76.feedback.hold_pace==(run?2u:1u)&&commander_input76.feedback.issued[2],
                "Wheel acknowledgement lost the committed Hold Here pace");
        fixture.clear_queue();service102();state->input=FORWARD;
        command_window_wheel101({0,run});process_orders(P::ptr(fixture.camera));
        require(packets100.size()==1&&packets100[0].bytes[0]==1&&commander_input76.feedback.issued[0],
                "Wheel Follow Me did not use the native automatic pace");
    }
    state->follow=0;state->input=0;command_follow100.reset();command_pace102.reset();
    std::array<unsigned char,32> controller101{},crosshair101{};
    interface_root=P::ptr(fixture.root);interface_environment=environment;
    interface_controller=reinterpret_cast<uintptr_t>(controller101.data());interface_crosshair=reinterpret_cast<uintptr_t>(crosshair101.data());
    field<uintptr_t>(base,0x1bbe364)=interface_root;field<uintptr_t>(P::ptr(fixture.root),0xd08)=interface_controller;
    field<uintptr_t>(interface_controller,4)=interface_crosshair;interface_original_state[0]=0;saved_widget_count=0;
    auto test_camera=P::ptr(fixture.camera);interface_hip_fov=.6f;
    for(unsigned lifecycle=0;lifecycle<3;++lifecycle){
        field<unsigned char>(P::ptr(fixture.root),0xca0)=lifecycle&1;
        interface_camera=test_camera;interface_active=1;field<float>(test_camera,0x34)=.15f;field<float>(test_camera,0x38)=5000;
        require(interface_clip101.capture(.15f,5000)&&interface_clip101.apply(field<float>(test_camera,0x34)),"Native interface near capture");
        if(lifecycle==1)field<float>(test_camera,0x34)=.07f;
        if(lifecycle==2)field<uintptr_t>(P::ptr(fixture.manager),0x8e0)=test_camera+4;
        interface_restore();
        require(ui_restores151==lifecycle+1,"Exit did not reapply the current native HUD state");
        require(field<float>(test_camera,0x34)==(lifecycle==0?.15f:lifecycle==1?.07f:.02f),"Interface changed a foreign/stale clip plane or failed exact restoration");
        require(field<float>(test_camera,0x38)==5000,"Near correction changed native far plane");
        field<uintptr_t>(P::ptr(fixture.manager),0x8e0)=test_camera;
    }
    aim_frame={};aim_frame.actor=possessed;aim_frame.env=environment;aim_frame.tick=GetTickCount64();
    aim_frame.lease101=possession_lease.generation;aim_frame.serial101=possession_lease.identity.actor_id;aim_frame.unit101=possession_lease.identity.unit_id;
    ViewRay strict_eye101{};require(read_aim_frame(strict_eye101,true)&&read_aim_frame(strict_eye101),"Current eye identity rejected");
    ++aim_frame.lease101;require(!read_aim_frame(strict_eye101,true)&&!read_aim_frame(strict_eye101),"Prior possession eye reused by presentation or gameplay");--aim_frame.lease101;
    ++aim_frame.serial101;require(!read_aim_frame(strict_eye101,true)&&!read_aim_frame(strict_eye101),"Foreign actor eye reused by presentation or gameplay");--aim_frame.serial101;
    ++aim_frame.unit101;require(!read_aim_frame(strict_eye101,true)&&!read_aim_frame(strict_eye101),"Foreign unit eye reused by presentation or gameplay");
    input_window=nullptr;native_window_proc=nullptr;DestroyWindow(window);
    require(context_cleanups100==group_cleanups100,"Native order/context cleanup lifecycles differ");
    std::cout<<"PASS: "<<calls<<" original native formation packets; point/fire/guard ABI and atomic queue failure; actual lease/foreign guards; native scene/sort lifetime; per-draw command volleys; 1000 original skin-cache/visibility cycles and restoration; stock native Q/RMB press-drag-release and no-order focus cleanup. Providers/allocators are fixture substitutions; no game loaded or launched.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
