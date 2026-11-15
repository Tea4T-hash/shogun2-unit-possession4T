#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>
#include "native214_generated.h"
namespace native214 {
inline uintptr_t admitted_image{};inline unsigned admitted_features{};
inline bool read(uintptr_t at,void* out,SIZE_T n){SIZE_T got{};return at&&ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),out,n,&got)&&got==n;}
template<class T>inline bool value(uintptr_t at,T& out){return read(at,&out,sizeof out);}
inline bool admit(uintptr_t image,unsigned required=15){
 if(admitted_image!=image)admitted_features=0;
 if(admitted_image==image&&image&&(admitted_features&required)==required)return true;
 BCRYPT_ALG_HANDLE hash{};if(!image||BCryptOpenAlgorithmProvider(&hash,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;
 bool good=true;for(const auto& s:spans){
  if(!(required&s.feature))continue;
  std::vector<unsigned char> body(s.size);if(!s.size||s.size>32768||!read(image+s.rva,body.data(),body.size())){good=false;break;}
  for(unsigned n=0;n<s.count;++n){unsigned offset=relocations[s.start+n];if(offset+4>s.size){good=false;break;}uint32_t v;memcpy(&v,body.data()+offset,4);v-=uint32_t(image-0x10000000u);memcpy(body.data()+offset,&v,4);}
  unsigned char digest[32]{};if(!good||BCryptHash(hash,nullptr,0,body.data(),ULONG(body.size()),digest,32)<0){good=false;break;}
  char text[65]{};const char* digits="0123456789abcdef";for(unsigned n=0;n<32;++n){text[n*2]=digits[digest[n]>>4];text[n*2+1]=digits[digest[n]&15];}
  if(strcmp(text,s.sha)){good=false;break;}
 }
 BCryptCloseAlgorithmProvider(hash,0);if(good){admitted_image=image;admitted_features|=required;}return good;
}
// Exact read-only equivalent of PanelManager.OpenPanels'1096240 predicate.
// The old battle blocker2e5810 is a replay test, not a menu predicate.
inline bool modal(uintptr_t image){
 if(!image||admitted_image!=image||!(admitted_features&1))return false;uintptr_t manager{};unsigned first{},second{};
 return value(image+0x1bc73e4,manager)&&manager&&value(manager+0x30,first)&&value(manager+0x3c,second)&&first<=32&&second<=32&&(first||second);
}
struct BattleSpeed {
 uintptr_t root{},environment{};float previous{1.f};
 void reset(){root=environment=0;previous=1.f;}
 bool apply(uintptr_t image,uintptr_t expected_root,uintptr_t expected_environment,bool pause){
  if(!image||!admit(image,3)||!expected_root||!expected_environment||modal(image))return false;
  uintptr_t current{},env{},widget{};unsigned char replay{},movie{};
  // This is the same BattleUI singleton used by original CycleBattleSpeed.
  if(!value(image+0x1bc8194,current)||current!=expected_root||!value(current+0x110,env)||env!=expected_environment||
     !value(current+0xd18,widget)||!value(env+0x28254,replay)||replay||!value(env+0x281e8,movie)||movie)return false;
  if(root!=current||environment!=env){root=current;environment=env;previous=1.f;}
  if(widget)return controller(image,current,env,widget,pause);
  return call(image,current,env,pause);
 }
 // Original Cycle/Pause/Resume use BattleTimerControl at root+d18->318.
 // Fresh Ghidra constructor10d6dc0 assigns vtable170b5c0; the selector10e3380
 // retains native eligibility/commander checks and updates the native buttons.
 bool controller(uintptr_t image,uintptr_t current,uintptr_t env,uintptr_t widget,bool pause){
  uintptr_t object{},table{},owner{},component{};unsigned mode{};unsigned char enabled{},slow_allowed{};
  if(!value(widget+0x318,object)||!object||!value(object,table)||table!=image+0x170b5c0||
     !value(object+4,component)||component!=widget||!value(object+0x44,owner)||owner!=current||
     !value(object+0x38,mode)||mode>4||!value(object+0x40,enabled)||enabled!=1||
     !value(object+0x41,slow_allowed)||slow_allowed>1||(!pause&&mode!=1&&!slow_allowed))return false;
  return controller_call(image,current,env,widget,object,mode,pause);
 }
 bool controller_call(uintptr_t image,uintptr_t current,uintptr_t env,uintptr_t widget,uintptr_t object,unsigned mode,bool pause){
  __try{
   using Select=void(__thiscall*)(void*,unsigned);using Action=void(__thiscall*)(void*);
   unsigned target=mode==1?2:1;
   if(pause){
    if(mode){reinterpret_cast<Action>(image+0x10e0a00)(reinterpret_cast<void*>(object));target=0;}
    else {unsigned saved{};if(!value(object+0x3c,saved)||saved>5)return false;target=saved==5?2:saved;reinterpret_cast<Action>(image+0x10e20c0)(reinterpret_cast<void*>(object));}
   }else reinterpret_cast<Select>(image+0x10e3380)(reinterpret_cast<void*>(object),target);
   uintptr_t live{},live_env{},live_widget{},live_object{};unsigned selected{};
   return value(image+0x1bc8194,live)&&live==current&&value(live+0x110,live_env)&&live_env==env&&
      value(live+0xd18,live_widget)&&live_widget==widget&&value(widget+0x318,live_object)&&live_object==object&&
      value(object+0x38,selected)&&selected==target;
  }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
 }
 bool call(uintptr_t image,uintptr_t current,uintptr_t env,bool pause){
  __try{
   float speed=reinterpret_cast<float(__thiscall*)(void*)>(image+0x321880)(reinterpret_cast<void*>(env));
   if(!std::isfinite(speed)||speed<0||speed>4.f)return false;
   float target;
   if(pause){if(speed>.001f){previous=speed;target=0.f;}else target=previous;}
   else{target=std::fabs(speed-.4f)<.01f?1.f:.4f;previous=target;}
   // Native speed-order producer, including its own commander/game-state checks.
   reinterpret_cast<void(__thiscall*)(void*,float,unsigned char)>(image+0x10e58a0)(reinterpret_cast<void*>(current),target,0);
   return true;
  }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
 }
};
}
