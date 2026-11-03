#pragma once
#include "battle_ready181.h"
#include "legend_draw184.h"
// Existing read-only HUD process; bounded small texture and original heartbeat.
static bool deck_legend181(){
 deck181::presentation_pid()=pid;deck181::Sample sample;
 if(!deck181::read_sample(pid,sample)||sample.context!=unsigned(deck174::Context::RTS))return false;
 Status status{};if(!snapshot(status)||!status.ready||status.stop||GetTickCount64()-status.heartbeat>500)return false;
 auto root=read<uintptr_t>(base+0x1bc8180);if(!root||read<uintptr_t>(root)!=base+0x170d284||read<unsigned char>(root+0xca0))return false;
 if(!deck181::battle_ready(base,[](uintptr_t at){return read<uintptr_t>(at);}))return false;
 RECT client{};if(!GetClientRect(game,&client)||client.right<640||client.bottom<360)return false;
 const float scale=deck184::legend_scale(float(client.right));
 const unsigned w=unsigned(std::lround(deck184::legend_width*scale)),h=unsigned(std::lround(deck184::legend_height*scale));
 static std::vector<unsigned char> pixels;pixels.assign(size_t(w)*h*4,0);
 Gdiplus::Bitmap surface(w,h,w*4,PixelFormat32bppPARGB,pixels.data());Gdiplus::Graphics g(&surface);g.ScaleTransform(scale,scale);
 if(!deck184::draw_legend(g,deck174::Context::RTS,[](unsigned slot,deck181::Glyph& glyph){return deck181::reader().glyph(slot,glyph);},[&](const wchar_t* text,Gdiplus::RectF bounds,float size){hud80::native_fonts.paint(g,text,bounds,size,Gdiplus::Color(255,242,233,207));}))return false;
 overlay106::Frame frame{};frame.key={181,0,0};frame.width=w;frame.height=h;frame.view_width=client.right;frame.view_height=client.bottom;frame.rect_count=1;
 frame.rect[0]={(client.right-int(w))/2,deck184::legend_top,int(w),int(h)};
 if(!pixels106.open(pid,overlay106::HUD)||!pixels106.publish(frame,pixels.data()))return false;
 ShowWindow(overlay,SW_HIDE);return true;
}
