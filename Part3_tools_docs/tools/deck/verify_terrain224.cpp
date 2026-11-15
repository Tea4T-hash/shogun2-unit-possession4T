// Actual game point-request constructor, terrain bounds clamp and search loop.
// Geometry/admission and point materialization are explicit owned-image providers.
#define wmain earlier_ui224
#include "verify_native_ui185.cpp"
#undef wmain
#include "../../native/deck/terrain224.h"
#include "terrain_fixture224.h"
#include <string>
int wmain(int argc,wchar_t** argv){assert(argc==3||argc==4);SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);unsigned cases=0;
 if(argc==4){auto image=LoadLibraryExW(argv[2],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto b=reinterpret_cast<uintptr_t>(image);
  std::vector<unsigned char> e(32),co(0x100),m(16);auto cp=reinterpret_cast<uintptr_t>(co.data()),ep=reinterpret_cast<uintptr_t>(e.data());memcpy(e.data()+8,&cp,4);memcpy(m.data(),&ep,4);fixture224::Terrain world(b,cp);
  fixture224::legacy=true;fixture224::legacy_event=OpenEventW(EVENT_MODIFY_STATE,FALSE,argv[3]);assert(fixture224::legacy_event);
  // Old float[2] call with controlled zero-valued neighbouring stack bytes.
  // The native search reads them as stride/radius/bound and makes no progress.
  std::array<unsigned char,64> padded{};float xz[]{10,20};memcpy(padded.data(),xz,8);terrain224::Point result{};
  reinterpret_cast<void*(__thiscall*)(void*,void*,const void*,unsigned)>(b+0x37b430)(m.data(),&result,padded.data(),1);return 9;
 }
 for(int f=1;f<argc;++f){auto image=LoadLibraryExW(argv[f],nullptr,DONT_RESOLVE_DLL_REFERENCES);assert(image);auto b=reinterpret_cast<uintptr_t>(image);native214::admitted_image=0;assert(native214::admit(b,8));{
  std::vector<unsigned char> environment(32),common(0x100),manager(16);auto e=reinterpret_cast<uintptr_t>(environment.data()),co=reinterpret_cast<uintptr_t>(common.data()),m=reinterpret_cast<uintptr_t>(manager.data());memcpy(environment.data()+8,&co,4);memcpy(manager.data(),&e,4);
  fixture224::Terrain world(b,co);
  for(auto pos:std::vector<std::array<float,2>>{{10,20},{-10,-20},{9999,9999},{-9999,-9999}}){
   for(unsigned delayed:{0u,1u,3u,32u}){fixture224::probes=0;fixture224::allow_after=delayed;terrain224::Point result{{-77,-77,-77},0,0};auto before=result;
    bool ok=terrain224::locate(b,m,pos.data(),result);assert(fixture224::probes>0&&fixture224::probes<=9);
    if(delayed==32)assert(!ok&&!memcmp(&result,&before,sizeof result));
    else assert(ok&&fixture224::probes==delayed+1&&std::fabs(result.p[0])<=1004&&std::fabs(result.p[2])<=1004);++cases;
   }
  }
  // Fully rejected search is bounded and never exports its native fallback point.
  fixture224::outside=true;fixture224::probes=0;float valid[]{10,20};terrain224::Point result{};assert(!terrain224::locate(b,m,valid,result)&&fixture224::probes<=9);++cases;
  for(float bad:{INFINITY,-INFINITY,NAN,100001.f}){fixture224::probes=0;float xz[]{bad,0};assert(!terrain224::locate(b,m,xz,result)&&!fixture224::probes);++cases;}
  assert(!terrain224::locate(b,0,valid,result));++cases;
 }assert(FreeLibrary(image));
  // Reproduce the old zero-stride hang only in our newly-created inert child;
  // wait for512 actual native search probes before verifying and terminating it.
  wchar_t self[MAX_PATH]{};assert(GetModuleFileNameW(nullptr,self,MAX_PATH));
  std::wstring name=L"Local\\S2UP224-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(f);HANDLE ready=CreateEventW(nullptr,TRUE,FALSE,name.c_str());assert(ready);
  std::wstring line=L"\""+std::wstring(self)+L"\" --legacy-hang \""+argv[f]+L"\" \""+name+L"\"";
  STARTUPINFOW si{sizeof(si)};si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_HIDE;PROCESS_INFORMATION child{};
  assert(CreateProcessW(self,line.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&child));
  DWORD signaled=WaitForSingleObject(ready,5000),state=WaitForSingleObject(child.hProcess,100);
  BOOL stopped=TerminateProcess(child.hProcess,0);WaitForSingleObject(child.hProcess,5000);CloseHandle(child.hThread);CloseHandle(child.hProcess);CloseHandle(ready);
  assert(signaled==WAIT_OBJECT_0&&state==WAIT_TIMEOUT&&stopped);cases+=2;
 }
 std::cout<<"PASS terrain224: "<<cases<<" original constructor/clamp/bounded-search ABI cases including isolated legacy zero-stride hang; admission/point materialization are providers; no live game\n";
}
