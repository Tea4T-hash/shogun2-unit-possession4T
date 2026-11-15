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

// Commander and the RTS controls header share the existing HUD surface owner.
static bool commander_rts_hud76(){
 commander76::Display display;
 if(!commander76::read_display(pid,display)||!(display.flags&commander76::Rts))return false;
 const bool wheel=(display.flags&(commander76::PcWheel|commander76::DeckWheel))!=0;
 if(!wheel){
  if(device192::enabled(pid)||!(display.flags&commander76::Available))return false;
  const auto root=read<uintptr_t>(base+0x1bc8180);
  if(!root||read<unsigned char>(root+0xca0))return false;
 }
 RECT client{};
 if(!GetClientRect(game,&client)||client.right<640||client.bottom<360)return false;
 const float scale=wheel?hudlayout108::scale(float(client.right),float(client.bottom)):
                         deck186::header_scale(float(client.right));
 const int w=wheel?std::min(int(client.right),int(std::ceil(660*scale))):int(std::ceil(deck186::header_width*scale));
 const int h=wheel?std::min(int(client.bottom),int(std::ceil(670*scale))):int(std::ceil(deck186::header_height*scale));
 const int x=wheel?(client.right-w)/2:int(deck186::header_left(float(client.right)));
 const int y=wheel?std::clamp(int(client.bottom*.48f)-h/2,0,int(client.bottom)-h):deck186::header_top;
 static std::vector<unsigned char> pixels;
 const unsigned char* data=nullptr;
 if(wheel){
  std::shared_ptr<card217::Picture> crest;
  const hud80::FactionArt* stock{};
  try{
   const auto root=read<uintptr_t>(base+0x1bc8180);
   const auto env=read<uintptr_t>(root+0x110),common=read<uintptr_t>(env+8);
   const auto army=card_preload.human_army;
   if(army&&card_preload.phase==read<uintptr_t>(common+0xb4)&&read<unsigned char>(army+0x270)==1){
    const auto key=army_clan217(army);
    crest=cards217.crest(key);
    stock=hud80::faction_art(key.c_str());
   }
  }catch(...){}
  static WheelSurface121 surface;
  data=surface.render(assets,client,x,y,w,h,stock?stock->mon:L"",crest,hud80::native_fonts.boshin,
      display.lease,display.sector,0,false,(display.flags&commander76::PcWheel)!=0,
      &display,nullptr,display.pointer);
 }else{
  pixels.assign(size_t(w)*h*4,0);
  Gdiplus::Bitmap surface(w,h,w*4,PixelFormat32bppPARGB,pixels.data());
  Gdiplus::Graphics g(&surface);
  g.ScaleTransform(scale,scale);
  deck186::draw_commander_pc([&](const wchar_t* label,Gdiplus::RectF box,float size){
   auto shadow=box;shadow.X+=1;shadow.Y+=1;
   hud80::native_fonts.paint(g,label,shadow,size,Gdiplus::Color(220,25,21,18));
   hud80::native_fonts.paint(g,label,box,size,Gdiplus::Color(255,242,233,207));
  });
  data=pixels.data();
 }
 overlay106::Frame frame{};
 frame.key={commander76::rts_overlay_key,display.lease,wheel?0u:1u};
 frame.width=w;frame.height=h;frame.view_width=client.right;frame.view_height=client.bottom;
 frame.rect_count=1;frame.rect[0]={x,y,w,h};
 if(!pixels106.open(pid,overlay106::HUD)||!pixels106.publish(frame,data))return false;
 ShowWindow(overlay,SW_HIDE);return true;
}
