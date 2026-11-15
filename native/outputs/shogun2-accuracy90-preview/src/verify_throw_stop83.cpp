// Exercise the production weapon hooks outside the game with synthetic native
// object layouts. No process injection, game input, or live battle test.
#include "identity.h"
#include <sstream>
#include "steering.h"
#include "aim_geometry.h"
#include "../vendor/include/MinHook.h"
static uintptr_t base,possessed,unit,environment;
static volatile LONG enabled=1;
static Status storage{},*state=&storage;
static bool identity_valid=true;static bool mounted_weapon=false;
static bool weapon_mounted(){return mounted_weapon;}
static bool personal_direct_projectile(uintptr_t p){return p!=0;}
static bool is_bow_projectile(uintptr_t p){return p&&*reinterpret_cast<unsigned*>(p)==3;}
static uintptr_t record;
template<class T>static T& field(uintptr_t p,unsigned off){return *reinterpret_cast<T*>(p+off);}
static bool followed(uintptr_t actor,bool){return identity_valid&&actor==possessed&&field<uintptr_t>(actor,0x214)==unit;}
static uintptr_t bow_record(uintptr_t actor_unit){return actor_unit==unit?record:0;}
static bool held83=true;static bool weapon_ready_held(){return held83;}
using NativeGet=void*(__thiscall*)(void*);
static bool posed_arrow_origin(float*){return false;}
static void pose_manual_origin(float*){}
static bool read_aim_frame(ViewRay&){return false;}
static void manual_bow_apply(const uint32_t*,uintptr_t){}
#include "equipment_policy81.h"
static volatile LONG equipment_personal_override214{};
static LONG equipment_slot81=equipment81::Ranged;
static bool equipment_crouched81(){return false;}
// This isolated fixture covers ordinary native firing. Full command-mode
// behavior is exercised with actual adapter ownership in verify_native_commands100.
static bool command_draw_permitted100(void*){return true;}
static void command_draw_started100(void*){}
static bool command_release_managed100(void*){return false;}
static void command_on_shot100(){}
// Standalone firing fixture: presentation eligibility is an external provider.
// Full native projectile presentation is covered by verify_melee_presentation137.
#include "personal_weapon_catalog.h"
static PersonalWeaponKind native_personal_kind(uintptr_t){return PersonalWeaponKind::Unsupported;}
#include "fixture_weapon_edges143.h"
#include "weapon_animation.h"


static unsigned forwarded;
struct Actor83 {unsigned char bytes[0x700]{};
 __declspec(noinline) void stop(){++forwarded;field<uintptr_t>(reinterpret_cast<uintptr_t>(this),0x474)=0;}
};
template<class T>static void* address83(T member){static_assert(sizeof(member)==4,"x86 ABI");void* p;memcpy(&p,&member,4);return p;}
int main(){try{
 Actor83 actor,other;possessed=reinterpret_cast<uintptr_t>(&actor);unit=0x100000;
 base=reinterpret_cast<uintptr_t>(VirtualAlloc(nullptr,0x400000,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));require(base!=0,"Caller fixture allocation");
 original_animation_stop82=reinterpret_cast<RangedAction>(address83(&Actor83::stop));
 // Reproduce the captured x86 CALL return address, invoking the real hook.
 auto call=base+0x3e93c3;field<unsigned char>(call,0)=0xe8;
 field<int>(call,1)=int(reinterpret_cast<uintptr_t>(&weapon_animation_stop_hook82)-(call+5));field<unsigned char>(call,5)=0xc3;
 auto reset=reinterpret_cast<RangedAction>(call);
 unsigned projectile=49;record=reinterpret_cast<uintptr_t>(&projectile);
 unsigned char asset[0xa0]{},hold_asset[0xa0]{},db[32]{};uintptr_t clips[131]{};
 auto ap=reinterpret_cast<uintptr_t>(asset),hp=reinterpret_cast<uintptr_t>(hold_asset);
 static const wchar_t path[]=L"animations/shogun_animation/battle/men/sling/sling_poise/sli_throw.anim";
 field<unsigned>(ap,8)=_countof(path)-1;field<uintptr_t>(ap,16)=reinterpret_cast<uintptr_t>(path);field<unsigned>(ap,0x64)=35;field<unsigned>(hp,0x64)=4;
 unsigned fire[2]{130,unsigned(ap)},hold[2]{124,unsigned(hp)};
 auto fp=reinterpret_cast<uintptr_t>(fire),holdp=reinterpret_cast<uintptr_t>(hold);
 clips[130]=fp;clips[124]=holdp;field<uintptr_t>(reinterpret_cast<uintptr_t>(db),0xc)=reinterpret_cast<uintptr_t>(clips);
 field<uintptr_t>(possessed,0x214)=unit;field<uintptr_t>(possessed,0x23c)=reinterpret_cast<uintptr_t>(db);
 auto arm=[&](){enabled=1;identity_valid=true;mounted_weapon=false;held83=true;own_weapon_call=0;equipment_slot81=equipment81::Ranged;
  state->mode=POSSESSED;state->release=state->stop=0;state->weapon_phase=WEAPON_DRAW;state->aim_pose=1;
  weapon_actor=possessed;weapon_clip=130;field<unsigned char>(possessed,0xf0)=1;field<unsigned>(possessed,0x1f0)=14;
  field<uintptr_t>(possessed,0x688)=0;field<uintptr_t>(possessed,0x474)=fp;field<unsigned>(possessed,0x1d8)=0;
  field<unsigned>(possessed,0x1d4)=130;field<uintptr_t>(fp,4)=ap;
  field<unsigned>(ap,0x64)=35;field<unsigned>(ap,8)=_countof(path)-1;};
 arm();reset(&actor);require(!forwarded&&field<uintptr_t>(possessed,0x474)==fp,"Captured frame-zero reset cancelled owned throw");
 for(unsigned f=1;f<35;++f){field<unsigned>(possessed,0x1d8)=f;state->weapon_phase=f<10?WEAPON_DRAW:WEAPON_FIRED;reset(&actor);}
 require(!forwarded&&field<uintptr_t>(possessed,0x474)==fp,"Protected throw lost before native completion");
 field<unsigned>(possessed,0x1d8)=35;reset(&actor);require(forwarded==1&&!field<uintptr_t>(possessed,0x474),"Native completion not forwarded");
 arm();state->weapon_phase=0;field<uintptr_t>(possessed,0x474)=holdp;field<unsigned>(possessed,0x1d4)=124;reset(&actor);require(forwarded==1,"Held native sling pose cancelled");
 for(unsigned guard=0;guard<15;++guard){arm();switch(guard){
  case 0:held83=false;break;case 1:enabled=0;break;case 2:identity_valid=false;break;
  case 3:state->mode=IDLE;break;case 4:state->release=1;break;case 5:state->stop=1;break;
  case 6:field<unsigned char>(possessed,0xf0)=0;break;case 7:mounted_weapon=true;break;
  case 8:equipment_slot81=equipment81::Melee;break;case 9:field<uintptr_t>(possessed,0x688)=1;break;
  case 10:field<unsigned>(possessed,0x1f0)=8;break;case 11:own_weapon_call=1;break;
  case 12:field<unsigned>(ap,0x64)=0;break;case 13:field<uintptr_t>(fp,4)=0;break;
  case 14:weapon_clip=758;break;}
  unsigned n=forwarded;reset(&actor);require(forwarded==n+1&&!field<uintptr_t>(possessed,0x474),"Stop guard failed to forward");
 }
 arm();unsigned n=forwarded;weapon_animation_stop_hook82(&actor,nullptr);require(forwarded==n+1,"Unrelated caller suppressed");
 arm();n=forwarded;reset(&other);require(forwarded==n+1&&field<uintptr_t>(possessed,0x474)==fp,"Other actor changed");
 VirtualFree(reinterpret_cast<void*>(base),0,MEM_RELEASE);
 std::cout<<"PASS: retained production sling sequence protects the captured reset through native frames; completion, release, death, mounted, QTE, mode, invalid asset, explicit calls and other actors forward.\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
