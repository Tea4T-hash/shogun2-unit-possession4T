#define NOMINMAX
#define S2UP_CURSOR194_FIXTURE
#include "../../native/deck/cursor194.h"
#include "../../native/deck/controls181.h"
#include "../../native/deck/campaign_keys185.h"
#include "../../native/deck/menu178.h"
#include "../../native/deck/help_input188.h"
#include <cassert>
#include <iostream>
using namespace deck174;
static unsigned checks{};
#define CHECK(v) do{assert(v);++checks;}while(0)
static Frame frame(unsigned n,uint64_t buttons=0){Frame f;f.sequence=n;f.epoch=1;f.sample_us=1000000+uint64_t(n)*8000;f.buttons=buttons;f.source=Source::Simulation;f.active_buttons=all_buttons;f.connected=f.focused=f.native_bindings=true;return f;}
struct Mouse {unsigned left_down{},left_up{},right_down{},right_up{};float x{},y{};void release_owned(bool,bool){}void reset_cursor(){}bool pointer(float a,float b){x+=a;y+=b;return true;}bool scroll(float){return true;}bool button(bool left,bool down){++(left?(down?left_down:left_up):(down?right_down:right_up));return true;}};
int main(){
 // Complete campaign button policy, then original keyboard consumer where used.
 const std::pair<Button,unsigned> campaign[]={{Button::X,deck183::Home},{Button::Up,deck183::Arts},{Button::Down,deck183::Finance},{Button::Left,deck183::Diplomacy},{Button::Right,deck183::Clan},{Button::A,deck183::MoveSpeed},{Button::R5,deck183::MoveSpeed},{Button::L1,deck183::Previous},{Button::R1,deck183::Next}};
 for(auto v:campaign){deck183::CampaignButtons c;auto f=frame(1,bit(v.first));Edges e;e.down=bit(v.first);CHECK(c.sample(f,e,f.sample_us)==v.second);e={};CHECK(c.sample(f,e,f.sample_us+8000)==0);}
 for(auto button:{Button::Y,Button::R4}){
  const unsigned expected=button==Button::Y?deck183::EndTurn:deck183::QuickSave;
  deck183::CampaignButtons c;auto f=frame(1,bit(button));Edges e;
  CHECK(!c.sample(f,e,1000000));CHECK(!c.sample(f,e,1599999));CHECK(c.sample(f,e,1600000)==expected);CHECK(!c.sample(f,e,5000000));
  // Old implementation consumed the hold while modal; new one starts a fresh
  // interval after returning to map and never fires a leftover partial hold.
  CHECK(!c.sample(f,e,6000000,false));CHECK(!c.sample(f,e,8000000,false));CHECK(!c.sample(f,e,9000000,true));CHECK(!c.sample(f,e,9599999));CHECK(c.sample(f,e,9600000)==expected);
  c.reset();CHECK(!c.sample(f,e,10000000));CHECK(c.sample(f,e,10600000)==expected);
 }
 const std::pair<unsigned,unsigned> scans[]={{deck183::Arts,0x32},{deck183::Finance,0x31},{deck183::Diplomacy,0x25},{deck183::EndTurn,0x1c},{deck183::MoveSpeed,0x39},{deck183::Back,0x01}};
 for(auto p:scans){deck185::CampaignKeys q;unsigned char keys[256]{};CHECK(q.put(p.first,1000,3));CHECK(q.apply(keys,true,true,1001,3));CHECK(keys[p.second]==0x80);memset(keys,0,256);CHECK(!q.apply(keys,true,true,1002,3)&&!keys[p.second]);}
 {deck185::CampaignKeys q;unsigned char keys[256]{};unsigned native{};CHECK(q.put(deck183::Clan,1000,3)&&q.apply(keys,true,true,1001,3,&native)&&native==deck183::Clan&&!keys[0x26]);}
 // RT/LT have identical select/order meaning through actual menu transforms.
 for(bool campaign_mode:{false,true})for(bool right_trigger:{false,true}){deck178::Menu178 m;Mouse h;bool back{};auto convert=[&](Frame f){return campaign_mode?deck188::campaign_menu_frame189(f,false):deck188::menu_frame188(f);};auto f=convert(frame(1));CHECK(m.sample(f,f.sample_us,{},h,back));f=frame(2);(right_trigger?f.right_trigger:f.left_trigger)=1;f=convert(f);CHECK(m.sample(f,f.sample_us,{},h,back));CHECK(right_trigger?(h.left_down==1&&!h.right_down):(h.right_down==1&&!h.left_down));f=convert(frame(3));CHECK(m.sample(f,f.sample_us,{},h,back));CHECK(right_trigger?h.left_up==1:h.right_up==1);}
 // Every direct possession button reaches its existing native key consumer.
 const std::pair<Button,unsigned> possession[]={{Button::A,deck181::F},{Button::B,deck181::M},{Button::X,deck181::One},{Button::Y,deck181::F10},{Button::L1,deck181::Q},{Button::L3,deck181::Shift},{Button::L4,deck181::Middle},{Button::L5,deck181::Control},{Button::R4,deck181::E},{Button::R5,deck181::C},{Button::Up,deck181::T},{Button::Down,deck181::G},{Button::Menu,deck181::Escape}};
 for(auto p:possession){deck181::Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);f=frame(2,bit(p.first));auto out=c.sample(f,Context::Possession,f.sample_us);CHECK(out.accepted&&out.held(p.second));f=frame(3);CHECK(!c.sample(f,Context::Possession,f.sample_us).held(p.second));}
 for(bool aim:{false,true}){deck181::Controls c;auto f=frame(1);c.sample(f,Context::Possession,f.sample_us);for(unsigned n=2;n<8;++n){f=frame(n);if(n%2==0)(aim?f.left_trigger:f.right_trigger)=1;auto out=c.sample(f,Context::Possession,f.sample_us);CHECK(out.held(aim?deck181::Aim:deck181::Primary)==(n%2==0));}}
 // X is a native run/walk shortcut only in RTS; holding never repeats.
 {deck181::Controls c;auto f=frame(1);c.sample(f,Context::RTS,f.sample_us);f=frame(2,bit(Button::X));CHECK(c.sample(f,Context::RTS,f.sample_us).shortcuts==(1u<<deck183::ToggleRun));for(unsigned n=3;n<30;++n){f=frame(n,bit(Button::X));CHECK(!c.sample(f,Context::RTS,f.sample_us).shortcuts);}f=frame(30);c.sample(f,Context::RTS,f.sample_us);f=frame(31,bit(Button::X));CHECK(c.sample(f,Context::RTS,f.sample_us).shortcuts==(1u<<deck183::ToggleRun));}
 // Cursor lifecycle: no window/permission, first touch, continuous rest,
 // native idle, window transition, focus loss, clock rollback.
 {deck194::CursorActivity a;CHECK(!a.update(1,true,false,100));CHECK(a.update(1,true,true,200));CHECK(a.update(1,true,false,2199));CHECK(!a.update(1,true,false,2200));CHECK(a.update(1,true,true,3000));CHECK(a.update(1,true,true,9000));CHECK(!a.update(2,true,false,9001));CHECK(a.update(2,true,true,9002));CHECK(!a.update(2,false,true,9003));CHECK(!a.update(2,true,false,9004));CHECK(a.update(2,true,true,10000));CHECK(!a.update(2,true,false,9000));CHECK(!a.update(0,true,true,10001));}
 CHECK(deck194::cursor_window(nullptr,WM_NCHITTEST,0,0)==HTTRANSPARENT);CHECK(deck194::cursor_window(nullptr,WM_MOUSEACTIVATE,0,0)==MA_NOACTIVATE);
 // Actual DrawIconEx and layered DIB upload, never show a window or send input.
 HWND owner=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"owned cursor fixture",WS_POPUP,0,0,1280,800,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);CHECK(owner);
 DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 for(unsigned n=0;n<80;++n){deck194::CursorRenderer r;auto cursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(n%2?32512:32649));CHECK(cursor&&r.fixture_render(owner,cursor));CHECK(!IsWindowVisible(r.fixture_window())&&GetWindow(r.fixture_window(),GW_OWNER)==owner);const auto& p=r.fixture_pixels();CHECK(!p.empty()&&p.size()<=128*128);CHECK(std::any_of(p.begin(),p.end(),[](uint32_t x){return x>>24;}));CHECK(std::any_of(p.begin(),p.end(),[](uint32_t x){return !x;}));}
 CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==before);DestroyWindow(owner);
 std::cout<<"PASS bindings194: "<<checks<<" actual control/native queue/menu routing/cursor lifecycle/render assertions; no game or system input\n";
}
