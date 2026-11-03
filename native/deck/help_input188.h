#pragma once
#include "input174.h"
#include "view_help186.h"
namespace deck188 {
// All game menus use the same pointer/trigger convention. This does not touch
// campaign or battle camera vectors; only modal menu inputs are normalized.
inline deck174::Frame menu_frame188(deck174::Frame frame){
 std::swap(frame.left_trigger,frame.right_trigger);frame.left_stick={};
 frame.buttons&=~(deck174::bit(deck174::Button::Up)|deck174::bit(deck174::Button::Down)|deck174::bit(deck174::Button::Left)|deck174::bit(deck174::Button::Right));return frame;
}
// Keep the usual right-pad cursor. In an open campaign panel, L1 temporarily
// shifts that same pad to scrolling; pad click remains unassigned.
inline deck174::Frame campaign_menu_frame189(deck174::Frame frame,bool modal){
 frame=menu_frame188(frame);frame.buttons&=~deck174::bit(deck174::Button::A); // Campaign A is Space, never a pointer click.
 if(modal&&(frame.buttons&deck174::bit(deck174::Button::L1))){
  frame.left_pad=frame.right_pad;
  frame.buttons&=~deck174::bit(deck174::Button::LeftPadTouch);
  if(frame.buttons&deck174::bit(deck174::Button::RightPadTouch))frame.buttons|=deck174::bit(deck174::Button::LeftPadTouch);
  frame.buttons&=~deck174::bit(deck174::Button::RightPadTouch);
 }
 return frame;
}
struct HelpInput {
 deck186::ViewGesture view;bool open{};unsigned page{};
 void reset(){view.reset();open=false;page=0;}
 struct Result {bool blocked{},toggle_hud{};};
 Result sample(const deck174::Frame& frame,const deck174::Edges& edges,uint64_t now,bool can_hide){
  using deck174::Button;auto held=[&](Button b){return(frame.buttons&deck174::bit(b))!=0;};auto down=[&](Button b){return(edges.down&deck174::bit(b))!=0;};bool was=open;
  auto event=view.sample(held(Button::View),now,can_hide);
  if(event==deck186::ViewEvent::Controls){open=!open;page=0;}
  if(was&&down(Button::B))open=false;
  if(open&&(down(Button::L1)||down(Button::R1)))page=1-page;
  // A View tap closes the guide without falling through to HUD toggling.
  if(was&&event==deck186::ViewEvent::HideHud)open=false;
  return{was||open,!was&&!open&&event==deck186::ViewEvent::HideHud};
 }
};
}
