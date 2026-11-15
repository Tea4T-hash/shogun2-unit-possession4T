// Uses the original preview constructor, entry allocation and order serializer.
// The final renderer, terrain providers and engine allocator are isolated fixtures.
#define wmain old_ui_rigid225
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/formation218.h"
#include "terrain_fixture224.h"
static void* __fastcall copy218(void* out,void*,const void* source){memcpy(out,source,12);return out;}
static void __fastcall free218(void*,void*){}
static unsigned __fastcall run218(const void*,void*){return 0;}
struct Point218{float x,y,z;uintptr_t surface;unsigned id;};
static void put218(uintptr_t p,uintptr_t v){memcpy(reinterpret_cast<void*>(p),&v,4);}

struct Preview225 {
    uintptr_t subject;
    Point218 point;
    unsigned short heading, padding;
    float scale;
};
struct List225 { unsigned capacity,count; const Preview225* entries; };
static_assert(sizeof(Preview225)==32 && sizeof(List225)==12);
static std::vector<Preview225> rendered_groups,rendered_units;
static unsigned allocated{},audio_calls{},native_dispatches{},native_action{};
static float native_amount{};
static uintptr_t cue_queue{};
static uintptr_t expected_audio{};
static float cue_position[3]{};
static void __fastcall sound225(uintptr_t audio,void*,void* result,unsigned event,const unsigned char* args) {
    assert(audio==expected_audio&&result&&event==0x12e);
    assert(*reinterpret_cast<const unsigned*>(cue_queue+4+0x5000)>0);
    assert(!memcmp(args,cue_position,sizeof cue_position));
    assert(*reinterpret_cast<const unsigned*>(args+0xc)==0x1234);
    assert(!args[0x10]&&args[0x1d]==1&&!args[0x1e]&&!args[0x24]&&args[0x25]==1);
    assert(*reinterpret_cast<const float*>(args+0x14)==-1.f);
    assert(*reinterpret_cast<const float*>(args+0x18)==-1.f);
    assert(*reinterpret_cast<const float*>(args+0x20)==1.f);
    assert(*reinterpret_cast<const unsigned*>(args+0x28)==5&&!args[0x2c]);
    ++audio_calls;
}
static void __fastcall dispatch225(uintptr_t,void*,unsigned action,float amount) {
    ++native_dispatches;native_action=action;native_amount=amount;
    *reinterpret_cast<unsigned*>(cue_queue+4+0x5000)=16;
}
static unsigned __fastcall zero225(void*,void*) { return 0; }
static float __fastcall scale225(void*,void*) { return 1.f; }
static void* __cdecl allocate225(unsigned count,unsigned) {
    auto memory=calloc(count,32);
    assert(memory);
    ++allocated;
    return memory;
}
static void __cdecl free225(void* memory) {
    if(memory) { assert(allocated);--allocated;free(memory); }
}
static void __fastcall render225(void*,void*,const List225* groups,const List225* units) {
    rendered_groups.clear();
    rendered_units.clear();
    if(groups->count)rendered_groups.assign(groups->entries,groups->entries+groups->count);
    if(units->count)rendered_units.assign(units->entries,units->entries+units->count);
}
int wmain(int argc,wchar_t** argv) {
 assert(argc==2);
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 auto image=LoadLibraryExW(argv[1],nullptr,DONT_RESOLVE_DLL_REFERENCES);
 assert(image&&MH_Initialize()==MH_OK);
 const auto b=reinterpret_cast<uintptr_t>(image);
 assert(native214::admit(b,383));
 unsigned char original[12]{},original_move[12]{};
 memcpy(original,reinterpret_cast<const void*>(b+0x42e540),sizeof original);
 memcpy(original_move,reinterpret_cast<const void*>(b+0x42dfd0),sizeof original_move);
 unsigned cases{};
 {std::vector<unsigned char> root(0xe00),env(0x28400),ui(0x328),widget(0x328),ctrl(0x60),owner(0x200),selected(0x500),queue(0x5010),common(0x100),manager(0x200),unit1(0xb00),unit2(0xb00),pose1(0x50),pose2(0x50);uintptr_t children[1]{},units[2]{};unsigned char registry[28]{};
 auto r=reinterpret_cast<uintptr_t>(root.data()),e=reinterpret_cast<uintptr_t>(env.data()),u=reinterpret_cast<uintptr_t>(ui.data()),w=reinterpret_cast<uintptr_t>(widget.data()),c=reinterpret_cast<uintptr_t>(ctrl.data()),o=reinterpret_cast<uintptr_t>(owner.data()),s=reinterpret_cast<uintptr_t>(selected.data()),q=reinterpret_cast<uintptr_t>(queue.data()),co=reinterpret_cast<uintptr_t>(common.data()),m=reinterpret_cast<uintptr_t>(manager.data()),a=reinterpret_cast<uintptr_t>(unit1.data()),z=reinterpret_cast<uintptr_t>(unit2.data()),p1=reinterpret_cast<uintptr_t>(pose1.data()),p2=reinterpret_cast<uintptr_t>(pose2.data());children[0]=w;units[0]=a;units[1]=z;
 put218(r+0x110,e);put218(r+0x5b8,e);put218(r+0x5b4,o);put218(e+0x2826c,s);put218(s+0x4a8,o);put218(e+0x28048,q);put218(e+8,co);put218(co+0xb8,m);put218(m,e);
 put218(u,b+0x1704cdc);put218(u+0x9c,1);put218(u+0xa0,reinterpret_cast<uintptr_t>(children));put218(w,b+0x1704cdc);put218(w+0x94,u);put218(w+0x318,c);put218(c,b+0x1709cc8);put218(c+4,w);put218(c+0x40,165);ctrl[0x50]=1;
 put218(o+0xb0,b+0x15c4da0);put218(o+0xc0,2);put218(o+0xc4,2);put218(o+0xc8,reinterpret_cast<uintptr_t>(units));put218(a,b+0x15c4da0);put218(z,b+0x15c4da0);put218(a+4,11);put218(z+4,22);put218(a+0x440,p1);put218(z+0x440,p2);Point218 origin1{-10,0,0,0,0},origin2{10,0,0,0,0};memcpy(pose1.data()+0x10,&origin1,20);memcpy(pose2.data()+0x10,&origin2,20);unsigned short head=16384;memcpy(pose2.data()+0x28,&head,2);
 unsigned key=0x12345678,n=1;put218(reinterpret_cast<uintptr_t>(registry),key);uintptr_t reg=reinterpret_cast<uintptr_t>(registry);float trig[2]{0,1},trig2[2]{1,0};
 Patch185 rootGlobal(b+0x1bc8180,&r,4),uiGlobal(b+0x1bbe364,&u,4),keyGlobal(b+0x192d014,&key,4),registryGlobal(b+0x18d9000,&reg,4),countGlobal(b+0x18d9004,&n,4),angle0(b+0x1bd9560,trig,8),angle90(b+0x1bd9560+16384*8,trig2,8);
 auto v=jump185(b+0x10ac90,reinterpret_cast<void*>(&copy218)),f=jump185(b+0x1d10f0,reinterpret_cast<void*>(&free218)),run=jump185(b+0x4596f0,reinterpret_cast<void*>(&run218));
 Patch185 vectors(b+0x10ac90,v.data(),5),release(b+0x1d10f0,f.data(),5),running(b+0x4596f0,run.data(),5);
 fixture224::Terrain terrain(b,co);
 formation218::FormationPad pad;auto current=[]{return true;};auto used=[&](){return *reinterpret_cast<unsigned*>(q+4+0x5000);};auto reset=[&](){pad.cancel();put218(q+4+0x5000,0);rendered_groups.clear();rendered_units.clear();audio_calls=native_dispatches=0;root[0x598]=0;fixture224::probes=fixture224::allow_after=0;fixture224::outside=false;fixture224::successful_probes=UINT_MAX;};

 auto patch=[&](uintptr_t rva,void* handler) {
    auto jump=jump185(b+rva,handler);
    return std::make_unique<Patch185>(b+rva,jump.data(),5);
 };
 auto sort=patch(0x429dd0,reinterpret_cast<void*>(&zero225));
 auto special=patch(0x420ed0,reinterpret_cast<void*>(&zero225));
 auto allocate=patch(0x9ea10,reinterpret_cast<void*>(&allocate225));
 auto deallocate=patch(0xf04d0,reinterpret_cast<void*>(&free225));
 auto render=patch(0x295a90,reinterpret_cast<void*>(&render225));
 auto sound=patch(0x2e78b0,reinterpret_cast<void*>(&sound225));
 auto dispatch=patch(0x10dfb80,reinterpret_cast<void*>(&dispatch225));
 const uintptr_t audio_table=b+0x18d9030, audio_method=b+0x2e78b0;
 const unsigned tag=0x1234;
 Patch185 audio_vtable(audio_table+0x18,&audio_method,4),audio_tag(b+0x17988d8,&tag,4);
 uintptr_t audio[1]{audio_table},audio_holder[3]{0,0,reinterpret_cast<uintptr_t>(audio)};
 cue_queue=q;
 expected_audio=audio_holder[2];
 memcpy(cue_position,reinterpret_cast<const void*>(b+0x1931f64),sizeof cue_position);
 put218(e+0x2804c,reinterpret_cast<uintptr_t>(audio_holder));
 uintptr_t table[]{reinterpret_cast<uintptr_t>(&scale225)};
 for(auto unit:units)put218(unit+0xc,reinterpret_cast<uintptr_t>(table));
 alignas(4) unsigned char first_group[0x90]{},second_group[0x90]{};
 uintptr_t groups[]{reinterpret_cast<uintptr_t>(first_group),reinterpret_cast<uintptr_t>(second_group)};
 uintptr_t grouped_units[]{units[0],units[1]};
 put218(units[0]+0x384,200);
 put218(units[1]+0x384,120);
 for(unsigned i=0;i<2;++i) {
    auto group=groups[i];
    put218(group,reinterpret_cast<uintptr_t>(table));
    put218(group+0x7c,31+i);
    put218(group+0x20,1); // original 2fc880 counts the land units in this group
    put218(group+0x24,reinterpret_cast<uintptr_t>(grouped_units+i));
    auto& point=i?origin2:origin1;
    memcpy(reinterpret_cast<void*>(group+0x2c),&point,20);
 }
 uintptr_t commander[2]{0,67};
 put218(o+0xe4,reinterpret_cast<uintptr_t>(commander));
 auto select=[&](unsigned group_count,unsigned unit_count,unsigned start=0) {
    put218(o+0xb4,2);put218(o+0xb8,group_count);
    put218(o+0xbc,reinterpret_cast<uintptr_t>(groups));
    put218(o+0xe8,2);put218(o+0xec,group_count);
    put218(o+0xf0,reinterpret_cast<uintptr_t>(groups));
    put218(o+0xc4,unit_count);put218(o+0xc8,reinterpret_cast<uintptr_t>(units+start));
 };
 auto gesture=[&]() {
    deck216::FineIntent in;
    in.began=in.active=in.turning=true;
    assert(pad.sample(b,r,e,in,current)&&used()==0);
    in.began=false;in.turn=90;
    assert(pad.sample(b,r,e,in,current)&&used()==0);
    assert(*reinterpret_cast<unsigned short*>(r+0x59a)==16384);
    return in;
 };
 auto draw=[&](unsigned angle=16384) {
    alignas(4) unsigned char ghost[0x48]{};
    using Construct=void*(__thiscall*)(void*,uintptr_t,uintptr_t,unsigned);
    assert(reinterpret_cast<Construct>(b+0x2647e0)(ghost,co,o+0xb0,angle)==ghost);
    assert(!allocated&&used()==0);
 };
 auto release_gesture=[&](deck216::FineIntent in) {
    in.active=false;in.released=true;
    return pad.sample(b,r,e,in,current);
 };
 auto packet=[&](const std::vector<Preview225>& expected_groups,const std::vector<Preview225>& expected_units,bool append=false) {
    unsigned char reference[0x5004]{};
    auto encode=[&](const Preview225& placement,bool is_group) {
        alignas(4) unsigned char view[0x44]{},order[0x80]{},context[16]{};
        auto selection=reinterpret_cast<uintptr_t>(view);
        auto object=placement.subject-(is_group?0u:12u);
        auto offset=is_group?4u:0x10u;
        put218(selection,b+0x15c4da0);
        put218(selection+0x34,reinterpret_cast<uintptr_t>(commander));
        put218(selection+offset,1);put218(selection+offset+4,1);
        put218(selection+offset+8,reinterpret_cast<uintptr_t>(&object));
        using Construct=void*(__thiscall*)(void*,const void*,const void*,unsigned,unsigned,unsigned);
        reinterpret_cast<Construct>(b+0x255680)(order,view,&placement.point,placement.heading,0,append);
        reinterpret_cast<void(__thiscall*)(void*,void*,unsigned,unsigned)>(b+0x1cc200)(context,reference,key,0);
        assert(reinterpret_cast<unsigned(__thiscall*)(void*,void*)>(b+0x2e6eb0)(order,context));
        reinterpret_cast<void(__thiscall*)(void*)>(b+0x1d0d40)(context);
    };
    for(auto& item:expected_groups)encode(item,true);
    for(auto& item:expected_units)encode(item,false);
    unsigned length{};
    memcpy(&length,reference+0x5000,4);
    assert(length&&length==used()&&!memcmp(reference,queue.data()+4,length));
 };
 for(auto counts:{std::array<unsigned,3>{0,2,0},{1,1,1},{2,0,0},{1,0,0}}) {
    reset();select(counts[0],counts[1],counts[2]);auto in=gesture();draw();
    assert(rendered_groups.size()==counts[0]&&rendered_units.size()==counts[1]);
    for(unsigned i=0;i<counts[0];++i) {
        auto& item=rendered_groups[i];
        assert(item.subject==groups[i]&&item.heading==16384&&item.scale==1.f);
        assert(std::fabs(item.point.x-(counts[0]+counts[1]==1?-10.f:0.f))<.001f);
        assert(std::fabs(item.point.z-(counts[0]+counts[1]==1?0.f:(i?-10.f:10.f)))<.001f);
    }
    for(unsigned i=0;i<counts[1];++i) {
        const unsigned index=i+counts[2];
        auto& item=rendered_units[i];
        assert(item.subject==units[index]+0xc&&item.heading==(index?32768:16384)&&item.scale==1.f);
        assert(std::fabs(item.point.x)<.001f&&std::fabs(item.point.z-(index?-10.f:10.f))<.001f);
    }
    assert(release_gesture(in)&&audio_calls==1&&*reinterpret_cast<unsigned short*>(r+0x59a)==0);
    packet(rendered_groups,rendered_units);
    assert(!release_gesture(in)&&audio_calls==1);
    ++cases;
 }
 reset();select(1,1,1);auto in=gesture();
 assert(!release_gesture(in)&&used()==0&&!audio_calls);++cases;

 reset();in=gesture();fixture224::successful_probes=1;draw();
 assert(rendered_groups.empty()&&rendered_units.empty()&&!release_gesture(in)&&used()==0&&!audio_calls);++cases;

 reset();in=gesture();draw();put218(groups[0]+0x7c,71);
 assert(!release_gesture(in)&&used()==0&&!audio_calls);put218(groups[0]+0x7c,31);++cases;

 reset();in=gesture();draw();in.turn=-90;assert(pad.sample(b,r,e,in,current));
 assert(release_gesture(in)&&audio_calls==1);packet(rendered_groups,rendered_units);++cases;

 reset();in=gesture();draw();in.turn=0;assert(pad.sample(b,r,e,in,current));
 assert(!release_gesture(in)&&used()==0&&!audio_calls);++cases;

 reset();in=gesture();
 // A direct call has no preview-caller return address, so the original
 // planner handles an empty native selection without consuming our gesture.
 alignas(4) unsigned char empty[0x44]{};
 List225 list1{},list2{};
 using Plan=void(__thiscall*)(void*,uintptr_t,unsigned,List225*,List225*);
 reinterpret_cast<Plan>(b+0x42e540)(empty,co,16384,&list1,&list2);
 assert(!list1.count&&!list2.count);
 draw();assert(release_gesture(in)&&audio_calls==1);packet(rendered_groups,rendered_units);++cases;

 reset();in=gesture();draw();put218(q+4+0x5000,0x4fff);
 assert(!release_gesture(in)&&used()==0x4fff&&!audio_calls);++cases;
 // A malformed fifth vector must still be rejected.
 reset();select(1,1,1);put218(o+0xe8,0);
 in={};in.began=in.active=in.turning=true;
 assert(!pad.sample(b,r,e,in,current)&&used()==0);++cases;
 assert(!audio_calls);
 auto start_move=[&](deck174::Vec2 direction) {
    deck216::FineIntent move;
    move.began=move.active=true;
    move.move=direction;
    assert(pad.sample(b,r,e,move,current)&&used()==0&&!audio_calls);
    move.began=false;
    return move;
 };
 auto draw_move=[&](float distance) {
    memcpy(root.data()+0x594,&distance,4);
    alignas(4) unsigned char ghost[0x48]{};
    using Construct=void*(__thiscall*)(void*,uintptr_t,uintptr_t,float,unsigned);
    assert(reinterpret_cast<Construct>(b+0x264600)(ghost,co,o+0xb0,distance,0)==ghost);
    assert(!allocated&&used()==0&&!audio_calls);
    root[0x598]=1;
 };
 for(unsigned step=0;step<16;++step) {
    reset();select(1,1,1);
    const double angle=(double(step)+.5)*6.283185307179586/16.;
    const float x=float(std::cos(angle)),y=float(std::sin(angle));
    auto move=start_move({x,y});
    assert(pad.sample(b,r,e,move,current));
    draw_move(5.f);
    assert(rendered_groups.size()==1&&rendered_units.size()==1);
    const auto& g=rendered_groups[0];
    const auto& unit_preview=rendered_units[0];
    assert(g.subject==groups[0]&&g.heading==0&&unit_preview.subject==units[1]+0xc&&unit_preview.heading==16384);
    assert(std::fabs(g.point.x-(-10.f+5.f*x))<.001f&&std::fabs(g.point.z-5.f*y)<.001f);
    assert(std::fabs(unit_preview.point.x-(10.f+5.f*y))<.001f&&std::fabs(unit_preview.point.z+5.f*x)<.001f);
    assert(release_gesture(move)&&audio_calls==1&&!native_dispatches);
    packet(rendered_groups,rendered_units);
    assert(!release_gesture(move)&&audio_calls==1);
    ++cases;
 }
 reset();select(1,1,1);
 auto move=start_move({.6f,.8f});
 assert(!release_gesture(move)&&!used()&&!audio_calls);++cases;

 reset();move=start_move({.6f,.8f});fixture224::successful_probes=1;draw_move(5.f);
 assert(rendered_groups.empty()&&rendered_units.empty());
 assert(!release_gesture(move)&&!used()&&!audio_calls);++cases;

 reset();move=start_move({.6f,.8f});draw_move(5.f);root[0x598]=0;
 assert(!release_gesture(move)&&!used()&&!audio_calls);++cases;

 reset();move=start_move({.6f,.8f});draw_move(5.f);put218(q+4+0x5000,0x4fff);
 assert(!release_gesture(move)&&used()==0x4fff&&!audio_calls);++cases;

 reset();move=start_move({.6f,.8f});
 List225 no_groups{},no_units{};
 using MovePlan=void(__thiscall*)(void*,uintptr_t,float,List225*,List225*);
 reinterpret_cast<MovePlan>(b+0x42dfd0)(empty,co,5.f,&no_groups,&no_units);
 assert(!no_groups.count&&!no_units.count&&!used()&&!audio_calls);
 draw_move(5.f);assert(release_gesture(move)&&audio_calls==1);packet(rendered_groups,rendered_units);++cases;

 for(bool append:{false,true})for(float strafe:{-1.f,1.f}) {
    reset();select(1,1,1);
    auto forward=start_move({0,1});
    draw_move(4.f);
    assert(release_gesture(forward)&&audio_calls==1&&!native_dispatches);
    packet(rendered_groups,rendered_units);
    const auto previous_bytes=used();
    const std::vector<unsigned char> previous(queue.data()+4,queue.data()+4+previous_bytes);
    // A separate sideways gesture uses its own visible endpoints and release modifier.
    put218(q+4+0x5000,0);audio_calls=0;
    move=start_move({strafe,0});
    draw_move(4.f);
    assert(rendered_groups.size()==1&&rendered_units.size()==1);
    assert(rendered_groups[0].point.x==-10.f+4.f*strafe&&rendered_groups[0].point.z==0.f);
    assert(rendered_units[0].point.x==10.f&&rendered_units[0].point.z==-4.f*strafe);
    memcpy(queue.data()+4,previous.data(),previous_bytes);
    put218(q+4+0x5000,previous_bytes);
    move.append=append;
    assert(release_gesture(move)&&audio_calls==1&&!native_dispatches&&used()>previous_bytes);
    assert(!memcmp(queue.data()+4,previous.data(),previous_bytes));
    const auto length=used()-previous_bytes;
    memmove(queue.data()+4,queue.data()+4+previous_bytes,length);
    put218(q+4+0x5000,length);
    packet(rendered_groups,rendered_units,append);
    ++cases;
 }
 reset();select(1,1,1);in=gesture();draw();in.append=true;
 assert(release_gesture(in)&&audio_calls==1);packet(rendered_groups,rendered_units,true);++cases;

 reset();move=start_move({1,0});draw_move(4.f);put218(units[1]+4,99);
 assert(!release_gesture(move)&&!used()&&!audio_calls);put218(units[1]+4,22);++cases;

 reset();move=start_move({1,0});draw_move(4.f);move.released=true;move.active=false;
 assert(!pad.sample(b,r,e,move,[]{return false;})&&!used()&&!audio_calls);++cases;

 // Recreate LT occupying the cursor, followed by release in the same
 // selection. The pad starts only once the original cursor is available.
 reset();select(1,1,1);
 deck216::FineIntent pending;pending.began=pending.active=true;pending.move={1,0};
 assert(pad.sample(b,r,e,pending,current,true)&&*reinterpret_cast<int*>(c+0x40)==165);
 pending.began=false;
 assert(pad.sample(b,r,e,pending,current,false)&&*reinterpret_cast<int*>(c+0x40)==14);
 draw_move(4.f);assert(release_gesture(pending));packet(rendered_groups,rendered_units);++cases;

 // The preceding LT order clears the native ghost flag. A fresh native
 // draw restores the flag, and the pending pad release commits once.
 reset();move=start_move({1,0});draw_move(4.f);root[0x598]=0;
 assert(!release_gesture(move)&&pad.pending_release()&&!used());
 draw_move(4.f);
 assert(pad.sample(b,r,e,{},current)&&!pad.pending_release()&&audio_calls==1);
 packet(rendered_groups,rendered_units);++cases;

 // A native selection storage replacement is safe only when every member
 // remains identical. Its address alone does not change the selection.
 reset();move=start_move({1,0});draw_move(4.f);
 uintptr_t relocated_units[]{units[1]};
 put218(o+0xc8,reinterpret_cast<uintptr_t>(relocated_units));
 assert(release_gesture(move));packet(rendered_groups,rendered_units);++cases;
 select(1,1,1);

 for(int width:{-200,200}) {
    reset();select(1,1,1);
    deck216::FineIntent resize;resize.began=resize.active=resize.resizing=true;
    assert(pad.sample(b,r,e,resize,current));
    resize.began=false;resize.width_fraction=width<0?-1.f:1.f;root[0x598]=1;
    assert(pad.sample(b,r,e,resize,current)&&!root[0x598]);
    assert(*reinterpret_cast<int*>(r+0x590)==width&&!audio_calls&&!native_dispatches);
    assert(!*reinterpret_cast<unsigned*>(r+0x594)&&!*reinterpret_cast<unsigned short*>(r+0x59a));
    assert(!release_gesture(resize)&&pad.pending_release()&&!used());
    root[0x598]=1;
    assert(pad.sample(b,r,e,{},current)&&used()==16&&native_dispatches==1&&!audio_calls);
    assert(native_action==(width>0?0xfu:0x10u)&&native_amount==200.f&&!root[0x598]);
    assert(!*reinterpret_cast<int*>(r+0x590)&&!pad.pending_release());
    assert(!pad.sample(b,r,e,{},current)&&native_dispatches==1);++cases;
 }

 reset();select(1,1,1);
 deck216::FineIntent resize;resize.began=resize.active=resize.resizing=true;resize.width_fraction=.25f;
 assert(pad.sample(b,r,e,resize,current));root[0x598]=1;put218(q+4+0x5000,0x4fff);
 resize.began=false;
 assert(!release_gesture(resize)&&!pad.pending_release()&&!native_dispatches&&used()==0x4fff);++cases;

 reset();select(1,1,1);
 resize.began=true;assert(pad.sample(b,r,e,resize,current));root[0x598]=1;
 resize.began=false;resize.cancel=true;
 assert(!pad.sample(b,r,e,resize,current)&&!native_dispatches&&!*reinterpret_cast<int*>(r+0x590));++cases;

 // Exercise the original adjustable frontage policy as well as the
 // packet dispatch provider. A roster-sized request reaches either endpoint.
 alignas(4) unsigned char frontage[0x18]{};
 const auto policy=reinterpret_cast<uintptr_t>(frontage);
 put218(policy,b+0x15e05e0);
 const float stride=1.f;
 memcpy(frontage+8,&stride,4);
 memcpy(frontage+0x10,&stride,4);
 using Width= float(__thiscall*)(void*,float,unsigned,unsigned);
 const auto widen=reinterpret_cast<Width>(b+0x3d55d0);
 const auto narrow=reinterpret_cast<Width>(b+0x3cdd50);
 for(unsigned count:{40u,120u,200u,320u}) {
    assert(widen(frontage,1.f,count,count)==float(count));
    assert(narrow(frontage,float(count),count,count)==1.f);
    ++cases;
 }

 reset();
 formation218::FormationPad::disable_preview_hook();
 assert(!memcmp(original,reinterpret_cast<const void*>(b+0x42e540),sizeof original));
 assert(!memcmp(original_move,reinterpret_cast<const void*>(b+0x42dfd0),sizeof original_move));
 }
 assert(MH_Uninitialize()==MH_OK&&FreeLibrary(image));
 std::cout<<"PASS rigid225: "<<cases<<" native preview/release cases for one engine image\n";
}
