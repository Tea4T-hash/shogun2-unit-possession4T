
#include "identity.h"
#include "steering.h"
#include "equipment_policy81.h"
#include "ammunition_protocol83.h"
#include "personal_weapon_catalog.h"
static Status storage{},*state=&storage;
static uintptr_t unit;
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
static ammo83::Status wire{},*ammunition_status83=&wire;
static SRWLOCK ammunition_publish_lock83=SRWLOCK_INIT;
static bool eligible[256]{};
static bool equipment_unit_ability81(unsigned id,bool){return id<256&&eligible[id];}
#include "ammunition_publish83.h"
struct Record {unsigned id,length,capacity;uintptr_t chars;unsigned char rest[0x80]{};};
int main(){try{
 unsigned char bytes[0x1000]{};unit=reinterpret_cast<uintptr_t>(bytes);field<float>(unit,0xf78)=20;
 state->lease_generation=19;state->actor_id=43;state->unit_id=7;
 wire.magic=0x334d4153;wire.version=83;wire.size=sizeof wire;wire.pid=GetCurrentProcessId();
 Record normal{3,5,5,reinterpret_cast<uintptr_t>(L"arrow")},fire{11,14,14,reinterpret_cast<uintptr_t>(L"flaming_arrows")},whistle{12,16,16,reinterpret_cast<uintptr_t>(L"whistling_arrows")};
 equipment81::AmmoOption options[3]{{0,reinterpret_cast<uintptr_t>(&normal),false,true},{24,reinterpret_cast<uintptr_t>(&fire),true,true},{25,reinterpret_cast<uintptr_t>(&whistle),false,false}};
 eligible[24]=true;
 publish_ammunition83(options,3,options[0].record,2,0);
 require(wire.count==3&&wire.current==0&&wire.pending==2&&!(wire.sequence&1),"Pending switch falsely became selection");
 require(wire.items[1].icon==ammo83::FireArrow&&wire.items[2].icon==ammo83::WhistleArrow,"Native ammo icon classification");
 require(wire.items[1].flags==ammo83::Available&&wire.items[2].flags==0,"Native eligibility ignored");
 publish_ammunition83(options,3,options[1].record,0,0);require(wire.current==1&&(wire.items[1].flags&ammo83::Selected),"Confirmed style not selected");
 field<float>(unit,0xf78)=0;eligible[24]=false;publish_ammunition83(options,3,options[1].record,0,2);
 require(!(wire.items[0].flags&ammo83::Available)&&wire.items[1].flags==ammo83::Selected,"Unavailable selection or empty ammo mislabeled");
 require(ammo83::valid(wire,wire.pid,19,43,7,wire.tick),"Fresh native snapshot rejected");
 require(!ammo83::valid(wire,wire.pid,20,43,7,wire.tick)&&!ammo83::valid(wire,wire.pid,19,44,7,wire.tick)&&!ammo83::valid(wire,wire.pid,19,43,7,wire.tick+1000),"Stale ownership accepted");
 publish_ammunition83(options,3,0,0,0);require(!wire.count,"Unknown active pointer invented normal selection");
 publish_ammunition83(options,1,options[0].record,0,0);require(wire.count==1&&wire.current==0,"Single ammo unavailable");
 struct Key {const wchar_t* key;ammo83::Icon icon;};Key keys[]={{L"fire_bomb_blinding",ammo83::BlindBomb},{L"rifle_suppressive",ammo83::Suppression},{L"rocket",ammo83::Rocket},{L"handmortar",ammo83::Mortar},{L"heavygunner",ammo83::HeavyShot},{L"donderbuss",ammo83::Bullet},{L"unmapped",ammo83::Unknown}};
 for(auto k:keys)require(ammo83::icon_for_key(k.key)==k.icon,"Specialty family classification");
 for(auto p:PERSONAL_WEAPONS)require(ammo83::icon_for_key(p.key)!=ammo83::Unknown,"Supported projectile missing ammo icon");
 std::cout<<"PASS: production ammo publisher confirms native pointer selection, pending state, eligibility, empty ammo, single style, stale leases and specialty icons.\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
