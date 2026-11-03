// Offline boundary checks only: no actual worker/Steam/game/UI startup.
#define S2UP165_LUA_FIXTURE 1
#define main prior_input_main178
#include "verify174.cpp"
#undef main
static std::wstring own165(){return L"fixture-not-used";}
static bool battle_ready(uintptr_t){return false;}
static bool caller165(void* value){return value&&*static_cast<unsigned*>(value)==1;}
#include "../../native/deck/frontend178.h"
int main(){
    unsigned one=1,two=2;unsigned checks=0;
    assert(s2up_menu_prepare178(&two)==0&&s2up_menu_poll178(nullptr)==0&&s2up_menu_stop178(&two)==0);++checks;
    assert(!menu178::prepare()&&!menu178::running&&!menu178::input_thread);++checks;
    menu178::remember("first",5);menu178::remember("second",6);assert(!strcmp(menu178::error_text,"first"));++checks;
    memset(menu178::error_text,0,sizeof(menu178::error_text));char long_text[900];memset(long_text,'A',sizeof(long_text));menu178::remember(long_text,sizeof(long_text));assert(strlen(menu178::error_text)==511);++checks;
    InterlockedExchange(&menu178::back_pending,1);menu178::running=1;assert(s2up_menu_poll178(&one)==1&&s2up_menu_cancel178(&one)==1);assert(s2up_menu_poll178(&one)==1&&!s2up_menu_cancel178(&one));++checks;
    menu178::stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);assert(menu178::stop_event);assert(s2up_menu_stop178(&one)==1&&WaitForSingleObject(menu178::stop_event,0)==WAIT_OBJECT_0);assert(!s2up_menu_poll178(&one));CloseHandle(menu178::stop_event);menu178::stop_event=nullptr;menu178::running=0;++checks;
    std::cout<<"PASS "<<checks<<" frontend178 native worker boundary checks; no actual worker or Deck execution\n";
}
