#pragma once
#include "steam_input174.h"
#include "proof174.h"
#include <sstream>
#include <iomanip>

namespace deck174 {
constexpr unsigned diagnostic_revision=3;
struct Measurement {
    static constexpr uint64_t duration_us=90000000;
    uint64_t start_us{};bool started{},cancelled{};
    bool start(uint64_t now){if(started||cancelled)return false;start_us=now;started=true;return true;}
    void cancel(){cancelled=true;}
    uint64_t elapsed(uint64_t now)const{return started&&now>=start_us?now-start_us:0;}
    bool finished(uint64_t now)const{return started&&!cancelled&&elapsed(now)>=duration_us;}
};
// Shared production flow: waiting for the user's Start confirmation never
// consumes measurement time. Fixtures inject the clock/prompt/backend only.
template<class Clock,class Prompt,class Sample,class Delay>
bool measure(Measurement& timer,Proof& proof,uint64_t& elapsed,
    Clock now,Prompt prompt,Sample sample,Delay delay){
    elapsed=0;
    if(!prompt()){timer.cancel();return false;}
    if(!timer.start(now()))return false;
    while(!timer.finished(now())){
        auto stamp=now();Frame frame;
        if(sample(stamp,frame))proof.observe(frame,stamp);else ++proof.rejected;
        delay();
    }
    elapsed=timer.elapsed(now());return true;
}
inline const wchar_t* backend_label(BackendState state){
    switch(state){
    case BackendState::Ready:return L"Native bindings ready";
    case BackendState::WaitingBindings:return L"Waiting for valid native bindings";
    case BackendState::WaitingDevice:return L"Steam Deck not detected";
    case BackendState::UnsupportedEnvironment:return L"Run this on Steam Deck through Proton";
    case BackendState::ManifestRejected:return L"Steam rejected the input configuration";
    case BackendState::NoInterface:return L"Native Steam Input interface unavailable";
    case BackendState::RemoteDevice:return L"Remote-play controller refused";
    case BackendState::AmbiguousDevice:return L"More than one Deck controller detected";
    default:return L"Native input initialization incomplete";
    }
}
inline bool motion_observed(const Proof& p){
    for(auto& r:p.gyro)if(r.samples&&r.maximum-r.minimum>.08f)return true;
    return false;
}
inline std::wstring result_text(const Proof& p,BackendState state,BackendInfo info,
    const Measurement& measurement,uint64_t elapsed_us,unsigned legacy_edges,int gamepad,bool shutdown_ok){
    std::wostringstream out;out<<L"Steam Deck input test - revision "<<diagnostic_revision<<L"\n\n";
    if(measurement.cancelled)out<<L"Test cancelled.\n";
    else if(!measurement.started)out<<L"The timed test did not start.\n";
    else out<<L"Measured time: "<<std::fixed<<std::setprecision(1)<<double(elapsed_us)/1000000.0<<L" seconds\n";
    out<<L"Accepted reads: "<<p.frames<<L"    Rejected reads: "<<p.rejected<<L"\n"<<backend_label(state)<<L"\n";
    out<<L"Required input coverage: "<<(measurement.started&&p.required_inputs_observed()?L"COMPLETE":L"INCOMPLETE")<<L"\n\n";
    auto count=[&](Button b){return p.presses[unsigned(b)];};
    out<<L"Rear grips - L4: "<<count(Button::L4)<<L"  L5: "<<count(Button::L5)
       <<L"  R4: "<<count(Button::R4)<<L"  R5: "<<count(Button::R5)<<L"\n";
    unsigned separate=0;for(auto b:{Button::L4,Button::L5,Button::R4,Button::R5})if(p.independent_grips&bit(b))++separate;
    out<<L"Grips exercised separately: "<<separate<<L" / 4\n";
    unsigned buttons=0;for(unsigned i=0;i<unsigned(Button::LeftStickTouch);++i)if(p.presses[i])++buttons;
    out<<L"Buttons/touches exercised: "<<buttons<<L" / 22\n";
    if(buttons<22){out<<L"Unrecorded: ";bool first=true;
        for(unsigned i=0;i<unsigned(Button::LeftStickTouch);++i)if(!p.presses[i]){
            if(!first)out<<L", ";first=false;for(auto c:std::string(steam006::digital[i].name))out<<wchar_t(c);
        }out<<L"\n";}
    out<<L"Stick range - left: "<<(p.axes[0].bipolar()&&p.axes[1].bipolar()?L"OK":L"MISSING")
       <<L"   right: "<<(p.axes[2].bipolar()&&p.axes[3].bipolar()?L"OK":L"MISSING")<<L"\n";
    out<<L"Stick clicks - L3: "<<count(Button::L3)<<L"   R3: "<<count(Button::R3)<<L"\n";
    out<<L"Pad range - left: "<<(p.axes[4].bipolar()&&p.axes[5].bipolar()?L"OK":L"MISSING")
       <<L"   right: "<<(p.axes[6].bipolar()&&p.axes[7].bipolar()?L"OK":L"MISSING")<<L"\n";
    out<<L"Pad touch/click - left: "<<count(Button::LeftPadTouch)<<L"/"<<count(Button::LeftPadClick)
       <<L"   right: "<<count(Button::RightPadTouch)<<L"/"<<count(Button::RightPadClick)<<L"\n";
    auto trigger=[&](const wchar_t* name,const Range& range){out<<name<<L": ";
        if(range.samples)out<<std::setprecision(2)<<range.minimum<<L" to "<<range.maximum
            <<(range.trigger()?L" (analog OK)":L" (squeeze slowly through full range)");
        else out<<L"NOT CAPTURED";out<<L"\n";};
    trigger(L"L2",p.axes[8]);trigger(L"R2",p.axes[9]);
    out<<L"Gyro movement: "<<(motion_observed(p)?L"DETECTED":L"NOT CAPTURED")<<L"\n";
    out<<L"Legacy key/mouse edges: "<<legacy_edges<<L"    Emulated gamepad index: "<<gamepad<<L"\n";
    if(info.bad_origin_expected||info.bad_origin_observed)out<<L"Binding mismatch: expected "<<info.bad_origin_expected
        <<L", received "<<info.bad_origin_observed<<L"\n";
    out<<L"Steam Input shutdown: "<<(shutdown_ok?L"completed":L"not confirmed")<<L"\n\n";
    out<<L"Send a photo of THIS screen. Gameplay controls are not enabled.\n"
          L"Coverage is a diagnostic result; full Deck gameplay is still unverified.\n"
          L"Disable the probe and restart the Deck before normal play.";
    return out.str();
}
} // namespace deck174
