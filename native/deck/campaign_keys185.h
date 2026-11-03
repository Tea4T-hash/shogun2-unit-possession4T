#pragma once
#include <windows.h>
#include "bindings183.h"
namespace deck185 {
// Original default_keys.xml. Native input owns shortcut restrictions and panel
// toggles; no UI coordinates, Lua panel mutation or synthetic global keys.
struct CampaignKeys {
 static constexpr unsigned handled=deck183::Arts|deck183::Finance|deck183::Diplomacy|deck183::Clan|deck183::EndTurn|deck183::Encyclopedia|deck183::Back|deck183::MoveSpeed;
 struct Entry {unsigned scan{};uint64_t tick{};LONG epoch{};};
 SRWLOCK lock=SRWLOCK_INIT;Entry queue[8]{};unsigned count{};bool gap{};uint64_t last_panel{};
 unsigned modifiers{};uint64_t modifier_tick{};LONG modifier_epoch{};
 void hold(unsigned flags,uint64_t now,LONG epoch){AcquireSRWLockExclusive(&lock);modifiers=flags&(MK_CONTROL|MK_SHIFT);modifier_tick=now;modifier_epoch=epoch;ReleaseSRWLockExclusive(&lock);}
 static bool panel(unsigned scan){return scan==0x32||scan==0x31||scan==0x25||scan==0x26;}
 // Panel creation/destruction is synchronous but its native layout/modal work
 // commits later. Keep only the newest panel request, allowing a brief settle
 // interval rather than replaying a rapid Dpad burst as eight native opens.
 static constexpr uint64_t panel_settle_ms=500;
 void clear(bool retain_settle=false){AcquireSRWLockExclusive(&lock);count=0;gap=false;modifiers=0;modifier_tick=0;if(!retain_settle)last_panel=0;ReleaseSRWLockExclusive(&lock);}
 bool put(unsigned actions,uint64_t now,LONG epoch){
  const unsigned flags[]={deck183::Arts,deck183::Finance,deck183::Diplomacy,deck183::Clan,deck183::EndTurn,deck183::Encyclopedia,deck183::Back,deck183::MoveSpeed};
  const unsigned scans[]={0x32,0x31,0x25,0x26,0x1c,0x57,0x01,0x39};bool ok=true;
  AcquireSRWLockExclusive(&lock);
  if(actions&deck183::Back){count=1;queue[0]={0x01,now,epoch};ReleaseSRWLockExclusive(&lock);return true;}
  for(unsigned i=0;i<8;++i)if(actions&flags[i]){
   if(panel(scans[i])){unsigned existing=0;while(existing<count&&!panel(queue[existing].scan))++existing;
    if(existing<count){queue[existing]={scans[i],now,epoch};continue;}}
   if(count==8){ok=false;break;}queue[count++]={scans[i],now,epoch};
  }
  ReleaseSRWLockExclusive(&lock);return ok;
 }
 bool apply(unsigned char* current,bool valid,bool allowed,uint64_t now,LONG epoch,unsigned* native_action=nullptr){
  if(native_action)*native_action=0;
  AcquireSRWLockExclusive(&lock);bool applied=false;
  if(!allowed||!valid||!current){count=0;gap=false;}
  else if(gap)gap=false;
  else if(count){auto e=queue[0];
   if(e.epoch!=epoch||now<e.tick||now-e.tick>(panel(e.scan)?1000u:250u)){count=0;last_panel=0;}
   else if(panel(e.scan)&&last_panel&&now>=last_panel&&now-last_panel<panel_settle_ms){}
   else{for(unsigned i=1;i<count;++i)queue[i-1]=queue[i];--count;
    if(native_action&&e.scan==0x26)*native_action=deck183::Clan;
    else{current[e.scan]|=0x80;if(e.scan==0x57)current[0x1d]|=0x80;}
    if(panel(e.scan))last_panel=now;gap=true;applied=true;}
  }
  // Original poll has just replaced the DI state. Merge held controller
  // modifiers here, before native UI selection consumes it; never mutate OS
  // keys or retain a modifier across focus/root/controller loss.
  if(allowed&&valid&&current&&modifier_epoch==epoch&&now>=modifier_tick&&now-modifier_tick<=100){
   if(modifiers&MK_CONTROL)current[0x1d]|=0x80;
   if(modifiers&MK_SHIFT)current[0x2a]|=0x80;
  }
  ReleaseSRWLockExclusive(&lock);return applied;
 }
};
}
