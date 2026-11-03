#define NOMINMAX
#include <windows.h>
#include "../../native/deck/cursor_idle185.h"
#include "../../native/deck/campaign_keys185.h"
#include <cassert>
#include <iostream>
int main(){unsigned cases=0;deck185::CursorIdle cursor;
 assert(!cursor.ready(1000,false));++cases;cursor.activity(1100);
 assert(!cursor.ready(10000,true));++cases;assert(!cursor.ready(2599,false)&&cursor.ready(2600,false));++cases;
 assert(!cursor.ready(500,false)&&!cursor.ready(1500,false)&&cursor.ready(2000,false));++cases;
 deck185::CampaignKeys keys;unsigned char current[256]{};
 const unsigned flags[]={deck183::Arts,deck183::Finance,deck183::Diplomacy,deck183::Clan,deck183::EndTurn,deck183::Encyclopedia};const unsigned scans[]={0x32,0x31,0x25,0x26,0x1c,0x57};
 for(unsigned i=0;i<6;++i){keys.clear();memset(current,0,sizeof current);current[0x11]=0x80;assert(keys.put(flags[i],1000,5)&&keys.apply(current,true,true,1010,5));assert((current[scans[i]]&0x80)&&current[0x11]==0x80);if(i==5)assert(current[0x1d]==0x80);memset(current,0,sizeof current);assert(!keys.apply(current,true,true,1020,5)&&current[scans[i]]==0);++cases;}
 keys.clear();assert(keys.put(deck183::Arts,2000,3)&&!keys.apply(current,true,true,3301,3));++cases;
 assert(keys.put(deck183::Arts,3000,3)&&!keys.apply(current,true,true,3001,4));++cases;
 assert(keys.put(deck183::Arts,3000,3)&&!keys.apply(current,true,false,3001,3)&&!keys.count);++cases;
 assert(keys.put(deck183::Arts,3000,3)&&!keys.apply(current,false,true,3001,3)&&!keys.count);++cases;
 for(unsigned i=0;i<8;++i)assert(keys.put(deck183::EndTurn,4000,3));assert(!keys.put(deck183::Clan,4000,3)&&keys.count==8);++cases;keys.clear();
 // Alternating actual press/release, no stuck/repeating long-held end-turn.
 deck183::CampaignButtons buttons;deck174::Frame f;deck174::Edges edge;f.buttons=deck174::bit(deck174::Button::Y);assert(!(buttons.sample(f,edge,1000000)&deck183::EndTurn));assert(buttons.sample(f,edge,1600000)&deck183::EndTurn);assert(!(buttons.sample(f,edge,5000000)&deck183::EndTurn));f.buttons=0;buttons.sample(f,edge,5001000);f.buttons=deck174::bit(deck174::Button::Y);assert(!(buttons.sample(f,edge,5002000)&deck183::EndTurn));++cases;
 std::cout<<"PASS "<<cases<<" usability185 cursor visibility/idle/native shortcuts/epoch/focus/bounds/held end-turn checks\n";
}
