#define NOMINMAX
#include <windows.h>
#include <gdiplus.h>
#include <filesystem>
#include <cassert>
#include <iostream>
#include "../../native/deck/help_header186.h"
#include "../../native/common/local_assets159.h"
#include "../../native/outputs/shogun2-ammunition83-preview/hud-src/native_font.h"
static void save(Gdiplus::Bitmap& image,const std::filesystem::path& path){UINT count{},size{};assert(Gdiplus::GetImageEncodersSize(&count,&size)==Gdiplus::Ok);std::vector<unsigned char> data(size);auto codecs=reinterpret_cast<Gdiplus::ImageCodecInfo*>(data.data());assert(Gdiplus::GetImageEncoders(count,size,codecs)==Gdiplus::Ok);for(unsigned i=0;i<count;++i)if(!wcscmp(codecs[i].MimeType,L"image/png")){assert(image.Save(path.c_str(),&codecs[i].Clsid)==Gdiplus::Ok);return;}assert(false);}
int wmain(int argc,wchar_t**argv){assert(argc==4);std::filesystem::path output=argv[3];std::filesystem::create_directory(output);ULONG_PTR token{};Gdiplus::GdiplusStartupInput startup;assert(Gdiplus::GdiplusStartup(&token,&startup,nullptr)==Gdiplus::Ok);
 {assets159::Store store;assert(store.load(argv[1],1));hud80::NativeFonts fonts;assert(fonts.load_local([&](unsigned id){return store.get(id);}));
 Gdiplus::Bitmap source(argv[2]);assert(source.GetLastStatus()==Gdiplus::Ok);deck181::Glyph glyph;Gdiplus::Bitmap target(32,32,128,PixelFormat32bppPARGB,glyph.bgra);{Gdiplus::Graphics g(&target);g.Clear(Gdiplus::Color(0,0,0,0));assert(g.DrawImage(&source,0,0,32,32)==Gdiplus::Ok);}glyph.ready=1;
 for(bool campaign:{false,true})for(auto dims:{std::pair<int,int>{1280,800},{1280,720},{640,360},{1920,1080}}){
  float scale=deck186::header_scale(float(dims.first)),left=deck186::header_left(float(dims.first));int w=int(std::lround(deck186::header_width*scale)),h=int(std::lround(deck186::header_height*scale));assert(left>=100&&left+w<dims.first&&h<=26);
  Gdiplus::Bitmap header(w,h,PixelFormat32bppPARGB);{Gdiplus::Graphics g(&header);g.Clear(Gdiplus::Color(0,0,0,0));g.ScaleTransform(scale,scale);unsigned labels=0;
   assert(deck186::draw_header(g,[&](unsigned slot,deck181::Glyph& out){assert(slot==unsigned(deck174::Button::View));out=glyph;return true;},[&](const wchar_t* text,Gdiplus::RectF box,float size){assert(box.Y>=0&&box.Y+box.Height<=26&&size==16);assert(campaign?!wcsstr(text,L"HUD"):wcsstr(text,L"HUD")!=nullptr);auto shadow=box;shadow.X+=1;shadow.Y+=1;fonts.paint(g,text,shadow,size,Gdiplus::Color(220,25,21,18));fonts.paint(g,text,box,size,Gdiplus::Color(255,242,233,207));++labels;},campaign));assert(labels==1);
  }
  unsigned transparent=0,ink=0;for(int y=0;y<h;++y)for(int x=0;x<w;++x){Gdiplus::Color c;assert(header.GetPixel(x,y,&c)==Gdiplus::Ok);if(!c.GetA())++transparent;else ++ink;}assert(transparent>unsigned(w*h)/2&&ink>80);Gdiplus::Color corner;header.GetPixel(w-1,0,&corner);assert(!corner.GetA());
  Gdiplus::Bitmap image(dims.first,dims.second,PixelFormat32bppPARGB);{Gdiplus::Graphics g(&image);g.Clear(Gdiplus::Color(255,75,82,65));g.DrawImage(&header,int(left),deck186::header_top,w,h);}save(image,output/((campaign?L"campaign-":L"battle-")+std::to_wstring(dims.first)+L"x"+std::to_wstring(dims.second)+L".png"));
  if(dims.first==1280&&dims.second==800)save(header,output/(campaign?L"campaign-detail.png":L"battle-detail.png"));
 }
 }Gdiplus::GdiplusShutdown(token);std::cout<<"PASS header193: eight real-font/Valve-glyph renders; transparent background and shared left anchor; campaign binding accurate\n";
}
