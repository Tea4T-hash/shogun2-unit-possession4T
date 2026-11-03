#include "binding_profiles182.h"
#pragma once
// Versioned public Steam ABI, checked against Valve's Proton SDK162 declarations.
// No Steam SDK binary/header is redistributed and no game Steam DLL is replaced.
#include "input174.h"
#include "device192.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <stdexcept>
#include <type_traits>

namespace deck174 {
namespace steam006 {
constexpr int deck_type=14, joystick_mode=6, trigger_mode=10;
constexpr char interface_name[]="SteamInput006";
#pragma pack(push,1)
// These must remain trivial PODs, like Valve's declarations. Default member
// initializers change x86 MSVC's small-structure return convention.
struct Digital { bool state,active; };
struct Analog { int mode;float x,y;bool active; };
struct MotionData {float qx,qy,qz,qw,ax,ay,az,vx,vy,vz;};
#pragma pack(pop)
static_assert(sizeof(Digital)==2&&sizeof(Analog)==13&&sizeof(MotionData)==40,"SteamInput006 ABI drift");
static_assert(std::is_trivial_v<Digital>&&std::is_trivial_v<Analog>&&std::is_trivial_v<MotionData>,"Steam return types must remain trivial");
struct DigitalSpec {const char* name;int origin;};
inline constexpr DigitalSpec digital[button_count]={
    {"A",333},{"B",334},{"X",335},{"Y",336},{"L1",337},{"R1",338},
    {"L3",360},{"R3",367},{"Up",378},{"Down",379},{"Left",380},{"Right",381},
    {"View",340},{"Menu",339},{"L4",373},{"L5",375},{"R4",374},{"R5",376},
    {"LeftPadTouch",341},{"RightPadTouch",348},{"LeftPadClick",343},{"RightPadClick",350},
    {"LeftStickTouch",365},{"RightStickTouch",372}};
inline constexpr const char* analog[6]={"LeftStick","RightStick","LeftPad","RightPad","L2","R2"};
inline constexpr int analog_origin[6]={359,366,342,349,355,357};
constexpr int left_trigger_soft=355,left_trigger_full=356,right_trigger_soft=357,right_trigger_full=358;
inline bool same_physical_origin(int expected,int actual,bool digital_action){
    if(actual==expected)return true;
    // Steam can return both pull origins for the SAME analog trigger. Keep
    // left/right distinct and keep every digital/stick/pad origin exact.
    return !digital_action&&((expected==left_trigger_soft&&actual==left_trigger_full)||
                      (expected==right_trigger_soft&&actual==right_trigger_full));
}
// Minimal versioned public interface. Explicit placeholders retain unused
// SteamInput006 slots. Typed virtual calls preserve MSVC's member-function
// structure-return ABI; an ordinary function-pointer cast does not.
struct Interface {
    virtual bool Init(bool)=0; // 0
    virtual bool Shutdown()=0;
    virtual bool SetInputActionManifestFilePath(const char*)=0;
    virtual void RunFrame(bool)=0;
    virtual void unused4()=0;
    virtual void unused5()=0;
    virtual int GetConnectedControllers(uint64_t*)=0;
    virtual void unused7()=0;
    virtual void unused8()=0;
    virtual uint64_t GetActionSetHandle(const char*)=0;
    virtual void ActivateActionSet(uint64_t,uint64_t)=0;
    virtual uint64_t GetCurrentActionSet(uint64_t)=0;
    virtual void unused12()=0;
    virtual void unused13()=0;
    virtual void unused14()=0;
    virtual void unused15()=0;
    virtual uint64_t GetDigitalActionHandle(const char*)=0;
    virtual Digital GetDigitalActionData(uint64_t,uint64_t)=0;
    virtual int GetDigitalActionOrigins(uint64_t,uint64_t,uint64_t,int*)=0;
    virtual void unused19()=0;
    virtual uint64_t GetAnalogActionHandle(const char*)=0;
    virtual Analog GetAnalogActionData(uint64_t,uint64_t)=0;
    virtual int GetAnalogActionOrigins(uint64_t,uint64_t,uint64_t,int*)=0;
    virtual const char* GetGlyphPNGForActionOrigin(int,int,uint32_t)=0;
    virtual void unused24()=0;
    virtual void unused25()=0;
    virtual void unused26()=0;
    virtual void unused27()=0;
    virtual void unused28()=0;
    virtual MotionData GetMotionData(uint64_t)=0;
    virtual void unused30()=0;
    virtual void unused31()=0;
    virtual void unused32()=0;
    virtual void unused33()=0;
    virtual void unused34()=0;
    virtual void unused35()=0;
    virtual bool ShowBindingPanel(uint64_t)=0; //36, Valve SteamInput006
    virtual int GetInputTypeForHandle(uint64_t)=0;
    virtual void unused38()=0;
    virtual int GetGamepadIndexForController(uint64_t)=0;
    virtual void unused40()=0;
    virtual void unused41()=0;
    virtual void unused42()=0;
    virtual void unused43()=0;
    virtual void unused44()=0;
    virtual uint32_t GetRemotePlaySessionID(uint64_t)=0;
    virtual void unused46()=0;
    virtual void unused47()=0;
};
struct Api {
    void* self{};
    Interface* methods()const{return static_cast<Interface*>(self);}
    bool init()const{return methods()->Init(true);}
    bool shutdown()const{return methods()->Shutdown();}
    bool manifest(const char* p)const{return methods()->SetInputActionManifestFilePath(p);}
    void run()const{methods()->RunFrame(true);}
    int connected(uint64_t* h)const{return methods()->GetConnectedControllers(h);}
    uint64_t action_set(const char* n)const{return methods()->GetActionSetHandle(n);}
    void activate(uint64_t h,uint64_t a)const{methods()->ActivateActionSet(h,a);}
    uint64_t current(uint64_t h)const{return methods()->GetCurrentActionSet(h);}
    uint64_t digital_handle(const char* n)const{return methods()->GetDigitalActionHandle(n);}
    Digital digital_data(uint64_t h,uint64_t a)const{return methods()->GetDigitalActionData(h,a);}
    int digital_origins(uint64_t h,uint64_t a,uint64_t d,int* o)const{return methods()->GetDigitalActionOrigins(h,a,d,o);}
    uint64_t analog_handle(const char* n)const{return methods()->GetAnalogActionHandle(n);}
    Analog analog_data(uint64_t h,uint64_t a)const{return methods()->GetAnalogActionData(h,a);}
    int analog_origins(uint64_t h,uint64_t a,uint64_t d,int* o)const{return methods()->GetAnalogActionOrigins(h,a,d,o);}
    const char* glyph(int origin)const{return methods()->GetGlyphPNGForActionOrigin(origin,0,2);}
    MotionData motion(uint64_t h)const{return methods()->GetMotionData(h);}
    int type(uint64_t h)const{return methods()->GetInputTypeForHandle(h);}
    int gamepad_index(uint64_t h)const{return methods()->GetGamepadIndexForController(h);}
    bool binding_panel(uint64_t h)const{return methods()->ShowBindingPanel(h);}
    uint32_t remote_session(uint64_t h)const{return methods()->GetRemotePlaySessionID(h);}
};
} // steam006

enum class BackendState : unsigned {Idle,WaitingSteam,NoInterface,UnsupportedEnvironment,
    InitializationFailed,ManifestRejected,WaitingDevice,AmbiguousDevice,RemoteDevice,
    WaitingBindings,Ready,WrongThread,Failed};
struct BackendInfo {
    bool initialized{},manifest_accepted{},handles_present{};
    int connected_count{},bad_origin_expected{},bad_origin_observed{};
    uint64_t active_physical_buttons{};std::array<int,6> analog_modes{};
};
class SteamBackend {
    steam006::Api api_{};DWORD owner_{};bool initialized_{};
    uint64_t handle_{},action_set_{},sequence_{},epoch_{1};
    std::array<uint64_t,button_count> digital_{};std::array<uint64_t,6> analog_{};
    uint64_t last_sample_{};
    std::string pending_manifest_;
    std::string context_set_,context_prefix_;
    Context selected_context_{Context::Inactive};
    BackendState state_{BackendState::Idle};int gamepad_index_{-2};mutable BackendInfo info_{};
    bool valid_origins(uint64_t handle,uint64_t action,int expected,bool digital)const {
        int origins[8]{};
        int count=digital?api_.digital_origins(handle,action_set_,action,origins):
            api_.analog_origins(handle,action_set_,action,origins);
        if(count<1||count>8){info_.bad_origin_expected=expected;info_.bad_origin_observed=-1;return false;}
        // Physical controls must be distinct, not a grip aliased to a face key.
        for(int i=0;i<count;++i)if(!steam006::same_physical_origin(expected,origins[i],digital)){
            info_.bad_origin_expected=expected;info_.bad_origin_observed=origins[i];return false;}
        return true;
    }
    bool fetch_handles() {
        if(!action_set_)action_set_=api_.action_set(context_set_.empty()?"DeckHardware174":context_set_.c_str());
        bool valid=action_set_!=0;
        for(unsigned i=0;i<button_count;++i){const bool used=(deck182::buttons(selected_context_)&(uint64_t(1)<<i))!=0;digital_[i]=used?api_.digital_handle((context_prefix_+steam006::digital[i].name).c_str()):0;valid&=!used||digital_[i]!=0;}
        for(unsigned i=0;i<6;++i){const bool used=(deck182::analogs(selected_context_)&(1u<<i))!=0;analog_[i]=used?api_.analog_handle((context_prefix_+steam006::analog[i]).c_str()):0;valid&=!used||analog_[i]!=0;}
        return valid;
    }
public:
    bool select_context(Context context){
        if(initialized_&&owner_!=GetCurrentThreadId())return false;
        const char* prefix=context==Context::Possession?"Possession_":context==Context::RTS?"RTS_":context==Context::Campaign?"Campaign_":"Menu_";
        const char* name=context==Context::Possession?"Possession181":context==Context::RTS?"RTS181":context==Context::Campaign?"Campaign181":"Menu181";
        selected_context_=context;
        if(context_set_!=name){context_set_=name;context_prefix_=prefix;action_set_=0;digital_={};analog_={};++epoch_;last_sample_=0;}
        return true;
    }
    const std::string& context_set()const{return context_set_;}
    BackendState state()const{return state_;}
    BackendInfo info()const{return info_;}
    int emulated_gamepad_index()const{return gamepad_index_;}
    bool initialized()const{return initialized_;}
    const char* glyph_file(int origin)const{return initialized_&&GetCurrentThreadId()==owner_&&info_.manifest_accepted?api_.glyph(origin):nullptr;}
    bool initialize(steam006::Api api,const char* absolute_manifest,bool require_deck=false) {
        if(initialized_||!api.self||!absolute_manifest||!absolute_manifest[0])return false;
        owner_=GetCurrentThreadId();api_=api;info_={};pending_manifest_.clear();
        initialized_=api_.init();
        info_.initialized=initialized_;
        if(!initialized_){state_=BackendState::InitializationFailed;return false;}
        if(require_deck){
            api_.run();uint64_t controllers[16]{};int count=api_.connected(controllers);unsigned decks=0;
            if(count<0||count>16){state_=BackendState::Failed;close();return false;}
            // Discovery can complete on a later RunFrame. Keep this owned Init
            // alive instead of repeatedly shutting it down before data arrives.
            if(!count){pending_manifest_=absolute_manifest;state_=BackendState::WaitingDevice;return true;}
            for(int i=0;i<count;++i)if(controllers[i]&&api_.type(controllers[i])==steam006::deck_type&&!api_.remote_session(controllers[i]))++decks;
            if(decks!=1){state_=decks?BackendState::AmbiguousDevice:BackendState::WaitingDevice;close();return false;}
        }
        if(!api_.manifest(absolute_manifest)){state_=BackendState::ManifestRejected;close();return false;}
        info_.manifest_accepted=true;
        api_.run();state_=BackendState::WaitingDevice;return true;
    }
    bool open_existing(const char* absolute_manifest,bool require_deck=false) {
        if(!device192::enabled()){
            state_=BackendState::UnsupportedEnvironment;return false;
        }
        HMODULE steam=GetModuleHandleW(L"steam_api.dll");
        if(!steam){state_=BackendState::WaitingSteam;return false;}
        using HandleFn=int(__cdecl*)();using ClientFn=void*(__cdecl*)();
        using GenericFn=void*(__cdecl*)(void*,int,int,const char*);
        auto user=reinterpret_cast<HandleFn>(GetProcAddress(steam,"SteamAPI_GetHSteamUser"));
        auto pipe=reinterpret_cast<HandleFn>(GetProcAddress(steam,"SteamAPI_GetHSteamPipe"));
        auto client=reinterpret_cast<ClientFn>(GetProcAddress(steam,"SteamClient"));
        auto generic=reinterpret_cast<GenericFn>(GetProcAddress(steam,"SteamAPI_ISteamClient_GetISteamGenericInterface"));
        if(!user||!pipe||!client||!generic){state_=BackendState::NoInterface;return false;}
        int u=user(),p=pipe();void* c=client();
        if(!u||!p||!c){state_=BackendState::WaitingSteam;return false;}
        void* iface=generic(c,u,p,steam006::interface_name);
        if(!iface){state_=BackendState::NoInterface;return false;}
        // Inspect before indirect calls. No interface probing by arbitrary indices.
        void** table{};SIZE_T read{};
        if(!ReadProcessMemory(GetCurrentProcess(),iface,&table,sizeof table,&read)||read!=sizeof table||!table){state_=BackendState::NoInterface;return false;}
        void* methods[48]{};
        if(!ReadProcessMemory(GetCurrentProcess(),table,methods,sizeof methods,&read)||read!=sizeof methods){state_=BackendState::NoInterface;return false;}
        for(unsigned i:{0u,1u,2u,3u,6u,9u,10u,11u,16u,17u,18u,20u,21u,22u,23u,29u,37u,39u,45u}) {
            MEMORY_BASIC_INFORMATION info{};
            if(!VirtualQuery(methods[i],&info,sizeof info)||info.State!=MEM_COMMIT||
                (info.Protect&(PAGE_GUARD|PAGE_NOACCESS))||
                !(info.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY))) {
                state_=BackendState::NoInterface;return false;
            }
        }
        return initialize({iface},absolute_manifest,require_deck);
    }
    bool sample(uint64_t now,bool focused,Frame& f) {
        f={};f.sample_us=now;f.source=Source::SteamInput006;f.focused=focused;
        if(!initialized_)return false;
        if(GetCurrentThreadId()!=owner_){state_=BackendState::WrongThread;return false;}
        info_.bad_origin_expected=info_.bad_origin_observed=0;
        api_.run();uint64_t handles[16]{};int count=api_.connected(handles);
        info_.connected_count=count;
        if(count<0||count>16){state_=BackendState::Failed;return false;}
        uint64_t selected=0;
        for(int i=0;i<count;++i)if(handles[i]&&api_.type(handles[i])==steam006::deck_type) {
            if(selected){state_=BackendState::AmbiguousDevice;return false;}selected=handles[i];
        }
        if(!selected){if(handle_){handle_=0;++epoch_;}state_=BackendState::WaitingDevice;return false;}
        if(api_.remote_session(selected)){state_=BackendState::RemoteDevice;return false;}
        if(!pending_manifest_.empty()){
            if(!api_.manifest(pending_manifest_.c_str())){state_=BackendState::ManifestRejected;return false;}
            pending_manifest_.clear();info_.manifest_accepted=true;api_.run();
        }
        gamepad_index_=api_.gamepad_index(selected);
        if(handle_!=selected||last_sample_&&now-last_sample_>250000){handle_=selected;++epoch_;}
        last_sample_=now;
        info_.handles_present=fetch_handles();
        if(!info_.handles_present){state_=BackendState::WaitingBindings;return false;}
        if(api_.current(handle_)!=action_set_)api_.activate(handle_,action_set_);
        f.epoch=epoch_;f.sequence=++sequence_;f.connected=true;
        f.capabilities=Sticks|Triggers|Pads|Grips|Motion|StickTouch;
        bool bound=true;
        for(unsigned i=0;i<button_count;++i) {
            if(!digital_[i])continue;
            auto d=api_.digital_data(handle_,digital_[i]);
            const bool physical=d.active&&valid_origins(handle_,digital_[i],steam006::digital[i].origin,true);
            if(physical)f.active_buttons|=uint64_t(1)<<i;
            if(physical&&d.state)f.buttons|=uint64_t(1)<<i;
            if(i<unsigned(Button::LeftStickTouch))bound&=physical;
        }
        if((f.active_buttons&(bit(Button::LeftStickTouch)|bit(Button::RightStickTouch)))!=
            (bit(Button::LeftStickTouch)|bit(Button::RightStickTouch)))f.capabilities&=~StickTouch;
        info_.active_physical_buttons=f.active_buttons;
        Vec2 values[6]{};
        for(unsigned i=0;i<6;++i) {
            if(!analog_[i])continue;
            auto a=api_.analog_data(handle_,analog_[i]);
            info_.analog_modes[i]=a.mode;
            bool good=a.active&&a.mode==(i<4?steam006::joystick_mode:steam006::trigger_mode)&&
                std::isfinite(a.x)&&std::isfinite(a.y)&&
                valid_origins(handle_,analog_[i],steam006::analog_origin[i],false);
            if(!good)bound=false;
            values[i]={a.x,a.y};
        }
        f.left_stick=values[0];f.right_stick=values[1];f.left_pad=values[2];f.right_pad=values[3];
        f.left_trigger=values[4].x;f.right_trigger=values[5].x;
        const auto m=api_.motion(handle_);
        // SDK162 specifies +/-32767 for +/-2000 degrees/s, and +/-2g acceleration.
        constexpr float radians_per_unit=2000.f/32767.f*3.14159265358979323846f/180.f;
        constexpr float acceleration_per_unit=2.f*9.80665f/32767.f;
        f.gyro={m.vx*radians_per_unit,m.vy*radians_per_unit,m.vz*radians_per_unit};
        f.accelerometer={m.ax*acceleration_per_unit,m.ay*acceleration_per_unit,m.az*acceleration_per_unit};
        f.native_bindings=bound;
        if(!bound||!valid(f,now)){state_=BackendState::WaitingBindings;return false;}
        state_=BackendState::Ready;return true;
    }
    bool show_bindings(){
        if(!initialized_||GetCurrentThreadId()!=owner_||state_!=BackendState::Ready||!handle_||!info_.manifest_accepted)return false;
        // The native action set supplies Steam's real labels for this context.
        // Steam owns its diagram, overlay focus and dismissal, not a fake panel.
        api_.activate(handle_,action_set_);return api_.binding_panel(handle_);
    }
    bool close() {
        if(!initialized_)return true;
        if(GetCurrentThreadId()!=owner_){state_=BackendState::WrongThread;return false;}
        bool ok=api_.shutdown();initialized_=false;handle_=action_set_=sequence_=last_sample_=0;
        digital_={};analog_={};pending_manifest_.clear();++epoch_;return ok;
    }
};
} // namespace deck174
