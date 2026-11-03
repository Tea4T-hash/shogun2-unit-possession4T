// Offline boundary checks only: no actual worker/Steam/game/UI startup.
#define S2UP165_LUA_FIXTURE 1
#define main prior_input_main180
#include "verify174.cpp"
#undef main
static std::wstring own165(){return L"fixture-not-used";}
static bool battle_ready(uintptr_t){return false;}
static bool caller165(void* value){return value&&*static_cast<unsigned*>(value)==1;}
#include "../../native/deck/frontend180.h"
struct NavigationHost180 {
 int moves{},clicks{},direction{};bool waiting{};
 bool pointer(float,float){++moves;waiting=false;return true;}
 bool scroll(float){return true;}void reset_cursor(){}
 void release_owned(bool,bool){}void clear_navigation(){waiting=false;direction=0;}
 void navigate(int value){direction=value;waiting=true;}bool navigation_pending()const{return waiting;}
 bool button(bool left,bool down){if(left&&down)++clicks;return true;}
};
int main(){
    unsigned one=1,two=2;unsigned checks=0;
    assert(s2up_menu_prepare180(&two)==0&&s2up_menu_poll180(nullptr)==0&&s2up_menu_stop180(&two)==0);++checks;
    assert(!menu180::prepare()&&!menu180::running&&!menu180::input_thread);++checks;
    menu180::remember("first",5);menu180::remember("second",6);assert(!strcmp(menu180::error_text,"first"));++checks;
    memset(menu180::error_text,0,sizeof(menu180::error_text));char long_text[900];memset(long_text,'A',sizeof(long_text));menu180::remember(long_text,sizeof(long_text));assert(strlen(menu180::error_text)==511);++checks;
    InterlockedExchange(&menu180::back_pending,1);menu180::running=1;assert(s2up_menu_poll180(&one)==1&&s2up_menu_cancel180(&one)==1);assert(s2up_menu_poll180(&one)==1&&!s2up_menu_cancel180(&one));++checks;
    menu180::stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);assert(menu180::stop_event);assert(s2up_menu_stop180(&one)==1&&WaitForSingleObject(menu180::stop_event,0)==WAIT_OBJECT_0);assert(!s2up_menu_poll180(&one));CloseHandle(menu180::stop_event);menu180::stop_event=nullptr;menu180::running=0;++checks;
    assert(menu180::focus_text("640,400")&&menu180::target_x==640&&menu180::target_y==400);assert(!menu180::focus_text("-1,2")&&!menu180::focus_text("32768,0")&&!menu180::focus_text("1,2,3"));++checks;
    deck179::Menu179 policy;NavigationHost180 host;deck174::Config config;deck174::Frame f;f.source=deck174::Source::Simulation;f.epoch=1;f.sequence=1;f.sample_us=1000000;f.active_buttons=deck174::all_buttons;f.connected=f.focused=f.native_bindings=true;bool back=false;
    auto step=[&](unsigned milliseconds){f.sequence++;f.sample_us+=uint64_t(milliseconds)*1000;policy.sample(f,f.sample_us,config,host,back);};
    step(8);f.buttons=deck174::bit(deck174::Button::Down);step(8);assert(host.direction==2&&host.moves==0);++checks;
    f.buttons|=deck174::bit(deck174::Button::A);step(8);assert(host.clicks==0);host.waiting=false;step(8);assert(host.clicks==1);++checks;
    f.buttons=0;step(8);f.left_stick={-1,0};step(8);assert(host.direction==3&&host.moves==0);++checks;
    f.focused=false;step(8);assert(!host.waiting);++checks;
    assert(menu180::focus_text("")&&menu180::target_x==-1);++checks;
    std::cout<<"PASS "<<checks<<" frontend180 native worker boundary checks; no actual worker or Deck execution\n";
}
