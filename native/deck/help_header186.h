#pragma once
#include "glyphs181.h"
namespace deck186 {
inline constexpr int header_width=320,header_height=26,header_top=8;
// Reserve the native top-left menu controls; keep one anchor in every mode.
inline float header_left(float width){return std::min(208.f,width*.1625f);}
inline float header_scale(float width){return std::min(1.f,(width-header_left(width)-12.f)/header_width);}
template<class Glyph,class Text>bool draw_header(Gdiplus::Graphics& g,Glyph glyph,Text text,bool campaign=false){
 deck181::Glyph view;if(!glyph(unsigned(deck174::Button::View),view))return false;
 Gdiplus::Bitmap icon(deck181::glyph_side,deck181::glyph_side,deck181::glyph_side*4,PixelFormat32bppPARGB,view.bgra);
 g.DrawImage(&icon,Gdiplus::RectF(1,2,22,22));
 text(campaign?L"Hold: Controls":L"Tap: HUD   Hold: Controls",Gdiplus::RectF(31,3,287,22),16.f);return true;
}
}
