#pragma once
#include <cmath>
#include <algorithm>
namespace accuracy90 {
inline constexpr float maximum_aimed_spread125=.205f;
inline constexpr float fixed_sight_width_scale125=maximum_aimed_spread125/.235f;
struct Envelope {float half_angle{},horizontal{},vertical{},chance{},calibration{},core{},control{};};
inline bool envelope(float variance,float chance,float calibration,float ratio,float multiplier,Envelope& out){
 if(!std::isfinite(variance)||variance<0||!std::isfinite(chance)||chance<.01f||chance>1.00001f||
    !std::isfinite(calibration)||calibration<=0||!std::isfinite(ratio)||ratio<=0||!std::isfinite(multiplier)||multiplier<0)return false;
 float root=std::sqrt(variance/chance)/calibration;
 float h=2*std::atan(.5f*root*ratio),v=2*std::atan(.5f*root/ratio),angle=.5f*std::max(h,v)*multiplier;
 if(!std::isfinite(angle)||angle<0||angle>=1.570796327f)return false;
 out.half_angle=angle;out.horizontal=h;out.vertical=v;out.chance=chance;out.calibration=calibration;return true;
}
inline bool projected_radius(float half_angle,float projection_y,float physical_height,float& radius){
 if(!std::isfinite(half_angle)||half_angle<0||half_angle>=1.570796327f||!std::isfinite(projection_y)||projection_y<=0||
    !std::isfinite(physical_height)||physical_height<=0)return false;
 radius=physical_height*.5f*projection_y*std::tan(half_angle);return std::isfinite(radius)&&radius>=0;
}
}
