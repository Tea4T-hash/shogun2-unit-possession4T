#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <vector>
// Optional numeric/pixel channel. Existing gameplay/QTE protocols are unchanged.
namespace overlay106 {
constexpr unsigned Magic=0x36314f53,Version=106,HeaderBytes=256;
enum Layer:unsigned {HUD=1,QTE=2,Death=3};
inline unsigned capacity(unsigned layer){return layer==HUD?3840u*2160u*4u:layer==QTE?1024u*1024u*4u:4u;}
inline double wall(){LARGE_INTEGER n,f;QueryPerformanceCounter(&n);QueryPerformanceFrequency(&f);return double(n.QuadPart)/double(f.QuadPart);}
struct Key {unsigned lease{},actor{},unit{};};
inline bool same(Key a,Key b){return a.lease==b.lease&&a.actor==b.actor&&a.unit==b.unit;}
struct Rect {int x{},y{},width{},height{};};
struct Frame {Key key{};unsigned active{},width{},height{},view_width{},view_height{},rect_count{};Rect rect[2]{};uint64_t number{},tick{};};
struct Ack {Key key{};unsigned backend{};uint64_t number{},tick{};double submitted{};};
struct alignas(8) Header {
 unsigned magic{},version{},bytes{},pid{},layer{},capacity{};
 volatile LONG sequence{};unsigned reserved{};Frame frame{};
 volatile LONG ack_sequence{};unsigned ack_reserved{};Ack ack{};
};
static_assert(sizeof(Header)<=HeaderBytes,"Overlay header capacity");
inline void name(wchar_t* out,DWORD pid,unsigned layer){swprintf_s(out,100,L"Local\\Shogun2Overlay106-%lu-%u",pid,layer);}
inline bool valid_header(const Header& h,DWORD pid,unsigned layer){return h.magic==Magic&&h.version==Version&&h.bytes==sizeof(Header)&&h.pid==pid&&h.layer==layer&&h.capacity==capacity(layer);}
inline bool valid_frame(const Frame& f,unsigned layer,uint64_t now){
 if((f.active!=1&&!((layer==QTE||layer==HUD)&&f.active==2))||!f.key.lease||!f.number||now<f.tick||now-f.tick>(layer==HUD?750u:200u)||
    !f.width||!f.height||f.width>3840||f.height>2160||uint64_t(f.width)*f.height*4>capacity(layer)||
    f.view_width<640||f.view_height<360||f.view_width>16384||f.view_height>16384||!f.rect_count||f.rect_count>2)return false;
 for(unsigned i=0;i<f.rect_count;++i){auto q=f.rect[i];if(q.width<=0||q.height<=0||q.width>16384||q.height>16384||
  q.x < -16384||q.y < -16384||q.x>16384||q.y>16384)return false;}
 return true;
}
class Channel {
protected:
 HANDLE mapping_{};Header* header_{};DWORD pid_{};unsigned layer_{};
 bool map(DWORD pid,unsigned layer,bool create){
  if(header_)return pid_==pid&&layer_==layer;
  if(layer<HUD||layer>Death)return false;wchar_t n[100];name(n,pid,layer);
  HANDLE m=create?CreateFileMappingW(INVALID_HANDLE_VALUE,nullptr,PAGE_READWRITE,0,HeaderBytes+capacity(layer),n):OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,n);
  if(!m)return false;DWORD error=GetLastError();auto h=static_cast<Header*>(MapViewOfFile(m,FILE_MAP_ALL_ACCESS,0,0,HeaderBytes+capacity(layer)));
  if(!h){CloseHandle(m);return false;}
  if(create&&error!=ERROR_ALREADY_EXISTS){memset(h,0,HeaderBytes);h->version=Version;h->bytes=sizeof(Header);h->pid=pid;h->layer=layer;h->capacity=capacity(layer);MemoryBarrier();h->magic=Magic;}
  if(!valid_header(*h,pid,layer)){UnmapViewOfFile(h);CloseHandle(m);return false;}
  mapping_=m;header_=h;pid_=pid;layer_=layer;return true;
 }
public:
 Channel()=default;Channel(const Channel&)=delete;Channel& operator=(const Channel&)=delete;
 ~Channel(){close();}
 void close(){if(header_)UnmapViewOfFile(header_);if(mapping_)CloseHandle(mapping_);header_=nullptr;mapping_=nullptr;pid_=0;layer_=0;}
 bool ack(Ack& out)const{if(!header_)return false;for(unsigned i=0;i<3;++i){LONG s=header_->ack_sequence;if(s&1)continue;MemoryBarrier();auto a=header_->ack;MemoryBarrier();if(s==header_->ack_sequence){out=a;return a.backend&&GetTickCount64()>=a.tick&&GetTickCount64()-a.tick<200;}}return false;}
};
class Publisher:public Channel {
 uint64_t next_{};
public:
 bool open(DWORD pid,unsigned layer){if(header_)return pid_==pid&&layer_==layer;if(!map(pid,layer,true))return false;next_=header_->frame.number;return true;}
 void hide(){if(!header_)return;InterlockedIncrement(&header_->sequence);header_->frame.active=0;MemoryBarrier();InterlockedIncrement(&header_->sequence);}
 uint64_t publish(Frame f,const void* pixels){
  if(!header_||!pixels)return 0;f.active=f.active==2?2:1;f.tick=GetTickCount64();f.number=++next_;if(!valid_frame(f,layer_,f.tick))return 0;
  InterlockedIncrement(&header_->sequence);MemoryBarrier();memcpy(reinterpret_cast<unsigned char*>(header_)+HeaderBytes,pixels,size_t(f.width)*f.height*4);header_->frame=f;MemoryBarrier();InterlockedIncrement(&header_->sequence);return f.number;
 }
};
struct Snapshot {Frame frame{};std::vector<unsigned char> pixels;};
class Reader:public Channel {
 Snapshot snapshot_{};
 std::vector<unsigned char> pending_pixels_;
public:
 bool open(DWORD pid,unsigned layer){return map(pid,layer,false);}
 const Snapshot* read(uint64_t now){
  if(!header_||!valid_header(*header_,pid_,layer_))return nullptr;
  for(unsigned i=0;i<3;++i){LONG s=header_->sequence;if(s&1)continue;MemoryBarrier();Frame f=header_->frame;
   if(!valid_frame(f,layer_,now)){MemoryBarrier();if(s==header_->sequence)return nullptr;continue;}
   bool changed=f.number!=snapshot_.frame.number||!same(f.key,snapshot_.frame.key)||f.width!=snapshot_.frame.width||f.height!=snapshot_.frame.height;
   if(changed){pending_pixels_.resize(size_t(f.width)*f.height*4);memcpy(pending_pixels_.data(),reinterpret_cast<unsigned char*>(header_)+HeaderBytes,pending_pixels_.size());}
   MemoryBarrier();if(s!=header_->sequence)continue;if(changed)snapshot_.pixels.swap(pending_pixels_);snapshot_.frame=f;return &snapshot_;
  }
  // A publisher writing its next DIB must not make a completed image blink.
  // This is a bounded pixel cache; the consumer still proves CURRENT numeric
  // lease/actor/unit on every draw. No native identity grace is introduced.
  return valid_frame(snapshot_.frame,layer_,now)?&snapshot_:nullptr;
 }
 void acknowledge(unsigned backend,const Frame* f=nullptr,double submitted=0){if(!header_)return;Ack a{};a.backend=backend;a.tick=GetTickCount64();a.submitted=submitted>0?submitted:wall();if(f){a.key=f->key;a.number=f->number;}
  InterlockedIncrement(&header_->ack_sequence);MemoryBarrier();header_->ack=a;MemoryBarrier();InterlockedIncrement(&header_->ack_sequence);}
};
}
