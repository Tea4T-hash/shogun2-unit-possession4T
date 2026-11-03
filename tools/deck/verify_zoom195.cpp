#include "../../native/deck/rts175.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
using namespace deck174;using namespace deck175;
struct Camera {Vec2 pan{};float turn{},zoom{},pitch{},dt{};};
struct FakeHost:Host {
 Identity identity{1,2,3,4,5,6};bool alive=true;std::vector<Camera> moves;float scrolled{};
 bool current(const Identity& i)override{return alive&&identity==i;}
 void cancel_owned(const Identity&)override{}
 bool pointer(Vec2)override{return true;}
 bool select(bool,bool)override{return true;}
 bool order(unsigned,bool)override{return true;}
 bool halt()override{return true;}
 bool scroll(float n)override{scrolled+=n;return true;}
 bool camera(Vec2 p,float t,float z,float h,float dt)override{moves.push_back({p,t,z,h,dt});return true;}
};
struct Exercise {
 Rts r;FakeHost host;uint64_t now=1000000,sequence=0;
 Exercise(){Config c;c.community_rts=true;r.configure(c);feed();}
 ContextProof proof(){return {host.identity,true,true,true,false,false};}
 Frame next(uint64_t buttons,Vec2 pan={},Vec2 right={},Vec2 pad={}){
  Frame f;f.source=Source::Simulation;f.sample_us=now;f.sequence=++sequence;f.epoch=1;
  f.connected=f.focused=f.native_bindings=true;f.active_buttons=all_buttons;f.buttons=buttons;
  f.left_stick=pan;f.right_stick=right;f.left_pad=pad;return f;
 }
 void feed(uint64_t b=0,Vec2 pan={},Vec2 right={},uint64_t dt=8000,Vec2 pad={}){
  now+=dt;auto p=proof();r.begin(p,host,now);r.sample(next(b,pan,right,pad),p,host,now);r.finish(host,now);
 }
 float speed()const{return host.moves.empty()?0:host.moves.back().zoom;}
 void full(){for(unsigned i=0;i<75;++i)feed(bit(Button::L1));assert(std::fabs(speed()-.8f)<.000001f);}
};
static unsigned passed;
#define TEST(label,...) do{__VA_ARGS__;++passed;std::cout<<"PASS " label "\n";}while(0)
int main(){
 std::cout.setf(std::ios::unitbuf);
 TEST("callback partitions give equal travel and a lower steady speed",{
  for(int hz:{30,60,125,250})for(int sign:{-1,1}){
   deck195::ShoulderZoom z;double travel=0;float previous=0;
   for(int i=0;i<hz;++i){float v=z.step(float(sign),1.f/hz);assert(std::fabs(v)+.000001f>=previous&&std::fabs(v)<=.800001f);previous=std::fabs(v);travel+=v/hz;}
   assert(std::fabs(travel-sign*.62)<.000001);
   assert(std::fabs(z.step(float(sign),.008f)-sign*.8f)<.000001f);
  }
  deck195::ShoulderZoom z;int us=0;double distance=0;unsigned i=0;
  while(us<1000000){int step=std::min(1000000-us,(i++%3==0?4000:i%3==1?17000:33000));float dt=step/1000000.f;distance+=z.step(1,dt)*dt;us+=step;}
  assert(std::fabs(distance-.62)<.000001);
 });
 TEST("actual RTS shoulders accelerate symmetrically and cancel together",{
  Exercise a,b;float prior=0;
  for(unsigned i=0;i<75;++i){a.feed(bit(Button::L1));b.feed(bit(Button::R1));assert(a.speed()>0&&a.speed()<=.800001f&&a.speed()>=prior-.000001f&&std::fabs(a.speed()+b.speed())<.000001f);prior=a.speed();}
  const auto count=a.host.moves.size();a.feed(bit(Button::L1)|bit(Button::R1));assert(a.host.moves.size()==count);
  a.feed(bit(Button::L1));assert(a.speed()>0&&a.speed()<.002f);
 });
 TEST("release stops without coasting; reversal starts slowly",{
  Exercise x;x.full();auto count=x.host.moves.size();x.feed();x.feed();assert(x.host.moves.size()==count);
  x.feed(bit(Button::L1));assert(x.speed()>0&&x.speed()<.002f);x.full();x.feed(bit(Button::R1));assert(x.speed()<0&&x.speed()>-.002f);
 });
 TEST("release and repress inside a callback resets held speed",{
  Exercise x;x.full();auto p=x.proof();x.now+=4000;x.r.sample(x.next(0),p,x.host,x.now);
  x.now+=4000;x.r.sample(x.next(bit(Button::L1)),p,x.host,x.now);x.r.finish(x.host,x.now);assert(x.speed()>0&&x.speed()<.002f);
 });
 TEST("focus modal possession thread and identity transitions reset speed",{
  for(unsigned kind=0;kind<6;++kind){Exercise x;x.full();auto p=x.proof();auto n=x.host.moves.size();
   if(kind==0)p.focused=false;if(kind==1)p.blocked=true;if(kind==2)p.possessed=true;if(kind==3)p.owner_thread=false;if(kind==4)p.rts=false;if(kind==5)++p.identity.root;
   x.now+=8000;x.r.begin(p,x.host,x.now);x.r.sample(x.next(bit(Button::L1)),p,x.host,x.now);x.r.finish(x.host,x.now);
   assert(x.host.moves.size()==n);x.feed();x.feed(bit(Button::L1));assert(x.speed()>0&&x.speed()<.002f);
  }
 });
 TEST("invalid or disconnected frames cannot carry ramp forward",{
  for(unsigned kind=0;kind<4;++kind){Exercise x;x.full();auto p=x.proof();auto n=x.host.moves.size();x.now+=8000;auto f=x.next(bit(Button::L1));
   if(kind==0)f.connected=false;if(kind==1)f.focused=false;if(kind==2)f.sample_us-=1000000;if(kind==3)f.right_stick.x=std::numeric_limits<float>::quiet_NaN();
   x.r.sample(f,p,x.host,x.now);x.r.finish(x.host,x.now);assert(x.host.moves.size()==n);
   x.feed();x.feed(bit(Button::L1));assert(x.speed()>0&&x.speed()<.002f);
  }
 });
 TEST("missing samples and stalled callbacks restart rather than retain speed",{
  Exercise x;x.full();auto n=x.host.moves.size();x.now+=8000;x.r.finish(x.host,x.now);assert(x.host.moves.size()==n);
  x.feed(bit(Button::L1));assert(x.speed()>0&&x.speed()<.002f);x.full();x.feed(bit(Button::L1),{},{},300000);assert(x.speed()>0&&x.speed()<.03f);
 });
 TEST("pan turn pitch and pad scroll stay independent of shoulder smoothing",{
  Exercise a,b;for(unsigned i=0;i<80;++i){a.feed(0,{.6f,-.8f},{-.5f,.8f});b.feed(bit(Button::L1),{.6f,-.8f},{-.5f,.8f});
   auto x=a.host.moves.back(),y=b.host.moves.back();assert(x.pan.x==y.pan.x&&x.pan.y==y.pan.y&&x.turn==y.turn&&x.pitch==y.pitch&&x.dt==y.dt&&x.zoom==0&&y.zoom>0);}
  a.feed(bit(Button::LeftPadTouch),{},{},8000,{0,0});b.feed(bit(Button::LeftPadTouch)|bit(Button::L1),{},{},8000,{0,0});
  a.feed(bit(Button::LeftPadTouch),{},{},8000,{0,.5f});b.feed(bit(Button::LeftPadTouch)|bit(Button::L1),{},{},8000,{0,.5f});assert(a.host.scrolled==.5f&&a.host.scrolled==b.host.scrolled);
 });
 TEST("invalid durations and values clear the helper",{
  for(float dt:{0.f,-.01f,.1f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}){deck195::ShoulderZoom z;for(int i=0;i<100;++i)z.step(1,.008f);assert(z.step(1,dt)==0&&z.step(1,.008f)<.002f);}
  for(float input:{0.f,2.f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}){deck195::ShoulderZoom z;for(int i=0;i<100;++i)z.step(1,.008f);assert(z.step(input,.008f)==0&&z.step(1,.008f)<.002f);}
 });
 std::cout<<"PASS zoom195 policy: "<<passed<<" ramp, real RTS lifecycle, isolation and callback-rate checks; no live game or inputs\n";
}
