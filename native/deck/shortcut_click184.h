#pragma once
#include <cstdint>
namespace deck184 {
struct ShortcutClick {
 unsigned x{},y{},phase{};uint64_t started{},tick{};unsigned frame{};
 bool begin(unsigned px,unsigned py,uint64_t now,unsigned ui_frame){if(phase)return false;x=px;y=py;phase=1;started=tick=now;frame=ui_frame;return true;}
 template<class Host>void cancel(Host& host){if(phase==3)host.button(true,false);phase=0;}
 template<class Host>void step(uint64_t now,unsigned ui_frame,Host& host,bool allowed){
  if(!phase)return;
  if(!allowed||now<started||now-started>500){cancel(host);return;}
  if(phase==1){if(!host.target(int(x),int(y))){cancel(host);return;}phase=2;tick=now;frame=ui_frame;return;}
  if(ui_frame==frame||now-tick<16)return;
  if(phase==2){if(!host.button(true,true)){cancel(host);return;}phase=3;tick=now;frame=ui_frame;return;}
  if(phase==3){host.button(true,false);phase=0;}
 }
};
}
