#pragma once
#include "first_person_presentation101.h"
#include "../../../deck/native214.h"
// Native UI operations execute only on the game's window/camera thread.
using UIVisible=void(__thiscall*)(void*,unsigned);
using UIState=bool(__thiscall*)(void*,const void*);
using UIUpdate=void(__thiscall*)(void*,float);
using NativeString=void*(__thiscall*)(void*,const char*);
using DestroyString=void(__thiscall*)(void*);
using CameraFOV=void(__thiscall*)(void*,float);
static UIVisible original_ui_visible;
static UIState original_ui_state;
static UIUpdate original_crosshair_update;
using SetCursorFn=HCURSOR(WINAPI*)(HCURSOR);
static SetCursorFn original_set_cursor;
static void* cursor_target;
struct SavedWidget {uintptr_t object;unsigned visible,desired;bool suppress{true};};
static bool interface_modal214;
static SavedWidget saved_widgets[96];
static unsigned saved_widget_count;
static uintptr_t interface_root,interface_environment,interface_crosshair,interface_controller,interface_camera;
static volatile LONG interface_active;
static float interface_hip_fov;
static firstperson101::NearClip interface_clip101;
static float interface_original_up[3];
static unsigned char interface_matrix_override;
static char interface_original_state[64];
static HCURSOR interface_cursor;
static __declspec(thread) unsigned own_ui_call;

static bool ui_context_valid(){
    __try {
        uintptr_t root=field<uintptr_t>(base,0x1bc8180);
        return root&&field<uintptr_t>(root,0x110)==interface_environment
            &&field<uintptr_t>(base,0x1bbe364)==interface_root
            &&field<uintptr_t>(root,0xd08)==interface_controller
            &&field<uintptr_t>(interface_controller,4)==interface_crosshair;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static void ui_state(uintptr_t widget,const char* name){
    alignas(4) unsigned char text[12]{};
    reinterpret_cast<NativeString>(base+0xecfa0)(text,name);
    __try {original_ui_state(reinterpret_cast<void*>(widget),text);}
    __finally {reinterpret_cast<DestroyString>(base+0xa0b00)(text);}
}
static bool saved_widget_live(const SavedWidget& widget){
    for(uintptr_t parent:{interface_root,interface_crosshair}){
        unsigned n=field<unsigned>(parent,0x9c);uintptr_t a=field<uintptr_t>(parent,0xa0);
        if(!a||n>128)continue;
        for(unsigned i=0;i<n;++i)if(field<uintptr_t>(a,i*4)==widget.object)return true;
    }
    return false;
}
static void interface_restore(){
    restore_soldier_pose();
    restore_raw_mouse();
    InterlockedExchange(&state->aiming,0);state->aim_blend=0;
    if(!interface_active)return;
    // Clear ownership before native UI callbacks to prevent reentrant suppression.
    InterlockedExchange(&interface_active,0);
    __try {
        if(ui_context_valid()){
            own_ui_call=1;
            for(unsigned i=0;i<saved_widget_count;++i)if(saved_widget_live(saved_widgets[i]))
                original_ui_visible(reinterpret_cast<void*>(saved_widgets[i].object),saved_widgets[i].visible);
            if(interface_original_state[0])ui_state(interface_crosshair,interface_original_state);
            // K may have changed during possession. Reapply the CURRENT stock
            // Hide UI state after restoring widgets; a saved entry snapshot
            // must not undo it. Same four arguments as native1210a10.
            auto root151=field<uintptr_t>(base,0x1bc8180);
            using NativeUI151=void(__thiscall*)(void*,unsigned,unsigned,unsigned,unsigned);
            reinterpret_cast<NativeUI151>(base+0x10f1060)(reinterpret_cast<void*>(root151),field<unsigned char>(root151,0xca0),1,0,1);
            uintptr_t manager=field<uintptr_t>(interface_environment,0x281fc);
            if(manager&&field<uintptr_t>(manager,0x8e0)==interface_camera&&field<uintptr_t>(interface_camera,0)==base+0x15d22e8){
                interface_clip101.restore(field<float>(interface_camera,0x34),true);
                reinterpret_cast<CameraFOV>(base+0x136ec70)(reinterpret_cast<void*>(interface_camera),interface_hip_fov);
                for(unsigned i=0;i<3;++i)field<float>(interface_camera,0x58+i*4)=interface_original_up[i];
                field<unsigned char>(interface_camera,0xa4)=interface_matrix_override;
            }
            own_ui_call=0;
        }
        if(original_set_cursor)original_set_cursor(interface_cursor?interface_cursor:LoadCursorW(nullptr,MAKEINTRESOURCEW(32512)));
        ++state->interface_restores;
    }__except(EXCEPTION_EXECUTE_HANDLER){state->interface_error=103;own_ui_call=0;}
    interface_modal214=false;interface_clip101={};saved_widget_count=0;interface_camera=0;state->hud_hidden=state->crosshair_visible=0;
}
static void __fastcall ui_visible_hook(void* widget,void*,unsigned visible){
    if(interface_active&&!own_ui_call&&enabled&&!state->stop&&!state->release&&state->mode==POSSESSED){
        uintptr_t p=reinterpret_cast<uintptr_t>(widget);
        for(unsigned i=0;i<saved_widget_count;++i)if(saved_widgets[i].object==p){saved_widgets[i].visible=visible;if(saved_widgets[i].suppress&&!native_modal214())visible=saved_widgets[i].desired;break;}
    }
    original_ui_visible(widget,visible);
}
static bool __fastcall ui_state_hook(void* widget,void*,const void* text){
    if(interface_active&&!own_ui_call&&!native_modal214()&&reinterpret_cast<uintptr_t>(widget)==interface_crosshair
        &&enabled&&!state->stop&&!state->release&&state->mode==POSSESSED)return true;
    return original_ui_state(widget,text);
}
static void __fastcall crosshair_update_hook(void* controller,void*,float dt){
    if(interface_active&&!native_modal214()&&reinterpret_cast<uintptr_t>(controller)==interface_controller
        &&enabled&&!state->stop&&!state->release&&state->mode==POSSESSED)return;
    original_crosshair_update(controller,dt);
}
static HCURSOR WINAPI set_cursor_hook(HCURSOR cursor){
    if(enabled&&state&&!state->stop&&!state->release&&state->mode==POSSESSED){
        DWORD pid=0;GetWindowThreadProcessId(GetForegroundWindow(),&pid);
        if(pid==GetCurrentProcessId()&&!command_placing100&&!native_modal214()){++state->cursor_suppressed;return original_set_cursor(nullptr);}
    }
    return original_set_cursor(cursor);
}
static void save_widget(uintptr_t p,unsigned desired){
    if(!p||saved_widget_count>=96)return;
    for(unsigned i=0;i<saved_widget_count;++i)if(saved_widgets[i].object==p)return;
    unsigned visible=field<unsigned char>(p,0xf5);
    saved_widgets[saved_widget_count++]={p,visible,desired,visible!=0||p==interface_crosshair};
}
static bool interface_capture(uintptr_t camera){
    __try {
        uintptr_t main=field<uintptr_t>(base,0x1bc8180);interface_root=field<uintptr_t>(base,0x1bbe364);
        interface_environment=environment;interface_controller=field<uintptr_t>(main,0xd08);
        if(!interface_root||!interface_controller||field<uintptr_t>(interface_controller,0)!=base+0x172814c)return false;
        interface_crosshair=field<uintptr_t>(interface_controller,4);
        if(!interface_crosshair||field<uintptr_t>(interface_crosshair,0x94)!=interface_root)return false;
        unsigned n=field<unsigned>(interface_root,0x9c);uintptr_t array=field<uintptr_t>(interface_root,0xa0);
        if(!array||n<1||n>64)return false;
        interface_hip_fov=field<float>(camera,0x24);
        if(!interface_clip101.capture(field<float>(camera,0x34),field<float>(camera,0x38)))return false;
        interface_matrix_override=field<unsigned char>(camera,0xa4);
        for(unsigned i=0;i<3;++i)interface_original_up[i]=field<float>(camera,0x58+i*4);
        if(!std::isfinite(interface_hip_fov)||interface_hip_fov<0.1f||interface_hip_fov>2.5f)return false;
        saved_widget_count=0;
        for(unsigned i=0;i<n;++i){uintptr_t p=field<uintptr_t>(array,i*4);save_widget(p,p==interface_crosshair?1:0);}
        n=field<unsigned>(interface_crosshair,0x9c);array=field<uintptr_t>(interface_crosshair,0xa0);
        if(n>24){saved_widget_count=0;return false;}
        for(unsigned i=0;i<n;++i)save_widget(field<uintptr_t>(array,i*4),0);
        interface_original_state[0]=0;
        uintptr_t native_state=field<uintptr_t>(interface_crosshair,0xb8);
        if(native_state){const char* s=reinterpret_cast<const char*>(field<uintptr_t>(native_state,0x18));
            for(unsigned i=0;i<63;++i){interface_original_state[i]=s[i];interface_original_state[i+1]=0;if(!s[i])break;}}
        interface_camera=camera;state->aim_blend=0;state->hip_fov=interface_hip_fov;
        if(!capture_raw_mouse()){state->interface_error=106;return false;}
        interface_cursor=GetCursor();own_ui_call=1;
        ui_state(interface_crosshair,"static_target");
        for(unsigned i=0;i<saved_widget_count;++i)original_ui_visible(reinterpret_cast<void*>(saved_widgets[i].object),saved_widgets[i].suppress?saved_widgets[i].desired:saved_widgets[i].visible);
        own_ui_call=0;MemoryBarrier();InterlockedExchange(&interface_active,1);
        state->hud_hidden=1;state->crosshair_visible=field<unsigned char>(interface_crosshair,0xf5);
        if(original_set_cursor)original_set_cursor(nullptr);
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){own_ui_call=0;restore_raw_mouse();state->interface_error=101;return false;}
}
static void interface_update(uintptr_t camera){
    if(GetCurrentThreadId()!=state->window_thread)return;
    if(!enabled||state->stop||state->release||state->mode!=POSSESSED||!camera_owned(camera)){
        interface_restore();return;
    }
    if(interface_active&&(!ui_context_valid()||interface_camera!=camera))interface_restore();
    if(!interface_active&&!interface_capture(camera)){state->interface_error=102;return;}
    __try {
        // Tooltips and other top-level widgets can be created after possession.
        for(unsigned i=0;i<saved_widget_count;){
            if(saved_widget_live(saved_widgets[i]))++i;
            else saved_widgets[i]=saved_widgets[--saved_widget_count];
        }
        unsigned children=field<unsigned>(interface_root,0x9c);uintptr_t array=field<uintptr_t>(interface_root,0xa0);
        if(children<=64&&array)for(unsigned i=0;i<children;++i){
            uintptr_t widget=field<uintptr_t>(array,i*4);unsigned old_count=saved_widget_count;
            // A new root child belongs to native UI (menus/results/tooltips).
            save_widget(widget,widget==interface_crosshair?1:0);
            if(old_count!=saved_widget_count)saved_widgets[old_count].suppress=false;
            if(old_count!=saved_widget_count){own_ui_call=1;original_ui_visible(reinterpret_cast<void*>(widget),saved_widgets[old_count].visible);own_ui_call=0;}
        }
        const bool modal214=native_modal214();
        if(modal214!=interface_modal214){
            interface_modal214=modal214;own_ui_call=1;
            for(unsigned i=0;i<saved_widget_count;++i){auto& w=saved_widgets[i];if(saved_widget_live(w))original_ui_visible(reinterpret_cast<void*>(w.object),modal214||!w.suppress?w.visible:w.desired);}
            own_ui_call=0;
            if(modal214){command_native_cancel100();restore_raw_mouse();InterlockedExchange(&state->aiming,0);if(original_set_cursor)original_set_cursor(interface_cursor);}
            else capture_raw_mouse();
        }
        if(modal214)return; // Native menu owns cursor/input; keep the actor lease.
        // During Q the stock context cursor stays at the centered aim point.
        unsigned crosshair=0;
        // Test90 HUD owns the four movable firearm bars and projectile drop dashes.
        for(unsigned i=0;i<saved_widget_count;++i)if(saved_widgets[i].object==interface_crosshair){
            saved_widgets[i].desired=crosshair;own_ui_call=1;
            original_ui_visible(reinterpret_cast<void*>(interface_crosshair),crosshair);own_ui_call=0;break;
        }
        state->crosshair_visible=crosshair;
        // Native view/projection getters use precomputed matrices when +a4 is set.
        // Render possession from its eye, full look direction, and stable up vector.
        ViewRay view{};float up[3];
        bool valid_view=camera_ray(reinterpret_cast<float*>(camera+8),reinterpret_cast<float*>(camera+0x14),view);
        if(valid_view){
            if(dead_view()&&death_model_valid&&!death_third_person156){memcpy(up,death_up,sizeof up);valid_view=deathpose::normalize(up)&&std::abs(deathpose::dot(up,view.direction))<.01f;}
            else if(animation_view_active78){memcpy(up,animation_view_up78,sizeof up);valid_view=deathpose::normalize(up)&&std::abs(deathpose::dot(up,view.direction))<.01f;}
            else valid_view=level_camera_up(view,up);
        }
        if(!valid_view){
            state->interface_error=105;InterlockedExchange(&state->release,1);return;
        }
        for(unsigned i=0;i<3;++i)field<float>(camera,0x58+i*4)=up[i];
        field<unsigned char>(camera,0xa4)=0;
        state->camera_pitch=std::asin(std::clamp(view.direction[1],-1.0f,1.0f));
        state->native_matrix_override=interface_matrix_override;
        state->aim_blend=0;state->view_fov=POSSESSION_FOV;
        reinterpret_cast<CameraFOV>(base+0x136ec70)(reinterpret_cast<void*>(camera),state->view_fov);
        interface_clip101.apply(field<float>(camera,0x34),!dead_view());
        float projection[16]{};
        using Projection90=float*(__thiscall*)(void*,float*);
        reinterpret_cast<Projection90>(base+0x1367e20)(reinterpret_cast<void*>(camera),projection);
        accuracy_projection90(projection[5]);

        for(unsigned i=0;i<3;++i)state->view_origin[i]=field<float>(camera,8+i*4);
        auto refresh=reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4));refresh(reinterpret_cast<void*>(camera));
    }__except(EXCEPTION_EXECUTE_HANDLER){own_ui_call=0;state->interface_error=104;InterlockedExchange(&state->release,1);}
}
static void restore_interface_on_window(){
    if(!input_window||!interface_active)return;
    DWORD_PTR result=0;SendMessageTimeoutW(input_window,RESTORE_INTERFACE_MESSAGE,0,0,SMTO_ABORTIFHUNG,500,&result);
}
static void create_interface_hooks(){
    require(native214::admit(base,1),"Native QoL dependency guard failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0xf9c360),reinterpret_cast<void*>(&ui_visible_hook),reinterpret_cast<void**>(&original_ui_visible))==MH_OK,"UI visibility hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0xf9bf20),reinterpret_cast<void*>(&ui_state_hook),reinterpret_cast<void**>(&original_ui_state))==MH_OK,"UI state hook failed");
    require(MH_CreateHook(reinterpret_cast<void*>(base+0x122f740),reinterpret_cast<void*>(&crosshair_update_hook),reinterpret_cast<void**>(&original_crosshair_update))==MH_OK,"Crosshair update hook failed");
    cursor_target=reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetCursor"));require(cursor_target!=nullptr,"Cursor API unavailable");
    require(MH_CreateHook(cursor_target,reinterpret_cast<void*>(&set_cursor_hook),reinterpret_cast<void**>(&original_set_cursor))==MH_OK,"Cursor hook failed");
}
