// Actual native UI Resize/MoveTo/Position and OpenPanels predicate in owned
// mapped stock/Bugfix images. Font renderer and speed-command consumer are
// explicit providers; never call game entry, simulation or Steam.
#define wmain old_native_ui214
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/ui214.h"
struct Node191 {unsigned char bytes[0x328]{};};
static void __fastcall event191(void*,void*){}
static void __fastcall position_event191(Node191* self,void*,const float* p){memcpy(self->bytes+0xa4,p,8);}
static void set191(Node191& n,unsigned at,uintptr_t v){memcpy(n.bytes+at,&v,4);}
static void xy191(Node191& n,float x,float y){float p[2]{x,y};memcpy(n.bytes+0xa4,p,8);}
struct State214 {uintptr_t table{},owner{};unsigned char rest[0x118]{};};
static void size214(State214& s,float w,float h){
 memcpy(reinterpret_cast<char*>(&s)+0x28,&w,4);
 memcpy(reinterpret_cast<char*>(&s)+0x2c,&h,4);
}
static unsigned refreshes214{},speed_calls214{};static float speed214=1.f;
static bool __fastcall state214(State214* s,void*,float w,float h,bool images){assert(images);memcpy(reinterpret_cast<char*>(s)+0x28,&w,4);memcpy(reinterpret_cast<char*>(s)+0x2c,&h,4);return true;}
static void __fastcall font214(void* s,void*){auto p=reinterpret_cast<unsigned char*>(s);assert(*reinterpret_cast<int*>(p+0xd0)==*reinterpret_cast<int*>(p+0xe4));++refreshes214;}
static float __fastcall get_speed214(void*,void*){return speed214;}
static void __fastcall set_speed214(void*,void*,float speed,unsigned char flag){assert(!flag);speed214=speed;++speed_calls214;}
static unsigned char graphics_flags214[0x30]{};
static void* __fastcall graphics_flags(void*,void*){return graphics_flags214;}
static float backend_offset214[2]{};
static float* __fastcall viewport_offset(void*,void*,float* out){
 out[0]=backend_offset214[0];out[1]=backend_offset214[1];out[2]=0;return out;
}
static uintptr_t* __fastcall viewport_owner(void*,void*){
 static uintptr_t table[15]{};
 static uintptr_t object=reinterpret_cast<uintptr_t>(table);
 table[0x38/4]=reinterpret_cast<uintptr_t>(&viewport_offset);
 return &object;
}
static void test_popup_projection(uintptr_t base,void* renderer,const void* context,Node191& widget,deck185::UiSize view){
 const auto address=reinterpret_cast<uintptr_t>(&widget);
 deck85::Projection scoped;
 assert(deck85::tooltip_projection(address,context,scoped)&&scoped.renderer==renderer);
 auto place=scoped.place;assert(place.scale>1&&place.scale<=1.5f);
 auto before=widget;
 for(unsigned mode=0;mode<2;++mode){
  auto point=reinterpret_cast<deck85::Convert>(base+deck85::origin_rvas[mode]);
  auto extent=reinterpret_cast<deck85::Convert>(base+deck85::size_rvas[mode]);
  for(const auto dimensions:{deck85::Point{240,112},deck85::Point{22,22},deck85::Point{8,14}}){
   deck85::Point native_pos,expected_pos,actual_pos,native_size,actual_size;
   const float x=place.from.x+15,y=place.from.y+19;
   point(renderer,&native_pos,x,y);
   point(renderer,&expected_pos,place.to.x+15*place.scale,place.to.y+19*place.scale);
   extent(renderer,&native_size,dimensions.x,dimensions.y);
   deck85::projection=scoped;
   assert(point(renderer,&actual_pos,x,y)==&actual_pos);
   assert(extent(renderer,&actual_size,dimensions.x,dimensions.y)==&actual_size);
   deck85::projection={};
   assert(std::fabs(actual_pos.x-expected_pos.x)<.00001f&&std::fabs(actual_pos.y-expected_pos.y)<.00001f);
   assert(std::fabs(actual_size.x-native_size.x*place.scale)<.00001f&&std::fabs(actual_size.y-native_size.y*place.scale)<.00001f);
   point(renderer,&actual_pos,x,y);
   assert(actual_pos.x==native_pos.x&&actual_pos.y==native_pos.y);
  }
 }
 assert(place.to.x>=12&&place.to.y>=12&&place.to.x+240*place.scale<=view.width-11&&place.to.y+112*place.scale<=view.height-11);
 assert(!std::memcmp(&before,&widget,sizeof widget));
}
static void test_card_strip(uintptr_t base,uintptr_t root,uintptr_t tip,const void* context,
                            deck185::UiSize view){
 Node191 strip,card,decoration,button;
 State214 strip_state,card_state;
 const auto strip_ptr=reinterpret_cast<uintptr_t>(&strip),card_ptr=reinterpret_cast<uintptr_t>(&card);
 const auto decorated=reinterpret_cast<uintptr_t>(&decoration),unrelated=reinterpret_cast<uintptr_t>(&button);
 set191(strip,0,base+0x1704cdc);set191(card,0,base+0x1704cdc);
 set191(strip,0x94,root);set191(card,0x94,strip_ptr);
 set191(decoration,0x94,card_ptr);set191(button,0x94,strip_ptr);
 set191(strip,0xb8,reinterpret_cast<uintptr_t>(&strip_state));
 set191(card,0xb8,reinterpret_cast<uintptr_t>(&card_state));
 strip.bytes[0xf5]=card.bytes[0xf5]=1;
 size214(strip_state,800,160);size214(card_state,80,110);
 const float top=float(view.height)-160;
 xy191(strip,40,top);xy191(card,48,top+14);
 std::array<unsigned char,0x5b8> battle{};
 std::array<uintptr_t,0x150/4> manager{};
 uintptr_t entry[]{0,0,43,card_ptr},bucket[]{reinterpret_cast<uintptr_t>(entry),0,0,0};
 const auto battle_ptr=reinterpret_cast<uintptr_t>(battle.data());
 const auto manager_ptr=reinterpret_cast<uintptr_t>(manager.data());
 const uintptr_t battle_table=base+0x170d284;
 memcpy(battle.data(),&battle_table,4);
 memcpy(battle.data()+0x5b4,&manager_ptr,4);
 manager[0]=base+0x170d184;manager[0x98/4]=battle_ptr;manager[0x9c/4]=strip_ptr;
 manager[0x70/4]=1;manager[0x74/4]=reinterpret_cast<uintptr_t>(bucket);
 Patch185 battle_root(base+0x1bc8180,&battle_ptr,4),hover(base+0x1bbeac8,&decorated,4);
 const auto plain=deck85::placement({float(view.width)-242,float(view.height)-114},240,112,view);
 deck85::Projection projected;
 uintptr_t owner_manager{};
 assert(deck85::battle_card_strip(root,owner_manager)==strip_ptr&&owner_manager==manager_ptr);
 assert(deck85::battle_card_strip_top(manager_ptr,strip_ptr,view)==top);
 assert(deck85::tooltip_projection(tip,context,projected));
 assert(projected.place.scale==plain.scale&&projected.place.to.x==plain.to.x);
 assert(projected.place.to.y+112*projected.place.scale<=top-5.99f);
 assert(projected.place.to.y<plain.to.y);
 card.bytes[0xf5]=0;
 assert(deck85::battle_card_strip_top(manager_ptr,strip_ptr,view)==0);
 card.bytes[0xf5]=1;
 {Patch185 noncard(base+0x1bbeac8,&unrelated,4);
  assert(deck85::battle_card_strip_top(manager_ptr,strip_ptr,view)==0);
  assert(deck85::tooltip_projection(tip,context,projected)&&projected.place.to.y==plain.to.y);}
 // An old battle manager may survive leaving the battlefield. Its
 // review strip must still belong to the currently drawn root.
 set191(strip,0x94,0);
 assert(!deck85::tooltip_projection(tip,context,projected));
 const auto large=deck85::placement({-30,top},float(view.width),float(view.height),view,top);
 assert(large.scale>0&&large.scale<1&&large.to.y>=12&&large.to.y+view.height*large.scale<=top-5.99f);
}

template<class Check>
static void with_campaign_ui(uintptr_t base,Check check){
 std::array<uintptr_t,0xa9c/4> campaign{};
 std::array<uintptr_t,0x13c/4> camera{};
 uintptr_t cameras[]{reinterpret_cast<uintptr_t>(camera.data())};
 uintptr_t node[]{0,1,reinterpret_cast<uintptr_t>(cameras)};
 uintptr_t nodes[]{reinterpret_cast<uintptr_t>(node)};
 uintptr_t stack[]{0,0,0,reinterpret_cast<uintptr_t>(nodes),0};
 uintptr_t manager[]{0,0,0,0,0,reinterpret_cast<uintptr_t>(stack)};
 campaign[0xa98/4]=reinterpret_cast<uintptr_t>(manager);
 camera[0]=base+0x16209f8;camera[0x138/4]=1;
 const auto current=reinterpret_cast<uintptr_t>(campaign.data());
 Patch185 owner(base+0x1bc8fe4,&current,4);
 check();
}

int wmain(int argc,wchar_t** argv){assert(argc==3);unsigned cases{},strip_cases{},setup_cases{};
 for(int index=1;index<argc;++index){
  auto image=LoadLibraryExW(argv[index],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);
  native214::admitted_image=0;assert(native214::admit(base,1023));
  // Every new dependency is rejected if occupied/changed before admission.
  for(const auto& span:native214::spans){unsigned char changed=*reinterpret_cast<unsigned char*>(base+span.rva)^1;{Patch185 bad(base+span.rva,&changed,1);native214::admitted_image=0;assert(!native214::admit(base,1023));}assert(native214::admit(base,1023));++cases;}
  {unsigned char manager[0x60]{};auto manager_ptr=reinterpret_cast<uintptr_t>(manager);Patch185 manager_global(base+0x1bc73e4,&manager_ptr,4);
  auto native_modal=reinterpret_cast<bool(__thiscall*)(void*)>(base+0x1096240);
  for(unsigned a:{0u,1u,32u})for(unsigned b:{0u,1u,32u}){memcpy(manager+0x30,&a,4);memcpy(manager+0x3c,&b,4);assert(native214::modal(base)==native_modal(manager));++cases;}
  memset(manager,0,sizeof manager);
  assert(deck185::install_ui185(base));
  {auto physical=jump185(base+0x13f7b40,reinterpret_cast<void*>(&physical185));Patch185 p(base+0x13f7b40,physical.data(),5);
   uintptr_t mode_ptr=reinterpret_cast<uintptr_t>(&mode185),display_ptr=reinterpret_cast<uintptr_t>(&display_info185);Patch185 mode(base+0x1754574+0x248,&mode_ptr,4),display(base+0x1754574+0x30c,&display_ptr,4);
   uintptr_t renderer[8]{};renderer[0]=base+0x1754574;
   Node191 root,corner,button;State214 root_state,corner_state,button_state;uintptr_t sizes[]{0,reinterpret_cast<uintptr_t>(&state214)};
   Node191* widgets[]{&root,&corner,&button};State214* states[]{&root_state,&corner_state,&button_state};uintptr_t state_ptrs[3]{};
   for(unsigned n=0;n<3;++n){auto w=reinterpret_cast<uintptr_t>(widgets[n]);state_ptrs[n]=reinterpret_cast<uintptr_t>(states[n]);states[n]->table=reinterpret_cast<uintptr_t>(sizes);states[n]->owner=w;set191(*widgets[n],0,base+0x1704cdc);set191(*widgets[n],0xb8,state_ptrs[n]);set191(*widgets[n],0xb0,1);set191(*widgets[n],0xb4,reinterpret_cast<uintptr_t>(&state_ptrs[n]));set191(*widgets[n],0x94,n?reinterpret_cast<uintptr_t>(n==1?&root:&corner):0);widgets[n]->bytes[0xf5]=1;
    *reinterpret_cast<int*>(reinterpret_cast<char*>(states[n])+0xe4)=12;*reinterpret_cast<int*>(reinterpret_cast<char*>(states[n])+0xd0)=12;
   }
   auto rp=reinterpret_cast<uintptr_t>(&root),cp=reinterpret_cast<uintptr_t>(&corner),bp=reinterpret_cast<uintptr_t>(&button);set191(root,8,reinterpret_cast<uintptr_t>(renderer));set191(root,0x9c,1);set191(root,0xa0,reinterpret_cast<uintptr_t>(&cp));set191(corner,0x9c,1);set191(corner,0xa0,reinterpret_cast<uintptr_t>(&bp));
   Patch185 current(base+0x1bbe364,&rp,4);auto event=jump185(base+0xf945f0,reinterpret_cast<void*>(&event191));Patch185 notify(base+0xf945f0,event.data(),5);uintptr_t position=reinterpret_cast<uintptr_t>(&position_event191);Patch185 position_cb(base+0x1704cdc+0xc,&position,4);
   auto refresh=jump185(base+0xf87480,reinterpret_cast<void*>(&font214));Patch185 font_provider(base+0xf87480,refresh.data(),5);
   unsigned char disabled_draw=1;
   Patch185 skip_render(base+0x1bbeae0,&disabled_draw,1);
   uintptr_t graphics_table[2]{0,reinterpret_cast<uintptr_t>(&graphics_flags)};
   uintptr_t graphics_object=reinterpret_cast<uintptr_t>(graphics_table);
   auto graphics_ptr=reinterpret_cast<uintptr_t>(&graphics_object);
   auto view_owner=reinterpret_cast<uintptr_t>(&viewport_owner);
   Patch185 graphics(base+0x1c9a93c,&graphics_ptr,4),view_source(base+0x1754574+0x98,&view_owner,4);
   uintptr_t batch[]{base+0x1752370,0,0,0,reinterpret_cast<uintptr_t>(renderer)};
   uintptr_t text[]{base+0x1752488,0,reinterpret_cast<uintptr_t>(batch)};
   uintptr_t context[]{reinterpret_cast<uintptr_t>(batch),reinterpret_cast<uintptr_t>(text)};
   uintptr_t active_ptr=0;Patch185 active(base+0x1bbe368,&active_ptr,4);
   auto set_active=[&](uintptr_t value){DWORD old{},unused{};auto slot=reinterpret_cast<void*>(base+0x1bbe368);assert(VirtualProtect(slot,4,PAGE_READWRITE,&old));memcpy(slot,&value,4);assert(VirtualProtect(slot,4,old,&unused));};
   assert(deck85::install_tooltips(base)&&deck85::install_tooltips(base));
   for(auto dimensions:{std::pair<unsigned,unsigned>{1280,800},{1280,720}}){
    width185=dimensions.first;height185=dimensions.second;
    for(auto panel:{std::pair<unsigned,unsigned>{0,0},{1344,980}}){
     deck185::panel_extent191_set(panel.first,panel.second);
     const auto view=deck185::active_ui185();
     size214(root_state,float(view.width),float(view.height));
     size214(corner_state,240,112);xy191(corner,float(view.width)-242,float(view.height)-114);
     backend_offset214[0]=9;backend_offset214[1]=5;
     graphics_flags214[0x2b]=panel.first?1:0;
     deck85::Projection unavailable;
     deck85::campaign_tooltips={};
     assert(!deck85::tooltip_projection(cp,context,unavailable));
     set_active(cp);
     // Custom Battle setup uses the same native active slot and renderer,
     // without a campaign camera or a battle card manager owning this root.
     assert(!deck85::tooltip_projection(cp,context,unavailable));
     auto draw=reinterpret_cast<deck85::Draw>(base+deck85::draw_rva);
     assert(draw(&corner,context,0,1,1)==0&&!deck85::projection.renderer);
     ++setup_cases;
     with_campaign_ui(base,[&]{
      test_popup_projection(base,renderer,context,corner,view);
      corner.bytes[0xf5]=0;
      assert(!deck85::tooltip_projection(cp,context,unavailable));
      corner.bytes[0xf5]=1;
     });
     test_card_strip(base,rp,cp,context,view);++strip_cases;
     assert(!deck85::tooltip_projection(cp,context,unavailable));++setup_cases;
     set_active(0);
     assert(deck85::observe_campaign_tooltip(rp,cp));
     with_campaign_ui(base,[&]{
      test_popup_projection(base,renderer,context,corner,view);
      text[2]=0;assert(!deck85::tooltip_projection(cp,context,unavailable));text[2]=context[0];
      set191(corner,0x94,cp);assert(!deck85::tooltip_projection(cp,context,unavailable));set191(corner,0x94,rp);
     });
     assert(!deck85::tooltip_projection(cp,context,unavailable));++setup_cases;
     auto wide=deck85::placement({0,0},view.width*.6f,view.height*.5f,view);
     assert(wide.scale>1&&wide.scale<1.2f&&wide.to.x==12&&wide.to.y==12);
     assert(deck85::placement({0,0},float(view.width),float(view.height),view).scale==1);
     assert(deck85::placement({0,0},NAN,200,view).scale==1);
     cases+=16;
    }
    size214(corner_state,184,204);size214(button_state,80,80);xy191(root,0,0);xy191(corner,350,350);xy191(button,20,20);
    deck214::owner_root=0;
    deck214::Node before;
    assert(deck214::node(cp,before)&&deck214::decorate_corner(rp,cp));
    deck214::Node after;assert(deck214::node(cp,after));
    assert(std::fabs(after.width-before.width*1.15f)<.01f&&std::fabs(after.height-before.height*1.15f)<.01f);
    const auto refreshes=refreshes214;
    for(unsigned n=0;n<100;++n)assert(deck214::decorate_corner(rp,cp));
    assert(refreshes214==refreshes);cases+=2;
    for(auto* s:states){*reinterpret_cast<int*>(reinterpret_cast<char*>(s)+0xe4)=12;*reinterpret_cast<int*>(reinterpret_cast<char*>(s)+0xd0)=12;}
   }
  }
  {std::vector<unsigned char> root(0xe00),env(0x28400);auto r=reinterpret_cast<uintptr_t>(root.data()),e=reinterpret_cast<uintptr_t>(env.data());memcpy(root.data()+0x110,&e,4);Patch185 global(base+0x1bc8194,&r,4);
   auto get=jump185(base+0x321880,reinterpret_cast<void*>(&get_speed214)),set=jump185(base+0x10e58a0,reinterpret_cast<void*>(&set_speed214));Patch185 getter(base+0x321880,get.data(),5),setter(base+0x10e58a0,set.data(),5);native214::BattleSpeed speed;speed214=1.f;
   assert(speed.apply(base,r,e,false)&&std::fabs(speed214-.4f)<.001);assert(speed.apply(base,r,e,true)&&speed214==0);assert(speed.apply(base,r,e,true)&&std::fabs(speed214-.4f)<.001);assert(speed.apply(base,r,e,false)&&speed214==1);cases+=4;
   unsigned calls=speed_calls214;root[0xd18]=1;assert(!speed.apply(base,r,e,true));root[0xd18]=0;env[0x281e8]=1;assert(!speed.apply(base,r,e,true));env[0x281e8]=0;env[0x28254]=1;assert(!speed.apply(base,r,e,true));env[0x28254]=0;assert(!speed.apply(base,r+4,e,true)&&speed_calls214==calls);cases+=4;
  }
  for(auto rva:{deck85::draw_rva,deck85::origin_rvas[0],deck85::origin_rvas[1],deck85::size_rvas[0],deck85::size_rvas[1]}){
   auto target=reinterpret_cast<void*>(base+rva);
   assert(MH_DisableHook(target)==MH_OK&&MH_RemoveHook(target)==MH_OK);
  }
  deck85::image=0;deck85::original_draw=nullptr;deck85::campaign_tooltips={};
  for(auto target:deck185::native_ui185.targets){assert(MH_DisableHook(target)==MH_OK);assert(MH_RemoveHook(target)==MH_OK);}deck185::native_ui185={};}native214::admitted_image=0;assert(FreeLibrary(image));
 }
 std::cout<<"PASS native battle card-strip placement: "<<strip_cases
          <<" native-image cases with hover ownership, hidden/unrelated widget and fit checks\n";
 std::cout<<"PASS native Custom Battle setup tooltip bypass: "<<setup_cases
          <<" cases, including campaign/battle exit and stale observations\n";
 std::cout<<"PASS native hover draw admission, projection, compact bounds and state preservation\n";
 std::cout<<"PASS native214: "<<cases<<" original-image modal, geometry, projection, guard and speed-route cases; renderer providers are mocked; no gameplay\n";
}
