#define NOMINMAX
#define S2FP_LOCAL_ASSETS
#include "../../native/outputs/shogun2-ammunition83-preview/hud-src/hud_renderer.h"
#include "../../native/deck/wheel197.h"
#include <filesystem>
#include <cassert>
#include <iostream>
static void save(Gdiplus::Bitmap& image,const std::filesystem::path& path){UINT count{},size{};assert(Gdiplus::GetImageEncodersSize(&count,&size)==Gdiplus::Ok);std::vector<unsigned char> data(size);auto codecs=reinterpret_cast<Gdiplus::ImageCodecInfo*>(data.data());assert(Gdiplus::GetImageEncoders(count,size,codecs)==Gdiplus::Ok);for(unsigned i=0;i<count;++i)if(!wcscmp(codecs[i].MimeType,L"image/png")){assert(image.Save(path.c_str(),&codecs[i].Clsid)==Gdiplus::Ok);return;}assert(false);}
int wmain(int argc,wchar_t**argv){assert(argc==3);std::filesystem::path glyphs=argv[1],output=argv[2];std::filesystem::create_directory(output);
 ULONG_PTR token{};Gdiplus::GdiplusStartupInput startup;assert(Gdiplus::GdiplusStartup(&token,&startup,nullptr)==Gdiplus::Ok);
 {hud80::Assets assets;assert(assets.load(GetModuleHandleW(nullptr)));std::unique_ptr<Gdiplus::Bitmap> icons[4];unsigned count=0;
 const wchar_t* directions[]={L"up",L"down",L"left",L"right"};for(unsigned i=0;i<4;++i){auto file=glyphs/(std::wstring(L"shared_dpad_")+directions[i]+L"_md.png");icons[i]=std::make_unique<Gdiplus::Bitmap>(file.c_str());assert(icons[i]->GetLastStatus()==Gdiplus::Ok);}
 for(auto dims:{std::pair<int,int>{1280,800},{1280,720}})for(int chosen:{-1,0,1,2,3}){
  Gdiplus::Bitmap image(dims.first,dims.second,PixelFormat32bppPARGB);Gdiplus::Graphics g(&image);g.Clear(Gdiplus::Color(255,62,76,57));
  hud80::View v;v.unit=hud80::unit_art(L"Inf_Missile_Bow_Ashigaru");v.men=100;v.initial=120;v.mon=chosen%2?L"ui/clan_mons/data__shimazu.png":L"ui/clan_mons/data__oda.png";v.faction=chosen%2?L"Shimazu":L"Oda";v.commands_known=true;v.commands.fire_mode=1;v.has_ranged=v.unit_mode_known=true;
  assert(hud80::sprite(v.mon.c_str()));hud80::render(g,assets,float(dims.first),float(dims.second),v);
  const float scale=hudlayout108::scale(float(dims.first),float(dims.second));g.ScaleTransform(scale,scale);
  unsigned labels=0;deck197::draw(g,assets,dims.first/scale,dims.second/scale,v.mon.c_str(),chosen,1,chosen==0,[&](unsigned i,Gdiplus::RectF box){return g.DrawImage(icons[i].get(),box)==Gdiplus::Ok;},[&](const wchar_t* text,Gdiplus::RectF box,float size,Gdiplus::Color color){assert(box.X>=0&&box.Y>=0&&box.GetRight()<=dims.first/scale&&box.GetBottom()<=dims.second/scale);hud80::native_fonts.paint(g,text,box,size,color,true);++labels;});assert(labels>=6);
  g.ResetTransform();save(image,output/(std::to_wstring(dims.first)+L"x"+std::to_wstring(dims.second)+L"-"+std::to_wstring(chosen)+L".png"));++count;
 }
 // Center hole and exterior are transparent on a standalone wheel. Real mons
 // are loaded by key; unsupported clan art remains absent instead of substituted.
 Gdiplus::Bitmap image(1280,800,PixelFormat32bppPARGB);Gdiplus::Graphics g(&image);g.Clear(Gdiplus::Color(0,0,0,0));
 deck197::draw(g,assets,1280,800,L"",-1,-1,false,[](unsigned,Gdiplus::RectF){return false;},[](const wchar_t*,Gdiplus::RectF,float,Gdiplus::Color){});
 Gdiplus::Color c;assert(image.GetPixel(0,0,&c)==Gdiplus::Ok&&!c.GetA());assert(image.GetPixel(640,384,&c)==Gdiplus::Ok&&c.GetA());
 std::cout<<"PASS wheel197 render: "<<count<<" owned-game texture/font/mon and local Valve glyph previews; 720p/800p bounds\n";
 }Gdiplus::GdiplusShutdown(token);
}
