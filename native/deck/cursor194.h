#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <vector>
namespace deck194 {
// Native Steam Input changes the focused game's cursor position, but that is
// not necessarily a physical mouse event for Gamescope. Draw the current game
// cursor locally on Proton while the right pad is in use. Never change global
// ShowCursor counts, cursor settings, device input, or the game's cursor image.
struct CursorActivity {
 uint64_t last{};uintptr_t window{};bool seen{};
 void reset(){last=0;window=0;seen=false;}
 bool update(uintptr_t owner,bool allowed,bool touch,uint64_t now){
  if(!allowed||!owner){reset();return false;}
  if(owner!=window){reset();window=owner;}
  if(seen&&now<last){reset();window=owner;}
  if(touch){last=now;seen=true;}
  return seen&&now>=last&&now-last<2000;
 }
};
inline LRESULT CALLBACK cursor_window(HWND window,UINT message,WPARAM w,LPARAM l){
 if(message==WM_NCHITTEST)return HTTRANSPARENT;
 if(message==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
 return DefWindowProcW(window,message,w,l);
}
class CursorRenderer {
 HWND window{};CursorActivity activity;HCURSOR image{};uint64_t redraw{};
 unsigned width{},height{},hot_x{},hot_y{};bool shown{};std::vector<uint32_t> pixels;
 bool create(HWND game){
  if(window)return GetWindow(window,GW_OWNER)==game;
  WNDCLASSEXW cls{sizeof cls};cls.lpfnWndProc=cursor_window;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"S2UP-DeckNativeCursor194";
  if(!RegisterClassExW(&cls)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS)return false;
  window=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,cls.lpszClassName,L"Unit Possession native cursor",WS_POPUP,0,0,1,1,game,nullptr,cls.hInstance,nullptr);
  return window!=nullptr;
 }
 static bool geometry(HCURSOR cursor,unsigned& w,unsigned& h,unsigned& hx,unsigned& hy){
  ICONINFO info{};if(!GetIconInfo(cursor,&info))return false;BITMAP bitmap{};bool ok=GetObjectW(info.hbmColor?info.hbmColor:info.hbmMask,sizeof bitmap,&bitmap)==sizeof bitmap;
  w=unsigned(bitmap.bmWidth);h=unsigned(bitmap.bmHeight)/(info.hbmColor?1:2);hx=info.xHotspot;hy=info.yHotspot;
  if(info.hbmColor)DeleteObject(info.hbmColor);if(info.hbmMask)DeleteObject(info.hbmMask);
  return ok&&w&&h&&w<=128&&h<=128&&hx<w&&hy<h;
 }
 bool render(HCURSOR cursor,POINT at){
  unsigned w{},h{},hx{},hy{};if(!geometry(cursor,w,h,hx,hy))return false;
  BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=LONG(w);info.bmiHeader.biHeight=-LONG(h);info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
  HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);void* bits{};HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&bits,nullptr,0);
  if(!dc||!bitmap||!bits){if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);if(screen)ReleaseDC(nullptr,screen);return false;}
  auto previous=SelectObject(dc,bitmap);memset(bits,0,size_t(w)*h*4);
  bool ok=DrawIconEx(dc,0,0,cursor,int(w),int(h),0,nullptr,DI_NORMAL)!=FALSE;
  if(ok){
   auto* rgba=static_cast<uint32_t*>(bits);pixels.assign(rgba,rgba+size_t(w)*h);
   const bool alpha=std::any_of(pixels.begin(),pixels.end(),[](uint32_t p){return (p>>24)!=0;});
   if(!alpha){
    // Classic masked cursors lack an alpha channel. Use their native AND mask,
    // preserving transparent pixels, rather than drawing an opaque rectangle.
    memset(bits,0,size_t(w)*h*4);ok=DrawIconEx(dc,0,0,cursor,int(w),int(h),0,nullptr,DI_MASK)!=FALSE;
    if(ok)for(size_t i=0;i<pixels.size();++i)pixels[i]=(rgba[i]&0x00ffffffu)==0x00ffffffu?0:(pixels[i]|0xff000000u);
   }
   if(ok){memcpy(bits,pixels.data(),pixels.size()*4);POINT origin{at.x-LONG(hx),at.y-LONG(hy)},source{};SIZE size{LONG(w),LONG(h)};BLENDFUNCTION blend{AC_SRC_OVER,0,255,AC_SRC_ALPHA};ok=UpdateLayeredWindow(window,screen,&origin,&size,dc,&source,0,&blend,ULW_ALPHA)!=FALSE;}
  }
  SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);if(screen)ReleaseDC(nullptr,screen);
  if(ok){width=w;height=h;hot_x=hx;hot_y=hy;image=cursor;}return ok;
 }
 void hide(){if(window&&shown)ShowWindow(window,SW_HIDE);shown=false;}
public:
#ifdef S2UP_CURSOR194_FIXTURE
 bool fixture_render(HWND owner,HCURSOR cursor){return create(owner)&&render(cursor,POINT{0,0});}
 const std::vector<uint32_t>& fixture_pixels()const{return pixels;}
 HWND fixture_window()const{return window;}
#endif
 ~CursorRenderer(){hide();if(window)DestroyWindow(window);}
 void reset(){activity.reset();hide();}
 void tick(HWND game,bool allowed,bool touched,uint64_t now){
  static const bool proton=GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"wine_get_version")!=nullptr;
  DWORD pid{};RECT client{};POINT point{};
  const bool focused=game&&IsWindow(game)&&GetWindowThreadProcessId(game,&pid)&&pid==GetCurrentProcessId()&&GetForegroundWindow()==game&&GetClientRect(game,&client)&&GetCursorPos(&point);
  if(!activity.update(reinterpret_cast<uintptr_t>(game),allowed&&proton&&focused,touched,now)){hide();return;}
  POINT local=point;if(!ScreenToClient(game,&local)||local.x<0||local.y<0||local.x>=client.right||local.y>=client.bottom){hide();return;}
  CURSORINFO cursor{sizeof cursor};if(!GetCursorInfo(&cursor)||!(cursor.flags&CURSOR_SHOWING)||!cursor.hCursor){hide();return;}
  // Mirror only an image the game currently wants visible. It can still hide
  // the pointer for cinematics. No stock arrow or replacement artwork is used.
  if(window&&GetWindow(window,GW_OWNER)!=game){hide();DestroyWindow(window);window=nullptr;image=nullptr;}
  if(!create(game)){hide();return;}
  if(!shown||cursor.hCursor!=image||now>=redraw){if(!render(cursor.hCursor,point)){hide();return;}redraw=now+33;}
  if(!SetWindowPos(window,HWND_TOPMOST,point.x-int(hot_x),point.y-int(hot_y),int(width),int(height),SWP_NOACTIVATE|SWP_SHOWWINDOW)){hide();return;}shown=true;
  MSG message;while(PeekMessageW(&message,window,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
 }
};
}
