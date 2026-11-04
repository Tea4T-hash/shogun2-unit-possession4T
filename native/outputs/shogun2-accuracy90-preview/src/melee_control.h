#pragma once
#include "../../shogun2-melee-qte63-preview/src/melee_commitment.h"
#include "death_pose_geometry.h"
#include "animation_view.h"
#include "../../shogun2-possession-cleanup-v46/src/life_stats173.h"
static animationview::Handoff melee_handoff78;
static animationview::Handoff melee_opponent_motion78;
static bool animation_view_active78;
static float animation_view_up78[3];
static volatile LONG animation_view_updates78,melee_handoff_updates78,melee_playback_hold_bypasses78;
static float melee_handoff_root78[3];
static melee60::Channel melee_channel;
static __declspec(align(8)) volatile LONG64 death_started;
static volatile LONG death_return_pending;
static bool death_third_person156;
static HWND eyelid_top,eyelid_bottom;
static overlay106::Publisher death_pixels106;
static float death_eye[6];static bool death_eye_valid;
static float last_owned_eye[6];static bool last_owned_eye_valid;
static float death_up[3]={0,1,0};static bool death_model_valid;
struct ModelEye {uint32_t lease{},actor{},unit{};uint64_t tick{};deathpose::Eye eye{};};
struct ModelEyeChannel {volatile LONG sequence{};ModelEye value{};};
static ModelEyeChannel owned_eye_channel,model_eye_channel,native_eye_channel78;
static void publish_model_eye(ModelEyeChannel& channel,const ModelEye& value){InterlockedIncrement(&channel.sequence);MemoryBarrier();channel.value=value;MemoryBarrier();InterlockedIncrement(&channel.sequence);}
static bool read_model_eye(const ModelEyeChannel& channel,ModelEye& value){for(unsigned i=0;i<4;++i){LONG n=channel.sequence;if(n&1)continue;MemoryBarrier();auto copy=channel.value;MemoryBarrier();if(n==channel.sequence){value=copy;return true;}}return false;}
static bool leased_model_eye(const ModelEye& value){return possession_lease.active&&value.lease==possession_lease.generation&&value.actor==possession_lease.identity.actor_id&&value.unit==possession_lease.identity.unit_id;}
static bool dead_view(){return InterlockedCompareExchange64(&death_started,0,0)!=0;}
static uint64_t death_elapsed(){auto start=uint64_t(InterlockedCompareExchange64(&death_started,0,0));return start?GetTickCount64()-start:0;}
static void reset_death_view(){InterlockedExchange64(&death_started,0);InterlockedExchange(&death_return_pending,0);death_third_person156=false;death_eye_valid=false;last_owned_eye_valid=false;death_model_valid=false;animation_view_active78=false;melee_handoff78.reset();melee_opponent_motion78.reset();}
static void remember_owned_eye(uintptr_t camera){
    float eye[6];bool valid=camera!=0;
    if(camera)for(unsigned i=0;i<6;++i){eye[i]=*reinterpret_cast<float*>(camera+8+i*4);valid=valid&&std::isfinite(eye[i]);}
    if(valid){memcpy(last_owned_eye,eye,sizeof eye);last_owned_eye_valid=true;
        ModelEye sample;sample.lease=possession_lease.generation;sample.actor=possession_lease.identity.actor_id;sample.unit=possession_lease.identity.unit_id;sample.tick=GetTickCount64();memcpy(sample.eye.view,eye,sizeof eye);
        for(unsigned i=0;i<3;++i)sample.eye.up[i]=*reinterpret_cast<float*>(camera+0x58+i*4);
        if(!deathpose::normalize(sample.eye.up)){sample.eye.up[0]=sample.eye.up[2]=0;sample.eye.up[1]=1;}publish_model_eye(owned_eye_channel,sample);}
}
// Consume numeric poses captured on the native skin callback. No dereference of
// corpse, retired holder, or skeleton memory from the camera/timer threads.
static void update_death_model_view(){
    ModelEye sample;auto start=uint64_t(InterlockedCompareExchange64(&death_started,0,0));
    if(!start||!read_model_eye(model_eye_channel,sample)||!leased_model_eye(sample)||sample.tick+500<start||sample.tick>GetTickCount64())return;
    memcpy(death_eye,sample.eye.view,sizeof death_eye);memcpy(death_up,sample.eye.up,sizeof death_up);death_eye_valid=death_model_valid=true;
}
// Kit v10: death counter for the kills-per-life display and battle statistics. Never required.
static HANDLE life_mapping173=nullptr;static life173::Shared* life_shared173=nullptr;
static void life_mapping173_create(){
    if(life_shared173)return;wchar_t name[96];life173::mapping_name(name,GetCurrentProcessId());
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(life173::Shared),name);
    if(!map)return;if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(map);return;}
    auto view=static_cast<life173::Shared*>(MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(life173::Shared)));
    if(!view){CloseHandle(map);return;}
    memset(view,0,sizeof *view);view->magic=life173::Magic;view->version=life173::Version;view->size=sizeof *view;view->pid=GetCurrentProcessId();
    life_mapping173=map;life_shared173=view;
}
static void begin_death_view(){
    if(!InterlockedCompareExchange64(&death_started,LONG64(GetTickCount64()),0)){
        if(life_shared173){InterlockedExchange(&life_shared173->last_death_lease,LONG(possession_lease.generation));InterlockedIncrement(&life_shared173->deaths);}
        death_third_person156=third_person152!=0;
        if(!last_owned_eye_valid)remember_owned_eye(possession_lease.identity.camera);
        death_eye_valid=last_owned_eye_valid;if(death_eye_valid)memcpy(death_eye,last_owned_eye,sizeof death_eye);
        ModelEye sample;bool native=read_model_eye(native_eye_channel78,sample)&&leased_model_eye(sample)&&GetTickCount64()-sample.tick<=500;
        if(native||(read_model_eye(owned_eye_channel,sample)&&leased_model_eye(sample))){memcpy(death_eye,sample.eye.view,sizeof death_eye);memcpy(death_up,sample.eye.up,sizeof death_up);death_eye_valid=true;}
        update_death_model_view();
        InterlockedExchange(&state->input,0);InterlockedExchange(&state->aiming,0);
        InterlockedExchange(&state->fire_request,0);InterlockedExchange(&state->pending_order,0);
        InterlockedExchange(&state->follow,0);
    }
}
static bool melee_request(melee60::Request& r){
    if(!state||state->mode!=POSSESSED||state->stop||state->release||dead_view()||!melee60::read(melee_channel,r))return false;
    uint32_t actor=state->actor_id;
    if(r.player_kind==1){
        if(!possession_lease.active||!possession_lease.identity.mount||possession_lease.generation!=state->lease_generation)return false;
        actor=possession_lease.identity.mount_id;
    }
    return melee60::live(r,GetCurrentProcessId(),state->lease_generation,actor,state->unit_id,GetTickCount64());
}
extern "C" __declspec(dllexport) uint32_t __cdecl PossessionMeleeCommitment63(const melee60::Request* request){
    if(!request||!melee60::valid(*request))return 0;
    if(request->phase!=melee60::Off&&request->pid!=GetCurrentProcessId())return 0;
    melee60::publish(melee_channel,*request);
    if(state&&request->phase!=melee60::Off){
        InterlockedExchange(&state->aiming,0);InterlockedExchange(&state->fire_request,0);
        InterlockedExchange(&state->weapon_phase,0);state->aim_pose=0;
    }
    return 63;
}
extern "C" __declspec(dllexport) uint32_t __cdecl PossessionRTS63(){
    if(state)InterlockedExchange(&state->release,1);return 63;
}
static void end_death_on_window();
static LRESULT CALLBACK eyelid_proc(HWND window,UINT message,WPARAM w,LPARAM l){
    if(message==WM_NCHITTEST)return HTTRANSPARENT;
    if(message==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
    if(message==WM_PAINT){PAINTSTRUCT paint;HDC dc=BeginPaint(window,&paint);FillRect(dc,&paint.rcPaint,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));EndPaint(window,&paint);return 0;}
    return DefWindowProcW(window,message,w,l);
}
static void update_eyelids(bool focused){
    if(!dead_view()||death_third_person156||!focused||!input_window||IsIconic(input_window)||state->release||state->stop){
        death_pixels106.hide();
        if(eyelid_top)ShowWindow(eyelid_top,SW_HIDE);if(eyelid_bottom)ShowWindow(eyelid_bottom,SW_HIDE);return;
    }
    if(!eyelid_top){WNDCLASSW cls{};cls.hInstance=self_module;cls.lpfnWndProc=eyelid_proc;cls.lpszClassName=L"ShogunPossessionDeathEyes60";
        if(!RegisterClassW(&cls)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return;
        const DWORD ex=WS_EX_NOACTIVATE|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_TOPMOST|WS_EX_LAYERED;
        eyelid_top=CreateWindowExW(ex,cls.lpszClassName,L"",WS_POPUP,0,0,1,1,nullptr,nullptr,self_module,nullptr);
        eyelid_bottom=CreateWindowExW(ex,cls.lpszClassName,L"",WS_POPUP,0,0,1,1,nullptr,nullptr,self_module,nullptr);
        if(!eyelid_top||!eyelid_bottom){
            if(eyelid_top)DestroyWindow(eyelid_top);if(eyelid_bottom)DestroyWindow(eyelid_bottom);
            eyelid_top=eyelid_bottom=nullptr;return;
        }
    }
    SetLayeredWindowAttributes(eyelid_top,0,255,LWA_ALPHA);SetLayeredWindowAttributes(eyelid_bottom,0,255,LWA_ALPHA);
    MSG paint;for(HWND lid:{eyelid_top,eyelid_bottom})while(PeekMessageW(&paint,lid,0,0,PM_REMOVE)){TranslateMessage(&paint);DispatchMessageW(&paint);}
    RECT client{};POINT origin{};if(!GetClientRect(input_window,&client)||!ClientToScreen(input_window,&origin))return;
    int height=int(std::ceil(float(client.bottom)*.5f*animationview::eyelid_fraction(death_elapsed())));
    if(!height){death_pixels106.hide();ShowWindow(eyelid_top,SW_HIDE);ShowWindow(eyelid_bottom,SW_HIDE);return;}
    if(death_pixels106.open(GetCurrentProcessId(),overlay106::Death)){
        overlay106::Frame f{};f.key={state->lease_generation,state->actor_id,state->unit_id};f.width=f.height=1;f.view_width=client.right;f.view_height=client.bottom;
        f.rect_count=2;f.rect[0]={0,0,client.right,height};f.rect[1]={0,client.bottom-height,client.right,height};unsigned black=0xff000000;
        if(death_pixels106.publish(f,&black)){overlay106::Ack ack;if(death_pixels106.ack(ack)){ShowWindow(eyelid_top,SW_HIDE);ShowWindow(eyelid_bottom,SW_HIDE);return;}}
    }
    SetWindowPos(eyelid_top,HWND_TOPMOST,origin.x,origin.y,client.right,height,SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    SetWindowPos(eyelid_bottom,HWND_TOPMOST,origin.x,origin.y+client.bottom-height,client.right,height,SWP_ASYNCWINDOWPOS|SWP_NOACTIVATE|SWP_SHOWWINDOW);
    InvalidateRect(eyelid_top,nullptr,FALSE);UpdateWindow(eyelid_top);InvalidateRect(eyelid_bottom,nullptr,FALSE);UpdateWindow(eyelid_bottom);
}

