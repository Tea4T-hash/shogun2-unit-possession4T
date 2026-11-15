#pragma once
#include "command_policy100.h"
#include "aim_geometry.h"
namespace command100 {
enum class Input:unsigned {Stop,Hold,Follow,Fire,Skirmish};
enum class Pace {Automatic,Walk,Run};
struct Frame {
    Point position{};float yaw{};
    std::uint64_t tick{};unsigned lease{},actor{},unit{};
};
struct Event {Input input{};Frame frame{};std::uint64_t tick{};bool running102{};Pace pace{};};

}
// Window messages preserve short taps. Only numeric published camera data crosses
// to the input path; native object reads and all native commands stay on the
// validated camera/window boundary.
static SRWLOCK command_input_lock100=SRWLOCK_INIT;
static command100::Frame command_frame100;
static command100::Event command_events100[32];
static unsigned command_read100,command_write100,command_key_held100;
static uint64_t command_fire_press_tick62;
static unsigned command_fire_press_lease62;
static void command_cancel_fire62(){
    command_fire_press_tick62=0;
    command_fire_press_lease62=0;
    command_key_held100&=~command100::key_bit('R');
}
static bool command_window_fire62(command100::Input input);
static command100::Tap102 command_q_tap102,command_z_tap102;
static command100::Frame command_z_anchor102;
static volatile LONG command_placing100,command_input_overflow100,command_recenter100;
static constexpr UINT COMMAND_CANCEL_MESSAGE100=WM_APP+0x53c;
static bool command_window100(HWND window,UINT message,WPARAM key,LPARAM flags);
static void command_input_reset100(){
    AcquireSRWLockExclusive(&command_input_lock100);command_read100=command_write100=0;command_frame100={};command_q_tap102.reset();command_z_tap102.reset();command_z_anchor102={};
    ReleaseSRWLockExclusive(&command_input_lock100);InterlockedExchange(&command_placing100,0);InterlockedExchange(&command_recenter100,1);
}
static void command_publish_frame100(const command100::Frame& frame){
    AcquireSRWLockExclusive(&command_input_lock100);command_frame100=frame;ReleaseSRWLockExclusive(&command_input_lock100);
}
static bool command_pop100(command100::Event& event){
    AcquireSRWLockExclusive(&command_input_lock100);bool valid=command_read100!=command_write100;
    if(valid)event=command_events100[command_read100++%32];ReleaseSRWLockExclusive(&command_input_lock100);return valid;
}
