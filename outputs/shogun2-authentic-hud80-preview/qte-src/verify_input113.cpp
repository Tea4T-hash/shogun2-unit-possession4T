#include "qte_presented113.h"
#include <atomic>
#include <iostream>
#include <stdexcept>
using namespace qte;
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Concurrent113 {PresentedImages113 images;std::atomic<unsigned> number{0},accepted{0};std::atomic<bool> done{false},failed{false};};
static DWORD WINAPI produce113(void* raw){auto& x=*static_cast<Concurrent113*>(raw);
    for(unsigned n=1;n<=40000;++n){Snapshot p{};p.serial=9;p.elapsed=float(n);p.response_elapsed=double(n)*.00001;
        x.images.remember(n,{4,5,6},7,p);x.number.store(n);if(n%37==0)SwitchToThread();}
    x.done=true;return 0;
}
static DWORD WINAPI consume113(void* raw){auto& x=*static_cast<Concurrent113*>(raw);unsigned trailing=0;
    while(!x.done||trailing++<1000){unsigned n=x.number.load();if(!n)continue;const double now=overlay106::wall();
        overlay106::Ack ack{{4,5,6},11,n,GetTickCount64(),now-.001};Frame93 output;
        if(x.images.read(ack,{4,5,6},7,9,now,output)){
            ++x.accepted;if(output.shown.elapsed!=float(n)||output.shown.response_elapsed!=double(n)*.00001)x.failed=true;
        }
    }return 0;
}
int main(){try{unsigned cases=0,old_wrong=0;
    for(unsigned hz:{30u,60u,75u,120u,144u,240u})for(unsigned delay:{1u,2u,3u})for(unsigned backend:{9u,11u}){
        PresentedImages113 images;UnitState own{Required|Strength,2,2,0,.5f,.8f,false};Pair pair{7,4,1,2,3,4};
        Event old_event,new_event;check(old_event.begin(pair,Action::Attack,own,own,.5f)&&new_event.begin(pair,Action::Attack,own,own,.5f),"Begin fixture");
        const double elapsed=new_event.snapshot().duration*.5;
        old_event.update(pair,own,own,elapsed,false);new_event.update(pair,own,own,elapsed,false);
        auto current=new_event.snapshot(),cached=current;
        cached.elapsed=cached.duration*(zone_left(current)-.02f);cached.response_elapsed=cached.elapsed;
        const double now=100.,age=std::min(.045,double(delay)/hz);
        Frame93 render_cache{cached,7,4,now-age};overlay106::Key key{4,1,3};
        images.remember(1,key,7,cached);images.remember(2,key,7,current);
        // The game has presented image2 since the last render-worker cache poll.
        // A still newer CPU image3 is deliberately OUTSIDE and not acknowledged.
        auto cpu_ahead=current;cpu_ahead.elapsed=cpu_ahead.duration*.95f;images.remember(3,key,7,cpu_ahead);
        overlay106::Ack ack{key,backend,2,1,now-.0005};
        transport::Payload live;live.pid=99;live.enabled=live.occupied=1;live.lease=4;live.prompt=current;
        live.anchor={7,now,10.,10.1,1.,true,false};live.published_wall=now;
        Click old_click;check(frame_click93(live,render_cache,now,true,false,true,old_click),"Old cache fixture not eligible");
        check(old_event.press_visible93(current.serial,Button::Left,true,old_click.shown93)==Result::Early,"Old input did not reproduce stale-image rejection");++old_wrong;
        Frame93 selected=render_cache;check(images.newest(ack,key,live,now,selected),"Newest acknowledged image not selected");
        Click click;check(frame_click93(live,selected,now,true,false,true,click)&&click.visible93,"Latest image not eligible");
        check(click.shown93.elapsed==current.elapsed&&click.shown93.elapsed!=cpu_ahead.elapsed,"Unsubmitted CPU image selected");
        check(new_event.press_visible93(current.serial,Button::Left,true,click.shown93)==Result::Success,"Presented success judged differently");
        for(unsigned bad=0;bad<11;++bad){auto invalid=ack;auto payload=live;auto expected=key;double at=now;
            if(bad==0)invalid.number=0;if(bad==1)invalid.number+=32;if(bad==2)++invalid.key.lease;if(bad==3)++invalid.key.actor;if(bad==4)++invalid.key.unit;
            if(bad==5)invalid.submitted=NAN;if(bad==6)invalid.submitted=now+.001;if(bad==7)invalid.backend=0;
            if(bad==8)++payload.anchor.battle;if(bad==9)++payload.prompt.serial;if(bad==10)++payload.lease;
            Frame93 unchanged=render_cache;check(!images.newest(invalid,expected,payload,at,unchanged)&&unchanged.shown.elapsed==render_cache.shown.elapsed,"Invalid acknowledgement changed selected frame");++cases;
        }
        auto old_ack=ack;old_ack.submitted=now-.049;Frame93 fresh=selected;
        check(!images.newest(old_ack,key,live,now,fresh)&&fresh.submitted==selected.submitted,"Older ack replaced a newer fallback frame");
        auto stale_ack=ack;stale_ack.submitted=now-.051;Frame93 stale;
        check(images.read(stale_ack,key,7,current.serial,now,stale)&&!frame_click93(live,stale,now,true,false,true,click),"Expired presented image judged");
        auto paused=live;paused.prompt.paused=paused.anchor.paused=true;
        check(frame_click93(paused,selected,now,true,false,true,click)&&click.paused&&!click.visible93,"Paused input judged");
        auto expired=live;expired.prompt.result=Result::Timeout;expired.prompt.response_elapsed=3.;expired.prompt.elapsed=float(std::max(2.*double(expired.prompt.duration),3.));
        check(!frame_click93(expired,selected,now,true,false,true,click),"Image selection revived expired event");cases+=5;
    }
    {overlay106::Publisher publisher;overlay106::Reader native,input;
        check(publisher.open(GetCurrentProcessId(),overlay106::QTE)&&native.open(GetCurrentProcessId(),overlay106::QTE)&&input.open(GetCurrentProcessId(),overlay106::QTE),"Real numeric/pixel endpoints unavailable");
        overlay106::Frame f{};f.key={4,1,3};f.width=f.height=1;f.view_width=1920;f.view_height=1080;f.rect_count=1;f.rect[0]={0,0,1,1};unsigned pixel=0xffffffff;
        PresentedImages113 images;Snapshot first{};first.serial=8;first.elapsed=.2f;Snapshot second=first;second.elapsed=.5f;
        auto n=publisher.publish(f,&pixel);images.remember(n,f.key,7,first);auto old=native.read();check(old&&old->frame.number==n,"First channel image");native.acknowledge(11,&old->frame,overlay106::wall());
        Frame93 cached{first,7,4,overlay106::wall()-.01};n=publisher.publish(f,&pixel);images.remember(n,f.key,7,second);auto current=native.read();check(current&&current->frame.number==n,"Second channel image");native.acknowledge(11,&current->frame,overlay106::wall());
        // No render-worker acknowledgement poll occurs between these presents
        // and input's independent read of the existing shared channel.
        overlay106::Ack ack;check(input.ack(ack),"Input acknowledgement read");transport::Payload live;live.lease=4;live.anchor.battle=7;live.prompt.serial=8;
        check(images.newest(ack,f.key,live,overlay106::wall(),cached)&&cached.shown.elapsed==second.elapsed,"Input endpoint retained render-worker cache");++cases;
    }
    Concurrent113 concurrent;HANDLE threads[2]={CreateThread(nullptr,0,produce113,&concurrent,0,nullptr),CreateThread(nullptr,0,consume113,&concurrent,0,nullptr)};
    check(threads[0]&&threads[1],"Concurrency workers unavailable");check(WaitForMultipleObjects(2,threads,TRUE,10000)==WAIT_OBJECT_0,"History concurrency timeout");
    for(auto h:threads)CloseHandle(h);check(!concurrent.failed&&concurrent.accepted>0,"Torn or untested history read");
    std::cout<<"PASS: "<<cases<<" presented-input checks; old render-cache input rejects "<<old_wrong<<" already-presented successes, corrected input matches all36; DX9/DX11,30/60/75/120/144/240Hz, key/battle/serial/ring/expiry/pause and40000 concurrent publications ("<<concurrent.accepted<<" validated reads). No physical scanout claim.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
