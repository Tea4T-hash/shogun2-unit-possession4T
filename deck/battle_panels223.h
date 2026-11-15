#pragma once
#include "native214.h"
namespace deck223 {
// PanelManager.OpenPanels includes the ordinary finish_deployment panel (kind2,
// present in BOTH lists). It is not a pause menu. The live deployment graph and
// original registration109.../1089d80 establish this one exception; all other
// registered panels retain the normal menu block. The same registered
// deployment control can be hidden while it is being shown or dismissed.
inline bool finish_deployment(uintptr_t image,uintptr_t widget,uintptr_t root){
 uintptr_t primary{},secondary{},getter{},name{},parent{};unsigned length{},capacity{};unsigned char visible{};
 if(!widget||!root||widget==root||!native214::value(widget,primary)||
    (primary!=image+0x16e2aac&&primary!=image+0x1704cdc)||!native214::value(widget+0x18,secondary)||
    secondary!=image+(primary==image+0x16e2aac?0x16e2b14:0x1704d44)||
    !native214::value(secondary+4,getter)||getter!=image+0xf8e840||
    !native214::value(widget+0xf5,visible)||visible>1)return false;
 // Original GetName f8e840 uses its secondary this+70 = component+88.
 // ecfa0/8c510 define the bounded length/capacity/UTF8 pointer representation.
 if(!native214::value(widget+0x88,length)||length!=17||!native214::value(widget+0x8c,capacity)||capacity<length||capacity>4096||
    !native214::value(widget+0x90,name)||!name)return false;
 char text[18]{};if(!native214::read(name,text,sizeof text)||memcmp(text,"finish_deployment",sizeof text))return false;
 parent=widget;for(unsigned i=0;parent&&parent!=root&&i<20;++i){uintptr_t next{};if(!native214::value(parent+0x94,next)||next==parent)return false;parent=next;}
 return parent==root;
}
inline bool battle_modal(uintptr_t image,bool deployment){
 if(!deployment)return native214::modal(image);
 if(!image||native214::admitted_image!=image||!(native214::admitted_features&1))return true;
 uintptr_t manager{},root{};
 if(!native214::value(image+0x1bc73e4,manager)||!manager||!native214::value(image+0x1bbe364,root)||!root)return true;
 // Verify the original name-layout getter, not only the object's table address.
 constexpr unsigned char expected[]={0x8d,0x41,0x70,0x8b,0x4c,0x24,0x04,0x50,0xe8,0x93,0xe5,0x15,0xff,0x8b,0x44,0x24,0x04,0xc2,0x04,0x00};
 unsigned char actual[sizeof expected]{};if(!native214::read(image+0xf8e840,actual,sizeof actual)||memcmp(actual,expected,sizeof actual))return true;
 for(unsigned offset:{0x30u,0x3cu}){
  unsigned count{};uintptr_t items{};
  if(!native214::value(manager+offset,count)||count>32||!native214::value(manager+offset+4,items)||(count&&!items))return true;
  for(unsigned i=0;i<count;++i){uintptr_t widget{};if(!native214::value(items+i*4,widget)||!finish_deployment(image,widget,root))return true;}
 }
 return false;
}
}
