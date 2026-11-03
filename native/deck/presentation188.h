#pragma once
#include <windows.h>
#include <cstdint>
#include <new>
#include "input174.h"
namespace deck188 {
// One fixed, numeric presentation record per process. No histories or files.
struct Presentation {unsigned magic{0x38505244},version{1},bytes{sizeof(Presentation)},pid{};volatile LONG sequence{},writer{};unsigned context{},help{},page{},hud_stage{};uint64_t tick{};};
inline Presentation last_presentation188;
class PresentationChannel {
 HANDLE mapping{};Presentation* shared{};
public:
 ~PresentationChannel(){if(shared)UnmapViewOfFile(shared);if(mapping)CloseHandle(mapping);}
 bool open(){if(shared)return true;wchar_t name[96];swprintf_s(name,L"Local\\S2UP-DeckPresentation188-%lu",GetCurrentProcessId());mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Presentation),name);if(!mapping)return false;DWORD error=GetLastError();shared=static_cast<Presentation*>(MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(Presentation)));if(!shared){CloseHandle(mapping);mapping=nullptr;return false;}if(error!=ERROR_ALREADY_EXISTS){new(shared)Presentation;shared->pid=GetCurrentProcessId();}bool valid=shared->magic==0x38505244&&shared->version==1&&shared->bytes==sizeof(Presentation)&&shared->pid==GetCurrentProcessId();if(!valid){UnmapViewOfFile(shared);CloseHandle(mapping);shared=nullptr;mapping=nullptr;}return valid;}
 void publish(unsigned context,bool help,unsigned page,unsigned hud_stage){if(!open()||InterlockedCompareExchange(&shared->writer,1,0))return;InterlockedIncrement(&shared->sequence);shared->context=context;shared->help=help;shared->page=page;shared->hud_stage=hud_stage;shared->tick=GetTickCount64();MemoryBarrier();InterlockedIncrement(&shared->sequence);
 InterlockedIncrement(&last_presentation188.sequence);last_presentation188.pid=GetCurrentProcessId();last_presentation188.context=context;last_presentation188.help=help;last_presentation188.page=page;last_presentation188.hud_stage=hud_stage;last_presentation188.tick=shared->tick;MemoryBarrier();InterlockedIncrement(&last_presentation188.sequence);
 InterlockedExchange(&shared->writer,0);}
 bool read(Presentation& value){if(!open())return false;for(unsigned n=0;n<3;++n){LONG sequence=shared->sequence;if(sequence&1)continue;MemoryBarrier();value=*shared;MemoryBarrier();if(sequence==shared->sequence)return value.magic==0x38505244&&value.version==1&&value.bytes==sizeof(value)&&value.pid==GetCurrentProcessId()&&value.help<=1&&value.page<2&&value.hud_stage<3&&GetTickCount64()>=value.tick&&GetTickCount64()-value.tick<=500;}return false;}
};
inline PresentationChannel presentation188;
inline bool read_presentation188(DWORD pid,Presentation& value){
 struct Reader {HANDLE handle{};const Presentation* shared{};DWORD bound{};~Reader(){if(shared)UnmapViewOfFile(shared);if(handle)CloseHandle(handle);}};
 static thread_local Reader owner;auto& handle=owner.handle;auto& shared=owner.shared;auto& bound=owner.bound;
 if(bound!=pid){if(shared)UnmapViewOfFile(shared);if(handle)CloseHandle(handle);shared=nullptr;handle=nullptr;bound=pid;}
 if(!pid)return false;if(!shared){wchar_t name[96];swprintf_s(name,L"Local\\S2UP-DeckPresentation188-%lu",pid);handle=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(!handle)return false;shared=static_cast<const Presentation*>(MapViewOfFile(handle,FILE_MAP_READ,0,0,sizeof(Presentation)));if(!shared){CloseHandle(handle);handle=nullptr;return false;}}
 for(unsigned n=0;n<3;++n){LONG sequence=shared->sequence;if(sequence&1)continue;MemoryBarrier();value=*shared;MemoryBarrier();if(sequence==shared->sequence)return value.magic==0x38505244&&value.version==1&&value.bytes==sizeof(value)&&value.pid==pid&&value.help<=1&&value.page<2&&value.hud_stage<3&&GetTickCount64()>=value.tick&&GetTickCount64()-value.tick<=500;}return false;
}
inline bool help_open188(){Presentation value;return presentation188.read(value)&&value.help;}
}

extern "C" __declspec(dllexport) const deck188::Presentation* __cdecl S2UPDeckPresentationProbe188(){return &deck188::last_presentation188;}
