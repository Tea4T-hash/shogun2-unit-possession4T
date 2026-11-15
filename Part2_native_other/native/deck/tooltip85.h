#pragma once
#include "campaign181.h"
namespace deck85 {

struct Point {float x{},y{};};
struct Placement {Point from{},to{};float scale{1};};
inline Placement placement(Point position,float width,float height,deck185::UiSize view,float strip_top=0.f){
    if(!deck185::ui_size_valid(view)||!std::isfinite(position.x)||!std::isfinite(position.y)||
       !std::isfinite(width)||!std::isfinite(height)||width<=0||height<=0)return {};
    constexpr float margin=12.f;
    // Leave the hovered controls and surrounding battlefield visible.
    const float factor=std::min({1.5f,view.width*.68f/width,view.height*.58f/height});
    if(strip_top>0.f){
        constexpr float gap=6.f;
        const float bottom=strip_top-gap;
        const float scale=std::min({factor<=1.01f?1.f:factor,
                                    (view.width-2*margin)/width,(bottom-margin)/height});
        const float right=std::max(margin,view.width-width*scale-margin);
        const float upper_y=std::max(margin,bottom-height*scale);
        return {position,{std::clamp(position.x,margin,right),
                         std::clamp(position.y,margin,upper_y)},scale};
    }
    if(factor<=1.01f)return {};
    return {position,
        {std::clamp(position.x,margin,view.width-width*factor-margin),
         std::clamp(position.y,margin,view.height-height*factor-margin)},factor};
}

struct Projection {void* renderer{};Placement place;};
inline thread_local Projection projection;
using Convert=Point*(__thiscall*)(void*,Point*,float,float);
using Draw=unsigned char(__thiscall*)(void*,const void*,uintptr_t,unsigned,unsigned);
inline Convert origin[2]{},size[2]{};
inline Draw original_draw{};
inline uintptr_t image{};
inline constexpr unsigned draw_rva=0xf8a4e0;
inline constexpr unsigned origin_rvas[]{0x1401080,0x1421f70};
inline constexpr unsigned size_rvas[]{0x13e35d0,0x1421ec0};

template<unsigned Mode,bool Position>
inline Point* __fastcall convert(void* renderer,void*,Point* output,float x,float y){
    const auto& place=projection.place;
    if(renderer==projection.renderer){
        if constexpr(Position){
            x=place.to.x+(x-place.from.x)*place.scale;
            y=place.to.y+(y-place.from.y)*place.scale;
        }else{
            x*=place.scale;
            y*=place.scale;
        }
    }
    return (Position?origin[Mode]:size[Mode])(renderer,output,x,y);
}

struct Observation {uintptr_t root{},widget{},state{};ULONGLONG seen{};};
inline thread_local std::array<Observation,16> campaign_tooltips;
inline bool attached(uintptr_t widget,uintptr_t root){
    if(!root||!widget||widget==root)return false;
    for(unsigned i=0;widget&&i<16;++i){
        if(widget==root)return true;
        uintptr_t parent{};
        if(!native214::value(widget+0x94,parent)||parent==widget)return false;
        widget=parent;
    }
    return false;
}
inline bool observe_campaign_tooltip(uintptr_t root,uintptr_t widget){
    deck214::Node node;
    if(!image||!attached(widget,root)||!deck214::node(widget,node))return false;
    const auto now=GetTickCount64();
    auto slot=std::find_if(campaign_tooltips.begin(),campaign_tooltips.end(),[&](const Observation& item){
        return item.widget==widget||item.root!=root||now-item.seen>250;
    });
    if(slot==campaign_tooltips.end())return false;
    *slot={root,widget,node.state,now};
    return true;
}

inline uintptr_t battle_card_strip(uintptr_t root,uintptr_t& manager){
    // The native battle card manager owns the review panel at manager+9c.
    // Its attachment to the current UI root distinguishes it from the
    // front-end army setup and from stale battle state on return to menus.
    uintptr_t battle{},table{},owner{},strip{};
    unsigned char visible{};
    if(!native214::value(image+0x1bc8180,battle)||!battle||
       !native214::value(battle,table)||table!=image+0x170d284||
       !native214::value(battle+0x5b4,manager)||!manager||
       !native214::value(manager,table)||table!=image+0x170d184||
       !native214::value(manager+0x98,owner)||owner!=battle||
       !native214::value(manager+0x9c,strip)||!attached(strip,root)||
       !native214::value(strip+0xf5,visible)||visible!=1)return 0;
    return strip;
}

inline float battle_card_strip_top(uintptr_t manager,uintptr_t strip,deck185::UiSize view){
    // 10f9e640 publishes the hit widget. Check the native serial-to-card
    // registry so adjacent button tooltips use their normal placement.
    uintptr_t hover{};
    if(!strip||!native214::value(image+0x1bbeac8,hover)||!attached(hover,strip))return 0.f;
    unsigned char visible{};
    uintptr_t path[16]{};
    unsigned depth=0;
    for(;hover!=strip&&depth<16;++depth){
        path[depth]=hover;
        if(!native214::value(hover+0x94,hover))return 0.f;
    }
    if(hover!=strip)return 0.f;

    uintptr_t card{};
    __try{
        const auto buckets=*reinterpret_cast<const unsigned*>(manager+0x70);
        const auto rows=*reinterpret_cast<const uintptr_t*>(manager+0x74);
        if(!buckets||buckets>8192||!rows)return 0.f;
        unsigned entries=0;
        for(unsigned i=0;i<buckets&&!card;++i){
            auto row=reinterpret_cast<const uintptr_t*>(rows+i*16);
            for(auto entry=row[0];entry&&entry!=row[1];){
                if(++entries>512)return 0.f;
                auto item=reinterpret_cast<const uintptr_t*>(entry);
                if(item[3]&&std::find(path,path+depth,item[3])!=path+depth){card=item[3];break;}
                entry=item[1];
            }
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){return 0.f;}
    deck214::Node panel,hit;
    if(!card||!native214::value(card+0xf5,visible)||visible!=1||
       !deck214::node(strip,panel)||!deck214::node(card,hit)||
       !deck214::original_position(panel)||!deck214::original_position(hit))return 0.f;
    const float top=std::min(panel.y,hit.y);
    return top>=view.height*.5f&&top<view.height?top:0.f;
}

inline bool tooltip_projection(uintptr_t widget,const void* context,Projection& result){
    // Ghidra: both native Tooltip and SpecialTooltip publish this pointer,
    // which the root draws last at f8b0fe. Campaign Lua observes separately
    // named native hover templates that can use the ordinary child path.
    const auto active=*reinterpret_cast<const uintptr_t*>(image+0x1bbe368);
    const auto root=*reinterpret_cast<const uintptr_t*>(image+0x1bbe364);
    const auto observed=std::find_if(campaign_tooltips.begin(),campaign_tooltips.end(),[&](const Observation& item){
        return item.root==root&&item.widget==widget;
    });
    if(widget!=active&&(observed==campaign_tooltips.end()||GetTickCount64()-observed->seen>250))return false;
    unsigned char visible{};
    deck214::Node node;
    if(!attached(widget,root)||!deck214::node(widget,node)||
       (widget!=active&&node.state!=observed->state)||
       !native214::value(widget+0xf5,visible)||visible!=1||!deck214::original_position(node))return false;
    // The shared active-tooltip slot is also used by Custom Battle setup.
    // Project only with a live campaign camera or a battle-owned card panel
    // attached to this root. Otherwise let Shogun draw the popup as authored.
    uintptr_t manager{};
    const auto strip=battle_card_strip(root,manager);
    if(!strip&&!deck181::campaign_camera(image,[](uintptr_t address){
        uintptr_t value{};
        native214::value(address,value);
        return value;
    }))return false;
    // Native draw context starts with sprite and font renderers. Both use
    // the same logical viewport to convert their origins and dimensions.
    uintptr_t draw_context[2]{},batch_table{},font_table{},font_batch{},renderer{},render_table{};
    if(!native214::read(reinterpret_cast<uintptr_t>(context),draw_context,sizeof draw_context)||
       !native214::value(draw_context[0],batch_table)||batch_table!=image+0x1752370||
       !native214::value(draw_context[1],font_table)||font_table!=image+0x1752488||
       !native214::value(draw_context[1]+8,font_batch)||font_batch!=draw_context[0]||
       !native214::value(draw_context[0]+0x10,renderer)||
       !native214::value(renderer,render_table)||render_table!=image+0x1754574)return false;
    auto view=deck185::active_ui185();
    if(!deck185::ui_size_valid(view)||node.width<=0||node.height<=0)return false;
    const float strip_top=widget==active?battle_card_strip_top(manager,strip,view):0.f;
    const auto place=placement({node.x,node.y},node.width,node.height,view,strip_top);
    if(!strip_top&&place.scale<=1.f)return false;
    result={reinterpret_cast<void*>(renderer),place};
    return true;
}

inline unsigned char __fastcall draw(void* widget,void*,const void* context,uintptr_t time,unsigned layer,unsigned options){
    Projection candidate;
    if(projection.renderer||!image||!tooltip_projection(reinterpret_cast<uintptr_t>(widget),context,candidate))
        return original_draw(widget,context,time,layer,options);
    // Scope affects only the vertices of this popup subtree. Native layout,
    // font selection, textures, content lifetime and mouse routing are intact.
    projection=candidate;
    __try{return original_draw(widget,context,time,layer,options);}
    __finally{projection={};}
}

inline bool install_tooltips(uintptr_t base){
    if(image)return image==base;
    if(!deck185::native_ui185.ready||!native214::admit(base,512))return false;
    const unsigned rvas[]{origin_rvas[0],size_rvas[0],origin_rvas[1],size_rvas[1],draw_rva};
    void* callbacks[]{reinterpret_cast<void*>(&convert<0,true>),reinterpret_cast<void*>(&convert<0,false>),
                      reinterpret_cast<void*>(&convert<1,true>),reinterpret_cast<void*>(&convert<1,false>),
                      reinterpret_cast<void*>(&draw)};
    void** originals[]{reinterpret_cast<void**>(&origin[0]),reinterpret_cast<void**>(&size[0]),
                       reinterpret_cast<void**>(&origin[1]),reinterpret_cast<void**>(&size[1]),
                       reinterpret_cast<void**>(&original_draw)};
    unsigned created=0;
    for(;created<std::size(rvas);++created){
        auto target=reinterpret_cast<void*>(base+rvas[created]);
        if(MH_CreateHook(target,callbacks[created],originals[created])!=MH_OK)break;
    }
    if(created==std::size(rvas)){
        unsigned enabled=0;
        for(;enabled<created;++enabled)
            if(MH_EnableHook(reinterpret_cast<void*>(base+rvas[enabled]))!=MH_OK)break;
        if(enabled==created){image=base;return true;}
    }
    while(created){
        --created;
        auto target=reinterpret_cast<void*>(base+rvas[created]);
        MH_DisableHook(target);
        MH_RemoveHook(target);
        *originals[created]=nullptr;
    }
    return false;
}
} // deck85
