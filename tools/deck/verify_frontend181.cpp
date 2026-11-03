// Offline boundary checks only: no actual worker/Steam/game/UI startup.
#define S2UP165_LUA_FIXTURE 1
#define S2UP_FRONTEND181_STANDALONE 1
#define main prior_input_main181
#include "verify174.cpp"
#undef main
static std::wstring own165(){return L"fixture-not-used";}
static bool battle_ready(uintptr_t){return false;}
static bool caller165(void* value){return value&&*static_cast<unsigned*>(value)==1;}
#include "../../native/deck/frontend181.h"
struct NavigationHost181 {
 int moves{},clicks{},direction{},escapes{},parks{};bool waiting{};
 bool hidden{};bool cursor_hidden_idle(){return hidden;}
 bool park_camera(){++parks;return true;}
 bool escape(){++escapes;return true;}
 bool pointer(float,float){++moves;waiting=false;return true;}
 float scrolled{};bool scroll(float value){scrolled+=value;return true;}bool zoom(float value){scrolled+=value;return true;}void reset_cursor(){}
 void release_owned(bool,bool){}void clear_navigation(){waiting=false;direction=0;}
 void navigate(int value){direction=value;waiting=true;}bool navigation_pending()const{return waiting;}
 bool button(bool left,bool down){if(left&&down)++clicks;return true;}
};
struct Numbers181 {float values[2]{};unsigned count{};};
static void __cdecl capture181(void* state,float value){auto p=static_cast<Numbers181*>(state);assert(p->count<2);p->values[p->count++]=value;}
int main(){
    unsigned one=1,two=2;unsigned checks=0;
    assert(s2up_menu_prepare181(&two)==0&&s2up_menu_poll181(nullptr)==0&&s2up_menu_stop181(&two)==0);++checks;
    assert(!menu181::prepare()&&!menu181::running&&!menu181::input_thread);++checks;
    menu181::remember("first",5);menu181::remember("second",6);assert(!strcmp(menu181::error_text,"first"));++checks;
    memset(menu181::error_text,0,sizeof(menu181::error_text));char long_text[900];memset(long_text,'A',sizeof(long_text));menu181::remember(long_text,sizeof(long_text));assert(strlen(menu181::error_text)==511);++checks;
    InterlockedExchange(&menu181::back_pending,1);menu181::running=1;assert(s2up_menu_poll181(&one)==1&&s2up_menu_cancel181(&one)==1);assert(s2up_menu_poll181(&one)==1&&!s2up_menu_cancel181(&one));++checks;
    menu181::stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);assert(menu181::stop_event);assert(s2up_menu_stop181(&one)==1&&WaitForSingleObject(menu181::stop_event,0)==WAIT_OBJECT_0);assert(!s2up_menu_poll181(&one));CloseHandle(menu181::stop_event);menu181::stop_event=nullptr;menu181::running=0;++checks;
    assert(menu181::focus_text("640,400")&&menu181::target_x==640&&menu181::target_y==400);assert(!menu181::focus_text("-1,2")&&!menu181::focus_text("32768,0")&&!menu181::focus_text("1,2,3"));++checks;
    deck179::Menu179 policy;NavigationHost181 host;deck174::Config config;deck174::Frame f;f.source=deck174::Source::Simulation;f.epoch=1;f.sequence=1;f.sample_us=1000000;f.active_buttons=deck174::all_buttons;f.connected=f.focused=f.native_bindings=true;bool back=false;
    auto step=[&](unsigned milliseconds){f.sequence++;f.sample_us+=uint64_t(milliseconds)*1000;policy.sample(f,f.sample_us,config,host,back);};
    step(8);f.buttons=deck174::bit(deck174::Button::Down);step(8);assert(host.direction==2&&host.moves==0);++checks;
    f.buttons|=deck174::bit(deck174::Button::A);step(8);assert(host.clicks==0);host.waiting=false;step(8);assert(host.clicks==1);++checks;
    f.buttons=0;step(8);f.left_stick={-1,0};step(8);assert(host.direction==3&&host.moves==0);++checks;
    f.focused=false;step(8);assert(!host.waiting);++checks;
    assert(menu181::focus_text("")&&menu181::target_x==-1);++checks;
    assert(menu181::attach("campaign:123")&&menu181::campaign_mode);assert(!menu181::attach("invalid"));++checks;
    assert(menu181::attach("menu:456")&&!menu181::campaign_mode);assert(menu181::detach("campaign:123"));++checks;
    assert(menu181::focus_text("cursor")&&menu181::pointer_mode181&&!menu181::nav_waiting);++checks;
    assert(!menu181::focus_text("cursor-junk")&&menu181::pointer_mode181);assert(menu181::focus_text("directional")&&!menu181::pointer_mode181);++checks;
    deck178::Menu178 pointer;NavigationHost181 pointer_host;f.buttons=0;f.focused=true;f.left_stick={};f.sequence++;f.sample_us+=8000;pointer.sample(f,f.sample_us,config,pointer_host,back);f.left_stick.x=1;f.sequence++;f.sample_us+=16000;assert(pointer.sample(f,f.sample_us,config,pointer_host,back)&&pointer_host.moves==1);f.buttons=deck174::bit(deck174::Button::A);f.sequence++;f.sample_us+=8000;assert(pointer.sample(f,f.sample_us,config,pointer_host,back)&&pointer_host.clicks==1);++checks;
    assert(menu181::focus_text("cursor")&&menu181::attach("menu:new-root")&&!menu181::pointer_mode181);++checks;
    NavigationHost181 menu_host;deck174::Edges menu_edge;menu_edge.down=deck174::bit(deck174::Button::Menu);InterlockedExchange(&menu181::pause_pending,0);assert(menu181::menu_press181(false,menu_edge,menu_host)&&menu_host.escapes==1&&!menu181::pause_pending);++checks;
    menu_edge={};assert(!menu181::menu_press181(false,menu_edge,menu_host)&&menu_host.escapes==1);++checks;
    menu_edge.down=deck174::bit(deck174::Button::Menu);assert(menu181::menu_press181(true,menu_edge,menu_host)&&menu_host.escapes==1&&menu181::pause_pending);++checks;
    menu181::Host no_window;assert(!no_window.escape());++checks;
    assert(menu181::viewport181::valid(1280,800,1000,1250)&&!menu181::viewport181::valid(639,800,1000,1000)&&!menu181::viewport181::valid(1280,359,1000,1000)&&!menu181::viewport181::valid(32768,800,1000,1000));++checks;
    assert(!menu181::viewport181::valid(1280,800,0,1000)&&!menu181::viewport181::valid(1280,800,1000,1251)&&!menu181::viewport181::valid(1280,800,1001,1000));++checks;
    Numbers181 numbers;assert(menu181::viewport181::push(&numbers,1280,800,1000,1000,capture181)==2&&numbers.count==2&&numbers.values[0]==1280&&numbers.values[1]==800);++checks;
    Numbers181 bad;assert(!menu181::viewport181::push(&bad,-1,800,1000,1000,capture181)&&!bad.count);assert(!menu181::viewport181::push(&bad,1280,800,1000,1251,capture181)&&!bad.count);assert(!menu181::viewport181::push(&bad,1280,800,1000,1000,nullptr)&&!bad.count);++checks;
    assert(!s2up_menu_viewport181(nullptr)&&!s2up_menu_viewport181(&two)&&!s2up_menu_viewport181(&one));++checks;
    NavigationHost181 campaign_host;f.buttons=0;f.left_trigger=f.right_trigger=0;menu181::campaign_cursor181(f,{0,1},campaign_host);assert(campaign_host.parks==0);campaign_host.hidden=true;menu181::campaign_cursor181(f,{0,1},campaign_host);assert(campaign_host.parks==1);++checks;
    menu181::campaign_cursor181(f,{},campaign_host);assert(campaign_host.parks==1);f.buttons=deck174::bit(deck174::Button::RightPadTouch);menu181::campaign_cursor181(f,{1,0},campaign_host);assert(campaign_host.parks==1);++checks;
    f.buttons=0;f.left_trigger=1;menu181::campaign_cursor181(f,{1,0},campaign_host);f.left_trigger=0;f.right_trigger=1;menu181::campaign_cursor181(f,{1,0},campaign_host);assert(campaign_host.parks==1);++checks;
    menu181::Host residual;residual.wheel=.875f;residual.reset_cursor();assert(residual.wheel==.875f);residual.release_owned(false,false);assert(residual.wheel==0);++checks;
    assert(menu181::attach("campaign:detach-check"));menu181::running=1;menu181::stopping=0;assert(menu181::detach("campaign:detach-check")&&menu181::running&&!menu181::stopping&&!menu181::campaign_mode);menu181::running=0;++checks;
    NavigationHost181 zoom_host;f.right_stick={1,.005f};assert(menu181::campaign_zoom181(f,.02f,zoom_host)&&zoom_host.scrolled==0);f.right_stick={0,1};assert(menu181::campaign_zoom181(f,.02f,zoom_host)&&zoom_host.scrolled<0);float amount=zoom_host.scrolled;f.right_stick={0,-1};assert(menu181::campaign_zoom181(f,.02f,zoom_host)&&zoom_host.scrolled>amount);assert(!menu181::campaign_zoom181(f,.06f,zoom_host));++checks;
    // Campaign-only L3 boost; shoulders cannot add turn, Ctrl route is separate.
    f.left_stick={.4f,.8f};f.right_stick={.2f,.6f};f.buttons=0;
    const auto ordinary=menu181::campaign_axes194(f);assert(!ordinary.fast);
    f.buttons=deck174::bit(deck174::Button::L3)|deck174::bit(deck174::Button::L1)|deck174::bit(deck174::Button::R1);
    const auto fast=menu181::campaign_axes194(f);assert(fast.fast&&fast.pan.x==ordinary.pan.x&&fast.pan.y==ordinary.pan.y&&fast.turn==ordinary.turn&&fast.pitch==ordinary.pitch);
    std::cout<<"PASS "<<checks<<" frontend181 native worker boundary checks; no actual worker or Deck execution\n";
}
