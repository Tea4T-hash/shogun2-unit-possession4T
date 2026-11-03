#pragma once
#include "input174.h"
#include <string>
#include <sstream>
#include <cstdlib>
#include <set>

namespace deck174 {
inline bool parse_config(const std::string& text,Config& result,std::string& error) {
    if(text.size()>16384){error="Configuration exceeds 16KiB";return false;}
    Config next=result;std::istringstream input(text);std::string line;std::set<std::string> seen;
    auto trim=[](std::string s){auto a=s.find_first_not_of(" \t\r");if(a==std::string::npos)return std::string{};return s.substr(a,s.find_last_not_of(" \t\r")-a+1);};
    while(std::getline(input,line)) {
        auto comment=line.find_first_of(";#");if(comment!=std::string::npos)line.resize(comment);
        line=trim(line);if(line.empty())continue;
        auto equal=line.find('=');if(equal==std::string::npos){error="Expected name=value";return false;}
        auto key=trim(line.substr(0,equal)),value=trim(line.substr(equal+1));
        if(key.empty()||!seen.insert(key).second||value.empty()){error="Empty or duplicate setting";return false;}
        char* end=nullptr;double number=std::strtod(value.c_str(),&end);
        if(!end||*end||!std::isfinite(number)){error="Invalid finite numeric setting";return false;}
        float f=float(number);bool known=true;
        if(key=="left_deadzone")next.left_deadzone=f;
        else if(key=="right_deadzone")next.right_deadzone=f;
        else if(key=="radial_threshold")next.radial_threshold=f;
        else if(key=="trigger_press")next.trigger_press=f;
        else if(key=="trigger_release")next.trigger_release=f;
        else if(key=="look_sensitivity")next.look_sensitivity=f;
        else if(key=="cursor_sensitivity")next.cursor_sensitivity=f;
        else if(key=="camera_pan_sensitivity")next.camera_pan_sensitivity=f;
        else if(key=="camera_turn_sensitivity")next.camera_turn_sensitivity=f;
        else if(key=="gyro_sensitivity")next.gyro_sensitivity=f;
        else if(key=="gyro_smoothing")next.gyro_smoothing=f;
        else if(key=="gyro_deadzone")next.gyro_deadzone=f;
        else if(key=="double_tap_ms"||key=="hold_ms"||key=="overwrite_ms"||key=="stale_ms") {
            if(number<0||number>2000||std::floor(number)!=number){error="Invalid integral timing";return false;}
            auto v=uint32_t(number);
            if(key=="double_tap_ms")next.double_tap_ms=v;else if(key=="hold_ms")next.hold_ms=v;
            else if(key=="overwrite_ms")next.overwrite_ms=v;else next.stale_ms=v;
        }else if(key=="gyro_rts"||key=="gyro_possession"||key=="invert_x"||key=="invert_y"||key=="community_rts") {
            if(number!=0&&number!=1){error="Boolean setting must be 0 or 1";return false;}
            if(key=="gyro_rts")next.gyro_rts=number==1;else if(key=="gyro_possession")next.gyro_possession=number==1;
            else if(key=="invert_x")next.invert_x=number==1;else if(key=="invert_y")next.invert_y=number==1;else next.community_rts=number==1;
        }else known=false;
        if(!known){error="Unknown controller setting";return false;}
    }
    if(!next.valid()){error="Controller setting outside supported bounds";return false;}
    result=next;error.clear();return true;
}
} // namespace deck174
