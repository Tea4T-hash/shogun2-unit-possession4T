#include "../../../deck/hud_cycle198.h"
#pragma once
#include "../../shogun2-possession-cleanup-v46/src/hud_visibility148.h"
static HANDLE hud_visibility_mapping148;
static hudvisibility148::Shared* hud_visibility_shared148;
static void hud_visibility_create148(){
    wchar_t name[96];hudvisibility148::name(name,GetCurrentProcessId());
    hud_visibility_mapping148=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(hudvisibility148::Shared),name);
    require(hud_visibility_mapping148!=nullptr,"HUD visibility mapping");
    hud_visibility_shared148=static_cast<hudvisibility148::Shared*>(MapViewOfFile(hud_visibility_mapping148,FILE_MAP_ALL_ACCESS,0,0,sizeof(hudvisibility148::Shared)));
    require(hud_visibility_shared148!=nullptr,"HUD visibility view");
    new(hud_visibility_shared148)hudvisibility148::Shared;hud_visibility_shared148->pid=GetCurrentProcessId();
}
static void hud_visibility_sync151(bool valid,bool owned){
    if(!hud_visibility_shared148||!valid||!owned)return;
    unsigned hidden;uintptr_t root198{};
    __try{
        auto root=*reinterpret_cast<const uintptr_t*>(base+0x1bc8180);
        if(!root)return;
        root198=root; // Native RTS flag remains authoritative; mod visibility has three stages.
        hidden=*reinterpret_cast<const unsigned char*>(root+0xca0);if(hidden>1)return;
    }__except(EXCEPTION_EXECUTE_HANDLER){return;}
    hidden=hudcycle198::hidden(root198,state?state->lease_generation:0,enabled&&state&&state->mode==POSSESSED&&!state->stop&&!state->release,hidden);
    if(hud_visibility_shared148->hidden==hidden)return;
    InterlockedIncrement(&hud_visibility_shared148->sequence);MemoryBarrier();
    hud_visibility_shared148->hidden=hidden;++hud_visibility_shared148->changes;
    MemoryBarrier();InterlockedIncrement(&hud_visibility_shared148->sequence);
}
static void hud_visibility_close148(){
    if(hud_visibility_shared148){UnmapViewOfFile(hud_visibility_shared148);hud_visibility_shared148=nullptr;}
    if(hud_visibility_mapping148){CloseHandle(hud_visibility_mapping148);hud_visibility_mapping148=nullptr;}
}
