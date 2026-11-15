#include "playable_runtime.cpp"
int main(){try{LARGE_INTEGER f;QueryPerformanceFrequency(&f);frequency=double(f.QuadPart);
 using overlay106::Ack;using overlay106::Key;Key key{4,5,7};unsigned cases=0;
 UnitState own{Required|Strength,2,2,0,.5f,.8f,false};Pair pair{1,2,1,2,1,2};
 for(unsigned hz:{30u,60u,120u,144u,240u}){
  Event event;require(event.begin(pair,Action::Attack,own,own,.47f),"QTE event start");
  for(unsigned i=1;i<200;++i){event.update(pair,own,own,.01,false);auto prompt=event.snapshot();uint64_t number=i;images113.remember(number,key,1,prompt);
   // The CPU has drawn a later frame; native Present acknowledges an earlier one.
   unsigned lag=std::min(3u,i-1);Ack ack{key,11,number-lag,GetTickCount64(),wall()};Frame93 native;
   require(images113.read(ack,key,1,prompt.serial,wall(),native),"Native history lookup");
   require(acknowledged_image106(ack,key,prompt.serial,1),"Valid native frame refused");require(drawn_prompt106.response_elapsed==native.shown.response_elapsed&&drawn_wall106==ack.submitted,"CPU-ahead image used for native frame judgement");
   transport::Payload payload;payload.pid=1;payload.enabled=payload.occupied=1;payload.lease=key.lease;payload.anchor={1,ack.submitted,0,.1,1,true,false};payload.prompt=prompt;payload.published_wall=ack.submitted;
   Frame93 frame{drawn_prompt106,1,key.lease,drawn_wall106};Click click;require(frame_click93(payload,frame,ack.submitted+1./hz,true,false,true,click)&&click.visible93,"Presented geometry did not reach click path");++cases;
  }
 }
 Ack good{key,11,199,GetTickCount64(),wall()};auto serial=drawn_prompt106.serial;
 for(unsigned bad=0;bad<8;++bad){auto ack=good;if(bad==0)ack.number=0;if(bad==1)ack.number+=32;if(bad==2)++ack.key.lease;if(bad==3)++ack.key.actor;if(bad==4)++ack.key.unit;if(bad==5)ack.submitted=NAN;if(bad==6)ack.submitted=wall()+1;if(bad==7)ack.submitted=0;require(!acknowledged_image106(ack,key,serial,1),"Invalid/reused native image accepted");++cases;}
 require(!acknowledged_image106(good,key,serial+1,1),"Another QTE serial accepted");
 // Force the renderer to start its loop just before the producer commits.
 const double render_started=wall();double published=wall();
 while(published<=render_started)published=wall();
 transport::Payload latest;latest.pid=1;latest.enabled=latest.occupied=1;latest.lease=key.lease;
 latest.prompt=drawn_prompt106;
 latest.anchor={1,published,0,.1,1,true,false};latest.published_wall=published;
 transport::Publication channel;transport::publish(channel,latest);
 Snapshot projected;
 require(!transport::presentation(latest,render_started,projected),"Pre-copy timestamp did not reproduce the hidden bar");
 transport::Payload copied;double observed;
 require(sample_publication97(channel,copied,observed)&&observed>=published&&
     transport::presentation(copied,observed,projected)&&projected.serial==latest.prompt.serial,
     "A fresh QTE publication vanished during the copy/clock interleaving");
 QtePresenter96 presenter;
 require(presenter.use_native(4,1,1,true),"Ready native renderer missed initial prompt");
 for(unsigned i=0;i<60;++i)require(presenter.use_native(4,1,1,i%2==0),"Missed acknowledgement switched renderer");
 require(!presenter.use_native(4,1,2,false),"New prompt inherited the previous renderer");
 for(unsigned i=0;i<60;++i)require(!presenter.use_native(4,1,2,true),"A late native acknowledgement duplicated the fallback");
 require(presenter.use_native(4,2,2,true)&&!presenter.use_native(5,2,2,false),"Renderer crossed battle or possession lease");
 std::cout<<"PASS: "<<cases<<" production image-history/native-ack/click cases across30/60/120/144/240Hz; CPU-ahead prompt cannot replace acknowledged geometry, wrong lease/actor/unit/serial and expired ring entries rejected. No scanout/latency claim.\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
