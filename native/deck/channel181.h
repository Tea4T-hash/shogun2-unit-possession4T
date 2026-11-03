#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "controls181.h"
#include "device192.h"
#include <cstring>
namespace deck181 {
constexpr unsigned channel_magic=0x31384453,channel_version=181,glyph_count=30,glyph_side=32;
struct Glyph {unsigned ready{};unsigned char bgra[glyph_side*glyph_side*4]{};};
struct Sample {unsigned context{},lease{},actor{},unit{},focused{};uint64_t tick{};uint32_t keys[8]{};};
struct Shared {unsigned magic{},version{},bytes{},pid{};volatile LONG sequence{};Sample sample{};unsigned glyph_generation{};Glyph glyphs[glyph_count]{};};
inline void channel_name(wchar_t(&name)[100],DWORD pid){swprintf_s(name,L"Local\\S2UP-DeckControls181-%lu",pid);}
inline bool valid(const Shared& s,DWORD pid){return s.magic==channel_magic&&s.version==channel_version&&s.bytes==sizeof(Shared)&&s.pid==pid;}
inline bool fresh(const Sample& s,uint64_t now){return s.focused&&s.context<=unsigned(deck174::Context::Inactive)&&now>=s.tick&&now-s.tick<=100;}
struct Channel {
 SRWLOCK writer=SRWLOCK_INIT;HANDLE handle{};Shared* shared{};DWORD pid{};uint64_t retry{};bool writable{};
 Channel()=default;Channel(const Channel&)=delete;Channel& operator=(const Channel&)=delete;
 ~Channel(){close();}
 void close(){if(shared)UnmapViewOfFile(shared);if(handle)CloseHandle(handle);shared=nullptr;handle=nullptr;pid=0;}
 bool open(DWORD process,bool write){
  if(shared)return pid==process&&writable==write;
  if(!write&&GetTickCount64()<retry)return false;retry=GetTickCount64()+1000;
  wchar_t name[100];channel_name(name,process);
  HANDLE h=write?CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,sizeof(Shared),name):OpenFileMappingW(FILE_MAP_READ,FALSE,name);
  if(!h)return false;DWORD error=GetLastError();auto p=static_cast<Shared*>(MapViewOfFile(h,write?FILE_MAP_ALL_ACCESS:FILE_MAP_READ,0,0,sizeof(Shared)));
  if(!p){CloseHandle(h);return false;}
  if(write&&error!=ERROR_ALREADY_EXISTS){memset(p,0,sizeof(*p));p->version=channel_version;p->bytes=sizeof(Shared);p->pid=process;MemoryBarrier();p->magic=channel_magic;}
  if(!valid(*p,process)){UnmapViewOfFile(p);CloseHandle(h);return false;}
  shared=p;handle=h;pid=process;writable=write;return true;
 }
 bool read(Sample& out){
  if(!shared||!valid(*shared,pid))return false;
  for(unsigned i=0;i<3;++i){LONG seq=shared->sequence;if(seq&1)continue;MemoryBarrier();auto s=shared->sample;MemoryBarrier();if(seq==shared->sequence){out=s;return fresh(out,GetTickCount64());}}
  return false;
 }
 bool publish(const Sample& sample){if(!shared||!writable)return false;AcquireSRWLockExclusive(&writer);InterlockedIncrement(&shared->sequence);MemoryBarrier();shared->sample=sample;MemoryBarrier();InterlockedIncrement(&shared->sequence);ReleaseSRWLockExclusive(&writer);return true;}
 bool glyph(unsigned index,Glyph& out){if(!shared||index>=glyph_count||!valid(*shared,pid))return false;for(unsigned i=0;i<3;++i){LONG seq=shared->sequence;if(seq&1)continue;MemoryBarrier();memcpy(&out,&shared->glyphs[index],sizeof(out));MemoryBarrier();if(seq==shared->sequence)return out.ready==1;}return false;}
 void put_glyph(unsigned index,const Glyph& value){if(!shared||!writable||index>=glyph_count)return;AcquireSRWLockExclusive(&writer);InterlockedIncrement(&shared->sequence);memcpy(&shared->glyphs[index],&value,sizeof(value));++shared->glyph_generation;MemoryBarrier();InterlockedIncrement(&shared->sequence);ReleaseSRWLockExclusive(&writer);}
};
inline Channel& reader(){static thread_local Channel channel;return channel;}
inline bool read_sample(DWORD process,Sample& out){auto& r=reader();if(r.pid&&r.pid!=process)r.close();return device192::enabled(process)&&r.open(process,false)&&r.read(out);}
// Merge numeric first-party controller intent with the existing input consumers.
// No SendInput, OS key state mutation or Steam user configuration changes.
inline volatile LONG native_key_pending{};
struct ShortcutQueue183 {
 SRWLOCK lock=SRWLOCK_INIT;struct Item{unsigned action{};uint64_t tick{};};Item items[16]{};unsigned count{};bool gap{};
 void clear(){AcquireSRWLockExclusive(&lock);count=0;gap=false;ReleaseSRWLockExclusive(&lock);}
 bool put(unsigned action,uint64_t now){if(action>=deck183::ShortcutCount)return false;AcquireSRWLockExclusive(&lock);bool ok=count<16;if(ok)items[count++]={action,now};else count=0;ReleaseSRWLockExclusive(&lock);return ok;}
 bool take(uint64_t now,deck183::Chord& chord){AcquireSRWLockExclusive(&lock);bool ok=false;if(gap){gap=false;}else if(count){auto item=items[0];for(unsigned i=1;i<count;++i)items[i-1]=items[i];--count;if(now>=item.tick&&now-item.tick<=100){chord=deck183::chords[item.action];ok=true;gap=true;}else count=0;}ReleaseSRWLockExclusive(&lock);return ok;}
};
inline ShortcutQueue183 native_shortcuts183;
// Refresh the original game's cached modifiers after the DI merge. Its group
// handler f91e40 chooses assignment/recall from cached1bbe84f/852, not the
// freshly merged key buffer. f92ae0 and its readers are already hash-guarded.
inline void merge_rts_keys194(unsigned char* keys,const Sample& s,const deck183::Chord* chord,uintptr_t image){
 auto held=[&](unsigned key){return(s.keys[key/32]&(uint32_t(1)<<(key%32)))!=0;};
 const bool all=chord&&chord->control&&chord->scan==0x1e;
 if(held(VK_SHIFT)&&!all)keys[0x2a]|=0x80;if(held(VK_CONTROL))keys[0x1d]|=0x80;
 if(held(VK_SPACE))keys[0x39]|=0x80;
 if(chord){keys[chord->scan]|=0x80;if(chord->control)keys[0x1d]|=0x80;}
 if(image)reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(image+0xf92ae0)(0,0);
}
inline void native_keys(unsigned char* keys,bool valid,uintptr_t image=0){
 if(!valid||!keys)return;
 LONG pending=InterlockedExchange(&native_key_pending,0);if(pending&1)keys[0x25]|=0x80;if(pending&2)keys[0x1]|=0x80;
 static thread_local bool owned_modifiers{};
 Sample s;if(!read_sample(GetCurrentProcessId(),s)||s.context!=unsigned(deck174::Context::RTS)){
  native_shortcuts183.clear();if(owned_modifiers&&image)reinterpret_cast<void(__stdcall*)(uintptr_t,uintptr_t)>(image+0xf92ae0)(0,0);owned_modifiers=false;return;
 }
 deck183::Chord chord;const bool action=native_shortcuts183.take(GetTickCount64(),chord);
 merge_rts_keys194(keys,s,action?&chord:nullptr,image);owned_modifiers=true;
}
inline bool controller_held(const Sample& s,unsigned key){return key<256&&key!=VK_ESCAPE&&s.context==unsigned(deck174::Context::Possession)&&s.lease&&(s.keys[key/32]&(uint32_t(1)<<(key%32)));}
inline SHORT key_state(int key){
 SHORT physical=GetAsyncKeyState(key);if(key<0||key>=256)return physical;
 Sample s;if(read_sample(GetCurrentProcessId(),s)&&controller_held(s,unsigned(key)))physical=SHORT(physical|0x8000);
 return physical;
}
} // deck181
