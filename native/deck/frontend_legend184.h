#pragma once
#include "legend_resources184.h"
#include "legend_draw184.h"
#include "../common/local_assets159.h"
#include "../outputs/shogun2-ammunition83-preview/hud-src/native_font.h"
namespace deck184 {
inline volatile LONG campaign_legend_ready{};
inline bool write_campaign_legend(deck181::Channel& channel,const std::wstring& directory){
 auto path=resources();if(path.empty())return false;
 assets159::Store store;if(!store.load(path+L"\\hud-assets159.s2fp",1))return false;
 hud80::NativeFonts fonts;if(!fonts.load_local([&](unsigned id){return store.get(id);}))return false;
 std::vector<unsigned char> bytes(18+size_t(legend_width)*legend_height*4);bytes[2]=2;bytes[12]=legend_width&255;bytes[13]=legend_width>>8;bytes[14]=legend_height;bytes[16]=32;bytes[17]=0x28;
 Gdiplus::Bitmap bitmap(legend_width,legend_height,legend_width*4,PixelFormat32bppPARGB,bytes.data()+18);
 {Gdiplus::Graphics g(&bitmap);g.Clear(Gdiplus::Color(0,0,0,0));if(!draw_legend(g,deck174::Context::Campaign,[&](unsigned slot,deck181::Glyph& glyph){return channel.glyph(slot,glyph);},[&](const wchar_t* text,Gdiplus::RectF bounds,float size){fonts.paint(g,text,bounds,size,Gdiplus::Color(255,242,233,207));}))return false;}
 // GDI+ uses premultiplied channels; TGA and the game's UIImage use straight.
 for(size_t i=18;i<bytes.size();i+=4){unsigned a=bytes[i+3];if(a)for(unsigned c=0;c<3;++c)bytes[i+c]=static_cast<unsigned char>(std::min(255u,(unsigned(bytes[i+c])*255u+a/2)/a));}
 frontend177::exact_file(directory+L"\\campaign-legend184.tga",bytes.data(),bytes.size());
 InterlockedExchange(&campaign_legend_ready,1);return true;
}
}
