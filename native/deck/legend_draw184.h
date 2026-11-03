#pragma once
#include "glyphs181.h"
namespace deck184 {
inline constexpr int legend_width=1184,legend_height=92,legend_top=6;
struct Hint {unsigned button;const wchar_t* text;};
inline const Hint* hints(deck174::Context context){
 using deck174::Button;
 static const Hint campaign[]={{25,L"Select / interact"},{24,L"Move / attack"},{unsigned(Button::L1),L"Previous selection"},{unsigned(Button::R1),L"Next selection"},{unsigned(Button::Y),L"Hold: end turn"},{unsigned(Button::R4),L"Hold: quick-save"},{unsigned(Button::View),L"Encyclopedia"},{unsigned(Button::X),L"Zoom home"}};
 static const Hint battle[]={{25,L"Select / drag box"},{24,L"Order; double: run"},{unsigned(Button::L3),L"Hold: add selection"},{unsigned(Button::Y),L"Possess soldier"},{unsigned(Button::A),L"Hold: formations / range"},{unsigned(Button::X),L"Toggle run / walk"},{unsigned(Button::R5),L"Group / ungroup"},{unsigned(Button::L5),L"Hold: assign group"}};
 static const Hint possession[]={{25,L"Fire / melee"},{24,L"Ready / defend"},{unsigned(Button::L3),L"Hold: run"},{unsigned(Button::Y),L"Return to RTS"},{unsigned(Button::L1),L"Unit order / drag"},{unsigned(Button::R1),L"Hold: command wheel"},{unsigned(Button::A),L"Climb wall"},{unsigned(Button::B),L"Mount / dismount"}};
 return context==deck174::Context::Campaign?campaign:context==deck174::Context::RTS?battle:possession;
}
inline float legend_scale(float width){return std::min(1.f,(width-24.f)/float(legend_width));}
template<class Glyph,class Text>bool draw_legend(Gdiplus::Graphics& g,deck174::Context context,Glyph glyph,Text text){
 if(context!=deck174::Context::Campaign&&context!=deck174::Context::RTS&&context!=deck174::Context::Possession)return false;
 Gdiplus::SolidBrush back(Gdiplus::Color(226,33,29,22));g.FillRectangle(&back,0,0,legend_width,legend_height);
 const auto* entries=hints(context);
 for(unsigned i=0;i<8;++i){const float x=8+float(i%4)*296,y=3+float(i/4)*44;deck181::Glyph value;
  if(!glyph(entries[i].button,value))return false;
  Gdiplus::Bitmap image(deck181::glyph_side,deck181::glyph_side,deck181::glyph_side*4,PixelFormat32bppPARGB,value.bgra);
  g.DrawImage(&image,Gdiplus::RectF(x,y,42,42));text(entries[i].text,Gdiplus::RectF(x+50,y+2,232,38),20.f);
 }
 return true;
}
}
