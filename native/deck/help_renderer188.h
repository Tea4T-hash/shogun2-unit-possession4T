#pragma once
#include "presentation188.h"
#include "device192.h"
#include "help_header186.h"
#include "help_draw188.h"
#include "legend_resources184.h"
#include "../common/local_assets159.h"
#include "../outputs/shogun2-ammunition83-preview/hud-src/native_font.h"
#include "../outputs/shogun2-possession-cleanup-v46/src/overlay_surface106.h"
namespace deck188 {
// One bounded in-process guide. Native Present displays it in battle; a
// non-activating owned window is the menu fallback. No Steam screen or files.
class HelpRenderer {
 HWND window{};ULONG_PTR gdi{};assets159::Store assets;hud80::NativeFonts fonts;
 bool loaded{},visible{},fallback_visible{},last_header{};overlay106::Publisher pixels;std::vector<unsigned char> bytes;
 uint64_t next{},next_load{};unsigned last_context{},last_page{},last_w{},last_h{};
 void hide(){if(!visible)return;pixels.hide();if(window&&fallback_visible)ShowWindow(window,SW_HIDE);visible=fallback_visible=false;}
 bool load(){if(loaded)return true;if(GetTickCount64()<next_load)return false;auto folder=deck184::resources();if(folder.empty())return false;next_load=GetTickCount64()+2000;
  if(!gdi){Gdiplus::GdiplusStartupInput startup;if(Gdiplus::GdiplusStartup(&gdi,&startup,nullptr)!=Gdiplus::Ok)return false;}
  if(!assets.load(folder+L"\\hud-assets159.s2fp",1)||!fonts.load_local([&](unsigned id){return assets.get(id);}))return false;loaded=true;return true;
 }
 bool fallback(HWND game,int x,int y,unsigned width,unsigned height){
  if(!window){window=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"Unit Possession controls",WS_POPUP,0,0,1,1,game,nullptr,GetModuleHandleW(nullptr),nullptr);if(!window)return false;}
  POINT origin{x,y};if(!ClientToScreen(game,&origin))return false;
  BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=LONG(width);info.bmiHeader.biHeight=-LONG(height);info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
  HDC screen=GetDC(nullptr),memory=CreateCompatibleDC(screen);void* destination{};HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&destination,nullptr,0);if(!memory||!bitmap||!destination){if(bitmap)DeleteObject(bitmap);if(memory)DeleteDC(memory);if(screen)ReleaseDC(nullptr,screen);return false;}
  memcpy(destination,bytes.data(),bytes.size());auto prior=SelectObject(memory,bitmap);SIZE size{LONG(width),LONG(height)};POINT source{};BLENDFUNCTION blend{AC_SRC_OVER,0,255,AC_SRC_ALPHA};bool ok=UpdateLayeredWindow(window,screen,&origin,&size,memory,&source,0,&blend,ULW_ALPHA)!=FALSE;
  SelectObject(memory,prior);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(nullptr,screen);if(ok)SetWindowPos(window,HWND_TOPMOST,origin.x,origin.y,int(width),int(height),SWP_NOACTIVATE|SWP_SHOWWINDOW);return ok;
 }
public:
 ~HelpRenderer(){hide();if(window)DestroyWindow(window);/* GDI token is process-owned, as for existing native glyph preparation. */}
 template<class Glyph>void tick(HWND game,Glyph glyph){
  Presentation state;RECT client{};if(!game||!IsWindow(game)||GetForegroundWindow()!=game||!device192::enabled()||!presentation188.read(state)||(!state.help&&state.context!=unsigned(deck174::Context::Campaign))||!GetClientRect(game,&client)||client.right<640||client.bottom<360){hide();return;}
  const bool header=!state.help;
  if(!load()){hide();return;}float scale=header?deck186::header_scale(float(client.right)):std::min({1.f,float(client.right-32)/help_width188,float(client.bottom-32)/help_height188});unsigned width=unsigned(std::lround((header?deck186::header_width:help_width188)*scale)),height=unsigned(std::lround((header?deck186::header_height:help_height188)*scale));if(!width||!height||uint64_t(width)*height*4>4u*1024u*1024u)return;
  auto now=GetTickCount64();bool changed=header!=last_header||state.context!=last_context||state.page!=last_page||width!=last_w||height!=last_h;
  const bool refresh=now>=next||changed||!visible;if(refresh){bytes.assign(size_t(width)*height*4,0);Gdiplus::Bitmap surface(width,height,width*4,PixelFormat32bppPARGB,bytes.data());Gdiplus::Graphics g(&surface);g.ScaleTransform(scale,scale);
   if(header){
    if(!deck186::draw_header(g,glyph,[&](const wchar_t* text,Gdiplus::RectF bounds,float size){auto shadow=bounds;shadow.X+=1;shadow.Y+=1;fonts.paint(g,text,shadow,size,Gdiplus::Color(220,25,21,18));fonts.paint(g,text,bounds,size,Gdiplus::Color(255,242,233,207));},true)){hide();return;}
   }else if(!draw_help188(g,deck174::Context(state.context),state.page,glyph,[&](const wchar_t* text,Gdiplus::RectF bounds,float size,bool muted){fonts.paint(g,text,bounds,size,muted?Gdiplus::Color(255,118,62,36):Gdiplus::Color(255,53,42,29));})){hide();return;}
   last_header=header;last_context=state.context;last_page=state.page;last_w=width;last_h=height;next=now+200;
  }
  const int x=header?int(deck186::header_left(float(client.right))):(client.right-int(width))/2,y=header?deck186::header_top:(client.bottom-int(height))/2;
  if(header){
   // Campaign has no battle Present hook. Reuse the same non-activating owned
   // fallback, with a small transparent surface, never a full-screen window.
   pixels.hide();if(refresh)fallback_visible=fallback(game,x,y,width,height);visible=fallback_visible;
   MSG message;while(window&&PeekMessageW(&message,window,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}return;
  }
  overlay106::Frame frame{};frame.key={188,0,0};frame.width=width;frame.height=height;frame.view_width=client.right;frame.view_height=client.bottom;frame.rect_count=1;frame.rect[0]={x,y,int(width),int(height)};
  if(refresh&&pixels.open(GetCurrentProcessId(),4))pixels.publish(frame,bytes.data());
  overlay106::Ack ack;if(pixels.ack(ack)&&ack.key.lease==188&&!ack.key.actor&&!ack.key.unit){if(window&&fallback_visible)ShowWindow(window,SW_HIDE);fallback_visible=false;}else if(refresh)fallback_visible=fallback(game,x,y,width,height);visible=true;
  MSG message;while(window&&PeekMessageW(&message,window,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
 }
};
}
