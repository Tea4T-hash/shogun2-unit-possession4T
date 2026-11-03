#pragma once
#include "ability_protocol141.h"
#include "ability_catalog141.h"
struct AbilityRequest141 {unsigned lease{},steps{};bool use{};};
static SRWLOCK ability_input_lock141=SRWLOCK_INIT;
static AbilityRequest141 ability_request141;
static unsigned ability_previous141;
static __declspec(thread) unsigned ability_target_query143;
static HANDLE ability_map141;static ability141::Status* ability_shared141;
static unsigned ability_lease141,ability_selected141=~0u,ability_error141;
static uint64_t ability_next141,ability_pending_until141;
static void ability_reset141(){
 AcquireSRWLockExclusive(&ability_input_lock141);ability_request141={};ReleaseSRWLockExclusive(&ability_input_lock141);
 ability_lease141=0;ability_selected141=~0u;ability_next141=ability_pending_until141=0;ability_error141=0;
}
static void ability_sample141(bool focus){
 unsigned held=(GetAsyncKeyState('G')&0x8000?1u:0u)|(GetAsyncKeyState('F')&0x8000?2u:0u);
 unsigned edges=held&~ability_previous141;ability_previous141=held;
 bool active=focus&&state&&state->mode==POSSESSED&&!state->release&&!state->stop&&!dead_view();
 AcquireSRWLockExclusive(&ability_input_lock141);
 if(active){if(ability_request141.lease!=state->lease_generation)ability_request141={state->lease_generation};
  if(edges&1)ability_request141.steps=std::min(ability_request141.steps+1,64u);
  if(edges&2)ability_request141.use=true;
 }else ability_request141={};
 ReleaseSRWLockExclusive(&ability_input_lock141);
}
static void ability_initialize141(){
 wchar_t name[96];ability141::mapping(name,GetCurrentProcessId());
 ability_map141=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(ability141::Status),name);
 require(ability_map141&&GetLastError()!=ERROR_ALREADY_EXISTS,"Ability mapping already loaded");
 ability_shared141=static_cast<ability141::Status*>(MapViewOfFile(ability_map141,FILE_MAP_ALL_ACCESS,0,0,sizeof(ability141::Status)));
 require(ability_shared141!=nullptr,"Ability mapping failed");memset(ability_shared141,0,sizeof(*ability_shared141));
 ability_shared141->magic=ability141::Magic;ability_shared141->version=141;ability_shared141->size=sizeof(*ability_shared141);ability_shared141->pid=GetCurrentProcessId();
}
static void ability_process141(uintptr_t camera);
