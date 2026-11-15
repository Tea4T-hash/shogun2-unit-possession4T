#pragma once
#include <chrono>
// Called from the existing owned context-camera update after its normal aim
// ray has been captured. Shogun's camera object and manager never change.
enum class ArrowSample122 {Unreadable, Missing, Flying, Impact};
static ArrowSample122 read_arrow_flight107(const arrowcam107::Shot& shot,arrowcam107::Flight& flight,unsigned& slot){
    __try {
        if(!environment||!possessed||!shot.context||!shot.pool||
           field<uintptr_t>(shot.context,0)!=environment||field<uintptr_t>(shot.context,0xe8)!=shot.pool||
           state->lease_generation!=shot.lease)return ArrowSample122::Unreadable;
        const auto actor=possessed;
        const auto array=field<uintptr_t>(shot.pool,0x14);
        const unsigned count=field<unsigned>(shot.pool,0x10);
        if(count>16384||(!array&&count))return ArrowSample122::Unreadable;
        // Register41c760 appends projectiles. Recent arrows normally sit at
        // the tail. 481010 updates previous/current world position and velocity.
        unsigned index=slot;
        uintptr_t projectile=index<count?field<uintptr_t>(array,index*sizeof(uintptr_t)):0;
        if(!projectile||field<unsigned>(projectile,4)!=shot.id||field<uintptr_t>(projectile,0x1c)!=actor){
            projectile=0;
            for(unsigned i=count;i>0;--i){
                const auto candidate=field<uintptr_t>(array,(i-1)*sizeof(uintptr_t));
                if(candidate&&field<unsigned>(candidate,4)==shot.id&&field<uintptr_t>(candidate,0x1c)==actor){
                    index=i-1;
                    projectile=candidate;
                    break;
                }
            }
        }
        if(projectile){
            arrowcam107::Flight check{};
            const auto stopped=field<unsigned char>(projectile,0xc1);
            memcpy(&flight,reinterpret_cast<const void*>(projectile+0x74),sizeof flight);
            MemoryBarrier();
            memcpy(&check,reinterpret_cast<const void*>(projectile+0x74),sizeof check);
            // Original 481010 publishes the terminal position and flag;
            // original 483d40 retires that flagged slot on the next step.
            if(!memcmp(&flight,&check,sizeof flight)&&stopped==field<unsigned char>(projectile,0xc1)&&
               (stopped==1?arrowcam107::finite(flight):stopped==0&&arrowcam107::valid(flight))&&
               field<uintptr_t>(shot.pool,0x14)==array&&field<unsigned>(shot.pool,0x10)==count&&
               field<uintptr_t>(array,index*sizeof(uintptr_t))==projectile&&
               field<unsigned>(projectile,4)==shot.id&&field<uintptr_t>(projectile,0x1c)==actor&&
               possessed==actor&&state->lease_generation==shot.lease){
                slot=index;
                return stopped?ArrowSample122::Impact:ArrowSample122::Flying;
            }
        }else if(field<uintptr_t>(shot.pool,0x14)==array&&field<unsigned>(shot.pool,0x10)==count&&
                 possessed==actor&&state->lease_generation==shot.lease){
            return ArrowSample122::Missing;
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){}
    return ArrowSample122::Unreadable;
}
static bool apply_arrow_view107(uintptr_t camera){
    static thread_local arrowcam107::Presentation presentation;
    static thread_local unsigned slot=arrowcam107::no_projectile;
    if(!arrow_watch107||!input_window||GetForegroundWindow()!=input_window||
       GetCurrentThreadId()!=state->window_thread||state->release||state->stop||dead_view()||
       native_modal214()||commander76::pc_wheel||command_placing100||!camera_owned(camera)){
        presentation={};return false;
    }
    melee60::Request melee{};
    if(melee_request(melee)||wall_subject97(possessed)||native_airborne_subject99(possessed)){
        presentation={};return false;
    }
    const auto shot=arrow_shot107();
    const auto now=GetTickCount64();
    if(!arrowcam107::eligible(shot,state->lease_generation,now,presentation.tracking(shot))){presentation={};return false;}
    arrowcam107::Flight live{};
    if(!presentation.matches(shot))slot=arrowcam107::no_projectile;
    const auto sample=presentation.landed&&presentation.tracking(shot)?ArrowSample122::Unreadable:
        read_arrow_flight107(shot,live,slot);
    const bool observed=sample==ArrowSample122::Flying||sample==ArrowSample122::Impact;
    const bool ended=sample==ArrowSample122::Missing||sample==ArrowSample122::Impact;
    using Clock=std::chrono::steady_clock;
    const double sample_time=std::chrono::duration<double,std::milli>(Clock::now().time_since_epoch()).count();
    if(!presentation.update(shot,sample_time,observed?&live:nullptr,ended))return false;
    __try {
        float eye[3]{},target[3]{},up[3]{};
        if(!arrowcam107::chase(presentation.flight,eye,target))return false;
        const auto& arrow=presentation.flight.position;
        if(presentation.landed){
            target[0]=arrow[0];target[1]=arrow[1]+.12f;target[2]=arrow[2];
        }
        const float point[]={arrow[0],arrow[1]+.25f,arrow[2]};
        const float length=std::hypot(eye[0]-point[0],eye[1]-point[1],eye[2]-point[2]);
        const float clearance=camera_clearance152(point,eye);
        if(clearance<.3f||length<.3f)return false;
        if(clearance<length)for(unsigned i=0;i<3;++i)eye[i]=point[i]+(eye[i]-point[i])*clearance/length;
        ViewRay ray{};
        if(!camera_ray(eye,target,ray)||!level_camera_up(ray,up)||!camera_owned(camera))return false;
        memcpy(reinterpret_cast<void*>(camera+8),eye,sizeof eye);
        memcpy(reinterpret_cast<void*>(camera+0x14),target,sizeof target);
        memcpy(reinterpret_cast<void*>(camera+0x58),up,sizeof up);
        reinterpret_cast<Cancel>(field<uintptr_t>(field<uintptr_t>(camera,0),0xf4))(reinterpret_cast<void*>(camera));
        return true;
    }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}

