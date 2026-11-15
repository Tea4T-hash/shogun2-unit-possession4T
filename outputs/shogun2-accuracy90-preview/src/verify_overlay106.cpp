#include "native_overlay106.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <atomic>
using namespace overlay106;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Inspect:Reader {Header* header(){return header_;}};
int main(){try{
 Publisher writer;Inspect reader;DWORD pid=GetCurrentProcessId();check(!reader.open(pid,HUD),"Absent mapping opened");
 check(writer.open(pid,HUD)&&reader.open(pid,HUD),"Mapping handshake");Frame frame;frame.key={3,5,7};frame.width=frame.height=16;frame.view_width=1920;frame.view_height=1080;frame.rect_count=1;frame.rect[0]={0,0,1920,1080};
 std::vector<unsigned> pixels(256,0x80402010);auto n=writer.publish(frame,pixels.data());auto s=reader.read();check(n&&s&&s->frame.number==n&&!memcmp(s->pixels.data(),pixels.data(),1024),"Pixel publication");
 reader.acknowledge(11,&s->frame);Ack a;check(writer.ack(a)&&a.number==n&&same(a.key,frame.key)&&a.backend==11&&a.submitted>0,"Acknowledgement identity");
 for(unsigned test=0;test<14;++test){Frame bad=s->frame;
  if(test==0)bad.key.lease=0;if(test==1)bad.active=0;if(test==2)bad.active=2;if(test==3)bad.width=0;if(test==4)bad.width=3841;if(test==5)bad.height=2161;if(test==6)bad.rect_count=0;if(test==7)bad.rect_count=3;if(test==8)bad.rect[0].width=-1;if(test==9)bad.view_width=0;if(test==10)bad.tick=GetTickCount64()+1000;if(test==11)bad.tick=GetTickCount64()-751;if(test==12)bad.number=0;if(test==13)bad.rect[0].x=16385;
  check(!valid_frame(bad,HUD,GetTickCount64()),"Invalid numeric frame admitted");
 }
 auto h=reader.header();auto original=h->magic;h->magic=0;check(!reader.read(),"Corrupt header admitted");h->magic=original;
 InterlockedIncrement(&h->sequence);auto complete=reader.read();check(complete&&complete->frame.number==n&&!memcmp(complete->pixels.data(),pixels.data(),1024),"Busy publisher lost/corrupted completed image");InterlockedIncrement(&h->sequence);
 writer.hide();check(!reader.read(),"Inactive pixels retained");
 writer.close();check(writer.open(pid,HUD),"Publisher restart");auto second=writer.publish(frame,pixels.data());check(second>n&&reader.read()->frame.number==second,"Restart reused image serial");
 std::atomic<bool> done{false};std::thread producer([&]{for(unsigned i=0;i<1500;++i){std::fill(pixels.begin(),pixels.end(),i);writer.publish(frame,pixels.data());}done=true;});unsigned samples=0;
 while(!done){auto image=reader.read();if(!image)continue;auto words=reinterpret_cast<const unsigned*>(image->pixels.data());for(unsigned i=1;i<256;++i)check(words[i]==words[0],"Torn pixel image");++samples;}producer.join();
 h->ack.tick=GetTickCount64()-201;check(!writer.ack(a),"Expired native presentation hid fallback");reader.acknowledge(0);check(!writer.ack(a),"Failed native presentation hid fallback");reader.acknowledge(11);check(writer.ack(a)&&!a.number,"Backend readiness handshake");
 Publisher qte;Inspect q;check(qte.open(pid,QTE)&&q.open(pid,QTE),"QTE channel");
 // Force the interleaving: this presentation began before the frame was published.
 const auto presentation_start=GetTickCount64();
 while(GetTickCount64()==presentation_start)Sleep(1);
 frame.active=1;auto pending=qte.publish(frame,pixels.data());auto fresh=q.read();
 check(pending&&fresh&&fresh->frame.number==pending&&fresh->frame.tick>presentation_start&&
       !valid_frame(fresh->frame,QTE,presentation_start),"A newly completed frame was rejected using the old presentation clock");
 q.header()->frame.tick=GetTickCount64()+1000;
 check(!q.read(),"A genuinely future timestamp bypassed admission");
 frame.active=2;auto feedback=qte.publish(frame,pixels.data());check(feedback&&q.read()->frame.active==2,"Feedback channel");
 Publisher death;Reader d;check(death.open(pid,Death)&&d.open(pid,Death),"Death channel");frame.width=frame.height=1;frame.active=1;frame.rect_count=2;frame.rect[0]={0,0,1920,240};frame.rect[1]={0,840,1920,240};unsigned black=0xff000000;check(death.publish(frame,&black)&&d.read()->frame.rect_count==2,"Death numeric lids");
 std::cout<<"PASS: overlay106 pixel/ack identity, 14 bounds/stale cases, torn sequence/header, monotonic restart, "<<samples<<" concurrent samples/1500 writes, inactive/failure fallback and QTE/death channels.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
