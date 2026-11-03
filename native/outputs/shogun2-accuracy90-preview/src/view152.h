#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <cstdint>
#include "animation_view.h"
namespace view152 {
constexpr float yaw_limit=.610865238f,pitch_limit=.436332313f; // Eye look: 35 / 25 degrees.
inline bool eye(const deathpose::Eye& native,float yaw,float pitch,deathpose::Eye& out){
    if(!std::isfinite(yaw)||!std::isfinite(pitch))return false;
    float forward[3],up[3],right[3];for(unsigned i=0;i<3;++i){forward[i]=native.view[i+3]-native.view[i];up[i]=native.up[i];}
    if(!deathpose::normalize(forward)||!deathpose::normalize(up))return false;
    deathpose::cross(up,forward,right);if(!deathpose::normalize(right))return false;
    deathpose::cross(forward,right,up);if(!deathpose::normalize(up))return false;
    yaw=std::clamp(yaw,-yaw_limit,yaw_limit);pitch=std::clamp(pitch,-pitch_limit,pitch_limit);
    float cy=std::cos(yaw),sy=std::sin(yaw),cp=std::cos(pitch),sp=std::sin(pitch);
    out=native;
    for(unsigned i=0;i<3;++i){float facing=forward[i]*cy+right[i]*sy;out.view[i+3]=native.view[i]+facing*cp+up[i]*sp;out.up[i]=up[i]*cp-facing*sp;}
    float y{},p{};return animationview::angles(out,y,p);
}
struct Input {
    bool held{};unsigned toggles{};int wheel{};
    void key(std::uint8_t* current,std::uint8_t* previous,bool owned,bool valid){
        // P (0x19) or V (0x2F) switches first/third person.
        if(!valid)return;bool down=((current[0x19]|current[0x2F])&0x80)!=0;
        if(owned&&down&&!held)++toggles;
        if(owned||held)current[0x19]=previous[0x19]=current[0x2F]=previous[0x2F]=0;
        held=down&&(owned||held);
    }
};
}
static SRWLOCK view_lock152=SRWLOCK_INIT;
static view152::Input view_input152;
static volatile LONG third_person152;
static volatile LONG default_third_person160;
static void view_default160(bool third){InterlockedExchange(&default_third_person160,third?1:0);}
static unsigned view_lease152;
static bool orbit_held154;static float orbit_yaw154,orbit_pitch154;
static void orbit_button154(bool held){AcquireSRWLockExclusive(&view_lock152);orbit_held154=held&&third_person152;if(!orbit_held154)orbit_yaw154=orbit_pitch154=0;ReleaseSRWLockExclusive(&view_lock152);}
static bool orbit_mouse154(float yaw,float pitch){AcquireSRWLockExclusive(&view_lock152);bool active=orbit_held154&&third_person152;if(active){orbit_yaw154=std::remainder(orbit_yaw154+yaw,6.283185307f);orbit_pitch154=std::clamp(orbit_pitch154+pitch,-1.396263402f,1.396263402f);}ReleaseSRWLockExclusive(&view_lock152);return active;}
static float view_distance152=3.f,eye_yaw152,eye_pitch152;
// Third person keeps the player's world-facing view while the native body
// turns into a new combat pair. The native eye position remains its pivot.
static bool view_facing156(deathpose::Eye& eye,float yaw,float pitch){
    if(!std::isfinite(yaw)||!std::isfinite(pitch))return false;
    float cp=std::cos(pitch),sp=std::sin(pitch),sy=std::sin(yaw),cy=std::cos(yaw);
    eye.view[3]=eye.view[0]+sy*cp;eye.view[4]=eye.view[1]+sp;eye.view[5]=eye.view[2]+cy*cp;
    eye.up[0]=-sy*sp;eye.up[1]=cp;eye.up[2]=-cy*sp;return true;
}
static void eye_reset152(){AcquireSRWLockExclusive(&view_lock152);eye_yaw152=eye_pitch152=0;ReleaseSRWLockExclusive(&view_lock152);}
static void eye_mouse152(float yaw,float pitch){AcquireSRWLockExclusive(&view_lock152);eye_yaw152=std::clamp(eye_yaw152+yaw,-view152::yaw_limit,view152::yaw_limit);eye_pitch152=std::clamp(eye_pitch152+pitch,-view152::pitch_limit,view152::pitch_limit);ReleaseSRWLockExclusive(&view_lock152);}
static bool eye_apply152(const deathpose::Eye& native,deathpose::Eye& out){AcquireSRWLockShared(&view_lock152);float y=eye_yaw152,p=eye_pitch152;ReleaseSRWLockShared(&view_lock152);return view152::eye(native,y,p,out);}
static void view_key152(std::uint8_t* current,std::uint8_t* previous,bool owned,bool valid){AcquireSRWLockExclusive(&view_lock152);view_input152.key(current,previous,owned,valid);ReleaseSRWLockExclusive(&view_lock152);}
static void view_wheel152(short delta){AcquireSRWLockExclusive(&view_lock152);view_input152.wheel=std::clamp(view_input152.wheel+int(delta),-12000,12000);ReleaseSRWLockExclusive(&view_lock152);}
static void view_reset152(){AcquireSRWLockExclusive(&view_lock152);view_input152.toggles=0;view_input152.wheel=0;eye_yaw152=eye_pitch152=0;orbit_held154=false;orbit_yaw154=orbit_pitch154=0;view_lease152=0;InterlockedExchange(&third_person152,0);ReleaseSRWLockExclusive(&view_lock152);}

static bool view_held152(){AcquireSRWLockShared(&view_lock152);bool held=view_input152.held;ReleaseSRWLockShared(&view_lock152);return held;}
