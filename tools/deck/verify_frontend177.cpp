// Safe offline fixture: fake Valve interface, generated test PNG, no game host,
// no actual Steam calls, UI hooks, process memory, keyboard or mouse injection.
#define main prior_input_fixture_main177
#include "verify174.cpp"
#undef main
#include <filesystem>
#include <fstream>
static std::wstring fixture_path177;
static std::wstring own165(){return fixture_path177;}
static bool battle_ready(uintptr_t){return false;}
static bool caller165(void* state){return state&&*static_cast<unsigned*>(state)==1;}
#include "../../native/deck/frontend177.h"
struct GlyphSteam177:FakeSteam{
    std::string path;
    const char* s23(int,int size,uint32_t flags)override{assert(size==0&&flags==2);return path.c_str();}
};
static std::vector<unsigned char> read177(const std::filesystem::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int wmain(int argc,wchar_t** argv){
    assert(argc==2);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);_set_error_mode(_OUT_TO_STDERR);
    fixture_path177=argv[1];assert(!std::filesystem::exists(fixture_path177));std::filesystem::create_directories(fixture_path177);
    unsigned one=1,two=2;
    TEST("menu exports retain arity guard",{assert(s2up_deck_prepare177(&two)==0&&s2up_deck_poll177(nullptr)==0&&s2up_deck_stop177(&two)==0);});
    TEST("ordinary Windows cannot start private Proton frontend",{assert(!frontend177::prepare()&&!frontend177::prepared);});
    auto directory=std::filesystem::path(fixture_path177);auto cache=directory/L"cache";std::filesystem::create_directory(cache);
    TEST("exclusive exact-file reuse",{auto p=cache/L"test.bin";frontend177::exact_file(p.wstring(),"ABC",3);frontend177::exact_file(p.wstring(),"ABC",3);assert(read177(p)==std::vector<unsigned char>({'A','B','C'}));});
    TEST("foreign file preserved",{auto p=cache/L"test.bin";bool refused=false;try{frontend177::exact_file(p.wstring(),"ABD",3);}catch(...){refused=true;}assert(refused&&read177(p)==std::vector<unsigned char>({'A','B','C'}));});
    ULONG_PTR gdi{};Gdiplus::GdiplusStartupInput input;assert(Gdiplus::GdiplusStartup(&gdi,&input,nullptr)==Gdiplus::Ok);
    UINT count{},size{};assert(Gdiplus::GetImageEncodersSize(&count,&size)==Gdiplus::Ok);std::vector<unsigned char> storage(size);
    auto encoders=reinterpret_cast<Gdiplus::ImageCodecInfo*>(storage.data());assert(Gdiplus::GetImageEncoders(count,size,encoders)==Gdiplus::Ok);
    CLSID png{};bool found=false;for(unsigned i=0;i<count;++i)if(!wcscmp(encoders[i].MimeType,L"image/png")){png=encoders[i].Clsid;found=true;}assert(found);
    auto source=directory/L"fixture.png";
    {Gdiplus::Bitmap image(2,2,PixelFormat32bppARGB);assert(image.SetPixel(0,0,Gdiplus::Color(255,10,20,30))==Gdiplus::Ok);assert(image.Save(source.c_str(),&png)==Gdiplus::Ok);}
    GlyphSteam177 steam;steam.path=source.u8string();assert(frontend177::backend.initialize({&steam},"fixture",true));
    Frame f;assert(frontend177::backend.sample(1000000,true,f));frontend177::directory=cache.wstring();frontend177::owner=GetCurrentThreadId();
    TEST("actual PNG decode emits bounded alpha TGA",{frontend177::glyph(333,L"glyph.tga");auto b=read177(cache/L"glyph.tga");assert(b.size()==34&&b[2]==2&&b[12]==2&&b[14]==2&&b[16]==32&&b[17]==0x28&&b[18]==30&&b[19]==20&&b[20]==10&&b[21]==255);});
    TEST("glyph reuse creates no growing cache history",{auto before=read177(cache/L"glyph.tga");for(int i=0;i<25;++i)frontend177::glyph(333,L"glyph.tga");assert(read177(cache/L"glyph.tga")==before&&std::distance(std::filesystem::directory_iterator(cache),std::filesystem::directory_iterator{})==2);});
    TEST("foreign glyph preserved",{auto p=cache/L"foreign.tga";frontend177::exact_file(p.wstring(),"X",1);bool refused=false;try{frontend177::glyph(333,L"foreign.tga");}catch(...){refused=true;}assert(refused&&read177(p)==std::vector<unsigned char>({'X'}));});
    TEST("oversized glyph refused before writing",{auto big=directory/L"large.png";{Gdiplus::Bitmap image(257,2,PixelFormat32bppARGB);assert(image.Save(big.c_str(),&png)==Gdiplus::Ok);}steam.path=big.u8string();bool refused=false;try{frontend177::glyph(333,L"large.tga");}catch(...){refused=true;}assert(refused&&!std::filesystem::exists(cache/L"large.tga"));});
    TEST("wrong-thread stop cannot mutate owned input",{frontend177::prepared=true;frontend177::confirm=true;frontend177::owner=GetCurrentThreadId()+1;assert(!frontend177::stop()&&frontend177::prepared&&frontend177::confirm&&!steam.closed);frontend177::owner=GetCurrentThreadId();});
    TEST("bidirectional ownership exposes peers and rejects duplicate roles",{deck177::Lease front,battle,duplicate;assert(front.acquire(deck177::Owner::Frontend)&&deck177::busy(deck177::Owner::Frontend));assert(!duplicate.acquire(deck177::Owner::Frontend));assert(battle.acquire(deck177::Owner::Battle)&&deck177::busy(deck177::Owner::Battle));battle.release();assert(!deck177::busy(deck177::Owner::Battle));front.release();assert(!deck177::busy(deck177::Owner::Frontend));});
    TEST("handoff closes owned input and named lease",{assert(frontend177::lease.acquire(deck177::Owner::Frontend));assert(s2up_deck_ready177(&one)==1);assert(frontend177::stop()&&!frontend177::prepared&&steam.closed&&s2up_deck_ready177(&one)==0);assert(!deck177::busy(deck177::Owner::Frontend));});
    Gdiplus::GdiplusShutdown(gdi);
    std::cout<<"PASS "<<passed<<" frontend177 native lifecycle/glyph checks; actual Deck execution pending\n";
}
