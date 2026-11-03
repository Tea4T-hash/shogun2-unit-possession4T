#include "../../shogun2-possession-cleanup-v46/src/hud_layout108.h"
#include <iostream>
#include <stdexcept>
static void check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static bool overlaps(hudlayout108::Rect a,hudlayout108::Rect b){
    return a.x<b.x+b.w&&b.x<a.x+a.w&&a.y<b.y+b.h&&b.y<a.y+a.h;
}
int main(){try{
    using namespace hudlayout108;
    const float views[][2]={{640,360},{1280,720},{1280,1024},{1920,1080},{2560,1440},{2560,1080},{3440,1440},{3840,2160}};
    for(const auto& view:views){Layout l(view[0],view[1]);
        for(Rect r:{l.soldier,l.commands,l.context})check(r.x>=0&&r.y>=0&&r.x+r.w<=l.width&&r.y+r.h<=l.height,"HUD panel outside viewport");
        check(!overlaps(l.soldier,l.context)&&!overlaps(l.commands,l.context)&&!overlaps(l.soldier,l.commands),"HUD panels overlap");
        Rect q{l.width*.5f-qte_surface*.5f,qte_anchor_y(l.height)-qte_surface*.5f,qte_surface,qte_surface};
        check(q.y>=0&&q.y+q.h<l.height*.5f,"Active QTE clips viewport or covers lower HUD");
        check(q.y*l.zoom>=6.f+38.f*std::min(1.f,(view[0]-24.f)/760.f),"QTE overlaps the compact View header");
        check(!overlaps(q,l.soldier)&&!overlaps(q,l.commands)&&!overlaps(q,l.context),"QTE overlaps peripheral HUD");
    }
    Context idle{true,false,false,false,false,0,0,0,20,0};
    check(!context_visible(idle),"Idle melee retains unnecessary context");
    idle.ranged=true;check(!context_visible(idle),"Lowered idle ranged weapon retains unnecessary context");
    auto s=idle;s.aiming=true;check(context_visible(s),"Aiming lost ammunition context");
    s=idle;s.reload=2.4f;check(context_visible(s),"Reload lost context");
    s=idle;s.ammunition=0;check(context_visible(s),"Empty ammunition hidden");
    s=idle;s.error=1;check(context_visible(s),"Native ammunition error hidden");
    for(unsigned phase=1;phase<=4;++phase){s=idle;s.phase=phase;check(context_visible(s),"Weapon action hidden");}
    for(unsigned climb=1;climb<=4;++climb){s={};s.climb=climb;check(context_visible(s),"Climb prompt hidden");}
    s={};s.pending=true;check(context_visible(s),"Pending equipment hidden");
    s={};s.crouched=true;check(context_visible(s),"Kneeling context hidden");
    std::cout<<"PASS: eight viewport layouts, unobstructed aim/QTE, separated panels and contextual visibility.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
