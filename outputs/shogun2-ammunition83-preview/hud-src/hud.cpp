// Read-only authentic HUD candidate. Never writes game memory or handles gameplay input.
#include "../../shogun2-possession-cleanup-v46/src/identity.h"
#include "../../shogun2-possession-cleanup-v46/src/protocol.h"
#include "../../shogun2-possession-cleanup-v46/src/actor_types152.h"
#include "../../shogun2-possession-cleanup-v46/src/game_window105.h"
#include "../../shogun2-possession-cleanup-v46/src/overlay_surface106.h"
#include "hud_renderer.h"
#include "../../../deck/glyphs181.h"
#include "wheel_surface121.h"
#include "../../../deck/command_display184.h"
#include "../../../common/card_pack217.h"
#include "../../../common/unit_activity215.h"
#include "hud_surface100.h"
#include "accuracy_protocol90.h"
#include "../../shogun2-accuracy90-preview/src/climb_protocol97.h"
#include "../src/equipment_protocol81.h"
#include "../../shogun2-possession-cleanup-v46/src/hud_visibility148.h"
#ifndef S2UP_LIFE_STATS174
#define S2UP_LIFE_STATS174
// Kill/death statistics shared by the possession engine (deaths), the QTE runtime (kills,
// rounds, battle totals) and the HUD (display). Identical copy in each component (kit v13).
// The single statistics file is replaced in place every time it is written (never grows).
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string>
namespace life173 {
constexpr uint32_t Magic=0x3346494C,Version=174;
struct Shared {
    uint32_t magic,version,size,pid;
    volatile LONG deaths;            // possessed-soldier deaths since the game started (engine)
    volatile LONG last_death_lease;  // possession lease of the latest death (engine)
    volatile LONG sequence;          // QTE writer: odd while the fields below change
    uint32_t in_battle,battle_kills,battle_deaths;      // this battle (QTE)
    uint32_t life_lease,life_kills,life_rounds;         // current possession (QTE)
    uint32_t key_lease,key_actor,key_unit;              // last possessed soldier (overlay key)
    uint64_t tick;                                      // QTE heartbeat
};
inline void mapping_name(wchar_t* out,DWORD pid){swprintf_s(out,96,L"Local\\S2UPLife174-%lu",pid);}
inline bool stats_path(std::wstring& out){
    wchar_t base[32768]{};DWORD n=GetEnvironmentVariableW(L"APPDATA",base,32768);
    if(!n||n>=32768)return false;
    out=std::wstring(base)+L"\\The Creative Assembly\\Shogun2";
    DWORD attr=GetFileAttributesW(out.c_str());
    if(attr==INVALID_FILE_ATTRIBUTES||!(attr&FILE_ATTRIBUTE_DIRECTORY)||(attr&FILE_ATTRIBUTE_REPARSE_POINT))return false;
    out+=L"\\possession_battle_stats.txt";return true;
}
inline bool write_stats(const char* session,unsigned kills,unsigned deaths){
    std::wstring path;if(!stats_path(path))return false;
    char text[160];int n=sprintf_s(text,"S2UP_STATS 1\r\nsession %s\r\nkills %u\r\ndeaths %u\r\n",session,kills,deaths);
    if(n<=0)return false;
    HANDLE f=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(f==INVALID_HANDLE_VALUE)return false;
    DWORD written=0;bool ok=WriteFile(f,text,DWORD(n),&written,nullptr)&&written==DWORD(n);
    CloseHandle(f);return ok;
}
}
#endif
static_assert(sizeof(void*)==4,"Build x86 for the game's telemetry");
static card217::Catalog cards217;
static DWORD pid;static uintptr_t base;static HANDLE process;static Status* shared;
static EquipmentStatus81* equipment_shared;
static ability141::Status* abilities_shared141;
static ammo83::Status* ammunition_shared83;
static accuracy90::Status* accuracy_shared90;
static climb97::Status* climb_shared97;
static command100::Status* commands_shared100;static HANDLE commands_map100;static uint64_t commands_retry100;
static HudSurface100 hud_surface100;
static HWND game,overlay;static bool visible;static hud80::View display;static hud80::Assets assets;
static HANDLE visibility_mapping148;static const hudvisibility148::Shared* visibility_shared148;
static uint64_t visibility_retry148;
static overlay106::Publisher wheel_pixels117;
static bool cropped_wheel117,wheel_active121,fast_tick121;
static overlay106::Publisher pixels106;static overlay106::Key display_key106;
template<class T>T read(uintptr_t address){T v{};SIZE_T n=0;
    require(ReadProcessMemory(process,reinterpret_cast<void*>(address),&v,sizeof v,&n)&&n==sizeof v,"Native HUD read failed");return v;}
static bool snapshot(Status& s){for(int i=0;i<4;++i){LONG seq=shared->sequence;if(seq&1)continue;
    MemoryBarrier();memcpy(&s,shared,sizeof s);MemoryBarrier();if(seq==shared->sequence)return true;}return false;}
static std::wstring unit_key(uintptr_t unit,uintptr_t& definition){
    // Native unit:type getter1c1600 + string access95790, verified from assembly.
    definition=read<uintptr_t>(unit+0x1c);uintptr_t object=read<uintptr_t>(definition),chars=read<uintptr_t>(object+8);
    require(chars>0x10000,"Missing unit key");std::wstring out;
    for(unsigned i=0;i<160;++i){wchar_t c=read<wchar_t>(chars+i*2);if(!c){require(!out.empty(),"Empty unit key");
        require(read<uintptr_t>(unit+0x1c)==definition&&read<uintptr_t>(definition)==object&&read<uintptr_t>(object+8)==chars,"Unit key changed");return out;}
        require(c>=32,"Unit key control character");out+=c;}
    throw std::runtime_error("Unterminated unit key");
}
// UniString fields are inline shared-data pointers, not C-string pointers.
// Only the two documented native artwork fields are optional; live identity
// and ownership checks remain mandatory and independent of card availability.
static std::wstring card_alias217(uintptr_t definition,unsigned offset){
    try{auto object=read<uintptr_t>(definition+offset),chars=read<uintptr_t>(object+8);if(chars<0x10000)return {};std::wstring value;
        for(unsigned i=0;i<512;++i){auto c=read<wchar_t>(chars+i*2);if(!c){if(read<uintptr_t>(definition+offset)!=object||read<uintptr_t>(object+8)!=chars)return {};return value;}if(c<32)return {};value+=c;}
    }catch(...){}return {};
}
static std::wstring army_clan217(uintptr_t army){
    require(army&&read<uintptr_t>(army)==base+0x15bc860,"Unexpected battle army");
    uintptr_t object=read<uintptr_t>(army+0xf4),chars=read<uintptr_t>(object+8);
    require(chars>0x10000,"Missing clan key");
    std::wstring key;
    for(unsigned i=0;i<160;++i){
        wchar_t c=read<wchar_t>(chars+i*2);
        if(!c){
            require(!key.empty()&&read<uintptr_t>(army+0xf4)==object&&read<uintptr_t>(object+8)==chars,"Clan identity changed");
            return key;
        }
        require(c>=32,"Clan key control character");key+=c;
    }
    throw std::runtime_error("Unterminated clan key");
}
static const hud80::FactionArt* faction_owner(uintptr_t unit,uintptr_t& army,std::wstring& native_key){
    army=read<uintptr_t>(unit+0x2a4);
    require(read<uintptr_t>(army)==base+0x15bc860,"Unexpected battle army");
    unsigned count=read<unsigned>(army+0x114);uintptr_t members=read<uintptr_t>(army+0x118);
    require(count>0&&count<=256,"Invalid army roster");bool found=false;
    for(unsigned i=0;i<count;++i)if(read<uintptr_t>(members+i*4)==unit){found=true;break;}
    require(found,"Unit not owned by army");
    native_key=army_clan217(army);
    require(read<uintptr_t>(unit+0x2a4)==army,"Clan identity changed");
    return hud80::faction_art(native_key.c_str());
}
static std::shared_ptr<card217::Picture> unit_card(uintptr_t definition,const std::wstring& key,const std::wstring& clan){
    return cards217.card(key,clan,card_alias217(definition,0x54),card_alias217(definition,0x48));
}
static void configure_cards(){
    wchar_t executable[32768];DWORD length=32768;
    if(QueryFullProcessImageNameW(process,0,executable,&length))
        cards217.configure(std::filesystem::path(std::wstring(executable,length)).parent_path());
}
// Same phase/alliance/army traversal as qte::battle_clock (native 321880 ->
// 17a420). Warm only human armies, a few portraits per HUD tick, in the HUD
// process. Leave the lazy lookup available for reinforcements and late joins.
struct CardPreload {
    uintptr_t phase{},human_army{};
    std::vector<std::pair<uintptr_t,uintptr_t>> units;
    size_t next{};
};
static CardPreload card_preload;
static void preload_cards(){
    try{
        auto env=read<uintptr_t>(read<uintptr_t>(base+0x1bc8180)+0x110);
        auto common=read<uintptr_t>(env+8),phase=read<uintptr_t>(common+0xb4);
        unsigned id=read<unsigned>(phase+0x14);
        if(id!=3&&(id<5||id>8)){card_preload={};return;}
        if(card_preload.phase!=phase||card_preload.units.empty()){
            CardPreload next;
            next.phase=phase;
            bool several_human_armies=false;
            unsigned count=read<unsigned>(phase+0x88);
            auto alliances=read<uintptr_t>(phase+0x8c);
            if(count>32)return;
            for(unsigned i=0;i<count&&next.units.size()<128;++i){
                auto alliance=read<uintptr_t>(alliances+i*4);
                unsigned n=read<unsigned>(alliance+0x20);
                auto armies=read<uintptr_t>(alliance+0x24);
                if(n>32)return;
                for(unsigned j=0;j<n&&next.units.size()<128;++j){
                    auto army=read<uintptr_t>(armies+j*4);
                    if(read<uintptr_t>(army)!=base+0x15bc860||read<unsigned char>(army+0x270)!=1)continue;
                    if(next.human_army&&next.human_army!=army)several_human_armies=true;
                    next.human_army=army;
                    unsigned units=read<unsigned>(army+0x114);
                    auto members=read<uintptr_t>(army+0x118);
                    if(units>256)return;
                    for(unsigned k=0;k<units&&next.units.size()<128;++k){
                        auto unit=read<uintptr_t>(members+k*4);
                        if(unit)next.units.emplace_back(unit,army);
                    }
                    if(read<unsigned>(army+0x114)!=units||read<uintptr_t>(army+0x118)!=members)return;
                }
            }
            if(read<uintptr_t>(common+0xb4)!=phase||read<unsigned>(phase+0x14)!=id)return;
            if(several_human_armies)next.human_army=0;
            card_preload=std::move(next);
        }
        for(unsigned i=0;i<2&&card_preload.next<card_preload.units.size();++i){
            auto [unit,expected_army]=card_preload.units[card_preload.next++];
            try{
                if(read<uintptr_t>(unit+0x2a4)!=expected_army)continue;
                uintptr_t definition=0,army=0;
                auto key=unit_key(unit,definition);
                std::wstring clan;
                faction_owner(unit,army,clan);
                if(army==expected_army&&read<uintptr_t>(common+0xb4)==phase)
                    {unit_card(definition,key,clan);cards217.crest(clan);}
            }catch(...){} // Artwork can never affect possession or the RTS roster.
        }
    }catch(...){card_preload={};}
}
static bool measure(){deck181::Sample controller;if(deck181::read_sample(pid,controller)&&controller.context==unsigned(deck174::Context::Menu))return false;
    Status s{};if(!snapshot(s)||s.magic!=POC_MAGIC||s.version!=46||s.size!=sizeof s||s.pid!=pid)return false;
    uint64_t now=GetTickCount64();if(!s.ready||s.stop||s.release||s.mode!=POSSESSED||now<s.heartbeat||now-s.heartbeat>1000)return false;
    uintptr_t env=read<uintptr_t>(read<uintptr_t>(base+0x1bc8180)+0x110);
    uintptr_t common=read<uintptr_t>(env+8),phase=read<uintptr_t>(common+0xb4);
    unsigned phase_id=read<unsigned>(phase+0x14);if(phase_id<5||phase_id>8)return false;
    uintptr_t manager=read<uintptr_t>(env+0x281fc),camera=read<uintptr_t>(manager+0x8e0);
    if(read<uintptr_t>(camera)!=base+0x15d22e8)return false;
    uintptr_t actor=read<uintptr_t>(camera+0x188);
    const uintptr_t actor_table=read<uintptr_t>(actor);
    if(!actor152::human(base,actor_table)||read<unsigned>(actor+0x1c8)!=s.actor_id)return false;
    uintptr_t unit=read<uintptr_t>(actor+0x214);
    if(read<unsigned>(unit+4)!=s.unit_id||read<unsigned char>(actor+0xf0)!=1)return false;
    uintptr_t modal215=read<uintptr_t>(base+0x1bc73e4);
    if(modal215){unsigned first=read<unsigned>(modal215+0x30),second=read<unsigned>(modal215+0x3c);if(first<=32&&second<=32&&(first||second))return false;}
    deck181::presentation_pid()=pid;
    hud80::View d{};d.controller_ui=hudlayout108::deck_ui192;
    if(!visibility_shared148&&now>=visibility_retry148){
        visibility_retry148=now+1000;wchar_t name[96];hudvisibility148::name(name,pid);
        visibility_mapping148=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(visibility_mapping148)visibility_shared148=static_cast<const hudvisibility148::Shared*>(MapViewOfFile(visibility_mapping148,FILE_MAP_READ,0,0,sizeof(hudvisibility148::Shared)));
        if(visibility_mapping148&&!visibility_shared148){CloseHandle(visibility_mapping148);visibility_mapping148=nullptr;}
    }
    hudvisibility148::read(visibility_shared148,pid,d.hidden);
    d.fatigue=read<unsigned char>(unit+0xd3c)?0:read<unsigned>(unit+0xd34);d.morale=read<unsigned>(unit+0xce8);
    unsigned routing=read<unsigned>(unit+0xcec);d.routing=read<unsigned char>(phase+0x2778)!=0&&(routing==2||routing==3);
    d.men=read<unsigned>(unit+0x2bc);d.initial=read<unsigned>(unit+0xcbc);
    d.melee_mode=read<unsigned char>(unit+0xead)!=0;
    uintptr_t definition=0;d.native_unit_key=unit_key(unit,definition);d.unit=hud80::unit_art(d.native_unit_key.c_str());
    // Stock presentation metadata is optional. A custom unit's live identity,
    // ownership and stats are still validated below, independently of artwork.
    uintptr_t army=0;std::wstring clan217;d.owner=faction_owner(unit,army,clan217);
    try{d.native_card217=unit_card(definition,d.native_unit_key,clan217);}catch(...){}
    if(d.owner){d.faction=d.owner->name;d.mon=d.owner->mon;d.boshin=d.owner->boshin;}
    try{
        d.native_unit_title=cards217.title(L"units_on_screen_name_",d.native_unit_key);
        const auto title=cards217.title(L"factions_screen_name_",clan217);
        if(!title.empty())d.faction=title;
        d.native_crest=cards217.crest(clan217);
    }catch(...){}
    d.ranged=read<unsigned>(unit+0xf7c)!=0;d.bow=s.pose_weapon_record==3;
    d.aim_pose=s.aim_pose;d.weapon_phase=s.weapon_phase;d.aiming=s.aiming!=0;d.following=s.follow!=0;
    if(!commands_shared100&&now>=commands_retry100){
        commands_retry100=now+1000;wchar_t name[120];swprintf_s(name,L"Local\\Shogun2PossessionUnitCommands100-%lu",pid);
        commands_map100=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(commands_map100)commands_shared100=static_cast<command100::Status*>(MapViewOfFile(commands_map100,FILE_MAP_READ,0,0,sizeof(command100::Status)));
        if(commands_map100&&!commands_shared100){CloseHandle(commands_map100);commands_map100=nullptr;}
    }
    static deck184::CommandDisplay command_display184;
    d.commands_known=command_display184.read([&](command100::Status& commands184){
      auto result=commands_shared100?deck184::Read::Contended:deck184::Read::Missing;
      if(commands_shared100)for(unsigned retry=0;retry<4;++retry){
        LONG seq=commands_shared100->sequence;if(seq&1)continue;
        MemoryBarrier();memcpy(&commands184,commands_shared100,sizeof commands184);MemoryBarrier();
        if(seq!=commands_shared100->sequence)continue;
        result=deck184::Read::Stable;break;
      }
      return result;
    },[](){return GetTickCount64();},pid,s.lease_generation,s.actor_id,s.unit_id,d.commands);
    if(equipment_shared){for(unsigned retry=0;retry<4;++retry){LONG seq=equipment_shared->sequence;if(seq&1)continue;
        EquipmentStatus81 e{};MemoryBarrier();memcpy(&e,equipment_shared,sizeof e);MemoryBarrier();if(seq!=equipment_shared->sequence)continue;now=GetTickCount64();
        if(e.magic==EQUIPMENT_MAGIC81&&e.version==EQUIPMENT_VERSION&&e.size==sizeof e&&e.pid==pid&&e.lease==s.lease_generation&&
           e.actor==s.actor_id&&e.unit==s.unit_id&&now>=e.tick&&now-e.tick<1000&&e.slot>=1&&e.slot<=2){
            d.equipment_known=true;d.kind=e.kind<=3?e.kind:0;d.equipment_error=e.error;d.has_ranged=d.kind!=0;d.slot=e.slot;d.ranged=d.has_ranged&&e.slot==2;d.bow=e.kind==1;
            d.launcher_sight=e.launcher_sight;d.arrow_camera=e.arrow_camera!=0;
            d.styles=e.styles;d.crouch_supported=e.crouch_supported!=0;d.crouched=e.crouched!=0;d.pending=e.pending!=0;
            if(d.bow&&d.slot==2&&std::isfinite(e.bow_power)&&e.bow_power>=.35f&&e.bow_power<=1.f&&now-e.tick<150)
                d.bow_power=e.bow_power;
            e.ammunition[63]=0;d.ammunition=e.ammunition;
        }break;
    }}
    if(abilities_shared141)for(unsigned retry=0;retry<4;++retry){LONG seq=abilities_shared141->sequence;if(seq&1)continue;
        ability141::Status a{};MemoryBarrier();memcpy(&a,abilities_shared141,sizeof a);MemoryBarrier();if(seq!=abilities_shared141->sequence)continue;now=GetTickCount64();
        if(ability141::valid(a,pid,s.lease_generation,s.actor_id,s.unit_id,now)){d.abilities_known=true;d.abilities=a;}break;
    }
    if(ammunition_shared83){for(unsigned retry=0;retry<4;++retry){LONG seq=ammunition_shared83->sequence;if(seq&1)continue;
        ammo83::Status a{};MemoryBarrier();memcpy(&a,ammunition_shared83,sizeof a);MemoryBarrier();if(seq!=ammunition_shared83->sequence)continue;now=GetTickCount64();
        if(ammo83::valid(a,pid,s.lease_generation,s.actor_id,s.unit_id,now)){d.ammunition_known=true;d.ammunition_state=a;}break;
    }}
    if(accuracy_shared90){for(unsigned retry=0;retry<4;++retry){LONG seq=accuracy_shared90->sequence;if(seq&1)continue;
        accuracy90::Status a{};MemoryBarrier();memcpy(&a,accuracy_shared90,sizeof a);MemoryBarrier();if(seq!=accuracy_shared90->sequence)continue;now=GetTickCount64();
        if(accuracy90::valid(a,pid,s.lease_generation,s.actor_id,s.unit_id,now,read<unsigned>(common+0x50))){d.accuracy_known=true;d.cone_half_angle=a.half_angle;d.projection_y=a.projection_y;}break;
    }}
    if(climb_shared97){for(unsigned retry=0;retry<4;++retry){LONG seq=climb_shared97->sequence;if(seq&1)continue;
        climb97::Status c{};MemoryBarrier();memcpy(&c,climb_shared97,sizeof c);MemoryBarrier();if(seq!=climb_shared97->sequence)continue;now=GetTickCount64();
        if(climb97::valid(c,pid,s.lease_generation,s.actor_id,s.unit_id,now))d.climb_mode=c.mode;break;
    }}
    // Native Unit.ammo_left/starting_ammo(1c1ca0/1c1d10) use these fields.
    d.ammo_capacity=read<unsigned>(unit+0xf7c);
    struct ActivityCache215{unsigned lease{},actor{},unit{};uintptr_t address{};uint64_t next{};bool known{};unitactivity215::Snapshot value{};};
    static ActivityCache215 activity215;
    if(activity215.lease!=s.lease_generation||activity215.actor!=s.actor_id||activity215.unit!=s.unit_id||activity215.address!=unit||now>=activity215.next){
        activity215={s.lease_generation,s.actor_id,s.unit_id,unit,now+300};
        activity215.known=unitactivity215::sample([&](uintptr_t at,void* out,size_t bytes){SIZE_T got{};return ReadProcessMemory(process,reinterpret_cast<void*>(at),out,bytes,&got)&&got==bytes;},base,env,unit,s.unit_id,actor,activity215.value);
    }
    d.unit_activity_known=activity215.known;if(d.unit_activity_known)d.unit_activity=activity215.value.activity;
    d.ammo=read<float>(unit+0xf78);d.reload=read<float>(actor+0x390);
    d.speed=std::hypot(read<float>(actor+0x8c),read<float>(actor+0x94));
    if(d.fatigue>5||d.morale>7||!d.initial||d.initial>100000||d.men>d.initial||
        !std::isfinite(d.ammo)||d.ammo<0||!std::isfinite(d.reload)||!std::isfinite(d.speed))return false;
    if(read<uintptr_t>(manager+0x8e0)!=camera||read<uintptr_t>(camera+0x188)!=actor||read<uintptr_t>(actor)!=actor_table||
        read<unsigned char>(actor+0xf0)!=1||read<unsigned>(actor+0x1c8)!=s.actor_id||
        read<uintptr_t>(actor+0x214)!=unit||read<uintptr_t>(unit+0x1c)!=definition||read<uintptr_t>(unit+0x2a4)!=army||read<unsigned>(unit+4)!=s.unit_id)return false;
    Status final{};if(!snapshot(final)||final.lease_generation!=s.lease_generation||final.actor_id!=s.actor_id||final.unit_id!=s.unit_id||final.mode!=POSSESSED)return false;
    display=d;display_key106={s.lease_generation,s.actor_id,s.unit_id};return true;
}

// ---- Kit v13: kill/death panels -----------------------------------------------------------------
// Top left, only while possessing: kills and rounds of combat this life, then this battle's
// total kills, deaths and K/D ratio (kit v14).
// Positions are fractions of the screen; move them here if they cover any game UI.
constexpr float StatsTop174=.085f,StatsSide174=.012f;
static life173::Shared* life_shared174;static HANDLE life_map174;static uint64_t life_retry174;
static bool life_read174(life173::Shared& out){
    uint64_t now=GetTickCount64();
    if(!life_shared174&&now>=life_retry174){life_retry174=now+1000;wchar_t name[96];life173::mapping_name(name,pid);
        HANDLE map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(map){auto v=static_cast<life173::Shared*>(MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(life173::Shared)));
            if(v&&v->magic==life173::Magic&&v->version==life173::Version&&v->size==sizeof(life173::Shared)&&v->pid==pid){life_shared174=v;life_map174=map;}
            else{if(v)UnmapViewOfFile(v);CloseHandle(map);}}}
    if(!life_shared174)return false;
    for(int i=0;i<4;++i){LONG q=life_shared174->sequence;if(q&1)continue;MemoryBarrier();memcpy(&out,life_shared174,sizeof out);MemoryBarrier();
        if(q==life_shared174->sequence)return now>=out.tick&&now-out.tick<1500;}
    return false;
}
static void stats_panel174(Gdiplus::Graphics& g,float x,float y,float height,const wchar_t* const* lines,unsigned count){
    using namespace Gdiplus;
    const float font=std::max(13.f,height/56.f),pad=font*.55f,step=font*1.25f;
    FontFamily family(L"Arial");Font bold(&family,font,FontStyleBold,UnitPixel);
    float w=0;for(unsigned i=0;i<count;++i){RectF m;g.MeasureString(lines[i],-1,&bold,PointF(0,0),&m);w=std::max(w,m.Width);}
    const RectF box(x,y,w+pad*2,step*float(count)+pad);
    SolidBrush back(Color(185,24,18,12));g.FillRectangle(&back,box);
    Pen gold(Color(235,214,176,72),2.f);g.DrawRectangle(&gold,box);
    SolidBrush shadow(Color(220,10,8,6)),text(Color(255,255,246,224));
    for(unsigned i=0;i<count;++i){const float ty=box.Y+pad*.5f+step*float(i);
        g.DrawString(lines[i],-1,&bold,PointF(box.X+pad+1,ty+1),&shadow);g.DrawString(lines[i],-1,&bold,PointF(box.X+pad,ty),&text);}
}
static void stats174(Gdiplus::Graphics& g,float width,float height,const life173::Shared& l,bool possessed,unsigned lease){
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
    // Kit v14: one panel, top left, only while possessing (the top-right panel covered the map).
    if(!possessed)return;
    const float y=height*StatsTop174,side=width*StatsSide174;
    const bool same=l.life_lease==lease;wchar_t a[64]{},b[64]{},c[64]{},d[64]{};
    swprintf_s(a,L"Kills this life: %u",same?l.life_kills:0u);swprintf_s(b,L"Rounds of combat this life: %u",same?l.life_rounds:0u);
    unsigned count=2;
    if(l.in_battle){
        swprintf_s(c,L"Total kills: %u  |  Deaths: %u",l.battle_kills,l.battle_deaths);
        if(l.battle_deaths)swprintf_s(d,L"K/D: %.2f",double(l.battle_kills)/double(l.battle_deaths));else swprintf_s(d,L"K/D: %u.00",l.battle_kills);
        count=4;
    }
    const wchar_t* lines[4]={a,b,c,d};stats_panel174(g,side,y,height,lines,count);
}
// ---- Kit v19: possession gate display ----------------------------------------------------------
// While possession pre-loads after the first F9: top-left "Pre-load" with a ring loading bar at the
// mouse cursor, then "Done"; both vanish when possession is ready. Naval: a short notice instead.
#ifndef S2UP_GATE176
#define S2UP_GATE176
namespace gate176 {
constexpr uint32_t Magic=0x47505553,Version=176;
enum Stage:uint32_t{Off=0,Preload=1,Done=2,Ready=3,Naval=4};
struct Shared{uint32_t magic,version,size,pid;volatile LONG sequence;uint32_t stage;float progress;uint32_t reserved;uint64_t stage_tick,tick;};
inline void name(wchar_t* out,DWORD pid){swprintf_s(out,96,L"Local\\S2UPGate176-%lu",pid);}
constexpr uint64_t WarmupMs=2500,DoneMs=1500,NavalMs=2500,BattleLostMs=3000;
constexpr unsigned DisplayLease=0xFFFFFFFFu;
}
#endif
static gate176::Shared* gate_view176;static HANDLE gate_map176;static uint64_t gate_retry176;static bool gate_fast176;
static bool gate_read176(gate176::Shared& out){
    const uint64_t now=GetTickCount64();
    if(!gate_view176&&now>=gate_retry176){gate_retry176=now+1000;wchar_t name[96];gate176::name(name,pid);
        HANDLE map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(map){auto v=static_cast<gate176::Shared*>(MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(gate176::Shared)));
            if(v&&v->magic==gate176::Magic&&v->version==gate176::Version&&v->size==sizeof(gate176::Shared)&&v->pid==pid){gate_view176=v;gate_map176=map;}
            else{if(v)UnmapViewOfFile(v);CloseHandle(map);}}}
    if(!gate_view176)return false;
    for(int i=0;i<4;++i){LONG q=gate_view176->sequence;if(q&1)continue;MemoryBarrier();memcpy(&out,gate_view176,sizeof out);MemoryBarrier();
        if(q==gate_view176->sequence)return now>=out.tick&&now-out.tick<1000;}
    return false;
}
static bool gate_paint176(const gate176::Shared& gs){
    using namespace Gdiplus;
    RECT client{};POINT pos{};if(!gamewindow105::client(game,client,pos))return false;int width=client.right,height=client.bottom;
    if(!hud_surface100.ensure(width,height))return false;
    auto screen=hud_surface100.screen(),dc=hud_surface100.memory();auto pixels=hud_surface100.pixels();memset(pixels,0,size_t(width)*height*4);
    {Bitmap surface(width,height,width*4,PixelFormat32bppPARGB,static_cast<BYTE*>(pixels));Graphics g(&surface);
     g.SetSmoothingMode(SmoothingModeAntiAlias);g.SetTextRenderingHint(TextRenderingHintAntiAlias);
     const wchar_t* text=gs.stage==gate176::Preload?L"Pre-load":gs.stage==gate176::Done?L"Done":L"Possession unavailable at sea";
     const wchar_t* lines[1]={text};stats_panel174(g,float(width)*StatsSide174,float(height)*StatsTop174,float(height),lines,1);
     POINT cursor{};
     if(gs.stage!=gate176::Naval&&GetCursorPos(&cursor)){
        const float x=float(cursor.x-pos.x),y=float(cursor.y-pos.y),r=std::max(16.f,float(height)/40.f),w=r*.32f;
        const float progress=std::min(1.f,std::max(0.f,gs.progress));
        Pen back(Color(170,24,18,12),w);g.DrawEllipse(&back,x-r,y-r,r*2,r*2);
        if(progress>0){Pen gold(Color(240,214,176,72),w);g.DrawArc(&gold,x-r,y-r,r*2,r*2,-90.f,360.f*progress);}
     }}
    const overlay106::Key key{gate176::DisplayLease,0,0};
    if(gs.stage!=gate176::Naval&&pixels106.open(pid,overlay106::HUD)){
        overlay106::Frame f{};f.key=key;f.active=3;f.width=f.view_width=width;f.height=f.view_height=height;f.rect_count=1;f.rect[0]={0,0,width,height};
        if(pixels106.publish(f,pixels)){overlay106::Ack ack;if(pixels106.ack(ack)&&ack.number&&overlay106::same(ack.key,key)){ShowWindow(overlay,SW_HIDE);return true;}}
    }
    POINT origin{};SIZE size{width,height};BLENDFUNCTION alpha{AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    bool ok=UpdateLayeredWindow(overlay,screen,&pos,&size,dc,&origin,0,&alpha,ULW_ALPHA)!=0;
    if(ok)SetWindowPos(overlay,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);return ok;
}
static bool gate_update176(bool focused){
    gate176::Shared gs{};const bool show=gate_read176(gs)&&(gs.stage==gate176::Preload||gs.stage==gate176::Done||
        (gs.stage==gate176::Naval&&GetTickCount64()-gs.stage_tick<gate176::NavalMs));
    if(show!=gate_fast176){gate_fast176=show;SetTimer(overlay,1,show?50:150,nullptr);} // smooth ring while shown
    if(!show)return false;
    if(!focused||!IsWindow(game)||IsIconic(game)||!IsWindowVisible(game)){pixels106.hide();if(visible)ShowWindow(overlay,SW_HIDE);visible=false;return true;}
    visible=gate_paint176(gs);if(!visible){pixels106.hide();ShowWindow(overlay,SW_HIDE);}
    return true;
}
static bool paint(){
    RECT client{};POINT pos{};if(!gamewindow105::client(game,client,pos))return false;int width=client.right,height=client.bottom;
    if(!hud_surface100.ensure(width,height))return false;
    auto screen=hud_surface100.screen(),dc=hud_surface100.memory();auto pixels=hud_surface100.pixels();memset(pixels,0,size_t(width)*height*4);
    {Gdiplus::Bitmap surface(width,height,width*4,PixelFormat32bppPARGB,static_cast<BYTE*>(pixels));Gdiplus::Graphics g(&surface);hud80::render(g,assets,float(width),float(height),display,nullptr,false,!cropped_wheel117);
     life173::Shared l{};if(life_read174(l))stats174(g,float(width),float(height),l,true,display_key106.lease);} // kit v13 stats panel
    if(pixels106.open(pid,overlay106::HUD)){
        overlay106::Frame f{};f.key=display_key106;f.width=f.view_width=width;f.height=f.view_height=height;f.rect_count=1;f.rect[0]={0,0,width,height};
        if(pixels106.publish(f,pixels)){overlay106::Ack ack;if(pixels106.ack(ack)){ShowWindow(overlay,SW_HIDE);return ack.number&&overlay106::same(ack.key,display_key106);}}
    }
    POINT origin{};SIZE size{width,height};BLENDFUNCTION alpha{AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    bool ok=UpdateLayeredWindow(overlay,screen,&pos,&size,dc,&origin,0,&alpha,ULW_ALPHA)!=0;
    if(ok)SetWindowPos(overlay,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);return ok;
}
#include "../../../deck/help_rts186.h"
static void update(){
    game=gamewindow105::refresh(game,pid);
    {DWORD gfg=0;GetWindowThreadProcessId(GetForegroundWindow(),&gfg);if(gate_update176(gfg==pid))return;} // kit v19
    bool ok=false;try{ok=measure();}catch(...){ok=false;}DWORD fg=0;GetWindowThreadProcessId(GetForegroundWindow(),&fg);
    bool active=ok&&IsWindow(game)&&!IsIconic(game)&&IsWindowVisible(game)&&fg==pid;
    if(!active){preload_cards();try{if(fg==pid&&(commander_rts_hud76()||deck_header186())){visible=false;return;}}catch(...){}pixels106.hide();if(visible)ShowWindow(overlay,SW_HIDE);visible=false;return;}
    visible=paint();if(!visible)ShowWindow(overlay,SW_HIDE);}
// The main HUD keeps its 150 ms refresh. A dirty wheel is a separate
// cropped texture, so stick motion never forces a full-screen HUD upload.
struct WheelInput117 {
    unsigned flags{},lease{};
    int sector{-1};
    commander76::WheelPointer pointer{};
    commander76::Display info{};
    unsigned flashes{};
    bool open()const{return flags!=0;}
    bool same(const WheelInput117& other)const{
        return flags==other.flags&&lease==other.lease&&sector==other.sector&&flashes==other.flashes&&
            info.flags==other.info.flags&&info.targets==other.info.targets&&info.ai==other.info.ai&&
            info.orders.hold_pace==other.info.orders.hold_pace&&
            std::abs(pointer.x-other.pointer.x)<.001f&&std::abs(pointer.y-other.pointer.y)<.001f;
    }
};
static WheelInput117 wheel_input117(){
    DWORD foreground{};
    GetWindowThreadProcessId(GetForegroundWindow(),&foreground);
    if(foreground!=pid)return {};
    commander76::Display wheel;
    const bool valid=commander76::read_display(pid,wheel);
    const auto now=GetTickCount64();
    unsigned flashes=0;
    for(unsigned i=0;i<4;++i)if(wheel.orders.flash(i,now))flashes|=1u<<i;
    if(valid&&(wheel.flags&(commander76::PcWheel|commander76::DeckWheel)))
        return {wheel.flags,wheel.lease,wheel.sector,wheel.pointer,wheel,flashes};
    if(!hudlayout108::deck_ui192)return {};
    deck181::Sample sample;
    if(!deck181::read_sample(pid,sample)||!sample.lease||
       sample.context!=unsigned(deck174::Context::Possession))return {};
    auto held=[&](unsigned key){return(sample.keys[key/32]&(uint32_t(1)<<(key%32)))!=0;};
    if(!held(deck183::WheelHeld))return {};
    const unsigned sectors[]={deck183::WheelUp,deck183::WheelDown,deck183::WheelLeft,deck183::WheelRight};
    int sector=-1;
    for(unsigned i=0;i<4;++i)if(held(sectors[i]))sector=int(i);
    if(!valid||wheel.lease!=sample.lease){wheel={};flashes=0;}
    const bool commander_page=held(deck183::CommanderPage);
    return {commander_page?512u:256u,sample.lease,commander_page?wheel.sector:sector,wheel.pointer,wheel,flashes};
}
static bool paint_wheel117(const WheelInput117& wheel){
    if(!visible||wheel.lease!=display_key106.lease||!IsWindow(game)||IsIconic(game)||!IsWindowVisible(game))return false;
    Status status{};
    if(!snapshot(status)||!status.ready||status.stop||status.release||status.mode!=POSSESSED||
       !overlay106::same(display_key106,{status.lease_generation,status.actor_id,status.unit_id}))return false;
    overlay106::Ack ack;
    if(!pixels106.ack(ack)||!ack.number||!overlay106::same(ack.key,display_key106))return false;
    RECT client{};
    if(!GetClientRect(game,&client)||client.right<640||client.bottom<360)return false;
    const float scale=hudlayout108::scale(float(client.right),float(client.bottom));
    const float width=client.right/scale,height=client.bottom/scale;
    const float radius=commander76::wheel_radius(width,height);
    const int side=int(std::ceil(radius*2.33f*scale))+8;
    const int w=std::min(side,int(client.right)),h=std::min(side,int(client.bottom));
    const int x=(client.right-w)/2,y=std::clamp(int(client.bottom*.48f)-h/2,0,int(client.bottom)-h);
    if(uint64_t(w)*h*4>overlay106::capacity(overlay106::Wheel))return false;
    static WheelSurface121 surface;
    const bool pc=(wheel.flags&commander76::PcWheel)!=0;
    const bool commander_page=pc?!(wheel.flags&commander76::PcOrders):wheel.flags==512u;
    const auto* pixels=surface.render(assets,client,x,y,w,h,display.mon.c_str(),display.native_crest,
        display.boshin,wheel.lease,wheel.sector,display.commands_known?int(display.commands.fire_mode):-1,
        display.following,pc,commander_page?&wheel.info:nullptr,&wheel.info.orders,wheel.pointer);
    overlay106::Frame frame{};
    frame.key=display_key106;frame.width=w;frame.height=h;
    frame.view_width=client.right;frame.view_height=client.bottom;
    frame.rect_count=1;frame.rect[0]={x,y,w,h};
    return wheel_pixels117.open(pid,overlay106::Wheel)&&wheel_pixels117.publish(frame,pixels);
}
static void update_wheel117(){
    static uint64_t next_measure{},next_fallback{};
    static WheelInput117 previous;
    const auto now=GetTickCount64();
    const auto wheel=wheel_input117();
    wheel_active121=wheel.open();
    const bool arrow_changed=visible&&equipment_shared&&bool(equipment_shared->arrow_camera)!=display.arrow_camera;
    const bool due=now>=next_measure||wheel.open()!=previous.open()||arrow_changed;
    if(!due&&wheel.same(previous))return;
    if(!due&&!cropped_wheel117&&!(wheel.flags&commander76::Rts)&&now<next_fallback)return;
    previous=wheel;
    if((wheel.flags&commander76::Rts)&&!due){
        try{if(commander_rts_hud76())return;}catch(...){}
    }
    bool cropped=false;
    try{cropped=wheel.open()&&paint_wheel117(wheel);}catch(...){}
    const bool changed_layer=cropped!=cropped_wheel117;
    if(!cropped&&(changed_layer||!wheel.open()))wheel_pixels117.hide();
    cropped_wheel117=cropped;
    if(due||changed_layer||!cropped){
        update();
        if(due)next_measure=now+150;
        if(!cropped)next_fallback=now+32;
    }
}
static void tick_hud121(HWND window){
    if(WaitForSingleObject(process,0)==WAIT_OBJECT_0){DestroyWindow(window);return;}
    update_wheel117();
}
static LRESULT CALLBACK proc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
    if(msg==WM_NCHITTEST)return HTTRANSPARENT;if(msg==WM_MOUSEACTIVATE)return MA_NOACTIVATE;if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_DISPLAYCHANGE||msg==WM_DPICHANGED){hud_surface100.reset();return 0;}
    if(msg==WM_TIMER){if(!fast_tick121)tick_hud121(w);return 0;}
    if(msg==WM_DESTROY){wheel_pixels117.hide();pixels106.hide();PostQuitMessage(0);return 0;}return DefWindowProcW(w,msg,wp,lp);}
static void run_hud121(){
    SetTimer(overlay,1,10,nullptr);
    Handle timer(CreateWaitableTimerExW(nullptr,nullptr,CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_MODIFY_STATE|SYNCHRONIZE));
    bool usable=timer.h!=nullptr;
    for(;;){
        if(usable&&wheel_active121!=fast_tick121){
            LARGE_INTEGER due{};due.QuadPart=-80000; // Relative 8 ms.
            if(wheel_active121){
                usable=SetWaitableTimer(timer.h,&due,8,nullptr,nullptr,FALSE)!=FALSE;
                fast_tick121=usable;
            }else{
                CancelWaitableTimer(timer.h);
                fast_tick121=false;
            }
        }
        const DWORD wake=MsgWaitForMultipleObjectsEx(fast_tick121?1:0,fast_tick121?&timer.h:nullptr,
            INFINITE,QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(wake==WAIT_FAILED){
            // Keep the ordinary HUD path when a compatibility layer cannot wait on the timer.
            if(!fast_tick121)return;
            CancelWaitableTimer(timer.h);fast_tick121=usable=false;
        }else if(fast_tick121&&wake==WAIT_OBJECT_0)tick_hud121(overlay);
        MSG msg{};
        while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT)return;
            TranslateMessage(&msg);DispatchMessageW(&msg);
        }
    }
}
static int preview(int argc,wchar_t** argv){
    using namespace Gdiplus;require(argc>=4,"Use --render output.png clean|combat|qte|ranged [width height]");
    int width=argc>=6?_wtoi(argv[4]):1920,height=argc>=6?_wtoi(argv[5]):1080;require(width>=640&&width<=3840&&height>=360&&height<=2160,"Invalid preview size");
    bool bomb=wcscmp(argv[3],L"bomb")==0,gun=wcscmp(argv[3],L"gun")==0||wcscmp(argv[3],L"gun-accurate")==0||wcscmp(argv[3],L"gun-inaccurate")==0,single=wcscmp(argv[3],L"single")==0,pending=wcscmp(argv[3],L"pending")==0,empty=wcscmp(argv[3],L"empty")==0;
    bool reload=wcscmp(argv[3],L"reload")==0,lowered=wcscmp(argv[3],L"lowered")==0,drawing=wcscmp(argv[3],L"drawing")==0,
        mortar=wcscmp(argv[3],L"mortar")==0,rocket=wcscmp(argv[3],L"rocket")==0,fots=wcscmp(argv[3],L"fots")==0,unknown=wcscmp(argv[3],L"unknown")==0;
    const bool bow_deck=wcscmp(argv[3],L"bow-deck")==0;
    bool bow_accurate=wcscmp(argv[3],L"bow-accurate")==0,bow_inaccurate=wcscmp(argv[3],L"bow-inaccurate")==0||bow_deck;
    bool ranged=wcscmp(argv[3],L"ranged")==0||wcscmp(argv[3],L"arrow-camera")==0||bow_accurate||bow_inaccurate||bomb||gun||single||pending||empty||reload||lowered||drawing||mortar||rocket||fots||unknown||wcscmp(argv[3],L"ammo-qte")==0,combat=wcscmp(argv[3],L"combat")==0,qte_active=wcscmp(argv[3],L"qte")==0||wcsncmp(argv[3],L"qte-",4)==0||wcscmp(argv[3],L"ammo-qte")==0;
    bool command_preset=wcsncmp(argv[3],L"command-",8)==0;
    const bool hidden_preset=wcsncmp(argv[3],L"hidden",6)==0;
    qte_active=qte_active||wcscmp(argv[3],L"hidden-qte")==0;
    bool climb_preset=wcsncmp(argv[3],L"climb-",6)==0;
    require(hidden_preset||command_preset||climb_preset||ranged||combat||qte_active||(wcscmp(argv[3],L"clean")==0||wcscmp(argv[3],L"melee")==0),"Unknown preview preset");
    hud80::View v;v.fixture=true;v.unit=hud80::unit_art(ranged?L"Inf_Missile_Bow_Ashigaru":L"Inf_Sword_Katana_Samurai");require(v.unit!=nullptr,"Fixture catalog entry missing");
    v.abilities_known=true;v.abilities.count=1;v.abilities.selected=0;v.abilities.items[0]={argc>=7?unsigned(_wtoi(argv[6])):39,ability141::Available,0};
    v.hidden=hidden_preset;
    require(ability141::art(v.abilities.items[0].id)!=nullptr,"Unknown ability preview");
    if(v.abilities.items[0].id==35)v.abilities.items[0].flags|=ability141::Targeted;
    if(argc>=8)v.abilities.error=unsigned(_wtoi(argv[7]));
    v.faction=L"Oda";v.mon=L"ui/clan_mons/data__oda.png";v.men=combat?73:118;v.initial=120;v.morale=combat?5:2;v.fatigue=combat?4:1;v.veterancy=3;
    v.commands_known=true;v.commands.fire_mode=1;v.equipment_known=true;v.has_ranged=ranged;v.slot=ranged?2:1;v.styles=ranged?3:0;v.ammunition=L"Fire arrows";
    v.climb_mode=wcscmp(argv[3],L"climb-near")==0?1:wcscmp(argv[3],L"climb-top")==0?2:wcscmp(argv[3],L"climb-active")==0?3:wcscmp(argv[3],L"climb-crest")==0?4:0;
    if(wcscmp(argv[3],L"melee")==0){v.unit=hud80::unit_art(L"Inf_Missile_Bow_Ashigaru");v.has_ranged=true;v.slot=1;}
    v.controller_ui=bow_deck;
    v.arrow_camera=wcscmp(argv[3],L"arrow-camera")==0;
    v.launcher_sight=rocket?LauncherSight123::Rocket:mortar?LauncherSight123::Mortar:LauncherSight123::None;
    v.ranged=ranged;v.bow=ranged&&!gun&&!bomb&&!mortar&&!rocket&&!fots;v.kind=v.bow?1:gun||fots?2:3;
    v.ammo_capacity=20;v.unit_activity_known=true;v.unit_activity=combat?unitactivity215::Activity::Melee:ranged?unitactivity215::Activity::Firing:unitactivity215::Activity::Holding;
    v.aiming=ranged&&!lowered;v.aim_pose=drawing?2:lowered?0:1;v.ammo=empty?0:19;v.pending=pending;
    v.accuracy_known=v.bow||gun||fots;
    v.cone_half_angle=bow_accurate||wcscmp(argv[3],L"gun-accurate")==0?.013f:
        bow_inaccurate||wcscmp(argv[3],L"gun-inaccurate")==0?.146f:.06f;
    v.projection_y=1/std::tan(1.309f*.5f);v.bow_power=bow_accurate?1.f:bow_inaccurate?.58f:0.f;
    v.reload=reload?2.4f:0;v.weapon_phase=reload?4:0;
    if(gun)v.unit=hud80::unit_art(L"Inf_Missile_Matchlock_Ashigaru");
    if(fots){v.boshin=true;v.unit=hud80::unit_art(L"Boshin_Inf_Militia_Levy_Infantry");}
    if(unknown){v.kind=0;v.equipment_known=false;}
    if(ranged){v.ammunition_known=true;auto& a=v.ammunition_state;a.count=single||mortar||rocket?1:gun||bomb||fots?2:3;a.current=single||pending||empty?0:1;a.pending=pending;
        for(unsigned i=0;i<a.count;++i){auto& item=a.items[i];item.icon=mortar?ammo83::Mortar:rocket?ammo83::Rocket:(gun||fots)?(i?ammo83::Suppression:ammo83::Bullet):bomb?(i?ammo83::BlindBomb:ammo83::Bomb):i==0?ammo83::Arrow:i==1?ammo83::FireArrow:ammo83::WhistleArrow;
            item.flags=(i==a.current?ammo83::Selected:0)|(i==0?ammo83::Normal:0)|(!empty&&!(gun&&i==1)?ammo83::Available:0);wcscpy_s(item.label,ammo83::label(ammo83::Icon(item.icon)));}}
    if(command_preset){auto& c=v.commands;v.commands_known=true;c.tick=GetTickCount64();c.feedback_until=c.tick+1800;
        c.move_mode=wcscmp(argv[3],L"command-hold")==0?3:4;c.fire_mode=wcscmp(argv[3],L"command-fire")==0?2:wcscmp(argv[3],L"command-hold")==0?0:1;
        c.placing=wcscmp(argv[3],L"command-drag")==0||wcscmp(argv[3],L"command-point")==0||wcscmp(argv[3],L"command-invalid")==0;

    }
    qte::Snapshot s;s.active=qte_active;s.serial=1;s.action=wcscmp(argv[3],L"qte-left")==0?qte::Action::Attack:qte::Action::Defend;s.result=wcscmp(argv[3],L"qte-interrupted")==0?qte::Result::Canceled:qte::Result::Pending;s.duration=1.4f;s.elapsed=.84f;s.response_elapsed=.84;s.placement=.65f;s.zone_width=.17f;
    Bitmap bitmap(width,height,PixelFormat32bppARGB);{Graphics g(&bitmap);LinearGradientBrush backdrop(Point(0,0),Point(width,height),Color(255,75,83,70),Color(255,26,31,25));g.FillRectangle(&backdrop,0,0,width,height);
        hud80::render(g,assets,float(width),float(height),v,qte_active?&s:nullptr,true);}
    UINT count=0,bytes=0;require(GetImageEncodersSize(&count,&bytes)==Ok,"PNG encoder unavailable");std::vector<BYTE> buffer(bytes);auto enc=reinterpret_cast<ImageCodecInfo*>(buffer.data());require(GetImageEncoders(count,bytes,enc)==Ok,"Encoder read failed");
    for(UINT i=0;i<count;++i)if(!wcscmp(enc[i].MimeType,L"image/png")){require(bitmap.Save(argv[2],&enc[i].Clsid)==Ok,"Preview save failed");std::cout<<"Rendered fixture using production HUD renderer\n";return 0;}
    throw std::runtime_error("No PNG encoder");
}
int wmain(int argc,wchar_t** argv){Gdiplus::GdiplusStartupInput input;ULONG_PTR token{};if(Gdiplus::GdiplusStartup(&token,&input,nullptr)!=Gdiplus::Ok)return 1;
    int result=1;try{
        require(assets.load(GetModuleHandleW(nullptr)),"Embedded assets unavailable");
        if(argc>1&&!wcscmp(argv[1],L"--render"))result=preview(argc,argv);
        else{
            require(argc==1||argc==2&&!wcscmp(argv[1],L"--probe"),"Use no arguments, --probe, or --render");
            pid=find_process(0);hudlayout108::deck_ui192=device192::enabled(pid);Module engine=find_module(pid);base=engine.base;require(engine.size==0x1d88000,"Unsupported engine address layout");(void)engine172::admit(engine.path,hash_file(engine.path));
            cards217=card217::Catalog{};
            Handle handle(OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid));process=handle.h;require(process!=nullptr,"Cannot read game");
            try{configure_cards();}catch(...){}
            wchar_t name[80];mapping_name(name,pid);Handle mapping(OpenFileMappingW(FILE_MAP_READ,FALSE,name));require(mapping.h!=nullptr,"Possession not loaded");
            shared=static_cast<Status*>(MapViewOfFile(mapping.h,FILE_MAP_READ,0,0,sizeof(Status)));require(shared!=nullptr,"HUD mapping failed");
            wchar_t equipment_name[96];equipment_mapping_name81(equipment_name,pid);Handle equipment_map(OpenFileMappingW(FILE_MAP_READ,FALSE,equipment_name));
            if(equipment_map.h)equipment_shared=static_cast<EquipmentStatus81*>(MapViewOfFile(equipment_map.h,FILE_MAP_READ,0,0,sizeof(EquipmentStatus81)));
            wchar_t ability_name141[96];ability141::mapping(ability_name141,pid);Handle ability_map141(OpenFileMappingW(FILE_MAP_READ,FALSE,ability_name141));
            if(ability_map141.h)abilities_shared141=static_cast<ability141::Status*>(MapViewOfFile(ability_map141.h,FILE_MAP_READ,0,0,sizeof(ability141::Status)));
            wchar_t ammo_name83[96];ammo83::mapping_name(ammo_name83,pid);Handle ammo_map83(OpenFileMappingW(FILE_MAP_READ,FALSE,ammo_name83));
            if(ammo_map83.h)ammunition_shared83=static_cast<ammo83::Status*>(MapViewOfFile(ammo_map83.h,FILE_MAP_READ,0,0,sizeof(ammo83::Status)));
            wchar_t accuracy_name90[96];accuracy90::mapping_name(accuracy_name90,pid);Handle accuracy_map90(OpenFileMappingW(FILE_MAP_READ,FALSE,accuracy_name90));
            if(accuracy_map90.h)accuracy_shared90=static_cast<accuracy90::Status*>(MapViewOfFile(accuracy_map90.h,FILE_MAP_READ,0,0,sizeof(accuracy90::Status)));
            wchar_t climb_name97[96];climb97::mapping_name(climb_name97,pid);Handle climb_map97(OpenFileMappingW(FILE_MAP_READ,FALSE,climb_name97));
            if(climb_map97.h)climb_shared97=static_cast<climb97::Status*>(MapViewOfFile(climb_map97.h,FILE_MAP_READ,0,0,sizeof(climb97::Status)));
            if(argc==2){bool ok=measure();std::wcout<<L"active="<<ok<<L" unit="<<(display.unit?display.unit->name:L"unavailable")<<L" faction="<<display.faction<<L" mon="<<display.mon<<L" men="<<display.men<<L" morale="<<display.morale<<L" fatigue="<<display.fatigue<<L"\n";result=ok?0:2;}
            else{Handle singleton(CreateMutexW(nullptr,FALSE,L"Local\\Shogun2PossessionHUD46"));if(GetLastError()==ERROR_ALREADY_EXISTS)result=0;
                else{game=gamewindow105::find(pid);SetProcessDPIAware();WNDCLASSW cls{};cls.lpfnWndProc=proc;cls.hInstance=GetModuleHandleW(nullptr);cls.lpszClassName=L"ShogunAuthenticHUD80";require(RegisterClassW(&cls)!=0,"HUD class failed");
                    overlay=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST,cls.lpszClassName,L"Shogun 2 Unit Possession HUD",WS_POPUP,0,0,0,0,nullptr,nullptr,cls.hInstance,nullptr);require(overlay!=nullptr,"HUD window failed");
                    run_hud121();result=0;}}
            if(visibility_shared148)UnmapViewOfFile(visibility_shared148);if(visibility_mapping148)CloseHandle(visibility_mapping148);
            if(commands_shared100)UnmapViewOfFile(commands_shared100);if(commands_map100)CloseHandle(commands_map100);
            if(climb_shared97)UnmapViewOfFile(climb_shared97);if(accuracy_shared90)UnmapViewOfFile(accuracy_shared90);if(ammunition_shared83)UnmapViewOfFile(ammunition_shared83);if(equipment_shared)UnmapViewOfFile(equipment_shared);UnmapViewOfFile(shared);
        }
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';}
    assets.clear();hud_surface100.reset();Gdiplus::GdiplusShutdown(token);
    return result;
}
