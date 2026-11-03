#pragma once
#include "keyboard_policy84.h"
#include "view152.h"
#include "command_policy100.h"
#include "hud_visibility148.h"
using KeyboardPoll84=bool(__cdecl*)();
static KeyboardPoll84 original_keyboard_poll84;
static void* keyboard_poll_target84;
static keyboard84::Filter keyboard_filter84;
static command100::KeyFilter command_keyboard_filter100;
static bool climb_key_held97,mount_key_held154;static volatile LONG mount_request154;
static volatile LONG keyboard_wall98;static unsigned keyboard_wall_held98;
static volatile LONG keyboard_held84,keyboard_polls84,keyboard_blocked84,keyboard_last_mask84,keyboard_failures84;
static bool keyboard_fps84(){return enabled&&state&&state->mode==POSSESSED&&!state->stop&&!state->release;}
static bool __cdecl keyboard_poll84(){
    bool valid=original_keyboard_poll84();
    auto current=reinterpret_cast<std::uint8_t*>(base+0x1b30f58);
    auto previous=reinterpret_cast<std::uint8_t*>(base+0x1b31058);
    // Leave K in BOTH native buffers; stock Hide UI owns the single state.
    // This reads its result on the next input poll, including RTS/entry/exit.
    // The independent QTE and button prompt do not use this preference.
    hud_visibility_sync151(valid,enabled&&state&&!state->stop);
    unsigned blocked=keyboard_filter84.apply(current,previous,keyboard_fps84(),valid);
    if(valid){bool physical=(current[0x21]&0x80)!=0,owned=keyboard_fps84();
        if(owned||climb_key_held97)current[0x21]=previous[0x21]=0;
        climb_key_held97=owned?physical:climb_key_held97&&physical;
    }
    if(valid){const unsigned scans98[]={0x11,0x1e,0x1f,0x20,0x10,0x12,0x2a,0x36};unsigned physical98=0;
        for(unsigned i=0;i<8;++i)if(current[scans98[i]]&0x80)physical98|=1u<<i;
        unsigned owned98=keyboard_fps84()?(keyboard_wall98?255u:15u):0u;
        unsigned masked98=owned98|keyboard_wall_held98;
        keyboard_wall_held98=keyboard_fps84()?(physical98&owned98):keyboard_wall_held98&physical98;
        for(unsigned i=0;i<8;++i)if(masked98&(1u<<i))current[scans98[i]]=previous[scans98[i]]=0;
    }
    if(valid){bool down154=(current[0x32]&0x80)!=0,owned154=keyboard_fps84();
        // Mount/dismount is requested through E (interact201); M stays hidden from the game while possessed.
        if(owned154||mount_key_held154)current[0x32]=previous[0x32]=0;
        mount_key_held154=down154&&(owned154||mount_key_held154);
    }
    view_key152(current,previous,keyboard_fps84(),valid);
    // F1-F4 give unit orders while possessed; hide them from the game.
    if(valid&&keyboard_fps84()){const unsigned orders157[]={0x3B,0x3C,0x3D,0x3E};for(unsigned a:orders157)current[a]=previous[a]=0;}
    command_keyboard_filter100.apply(current,previous,keyboard_fps84(),valid);
    InterlockedExchange(&keyboard_held84,LONG(keyboard_filter84.held));
    InterlockedExchange(&keyboard_last_mask84,LONG(blocked));
    InterlockedIncrement(&keyboard_polls84);
    if(blocked)InterlockedIncrement(&keyboard_blocked84);
    if(!valid)InterlockedIncrement(&keyboard_failures84);
    return valid;
}
static bool keyboard_window_owned84(unsigned key){
    if(key=='M'&&(keyboard_fps84()||mount_key_held154))return true;
    if((key=='P'||key=='V')&&(keyboard_fps84()||view_held152()))return true;
    if(command100::key_bit(key)&&(keyboard_fps84()||(command_keyboard_filter100.held&command100::key_bit(key))))return true;
    return keyboard84::window_bit(key)&&(keyboard_fps84()||(unsigned(keyboard_held84)&keyboard84::window_bit(key)));
}
static void create_keyboard_hook84(){
    hud_visibility_create148();
    keyboard_poll_target84=reinterpret_cast<void*>(base+0xbd75f0);
    // Check relocated absolute operands separately; this function copies the
    // prior keyboard state, polls/reacquires the keyboard, then returns AL.
    const unsigned char prefix[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe};
    auto code=static_cast<unsigned char*>(keyboard_poll_target84);
    require(!memcmp(code,prefix,sizeof prefix)&&
        *reinterpret_cast<const uintptr_t*>(code+8)==base+0x1b30f58&&code[12]==0xbf&&
        *reinterpret_cast<const uintptr_t*>(code+13)==base+0x1b31058&&code[17]==0xf3&&code[18]==0xa5,
        "Native keyboard poll signature differs");
    require(MH_CreateHook(keyboard_poll_target84,reinterpret_cast<void*>(&keyboard_poll84),reinterpret_cast<void**>(&original_keyboard_poll84))==MH_OK,
        "Native keyboard poll hook failed");
}
