#pragma once
#include <windows.h>
#include <cstdint>
constexpr uint32_t POC_MAGIC=0x31504353;
constexpr uint32_t POC_VERSION=46;
// No native pointers cross the control/status channel.
struct Status {
    uint32_t magic,version,pid,size;
    volatile LONG stop,ready,mode,error,input,acquire,release;
    volatile LONG sequence;
    uint64_t heartbeat,frame;
    uint32_t unit_id,actor_index;
    uint32_t actor_id,lease_generation,identity_releases;
    uint32_t native_motion_thread,camera_input_thread,camera_callback_thread,acquisition_posts,camera_acquisitions,acquisition_error;
    float native_walk_speed,effective_speed,native_movement_factor,native_turn_rate;
    uint32_t native_intent_calls,native_intent_errors,native_intent_flag;
    float native_intent_magnitude,native_intent_x,native_intent_z,motion_animation_speed;
    uint32_t conical_shots,cone_errors;
    float cone_half_angle,native_spread_horizontal,native_spread_vertical;
    float before[3],after[3],velocity[3];
    uint32_t bearing,other_updates,controlled_updates,camera_cancels,camera_suppressed,cancel_caller;
    float yaw,pitch,native_eye_y,raised_eye_y;
    uint32_t mouse_events;
    volatile LONG pending_order,follow;
    uint32_t orders_sent,last_order,order_error,packet_size,queue_before,queue_after;
    uint32_t window_thread,camera_thread,formation_count;
    float order_target_x,order_target_z,formation_x,formation_z;
    volatile LONG fire_request,fire_ticket,fire_guard;
    uint32_t projectile_camera_suppressed;
    uint32_t shots_queued,shots_spawned,shot_error,shot_actor,shot_unit,shot_index,shot_packet_size;
    uint32_t shot_thread,shot_origin_thread;
    float ammo_before,ammo_after,shot_target[3];
    uint64_t last_motion_tick;
    volatile LONG weapon_phase;
    uint32_t weapon_record,weapon_animation,draws_started,reloads_started,ai_shots_suppressed;
    float weapon_release_time,weapon_reload_remaining;
    uint32_t weapon_mouse_captured;
    volatile LONG aiming;
    uint32_t hud_hidden,crosshair_visible,interface_error,interface_restores,cursor_suppressed,aim_pose;
    float aim_blend,hip_fov,view_fov,view_origin[3];
    uint32_t aim_clip,native_pose_clip;
    uint32_t shots_blocked,shots_canceled;
    uint32_t spread_removed,direct_fire;
    float shot_origin[3],shot_velocity[3],shot_view_direction[3],launch_delta;
    float camera_pitch;
    uint32_t native_matrix_override;
    uint32_t raw_mouse_active,raw_mouse_error,raw_mouse_events,raw_aim_events;
    LONG raw_y_total,raw_aim_y_total;
    uint32_t pose_updates,pose_aim_updates,pose_restores,pose_error,pose_bones;
    float pose_pitch;
    uint32_t pose_muzzle_updates;
    uint32_t pose_weapon_record,pose_native_clip;
    float pose_native_bias;
    uint32_t bow_origin_aligned;
    float bow_origin_correction,shot_camera_origin[3];
    uint32_t arrow_release_overrides,arrow_release_errors;
    uint32_t arrow_visual_updates,arrow_visual_hidden,arrow_visual_bone;
    uint32_t bow_attachment_origins,bow_attachment_misses;
    float bow_attachment_origin[3];
    float ungraded_speed,terrain_grade,terrain_speed_factor,terrain_probe_distance;
    uint32_t terrain_queries,terrain_errors;
    float surface_speed_factor,fatigue_speed_factor,requested_speed,previous_motion_speed;
    float native_acceleration,native_deceleration;
    uint32_t ground_type,speed_modifier_queries,speed_step_updates,speed_modifier_errors;
    uint32_t fatigue_updates,fatigue_action_updates,fatigue_errors,fatigue_grade_restores,fatigue_action_mode;
    LONG fatigue_before,fatigue_after,fatigue_native_action_cost,fatigue_personal_action_cost;
    float fatigue_personal_grade;
    unsigned char original_code[16],restored_code[16];
};
enum { IDLE=0, ARMED=1, POSSESSED=2, DISABLED=3 };
enum { FORWARD=1, BACKWARD=2, LEFT=4, RIGHT=8, TURN_LEFT=16, TURN_RIGHT=32, RUN=64, WALK_OVERRIDE=128 };
inline void mapping_name(wchar_t* out,DWORD pid){swprintf_s(out,80,L"Local\\Shogun2PossessionPOC46-%lu",pid);}


