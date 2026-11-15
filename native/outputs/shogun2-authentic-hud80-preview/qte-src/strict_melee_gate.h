#pragma once
#include "qte_native_bridge.h"
namespace qte { namespace strictmelee {
// Only called from melee witness hooks. Projectile code never uses this gate.
inline bool includes(const Reader& r,uint32_t actor,uint32_t witness){
    if(!actor||!witness||witness>UINT32_MAX-0x100)return false;
    uint32_t backlink{};if(r.get(actor+0x688,backlink)&&backlink==witness)return true;
    // One current, local participant snapshot. No cross-callback cache and no
    // native writes. Retain the individual reads when a page boundary prevents
    // reading the whole prefix: an earlier readable group can still match.
    std::array<uint32_t,20> prefix{};
    if(r.get(witness,prefix)){
        for(auto offsets:{std::pair<unsigned,unsigned>{0x10,0x24},{0x30,0x38},{0x3c,0x48}}){
            uint32_t count=prefix[offsets.second/4];if(count>5)continue;
            for(uint32_t i=0;i<count;++i)if(prefix[offsets.first/4+i]==actor)return true;
        }return false;
    }
    for(auto offsets:{std::pair<unsigned,unsigned>{0x10,0x24},{0x30,0x38},{0x3c,0x48}}){
        uint32_t count{};if(!r.get(witness+offsets.second,count)||count>5)continue;
        for(uint32_t i=0;i<count;++i){uint32_t member{};if(!r.get(witness+offsets.first+i*4,member))break;if(member==actor)return true;}
    }return false;
}
struct Authorization {
    WitnessKey key{};uint64_t serial{};NativeActor player{},enemy{};
    NativeVerdict verdict{};uint32_t matched{},fallback{};bool sealed{};
    bool permit(const WitnessKey& candidate,uint64_t shown,Result result,const NativeActor& p,const NativeActor& e){
        if(!candidate.valid()||!shown||result==Result::Pending||result==Result::Canceled||
           p.serial!=candidate.pair.player||e.serial!=candidate.pair.opponent||p.unit_serial!=candidate.pair.player_unit||
           e.unit_serial!=candidate.pair.opponent_unit||p.address==e.address||!p.address||!e.address)return false;
        *this={};key=candidate;serial=shown;player=p;enemy=e;return true;
    }
    bool matches(uint32_t address,uint64_t generation,uint64_t lease)const{return serial&&key.address==address&&key.generation==generation&&key.pair.lease==lease;}
    void removed(uint32_t address,uint64_t generation){if(key.address==address&&key.generation==generation)*this={};}
};
using WriteBytes=bool(*)(void*,uint32_t,const void*,size_t);
using ReleaseExcluded213=void(*)(void*,uint32_t,uint32_t);
inline bool retain_selected(const Reader& r,WriteBytes write,void* context,uint32_t engine,uint32_t witness,const NativeActor& player,const NativeActor& enemy,ReleaseExcluded213 release=nullptr){
    SelectionInput input;uint8_t alternate{};
    if(!write||!selection_input(r,witness,engine,input)||!r.get(witness+0xee,alternate)||alternate||
       player.address==enemy.address||player.unit==enemy.unit||player.side==enemy.side)return false;
    bool p=false,e=false;uint32_t original[5]{};
    for(unsigned i=0;i<5;++i)if(!r.get(witness+0x10+i*4,original[i]))return false;
    for(unsigned i=0;i<input.count;++i){uint32_t backlink{};
        if(!r.get(original[i]+0x688,backlink)||backlink)return false;
        p|=original[i]==player.address;e|=original[i]==enemy.address;
    }
    if(!p||!e||(input.count>2&&!release))return false;
    // Native3c0b10 reserves EVERY initial candidate in manager+88 before
    // preparation. A shortened list hides excluded actors from the original
    // destructor. Refuse duplicates and keep the full list on write failure.
    for(unsigned i=0;i<input.count;++i)for(unsigned j=0;j<i;++j)
        if(original[i]==original[j])return false;
    uint32_t pair[2]={player.address,enemy.address},count=2;
    if(!write(context,witness+0x10,pair,sizeof pair))return false;
    if(!write(context,witness+0x24,&count,sizeof count)){write(context,witness+0x10,original,sizeof original);return false;}
    // The initial list and every zero backlink were validated above. Release
    // excluded reservations through original3e1930 on this engine callback;
    // keep both selected candidates reserved for their native preparation.
    for(unsigned i=0;i<input.count;++i)
        if(original[i]!=player.address&&original[i]!=enemy.address)
            release(context,witness,original[i]);
    return true;
}
inline bool narrow(const Reader& r,WriteBytes write,void* context,uint32_t engine,uint32_t witness,const NativeActor& player,ReleaseExcluded213 release=nullptr){
    SelectionInput input;if(!selection_input(r,witness,engine,input))return false;
    if(input.count==2)return true;
    uint8_t alternate{};if(!r.get(witness+0xee,alternate)||alternate||!write)return false;
    NativeActor enemy{};float best=1e20f,px{},pz{};
    if(!r.get(player.address+0x48,px)||!r.get(player.address+0x50,pz))return false;
    bool found=false;
    for(uint32_t i=0;i<input.count;++i){uint32_t address{},mount{},link{};NativeActor actor;float x{},z{};
        if(!r.get(input.begin+i*4,address))return false;
        found|=address==player.address;
        if(!actor_identity(r,address,witness,engine,actor,true)||actor.side==player.side||actor.unit==player.unit||
           !r.get(address+0x21c,mount)||mount||!r.get(address+0x688,link)||link||
           !r.get(address+0x48,x)||!r.get(address+0x50,z)||!std::isfinite(x)||!std::isfinite(z))continue;
        float distance=(x-px)*(x-px)+(z-pz)*(z-pz);if(distance<best){best=distance;enemy=actor;}
    }
    if(!found||!enemy.address)return false;
    // Legacy helper is unused by production selection. Share the same native
    // reservation cleanup if a fixture or future caller retains this pair.
    return retain_selected(r,write,context,engine,witness,player,enemy,release);
}
} }
