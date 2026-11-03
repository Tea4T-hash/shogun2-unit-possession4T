#pragma once
#include "device192.h"
#include "frontend177.h" // retain verified file/glyph helpers, not old UI polling
#include "menu179.h"
#include "menu178.h"
#include "rts_schema178.h"
#include <cstring>
#include "glyphs181.h"
#include "presentation182.h"
#include "navigation182.h"
#include "campaign181.h"
#include "zoom181.h"
#include "input_trace181.h"
#include "native_guard175.h"
#include "campaign_keys185.h"
#include "cursor_idle185.h"
#include "native_input185.h"
#include "native_ui185.h"
#include "view_help186.h"
#include "shortcut_click184.h"
#include "help_input188.h"
#include "help_renderer188.h"
#include "cursor194.h"
#include "camera_edges205.h"

namespace menu181 {
static SRWLOCK lock=SRWLOCK_INIT;
static deck184::ShortcutClick shortcut_click184;
static deck185::CampaignKeys campaign_keys185;
static HANDLE stop_event,input_thread;
static volatile LONG running{},stopping{},ui_frames{},back_pending{},notified{};
static HWND game_window;
struct Snapshot {bool ready{},bindings{},failed{},focused{},glyphs{},glyph_failed{};int width{},height{};uint64_t viewport_tick{};};
static Snapshot snapshot;
static char error_text[512]{};
static bool ui_back,ui_pause;
static volatile LONG campaign_mode{},pause_pending{},next_pending{},previous_pending{};
static std::string root_token;static volatile LONG campaign_actions183{},button_epoch183{};
static volatile LONG pointer_mode181{};
// Unknown campaign state is modal until the original PanelManager getter reports.
static volatile LONG campaign_panel188{1};
static float camera_x{},camera_y{},camera_turn{},camera_zoom{};
static uint64_t camera_tick{};
static uintptr_t campaign_identity{};static DWORD ui_owner181{};static volatile LONG ui_tick181{};
static deck185::NativeInput native_input185;
static uintptr_t keyboard_image185{};static volatile LONG keyboard_allowed185{};
static bool __cdecl campaign_poll185(){
 bool valid=native_input185.original();const auto now=GetTickCount64();
 bool allowed=keyboard_allowed185&&campaign_mode&&GetForegroundWindow()==game_window&&DWORD(GetTickCount()-DWORD(ui_tick181))<250;
 unsigned native_action{};campaign_keys185.apply(reinterpret_cast<unsigned char*>(keyboard_image185+0x1b30f58),valid,allowed,now,button_epoch183,&native_action);
 // Selection reads the original UI modifier cache as well as DirectInput.
 // Refresh that cache on this native input thread after our held-bit merge;
 // worker mouse messages alone do not populate it. Exact body/RET8 guarded.
 if(valid&&allowed)reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(keyboard_image185+0xf92ae0)(0,0);
 // Clan is queued separately for the original toolbar button. Its callback
 // enters another component's Lua environment and does not belong in this poll.
 return valid;
}
static bool keyboard_start185(uintptr_t engine){
#ifdef S2UP165_LUA_FIXTURE
 (void)engine;return true;
#else
 if(!native_input185.enabled&&!deck185::native_admitted(engine,true))return false;
 if(!native_input185.target){if(!deck175::native_guards(engine))return false;keyboard_image185=engine;if(!native_input185.create(engine,&campaign_poll185))return false;}
 return native_input185.enable();
#endif
}
static bool keyboard_stop185(){InterlockedExchange(&keyboard_allowed185,0);campaign_keys185.clear();return native_input185.disable();}
static deck181::Channel controller_channel;
static deck182::Presentation controller_glyphs;
static bool attach(const std::string& token){
 bool campaign=token.compare(0,9,"campaign:")==0,menu=token.compare(0,5,"menu:")==0;
 if((!campaign&&!menu)||token.size()>96||token.find_first_of("\r\n")!=token.npos)return false;
 AcquireSRWLockExclusive(&lock);root_token=token;InterlockedIncrement(&button_epoch183);InterlockedExchange(&campaign_actions183,0);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=0;campaign_identity=0;ui_owner181=GetCurrentThreadId();InterlockedExchange(&ui_tick181,LONG(GetTickCount()));ReleaseSRWLockExclusive(&lock);
 deck185::panel_extent191_set(0,0);InterlockedExchange(&campaign_mode,campaign?1:0);InterlockedExchange(&campaign_panel188,1);InterlockedExchange(&pointer_mode181,0);return true;
}
static bool detach(const std::string& token);
static bool camera_step(unsigned axis,int sign){
 bool result=false;const float quantum=axis==2?.01f:.125f;
 AcquireSRWLockExclusive(&lock);
 auto& value=axis==0?camera_x:axis==1?camera_y:camera_turn;
 if(GetTickCount64()>=camera_tick&&GetTickCount64()-camera_tick<=100&&value*sign>=quantum){value-=sign*quantum;result=true;}
 ReleaseSRWLockExclusive(&lock);return result;
}

static volatile LONG nav_pending{},nav_waiting{},target_pending{};
static int ui_direction,target_x,target_y;
static Snapshot read(){AcquireSRWLockShared(&lock);auto s=snapshot;ReleaseSRWLockShared(&lock);return s;}
static void remember(const char* data,size_t length){
    AcquireSRWLockExclusive(&lock);
    if(!error_text[0]&&data&&length){length=std::min(length,sizeof(error_text)-1);memcpy(error_text,data,length);error_text[length]=0;}
    ReleaseSRWLockExclusive(&lock);
}
static void notify(){
    if(InterlockedCompareExchange(&notified,1,0))return;
    char detail[512]{};AcquireSRWLockShared(&lock);memcpy(detail,error_text,sizeof(detail));ReleaseSRWLockShared(&lock);
    wchar_t message[1100]{};wcscpy_s(message,L"Private Steam Deck test181 - menu status\n\n");
    wchar_t decoded[600]{};MultiByteToWideChar(CP_UTF8,0,detail,-1,decoded,600);wcscat_s(message,decoded);
    wcscat_s(message,L"\n\nPlease photograph this message. This is a one-time failure report, not a recording or timed control test.");
    MessageBoxW(game_window,message,L"Unit Possession - private Deck test181",MB_OK|MB_ICONWARNING);
}
static BOOL CALLBACK find_window(HWND window,LPARAM value){
    DWORD pid{};GetWindowThreadProcessId(window,&pid);RECT rect{};
    if(pid==GetCurrentProcessId()&&IsWindowVisible(window)&&GetClientRect(window,&rect)&&rect.right>=640&&rect.bottom>=400){
        *reinterpret_cast<HWND*>(value)=window;return FALSE;
    }return TRUE;
}
struct Host {
    HWND window{};WPARAM buttons{},modifiers183{};deck178::Cursor cursor;float wheel{};deck185::CursorIdle cursor_idle;
    void navigate(int direction){InterlockedExchange(&nav_waiting,1);InterlockedCompareExchange(&nav_pending,direction,0);}
    bool navigation_pending()const{return nav_waiting!=0;}
    void clear_navigation(){InterlockedExchange(&nav_pending,0);InterlockedExchange(&nav_waiting,0);InterlockedExchange(&target_pending,0);}
    bool target(int x,int y){
        RECT rect{};POINT point{};if(!position(rect,point)||x<0||y<0||x>=rect.right||y>=rect.bottom)return false;
        POINT screen{x,y};if(!ClientToScreen(window,&screen)||!SetCursorPos(screen.x,screen.y))return false;
        cursor.reset();return PostMessageW(window,WM_MOUSEMOVE,buttons,MAKELPARAM(x,y))!=FALSE;
    }
    bool focused()const{DWORD pid{};return IsWindow(window)&&GetWindowThreadProcessId(window,&pid)&&pid==GetCurrentProcessId()&&GetForegroundWindow()==window;}
    bool position(RECT& rect,POINT& point){
        if(!focused()||!GetClientRect(window,&rect)||rect.right<200||rect.bottom<200||rect.right>32767||rect.bottom>32767||
           !GetCursorPos(&point)||!ScreenToClient(window,&point))return false;
        point.x=std::clamp<LONG>(point.x,0,rect.right-1);point.y=std::clamp<LONG>(point.y,0,rect.bottom-1);return true;
    }
    bool pointer(float dx,float dy){
        clear_navigation();RECT rect{};POINT point{};if(!position(rect,point))return false;int x=point.x,y=point.y;
        if(!cursor.move(x,y,rect.right,rect.bottom,double(dx)*rect.right,-double(dy)*rect.bottom))return false;
        cursor_idle.activity(GetTickCount64());
        POINT screen{x,y};if(!ClientToScreen(window,&screen)||!SetCursorPos(screen.x,screen.y))return false;
        return PostMessageW(window,WM_MOUSEMOVE,buttons,MAKELPARAM(x,y))!=FALSE;
    }
    bool cursor_hidden_idle(){CURSORINFO info{sizeof info};return GetCursorInfo(&info)&&cursor_idle.ready(GetTickCount64(),(info.flags&CURSOR_SHOWING)!=0);}
    bool park_camera(){
        // A cursor left at the screen edge must not add native edge-scrolling
        // to stick panning. Do not change the player's edge-scroll preference.
        RECT rect{};POINT point{};if(!position(rect,point))return false;
        if(point.x==rect.right/2&&point.y==rect.bottom/2)return true;
        return target(rect.right/2,rect.bottom/2);
    }
    bool escape(){
        RECT rect{};POINT point{};if(!position(rect,point))return false;
        // Native focused-window Escape: the game decides skip/back. Never
        // change global key state or send input to another application.
        if(!PostMessageW(window,WM_KEYDOWN,VK_ESCAPE,LPARAM(0x00010001)))return false;
        return PostMessageW(window,WM_KEYUP,VK_ESCAPE,LPARAM(0xc0010001))!=FALSE;
    }
    bool button(bool left,bool down){
        RECT rect{};POINT point{};if(!position(rect,point))return false;
        cursor_idle.activity(GetTickCount64());
        WPARAM flag=left?MK_LBUTTON:MK_RBUTTON;if(down)buttons|=flag;else buttons&=~flag;
        UINT message=left?(down?WM_LBUTTONDOWN:WM_LBUTTONUP):(down?WM_RBUTTONDOWN:WM_RBUTTONUP);
        return PostMessageW(window,message,buttons|modifiers183,MAKELPARAM(point.x,point.y))!=FALSE;
    }
    bool scroll(float delta){
        if(!std::isfinite(delta)||std::fabs(delta)>2)return false;RECT rect{};POINT point{};if(!position(rect,point))return false;
        wheel+=delta*4.f;int notches=int(wheel);if(!notches)return true;wheel-=notches;
        if(!ClientToScreen(window,&point))return false;
        return PostMessageW(window,WM_MOUSEWHEEL,MAKEWPARAM(WORD(buttons),short(std::clamp(notches,-8,8)*WHEEL_DELTA)),MAKELPARAM(point.x,point.y))!=FALSE;
    }
    bool zoom(float delta){
        RECT rect{};POINT point{};if(!campaign_mode||!position(rect,point))return false;
        AcquireSRWLockExclusive(&lock);bool ok=zoom181::enqueue(camera_zoom,delta);ReleaseSRWLockExclusive(&lock);return ok;
    }
    void reset_cursor(){cursor.reset();}
    void release_owned(bool left,bool right){
        if(IsWindow(window)&&(buttons||left||right)){
            PostMessageW(window,WM_CANCELMODE,0,0);
            // Release outside the client so focus loss cannot click a menu.
            if(left||(buttons&MK_LBUTTON))PostMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(-1,-1));
            if(right||(buttons&MK_RBUTTON))PostMessageW(window,WM_RBUTTONUP,0,MAKELPARAM(-1,-1));
        }buttons=modifiers183=0;wheel=0;reset_cursor();
    }
};
template<class H>static bool menu_press181(bool campaign,const deck174::Edges& edges,H& host){
 if(!(edges.down&deck174::bit(deck174::Button::Menu)))return false;
 if(campaign){InterlockedExchange(&pause_pending,1);return true;}
 return host.escape();
}
template<class H>static void campaign_cursor181(const deck174::Frame& frame,deck174::Vec2 pan,H& host){
 if((pan.x||pan.y)&&host.cursor_hidden_idle()&&!(frame.buttons&deck174::bit(deck174::Button::RightPadTouch))&&frame.left_trigger<=.15f&&frame.right_trigger<=.15f)host.park_camera();
}
static deck174::CameraAxes178 campaign_axes194(const deck174::Frame& frame){
 // Campaign bumpers cycle selection. L3 retains its Ctrl selection modifier
 // and additionally enables the existing 2x pan rate while the stick moves.
 auto camera_frame=frame;
 camera_frame.buttons&=~(deck174::bit(deck174::Button::L1)|deck174::bit(deck174::Button::R1));
 return deck174::camera_axes178(camera_frame,frontend177::config);
}
template<class H>static bool campaign_zoom181(const deck174::Frame& frame,float dt,H& host){
 if(!std::isfinite(dt)||dt<0||dt>.05f)return false;
 // Component deadzone keeps a tiny vertical offset during a horizontal turn
 // from slowly changing zoom. Reuse native wheel handling and its own limits.
 const float zoom=deck174::deadzone({0,frame.right_stick.y},frontend177::config.right_deadzone).y;
 return !zoom||host.zoom(-zoom*dt*2.f); // owner-requested campaign stick inversion only
}
static uintptr_t campaign_camera181(uintptr_t engine){
 if(!engine)return 0;
 return deck181::campaign_camera(engine,[](uintptr_t at){uintptr_t value{};SIZE_T got{};return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&value,sizeof value,&got)&&got==sizeof value?value:uintptr_t{};});
}
static bool camera_apply181(){
#ifndef S2UP165_LUA_FIXTURE
 if(!campaign_mode||GetCurrentThreadId()!=ui_owner181||deck177::battle_request().active()||!game_window||GetWindowThreadProcessId(game_window,nullptr)!=GetCurrentThreadId())return false;
 const auto engine=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll"));
 static uintptr_t checked_engine{};if(engine!=checked_engine){if(!engine||!deck175::native_guards(engine))return false;checked_engine=engine;}
 auto camera=campaign_camera181(engine);deck181::CampaignStep step;
 AcquireSRWLockExclusive(&lock);step={campaign_identity,camera_x,camera_y,camera_turn,camera_zoom,camera_tick};camera_x=camera_y=camera_turn=camera_zoom=0;ReleaseSRWLockExclusive(&lock);
 bool moved=deck181::apply_campaign(step,camera,GetTickCount64(),[&](unsigned slot,float amount){
  // Native methods retain map bounds/target updates and orientation mechanics.
  using Move=void(__thiscall*)(void*,float);
  if(campaign_camera181(engine)==camera)reinterpret_cast<Move>(*reinterpret_cast<uintptr_t*>(engine+0x16209f8+slot))(reinterpret_cast<void*>(camera),amount);
 });
 if(!moved||!step.zoom)return moved;
 static uintptr_t zoom_engine{};if(engine!=zoom_engine){if(!zoom181::admitted(engine))return false;zoom_engine=engine;}
 if(campaign_camera181(engine)!=camera)return false;
 // Native relative zoom: float amount, native multiplier/clamping/scene update.
 // Same owning UI-thread gate as pan/yaw; no worker call or camera-field write.
 using RelativeZoom=void(__thiscall*)(void*,float);
 reinterpret_cast<RelativeZoom>(engine+zoom181::relative_rva)(reinterpret_cast<void*>(camera),step.zoom);
 return true;
#else
 return false;
#endif
}
static DWORD WINAPI worker(void*){
    frontend177::owner=GetCurrentThreadId();deck179::Menu179 policy;deck178::Menu178 pointer_policy;Host host;bool prior_pointer=false;
    uint64_t next_attempt{},next_glyph{},previous_sample{};unsigned glyph_attempts{};bool prior_campaign=false;deck174::Normalizer campaign_edges;deck183::CampaignButtons campaign_buttons183;deck188::HelpInput help188;deck188::HelpRenderer guide188;deck194::CursorRenderer cursor194;LONG button_epoch_seen183=button_epoch183;
    try{for(;;){
        if(WaitForSingleObject(stop_event,8)!=WAIT_TIMEOUT)break;
        auto engine=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll"));
        HWND present188=GetForegroundWindow();DWORD present_pid188{};GetWindowThreadProcessId(present188,&present_pid188);
        guide188.tick(present_pid188==GetCurrentProcessId()?present188:nullptr,[&](unsigned slot,deck181::Glyph& glyph){return controller_channel.glyph(slot,glyph);});
        if(deck177::battle_request().active()||deck177::busy(deck177::Owner::Battle)){
            cursor194.reset();
            if(!keyboard_stop185()){remember("Campaign input handoff could not restore its native poll",sizeof("Campaign input handoff could not restore its native poll")-1);continue;}
            if(frontend177::lease.handle){AcquireSRWLockExclusive(&lock);shortcut_click184.cancel(host);ReleaseSRWLockExclusive(&lock);policy.release(host);pointer_policy.release(host);frontend177::backend.close();deck181::Sample blank;controller_channel.publish(blank);frontend177::lease.release();}
            AcquireSRWLockExclusive(&lock);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=0;campaign_identity=0;snapshot={};ReleaseSRWLockExclusive(&lock);
            deck181::trace_input(0,unsigned(frontend177::backend.state()),0,host.window,0,0,0);previous_sample=0;campaign_edges.reset();campaign_buttons183.reset();help188.reset();InterlockedExchange(&campaign_actions183,0);continue;
        }
        if(!deck177::claim_frontend(frontend177::lease)){cursor194.reset();continue;}
        if(deck177::battle_request().active()){cursor194.reset();if(keyboard_stop185())frontend177::lease.release();continue;}
        // Intro/menu transitions may replace the native window. Never keep
        // sending input to a destroyed trailer window.
        if(host.window&&!IsWindow(host.window)){policy.release(host);pointer_policy.release(host);host.window=nullptr;game_window=nullptr;}
        HWND candidate=GetForegroundWindow();DWORD candidate_pid{};RECT candidate_rect{};
        if(candidate&&candidate!=host.window&&GetWindowThreadProcessId(candidate,&candidate_pid)&&candidate_pid==GetCurrentProcessId()&&
           IsWindowVisible(candidate)&&GetClientRect(candidate,&candidate_rect)&&candidate_rect.right>=640&&candidate_rect.bottom>=400){
            policy.release(host);pointer_policy.release(host);host.window=candidate;game_window=candidate;
        }
        if(!host.window){EnumWindows(find_window,reinterpret_cast<LPARAM>(&host.window));game_window=host.window;}

        auto now=GetTickCount64();auto& backend=frontend177::backend;
        const bool campaign=campaign_mode!=0;const bool panel=campaign&&campaign_panel188!=0;
        if(campaign&&host.focused()&&DWORD(GetTickCount()-DWORD(ui_tick181))<250){
            if(!keyboard_start185(engine)){InterlockedExchange(&keyboard_allowed185,0);campaign_keys185.clear();}
        }else if(!keyboard_stop185())continue;
        InterlockedExchange(&keyboard_allowed185,campaign&&host.focused()&&native_input185.enabled?1:0);
        if(campaign!=prior_campaign){AcquireSRWLockExclusive(&lock);shortcut_click184.cancel(host);ReleaseSRWLockExclusive(&lock);policy.release(host);pointer_policy.release(host);campaign_edges.reset();campaign_buttons183.reset();help188.reset();InterlockedExchange(&campaign_actions183,0);previous_sample=0;prior_campaign=campaign;}
        if(button_epoch_seen183!=button_epoch183){button_epoch_seen183=button_epoch183;campaign_edges.reset();campaign_buttons183.reset();help188.reset();InterlockedExchange(&campaign_actions183,0);}
        backend.select_context(campaign?deck174::Context::Campaign:deck174::Context::Menu);
        if(!backend.initialized()&&now>=next_attempt){next_attempt=now+2000;backend.open_existing(frontend177::utf8(frontend177::manifest).c_str(),true);}
        if(InterlockedExchange(&target_pending,0)){
            int x{},y{};AcquireSRWLockShared(&lock);x=target_x;y=target_y;ReleaseSRWLockShared(&lock);
            if(x>=0)host.target(x,y);InterlockedExchange(&nav_waiting,0);
        }
        bool pointer_mode=pointer_mode181!=0;
        if(pointer_mode!=prior_pointer){policy.release(host);pointer_policy.release(host);campaign_edges.reset();campaign_buttons183.reset();help188.reset();InterlockedExchange(&campaign_actions183,0);previous_sample=0;host.clear_navigation();prior_pointer=pointer_mode;}
        deck174::Frame frame;bool good=backend.sample(now*1000,host.focused(),frame);bool back=false;
        deck181::trace_input(1,unsigned(backend.state()),unsigned(campaign?deck174::Context::Campaign:deck174::Context::Menu),host.window,0,0,0,good?&frame:nullptr);
        if(good){
            auto menu_frame=campaign?deck188::campaign_menu_frame189(frame,panel):deck188::menu_frame188(frame);
            host.modifiers183=campaign?((frame.buttons&deck174::bit(deck174::Button::L4)?MK_SHIFT:0)|(frame.buttons&(deck174::bit(deck174::Button::L5)|deck174::bit(deck174::Button::L3))?MK_CONTROL:0)):0;
            // Directional menus are deferred. Only right-pad touch moves the cursor;
            // a missing Lua navigation acknowledgement must never gate A/click.
            menu_frame.left_stick={};menu_frame.buttons&=~(deck174::bit(deck174::Button::Up)|deck174::bit(deck174::Button::Down)|deck174::bit(deck174::Button::Left)|deck174::bit(deck174::Button::Right));
            deck174::Edges edges;bool armed=campaign_edges.accept(frame,now*1000,frontend177::config,edges);
            AcquireSRWLockExclusive(&lock);
            bool shortcut_owns184=shortcut_click184.phase!=0;
            shortcut_click184.step(now,unsigned(ui_frames),host,campaign&&host.focused()&&armed&&frame.left_trigger<frontend177::config.trigger_press&&frame.right_trigger<frontend177::config.trigger_press);
            ReleaseSRWLockExclusive(&lock);
            auto help_result188=armed?help188.sample(frame,edges,now*1000,false):deck188::HelpInput::Result{};
            deck188::presentation188.publish(unsigned(campaign?deck174::Context::Campaign:deck174::Context::Menu),help188.open,help188.page,0);
            if(help_result188.blocked){
                campaign_buttons183.reset();
                pointer_policy.release(host);policy.release(host);campaign_keys185.clear(true);InterlockedExchange(&campaign_actions183,0);InterlockedExchange(&back_pending,0);InterlockedExchange(&pause_pending,0);
                AcquireSRWLockExclusive(&lock);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=0;ReleaseSRWLockExclusive(&lock);
                if(!help188.open)campaign_edges.reset();
            }else{
            campaign_keys185.hold(campaign&&armed?unsigned(host.modifiers183):0,now,button_epoch183);
            if(!shortcut_owns184)pointer_policy.sample(menu_frame,now*1000,frontend177::config,host,back);
            if(back){if(DWORD(GetTickCount()-DWORD(ui_tick181))<250)InterlockedExchange(&back_pending,1);else host.escape();}
            if(armed){
                if(DWORD(GetTickCount()-DWORD(ui_tick181))<250)menu_press181(campaign,edges,host);else menu_press181(false,edges,host);
                if(campaign){
                    unsigned actions=campaign_buttons183.sample(frame,edges,now*1000,!panel);
                    if(panel)actions&=(deck183::Arts|deck183::Finance|deck183::Diplomacy|deck183::Clan);
                    if(actions&&!shortcut_owns184)pointer_policy.release(host); // panel/selection commands cancel an owned drag safely
                    if(actions&deck183::Next)InterlockedExchange(&next_pending,1);if(actions&deck183::Previous)InterlockedExchange(&previous_pending,1);
                    campaign_keys185.put(actions&~deck183::Clan,now,button_epoch183);
                    InterlockedOr(&campaign_actions183,LONG(actions&~(deck183::Next|deck183::Previous|(deck185::CampaignKeys::handled&~deck183::Clan))));
                    const float dt=previous_sample&&now>=previous_sample?float(std::min<uint64_t>(now-previous_sample,50))*.001f:0;
                    if(!panel){
                    const auto axes=campaign_axes194(frame);
                    auto pan=axes.pan;
                    if(!shortcut_owns184)campaign_cursor181(frame,pan,host);
                    const float speed=20.f*frontend177::config.camera_pan_sensitivity*(axes.fast?2.f:1.f);
                    const auto camera=campaign_camera181(engine);
                    AcquireSRWLockExclusive(&lock);
                    if(camera!=campaign_identity){camera_x=camera_y=camera_turn=camera_zoom=0;campaign_identity=camera;}
                    camera_x=deck182::accumulate(camera_x,pan.x,dt*speed,4.f);camera_y=deck182::accumulate(camera_y,pan.y,dt*speed,4.f);
                    camera_turn=deck182::accumulate(camera_turn,axes.turn,dt*1.5f*frontend177::config.camera_turn_sensitivity,.4f);
                    camera_tick=now;ReleaseSRWLockExclusive(&lock);
                    campaign_zoom181(frame,dt,host);
                    } // original accepted map math only runs while no native panel is open
                }
            }
            } // help owns its buttons and cannot pass commands to the game
            cursor194.tick(host.window,armed&&!help_result188.blocked, (frame.buttons&deck174::bit(deck174::Button::RightPadTouch))!=0,now);
            previous_sample=now;
            if(!controller_channel.shared)controller_channel.open(GetCurrentProcessId(),true);
            deck181::Sample presence;presence.tick=now;presence.focused=host.focused()&&armed;presence.context=unsigned(campaign&&!panel&&!help188.open?deck174::Context::Campaign:deck174::Context::Menu);controller_channel.publish(presence);
            controller_glyphs.poll(now,[&](unsigned slot){deck181::Glyph glyph;return controller_channel.glyph(slot,glyph);},[&](unsigned slot){int origin=slot<24?deck174::steam006::digital[slot].origin:deck174::steam006::analog_origin[slot==24?4:slot==25?5:slot-26];try{deck181::copy_glyph(controller_channel,backend,slot,origin);}catch(...){} });
        }
        else{cursor194.reset();campaign_keys185.clear(true);AcquireSRWLockExclusive(&lock);shortcut_click184.cancel(host);ReleaseSRWLockExclusive(&lock);policy.release(host);pointer_policy.release(host);campaign_edges.reset();campaign_buttons183.reset();help188.reset();InterlockedExchange(&campaign_actions183,0);previous_sample=0;AcquireSRWLockExclusive(&lock);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=0;campaign_identity=0;ReleaseSRWLockExclusive(&lock);deck181::Sample blank;controller_channel.publish(blank);}
        if(good&&!frontend177::glyphs_ready&&glyph_attempts<3&&now>=next_glyph){
            ++glyph_attempts;next_glyph=now+1500;try{frontend177::prepare_glyphs();}catch(const std::exception& e){if(glyph_attempts==3)remember(e.what(),strlen(e.what()));}
        }
        Snapshot state;auto status=backend.state();state.ready=status==deck174::BackendState::Ready;state.bindings=status==deck174::BackendState::WaitingBindings;
        state.failed=status!=deck174::BackendState::Ready&&status!=deck174::BackendState::WaitingBindings&&status!=deck174::BackendState::WaitingSteam&&status!=deck174::BackendState::WaitingDevice&&status!=deck174::BackendState::Idle;
        RECT viewport{};
        if(host.focused()&&GetClientRect(host.window,&viewport)&&viewport.right>=640&&viewport.bottom>=360&&viewport.right<=32767&&viewport.bottom<=32767){state.width=viewport.right;state.height=viewport.bottom;state.viewport_tick=now;}
        state.focused=host.focused();state.glyphs=frontend177::glyphs_ready;state.glyph_failed=glyph_attempts>=3&&!state.glyphs;
        AcquireSRWLockExclusive(&lock);snapshot=state;bool error=error_text[0]!=0;ReleaseSRWLockExclusive(&lock);
        // Root initialization can precede a long intro. Absence of an early
        // frame alone is not an initialization error; report real errors only.
        if(error&&!notified&&host.focused()){policy.release(host);pointer_policy.release(host);notify();policy.normalizer.reset();}
    }}catch(const std::exception& e){remember(e.what(),strlen(e.what()));if(host.focused())notify();}
    AcquireSRWLockExclusive(&lock);shortcut_click184.cancel(host);ReleaseSRWLockExclusive(&lock);
    if(!keyboard_stop185()){remember("Campaign input poll could not stop",sizeof("Campaign input poll could not stop")-1);return 0;}
    policy.release(host);pointer_policy.release(host);if(frontend177::lease.handle){deck181::Sample blank;controller_channel.publish(blank);}frontend177::stop();
    AcquireSRWLockExclusive(&lock);if(stop_event){CloseHandle(stop_event);stop_event=nullptr;}snapshot={};ReleaseSRWLockExclusive(&lock);
    InterlockedExchange(&running,0);return 0;
}
static bool prepare(){
    if(!device192::enabled())return false;

    if(running&&stopping)return true; // next root pulse completes the restart after the owned worker exits
    if(InterlockedCompareExchange(&running,1,0))return !stopping;
    try{
        if(input_thread){CloseHandle(input_thread);input_thread=nullptr;}
        if(!frontend177::prepare(true))throw std::runtime_error("Native menu resource preparation failed");
        InterlockedExchange(&stopping,0);InterlockedExchange(&ui_frames,0);InterlockedExchange(&back_pending,0);
        AcquireSRWLockExclusive(&lock);error_text[0]=0;snapshot={};ReleaseSRWLockExclusive(&lock);
        stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!stop_event)throw std::runtime_error("Native menu stop event unavailable");
        input_thread=CreateThread(nullptr,0,worker,nullptr,0,nullptr);
        if(!input_thread)throw std::runtime_error("Native menu input worker could not start");return true;
    }catch(const std::exception& e){remember(e.what(),strlen(e.what()));if(stop_event){CloseHandle(stop_event);stop_event=nullptr;}frontend177::stop();InterlockedExchange(&running,0);
#ifndef S2UP165_LUA_FIXTURE
        notify(); // bounded startup failure only; normal startup is silent
#endif
        return false;}
}
static bool stop(){
    InterlockedExchange(&stopping,1);AcquireSRWLockShared(&lock);if(stop_event)SetEvent(stop_event);ReleaseSRWLockShared(&lock);return true;
}
static bool poll(){InterlockedExchange(&ui_tick181,LONG(GetTickCount()));ui_pause=InterlockedExchange(&pause_pending,0)!=0;ui_back=InterlockedExchange(&back_pending,0)!=0;ui_direction=InterlockedExchange(&nav_pending,0);return running&&!stopping;}
static bool detach(const std::string& token){
 AcquireSRWLockExclusive(&lock);bool same=token==root_token;if(same){root_token.clear();InterlockedExchange(&campaign_actions183,0);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=ui_tick181=0;campaign_identity=0;InterlockedExchange(&campaign_mode,0);}ReleaseSRWLockExclusive(&lock);
 // Root destruction is a context change, not the lifetime of Steam Input.
 return true;
}
static std::string argument(void* lua){
#ifndef S2UP165_LUA_FIXTURE
    // Read-only Lua5.1 C API inside the admitted immutable engine. No retained
    // lua_State and no number-to-string coercion that could mutate the stack.
    auto base=reinterpret_cast<unsigned char*>(GetModuleHandleW(L"Empire.Retail.dll"));
    const unsigned char expected_type[]={0xff,0x74,0x24,0x08,0xff,0x74,0x24,0x08,0xe8,0xa3,0xed,0xff,0xff,0x83,0xc4,0x08};
    const unsigned char expected_string[]={0x56,0xff,0x74,0x24,0x0c,0x8b,0x74,0x24,0x0c,0x56,0xe8,0x01,0xef,0xff,0xff,0x8b};
    unsigned char actual[16]{};SIZE_T read{};
    auto matches=[&](unsigned offset,const unsigned char* wanted){return ReadProcessMemory(GetCurrentProcess(),base+offset,actual,16,&read)&&read==16&&!memcmp(actual,wanted,16);};
    if(!base||!matches(0xea3a90,expected_type)||!matches(0xea3930,expected_string))return {};
    using Type=int(__cdecl*)(void*,int);using String=const char*(__cdecl*)(void*,int,size_t*);
    if(reinterpret_cast<Type>(base+0xea3a90)(lua,1)!=4)return {};
    size_t size{};const char* text=reinterpret_cast<String>(base+0xea3930)(lua,1,&size);return text?std::string(text,std::min<size_t>(size,511)):std::string{};
#else
    (void)lua;return {}; // fixture never calls game APIs or opens a dialog
#endif
}
static void report(void* lua){auto text=argument(lua);remember(text.data(),text.size());}
static bool focus_text(const std::string& text){
    if(text=="cursor"||text=="directional"){
        InterlockedExchange(&pointer_mode181,text=="cursor"?1:0);InterlockedExchange(&nav_pending,0);InterlockedExchange(&nav_waiting,0);InterlockedExchange(&target_pending,0);return true;
    }
    int x=-1,y=-1;
    if(!text.empty()){
        if(text.size()>12)return false;unsigned a=0,b=0;bool comma=false,first=false,second=false;
        for(char c:text){if(c==','&&!comma){comma=true;continue;}if(c<'0'||c>'9')return false;
            auto& value=comma?b:a;value=value*10+unsigned(c-'0');if(value>32767)return false;if(comma)second=true;else first=true;}
        if(!comma||!first||!second)return false;x=int(a);y=int(b);
    }
    AcquireSRWLockExclusive(&lock);target_x=x;target_y=y;ReleaseSRWLockExclusive(&lock);
    InterlockedExchange(&target_pending,1);return true;
}

static HANDLE campaign_guard181=INVALID_HANDLE_VALUE;
#include "viewport181.h"
#include "context181.h"
static bool panel_state188(const std::string& value){
 if(!campaign_mode||GetCurrentThreadId()!=ui_owner181)return false;
 if(value!="open"&&value!="map"&&value!="unknown")return false;
 LONG modal=value=="map"?0:1;
 if(value!="unknown")deck185::sync_panels191(modal!=0);
 if(InterlockedExchange(&campaign_panel188,modal)!=modal){
  InterlockedIncrement(&button_epoch183);campaign_keys185.clear(true);
  InterlockedExchange(&campaign_actions183,0);InterlockedExchange(&next_pending,0);InterlockedExchange(&previous_pending,0);
  AcquireSRWLockExclusive(&lock);camera_x=camera_y=camera_turn=camera_zoom=0;camera_tick=0;ReleaseSRWLockExclusive(&lock);
 }return true;
}
static bool back188(){return campaign_mode&&GetCurrentThreadId()==ui_owner181&&campaign_keys185.put(deck183::Back,GetTickCount64(),button_epoch183);}
static bool context_state(void* lua){
#ifdef S2UP165_LUA_FIXTURE
 (void)lua;return context181::remember("fixture-context-not-executed",1);
#else
 return context181::observe(lua,argument(lua));
#endif
}
static bool prepare_campaign(void* lua){
#ifdef S2UP_FRONTEND181_STANDALONE
 (void)lua;return false;
#else
 if(!caller165(lua))return false;
 StateGuard165 guard;
 if(campaign_guard181!=INVALID_HANDLE_VALUE){CloseHandle(campaign_guard181);campaign_guard181=INVALID_HANDLE_VALUE;}
 auto selection=read_text(game165+L"\\used_mods.txt");
 auto bytes=workshop168::selected_root(game165,selection,hash_file(own165()),"ui/campaign ui/layout.root.luac",60729,"d14bea292474e62bb6e1e7a5f4938cdfa121b5bcca3003d693c9a31d5d703b50");
 campaign_guard181=workshop165::transient_root168(own165()+L".campaign.luac",bytes);return true;
#endif
}
}
#define MENU181(name,value) extern "C" __declspec(dllexport) int __cdecl s2up_menu_##name##181(void* lua){try{if(!caller165(lua))return 0;return (value)?1:0;}catch(const std::exception& e){menu181::remember(e.what(),strlen(e.what()));return 0;}}
MENU181(prepare,menu181::prepare())
MENU181(supported,device192::enabled())
MENU181(context,menu181::context_state(lua))
MENU181(camera,!menu181::campaign_panel188&&menu181::camera_apply181())
MENU181(panel188,menu181::panel_state188(menu181::argument(lua)))
MENU181(back188,menu181::back188())
MENU181(stop,menu181::stop())
MENU181(poll,menu181::poll())
MENU181(active,menu181::running&&!menu181::stopping)
MENU181(ready,menu181::read().ready)
MENU181(bindings,menu181::read().bindings)
MENU181(failed,menu181::read().failed)
MENU181(focused,menu181::read().focused)
MENU181(legend184,false) // retired export; no legend generation
MENU181(glyphs,menu181::read().glyphs)
MENU181(glyph_failed,menu181::read().glyph_failed)
MENU181(cancel,menu181::ui_back)
MENU181(heartbeat,(InterlockedIncrement(&menu181::ui_frames),true))
MENU181(up,menu181::ui_direction==1)
MENU181(down,menu181::ui_direction==2)
MENU181(left,menu181::ui_direction==3)
MENU181(right,menu181::ui_direction==4)
MENU181(focus,menu181::focus_text(menu181::argument(lua)))
MENU181(error,(menu181::report(lua),false))
MENU181(attach,menu181::attach(menu181::argument(lua)))
MENU181(detach,menu181::detach(menu181::argument(lua)))
MENU181(campaign_prepare,menu181::prepare_campaign(lua))
MENU181(pause,menu181::ui_pause)
MENU181(next,(InterlockedExchange(&menu181::next_pending,0)!=0))
MENU181(previous,(InterlockedExchange(&menu181::previous_pending,0)!=0))
MENU181(home,((InterlockedAnd(&menu181::campaign_actions183,~LONG(1))&1)!=0))
MENU181(arts,((InterlockedAnd(&menu181::campaign_actions183,~LONG(2))&2)!=0))
MENU181(finance,((InterlockedAnd(&menu181::campaign_actions183,~LONG(4))&4)!=0))
MENU181(diplomacy,((InterlockedAnd(&menu181::campaign_actions183,~LONG(8))&8)!=0))
MENU181(clan,((InterlockedAnd(&menu181::campaign_actions183,~LONG(16))&16)!=0))
MENU181(end_turn,((InterlockedAnd(&menu181::campaign_actions183,~LONG(32))&32)!=0))
MENU181(quick_save,((InterlockedAnd(&menu181::campaign_actions183,~LONG(64))&64)!=0))
MENU181(move_speed,((InterlockedAnd(&menu181::campaign_actions183,~LONG(128))&128)!=0))
MENU181(encyclopedia,((InterlockedAnd(&menu181::campaign_actions183,~LONG(256))&256)!=0))
MENU181(pan_left,menu181::camera_step(0,-1))
MENU181(pan_right,menu181::camera_step(0,1))
MENU181(pan_up,menu181::camera_step(1,1))
MENU181(pan_down,menu181::camera_step(1,-1))
MENU181(turn_left,menu181::camera_step(2,-1))
MENU181(turn_right,menu181::camera_step(2,1))
#undef MENU181

// Read-only, fixed-size in-memory evidence for the private hardware check.
// No file, periodic recording, retained lua_State or native validity reset.
extern "C" __declspec(dllexport) const menu181::context181::Trace* __cdecl S2UPDeckContextProbe181(){return &menu181::context181::trace;}

extern "C" __declspec(dllexport) int __cdecl s2up_menu_viewport181(void* lua){
 try{if(!caller165(lua))return 0;auto value=menu181::read();return menu181::viewport181::result(lua,value.width,value.height,value.viewport_tick,GetTickCount64());}catch(...){return 0;}
}

#if defined(S2UP165_LUA_FIXTURE) && !defined(S2UP_FRONTEND181_STANDALONE)
// Inert offline fixture only; absent from the distributed native bridge.
#define VIEWPORT181_FIXTURE(name,w,h,age) extern "C" __declspec(dllexport) int __cdecl s2up_viewport_##name##_fixture181(void* lua){if(!caller165(lua))return 0;AcquireSRWLockExclusive(&menu181::lock);menu181::snapshot.width=w;menu181::snapshot.height=h;menu181::snapshot.viewport_tick=GetTickCount64()-(age);ReleaseSRWLockExclusive(&menu181::lock);return 1;}
VIEWPORT181_FIXTURE(valid,1280,800,0)
VIEWPORT181_FIXTURE(stale,1280,800,10000)
VIEWPORT181_FIXTURE(invalid,-1,800,0)
#undef VIEWPORT181_FIXTURE
#endif

extern "C" __declspec(dllexport) const deck181::InputTrace* __cdecl S2UPDeckInputProbe181(){return &deck181::input_trace;}
extern "C" __declspec(dllexport) const deck185::UiTrace187* __cdecl S2UPDeckUiProbe187(){return &deck185::trace187;}

// Explicitly owner-approved game-window route for original campaign buttons.
// Same owned foreground window as Host.button; never SendInput/global OS state.
extern "C" __declspec(dllexport) int __cdecl s2up_menu_click183(void* lua){
 try{
  if(!caller165(lua)||!menu181::campaign_mode||GetCurrentThreadId()!=menu181::ui_owner181||deck177::battle_request().active())return 0;
  HWND window=menu181::game_window;DWORD pid{};if(!window||GetForegroundWindow()!=window||!GetWindowThreadProcessId(window,&pid)||pid!=GetCurrentProcessId())return 0;
  const auto value=menu181::argument(lua);if(value.empty()||value.size()>11)return 0;unsigned x=0,y=0;bool second=false,digit=false;
  for(char c:value){if(c==','&&!second&&digit){second=true;digit=false;continue;}if(c<'0'||c>'9')return 0;unsigned& n=second?y:x;n=n*10+unsigned(c-'0');if(n>32767)return 0;digit=true;}
  if(!second||!digit)return 0;
  RECT r{};POINT cursor{};if(!GetClientRect(window,&r)||r.right<640||r.bottom<360||r.right>32767||r.bottom>32767||x>=unsigned(r.right)||y>=unsigned(r.bottom)||!GetCursorPos(&cursor)||!ScreenToClient(window,&cursor))return 0;
  AcquireSRWLockExclusive(&menu181::lock);
  bool queued=menu181::shortcut_click184.begin(x,y,GetTickCount64(),unsigned(menu181::ui_frames));
  ReleaseSRWLockExclusive(&menu181::lock);return queued?1:0;
 }catch(...){return 0;}
}

extern "C" __declspec(dllexport) int __cdecl s2up_menu_ui_viewport185181(void* lua){try{
 if(!caller165(lua)||GetCurrentThreadId()!=menu181::ui_owner181)return 0;
 auto size=deck185::active_ui185();return menu181::viewport181::result(lua,int(size.width),int(size.height),GetTickCount64(),GetTickCount64());
}catch(...){return 0;}}
