#pragma once
#include "battle_ready181.h"
#include "help_header186.h"
// Read-only HUD process. One fixed small surface, refreshed5Hz, no histories.
static bool deck_header186(){
 deck181::presentation_pid()=pid;deck181::Sample sample;
 if(!deck181::read_sample(pid,sample)||sample.context!=unsigned(deck174::Context::RTS))return false;
 Status status{};if(!snapshot(status)||!status.ready||status.stop||GetTickCount64()-status.heartbeat>500)return false;
 auto root=read<uintptr_t>(base+0x1bc8180);if(!root||read<uintptr_t>(root)!=base+0x170d284||read<unsigned char>(root+0xca0))return false;
 if(!deck181::battle_ready(base,[](uintptr_t at){return read<uintptr_t>(at);}))return false;
 RECT client{};if(!GetClientRect(game,&client)||client.right<640||client.bottom<360)return false;
 static uint64_t next{};static int last_w{},last_h{};
 if(GetTickCount64()<next&&client.right==last_w&&client.bottom==last_h)return true;
 const float scale=deck186::header_scale(float(client.right));const unsigned w=unsigned(std::lround(deck186::header_width*scale)),h=unsigned(std::lround(deck186::header_height*scale));
 static std::vector<unsigned char> pixels;pixels.assign(size_t(w)*h*4,0);
 Gdiplus::Bitmap surface(w,h,w*4,PixelFormat32bppPARGB,pixels.data());Gdiplus::Graphics g(&surface);g.ScaleTransform(scale,scale);
 if(!deck186::draw_header(g,[](unsigned slot,deck181::Glyph& glyph){return deck181::reader().glyph(slot,glyph);},[&](const wchar_t* text,Gdiplus::RectF bounds,float size){auto shadow=bounds;shadow.X+=1;shadow.Y+=1;hud80::native_fonts.paint(g,text,shadow,size,Gdiplus::Color(220,25,21,18));hud80::native_fonts.paint(g,text,bounds,size,Gdiplus::Color(255,242,233,207));}))return false;
 overlay106::Frame frame{};frame.key={186,0,0};frame.width=w;frame.height=h;frame.view_width=client.right;frame.view_height=client.bottom;frame.rect_count=1;
 frame.rect[0]={int(deck186::header_left(float(client.right))),deck186::header_top,int(w),int(h)};
 if(!pixels106.open(pid,overlay106::HUD)||!pixels106.publish(frame,pixels.data()))return false;
 last_w=client.right;last_h=client.bottom;next=GetTickCount64()+200;ShowWindow(overlay,SW_HIDE);return true;
}
