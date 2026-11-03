// Offline production legend renderer with Valve's already installed glyph art.
// Fake ABI definitions are compiled only; no Steam, game, input or worker starts.
#define S2UP165_LUA_FIXTURE 1
#define S2UP_FRONTEND181_STANDALONE 1
#define main unused_input184
#include "verify174.cpp"
#undef main
static std::wstring own165(){return L"unused-offline-fixture";}
static bool battle_ready(uintptr_t){return false;}
static bool caller165(void*){return false;}
#include "../../native/deck/frontend181.h"
#include <filesystem>
static void save184(Gdiplus::Bitmap& image,const std::filesystem::path& path){
 UINT count{},size{};assert(Gdiplus::GetImageEncodersSize(&count,&size)==Gdiplus::Ok);std::vector<unsigned char> bytes(size);auto codecs=reinterpret_cast<Gdiplus::ImageCodecInfo*>(bytes.data());assert(Gdiplus::GetImageEncoders(count,size,codecs)==Gdiplus::Ok);
 for(unsigned i=0;i<count;++i)if(!wcscmp(codecs[i].MimeType,L"image/png")){assert(image.Save(path.c_str(),&codecs[i].Clsid)==Gdiplus::Ok);return;}assert(false);
}
int wmain(int argc,wchar_t** argv){
 assert(argc==4);std::filesystem::path glyphs=argv[1],resources=argv[2],out=argv[3];assert(!std::filesystem::exists(out));std::filesystem::create_directories(out);
 ULONG_PTR gdi{};Gdiplus::GdiplusStartupInput input;assert(Gdiplus::GdiplusStartup(&gdi,&input,nullptr)==Gdiplus::Ok);
 {deck181::Channel channel;assert(channel.open(GetCurrentProcessId(),true));
 const wchar_t* names[]={L"shared_color_button_a_md.png",L"shared_color_button_b_md.png",L"shared_color_button_x_md.png",L"shared_color_button_y_md.png",L"sd_l1_md.png",L"sd_r1_md.png",L"shared_lstick_click_md.png",L"shared_rstick_click_md.png",L"shared_dpad_up_md.png",L"shared_dpad_down_md.png",L"shared_dpad_left_md.png",L"shared_dpad_right_md.png",L"sd_button_view_md.png",L"sd_button_menu_md.png",L"sd_l4_md.png",L"sd_l5_md.png",L"sd_r4_md.png",L"sd_r5_md.png",L"sd_ltrackpad_md.png",L"sd_rtrackpad_md.png",L"sd_ltrackpad_click_md.png",L"sd_rtrackpad_click_md.png",L"shared_lstick_touch_md.png",L"shared_rstick_touch_md.png",L"sd_l2_md.png",L"sd_r2_md.png",L"shared_lstick_md.png",L"shared_rstick_md.png",L"sd_ltrackpad_md.png",L"sd_rtrackpad_md.png"};
 for(unsigned i=0;i<30;++i){auto path=glyphs/names[i];Gdiplus::Bitmap source(path.c_str());assert(source.GetLastStatus()==Gdiplus::Ok);deck181::Glyph value;Gdiplus::Bitmap dest(32,32,128,PixelFormat32bppPARGB,value.bgra);{Gdiplus::Graphics g(&dest);g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);assert(g.DrawImage(&source,0,0,32,32)==Gdiplus::Ok);}value.ready=1;channel.put_glyph(i,value);}
 deck184::publish_resources(resources.wstring());assert(deck184::write_campaign_legend(channel,out.wstring()));assert(deck184::write_campaign_legend(channel,out.wstring()));assert(std::filesystem::file_size(out/L"campaign-legend184.tga")==18+1184*92*4);
 assets159::Store store;assert(store.load((resources/L"hud-assets159.s2fp").wstring(),1));hud80::NativeFonts fonts;assert(fonts.load_local([&](unsigned id){return store.get(id);}));
 for(auto context:{deck174::Context::Campaign,deck174::Context::RTS,deck174::Context::Possession}){Gdiplus::Bitmap image(1184,92,PixelFormat32bppPARGB);{Gdiplus::Graphics g(&image);g.Clear(Gdiplus::Color(0,0,0,0));assert(deck184::draw_legend(g,context,[&](unsigned i,deck181::Glyph& value){return channel.glyph(i,value);},[&](const wchar_t* text,Gdiplus::RectF bounds,float size){fonts.paint(g,text,bounds,size,Gdiplus::Color(255,242,233,207));}));}save184(image,out/(context==deck174::Context::Campaign?L"campaign.png":context==deck174::Context::RTS?L"battle.png":L"possession.png"));}
 }
 Gdiplus::GdiplusShutdown(gdi);std::cout<<"PASS production legend184: three native-font/Valve-glyph strips; bounded repeat TGA reuse; no game or Steam execution\n";
}
