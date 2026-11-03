"""Connect context controls only in the isolated private build snapshot."""
from pathlib import Path
import json,hashlib
from adapt_runtime175 import substitute
import native_defaults181
ADAPTER='native/outputs/shogun2-accuracy90-preview/src/adapter.cpp'
QTE='native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp'
HUD='native/outputs/shogun2-ammunition83-preview/hud-src/hud.cpp'
RENDERERS=['native/outputs/shogun2-ammunition83-preview/hud-src/hud_renderer.h','native/outputs/shogun2-authentic-hud80-preview/src/hud_renderer.h']
ROOT=Path(__file__).resolve().parents[2]
def generate(snapshot):
 snapshot=Path(snapshot).resolve()
 if snapshot==ROOT or ROOT.is_relative_to(snapshot):raise ValueError('Use isolated source snapshot')
 changed={}
 def transform(path,fn):
  p=snapshot/path;before=p.read_text(encoding='utf-8');after=fn(before)
  if before==after:raise ValueError('Missing181 anchor: '+path)
  receipt=substitute(p,[(before,after)]);changed[path]=receipt
 def keyboard_merge(text):return text.replace('GetAsyncKeyState(', 'deck181::key_state(')
 # The normal source/Windows build are preserved. Every changed complete text
 # is reversibly compared by the existing snapshot adapter.
 for p in (snapshot/'native/outputs/shogun2-accuracy90-preview/src').iterdir():
  if p.suffix not in ('.h','.cpp')or p.name.startswith(('verify','launcher')):continue
  if 'GetAsyncKeyState('in p.read_text(encoding='utf-8'):
   transform(p.relative_to(snapshot).as_posix(),keyboard_merge)
 transform(ADAPTER,lambda s:s.replace('#include "identity.h"','#include "identity.h"\n#include "../../../deck/channel181.h"')
  .replace('    if(deck175_window(window,message,w,l))return 0;', '    if(message==WM_NCDESTROY&&window==input_window){auto prior=native_window_proc;deck175_window(window,message,w,l);input_window=nullptr;native_window_proc=nullptr;return prior?CallWindowProcW(prior,window,message,w,l):DefWindowProcW(window,message,w,l); }\n    if(deck175_window(window,message,w,l))return 0;')
  .replace('    if(input_window)return;','    if(input_window&&IsWindow(input_window)&&window==input_window)return;\n    RECT deck_rect{};if(!window||!IsWindowVisible(window)||!GetClientRect(window,&deck_rect)||deck_rect.right<640||deck_rect.bottom<400)return;\n    if(input_window&&IsWindow(input_window)){if(reinterpret_cast<WNDPROC>(GetWindowLongPtrW(input_window,GWLP_WNDPROC))!=&weapon_window_proc)return;SetWindowLongPtrW(input_window,GWLP_WNDPROC,reinterpret_cast<LONG_PTR>(native_window_proc));}\n    input_window=nullptr;native_window_proc=nullptr;')
  .replace('static __declspec(thread) LONG camera_callback_depth;', 'static __declspec(thread) LONG camera_callback_depth;\nstatic __declspec(thread) DWORD deck_entry_dispatch181;\n#include "../../../deck/handoff181.h"')
  .replace('camera_entry_owner(window_thread,state->camera_callback_thread,state->camera_input_thread),camera_callback_depth)', 'deck181::entry_owner(window_thread,state->camera_callback_thread,state->camera_input_thread,deck_entry_dispatch181),camera_callback_depth)')
  .replace('        require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Hook enable failed");','        require(MH_EnableHook(MH_ALL_HOOKS)==MH_OK,"Hook enable failed");\n        if(device192::enabled())require(keyboard_owner196.adopt_enabled(),"Deck keyboard owner admission failed");')
  .replace('        if(MH_DisableHook(keyboard_poll_target84)!=MH_OK)result=MH_UNKNOWN;','        {auto keyboard_stop196=MH_DisableHook(keyboard_poll_target84);if(keyboard_stop196!=MH_OK&&keyboard_stop196!=MH_ERROR_DISABLED)result=MH_UNKNOWN;}')
  .replace('static bool overlay_key106(', '#include "../../../deck/overlay_gate181.h"\nstatic bool overlay_key106(')
  .replace('    auto key=frame.key;', '    auto key=frame.key;\n    if(layer==4&&key.lease==188&&!key.actor&&!key.unit)return deck_help_gate188();\n    if(layer==overlay106::HUD&&key.lease==186&&!key.actor&&!key.unit)return deck_header_gate186();')
  .replace('            deck175::poll(self_module);','            if(device192::enabled()&&focus)capture_weapon_mouse(GetForegroundWindow());\n            deck175::poll(self_module);')
  .replace('static void mouse_look101(int dx,int dy,','static void mouse_look101(float dx,float dy,')
  .replace('mouse_look101(dx,dy,input,elapsed);','const auto deck_look201=deck175::controller181::look201.take();\n                        if(!device192::enabled()||dx||dy||deck_look201.x||deck_look201.y||(input&(TURN_LEFT|TURN_RIGHT)))mouse_look101(float(dx)+deck_look201.x,float(dy)+deck_look201.y,input,elapsed);'))
 # The exact owned-camera guard precedes the new analog integration. PC mouse,
 # trackpad and gyro still take their existing consumer and sensitivity path.
 transform(ADAPTER,lambda s:s.replace('static void adjust_eye(uintptr_t camera) {',
  'static void deck_look_frame202();\nstatic void adjust_eye(uintptr_t camera) {')
  .replace('        if(state->mode!=POSSESSED||!camera_owned(camera))return;',
   '        if(state->mode!=POSSESSED||!camera_owned(camera))return;\n        deck_look_frame202();')
  .replace('static DWORD WINAPI worker(void*) {',
   'static void deck_look_frame202(){\n'
   '    if(!device192::enabled())return;\n'
   '    bool allowed=enabled&&state&&state->mode==POSSESSED&&!state->release&&!state->stop&&!dead_view()&&!deck175::controller181::menu_open&&input_window&&GetForegroundWindow()==input_window;\n'
   '    deck202::Key key{state?state->lease_generation:0,state?state->actor_id:0,state?state->unit_id:0};\n'
   '    auto look=deck175::controller181::look202.advance(key,deck175::clock_us(),allowed);\n'
   '    if(look.x||look.y)mouse_look101(look.x,look.y,0,0.f);\n'
   '}\nstatic DWORD WINAPI worker(void*) {'))
 # Pitch must retain the same fraction as yaw; changing only the queue is not enough.
 transform('native/outputs/shogun2-accuracy90-preview/src/look_pitch96.h',lambda s:
  s.replace('mouse_pitch(float pitch,int dy,','mouse_pitch(float pitch,float dy,'))
 transform('native/outputs/shogun2-accuracy90-preview/src/native_keyboard84.h',lambda s:'#include "../../../deck/channel181.h"\n#include "../../../deck/keyboard_handoff196.h"\nstatic deck196::PollOwnership keyboard_owner196;\n'+s.replace('    InterlockedExchange(&keyboard_held84', '    deck181::native_keys(current,valid,base);\n    InterlockedExchange(&keyboard_held84')
  .replace('    require(MH_CreateHook(keyboard_poll_target84,','    if(device192::enabled())require(keyboard_owner196.prepare(keyboard_poll_target84,reinterpret_cast<void*>(&keyboard_poll84)),"Deck keyboard owner preparation failed");\n    require(MH_CreateHook(keyboard_poll_target84,'))
 # Shared PC/Deck HUD cycle, independent of Steam Input sampling/presentation.
 transform('native/outputs/shogun2-accuracy90-preview/src/hud_visibility148.h',lambda s:
  '#include "../../../deck/hud_cycle198.h"\n'+s.replace('    unsigned hidden;','    unsigned hidden;uintptr_t root198{};')
  .replace('        // Native global Hide UI (10f1060) owns root+ca0. Never XOR a second\n        // private toggle: it drifts at entry, exit, loading and other UI actions.','        root198=root; // Native RTS flag remains authoritative; mod visibility has three stages.')
  .replace('    if(hud_visibility_shared148->hidden==hidden)return;',
   '    hidden=hudcycle198::hidden(root198,state?state->lease_generation:0,enabled&&state&&state->mode==POSSESSED&&!state->stop&&!state->release,hidden);\n    if(hud_visibility_shared148->hidden==hidden)return;'))
 transform('native/outputs/shogun2-accuracy90-preview/src/native_keyboard84.h',lambda s:
  s.replace('    // Leave K in BOTH native buffers; stock Hide UI owns the single state.\n    // This reads its result on the next input poll, including RTS/entry/exit.\n    // The independent QTE and button prompt do not use this preference.',
   '    // Owning-window K uses the same cycle as View. Mask native duplication\n    // during possession, draining held K on exit; RTS keeps the stock toggle.')
  .replace('    hud_visibility_sync151(valid,enabled&&state&&!state->stop);',
   '    hudcycle198::filter(current,previous,keyboard_fps84(),valid);\n    hud_visibility_sync151(valid,enabled&&state&&!state->stop);')
  .replace('static bool keyboard_window_owned84(unsigned key){',
   'static bool keyboard_window_owned84(unsigned key){\n    if(key==\'K\'&&hudcycle198::window_owned(keyboard_fps84()))return true;'))
 def keyboard_hud_fixture198(s):
  start=s.index('    root151[0xca0]=1; // Result of stock Hide UI')
  end=s.index('    physical[0x19]=0x80;',start)
  return s[:start]+'    #include "../../../../tools/deck/verify_keyboard_hud198.h"\n'+s[end:]
 transform('native/outputs/shogun2-accuracy90-preview/src/verify_keyboard84.cpp',keyboard_hud_fixture198)
 # Independent fourth pixel layer; existing HUD/QTE/death protocols unchanged.
 transform('native/outputs/shogun2-possession-cleanup-v46/src/overlay_surface106.h',lambda s:s.replace('Death=3','Death=3,Help=4').replace('layer==QTE?1024u*1024u*4u:4u','(layer==QTE||layer==Help)?1024u*1024u*4u:4u').replace('layer>Death','layer>Help').replace('(layer==HUD?750u:200u)','((layer==HUD||layer==Help)?750u:200u)'))
 transform('native/outputs/shogun2-accuracy90-preview/src/native_overlay106.h',lambda s:s.replace('layer>3','layer>4').replace('textures_[3]','textures_[4]').replace('readers[3]','readers[4]').replace('retry[3]','retry[4]').replace('backend[3]','backend[4]').replace('frame[3]','frame[4]').replace('has[3]','has[4]').replace('for(unsigned i=0;i<3;++i)','for(unsigned i=0;i<4;++i)'))
 transform('native/outputs/shogun2-possession-cleanup-v46/src/hud_layout108.h',lambda s:s.replace('struct Rect {','inline bool deck_ui192=false;\nstruct Rect {').replace('s*.85f*1.15f','s*.85f*(deck_ui192?1.15f:1.f)').replace('height*.25f','height*(deck_ui192?.25f:.19f)'))
 transform('native/outputs/shogun2-ammunition83-preview/hud-src/verify_hud_layout108.cpp',lambda s:s.replace('using namespace hudlayout108;','using namespace hudlayout108;deck_ui192=true;'))
 transform(QTE,lambda s:keyboard_merge(s).replace('        LARGE_INTEGER f;QueryPerformanceFrequency','        hudlayout108::deck_ui192=device192::enabled();\n        LARGE_INTEGER f;QueryPerformanceFrequency').replace('#include "qte_service94.h"','#include "qte_service94.h"\n#include "../../../deck/channel181.h"'))
 def adapt_hud(s):
  s=s.replace('#include "hud_renderer.h"','#include "hud_renderer.h"\n#include "../../../deck/command_display184.h"')
  s=s.replace('static bool measure(){','static bool measure(){deck181::Sample controller;if(deck181::read_sample(pid,controller)&&controller.context==unsigned(deck174::Context::Menu))return false;')
  s=s.replace('pid=find_process(0);Module engine=', 'pid=find_process(0);hudlayout108::deck_ui192=device192::enabled(pid);Module engine=')
  s=s.replace('    hud80::View d{};','    deck181::presentation_pid()=pid;\n    hud80::View d{};')
  start=s.index('    if(commands_shared100)for(unsigned retry=0;retry<4;++retry){')
  end=s.index('    if(equipment_shared)',start)
  s=s[:start]+'''    static deck184::CommandDisplay command_display184;
    d.commands_known=command_display184.read([&](command100::Status& commands184){
      auto result=commands_shared100?deck184::Read::Contended:deck184::Read::Missing;
      if(commands_shared100)for(unsigned retry=0;retry<4;++retry){
        LONG seq=commands_shared100->sequence;if(seq&1)continue;
        MemoryBarrier();memcpy(&commands184,commands_shared100,sizeof commands184);MemoryBarrier();
        if(seq!=commands_shared100->sequence)continue;
        result=deck184::Read::Stable;break;
      }
      return result;
    },[](){return GetTickCount64();},pid,s.lease_generation,s.actor_id,s.unit_id,d.commands);
'''+s[end:]
  for shared in ('equipment_shared','abilities_shared141','ammunition_shared83','accuracy_shared90','climb_shared97'):
   anchor='if(seq!='+shared+'->sequence)continue;'
   if s.count(anchor)!=1:raise ValueError('Missing stable HUD sample anchor: '+shared)
   s=s.replace(anchor,anchor+'now=GetTickCount64();')
  return s.replace('static void update(){','#include "../../../deck/help_rts186.h"\nstatic void update(){').replace('    if(!active){pixels106.hide();','    if(!active){try{if(fg==pid&&deck_header186()){visible=false;return;}}catch(...){}pixels106.hide();')
 transform(HUD,adapt_hud)
 def wheel197(path):
  mode='v.commands_known?int(v.commands.fire_mode):-1'if 'ammunition83'in path else '-1'
  return '    deck181::draw_command_wheel183(g,a,width,height,v.mon.c_str(),'+mode+',v.following,[&](const wchar_t* label,Gdiplus::RectF box,float size,Gdiplus::Color color){native_fonts.paint(g,label,box,size,color,true);});'
 for path in RENDERERS:
  transform(path,lambda s:'#include "../../../deck/glyphs181.h"\n#include "../../../deck/help_header186.h"\n'+s.replace('inline void key_image(Graphics& g,const wchar_t* label,RectF r){','inline void key_image(Graphics& g,const wchar_t* label,RectF r){\n    if(deck181::draw_key(g,label,r))return;label=deck181::key_label(label);')
   .replace('inline void mouse_image(Graphics& g,RectF r,bool left,bool active=true){','inline void mouse_image(Graphics& g,RectF r,bool left,bool active=true){\n    if(deck181::controller_present()){key_image(g,left?L"RT":L"LT",r);return;}')
   .replace('L"RTS view"','deck181::controller_present()?L"Y: RTS":L"RTS view"')
   .replace('L"LMB: hold / release"','(deck181::controller_present()?L"RT: hold / release":L"LMB: hold / release")').replace('    (void)offline;', wheel197(path)+'\n    (void)offline;').replace('        g.Restore(all);return;', wheel197(path)+'\n        g.Restore(all);return;')
   .replace('    g.Restore(all);\n}', '    g.Restore(all);\n    if(deck181::controller_present()){auto saved=g.Save();float physical_width=width*scale;float zoom=deck186::header_scale(physical_width);g.TranslateTransform(deck186::header_left(physical_width),float(deck186::header_top));g.ScaleTransform(zoom,zoom);deck186::draw_header(g,[](unsigned slot,deck181::Glyph& glyph){return deck181::reader().glyph(slot,glyph);},[&](const wchar_t* label,Gdiplus::RectF bounds,float size){auto shadow=bounds;shadow.X+=1;shadow.Y+=1;native_fonts.paint(g,label,shadow,size,Gdiplus::Color(220,25,21,18));native_fonts.paint(g,label,bounds,size,Gdiplus::Color(255,242,233,207));});g.Restore(saved); }\n}'))
 transform('tools/native_build.py',lambda s:s.replace("ADAPTER_LIBS = BASE_LIBS + ['d3d11.lib', 'd3d9.lib', 'dxgi.lib', 'd3dcompiler.lib']", "ADAPTER_LIBS = BASE_LIBS + ['d3d11.lib', 'd3d9.lib', 'dxgi.lib', 'd3dcompiler.lib', 'gdiplus.lib']"))
 import adapt_gameplay203
 adapt_gameplay203.apply(transform)
 manifest,bindings=native_defaults181.values();settings=(snapshot/'native/deck/controller174.ini').read_text()
 (snapshot/'native/deck/defaults175_generated.h').write_text('#pragma once\nnamespace defaults175 {\n'+''.join('inline constexpr char '+n+'[]='+'{'+','.join(str(b)for b in v.encode('utf-8'))+',0};\n'for n,v in [('manifest',manifest),('bindings',bindings),('settings',settings)])+'}\n')
 return dict(changed_sources=changed,normal_source_modified=False,native_gameplay_rules_changed=False,action_sets=list(native_defaults181.SETS),glyphs='Steam client -> bounded memory; no generated glyph history')
