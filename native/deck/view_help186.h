#pragma once
#include <cstdint>
namespace deck186 {
enum class ViewEvent {None,Controls,HideHud};
struct ViewGesture {
 bool down{},fired{};uint64_t since{};
 void reset(){*this={};}
 ViewEvent sample(bool held,uint64_t now,bool can_hide){
  if(!held){bool tap=down&&!fired;reset();return tap&&can_hide?ViewEvent::HideHud:ViewEvent::None;}
  if(!down){down=true;since=now;}if(now<since){reset();return ViewEvent::None;}
  if(!fired&&now-since>=600000){fired=true;return ViewEvent::Controls;}
  return ViewEvent::None;
 }
};
}
