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
 static const HelpEntry menu[]={S(29,L"Right pad",L"Move cursor"),S(25,L"RT",L"Select / drag"),S(24,L"LT",L"Right click"),E(A,L"Select"),E(B,L"Back"),S(28,L"Left pad",L"Scroll"),E(Menu,L"Skip intro / game menu"),E(View,L"Hold: controls guide")};
 static const HelpEntry campaign[]={S(26,L"Left stick",L"Move around the map"),S(27,L"Right stick",L"Turn / smooth zoom"),S(29,L"Right pad",L"Move cursor"),S(28,L"Left pad",L"Scroll panel / zoom map"),S(25,L"RT",L"Select / interact"),S(24,L"LT",L"Move / attack"),E(A,L"Toggle movement speed (Space)"),E(B,L"Back / close current panel"),E(X,L"Zoom home"),E(Y,L"Hold: end turn"),E(Menu,L"Game menu"),E(View,L"Hold: controls guide")};
 static const HelpEntry campaign_more[]={E(Up,L"Mastery of Arts"),E(Down,L"Finances"),E(Left,L"Diplomacy"),E(Right,L"Clan Management"),E(L1,L"Previous selection"),E(R1,L"Next selection"),E(L4,L"Hold: SHIFT card range"),E(L5,L"Hold: add / remove selection"),E(R4,L"Hold: quick-save"),E(R5,L"Toggle character move speed"),E(L3,L"Hold: fast pan / CTRL select"),S(29,L"L1 + right pad",L"Scroll the list under cursor")};
 static const HelpEntry battle[]={S(26,L"Left stick",L"Move battle camera"),S(27,L"Right stick",L"Turn / tilt camera"),S(29,L"Right pad",L"Move cursor"),S(28,L"Left pad",L"Zoom camera"),S(25,L"RT",L"Select / drag; double: focus unit"),S(24,L"LT",L"Order; hold / drag formation"),E(L3,L"Hold: CTRL multi-select"),E(A,L"Hold: formations / range (Space)"),E(B,L"Cancel placement / deselect"),E(Y,L"Possess selected soldier"),E(X,L"Toggle run / walk"),E(Menu,L"Game menu")};
 static const HelpEntry battle_more[]={E(L1,L"Zoom in (hold)"),E(R1,L"Zoom out (hold)"),E(L4,L"Hold: SHIFT range / queue orders"),E(L5,L"Hold: add / group modifier"),S(unsigned(B::R4),L"R4 / L4 + R4",L"Melee mode / select all units"),E(R5,L"Group / ungroup units"),E(Up,L"Group 1: tap select / hold assign"),E(Down,L"Group 2: tap select / hold assign"),E(Left,L"Group 3: tap select / hold assign"),E(Right,L"Group 4: tap select / hold assign"),S(24,L"Double LT",L"Run / charge order"),E(View,L"Press: HUD; hold: controls")};
 static const HelpEntry possession[]={S(26,L"Left stick",L"Move your soldier"),S(27,L"Right stick",L"Look around"),S(29,L"Right pad",L"Look; touch enables gyro"),S(25,L"RT",L"Fire / primary melee response"),S(24,L"LT",L"Ready weapon / defend"),E(L3,L"Hold: sprint"),E(L5,L"Hold: crouch"),E(A,L"Climb wall / interact"),E(B,L"Mount / dismount unit"),E(X,L"Toggle unit melee / ranged"),E(Y,L"Return to RTS"),E(R3,L"Hold: first / third person")};
 static const HelpEntry possession_more[]={E(L1,L"Unit order; hold / drag formation"),S(unsigned(B::L1),L"Double L1",L"Unit run / charge order"),E(R1,L"Hold: unit command wheel"),E(R4,L"Stop your unit"),E(R5,L"Follow Me"),E(Up,L"Cycle ability / ammunition"),E(Down,L"Use / end selected ability"),E(Left,L"Third-person camera closer"),E(Right,L"Third-person camera farther"),E(L4,L"Hold: third-person orbit"),E(View,L"Press: cycle HUD visibility"),E(Menu,L"Game menu; stay possessed")};
 #undef W188
 #undef W188_
 #undef E
 #undef S
 if(mode==deck174::Context::Campaign)return {page?L"CAMPAIGN - PANELS & ORDERS":L"CAMPAIGN - MAP CONTROLS",page?campaign_more:campaign,12};
 if(mode==deck174::Context::RTS)return {page?L"BATTLE - ORDERS & GROUPS":L"BATTLE - CAMERA & SELECTION",page?battle_more:battle,12};
 if(mode==deck174::Context::Possession)return {page?L"POSSESSION - UNIT COMMANDS":L"POSSESSION - YOUR SOLDIER",page?possession_more:possession,12};
 return {L"MENU CONTROLS",menu,8};
}
template<class Glyph,class Text>bool draw_help188(Gdiplus::Graphics& g,deck174::Context mode,unsigned page,Glyph glyph,Text text){
 if(page>1)return false;auto data=help_page188(mode,page);
 Gdiplus::SolidBrush paper(Gdiplus::Color(248,224,209,172)),ink(Gdiplus::Color(255,56,44,32)),red(Gdiplus::Color(255,120,28,23));
 Gdiplus::Pen border(Gdiplus::Color(255,74,52,35),3),line(Gdiplus::Color(100,105,79,48),1);
 g.FillRectangle(&paper,0,0,help_width188,help_height188);g.DrawRectangle(&border,3,3,help_width188-7,help_height188-7);g.FillRectangle(&red,20,64,help_width188-40,3);
 text(data.title,Gdiplus::RectF(28,18,help_width188-56,40),28.f,false);
 text(mode==deck174::Context::Menu?L"1 / 1":page?L"2 / 2":L"1 / 2",Gdiplus::RectF(help_width188-84,22,60,30),18.f,false);
 for(unsigned i=0;i<data.count;++i){const auto& e=data.entries[i];unsigned column=i/6,row=i%6;float x=28.f+column*550.f,y=90.f+row*80.f;deck181::Glyph value{};
  if(glyph(e.button,value)){Gdiplus::Bitmap image(deck181::glyph_side,deck181::glyph_side,deck181::glyph_side*4,PixelFormat32bppPARGB,value.bgra);g.DrawImage(&image,Gdiplus::RectF(x,y,44,44));}
  text(e.key,Gdiplus::RectF(x+54,y,440,25),18.f,true);text(e.action,Gdiplus::RectF(x+54,y+27,440,38),22.f,false);
  g.DrawLine(&line,x,y+70,x+514,y+70);
 }
 if(mode==deck174::Context::Possession&&page)text(L"Hold R1: stick / D-pad to choose; release to order; B cancels",Gdiplus::RectF(28,574,help_width188-56,22),16.f,true);
 g.DrawLine(&line,24,592,help_width188-24,592);
 text(mode==deck174::Context::Menu?L"B: close":L"L1 / R1: page     B: close",Gdiplus::RectF(28,610,help_width188-56,30),22.f,false);
 return true;
}
}
