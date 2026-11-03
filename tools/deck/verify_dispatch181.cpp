// Compile against the final isolated adapter with /I<snapshot adapter src>.
// No game image, hooks, Steam backend, native constructor, input injection or
// foreground-window change. The actual dispatch/entry guard runs on owned data.
#include "adapter.cpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <atomic>
static unsigned ui188_calls;
static void __fastcall ui188(void* self,void*,unsigned hidden,unsigned a,unsigned b,unsigned c){assert(a==1&&!b&&c==1);field<unsigned char>(reinterpret_cast<uintptr_t>(self),0xca0)=static_cast<unsigned char>(hidden);++ui188_calls;}
int main(){
 SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
 // With no device record, the production Deck worker must do nothing: no
 // Steam ownership, controller mappings, native modifier hooks or manifest IO.
 assert(!device192::enabled());deck175::poll(nullptr);
 assert(!deck175::configured&&!deck175::running&&!deck175::checked&&!deck175::message_id);
 assert(!deck175::controller181::channel.shared);
 std::cout<<"PASS desktop192 adapter bypass: no controller startup\n";
 unsigned checks=0;Status storage{};state=&storage;enabled=1;startup168::ready();
 auto memory=VirtualAlloc(nullptr,0x1d88000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);base=reinterpret_cast<uintptr_t>(memory);
 std::vector<unsigned char> root_bytes(0xd00),env_bytes(0x28300),common_bytes(0x100),phase_bytes(0x40);
 auto address=[](auto& bytes){return reinterpret_cast<uintptr_t>(bytes.data());};
 auto root=address(root_bytes),env=address(env_bytes),common=address(common_bytes),phase=address(phase_bytes);
 field<uintptr_t>(base,0x1bc8180)=root;field<uintptr_t>(root,0x110)=env;field<uintptr_t>(env,8)=common;field<uintptr_t>(common,0xb4)=phase;field<unsigned>(phase,0x14)=5;
 input_window=CreateWindowExW(0,L"STATIC",L"Owned inert Deck entry fixture",0,0,0,1,1,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);assert(input_window);
 deck175::ContextProof proof;proof.owner_thread=proof.focused=proof.rts=true;proof.identity={root,env,common,phase,1,2};
 auto run=[&](unsigned key){deck181::Queued q;q.frame.source=deck174::Source::Simulation;q.frame.connected=q.frame.focused=q.frame.native_bindings=true;q.frame.active_buttons=deck174::all_buttons;q.frame.sequence=++checks;q.frame.epoch=1;q.frame.sample_us=deck175::clock_us();q.output.context=deck174::Context::RTS;q.output.accepted=true;q.output.set(key);deck175::controller181::dispatch(q,proof);};
 auto reset=[&](){storage={};storage.ready=1;storage.heartbeat=GetTickCount64();possessed=0;entry_attempted=entry_pending=0;deck_entry_dispatch181=0;};
 reset();run(deck181::F9);assert(state->mode==IDLE&&!state->acquire&&state->acquisition_error==3&&!state->camera_callback_thread&&!state->camera_input_thread&&!deck_entry_dispatch181);
 reset();state->camera_input_thread=GetCurrentThreadId()+1;run(deck181::F9);assert(state->mode==IDLE&&state->acquisition_error==1&&!deck_entry_dispatch181);
 reset();state->heartbeat=GetTickCount64()-1000;run(deck181::F9);assert(state->mode==IDLE&&!state->acquire&&state->acquisition_error==181&&!deck_entry_dispatch181);
 reset();state->mode=ARMED;state->acquire=1;run(deck181::F10);assert(state->mode==IDLE&&!state->acquire&&state->acquisition_error==181);
 reset();proof.owner_thread=false;run(deck181::F9);assert(state->mode==IDLE&&!state->acquire&&!state->acquisition_error);proof.owner_thread=true;
 reset();proof.blocked=true;run(deck181::F9);assert(state->mode==IDLE&&!state->acquire&&!state->acquisition_error);
 reset();proof.blocked=false;proof.possessed=true;proof.rts=false;state->mode=POSSESSED;state->lease_generation=1;state->actor_id=2;state->unit_id=3;
 for(unsigned n=0;n<100;++n){
  for(bool aim:{true,false}){deck181::Queued q;q.frame.source=deck174::Source::Simulation;q.frame.connected=q.frame.focused=q.frame.native_bindings=true;q.frame.active_buttons=deck174::all_buttons;q.frame.epoch=1;q.frame.sequence=n*2+(aim?1:2);q.frame.sample_us=deck175::clock_us();q.output.context=deck174::Context::Possession;q.output.accepted=true;q.lease=1;q.actor=2;q.unit=3;if(aim)q.output.set(deck181::Aim);deck175::controller181::dispatch(q,proof);assert(state->aiming==(aim?1:0));}
 }
 ++checks;
 deck175::controller181::previous.context=deck174::Context::Possession;state->aiming=1;deck175::controller181::cancel_window();assert(!state->aiming);++checks;
 // The production HUD dispatcher invokes the same native UI ABI. Only the
 // native UI callee is mocked; root/state/window ownership and cycle are real.
 {auto at=reinterpret_cast<unsigned char*>(base+0x10f1060);DWORD prior{},unused{};assert(VirtualProtect(at,32,PAGE_EXECUTE_READWRITE,&prior));at[0]=0xe9;uint32_t relative=uint32_t(reinterpret_cast<uintptr_t>(&ui188)-reinterpret_cast<uintptr_t>(at)-5);memcpy(at+1,&relative,4);FlushInstructionCache(GetCurrentProcess(),at,5);
 hud_visibility_create148();proof.focused=proof.owner_thread=true;state->lease_generation=2;field<unsigned char>(root,0xca0)=0;auto& stage=deck175::controller181::hud_stage188;
 for(unsigned i=1;i<=6;++i){assert(deck175::controller181::hud_toggle188(proof,true));assert(stage==i%3&&field<unsigned char>(root,0xca0)==(i%3?1:0));bool mod_hidden=false;assert(hudvisibility148::read(hud_visibility_shared148,GetCurrentProcessId(),mod_hidden)&&mod_hidden==(i%3==2));}++checks;
 auto before=ui188_calls;proof.focused=false;assert(!deck175::controller181::hud_toggle188(proof,true)&&before==ui188_calls);proof.focused=true;++checks;
 assert(deck175::controller181::hud_toggle188(proof,false)&&field<unsigned char>(root,0xca0)==1&&stage==0);assert(deck175::controller181::hud_toggle188(proof,false)&&field<unsigned char>(root,0xca0)==0);++checks;
 // PC K must work with the controller worker bypassed and no Steam sample.
 assert(!device192::enabled()&&!deck175::controller181::channel.shared);
 auto key198=[&](UINT msg,LPARAM flags=0){return deck175::controller181::keyboard_hud198(msg,'K',flags,proof);};
 ++state->lease_generation;unsigned calls198=ui188_calls;
 for(unsigned expected:{1u,2u,0u}){
  assert(key198(WM_KEYDOWN));assert(ui188_calls==++calls198);
  bool hidden=false;assert(hudvisibility148::read(hud_visibility_shared148,GetCurrentProcessId(),hidden)&&hidden==(expected==2));
  assert(field<unsigned char>(root,0xca0)==unsigned(expected!=0));
  for(unsigned n=0;n<100;++n)assert(key198(WM_KEYDOWN,1L<<30));
  assert(key198(WM_CHAR)&&key198(WM_KEYUP)&&ui188_calls==calls198);
  hud_visibility_sync151(true,true);assert(hudvisibility148::read(hud_visibility_shared148,GetCurrentProcessId(),hidden)&&hidden==(expected==2));
 }
 // Mixed keyboard/View uses a single cycle, without controller-only override.
 assert(key198(WM_KEYDOWN)&&stage==1);assert(deck175::controller181::hud_toggle188(proof,true)&&stage==2);
 assert(key198(WM_KEYDOWN)&&stage==0);calls198=ui188_calls;
 for(unsigned reject=0;reject<3;++reject){
  proof.focused=reject!=0;proof.owner_thread=reject!=1;proof.blocked=reject==2;
  assert(key198(WM_KEYDOWN)&&ui188_calls==calls198);
 }
 proof.focused=proof.owner_thread=true;proof.blocked=false;
 // A fresh lease starts from the actual native RTS flag, not another actor's stage.
 assert(key198(WM_KEYDOWN)&&stage==1);++state->lease_generation;
 assert(key198(WM_KEYDOWN)&&stage==2);assert(key198(WM_KEYDOWN)&&stage==0);
 state->mode=IDLE;assert(!key198(WM_KEYDOWN));state->mode=POSSESSED;
 std::cout<<"PASS HUD198: desktop K and View share native calls and HUD mapping; repeats, mixed input, ownership, lease and RTS fallback checked\n";
 // The real window hook must admit keyboard K on desktop even with no
 // controller-owned window/message ID. This inert window is unfocused, so it
 // consumes the event without calling the native UI function.
 calls198=ui188_calls;assert(!deck175::owned_window&&!deck175::message_id);
 assert(deck175::window(input_window,WM_KEYDOWN,'K',0)&&ui188_calls==calls198);
 assert(!deck175::window(nullptr,WM_KEYDOWN,'K',0));
 hud_visibility_close148();
 assert(VirtualProtect(at,32,prior,&unused));}
 // Wheel choices must reach the actual command consumer, not only a key bit.
 {const command100::Input expected[]={command100::Input::Follow,command100::Input::Stop,command100::Input::Hold,command100::Input::Fire};
 const deck174::Vec2 sticks[]={{0,1},{0,-1},{-1,0},{1,0}};unsigned delivered=0;
 for(bool pad:{false,true})for(unsigned choice=0;choice<4;++choice){
  reset();state->mode=POSSESSED;state->lease_generation=7;state->actor_id=8;state->unit_id=9;
  proof.possessed=proof.focused=proof.owner_thread=true;proof.rts=proof.blocked=false;
  command_input_reset100();command_key_held100=0;deck175::controller181::previous={};
  command100::Frame anchor{};anchor.position={10,2,30};anchor.yaw=.4f;anchor.tick=GetTickCount64();anchor.lease=7;anchor.actor=8;anchor.unit=9;command_publish_frame100(anchor);
  deck181::Controls wheel;deck174::Frame f;f.source=deck174::Source::Simulation;f.connected=f.focused=f.native_bindings=true;f.active_buttons=deck174::all_buttons;f.epoch=1;
  auto send=[&](uint64_t buttons,deck174::Vec2 stick){f.sequence++;f.sample_us=1000000+f.sequence*8000;f.buttons=buttons;f.right_stick=stick;deck181::Queued q;q.output=wheel.sample(f,deck174::Context::Possession,f.sample_us);q.frame=f;q.frame.sample_us=deck175::clock_us();q.lease=7;q.actor=8;q.unit=9;deck175::controller181::dispatch(q,proof);};
  send(0,{});send(deck174::bit(deck174::Button::R1)|(pad?deck174::bit(deck174::Button(unsigned(deck174::Button::Up)+choice)):0),pad?deck174::Vec2{}:sticks[choice]);
  command100::Event event;assert(!command_pop100(event));send(0,{});assert(command_pop100(event)&&event.input==expected[choice]&&event.frame.lease==7&&event.frame.actor==8&&event.frame.unit==9&&event.frame.position.x==10&&event.frame.position.z==30);++delivered;
  for(unsigned i=0;i<30;++i){send(0,{});assert(!command_pop100(event));}assert(!command_key_held100);
 }
 std::cout<<"PASS wheel197 dispatch: "<<delivered<<" actual adapter/window/native-command queue deliveries; identity and anchor retained, no repeats\n";
 }
 deck175::owned_window=input_window;deck175::posted=1;deck175::window(input_window,WM_NCDESTROY,0,0);assert(!deck175::owned_window&&!deck175::posted);++checks;
 // Exercise the actual adapter yield ordering with an owned mock hook.
 // No native game routine is called; only MinHook on this allocated fixture.
 {auto code=reinterpret_cast<unsigned char*>(base+0xbd75f0);DWORD prior{},unused{};
 assert(VirtualProtect(code,64,PAGE_EXECUTE_READWRITE,&prior));
 unsigned char stub[20]={0x56,0x57,0xb9,0x40,0,0,0,0xbe,0,0,0,0,0xbf,0,0,0,0,0xf3,0xa5,0xc3};
 uintptr_t cur=base+0x1b30f58,prev=base+0x1b31058;memcpy(stub+8,&cur,4);memcpy(stub+13,&prev,4);memcpy(code,stub,sizeof stub);FlushInstructionCache(GetCurrentProcess(),code,sizeof stub);
 auto callback=+[]()->bool{return true;};void* original196{};
 auto mh=MH_Initialize();assert(mh==MH_OK||mh==MH_ERROR_ALREADY_INITIALIZED);
 assert(keyboard_owner196.prepare(code,reinterpret_cast<void*>(callback)));
 assert(MH_CreateHook(code,reinterpret_cast<void*>(callback),&original196)==MH_OK);
 assert(MH_EnableHook(code)==MH_OK&&keyboard_owner196.adopt_enabled());
 for(unsigned round=0;round<12;++round){
  assert(deck175::input_lease177.acquire(deck177::Owner::Battle));
  deck175::yield181();assert(!keyboard_owner196.active()&&!deck175::input_lease177.handle&&!memcmp(code,stub,sizeof stub));
  deck177::Lease front;assert(front.acquire(deck177::Owner::Frontend));
  deck177::battle_request().set(true);assert(!deck177::claim_battle(deck175::input_lease177)&&!keyboard_owner196.active());
  front.release();assert(deck177::claim_battle(deck175::input_lease177));
  assert(keyboard_owner196.acquire());deck175::input_lease177.release();deck177::battle_request().set(false);
 }
 deck175::yield181();assert(MH_RemoveHook(code)==MH_OK);assert(MH_Uninitialize()==MH_OK);assert(VirtualProtect(code,64,prior,&unused));
 std::cout<<"PASS return196 adapter yield: 12 actual adapter release/lease/reacquire cycles; original bytes restored before frontend ownership\n";
 }
 #include "verify_look201.h"
 #include "verify_look202.h"
 assert(DestroyWindow(input_window));input_window=nullptr;state=nullptr;VirtualFree(memory,0,MEM_RELEASE);
 std::cout<<"PASS "<<checks<<" actual private adapter dispatch/entry recovery cases; synthetic owned data only\n";
}
