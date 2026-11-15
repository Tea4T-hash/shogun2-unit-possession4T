#include "../../native/deck/fine216.h"
#include <cassert>
#include <vector>
#include <iostream>
using namespace deck174;
int main(){unsigned cases=0;auto check=[&](bool ok){assert(ok);++cases;};deck216::FinePad pad;Frame f;f.buttons=bit(Button::LeftPadTouch);f.left_pad={1,1};check(!pad.sample(f,100).active);
 for(Vec2 d:std::vector<Vec2>{{0,1},{0,-1},{1,0},{-1,0},{1,1},{1,-1},{-1,1},{-1,-1}}){f.left_pad=d;f.buttons=bit(Button::LeftPadClick);auto first=pad.sample(f,1000);check(first.began&&first.active&&!first.released&&std::fabs(std::hypot(first.move.x,first.move.y)-1)<.001f);
  bool repeat=false;for(unsigned n=0;n<1000;++n){auto held=pad.sample(f,2000+n*8000);repeat|=held.began||held.released||held.turn!=0;}check(!repeat);
  f.buttons=0;auto released=pad.sample(f,9000000);check(released.released&&!released.active&&released.move.x*d.x+released.move.y*d.y>.9f);check(!pad.sample(f,9008000).released);
 }
 f.left_pad={0,1};f.buttons=bit(Button::LeftPadClick);pad.sample(f,100);
 f.left_pad={.05f,.98f};check(!pad.sample(f,200).turning);
 f.left_pad={.4f,0};auto slipped=pad.sample(f,300);
 check(!slipped.turning&&slipped.turn==0&&slipped.move.y==1.f);
 check(pad.sample(f,400).move.y==1.f);f.buttons=0;check(pad.sample(f,500).released);

 f.left_pad={.75f,0};f.buttons=bit(Button::LeftPadClick);auto center=pad.sample(f,600);
 check(center.began&&center.turning&&!center.move.x&&!center.move.y);
 f.left_pad={.79f,.6f};check(pad.sample(f,650).turn==0);
 f.left_pad={.99f,0};auto drag=pad.sample(f,700);
 check(drag.turning&&drag.turn>25.f&&!drag.released);
 f.buttons=0;f.left_pad={};auto rotated=pad.sample(f,800);
 check(rotated.released&&rotated.turn==drag.turn&&!rotated.move.x&&!rotated.move.y);

 f.left_pad={.76f,0};f.buttons=bit(Button::LeftPadClick);check(!pad.sample(f,900).turning);
 f.left_pad={2,0};check(pad.sample(f,1000).cancel);
 f.buttons=0;f.left_pad={};check(!pad.sample(f,1100).released);
 f.buttons=bit(Button::LeftPadClick);pad.sample(f,1200);f.left_pad={-1,0};
 check(pad.sample(f,1300).turn<0);
 pad.reset();
 constexpr float diagonal=.70710678f;
 const Vec2 directions[]={{1,0},{diagonal,diagonal},{0,1},{-diagonal,diagonal},
                          {-1,0},{-diagonal,-diagonal},{0,-1},{diagonal,-diagonal}};
 for(int sector=0;sector<8;++sector) {
  for(int offset:{-23,-22,22,23}) {
   const float angle=(sector*45+offset)*.01745329252f;
   const int adjacent=offset<-22?-1:offset>22?1:0;
   const auto expected=directions[(sector+adjacent+8)%8];
   f.buttons=bit(Button::LeftPadClick);
   f.left_pad={.9f*std::cos(angle),.9f*std::sin(angle)};
   const auto pressed=pad.sample(f,2000);
   f.left_pad={-f.left_pad.x,-f.left_pad.y};
   const auto held=pad.sample(f,2100);
   f.buttons=0;
   const auto released=pad.sample(f,2200);
   check(pressed.began&&pressed.active&&!pressed.turning&&
         pressed.move.x==expected.x&&pressed.move.y==expected.y&&
         !held.began&&!held.released&&held.move.x==expected.x&&held.move.y==expected.y&&
         released.released&&released.move.x==expected.x&&released.move.y==expected.y);
  }
 }
 for(bool hold_on_press:{false,true})for(bool hold_on_release:{false,true}) {
  f.left_pad={1,0};
  const auto l4=bit(Button::L4);
  f.buttons=bit(Button::LeftPadClick)|(hold_on_press?l4:0);
  const auto press=pad.sample(f,2300);
  f.buttons=hold_on_release?l4:0;
  const auto release=pad.sample(f,2400);
  check(press.began&&!press.append&&release.released&&release.append==hold_on_release);
 }
 const auto both=bit(Button::LeftPadClick)|bit(Button::RightPadClick);
 for(float direction:{-1.f,1.f}) {
  pad.reset();f.left_pad={.95f,0};f.right_pad={};f.buttons=bit(Button::LeftPadClick);
  check(pad.sample(f,3000).move.x==1);
  f.left_pad={};f.buttons=both;const auto press=pad.sample(f,3100);
  check(press.began&&press.resizing&&press.owns_right_pad&&!press.turning&&!press.move.x&&!press.width_fraction);
  f.right_pad={direction,0};
  check(!pad.sample(f,3140).width_fraction);
  f.left_pad={direction,0};
  check(!pad.sample(f,3170).width_fraction);
  f.left_pad={-.078f*direction,0};f.right_pad={.078f*direction,0};
  check(!pad.sample(f,3180).width_fraction);
  f.left_pad={-.085f*direction,0};f.right_pad={.085f*direction,0};
  check(pad.sample(f,3185).width_fraction*direction>0.f);
  f.left_pad={-.34f*direction,0};f.right_pad={.34f*direction,0};
  check(std::fabs(pad.sample(f,3190).width_fraction-.25f*direction)<.0001f);
  f.left_pad={-.60f*direction,0};f.right_pad={.60f*direction,0};
  const auto drag=pad.sample(f,3200);
  check(drag.active&&drag.resizing&&!drag.began&&drag.width_fraction==direction);
  f.buttons=bit(Button::RightPadClick);const auto release=pad.sample(f,3300);
  check(release.released&&release.resizing&&release.width_fraction==drag.width_fraction&&release.owns_right_pad);
  const auto tail=pad.sample(f,3400);check(!tail.active&&!tail.released&&tail.owns_right_pad);
  f.buttons=0;pad.sample(f,3500);
  f.left_pad={.95f,0};f.buttons=bit(Button::LeftPadClick);const auto move=pad.sample(f,3600);
  check(move.began&&!move.resizing&&move.move.x==1);
 }
 std::cout<<"PASS fine216: "<<cases<<" eight-sector, release-only, center-boundary and drag cases; held samples never reissue an order\n";
}
