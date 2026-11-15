#pragma once
#include "native214.h"
#include "fine216.h"
#include "orders175.h"
#include "terrain224.h"
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"
#include <array>
#include <intrin.h>
#include <vector>

namespace formation218 {
class FormationPad {
    static constexpr unsigned max_selection = 512;
    static constexpr unsigned queue_capacity = 0x5000;
    static constexpr int idle = 165;
    static constexpr double radians_per_turn_unit = 6.283185307179586 / 65536.0;
    struct SelectionList { unsigned capacity{}, count{}; uintptr_t entries{}; };
    struct Member {
        uintptr_t object{}, table{};
        unsigned id{};
        bool group{};
        float x{}, z{}, move_x{}, move_z{};
        uint16_t facing{};
    };
    struct Destination { terrain224::Point point{}; uint16_t facing{}; };
    struct PreviewEntry {
        uintptr_t subject{};
        terrain224::Point point{};
        uint16_t facing{}, padding{};
        float scale{};
    };
    static_assert(sizeof(SelectionList) == 12 && sizeof(PreviewEntry) == 32);
    using PreviewPlanner = void(__thiscall*)(uintptr_t, uintptr_t, unsigned,
                                              SelectionList*, SelectionList*);
    using MovePlanner = void(__thiscall*)(uintptr_t, uintptr_t, float,
                                           SelectionList*, SelectionList*);
    static inline PreviewPlanner original_preview_{};
    static inline MovePlanner original_move_preview_{};
    static inline uintptr_t preview_target_{}, move_preview_target_{};
    static inline thread_local FormationPad* preview_owner_{};

    uintptr_t image_{}, root_{}, env_{}, ui_{}, widget_{}, control_{}, owner_{};
    unsigned thread_{};
    std::array<SelectionList,2> lists_{};
    std::vector<Member> members_;
    std::array<Destination,max_selection> destinations_{};
    float pivot_x_{}, pivot_z_{}, planned_distance_{};
    uint16_t requested_angle_{}, planned_angle_{};
    int width_steps_{};
    unsigned width_file_budget_{};
    bool starting_{}, rotating_{}, translating_{}, resizing_{}, plan_valid_{}, preview_ready_{};
    bool release_append_{};
    ULONGLONG gesture_tick_{}, release_tick_{};

    template<class T> static bool get(uintptr_t p,T& v) { return native214::value(p,v); }
    template<class T> static bool set(uintptr_t p,const T& v) {
        SIZE_T written{};
        return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(p),&v,sizeof v,&written)
            && written==sizeof v;
    }
    static uintptr_t word(uintptr_t address) {
        uintptr_t value{};
        get(address,value);
        return value;
    }
    static bool table(uintptr_t image,uintptr_t widget) {
        const auto v=word(widget);
        return v==image+0x1704cdc||v==image+0x16e2aac;
    }
    bool control_live() const {
        unsigned char land{};
        if(!image_||GetCurrentThreadId()!=thread_||word(image_+0x1bc8180)!=root_||
           word(image_+0x1bbe364)!=ui_||word(widget_+0x318)!=control_||
           word(control_)!=image_+0x1709cc8||word(control_+4)!=widget_||
           !get(control_+0x50,land)||land!=1)return false;
        uintptr_t p=widget_;
        for(unsigned n=0;p&&p!=ui_&&n<20;++n)p=word(p+0x94);
        return p==ui_;
    }
    bool read_selection(std::array<SelectionList,2>& lists) const {
        const auto selected=word(env_+0x2826c);
        if(!selected||word(selected+0x4a8)!=owner_||word(root_+0x5b4)!=owner_||
           word(root_+0x110)!=env_||word(root_+0x5b8)!=env_)return false;
        const auto selection=owner_+0xb0;
        if(word(selection)!=image_+0x15c4da0)return false;
        for(unsigned at:{0x20u,0x2cu}) {
            unsigned count{};
            if(!get(selection+at,count)||count)return false;
        }
        // The fifth vector is copied by the original order constructor but
        // is not a battle-command target list. A real UI group populates it.
        SelectionList ui_selection{};
        if(!get(selection+0x38,ui_selection)||ui_selection.count>max_selection||
           (ui_selection.count&&(!ui_selection.entries||ui_selection.capacity<ui_selection.count)))return false;
        if(!get(selection+4,lists))return false;
        for(const auto& list:lists)
            if(list.count>max_selection||(list.count&&(!list.entries||list.capacity<list.count)))return false;
        const unsigned total=lists[0].count+lists[1].count;
        return total&&total<=max_selection;
    }
    bool selection_live() const {
        std::array<SelectionList,2> current{};
        if(!control_live()||!read_selection(current))return false;
        unsigned next=0;
        for(unsigned category=0;category<2;++category) {
            const auto& list=current[category];
            if(list.count!=lists_[category].count)return false;
            for(unsigned i=0;i<list.count;++i) {
                const auto& member=members_[next++];
                unsigned id{};
                if(word(list.entries+i*4)!=member.object||word(member.object)!=member.table||
                   !get(member.object+(member.group?0x7c:4),id)||id!=member.id)return false;
            }
        }
        return next==members_.size();
    }
    bool capture_selection() {
        owner_=word(root_+0x5b4);
        if(!owner_||!read_selection(lists_))return false;
        members_.reserve(lists_[0].count+lists_[1].count);
        for(unsigned category=0;category<2;++category) {
            const auto& list=lists_[category];
            for(unsigned i=0;i<list.count;++i) {
                Member member{};
                member.group=category==0;
                member.object=word(list.entries+i*4);
                if(!member.object||!(member.table=word(member.object))||
                   !get(member.object+(member.group?0x7c:4),member.id))return false;
                for(const auto& prior:members_)if(prior.object==member.object)return false;
                members_.push_back(member);
            }
        }
        return true;
    }
    bool find_compass() {
        ui_=word(image_+0x1bbe364);
        if(!ui_)return false;
        struct Node { uintptr_t address; unsigned depth; };
        std::vector<Node> nodes{{ui_,0}};
        for(size_t i=0;i<nodes.size();++i) {
            const auto node=nodes[i];
            if(!table(image_,node.address))continue;
            const auto control=word(node.address+0x318);
            unsigned char land{};
            if(control&&word(control)==image_+0x1709cc8&&word(control+4)==node.address&&
               get(control+0x50,land)&&land==1) {
                if(control_)return false;
                widget_=node.address;
                control_=control;
            }
            unsigned count{};
            uintptr_t children{};
            if(!get(node.address+0x9c,count)||count>512||!get(node.address+0xa0,children)||
               (count&&!children)||node.depth>20)return false;
            for(unsigned n=0;n<count;++n) {
                const auto child=word(children+n*4);
                if(nodes.size()>=4096||!child||word(child+0x94)!=node.address||
                   std::any_of(nodes.begin(),nodes.end(),[&](const Node& item){return item.address==child;}))
                    return false;
                nodes.push_back({child,node.depth+1});
            }
        }
        return control_&&control_live();
    }

    bool begin_move() {
        __try {
            reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned)>(image_+0x10dc0c0)(control_,2);
            return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
    void cancel_move() {
        __try {
            reinterpret_cast<void(__thiscall*)(uintptr_t,unsigned)>(image_+0x10dc060)(control_,0);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    bool read_pose(Member& member) const {
        // Native 42dfd0/42e290 read the point and heading from either a
        // group reference frame or the ordinary land-unit pose.
        const auto pose=member.group?member.object+0x2c:word(member.object+0x440);
        if(!pose||!get(pose+(member.group?0x14:0x28),member.facing))return false;
        float world[3]{};
        __try {
            reinterpret_cast<float*(__thiscall*)(uintptr_t,float*)>(image_+0x399f80)(
                member.group?pose:pose+0x10,world);
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
        member.x=world[0];
        member.z=world[2];
        return std::isfinite(member.x)&&std::isfinite(member.z)&&
            std::fabs(member.x)<=100000.f&&std::fabs(member.z)<=100000.f;
    }
    uintptr_t terrain_manager() const {
        const auto common=word(env_+8);
        const auto manager=common?word(common+0xb8):0;
        return manager&&word(manager)==env_?manager:0;
    }
    bool place(uintptr_t manager,size_t i,float x,float z,uint16_t facing) {
        const float xz[]{x,z};
        auto& destination=destinations_[i];
        if(!terrain224::locate(image_,manager,xz,destination.point))return false;
        // A terrain fallback must not split the selected formation.
        if(std::fabs(destination.point.p[0]-x)>.025f||
           std::fabs(destination.point.p[2]-z)>.025f)return false;
        destination.facing=facing;
        return true;
    }
    bool rotation_plan(uint16_t angle) {
        const auto manager=terrain_manager();
        if(!manager)return false;
        const double radians=angle*radians_per_turn_unit;
        const double sine=std::sin(radians),cosine=std::cos(radians);
        for(size_t i=0;i<members_.size();++i) {
            const auto& member=members_[i];
            const double dx=member.x-pivot_x_,dz=member.z-pivot_z_;
            if(!place(manager,i,float(pivot_x_+dx*cosine+dz*sine),
                       float(pivot_z_+dz*cosine-dx*sine),uint16_t(member.facing+angle)))return false;
        }
        return true;
    }
    bool translation_plan(float distance) {
        const auto manager=terrain_manager();
        if(!manager)return false;
        for(size_t i=0;i<members_.size();++i) {
            const auto& member=members_[i];
            if(!place(manager,i,member.x+member.move_x*distance,
                       member.z+member.move_z*distance,member.facing))return false;
        }
        return true;
    }
    bool add_preview(const Member& member,const Destination& destination,SelectionList* list) const {
        PreviewEntry entry{};
        __try {
            using Construct=void*(__thiscall*)(PreviewEntry*,uintptr_t,const terrain224::Point*,unsigned);
            auto subject=member.group?member.object:member.object+0xc;
            reinterpret_cast<Construct>(image_+0x40bd40)(&entry,subject,&destination.point,destination.facing);
            if(!std::isfinite(entry.scale))return false;
            reinterpret_cast<void(__thiscall*)(SelectionList*,const PreviewEntry*)>(image_+0x471950)(list,&entry);
            return true;
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
    }
    bool preview_angle_owned() const {
        uint16_t angle{};
        int mode{};
        return get(control_+0x40,mode)&&mode==idle&&get(root_+0x59a,angle)&&
            angle==requested_angle_;
    }
    bool preview_fields_owned() const {
        return preview_angle_owned()&&!word(root_+0x590)&&!word(root_+0x594)&&!word(root_+0x59c);
    }
    bool preview_context(uintptr_t selection,uintptr_t common,
                         const SelectionList* groups,const SelectionList* units) const {
        return GetTickCount64()-gesture_tick_<=450&&selection==owner_+0xb0&&
            common==word(env_+8)&&selection_live()&&groups&&units&&
            !groups->entries&&!groups->count&&!groups->capacity&&
            !units->entries&&!units->count&&!units->capacity;
    }
    void populate_preview(SelectionList* groups,SelectionList* units) {
        if(!plan_valid_)return;
        for(size_t i=0;i<members_.size();++i)
            if(!add_preview(members_[i],destinations_[i],members_[i].group?groups:units)) {
                groups->count=units->count=0;
                return;
            }
        preview_ready_=groups->count==lists_[0].count&&units->count==lists_[1].count;
    }
    void draw_preview(uintptr_t selection,uintptr_t common,uint16_t angle,
                      SelectionList* groups,SelectionList* units) {
        preview_ready_=false;
        if(!rotating_||!angle||angle!=requested_angle_||!preview_fields_owned()||
           !preview_context(selection,common,groups,units))return;
        if(!plan_valid_||planned_angle_!=angle) {
            plan_valid_=rotation_plan(angle);
            planned_angle_=angle;
        }
        populate_preview(groups,units);
    }
    bool translation_owned() const {
        uint16_t angle{};
        return word(control_+0x40)==14&&!word(root_+0x590)&&!word(root_+0x59c)&&
            get(root_+0x59a,angle)&&!angle;
    }
    void draw_move_preview(uintptr_t selection,uintptr_t common,float distance,
                           SelectionList* groups,SelectionList* units) {
        preview_ready_=false;
        float forward{};
        if(!translating_||!std::isfinite(distance)||distance<=0.f||distance>4096.f||
           !translation_owned()||!get(root_+0x594,forward)||distance!=forward||
           !preview_context(selection,common,groups,units))return;
        if(!plan_valid_||planned_distance_!=distance) {
            plan_valid_=translation_plan(distance);
            planned_distance_=distance;
        }
        populate_preview(groups,units);
    }
    static void __fastcall preview_hook(uintptr_t selection,void*,uintptr_t common,unsigned angle,
                                       SelectionList* groups,SelectionList* units) {
        auto* owner=preview_owner_;
        // 2647e0 is the preview constructor. Simulation calls the same planner
        // from a different return address and must always keep its original code.
        if(owner&&owner->rotating_&&selection==owner->owner_+0xb0&&
           reinterpret_cast<uintptr_t>(_ReturnAddress())==owner->image_+0x2648c9) {
            owner->draw_preview(selection,common,uint16_t(angle),groups,units);
            return;
        }
        original_preview_(selection,common,angle,groups,units);
    }
    static void __fastcall move_preview_hook(uintptr_t selection,void*,uintptr_t common,float distance,
                                            SelectionList* groups,SelectionList* units) {
        auto* owner=preview_owner_;
        // Only the forward ghost constructor calls this site at 2646fc.
        // Native order execution and ordinary compass actions pass through.
        if(owner&&owner->translating_&&selection==owner->owner_+0xb0&&
           reinterpret_cast<uintptr_t>(_ReturnAddress())==owner->image_+0x2646fc) {
            owner->draw_move_preview(selection,common,distance,groups,units);
            return;
        }
        original_move_preview_(selection,common,distance,groups,units);
    }
    static bool owns_hook(uintptr_t target,const void* detour) {
        unsigned char jump[5]{};
        uint32_t displacement{};
        if(!target||!get(target,jump)||jump[0]!=0xe9)return false;
        memcpy(&displacement,jump+1,sizeof displacement);
        return target+5+displacement==reinterpret_cast<uintptr_t>(detour);
    }
    bool install_hook(uintptr_t rva,unsigned feature,void* detour,void** original,uintptr_t& installed) {
        const auto target=image_+rva;
        if(installed)return installed==target&&owns_hook(target,detour);
        if(!native214::admit(image_,feature))return false;
        const auto address=reinterpret_cast<void*>(target);
        if(MH_CreateHook(address,detour,original)!=MH_OK)return false;
        if(MH_EnableHook(address)!=MH_OK) {
            MH_RemoveHook(address);
            *original=nullptr;
            return false;
        }
        installed=target;
        return owns_hook(target,detour);
    }
    bool begin_translation(deck174::Vec2 direction) {
        if(preview_owner_)return false;
        for(auto& member:members_) {
            if(!read_pose(member))return false;
            const double angle=member.facing*radians_per_turn_unit;
            const double sine=std::sin(angle),cosine=std::cos(angle);
            member.move_x=float(cosine*direction.x+sine*direction.y);
            member.move_z=float(cosine*direction.y-sine*direction.x);
        }
        if(!install_hook(0x42dfd0,32,reinterpret_cast<void*>(&move_preview_hook),
                         reinterpret_cast<void**>(&original_move_preview_),move_preview_target_))return false;
        translating_=true;
        if(!begin_move()||word(control_+0x40)!=14)return false;
        preview_owner_=this;
        gesture_tick_=GetTickCount64();
        return true;
    }
    bool begin_rotation() {
        if(preview_owner_||!preview_fields_owned())return false;
        double x=0,z=0;
        for(auto& member:members_) {
            if(!read_pose(member))return false;
            x+=member.x;
            z+=member.z;
        }
        pivot_x_=float(x/members_.size());
        pivot_z_=float(z/members_.size());
        if(!install_hook(0x42e540,16,reinterpret_cast<void*>(&preview_hook),
                         reinterpret_cast<void**>(&original_preview_),preview_target_))return false;
        rotating_=true;
        preview_owner_=this;
        gesture_tick_=GetTickCount64();
        return true;
    }
    bool turn_to(float degrees) {
        if(!std::isfinite(degrees)||std::fabs(degrees)>180.f||!preview_fields_owned())return false;
        const auto angle=uint16_t(std::lround(std::round(degrees)*65536.0/360.0));
        gesture_tick_=GetTickCount64();
        if(angle==requested_angle_)return true;
        if(!set(root_+0x59a,angle))return false;
        requested_angle_=angle;
        // A release can arrive before the next rendered frame. Keep the last
        // visible plan until the native preview replaces it, or return to zero.
        if(!angle)preview_ready_=false;
        return true;
    }

    bool width_owned() const {
        int width{};
        return preview_angle_owned()&&!word(root_+0x594)&&!word(root_+0x59c)&&
            get(root_+0x590,width)&&width==width_steps_;
    }
    unsigned width_file_budget() const {
        unsigned largest{};
        auto read_count=[&](uintptr_t unit) {
            unsigned files{};
            // The stock frontage policy uses the current soldier count as
            // its upper bound. Reject implausible counts before conversion.
            if(!unit||!get(unit+0x384,files)||files>65535)return false;
            largest=std::max(largest,files);
            return true;
        };
        for(const auto& member:members_) {
            if(!member.group) {
                if(!read_count(member.object))return 0;
                continue;
            }
            // Native 2fc880/2feac0 enumerate these same two group vectors.
            for(unsigned offset:{0x20u,0x60u}) {
                unsigned count{};
                uintptr_t entries{};
                if(!get(member.object+offset,count)||count>max_selection||
                   !get(member.object+offset+4,entries)||(count&&!entries))return 0;
                for(unsigned i=0;i<count;++i)
                    if(!read_count(word(entries+i*4)))return 0;
            }
        }
        return largest;
    }
    bool set_width(float fraction) {
        if(!std::isfinite(fraction)||std::fabs(fraction)>1.f||!width_owned())return false;
        int steps=0;
        if(fraction!=0.f) {
            const int amount=std::max(1,int(std::lround(std::fabs(fraction)*width_file_budget_)));
            steps=fraction>0.f?amount:-amount;
        }
        if(steps==width_steps_)return true;
        // The stock width renderer sets +598 after validating the new ghost.
        // An older cursor or width preview cannot approve a changed size.
        if(!set(root_+0x598,static_cast<unsigned char>(0))||!set(root_+0x590,steps))return false;
        width_steps_=steps;
        return true;
    }
    bool dispatch_width(uintptr_t queue,unsigned before) const {
        const unsigned action=width_steps_>0?0xf:0x10;
        __try {
            // The stock Increase Rank/File release passes this width and
            // action to the existing battle dispatcher after its ghost gate.
            using Dispatch=void(__thiscall*)(uintptr_t,unsigned,float);
            reinterpret_cast<Dispatch>(image_+0x10dfb80)(root_,action,float(std::abs(width_steps_)));
        } __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
        unsigned after{};
        return word(env_+0x28048)==queue&&get(queue+4+queue_capacity,after)&&
            after>before&&after<=queue_capacity;
    }
    template<class Current> bool commit_width(Current current) {
        const auto queue=word(env_+0x28048);
        unsigned before{};
        if(!queue||!get(queue+4+queue_capacity,before))return false;
        return deck175::native_order_ready(image_,root_,env_,word,current,true)&&
            dispatch_width(queue,before);
    }
    template<class Current> bool release_gesture(Current current) {
        const auto now=GetTickCount64();
        if(!release_tick_||now-release_tick_>180||now-gesture_tick_>450) {
            cancel();
            return false;
        }
        unsigned char ghost{};
        const bool valid_ghost=get(root_+0x598,ghost)&&ghost==1;
        bool committed=false;
        if(translating_&&translation_owned()&&
           owns_hook(move_preview_target_,reinterpret_cast<void*>(&move_preview_hook))) {
            if(!preview_ready_||!valid_ghost)return false;
            committed=commit_destinations(release_append_,current);
            if(committed)play_move_cue();
        } else if(resizing_&&width_steps_&&width_owned()) {
            if(!valid_ghost)return false;
            committed=commit_width(current);
        }
        cancel();
        return committed;
    }

    bool serialize(const Member& member,const Destination& destination,uintptr_t group_table,uintptr_t commander,
                   unsigned key,unsigned flags,bool append,unsigned char* output,unsigned& length) {
        alignas(16) unsigned char selection[0x44]{},order[0x80]{},context[16]{},packet[queue_capacity+4]{};
        bool order_live=false,context_live=false,ok=false;
        length=0;
        __try {
            memcpy(selection,&group_table,4);
            memcpy(selection+0x34,&commander,4);
            const SelectionList list{1,1,reinterpret_cast<uintptr_t>(&member.object)};
            memcpy(selection+(member.group?4:0x10),&list,sizeof list);
            auto running=reinterpret_cast<unsigned(__thiscall*)(const void*)>(image_+0x4596f0)(selection)&255u;
            using Construct=void*(__thiscall*)(void*,const void*,const terrain224::Point*,unsigned,unsigned,unsigned);
            // The oriented move stores the native queue flag after the run flag.
            reinterpret_cast<Construct>(image_+0x255680)(order,selection,&destination.point,destination.facing,running,append);
            order_live=true;
            uintptr_t table{};
            memcpy(&table,order,4);
            if(table==image_+0x15cffe0) {
                reinterpret_cast<void(__thiscall*)(void*,void*,unsigned,unsigned)>(image_+0x1cc200)(context,packet,key,flags);
                context_live=true;
                if(reinterpret_cast<unsigned(__thiscall*)(void*,void*)>(image_+0x2e6eb0)(order,context)) {
                    reinterpret_cast<void(__thiscall*)(void*)>(image_+0x1d0d40)(context);
                    context_live=false;
                    memcpy(&length,packet+queue_capacity,4);
                    if(length>=7&&length<=queue_capacity) {
                        unsigned bytes=unsigned(packet[0])|(unsigned(packet[1])<<8);
                        unsigned descriptor=unsigned(packet[2])<<24|unsigned(packet[3])<<16|unsigned(packet[4])<<8|packet[5];
                        if(bytes==length&&descriptor==key) { memcpy(output,packet,length);ok=true; }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) { ok=false; }
        if(context_live) {
            __try { reinterpret_cast<void(__thiscall*)(void*)>(image_+0x1d0d40)(context); }
            __except(EXCEPTION_EXECUTE_HANDLER) { ok=false; }
        }
        if(order_live) {
            __try { reinterpret_cast<void(__thiscall*)(void*)>(image_+0x1d10f0)(order+4); }
            __except(EXCEPTION_EXECUTE_HANDLER) { ok=false; }
        }
        return ok;
    }
    template<class Current> bool commit_destinations(bool append,Current current) {
        if(!current()||!selection_live())return false;
        const auto commander=word(owner_+0xe4);
        const auto registry=word(image_+0x18d9000);
        const auto entries=word(image_+0x18d9004);
        const auto key=word(image_+0x192d014);
        const auto queue=word(env_+0x28048);
        if(!registry||!entries||entries>4096||!key||!queue)return false;
        unsigned char flags{};
        bool found=false;
        for(unsigned i=0;i<entries;++i) {
            unsigned descriptor{};
            if(!get(registry+i*28,descriptor))return false;
            if(descriptor==key) { found=get(registry+i*28+0x10,flags);break; }
        }
        if(!found)return false;
        std::array<unsigned char,queue_capacity> staged{},packet{};
        unsigned total=0;
        for(size_t i=0;i<members_.size();++i) {
            unsigned length{};
            if(!serialize(members_[i],destinations_[i],image_+0x15c4da0,commander,key,flags,append,packet.data(),length)||
               length>queue_capacity-total)return false;
            memcpy(staged.data()+total,packet.data(),length);
            total+=length;
        }
        unsigned used{};
        if(!current()||!selection_live()||word(owner_+0xe4)!=commander||word(env_+0x28048)!=queue||
           !get(queue+4+queue_capacity,used)||used>queue_capacity||total>queue_capacity-used)return false;
        SIZE_T written{};
        if(!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(queue+4+used),
                               staged.data(),total,&written)||written!=total)return false;
        MemoryBarrier();
        InterlockedExchange(reinterpret_cast<volatile LONG*>(queue+4+queue_capacity),LONG(used+total));
        return true;
    }
    bool native_audio_address(uintptr_t address,bool executable) const {
        MEMORY_BASIC_INFORMATION info{};
        if(!address||!VirtualQuery(reinterpret_cast<void*>(address),&info,sizeof info)||
           reinterpret_cast<uintptr_t>(info.AllocationBase)!=image_||info.Type!=MEM_IMAGE||
           info.State!=MEM_COMMIT||(info.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        return !executable||(info.Protect&(PAGE_EXECUTE|PAGE_EXECUTE_READ|
                                             PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY));
    }
    void play_move_cue() {
        if(!control_live()||!native214::admit(image_,64)||!word(env_+0x2804c))return;
        __try {
            const auto audio=reinterpret_cast<uintptr_t(__thiscall*)(uintptr_t)>(image_+0x31e6d0)(env_);
            const auto table=word(audio);
            if(!native_audio_address(table,false))return;
            const auto play=word(table+0x18);
            if(!native_audio_address(play,true))return;
            // The original 2e94c0 UI order path uses this constructor and slot.
            // This is the non-positional BATTLE_UI_MOVE_EVENT.
            alignas(4) unsigned char args[0x30]{};
            unsigned result[4]{};
            using Construct=void*(__thiscall*)(void*,const void*,unsigned,unsigned,float,float,
                                               unsigned,unsigned,float,unsigned,unsigned);
            auto* cue=reinterpret_cast<Construct>(image_+0x269240)(args,
                reinterpret_cast<const void*>(image_+0x1931f64),word(image_+0x17988d8),
                0,-1.f,-1.f,1,0,1.f,0,1);
            reinterpret_cast<void(__thiscall*)(uintptr_t,void*,unsigned,const void*)>(play)(
                audio,result,0x12e,cue);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }
    void forget() {
        image_=root_=env_=ui_=widget_=control_=owner_=0;
        thread_=0;
        lists_={};
        members_.clear();
        requested_angle_=planned_angle_=0;
        planned_distance_=0;
        width_steps_=0;
        width_file_budget_=0;
        starting_=rotating_=translating_=resizing_=plan_valid_=preview_ready_=release_append_=false;
        gesture_tick_=release_tick_=0;
    }
public:
    static void disable_preview_hook() {
        if(owns_hook(preview_target_,reinterpret_cast<void*>(&preview_hook)))
            MH_DisableHook(reinterpret_cast<void*>(preview_target_));
        if(owns_hook(move_preview_target_,reinterpret_cast<void*>(&move_preview_hook)))
            MH_DisableHook(reinterpret_cast<void*>(move_preview_target_));
    }
    void cancel() {
        if(preview_owner_==this)preview_owner_=nullptr;
        if(control_live()) {
            if(rotating_&&preview_angle_owned()&&requested_angle_)set(root_+0x59a,uint16_t(0));
            else if(translating_&&word(control_+0x40)==14)cancel_move();
            else if(resizing_&&width_steps_&&width_owned())cancel_move();
        }
        forget();
    }
    bool pending_release() const { return release_tick_!=0; }
    template<class Current> bool sample(uintptr_t image,uintptr_t root,uintptr_t env,
                                       const deck216::FineIntent& input,Current current,bool pointer_busy=false) {
        if(input.cancel||!current()) { cancel();return false; }
        if(input.began) {
            cancel();
            image_=image;root_=root;env_=env;thread_=GetCurrentThreadId();
            if(!native214::admit(image,input.resizing?265:9)||!find_compass()||!capture_selection()) {
                forget();
                return false;
            }
            starting_=true;
            gesture_tick_=GetTickCount64();
        }
        if(!control_||!selection_live()) { cancel();return false; }
        if(starting_) {
            if(!input.active||GetTickCount64()-gesture_tick_>450) { cancel();return false; }
            // LT may still be releasing its cursor order on this same UI tick.
            if(pointer_busy||!preview_fields_owned())return true;
            starting_=false;
            if(input.resizing) {
                width_file_budget_=width_file_budget();
                resizing_=width_file_budget_!=0;
                if(resizing_&&set_width(input.width_fraction))return true;
            } else if(input.turning) {
                if(begin_rotation())return true;
            } else {
                const float magnitude=std::hypot(input.move.x,input.move.y);
                if(std::isfinite(magnitude)&&magnitude>=.99f&&magnitude<=1.01f&&begin_translation(input.move))return true;
            }
            cancel();
            return false;
        }
        if(pointer_busy) { cancel();return false; }
        if(pending_release())return release_gesture(current);
        if(input.turning!=rotating_||input.resizing!=resizing_) { cancel();return false; }
        if(!input.released) {
            if(translating_&&translation_owned()) { gesture_tick_=GetTickCount64();return true; }
            if(rotating_&&turn_to(input.turn))return true;
            if(resizing_&&set_width(input.width_fraction)) { gesture_tick_=GetTickCount64();return true; }
            cancel();
            return false;
        }
        if(!rotating_) {
            // Some releases arrive between the original preview reset and
            // the next battle draw. Keep the exact gesture through that draw.
            release_tick_=GetTickCount64();
            release_append_=input.append;
            return release_gesture(current);
        }
        const bool committed=preview_ready_&&requested_angle_&&planned_angle_&&
            GetTickCount64()-gesture_tick_<=450&&preview_fields_owned()&&
            owns_hook(preview_target_,reinterpret_cast<void*>(&preview_hook))&&
            commit_destinations(input.append,current);
        if(committed)play_move_cue();
        cancel();
        return committed;
    }
};
}
