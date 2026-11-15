#include "../../../deck/framing203.h"
#pragma once
#include "view152.h"
// One original world-only query per third-person camera update. No actor grid
// query, unit scan, persistent native pointer, or replacement collision solver.
static float camera_clearance152(const float* start,const float* end){
    struct Hits {unsigned capacity{},count{};uintptr_t data{};} hits;
    struct Segment {unsigned kind{3};float start[3]{},end[3]{};} segment;
    float length2=0,result=0;bool returned=false;
    __try{
        for(unsigned i=0;i<3;++i){if(!std::isfinite(start[i])||!std::isfinite(end[i]))return 0;segment.start[i]=start[i];segment.end[i]=end[i];float d=end[i]-start[i];length2+=d*d;}
        if(length2<.000001f||length2>400)return 0;
        using Query=void(__thiscall*)(void*,void*,const void*,unsigned,unsigned,unsigned,unsigned,unsigned);
        reinterpret_cast<Query>(base+0x158970)(reinterpret_cast<void*>(possession_lease.identity.phase),&hits,&segment,0,1,0x303,0,0);returned=true;
        if(hits.count<=4096&&hits.capacity>=hits.count&&(!hits.count||hits.data)){
            result=std::sqrt(length2);
            if(hits.count){float projection=0;for(unsigned i=0;i<3;++i){float p=field<float>(hits.data,4+i*4);if(!std::isfinite(p)){result=0;break;}projection+=(p-start[i])*(end[i]-start[i]);}
                result=std::clamp(projection/std::sqrt(length2)-.25f,0.f,result);}
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){result=0;}
    if(returned&&hits.data){__try{reinterpret_cast<void(__cdecl*)(void*)>(base+0xf04d0)(reinterpret_cast<void*>(hits.data));}__except(EXCEPTION_EXECUTE_HANDLER){result=0;}}
    return result;
}
static void view_prepare152(){
    if(!state||state->mode!=POSSESSED||!possession_lease.active)return;
    if(dead_view())return; // Keep the view selected at death until normal return.
    AcquireSRWLockExclusive(&view_lock152);
    if(view_lease152!=possession_lease.generation){view_lease152=possession_lease.generation;InterlockedExchange(&third_person152,InterlockedCompareExchange(&default_third_person160,0,0));}
    bool change=(view_input152.toggles&1)!=0;view_input152.toggles=0;
    if(change)InterlockedExchange(&third_person152,!third_person152);
    if(third_person152)view_distance152=std::clamp(view_distance152-float(view_input152.wheel)/120.f*.5f,1.f,12.f);
    view_input152.wheel=0;ReleaseSRWLockExclusive(&view_lock152);
    if(change){orbit_button154(false);restore_soldier_pose();}
}
static void view_camera152(uintptr_t camera){
    if(!third_person152||!camera_owned(camera)||(dead_view()&&!death_third_person156))return;
    __try{
        deathpose::Eye eye;memcpy(eye.view,reinterpret_cast<void*>(camera+8),24);memcpy(eye.up,reinterpret_cast<void*>(camera+0x58),12);
        // Ordinary mouse look changes forward before interface_update rebuilds
        // up. Its native body-up is therefore not orthogonal to a pitched eye.
        // Build the same level basis here; paired animation keeps native roll.
        ViewRay ray{};if(!camera_ray(eye.view,eye.view+3,ray))return;
        if(!animation_view_active78&&!level_camera_up(ray,eye.up))return;
        float yaw{},pitch{};if(!animationview::angles(eye,yaw,pitch))return;
        AcquireSRWLockShared(&view_lock152);float orbit_y154=orbit_yaw154,orbit_p154=orbit_pitch154;bool orbit154=orbit_held154;ReleaseSRWLockShared(&view_lock152);
        if(orbit154){
            yaw+=orbit_y154;pitch=std::clamp(pitch+orbit_p154,-1.396263402f,1.396263402f);
            eye.view[3]=eye.view[0]+std::sin(yaw)*std::cos(pitch);eye.view[4]=eye.view[1]+std::sin(pitch);eye.view[5]=eye.view[2]+std::cos(yaw)*std::cos(pitch);
            if(!camera_ray(eye.view,eye.view+3,ray)||!level_camera_up(ray,eye.up))return;
        }
        float forward[3],end[3];for(unsigned i=0;i<3;++i)forward[i]=eye.view[i+3]-eye.view[i];if(!deathpose::normalize(forward))return;
        // Keep the character beside the centre aim ray throughout scrolling.
        // Query the complete shoulder-offset segment, including native roll.
        float delta203[3],length203{};
        if(!framing203::segment(forward,eye.up,view_distance152,delta203,length203))return;
        for(unsigned i=0;i<3;++i)end[i]=eye.view[i]+delta203[i];
        float fraction203=framing203::fraction(camera_clearance152(eye.view,end),length203);
        for(unsigned i=0;i<3;++i){field<float>(camera,8+i*4)=eye.view[i]+delta203[i]*fraction203;field<float>(camera,0x14+i*4)=field<float>(camera,8+i*4)+forward[i];field<float>(camera,0x58+i*4)=eye.up[i];}
        reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4))(reinterpret_cast<void*>(camera));
        remember_owned_eye(camera);
    }__except(EXCEPTION_EXECUTE_HANDLER){state->error=22;}
}
