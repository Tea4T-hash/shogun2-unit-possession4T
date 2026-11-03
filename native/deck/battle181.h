#pragma once
// Included in the existing private adapter namespace, after the native host.
#include "presentation182.h"
#include "presentation188.h"
namespace controller181 {
static deck181::Controls controls;
static deck181::Channel channel;
static deck182::Presentation glyphs;
static deck181::Output previous;
static deck202::LookRate look202;
static deck201::LookQueue look201;static volatile LONG& hide_key=deck181::native_key_pending;
static float wheel{};
static unsigned sampled_lease{},sampled_actor{},sampled_unit{};
static unsigned hud_stage188{};static bool configured{};static volatile LONG reset_requested{},menu_open{};
static deck174::Context context(){if(menu_open)return deck174::Context::Menu;return state&&state->mode==POSSESSED&&!state->stop&&!state->release?deck174::Context::Possession:deck174::Context::RTS;}
static void clear_input(){
 InterlockedExchange(&reset_requested,1);deck181::Sample empty;channel.publish(empty);look201.clear();look202.clear();
}
static deck181::Output sample(const deck174::Frame& frame){
 if(!configured){controls=deck181::Controls(config);configured=true;}
 unsigned lease=state?state->lease_generation:0,actor=state?state->actor_id:0,unit=state?state->unit_id:0;
 if(InterlockedExchange(&reset_requested,0)||lease!=sampled_lease||actor!=sampled_actor||unit!=sampled_unit)controls.reset();
 sampled_lease=lease;sampled_actor=actor;sampled_unit=unit;
 auto output=controls.sample(frame,context(),frame.sample_us);
 if(output.accepted&&!output.help_blocked&&frame.focused&&output.context==deck174::Context::Possession&&!output.held(deck181::F10)&&!output.held(deck181::Escape))
  look202.publish({lease,actor,unit},frame.epoch,frame.sample_us,{output.stick_rate.x,output.stick_rate.y},config.stale_ms*1000);
 else look202.clear();
 if(!channel.shared)channel.open(GetCurrentProcessId(),true);
 deck181::Sample value;value.context=unsigned(output.context);value.focused=output.accepted&&frame.focused;value.tick=GetTickCount64();
 if(state&&output.context==deck174::Context::Possession){value.lease=state->lease_generation;value.actor=state->actor_id;value.unit=state->unit_id;}
 for(unsigned i=0;i<8;++i)value.keys[i]=output.keys[i];channel.publish(value);
 deck188::presentation188.publish(unsigned(output.context),output.help_open,output.help_page,hud_stage188);return output;
}
static void present(){
 if(!channel.shared||backend.state()!=deck174::BackendState::Ready)return;
 glyphs.poll(GetTickCount64(),[&](unsigned slot){deck181::Glyph value;return channel.glyph(slot,value);},[&](unsigned slot){
  int origin=slot<24?deck174::steam006::digital[slot].origin:deck174::steam006::analog_origin[slot==24?4:slot==25?5:slot-26];
  try{deck181::copy_glyph(channel,backend,slot,origin);}catch(...){}
 });
}
static void cancel_window(){
 if(previous.held(deck181::Q))command_native_cancel100();
 if(state&&previous.context==deck174::Context::Possession){InterlockedExchange(&state->aiming,0);}
 deck181::native_shortcuts183.clear();previous={};wheel=0;orbit_button154(false);look201.clear();look202.clear();
}

struct MenuHost {
 Identity identity;static inline WPARAM buttons{};static inline deck178::Cursor cursor;static inline float wheel{};
 bool current(){ContextProof p;return native_context(p)&&p.owner_thread&&p.focused&&p.blocked&&p.identity==identity;}
 bool position(RECT& rect,POINT& point){return current()&&GetClientRect(input_window,&rect)&&rect.right>=200&&rect.bottom>=200&&GetCursorPos(&point)&&ScreenToClient(input_window,&point);}
 bool pointer(float dx,float dy){RECT rect{};POINT point{};if(!position(rect,point))return false;int x=point.x,y=point.y;if(!cursor.move(x,y,rect.right,rect.bottom,dx*rect.right,-dy*rect.bottom))return false;POINT screen{x,y};if(!ClientToScreen(input_window,&screen)||!SetCursorPos(screen.x,screen.y))return false;CallWindowProcW(native_window_proc,input_window,WM_MOUSEMOVE,buttons,MAKELPARAM(x,y));return true;}
 bool button(bool left,bool down){RECT rect{};POINT point{};if(!position(rect,point))return false;WPARAM flag=left?MK_LBUTTON:MK_RBUTTON;if(down)buttons|=flag;else buttons&=~flag;CallWindowProcW(native_window_proc,input_window,left?(down?WM_LBUTTONDOWN:WM_LBUTTONUP):(down?WM_RBUTTONDOWN:WM_RBUTTONUP),buttons,MAKELPARAM(point.x,point.y));return true;}
 bool scroll(float delta){RECT rect{};POINT point{};if(!position(rect,point))return false;wheel+=delta*4.f;int notches=int(wheel);wheel-=notches;if(notches){ClientToScreen(input_window,&point);CallWindowProcW(native_window_proc,input_window,WM_MOUSEWHEEL,MAKEWPARAM(WORD(buttons),short(std::clamp(notches,-8,8)*WHEEL_DELTA)),MAKELPARAM(point.x,point.y));}return true;}
 void reset_cursor(){cursor.reset();}
 void release_owned(bool left,bool right){ContextProof p;if(native_context(p,true)&&p.identity==identity&&native_window_proc&&input_window&&(buttons||left||right)){CallWindowProcW(native_window_proc,input_window,WM_CANCELMODE,0,0);if(left||(buttons&MK_LBUTTON))CallWindowProcW(native_window_proc,input_window,WM_LBUTTONUP,0,MAKELPARAM(-1,-1));if(right||(buttons&MK_RBUTTON))CallWindowProcW(native_window_proc,input_window,WM_RBUTTONUP,0,MAKELPARAM(-1,-1));}buttons=0;wheel=0;cursor.reset();}
};
static deck178::Menu178 menu_policy;static uint64_t menu_buttons{};static bool in_menu{};
static void menu_dispatch(const deck181::Queued& queued,const ContextProof& proof){
 MenuHost host{proof.identity};bool back=false;
 if(!proof.blocked||!proof.focused||!proof.owner_thread){menu_policy.release(host);menu_buttons=0;in_menu=false;return;}
 cancel_window();in_menu=true;
 auto menu_frame=deck188::menu_frame188(queued.frame);
 bool good=menu_policy.sample(menu_frame,queued.frame.sample_us,config,host,back);
 if(good){bool pause=(queued.frame.buttons&deck174::bit(deck174::Button::Menu))&&!(menu_buttons&deck174::bit(deck174::Button::Menu));if(back||pause)InterlockedOr(&hide_key,2);menu_buttons=queued.frame.buttons;}else menu_buttons=0;
}

static bool hud_toggle188(const ContextProof& proof,bool possessed_mode){
 if(!proof.owner_thread||!proof.focused||proof.blocked||!proof.identity.root||
    (possessed_mode&&(!state||!enabled||state->mode!=POSSESSED||state->release||state->stop||dead_view()))||GetCurrentThreadId()!=GetWindowThreadProcessId(input_window,nullptr))return false;
 __try{
  auto root=proof.identity.root;unsigned native_hidden=field<unsigned char>(root,0xca0);if(native_hidden>1)return false;
  unsigned lease=state?state->lease_generation:0;
  unsigned next=possessed_mode?(hudcycle198::observe(root,lease,true,native_hidden)+1)%3:0;
  native_hidden=possessed_mode?unsigned(next!=0):!native_hidden;
  // Same guarded native function and four arguments as existing interface_restore
  // and original1210a10. Own mod visibility is separate from native RTS UI.
  using NativeUI=void(__thiscall*)(void*,unsigned,unsigned,unsigned,unsigned);
  reinterpret_cast<NativeUI>(base+0x10f1060)(reinterpret_cast<void*>(root),native_hidden,1,0,1);
  // Publish only after the native call completed and its flag agrees.
  if(field<unsigned char>(root,0xca0)!=native_hidden)return false;
  if(possessed_mode)hudcycle198::commit(root,lease,next);
  else hudcycle198::observe(root,lease,false,native_hidden);
  hud_stage188=next;hud_visibility_sync151(true,true);return true;
 }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
// Called from the existing owning-window input hook on PC and Deck alike.
// Native polling masks K separately so stock UI cannot toggle a second time.
static bool keyboard_hud198(UINT message,WPARAM key,LPARAM flags,const ContextProof& proof){
 if(message==WM_CHAR&&key=='k')key='K';
 if(key!='K'||(message!=WM_KEYDOWN&&message!=WM_KEYUP&&message!=WM_CHAR)||
    !hudcycle198::window_owned(keyboard_fps84()))return false;
 if(message==WM_KEYDOWN&&!(flags&(1L<<30))&&keyboard_fps84())hud_toggle188(proof,true);
 return true;
}
static void dispatch(const deck181::Queued& queued,const ContextProof& proof){
 const auto& output=queued.output;
 if(!deck174::valid(queued.frame,clock_us())||clock_us()-queued.frame.sample_us>uint64_t(config.stale_ms)*1000){cancel_window();clear_input();return;}
 if(output.help_blocked){MenuHost host{proof.identity};menu_policy.release(host);menu_buttons=0;in_menu=false;cancel_window();return;}
 if(output.context==deck174::Context::Menu){menu_dispatch(queued,proof);return;}
 if(in_menu){MenuHost host{proof.identity};menu_policy.release(host);menu_buttons=0;in_menu=false;}
 if(!output.accepted)return;
 if(output.help_blocked){cancel_window();return;}
 if(output.context==deck174::Context::RTS){
  bool hide=output.held(deck181::K)&&!previous.held(deck181::K),pause=output.held(deck181::Escape)&&!previous.held(deck181::Escape);
  if(previous.context!=deck174::Context::RTS)cancel_window();if(hide)hud_toggle188(proof,false);if(pause)InterlockedOr(&hide_key,2);previous=output;
  if(proof.permits_rts()){
   for(unsigned i=0;i<deck183::ShortcutCount;++i)if(output.shortcuts&(1u<<i))deck181::native_shortcuts183.put(i,GetTickCount64());
   if(output.fire_at_will){
    // Same native action1 used by10ce0e0 for battle_both_fire_at_will_toggle.
    // Reuse Halt's selection/queue preflight, not a guessed per-unit write.
    native_halt(base,proof.identity.root,proof.identity.environment,[](uintptr_t at){return field<uintptr_t>(at,0);},[&](){ContextProof current;return native_context(current)&&current.permits_rts()&&current.identity==proof.identity;},
     [&](uintptr_t root,unsigned,float){reinterpret_cast<void(__thiscall*)(void*,unsigned,float)>(base+0x10dfb80)(reinterpret_cast<void*>(root),1,0.f);});
   }
  }
  if(proof.permits_rts()&&state&&state->mode==ARMED&&output.held(deck181::F10)){
   InterlockedExchange(&entry_attempted,1);InterlockedExchange(&entry_pending,0);entry_failed(181);return;
  }
  if(proof.permits_rts()&&output.held(deck181::F9)){
   request_possession();
   if(state&&state->mode==ARMED&&state->acquire&&!state->release&&!state->stop){
    // This callback has already proved the native root/camera identity and
    // actual owning window thread. Preserve all constructor/selection guards.
    InterlockedExchange(&entry_attempted,1);InterlockedExchange(&entry_pending,0);
    deck_entry_dispatch181=GetCurrentThreadId();enter_selected_soldier();deck_entry_dispatch181=0;
    if(state->mode==ARMED&&!possessed)entry_failed(181);
   }
  }return;
 }
 if(output.context==deck174::Context::Possession&&output.held(deck181::F10)&&proof.focused&&proof.owner_thread&&state&&!state->stop&&queued.lease==state->lease_generation&&queued.actor==state->actor_id&&queued.unit==state->unit_id){weapon_window_proc(input_window,WM_KEYDOWN,VK_F10,0);cancel_window();return;}
 if(output.context!=deck174::Context::Possession||!proof.focused||!proof.owner_thread||proof.blocked||!state||state->mode!=POSSESSED||state->release||state->stop||dead_view()||
    queued.lease!=state->lease_generation||queued.actor!=state->actor_id||queued.unit!=state->unit_id){cancel_window();return;}
 if(output.held(deck181::F10)){command_native_cancel100();InterlockedExchange(&state->release,1);cancel_window();return;}
 if(output.held(deck181::P)){AcquireSRWLockExclusive(&view_lock152);++view_input152.toggles;ReleaseSRWLockExclusive(&view_lock152);}
 if(output.held(deck181::M)&&!previous.held(deck181::M)&&!charge_input_locked146())InterlockedExchange(&mount_request154,LONG(state->lease_generation));
 if(output.held(deck181::K)&&!previous.held(deck181::K))hud_toggle188(proof,true);
 if(output.held(deck181::Escape)&&!previous.held(deck181::Escape))InterlockedOr(&hide_key,2);
 if(output.held(deck181::Cancel)&&!previous.held(deck181::Cancel)&&command_placing100)command_native_cancel100();
 const unsigned commands[]={'Q','E','Z','C','R','F',VK_LBUTTON,VK_RBUTTON};
 for(unsigned key:commands)if(output.held(key)!=previous.held(key)){
  UINT message=key==VK_LBUTTON?(output.held(key)?WM_LBUTTONDOWN:WM_LBUTTONUP):key==VK_RBUTTON?(output.held(key)?WM_RBUTTONDOWN:WM_RBUTTONUP):(output.held(key)?WM_KEYDOWN:WM_KEYUP);
  weapon_window_proc(input_window,message,key==VK_LBUTTON||key==VK_RBUTTON?0:key,0);
 }
 orbit_button154(output.held(deck181::Middle));
 // Only displacement sources (pad/gyro) retain the existing worker path.
 // Stick turning is integrated by the owned camera callback, never twice.
 look201.add(output.look.x-output.stick_delta.x,output.look.y-output.stick_delta.y);
 wheel+=output.zoom*4.f;int notches=int(wheel);wheel-=notches;if(notches&&third_person152)view_wheel152(short(std::clamp(notches,-8,8)*WHEEL_DELTA));
 if(command_placing100&&(output.look.x||output.look.y))weapon_window_proc(input_window,WM_MOUSEMOVE,0,0);
 previous=output;
}
} // controller181
