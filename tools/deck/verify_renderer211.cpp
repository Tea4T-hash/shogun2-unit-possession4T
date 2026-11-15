// Original engine UI math with an unrelated renderer frame owner. No game,
// graphics device, Steam, third-party DllMain or third-party hook code executes.
#define wmain renderer211_previous_main
#include "verify_native_ui185.cpp"
#undef wmain
#include <fstream>
#include <iterator>
static unsigned frames211{};
static bool __fastcall unused_frame211(void*,void*){++frames211;return true;}
static std::string hash211(const void* data,size_t bytes){BCRYPT_ALG_HANDLE h{};assert(BCryptOpenAlgorithmProvider(&h,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0);unsigned char out[32]{};assert(BCryptHash(h,nullptr,0,(PUCHAR)data,ULONG(bytes),out,32)>=0);BCryptCloseAlgorithmProvider(h,0);const char* digits="0123456789abcdef";std::string text;for(auto v:out){text+=digits[v>>4];text+=digits[v&15];}return text;}
static bool legacy211(uintptr_t base){std::array<unsigned char,784> data{};memcpy(data.data(),(void*)(base+0x1754574),data.size());for(unsigned i=0;i<196;++i){uint32_t v{};memcpy(&v,data.data()+i*4,4);v-=uint32_t(base-0x10000000u);memcpy(data.data()+i*4,&v,4);}return hash211(data.data(),data.size())=="a34efd697ed267c3e753c9cad01041137dd0f96252f2bda4e8e9d14e057b8e13";}
int wmain(int argc,wchar_t** argv){assert(argc==3||argc==4);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);HMODULE loader{};std::vector<uintptr_t> owners{reinterpret_cast<uintptr_t>(&unused_frame211)};
 if(argc==4){std::ifstream in(argv[3],std::ios::binary);assert(in);std::vector<char> bytes{std::istreambuf_iterator<char>(in),{}};assert(hash211(bytes.data(),bytes.size())=="12b3b76114fdb4268673accf4b45fc0066467de072a10ef56957ffe43d8d9e16");loader=LoadLibraryExW(argv[3],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(loader);auto b=reinterpret_cast<uintptr_t>(loader);uint32_t slot{},hook{};memcpy(&slot,(void*)(b+0x83bd),4);memcpy(&hook,(void*)(b+0x8419),4);assert(slot==0x17547f8&&*(unsigned short*)(b+0x8417)==0x06c7&&hook==b+0x7230);owners.push_back(hook);}
 unsigned image_cases=0,owner_cases=0,refusals=0,points=0;
 for(int input=1;input<=2;++input){++image_cases;for(auto owner:owners)for(bool late:{false,true}){
  HMODULE image=LoadLibraryExW(argv[input],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto base=reinterpret_cast<uintptr_t>(image);assert(legacy211(base)&&deck185::native_admitted(base));
  if(late)assert(deck185::install_ui185(base));
  {Patch185 foreign(base+0x17547f8,&owner,4);assert(!legacy211(base));
   if(!late){assert(deck185::native_admitted(base));
   // Every previous protected byte except that unrelated four-byte slot stays
   // in the contract. Reject edits to all code spans and all195 other entries.
   for(const auto& span:native185::spans){unsigned char changed=*(unsigned char*)(base+span.rva)^1;Patch185 bad(base+span.rva,&changed,1);assert(!deck185::native_admitted(base));++refusals;}
   for(unsigned i=0;i<196;++i){if(i==161)continue;auto at=base+0x1754574+i*4;uint32_t changed=*(uint32_t*)at^1;Patch185 bad(at,&changed,4);assert(!deck185::native_admitted(base));++refusals;}
   assert(deck185::native_admitted(base));}
   assert(deck185::install_ui185(base));assert(*(uintptr_t*)(base+0x17547f8)==owner);
   {auto jump=jump185(base+0x13f7b40,reinterpret_cast<void*>(&physical185));Patch185 physical(base+0x13f7b40,jump.data(),5);uintptr_t flags=reinterpret_cast<uintptr_t>(&mode185),display=reinterpret_cast<uintptr_t>(&display_info185);Patch185 mode(base+0x1754574+0x248,&flags,4),device(base+0x1754574+0x30c,&display,4);
    uintptr_t table=base+0x1754574;std::array<unsigned char,32> renderer{};memcpy(renderer.data(),&table,4);std::array<unsigned char,0x328> root{};uintptr_t render=reinterpret_cast<uintptr_t>(renderer.data());memcpy(root.data()+8,&render,4);uintptr_t context=reinterpret_cast<uintptr_t>(root.data());Patch185 current(base+0x1bbe364,&context,4);
    using Transform=Point185*(__thiscall*)(void*,Point185*,float,float);auto inverse=reinterpret_cast<Transform>(base+0x14121a0),project=reinterpret_cast<Transform>(base+0x13e35d0);
    for(auto res:{std::pair<unsigned,unsigned>{1280,800},{1280,720},{1920,1080},{2560,1440}}){width185=res.first;height185=res.second;auto size=deck185::active_ui185();assert(size.width==unsigned(std::lround(expected_logical185(width185,height185,true)/1.25))&&size.height==unsigned(std::lround(expected_logical185(width185,height185,false)/1.25)));unsigned maximum[2]{};deck185::ui_maximum185(renderer.data(),nullptr,maximum);assert(maximum[0]==size.width&&maximum[1]==size.height);
     for(unsigned y=0;y<=8;++y)for(unsigned x=0;x<=16;++x){Point185 logical{},ndc{};inverse(renderer.data(),&logical,float(x)/16,float(y)/8);project(renderer.data(),&ndc,logical.x,logical.y);assert(fabs(ndc.x-2.f*x/16)<.00001&&fabs(ndc.y+2.f*y/8)<.00001);++points;}
    }
    assert(frames211==0&&*(uintptr_t*)(base+0x17547f8)==owner);
   }
   for(auto at:deck185::native_ui185.targets){assert(MH_DisableHook(at)==MH_OK&&MH_RemoveHook(at)==MH_OK);}deck185::native_ui185={};assert(deck185::native_admitted(base)&&!legacy211(base)&&*(uintptr_t*)(base+0x17547f8)==owner);++owner_cases;
  }
  assert(deck185::native_admitted(base)&&legacy211(base));assert(FreeLibrary(image));
 }}
 if(loader)assert(FreeLibrary(loader));
 std::cout<<"PASS renderer211: "<<image_cases<<" engine images; "<<owner_cases<<" frame owners; "<<refusals<<" critical mutations refused; "<<points<<" original projection/inverse points; foreign frame pointer preserved, zero calls, exact restoration.\n";
 if(argc==4)std::cout<<"PASS renderer211 exact loader0.5.2: file/hash/installer slot/hook address checked; mapped only, no foreign code or DllMain executed.\n";
 return 0;
}
