#define QTE_SESSION_FIXTURE_ONLY
#include "verify_session.cpp"
#include "qte_presentation93.h"
#include <iostream>
int main(){try{using namespace qte;unsigned cases=0;
 UnitState own{Required|Strength,2,2,0,.5f,.8f,false};Pair pair{1,2,1,2,1,2};
 for(unsigned hz:{30u,60u,75u,120u,144u,165u,240u,360u})for(unsigned n=1;n<350;++n){
  const double age=double(n)/140.;Event event;check(event.begin(pair,Action::Attack,own,own,.47f),"Event start");event.update(pair,own,own,age,false);
  auto current=event.snapshot(),seen=current;seen.elapsed=std::max(0.f,seen.elapsed-float(1./hz));seen.response_elapsed=std::max(0.,seen.response_elapsed-1./hz);
  auto expected=in_success_zone(seen)?Result::Success:reticle_position(seen)<zone_left(seen)?Result::Early:Result::Late;
  check(event.press_visible93(current.serial,Button::Left,true,seen)==expected,"Visible marker and judgement diverged across refresh rate");
  check(std::abs(reticle_position(event.snapshot())-reticle_position(seen))<1e-6,"Feedback jumped away from clicked frame");++cases;
 }
 for(unsigned bad=0;bad<8;++bad){Event event;event.begin(pair,Action::Attack,own,own,.5f);event.update(pair,own,own,.4,false);auto s=event.snapshot();
  if(bad==0)++s.serial;if(bad==1)++s.opponent;if(bad==2)++s.opponent_unit;if(bad==3)s.paused=true;if(bad==4)s.result=Result::Success;if(bad==5)s.elapsed=NAN;if(bad==6)s.response_elapsed=2;if(bad==7)s.zone_width=NAN;
  check(event.press_visible93(event.snapshot().serial,Button::Left,true,s)==Result::Pending,"Invalid frame judged");++cases;
 }
 {Event e;e.begin(pair,Action::Attack,own,own);e.update(pair,own,own,.5,false);auto old=e.snapshot();e.update(pair,own,own,3,false);
  check(e.press_visible93(old.serial,Button::Left,true,old)==Result::Timeout,"Late click revived expired event");++cases;}
 {Event e;e.begin(pair,Action::Defend,own,own);e.update(pair,own,own,.4,false);auto s=e.snapshot();check(e.press_visible93(s.serial,Button::Left,true,s)==Result::WrongButton,"Wrong mouse side accepted");++cases;}
 {transport::Payload p;p.pid=1;p.enabled=p.occupied=1;p.lease=2;p.anchor={1,100,0,.1,1,true,false};p.published_wall=100;Event e;e.begin(pair,Action::Attack,own,own);p.prompt=e.snapshot();
  Frame93 f{p.prompt,1,2,100};Click c;check(frame_click93(p,f,100.01,true,false,true,c)&&c.visible93,"Fresh renderer frame refused");
  for(unsigned bad=0;bad<5;++bad){auto wrong=f;if(bad==0)++wrong.battle;if(bad==1)++wrong.lease;if(bad==2)++wrong.shown.serial;if(bad==3)wrong.submitted=99.9;if(bad==4)wrong.submitted=101;
   check(!frame_click93(p,wrong,100.01,true,false,true,c),"Stale/foreign/future frame accepted");++cases;}
  p.prompt.paused=true;p.anchor.paused=true;f.shown.paused=true;f.submitted=99;check(frame_click93(p,f,100.01,true,false,true,c)&&c.paused&&!c.visible93,"Paused button levels caused stale-presentation cancellation");++cases;
 }
 {SessionFixture f;f.world.setup(true,0);Session s(f.world.reader(),f.world.selection(),f.calls(),World::engine,{0,5,true},true,false,false,false,true);f.session=&s;auto owner=f.world.owner();s.prepare(World::witness,1,owner,{1,0,100,.1,1},true);
  auto serial=s.publication(1,100).prompt.serial;s.levels(true,false);auto busy=owner;busy.observation_busy93=true;
  check(s.prepare(World::witness,1,busy,{1,1,100.1,.1,1},true)==0&&s.occupied()&&s.cancellation_stats92().total==0,"Torn ownership sample canceled a proven pair");
  s.prepare(World::witness,1,owner,{1,1,100.1,.1,1},true);check(s.publication(1,100.1).prompt.result==Result::Pending,"Unqueued level poll answered a frame-bound QTE");
  ++owner.lease;s.prepare(World::witness,1,owner,{1,2,100.2,.1,1},true);auto interrupted=s.interruption93();check(interrupted.active&&interrupted.result==Result::Canceled&&interrupted.serial==serial,"Cancellation feedback vanished with native removal");
  auto feedback=s.interruption_publication93(1,100.2);Snapshot displayed;
  check(feedback.lease==f.world.owner().lease&&transport::presentation(feedback,100.9,displayed)&&displayed.result==Result::Canceled,"Lost owner invalidated interruption presentation/retention");
  check(!transport::presentation(feedback,101.4,displayed),"Interruption stayed visible indefinitely");
  auto total=s.cancellation_stats92().total;s.removed(World::witness,1);s.release();check(s.cancellation_stats92().total==total,"Interruption counted twice");++cases;
 }
 {SessionFixture f;f.world.setup(true,0);Session s(f.world.reader(),f.world.selection(),f.calls(),World::engine,{0,5,true},true,false,false,false,true);
  f.session=&s;auto owner=f.world.owner();s.prepare(World::witness,1,owner,{1,0,100,.1,1},true);
  auto p=s.publication(1,100);Snapshot shown;check(transport::presentation(p,100.01,shown),"Fixture prompt unavailable");
  Click click;check(transport::click(p,100.01,true,false,true,click)&&s.enqueue(click),"Fixture click refused");
  s.update(World::witness,1,owner,{1,1,100.1,.1,1},true);
  check(s.publication(1,100.1).prompt.result!=Result::Pending,"Fixture click was not judged");
  s.removed(World::witness,1);
  const auto canceled=s.interruption93();
  check(canceled.active&&canceled.serial==shown.serial&&canceled.result==Result::Canceled,
        "Native removal after the click lost interruption feedback");
  s.removed(World::witness,1);check(s.cancellation_stats92().total==1,"Duplicate removal repeated interruption");++cases;
 }
 PreciseWait93 wait;LARGE_INTEGER hz,start,end;QueryPerformanceFrequency(&hz);QueryPerformanceCounter(&start);double values[200]{};
 for(unsigned i=0;i<200;++i){LARGE_INTEGER before,after;QueryPerformanceCounter(&before);wait.seconds(.001);QueryPerformanceCounter(&after);values[i]=double(after.QuadPart-before.QuadPart)/hz.QuadPart*1000.;}
 QueryPerformanceCounter(&end);std::sort(values,values+200);
 std::cout<<"PASS: "<<cases<<" presented-frame/refresh/expiry/identity/interruption cases across30/60/75/120/144/165/240/360Hz; authoritative deadlines, no fallback level-click, torn-owner hold and immutable native removal.\n";
 std::cout<<"TIMER93: high_resolution="<<wait.precise()<<" median_ms="<<values[100]<<" p95_ms="<<values[190]<<" max_ms="<<values[199]<<" (scheduler measurements, not hardware input-to-photon latency).\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
