#define NOMINMAX
#include "../../native/common/card_pack217.h"
#include "../../native/common/image_fit184.h"
#include <cassert>
#include <iostream>
using namespace card217;
static unsigned cases;
#define C(x) do{if(!(x)){std::cerr<<"cards217 line "<<__LINE__<<"\n";ExitProcess(3);}++cases;}while(0)
static void number(Bytes& b,unsigned n,unsigned size=4){for(unsigned i=0;i<size;++i)b.push_back(static_cast<unsigned char>(n>>(8*i)));}
static void string(Bytes& b,const std::wstring& s){number(b,unsigned(s.size()),2);for(auto c:s)number(b,c,2);}
static Bytes table(const std::wstring& alias,const std::wstring& name=L"CustomUniform",const std::wstring& faction=L"mod_clan",const std::wstring& unit=L"mod_unit"){
 Bytes b;number(b,0xfffefdfc);number(b,3);b.push_back(1);number(b,1);string(b,name);b.push_back(faction.empty()?0:1);
 if(!faction.empty())string(b,faction);
 string(b,alias);string(b,unit);for(unsigned i=0;i<4;++i)b.push_back(0);return b;
}
static Bytes tga(unsigned w,unsigned h,unsigned char red){Bytes b(18);b[2]=2;b[12]=w&255;b[13]=w>>8;b[14]=h&255;b[15]=h>>8;b[16]=32;b[17]=32;for(unsigned n=0;n<w*h;++n){b.push_back(0);b.push_back(0);b.push_back(red);b.push_back(255);}return b;}
static void save(const fs::path& p,const Bytes& b){fs::create_directories(p.parent_path());std::ofstream out(p,std::ios::binary);out.write(reinterpret_cast<const char*>(b.data()),b.size());C(bool(out));}
static void pack(const fs::path& p,unsigned type,const std::map<std::string,Bytes>& entries,bool pfh2=false,bool stamp=false){Bytes idx;for(auto& q:entries){number(idx,unsigned(q.second.size()));if(stamp){number(idx,0);number(idx,0);}idx.insert(idx.end(),q.first.begin(),q.first.end());idx.push_back(0);}Bytes b(32);memcpy(b.data(),pfh2?"PFH2":"PFH3",4);unsigned flags=type|(stamp?0x40:0),count=unsigned(entries.size()),size=unsigned(idx.size());memcpy(b.data()+4,&flags,4);memcpy(b.data()+16,&count,4);memcpy(b.data()+20,&size,4);b.insert(b.end(),idx.begin(),idx.end());for(auto& q:entries)b.insert(b.end(),q.second.begin(),q.second.end());save(p,b);}
static Bytes dds(unsigned code,unsigned w=3,unsigned h=2){Bytes b(128);memcpy(b.data(),"DDS ",4);auto set=[&](unsigned at,unsigned v){memcpy(b.data()+at,&v,4);};set(4,124);set(12,h);set(16,w);set(76,32);set(80,4);set(84,code);unsigned block=code==827611204u?8:16;Bytes data(block);unsigned start=code==861165636u||code==894720068u?8:0;if(code==861165636u)for(unsigned i=0;i<8;++i)data[i]=255;if(code==894720068u)data[0]=255;data[start]=0;data[start+1]=248;b.insert(b.end(),data.begin(),data.end());return b;}
int wmain(int argc,wchar_t** argv){C(argc==3);auto dir=fs::path(argv[1]);C(!fs::exists(dir));fs::create_directories(dir/L"data");
 Gdiplus::GdiplusStartupInput startup;ULONG_PTR token;C(Gdiplus::GdiplusStartup(&token,&startup,nullptr)==Gdiplus::Ok);
 for(auto code:{827611204u,861165636u,894720068u}){auto p=decode(dds(code));C(p.w==3&&p.h==2&&p.pixels[2]==255&&p.pixels[3]==255);}
 auto rgba=dds(0);unsigned rgbflags=64,bits=32,red=0xff0000,green=0xff00,blue=255,alpha=0xff000000;memcpy(rgba.data()+80,&rgbflags,4);memcpy(rgba.data()+88,&bits,4);memcpy(rgba.data()+92,&red,4);memcpy(rgba.data()+96,&green,4);memcpy(rgba.data()+100,&blue,4);memcpy(rgba.data()+104,&alpha,4);rgba.resize(128+3*2*4,255);C(decode(rgba).pixels.size()==24);
 auto dx=dds(808540228u);dx.resize(148+16);unsigned format=98,dim=3,array=1;memcpy(dx.data()+128,&format,4);memcpy(dx.data()+132,&dim,4);memcpy(dx.data()+140,&array,4);dx[148]=64;C(decode(dx).pixels.size()==24);
 auto art=tga(8,16,77);auto pic=decode(art);C(pic.w==8&&pic.h==16&&pic.pixels[2]==77);auto fitted=imagefit184::within({0,0,92,136},8,16);C(fitted.w==68&&fitted.h==136);
 for(auto bad:{std::string("../evil.tga"),std::string("ui/../evil.tga"),std::string("c:/evil"),std::string("/evil"),std::string("a//b")}){bool refused=false;try{normal(bad);}catch(...){refused=true;}C(refused);}
 auto invalid=art;invalid[12]=255;invalid[13]=255;bool refused=false;try{decode(invalid);}catch(...){refused=true;}C(refused);
 for(size_t n:{size_t(0),size_t(17),size_t(19)}){refused=false;try{decode(Bytes(art.begin(),art.begin()+n));}catch(...){refused=true;}C(refused);}
 pack(dir/L"data/data.pack",0,{{"ui/units/info/shared.tga",tga(8,16,10)}});
 pack(dir/L"data/disabled.pack",3,{{"ui/units/info/shared.tga",tga(8,16,20)}});
 pack(dir/L"data/enabled.pack",3,{{"db/uniforms_tables/custom",table(L"special_card")},{"ui/units/info/shared.tga",tga(8,16,30)},{"ui/units/info/special_card.tga",tga(9,17,40)}},true,true);
 std::wstring select=L"// enabled cards only\nmod \"enabled.pack\";";Bytes sel{255,254};for(auto c:select)number(sel,c,2);save(dir/L"used_mods.txt",sel);
 Catalog catalog;catalog.configure(dir);auto a=catalog.card(L"unknown_mod_unit",L"clan",L"shared",L"");C(a&&a->pixels[2]==30);auto custom=catalog.card(L"mod_unit",L"mod_clan",L"shared",L"");C(custom&&custom->w==9&&custom->h==17&&custom->pixels[2]==40);C(catalog.card(L"new_mod_unit",L"x",L"absent",L"")==nullptr);C(catalog.card(L"new_mod_unit",L"x",L"../../evil",L"")==nullptr);
 for(unsigned i=0;i<100;++i)catalog.card(std::to_wstring(i),L"x",L"absent",L"");C(a->pixels[2]==30&&custom->pixels[2]==40); // retained View ownership survives cache eviction

 // Reproduce the launcher bytes seen on the installed game: no BOM and a
 // Workshop working directory outside Data. A stale directory must not
 // discard the later valid one, and disabled artwork must not win.
 auto workshop=dir/L"workshop/3820000000";
 pack(workshop/L"workshop_art.pack",3,{
  {"db/uniforms_tables/default",table(L"default_portrait",L"DefaultModUniform",L"",L"default_unit")},
  {"db/uniforms_tables/default_reused",table(L"special_card",L"DefaultModUniform",L"",L"other_unit")},
  {"db/uniforms_tables/icon",table(L"uniform_icon",L"ModIcon",L"mod_clan",L"icon_unit")},
  {"db/uniforms_tables/variant_a",table(L"wrong_variant",L"VariantA",L"mod_clan",L"ambiguous_unit")},
  {"db/uniforms_tables/variant_b",table(L"special_card",L"VariantB",L"mod_clan",L"ambiguous_unit")},
  {"ui/units/info/shared.tga",tga(8,16,66)},
  {"ui/units/info/special_card.tga",tga(9,17,67)},
  {"ui/units/info/default_portrait.tga",tga(9,17,68)},
  {"ui/units/info/wrong_variant.tga",tga(9,17,69)},
  {"ui/units/icons_square/square_only.tga",tga(12,12,70)},
  {"ui/units/icons/uniform_icon.tga",tga(8,16,71)}});
 std::wstring launcher=L"add_working_directory \"Z:relative-path\";\r\nadd_working_directory \""+workshop.generic_wstring()+L"\";\r\nmod \"enabled.pack\";\r\nmod \"workshop_art.pack\";\r\n";
 Bytes actual_selection;for(auto ch:launcher)number(actual_selection,ch,2);
 C(selection_text(actual_selection)==launcher);
 C(selection_text(Bytes{'m','o','d'})==L"mod");
 C(selection_text(Bytes{0xef,0xbb,0xbf,'m','o','d'})==L"mod");
 C(selection_text(Bytes{0xfe,0xff,0,'m',0,'o',0,'d'})==L"mod");
 auto truncated=actual_selection;truncated.pop_back();refused=false;try{selection_text(truncated);}catch(...){refused=true;}C(refused);
 save(dir/L"used_mods.txt",actual_selection);
 Catalog installed;installed.configure(dir);
 auto has=[&](const wchar_t* unit,const wchar_t* faction,const wchar_t* info,const wchar_t* icon,unsigned expected){
  auto item=installed.card(unit,faction,info,icon);return item&&item->pixels[2]==expected;
 };
 C(has(L"modded",L"mod_clan",L"data\\UI\\Units\\Info\\Shared.TGA",L"",66));
 C(has(L"default_unit",L"another_clan",L"",L"",68));
 assert(has(L"DEFAULT_UNIT",L"Another_Clan",L"",L"",68)&&has(L"OTHER_UNIT",L"Another_Clan",L"",L"",67));
 std::cout<<"PASS reused overhaul uniform names and case-insensitive unit identity\n";
 C(has(L"default_unit",L"another_clan",L"shared",L"",66));
 C(has(L"ambiguous_unit",L"mod_clan",L"shared",L"",66));
 C(has(L"modded",L"mod_clan",L"../invalid",L"square_only",70));
 C(has(L"icon_unit",L"mod_clan",L"",L"",71));
 save(dir/L"used_mods.txt",truncated);
 Catalog bad_selection;bad_selection.configure(dir);
 auto base_picture=bad_selection.card(L"modded",L"mod_clan",L"shared",L"");
 C(base_picture&&base_picture->pixels[2]==10);
 auto owned=index(fs::path(argv[2])/L"data/data.pack");auto u=owned.find("db/uniforms_tables/uniforms");C(u!=owned.end());auto rows=uniforms(payload(u->second,8*1024*1024));C(rows.size()==331);C(rows.front().unit==L"Art_European_Cannons"&&rows.front().card==L"Ashigaru_Art_European_Cannons");auto native=owned.find("ui/units/info/ashigaru_inf_matchlock_ashigaru.tga");C(native!=owned.end());auto actual=decode(payload(native->second,16*1024*1024));C(actual.w>0&&actual.h>0&&actual.h>actual.w);
 // Encode a private generated PNG and read it through the same in-memory path.
 {Gdiplus::Bitmap bitmap(8,16,32,PixelFormat32bppARGB,pic.pixels.data());UINT n=0,bytes=0;Gdiplus::GetImageEncodersSize(&n,&bytes);Bytes enc(bytes);auto e=reinterpret_cast<Gdiplus::ImageCodecInfo*>(enc.data());C(Gdiplus::GetImageEncoders(n,bytes,e)==Gdiplus::Ok);for(unsigned i=0;i<n;++i)if(!wcscmp(e[i].MimeType,L"image/png")){auto p=dir/L"own.png";C(bitmap.Save(p.c_str(),&e[i].Clsid)==Gdiplus::Ok);auto png=decode(file(p,100000));C(png.w==8&&png.h==16&&png.pixels[2]==77);}}
 std::cout<<"PASS cards217: "<<cases<<" native-alias/custom/replacement/priority/UTF16/cache/aspect/malformed cases and owned uniforms/TGA; no game artwork distributed or persistent card cache\n";Gdiplus::GdiplusShutdown(token);
}
