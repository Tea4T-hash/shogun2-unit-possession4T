#pragma once
#include "glyphs181.h"
namespace deck188 {
inline constexpr int help_width188=1120,help_height188=660;
struct HelpEntry {unsigned button;const wchar_t* key;const wchar_t* action;};
struct HelpPage {const wchar_t* title;const HelpEntry* entries;unsigned count;};
inline HelpPage help_page188(deck174::Context mode,unsigned page){
 using B=deck174::Button;
 #define W188_(x) L##x
 #define W188(x) W188_(x)
 #define E(key,label) {unsigned(B::key),W188(#key),label}
 #define S(slot,key,label) {slot,key,label}
 static const HelpEntry menu[]={
  S(29,L"Right trackpad",L"Move the pointer"),
  S(25,L"R2",L"Click or drag"),
  S(24,L"L2",L"Right-click"),
  E(A,L"Click where the pointer is"),
  E(B,L"Go back"),
  S(28,L"Left trackpad",L"Swipe to scroll"),
  E(Menu,L"Skip the intro or go back"),
  E(View,L"Hold to open or close this guide"),
 };
 static const HelpEntry campaign[]={
  S(26,L"Left stick",L"Move around the map"),
  S(27,L"Right stick",L"Turn or zoom the camera"),
  S(29,L"Right trackpad",L"Move the pointer"),
  S(28,L"Left trackpad",L"Scroll a list or zoom the map"),
  S(25,L"R2",L"Select. Press twice for details."),
  S(24,L"L2",L"Move or attack where you point"),
  E(A,L"Switch between normal and fast travel"),
  E(B,L"Close the window or go back"),
  E(X,L"Jump back to your home province"),
  E(Y,L"Hold to end the turn"),
  E(Menu,L"Open the pause menu"),
  E(View,L"Hold to open or close this guide"),
 };
 static const HelpEntry campaign_more[]={
  S(unsigned(B::Up),L"D-pad up",L"Research a new art"),
  S(unsigned(B::Down),L"D-pad down",L"See your finances"),
  S(unsigned(B::Left),L"D-pad left",L"Open diplomacy with other clans"),
  S(unsigned(B::Right),L"D-pad right",L"Manage your clan"),
  E(L1,L"Previous town, army, or agent"),
  E(R1,L"Next town, army, or agent"),
  E(L4,L"Hold to select several cards in a row"),
  E(L5,L"Hold and press R2 to add or remove units"),
  E(R4,L"Hold to quick-save"),
  E(R5,L"Switch between normal and fast travel"),
  E(L3,L"Hold to move the camera faster"),
  S(29,L"L1 + right trackpad",L"Hold L1 and swipe to scroll a list"),
 };
 static const HelpEntry battle[]={
  E(Y,L"Take control of a soldier"),
  S(25,L"R2",L"Select a unit or drag a box"),
  S(24,L"L2",L"Move or attack. Drag to line up troops."),
  S(29,L"Right trackpad",L"Move the pointer"),
  S(26,L"Left stick",L"Move the camera"),
  S(27,L"Right stick",L"Turn or tilt the camera"),
  E(L3,L"Tap: Commander. Hold + RT: multi-select."),
  E(R3,L"Hold to show formations and range"),
  E(A,L"Halt the selected troops"),
  E(X,L"Switch between walking and running"),
  S(28,L"Left trackpad click",L"Edge: move. Center click and drag: rotate."),
  E(Menu,L"Skip the general's speech or pause"),
 };
 static const HelpEntry battle_more[]={
  E(L1,L"Hold to zoom in"),
  E(R1,L"Hold to zoom out"),
  E(L4,L"Hold to select a range or queue orders"),
  E(L5,L"Tap: fire at will. Hold: skirmish mode."),
  E(R4,L"Turn melee mode on or off"),
  E(R5,L"Tap: group or ungroup. Hold: lock or unlock."),
  S(unsigned(B::Up),L"D-pad up",L"Tap to select group 1. Hold to assign."),
  S(unsigned(B::Down),L"D-pad down",L"Tap to select group 2. Hold to assign."),
  S(unsigned(B::Left),L"D-pad left",L"Tap to select group 3. Hold to assign."),
  S(unsigned(B::Right),L"D-pad right",L"Tap to select group 4. Hold to assign."),
  S(24,L"Press L2 twice",L"Run to a spot or charge an enemy"),
  E(View,L"Tap to hide or show the battle UI"),
 };
 static const HelpEntry possession[]={
  S(26,L"Left stick",L"Move your soldier"),
  S(27,L"Right stick",L"Look around"),
  S(25,L"R2",L"Shoot. For bows, hold then release."),
  S(24,L"L2",L"Aim or ready your ranged weapon"),
  E(L3,L"Toggle sprint"),
  E(B,L"Crouch or stand up"),
  E(Y,L"Go back to commanding the army"),
  E(A,L"Climb a wall when prompted"),
  E(X,L"Switch your unit's weapon mode"),
  E(R3,L"Hold to switch camera views"),
  S(29,L"Right trackpad",L"Look around"),
  E(Menu,L"Open the pause menu"),
 };
 static const HelpEntry possession_more[]={
  E(L1,L"Tell your unit to move or attack"),
  S(unsigned(B::L1),L"Press L1 twice",L"Tell your unit to run or charge"),
  E(R1,L"Hold for orders. A: Commander. RT: assign."),
  E(R5,L"Pause or resume the battle"),
  E(R4,L"Switch slow motion on or off"),
  E(View,L"Tap to change what's shown on screen"),
  S(unsigned(B::Up),L"D-pad up",L"Choose an ability or special ammo"),
  S(unsigned(B::Down),L"D-pad down",L"Use your choice or end an active ability"),
  E(L5,L"Hold to toggle Skirmish"),
  S(unsigned(B::Left),L"D-pad left / right",L"Move the camera closer or farther away"),
  E(L4,L"Hold to look around your soldier"),
  S(28,L"Left trackpad",L"Swipe to adjust the camera distance"),
 };
 #undef W188
 #undef W188_
 #undef E
 #undef S
 if(mode==deck174::Context::Campaign)return {page?L"Campaign shortcuts":L"Campaign map",page?campaign_more:campaign,12};
 if(mode==deck174::Context::RTS)return {page?L"Battle orders and groups":L"Commanding your army",page?battle_more:battle,12};
 if(mode==deck174::Context::Possession)return {page?L"Your unit and its orders":L"Playing as a soldier",page?possession_more:possession,12};
 return {L"Using menus",menu,8};
}
template<class Glyph,class Text>bool draw_help188(Gdiplus::Graphics& g,deck174::Context mode,unsigned page,Glyph glyph,Text text){
 if(page>1)return false;auto data=help_page188(mode,page);
 Gdiplus::SolidBrush paper(Gdiplus::Color(248,224,209,172)),ink(Gdiplus::Color(255,56,44,32)),red(Gdiplus::Color(255,120,28,23));
 Gdiplus::Pen border(Gdiplus::Color(255,74,52,35),3),line(Gdiplus::Color(100,105,79,48),1);
 g.FillRectangle(&paper,0,0,help_width188,help_height188);g.DrawRectangle(&border,3,3,help_width188-7,help_height188-7);g.FillRectangle(&red,20,64,help_width188-40,3);
 text(data.title,Gdiplus::RectF(28,18,help_width188-56,40),28.f,false);
 text(mode==deck174::Context::Menu?L"1 / 1":page?L"2 / 2":L"1 / 2",Gdiplus::RectF(help_width188-84,22,60,30),18.f,false);
 for(unsigned i=0;i<data.count;++i){const auto& e=data.entries[i];unsigned column=i/6,row=i%6;float x=28.f+column*550.f,y=90.f+row*80.f;deck181::Glyph value{};
  if(glyph(e.button,value)){Gdiplus::Bitmap image(deck181::glyph_side,deck181::glyph_side,deck181::glyph_side*4,PixelFormat32bppPARGB,value.bgra);g.DrawImage(&image,Gdiplus::RectF(x,y,56,56));}
  text(e.key,Gdiplus::RectF(x+68,y,440,25),18.f,true);text(e.action,Gdiplus::RectF(x+68,y+27,440,38),22.f,false);
  g.DrawLine(&line,x,y+70,x+514,y+70);
 }
 const wchar_t* tip=nullptr;
 const wchar_t* extra=nullptr;
 if(mode==deck174::Context::Campaign&&page){
  tip=L"With town details open, use L1 and R1 to browse other towns.";
  extra=L"Hold L3 and press R2 to add or remove individual units.";
 }else if(mode==deck174::Context::RTS){
  if(page){
   tip=L"Tap a D-pad direction to select its group. Hold the direction to assign selected troops.";
   extra=L"Hold both pad clicks. Slide thumbs apart to widen or together to narrow. Release to confirm.";
  }else{
   tip=L"Select one of your units and press Y to play as a soldier. Press R2 twice on a unit to jump the camera to it.";
   extra=L"Hold the right trackpad click for formations and range. Press B to cancel placement or clear selection.";
  }
 }else if(mode==deck174::Context::Possession){
  if(page){
   tip=L"Hold RB for orders. Aim the right stick and press RT to confirm. Double-press RT on Hold Here to run. Press A for Commander.";
   extra=L"Hold L1 and drag to line up your unit. For an ability that targets an ally, aim at them before using it.";
  }else{
   tip=L"In a melee fight, press the trigger shown when the marker reaches the green part of the bar.";
   extra=L"To charge, sprint straight at an enemy in melee mode and hold R2. Keep L2 released.";
  }
 }
 if(tip)text(tip,Gdiplus::RectF(28,567,help_width188-56,18),16.f,true);
 if(extra)text(extra,Gdiplus::RectF(28,585,help_width188-56,18),16.f,true);
 g.DrawLine(&line,24,607,help_width188-24,607);
 text(mode==deck174::Context::Menu?L"B to close":L"L1 or R1 to change page     B to close",Gdiplus::RectF(28,616,help_width188-56,29),20.f,false);
 return true;
}
}
