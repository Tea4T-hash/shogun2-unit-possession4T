#pragma once
#include "channel181.h"
#include "steam_input174.h"
#include "proton_paths174.h"
#include <objidl.h>
#include <gdiplus.h>
#include <memory>
#include "wheel197.h"
namespace deck181 {
// Valve supplies the artwork from the installed Steam client. Decode once to
// bounded RAM; neither glyph files nor per-frame logs are generated here.
inline bool copy_glyph(Channel& channel,deck174::SteamBackend& backend,unsigned slot,int origin){
 const char* raw=backend.glyph_file(origin);if(!raw||!raw[0]||strnlen_s(raw,32769)>32768)return false;
 int count=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,raw,-1,nullptr,0);if(count<=0||count>32768)return false;
 std::wstring path(size_t(count),L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,raw,-1,path.data(),count);path.resize(size_t(count-1));
 if(path[0]==L'/')path=L"Z:"+path;for(auto& c:path)if(c==L'/')c=L'\\';
 deck174::paths::read_file(path);if(std::filesystem::file_size(path)>4*1024*1024)return false;
 static ULONG_PTR token{};if(!token){Gdiplus::GdiplusStartupInput input;if(Gdiplus::GdiplusStartup(&token,&input,nullptr)!=Gdiplus::Ok)return false;}
 Gdiplus::Bitmap source(path.c_str());if(source.GetLastStatus()!=Gdiplus::Ok||!source.GetWidth()||!source.GetHeight()||source.GetWidth()>256||source.GetHeight()>256)return false;
 Glyph g;Gdiplus::Bitmap target(glyph_side,glyph_side,glyph_side*4,PixelFormat32bppPARGB,g.bgra);
 {Gdiplus::Graphics paint(&target);paint.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);paint.Clear(Gdiplus::Color(0,0,0,0));paint.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);if(paint.DrawImage(&source,0,0,int(glyph_side),int(glyph_side))!=Gdiplus::Ok)return false;}
 g.ready=1;channel.put_glyph(slot,g);return true;
}
struct GlyphPublisher {
 unsigned attempts{};uint64_t retry{};bool complete{};
 void poll(Channel& channel,deck174::SteamBackend& backend){
  if(complete||attempts>=3||!channel.shared||backend.state()!=deck174::BackendState::Ready||GetTickCount64()<retry)return;
  ++attempts;retry=GetTickCount64()+2000;bool good=true;
  for(unsigned i=0;i<deck174::button_count;++i)try{good=copy_glyph(channel,backend,i,deck174::steam006::digital[i].origin)&&good;}catch(...){good=false;}
  const unsigned analogs[]={4,5,0,1,2,3};
  for(unsigned i=0;i<6;++i)try{good=copy_glyph(channel,backend,24+i,deck174::steam006::analog_origin[analogs[i]])&&good;}catch(...){good=false;}
  complete=good;
 }
};
inline DWORD& presentation_pid(){static DWORD value{};return value;}
inline bool controller_present(){Sample s;return read_sample(presentation_pid()?presentation_pid():GetCurrentProcessId(),s)&&s.context==unsigned(deck174::Context::Possession)&&s.lease;}
inline int key_glyph(const wchar_t* text){
 using deck174::Button;if(!text)return -1;
 struct Pair{const wchar_t* key;int glyph;};
 const Pair map[]={{L"Q",int(Button::L1)},{L"E",int(Button::R4)},{L"Z",int(Button::R1)},{L"C",int(Button::R5)},
  {L"R",int(Button::R1)},{L"F10",int(Button::Y)},{L"P",int(Button::R3)},{L"1",int(Button::X)},
  {L"F",int(Button::A)},{L"T",int(Button::Up)},{L"G",int(Button::Down)},{L"M",int(Button::B)},
  {L"SHIFT",int(Button::L3)},{L"ORBIT",int(Button::L4)},{L"W",26},{L"S",26},{L"RT",25},{L"LT",24}};
 for(auto p:map)if(wcscmp(text,p.key)==0)return p.glyph;return -1;
}
inline const wchar_t* key_label(const wchar_t* label){
 if(!controller_present())return label;
 const wchar_t* names[]={L"A",L"B",L"X",L"Y",L"LB",L"RB",L"L3",L"R3",L"DP-U",L"DP-D",L"DP-L",L"DP-R",L"View",L"Menu",L"L4",L"L5",L"R4",L"R5",L"LP",L"RP",L"LP",L"RP",L"LS",L"RS",L"LT",L"RT",L"LS",L"RS",L"LP",L"RP"};
 if(wcscmp(label,L"Z")==0)return L"RB + DP-L";if(wcscmp(label,L"R")==0)return L"RB + DP-R";
 int i=key_glyph(label);return i>=0&&i<int(glyph_count)?names[i]:label;
}
inline bool draw_key(Gdiplus::Graphics& paint,const wchar_t* label,Gdiplus::RectF bounds){
 if(!controller_present())return false;int index=key_glyph(label);if(index<0)return false;
 if(wcscmp(label,L"Z")==0||wcscmp(label,L"R")==0){
  const unsigned second=unsigned(wcscmp(label,L"Z")==0?deck174::Button::Left:deck174::Button::Right);Glyph a,b;
  if(!reader().glyph(unsigned(deck174::Button::R1),a)||!reader().glyph(second,b))return false;
  Gdiplus::Bitmap ia(glyph_side,glyph_side,glyph_side*4,PixelFormat32bppPARGB,a.bgra),ib(glyph_side,glyph_side,glyph_side*4,PixelFormat32bppPARGB,b.bgra);
  float side=std::min(bounds.Width*.5f,bounds.Height);return paint.DrawImage(&ia,Gdiplus::RectF(bounds.X,bounds.Y,side,side))==Gdiplus::Ok&&paint.DrawImage(&ib,Gdiplus::RectF(bounds.X+side,bounds.Y,side,side))==Gdiplus::Ok;
 }
 Glyph glyph;if(!reader().glyph(unsigned(index),glyph))return false;
 Gdiplus::Bitmap image(glyph_side,glyph_side,glyph_side*4,PixelFormat32bppPARGB,glyph.bgra);
 float side=std::min(bounds.Width,bounds.Height);Gdiplus::RectF r(bounds.X+(bounds.Width-side)*.5f,bounds.Y+(bounds.Height-side)*.5f,side,side);
 return paint.DrawImage(&image,r)==Gdiplus::Ok;
}
template<class Assets,class Text>inline void draw_command_wheel183(Gdiplus::Graphics& g,const Assets& assets,float width,float height,const wchar_t* mon,int fire_mode,bool following,Text text){
#ifndef HUD80_QTE_ONLY
 Sample sample;if(!read_sample(presentation_pid()?presentation_pid():GetCurrentProcessId(),sample)||sample.context!=unsigned(deck174::Context::Possession)||!sample.lease)return;
 auto held=[&](unsigned k){return(sample.keys[k/32]&(uint32_t(1)<<(k%32)))!=0;};if(!held(deck183::WheelHeld))return;
 const unsigned choices[]={deck183::WheelUp,deck183::WheelDown,deck183::WheelLeft,deck183::WheelRight};int selected=-1;
 for(unsigned i=0;i<4;++i)if(held(choices[i]))selected=int(i);
 deck197::draw(g,assets,width,height,mon,selected,fire_mode,following,[&](unsigned i,Gdiplus::RectF box){
  Glyph value;if(!reader().glyph(unsigned(deck174::Button::Up)+i,value))return false;
  Gdiplus::Bitmap image(glyph_side,glyph_side,glyph_side*4,PixelFormat32bppPARGB,value.bgra);return g.DrawImage(&image,box)==Gdiplus::Ok;
 },text);
#else
 (void)g;(void)assets;(void)width;(void)height;(void)mon;(void)fire_mode;(void)following;(void)text;
#endif
}
} // deck181
