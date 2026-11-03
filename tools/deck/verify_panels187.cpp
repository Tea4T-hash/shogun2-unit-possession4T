// Complete-window regression using original stock/Bugfix viewport, root resize,
// docking, MoveTo and Position in isolated owned images. Rendering, state
// storage and UI event dispatch are providers; no game startup or Steam input.
#define wmain prior_native_ui185_main
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/campaign_keys185.h"
struct Node191 {unsigned char bytes[0x328]{};};
struct State191 {uintptr_t table{};unsigned char pad[0x24]{};float w{},h{};unsigned font{0x12345678};};
static unsigned state_calls191{};
static void __fastcall state_size191(State191* self,void*,float w,float h,bool images){assert(!images);self->w=w;self->h=h;++state_calls191;}
static void __fastcall event191(void*,void*){}
static void __fastcall position_event191(Node191* self,void*,const float* p){memcpy(self->bytes+0xa4,p,8);}
static void set191(Node191& n,unsigned at,uintptr_t v){memcpy(n.bytes+at,&v,4);}
static void xy191(Node191& n,float x,float y){float p[2]{x,y};memcpy(n.bytes+0xa4,p,8);}
struct Geometry191 {float w,h,tab_y,close_x,close_y,close_w,close_h;};
int wmain(int argc,wchar_t** argv){assert(argc==3);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned cases=0,key_cases=0;
 for(int input=1;input<argc;++input){
  HMODULE image=LoadLibraryExW(argv[input],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);assert(deck185::install_ui185(base));
  {auto jump=jump185(base+0x13f7b40,reinterpret_cast<void*>(&physical185));Patch185 physical(base+0x13f7b40,jump.data(),5);
   uintptr_t flags=reinterpret_cast<uintptr_t>(&mode185),display=reinterpret_cast<uintptr_t>(&display_info185);Patch185 mode(base+0x1754574+0x248,&flags,4),device(base+0x1754574+0x30c,&display,4);
   uintptr_t render_table=base+0x1754574;unsigned char renderer[32]{};memcpy(renderer,&render_table,4);uintptr_t render=reinterpret_cast<uintptr_t>(renderer);
   Node191 root;State191 root_state;uintptr_t states_vtable[2]{0,reinterpret_cast<uintptr_t>(&state_size191)};root_state.table=reinterpret_cast<uintptr_t>(states_vtable);
   uintptr_t root_state_p=reinterpret_cast<uintptr_t>(&root_state),root_p=reinterpret_cast<uintptr_t>(&root);set191(root,0,base+0x1704cdc);set191(root,8,render);set191(root,0xb8,root_state_p);set191(root,0xb0,1);set191(root,0xb4,reinterpret_cast<uintptr_t>(&root_state_p));
   Patch185 current(base+0x1bbe364,&root_p,4);unsigned char manager[0x60]{};uintptr_t manager_p=reinterpret_cast<uintptr_t>(manager);Patch185 active(base+0x1bc73e4,&manager_p,4);
   auto cb=jump185(base+0xf945f0,reinterpret_cast<void*>(&event191));Patch185 notify(base+0xf945f0,cb.data(),5);uintptr_t pos_event=reinterpret_cast<uintptr_t>(&position_event191);Patch185 position_event(base+0x1704cdc+0xc,&pos_event,4);
   // Original f8a070 docking executes (not mocked). The event endpoint stores
   // native local coordinates; no frame-only recursive resize is permitted.
   for(auto res:{std::pair<unsigned,unsigned>{1280,800},{1280,720},{1920,1080},{2560,1440}}){width185=res.first;height185=res.second;
    for(auto g:{Geometry191{1280,1024,59,610,954,60,60}, // Clan capture / native layout
                Geometry191{624,768,60,279,710,66,50}, // Finance original
                Geometry191{1280,1024,67,645,956,60,60}, // Diplomacy actions
                Geometry191{1280,1024,74,607,955,60,60}, // Technology
                Geometry191{976,857,83,403,788,66,50}}){ // Agent skill frame reference
     Node191 panel,tab,contents,close;State191 ps,ts,cs,bs;Node191* nodes[]={&panel,&tab,&contents,&close};State191* states[]={&ps,&ts,&cs,&bs};uintptr_t ptrs[4]{};
     for(unsigned i=0;i<4;++i){ptrs[i]=reinterpret_cast<uintptr_t>(states[i]);states[i]->table=reinterpret_cast<uintptr_t>(states_vtable);set191(*nodes[i],0,base+0x1704cdc);set191(*nodes[i],0xb8,ptrs[i]);set191(*nodes[i],0xb0,1);set191(*nodes[i],0xb4,reinterpret_cast<uintptr_t>(&ptrs[i]));nodes[i]->bytes[0xf5]=1;set191(*nodes[i],0x94,i?reinterpret_cast<uintptr_t>(&panel):root_p);}
     ps.w=g.w;ps.h=g.h;ts.w=210;ts.h=40;cs.w=g.w-40;cs.h=g.close_y-130;bs.w=g.close_w;bs.h=g.close_h;
     xy191(panel,15,-102);xy191(tab,25,g.tab_y);xy191(contents,20,110);xy191(close,g.close_x,g.close_y);
     // The content is fixed-size and has its own fixed-size descendant. The
     // rejected old fitter moved the footer while this content stayed put.
     Node191 row;State191 rs;rs.table=reinterpret_cast<uintptr_t>(states_vtable);rs.w=280;rs.h=22;uintptr_t rs_p=reinterpret_cast<uintptr_t>(&rs);set191(row,0,base+0x1704cdc);set191(row,0x94,reinterpret_cast<uintptr_t>(&contents));set191(row,0xb8,rs_p);set191(row,0xb0,1);set191(row,0xb4,reinterpret_cast<uintptr_t>(&rs_p));xy191(row,12,cs.h-24);
     uintptr_t row_p=reinterpret_cast<uintptr_t>(&row);set191(contents,0x9c,1);set191(contents,0xa0,reinterpret_cast<uintptr_t>(&row_p));
     uintptr_t children[]{reinterpret_cast<uintptr_t>(&tab),reinterpret_cast<uintptr_t>(&contents),reinterpret_cast<uintptr_t>(&close)};set191(panel,0x9c,3);set191(panel,0xa0,reinterpret_cast<uintptr_t>(children));set191(panel,0xfc,5);
     uintptr_t panel_p=reinterpret_cast<uintptr_t>(&panel);set191(root,0x9c,1);set191(root,0xa0,reinterpret_cast<uintptr_t>(&panel_p));unsigned one=1;uintptr_t entries=reinterpret_cast<uintptr_t>(&panel_p);memcpy(manager+0x30,&one,4);memcpy(manager+0x34,&entries,4);memcpy(manager+0x3c,&one,4);memcpy(manager+0x40,&entries,4); // duplicate manager membership
     deck185::panel_extent191_set(0,0);auto map=deck185::active_ui185();root_state.w=float(map.width);root_state.h=float(map.height);
     state_calls191=0;assert(deck185::sync_panels191(true));auto viewport=deck185::active_ui185();assert(deck185::trace187.panels==1);
     assert(ps.w==g.w&&ps.h==g.h&&bs.w==g.close_w&&bs.h==g.close_h&&cs.w==g.w-40&&cs.h==g.close_y-130&&rs.w==280&&rs.h==22);
     assert(ps.font==0x12345678&&bs.font==0x12345678&&rs.font==0x12345678&&state_calls191<=1);
     float at[2]{};using Position=float*(__thiscall*)(void*,float*);auto position=reinterpret_cast<Position>(base+0xf955b0);
     for(unsigned i=0;i<4;++i){position(nodes[i],at);assert(at[0]>=15.9&&at[1]>=15.9&&at[0]+states[i]->w<=viewport.width-15.9&&at[1]+states[i]->h<=viewport.height-15.9);}
     position(&row,at);float button[2]{};position(&close,button);assert(at[1]+rs.h<button[1]); // close remains below content
     auto calls=state_calls191;for(unsigned frame=0;frame<60;++frame)assert(deck185::sync_panels191(true));assert(state_calls191==calls); // stable, no repeated shrink
     assert(deck185::sync_panels191(false));auto returned=deck185::active_ui185();assert(returned.width==map.width&&returned.height==map.height&&!root.bytes[0xed]&&!root.bytes[0xee]);
     ++cases;
    }
   }
   // New campaign setup is a full-screen child, not a PanelManager modal.
   // Original owned Japan/Boshin layouts are1280x1024; footer ends at1024.
   set191(root,0x9c,0);set191(root,0xa0,0); // previous modal fixture children have left scope
   unsigned frontend_cases=0;
   for(auto res:{std::pair<unsigned,unsigned>{1280,800},{1280,720},{1920,1080},{2560,1440}}){
    width185=res.first;height185=res.second;deck185::panel_extent191_set(0,0);
    auto normal=deck185::active_ui185();root_state.w=float(normal.width);root_state.h=float(normal.height);
    if(res.first==1280)assert(normal.height<1024); // reproduced clipping before correction
    const auto font=root_state.font;assert(deck185::fit_frontend209({1280,1024}));
    auto fit=deck185::active_ui185();assert(fit.width>=1280&&fit.height>=1024);
    // Actual authored Start Game bottom edges: Japan968, Boshin1015.
    for(float bottom:{968.f,1015.f})assert(bottom<=fit.height);
    auto calls=state_calls191;for(unsigned n=0;n<60;++n)assert(deck185::fit_frontend209({1280,1024}));assert(state_calls191==calls);
    assert(root_state.font==font&&!root.bytes[0xed]&&!root.bytes[0xee]);
    auto saved=InterlockedCompareExchange64(&deck185::panel_extent191,0,0);assert(!deck185::fit_frontend209({0,1024})&&!deck185::fit_frontend209({8193,1024})&&InterlockedCompareExchange64(&deck185::panel_extent191,0,0)==saved);
    assert(deck185::fit_frontend209({}));auto restored=deck185::active_ui185();assert(restored.width==normal.width&&restored.height==normal.height);++frontend_cases;
   }
   assert(frontend_cases==4);std::cout<<"PASS menu209: 4 native frontend campaign fitting cases; footer bounds, stable scale and map restoration.\n";
   // Captured revision35: root already1638x1024, but the adopted campaign
   // screen is1310x819 at164,102. Its visible Start footer starts exactly at
   // the ancestor's bottom and is clipped. Reproduce the COMPLETE hierarchy.
   unsigned parent_cases210=0;
   for(auto res:{std::pair<unsigned,unsigned>{1280,800},{1280,720},{1920,1080},{2560,1440}})for(bool japan:{true,false}){
    width185=res.first;height185=res.second;deck185::panel_extent191_set(0,0);auto normal=deck185::active_ui185();root_state.w=float(normal.width);root_state.h=float(normal.height);
    Node191 screen,main,footer,button;State191 ss,ms,fs,bs;Node191* nodes[]{&screen,&main,&footer,&button};State191* states[]{&ss,&ms,&fs,&bs};uintptr_t state_ptrs[4]{};
    for(unsigned i=0;i<4;++i){state_ptrs[i]=reinterpret_cast<uintptr_t>(states[i]);states[i]->table=reinterpret_cast<uintptr_t>(states_vtable);set191(*nodes[i],0,base+0x1704cdc);set191(*nodes[i],0xb8,state_ptrs[i]);set191(*nodes[i],0xb0,1);set191(*nodes[i],0xb4,reinterpret_cast<uintptr_t>(&state_ptrs[i]));nodes[i]->bytes[0xf5]=1;}
    auto sp=reinterpret_cast<uintptr_t>(&screen),mp=reinterpret_cast<uintptr_t>(&main),fp=reinterpret_cast<uintptr_t>(&footer),bp=reinterpret_cast<uintptr_t>(&button);
    set191(screen,0x94,root_p);set191(main,0x94,sp);set191(footer,0x94,japan?mp:sp);set191(button,0x94,fp);
    set191(root,0x9c,1);set191(root,0xa0,reinterpret_cast<uintptr_t>(&sp));set191(screen,0x9c,1);uintptr_t child=japan?mp:fp;set191(screen,0xa0,reinterpret_cast<uintptr_t>(&child));
    set191(main,0x9c,1);set191(main,0xa0,reinterpret_cast<uintptr_t>(&fp));set191(footer,0x9c,1);set191(footer,0xa0,reinterpret_cast<uintptr_t>(&bp));
    ss.w=float(normal.width);ss.h=float(normal.height);ms.w=1104;ms.h=912;fs.w=379;fs.h=56;bs.w=234;bs.h=38;
    screen.bytes[0xed]=screen.bytes[0xee]=1;set191(screen,0xfc,5);set191(main,0xfc,5);xy191(screen,0,0);xy191(main,(ss.w-ms.w)/2,(ss.h-ms.h)/2);xy191(footer,japan?363.f:450.f,japan?865.f:968.f);xy191(button,72,9);
    assert(deck185::fit_frontend209({1280,1024}));auto fit=deck185::active_ui185();
    using Position210=float*(__thiscall*)(void*,float*);auto position210=reinterpret_cast<Position210>(base+0xf955b0);
    float screen_xy[2]{},footer_xy[2]{},button_xy[2]{};position210(&screen,screen_xy);position210(&footer,footer_xy);
    if(res.first==1280)assert(footer_xy[1]+fs.h>screen_xy[1]+ss.h); // old viewport-only fix fails
    // Same original routines as UIComponent:Resize(false,false) and MoveTo.
    auto resize210=reinterpret_cast<deck185::Resize187>(base+0xf98900);
    auto move210=reinterpret_cast<void(__thiscall*)(void*,const float*)>(base+0xf920a0);
    resize210(&screen,float(fit.width),float(fit.height),false,false);const float origin[2]{};move210(&screen,origin);
    position210(&screen,screen_xy);position210(&footer,footer_xy);position210(&button,button_xy);
    assert(ss.w==fit.width&&ss.h==fit.height&&screen_xy[0]==0&&screen_xy[1]==0);
    assert(ms.w==1104&&ms.h==912&&fs.w==379&&fs.h==56&&bs.w==234&&bs.h==38);
    assert(footer_xy[0]>=0&&footer_xy[1]>=0&&footer_xy[0]+fs.w<=ss.w&&footer_xy[1]+fs.h<=ss.h);
    assert(button_xy[0]>=footer_xy[0]&&button_xy[1]>=footer_xy[1]&&button_xy[0]+bs.w<=footer_xy[0]+fs.w&&button_xy[1]+bs.h<=footer_xy[1]+fs.h);
    assert(ss.font==0x12345678&&ms.font==0x12345678&&fs.font==0x12345678&&bs.font==0x12345678&&screen.bytes[0xed]==1&&screen.bytes[0xee]==1);
    set191(root,0x9c,0);set191(root,0xa0,0);assert(deck185::fit_frontend209({}));++parent_cases210;
   }
   assert(parent_cases210==8);std::cout<<"PASS menu210: 8 native campaign parent clipping cases; actual Resize/MoveTo/docking, footer and button bounds, unchanged content sizes.\n";
   // Invalid or replaced ownership fails before geometry or policy writes.
   Node191 foreign;uintptr_t at=reinterpret_cast<uintptr_t>(&foreign),entries=reinterpret_cast<uintptr_t>(&at);unsigned count=1;memcpy(manager+0x30,&count,4);memcpy(manager+0x34,&entries,4);memset(manager+0x3c,0,4);
   auto before=InterlockedCompareExchange64(&deck185::panel_extent191,0,0);assert(!deck185::sync_panels191(true)&&InterlockedCompareExchange64(&deck185::panel_extent191,0,0)==before);++cases;
   count=33;memcpy(manager+0x30,&count,4);assert(!deck185::sync_panels191(true));++cases;
  }
  for(auto target:deck185::native_ui185.targets){assert(MH_DisableHook(target)==MH_OK&&MH_RemoveHook(target)==MH_OK);}deck185::native_ui185={};deck185::panel_extent191_set(0,0);assert(deck185::native_admitted(base));assert(FreeLibrary(image));
 }
 deck185::CampaignKeys keys;unsigned char current[256]{};assert(keys.put(deck183::Finance,1000,1)&&keys.apply(current,true,true,1000,1));++key_cases;
 for(unsigned n=1;n<=10000;++n){unsigned action=n%2?deck183::Diplomacy:deck183::Clan;assert(keys.put(action,1000+n/25,1));assert(keys.count==1);memset(current,0,sizeof current);assert(!keys.apply(current,true,true,1000+n/25,1));assert(!current[0x25]&&!current[0x26]);++key_cases;}
 assert(keys.apply(current,true,true,1500,1)&&current[0x26]==0x80&&!keys.count);++key_cases;memset(current,0,sizeof current);assert(!keys.apply(current,true,true,1501,1));++key_cases;
 std::cout<<"PASS panels191: "<<cases<<" native whole-window geometry/lifecycle cases; "<<key_cases<<" bounded burst/coalescing/settle cases; exact stock+Bugfix restoration; state/event/render providers mocked, no game/Steam.\n";
}
