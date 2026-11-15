#pragma once

#include "../../../deck/commander76.h"
#include "../../../deck/commander_display76.h"
#include "../../shogun2-possession-cleanup-v46/src/hud_layout108.h"

static commander76::Battle commander_battle76() {
    __try {
        if (!commander76::installed || !enabled || !state || !state->ready || state->stop)
            return {};
        const auto root = field<uintptr_t>(base, 0x1bc8180);
        if (!root || field<uintptr_t>(root, 0) != base + 0x170d284) return {};
        const auto environment = field<uintptr_t>(root, 0x110);
        const auto common = environment ? field<uintptr_t>(environment, 8) : 0;
        const auto simulation = common ? field<uintptr_t>(common, 0xb4) : 0;
        if (!simulation || field<uintptr_t>(environment, 0x28248) ||
            field<unsigned char>(environment, 0x28254)) return {};
        const auto phase = field<unsigned>(simulation, 0x14);
        return phase >= 5 && phase <= 8 ? commander76::Battle{environment, common, simulation} : commander76::Battle{};
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return {};
    }
}

static commander76::Battle commander_rts76(commander76::Battle battle) {
    __try {
        if (!battle.environment || state->mode != IDLE ||
            field<unsigned char>(battle.environment, 0x281e8)) return {};
        const auto manager = field<uintptr_t>(battle.environment, 0x281fc);
        if (!manager || field<uintptr_t>(manager, 0) != base + 0x15d26fc) return {};
        const auto camera = field<uintptr_t>(manager, 0x8e0);
        if (!camera || camera != field<uintptr_t>(manager, 0x8cc) ||
            field<uintptr_t>(camera, 0) != base + 0x15d1a74) return {};
        return battle;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return {};
    }
}

static commander76::Battle commander_rts76() {
    return commander_rts76(commander_battle76());
}

static bool commander_overlay76(unsigned generation, bool legend) {
    const auto current = commander76::exchange.state();
    if (!generation || current.possessed || current.generation != generation ||
        GetForegroundWindow() != input_window || native_modal214() || !commander_rts76().environment)
        return false;
    commander76::Display frame;
    if (!commander76::read_display(GetCurrentProcessId(), frame) || frame.lease != generation ||
        !(frame.flags & commander76::Rts)) return false;
    if (!legend) return (frame.flags & (commander76::PcWheel | commander76::DeckWheel)) != 0;
    uintptr_t root{};
    unsigned char hidden{};
    return current.available && native214::value(base + 0x1bc8180, root) &&
        root && native214::value(root + 0xca0, hidden) && !hidden;
}

static constexpr UINT COMMANDER_MESSAGE76 = WM_APP + 0x53e;
static_assert(COMMANDER_MESSAGE76 != COMMAND_CANCEL_MESSAGE100, "Commander message overlaps unit orders");
static commander76::DisplayWriter commander_display76;
struct CommanderInput76 {
    SRWLOCK lock = SRWLOCK_INIT;
    uint64_t alt_time{};
    float x{}, y{};
    commander76::WheelPointer pointer{};
    int sector{-1};
    bool alt{}, cancelled{}, rts{}, right_captured{}, left_captured{}, orders_page{};
    commander76::OrderClicks order_clicks;
    uint64_t right_cancelled_at{}, left_confirmed_at{};
    unsigned wheel_generation{};
    struct Event { UINT message; WPARAM key; LPARAM flags; commander76::UnitOrder order; };
    Event pending[12]{};
    unsigned count{}, generation{}, feedback_lease{};
    commander76::OrderFeedback feedback;
    WPARAM rts_return{};
};
static CommanderInput76 commander_input76;

static void commander_cancel76() {
    AcquireSRWLockExclusive(&commander_input76.lock);
    commander_input76.cancelled = true;
    commander_input76.alt = false;
    commander_input76.order_clicks.reset();
    InterlockedExchange(&commander76::pc_wheel, 0);
    ReleaseSRWLockExclusive(&commander_input76.lock);
}
static void commander_cursor76(HWND window, POINT point, unsigned targets, CommanderInput76& input) {
    input.sector=-1;input.pointer={};
    RECT rect{};
    if(!GetClientRect(window,&rect)||rect.right<=0||rect.bottom<=0)return;
    const float x=float(point.x)-rect.right*.5f, y=float(point.y)-rect.bottom*.48f;
    const float scale=hudlayout108::scale(float(rect.right),float(rect.bottom));
    const float radius=commander76::wheel_radius(rect.right/scale,rect.bottom/scale)*scale;
    input.pointer=commander76::wheel_direction(x,y,radius*.34f);
    if(std::hypot(x,y)<=radius)input.sector=commander76::WheelLayout(targets).point(x,y,radius*.34f);
}
static void commander_relative_cursor103(CommanderInput76& input,unsigned targets){
    input.pointer=commander76::wheel_direction(input.x,input.y,26.f);
    input.sector=input.orders_page?commander76::order_point(input.x,input.y,26.f):
        commander76::WheelLayout(targets).point(input.x,input.y,26.f);
}
static void command_window_wheel101(commander76::UnitOrder order);
static void commander_order101(HWND window,commander76::UnitOrder order){
    if(order.sector<0||!window||window!=input_window||GetForegroundWindow()!=window||
       !keyboard_fps84()||GetCurrentThreadId()!=state->window_thread||dead_view()||charge_input_locked146())return;
    auto& input=commander_input76;
    const auto info=commander76::exchange.state();
    if(!input.count&&(!info.formation||order.sector==3)){
        command_window_wheel101(order);
        return;
    }
    if(!input.count)input.generation=state->lease_generation;
    if(input.count<12)input.pending[input.count++]={0,0,0,order};
    if(info.formation)commander76::exchange.take_for_order();
}
static void commander_ack_order102(unsigned sector,bool run=false){
    auto& input=commander_input76;
    AcquireSRWLockExclusive(&input.lock);
    if(input.feedback_lease!=state->lease_generation){
        input.feedback={};
        input.feedback_lease=state->lease_generation;
    }
    input.feedback.confirm(sector,run,GetTickCount64());
    ReleaseSRWLockExclusive(&input.lock);
}
static void commander_tick_orders101(bool can_issue){
    if(!commander76::pc_wheel)return;
    const auto info=commander76::exchange.state();
    auto& input=commander_input76;
    commander76::UnitOrder order;
    AcquireSRWLockExclusive(&input.lock);
    if(!can_issue||input.cancelled||input.wheel_generation!=info.generation)
        input.order_clicks.reset();
    else if(input.alt&&input.orders_page)
        order=input.order_clicks.finish(GetTickCount64(),GetDoubleClickTime());
    ReleaseSRWLockExclusive(&input.lock);
    commander_order101(input_window,order);
}
static bool commander_mouse76(HWND window, UINT message, WPARAM& buttons, LPARAM point) {
    if (window != input_window) return false;
    const bool left = message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK;
    const bool right = message == WM_RBUTTONDOWN || message == WM_RBUTTONUP || message == WM_RBUTTONDBLCLK;
    if (!left && !right && message != WM_MOUSEMOVE) return false;
    const auto now = GetTickCount64();
    const auto info = commander76::exchange.state();
    const bool focused = commander76::pc_wheel && GetForegroundWindow() == window && !native_modal214();
    auto& input = commander_input76;
    AcquireSRWLockExclusive(&input.lock);
    const bool active = focused && input.alt && !input.cancelled &&
        input.wheel_generation == info.generation && now - input.alt_time >= 220;
    bool consumed = false;
    int choice = -1;
    std::array<commander76::UnitOrder,2> orders{};
    if (message == WM_MOUSEMOVE) {
        if (input.right_captured) buttons &= ~WPARAM(MK_RBUTTON);
        if (input.left_captured) buttons &= ~WPARAM(MK_LBUTTON);
        if (active && input.rts) {
            commander_cursor76(window,{short(LOWORD(point)),short(HIWORD(point))},info.targets,input);
            consumed = true;
        }
    } else {
        const bool up = message == WM_RBUTTONUP || message == WM_LBUTTONUP;
        const bool double_click = message == WM_RBUTTONDBLCLK || message == WM_LBUTTONDBLCLK;
        bool& captured = left ? input.left_captured : input.right_captured;
        auto& prior = left ? input.left_confirmed_at : input.right_cancelled_at;
        if (up) {
            consumed = captured;
            captured = false;
        } else {
            const bool same_click = double_click && prior && now - prior <= GetDoubleClickTime();
            consumed = active || same_click;
            captured = consumed;
            if (!double_click) prior = active ? now : 0;
            if (active && right) {input.cancelled = true;input.order_clicks.reset();}
            if (active && left) {
                if (input.rts) commander_cursor76(window,{short(LOWORD(point)),short(HIWORD(point))},info.targets,input);
                if(input.orders_page)orders=input.order_clicks.sample(input.sector,true,now,GetDoubleClickTime());
                else choice = input.sector;
            }
        }
    }
    ReleaseSRWLockExclusive(&input.lock);
    if (choice >= 0 && GetForegroundWindow() == window && !native_modal214())
        commander76::exchange.request(choice, info.generation);
    for(const auto order:orders)commander_order101(window,order);
    return consumed;
}

static bool commander_blocks_mouse76(int button) {
    AcquireSRWLockShared(&commander_input76.lock);
    const auto& input = commander_input76;
    const bool captured = button == VK_LBUTTON ? input.left_captured : input.right_captured;
    const bool blocked = captured || (input.alt && !input.cancelled);
    ReleaseSRWLockShared(&commander_input76.lock);
    return blocked;
}

static bool commander_point76(float dx, float dy) {
    if (!commander76::pc_wheel) return false;
    const auto info = commander76::exchange.state();
    AcquireSRWLockExclusive(&commander_input76.lock);
    auto& input = commander_input76;
    if (input.alt && !input.cancelled && !input.rts) {
        input.x+=dx;input.y+=dy;
        const float distance=std::hypot(input.x,input.y);
        if(distance>140.f){input.x*=140.f/distance;input.y*=140.f/distance;}
        commander_relative_cursor103(input,info.targets);
    }
    ReleaseSRWLockExclusive(&commander_input76.lock);
    return true;
}
static void commander_publish76(bool focused) {
    const auto battle = commander_battle76();
    const auto rts_battle = commander_rts76(battle);
    commander76::exchange.watch_rts(battle);
    const auto info = commander76::exchange.state();
    const bool rts = rts_battle.environment && info.generation && !info.possessed;
    const bool fps = info.possessed && keyboard_fps84() && !dead_view();
    const bool valid = focused && state && !native_modal214() && (rts || fps);
    if (!valid && commander76::pc_wheel) commander_cancel76();
    unsigned flags = 0;
    int sector = -1;
    commander76::OrderFeedback orders;
    commander76::WheelPointer pointer;
    if (valid) {
        if (info.available) flags |= commander76::Available;
        if (info.army) flags |= commander76::ArmyAI;
        if (info.formation) flags |= commander76::FormationAI;
        if (info.army_mixed) flags |= commander76::ArmyMixed;
        sector = InterlockedCompareExchange(&commander76::deck_sector, 0, 0);
        pointer=commander76::deck_pointer.load();
        if (rts) {
            flags |= commander76::Rts;
            if (commander76::deck_rts_wheel) flags |= commander76::DeckWheel;
        }
        AcquireSRWLockShared(&commander_input76.lock);
        if(fps&&commander_input76.feedback_lease==state->lease_generation)orders=commander_input76.feedback;
        if (commander_input76.alt && !commander_input76.cancelled &&
            commander_input76.wheel_generation == info.generation && commander_input76.rts == rts &&
            GetTickCount64() - commander_input76.alt_time >= 220) {
            flags |= commander76::PcWheel;
            if(commander_input76.orders_page)flags|=commander76::PcOrders;
            sector = commander_input76.sector;
            pointer=commander_input76.pointer;
        }
        ReleaseSRWLockShared(&commander_input76.lock);
    }
    commander_display76.publish(valid ? (rts ? info.generation : state->lease_generation) : 0, flags, sector, info.targets, info.ai, orders,pointer);
}
static bool commander_window76(HWND window, UINT message, WPARAM key, LPARAM flags) {
    auto& input = commander_input76;
    if (window != input_window || !state) return false;
    if (message == WM_KILLFOCUS || (message == WM_ACTIVATEAPP && !key) || message == WM_NCDESTROY) {
        commander_cancel76();
        input.count = 0;
        return false;
    }
    if (GetForegroundWindow() != window || native_modal214()) return false;
    if(input.count && input.generation != state->lease_generation)input.count=0;
    const bool down = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;
    const bool up = message == WM_KEYUP || message == WM_SYSKEYUP;
    const bool alt = key == VK_LMENU || (key == VK_MENU && !(flags & (1L << 24)));
    const auto rts_battle = alt && (down || up) ? commander_rts76() : commander76::Battle{};
    if (rts_battle.environment) commander76::exchange.watch_rts(rts_battle);
    const auto info = commander76::exchange.state();
    const bool rts = rts_battle.environment && !info.possessed;
    const bool fps = info.possessed && keyboard_fps84() && !dead_view();
    if (alt && (down || up) && (((rts || fps) && info.generation) || commander76::pc_wheel)) {
        commander76::UnitOrder order;
        AcquireSRWLockExclusive(&input.lock);
        if (down && !input.alt && !(flags & (1L << 30))) {
            input.alt = true;
            input.cancelled = false;
            input.rts = rts;
            input.orders_page=false;
            input.order_clicks.reset();
            input.wheel_generation = info.generation;
            input.alt_time = GetTickCount64();
            input.x = input.y = 0;
            input.sector = -1;input.pointer={};
            POINT point{};
            if(rts&&GetCursorPos(&point)&&ScreenToClient(window,&point))commander_cursor76(window,point,info.targets,input);
            InterlockedExchange(&commander76::pc_wheel, 1);
        } else if (up && input.alt) {
            if(!input.cancelled&&input.orders_page&&input.wheel_generation==info.generation)
                order=input.order_clicks.finish(GetTickCount64(),GetDoubleClickTime(),true);
            input.order_clicks.reset();
            input.alt = false;
            InterlockedExchange(&commander76::pc_wheel, 0);
        }
        ReleaseSRWLockExclusive(&input.lock);
        commander_order101(window,order);
        return true;
    }
    if (commander76::pc_wheel && (down || up)) {
        if(fps&&key==commander76::pc_page_key){
            commander76::UnitOrder order;
            AcquireSRWLockExclusive(&input.lock);
            if(down&&!(flags&(1L<<30))&&input.alt&&!input.cancelled&&
               input.wheel_generation==info.generation){
                order=input.order_clicks.finish(GetTickCount64(),GetDoubleClickTime(),true);
                input.orders_page=!input.orders_page;
                commander_relative_cursor103(input,info.targets);
            }
            ReleaseSRWLockExclusive(&input.lock);
            commander_order101(window,order);
            return true;
        }
        if (key == VK_ESCAPE) {
            AcquireSRWLockExclusive(&input.lock);
            if (down) {input.cancelled = true;input.order_clicks.reset();}
            const bool consume = !input.rts;
            ReleaseSRWLockExclusive(&input.lock);
            return consume;
        }
    }
    if (!fps) return false;
    const bool order_key = key == 'Q' || key == 'E' || key == 'Z' || key == 'C';
    const bool pointer = message == WM_MOUSEMOVE && input.count != 0;
    if (!(order_key && (down || up)) && !pointer) return false;
    if (!info.formation && !input.count) return false;
    if (down && (flags & (1L << 30))) return true;
    if (!input.count) {
        if (!down) return true;
        input.generation = state->lease_generation;
    }
    if (pointer && input.pending[input.count - 1].message == WM_MOUSEMOVE)
        --input.count;
    if (input.count == 12) return true;
    input.pending[input.count++] = {message, key, flags};
    if (info.formation) commander76::exchange.take_for_order();
    return true;
}
static void commander_wake76(HWND window) {
    auto& input = commander_input76;
    if (window != input_window || !state) return;
    const auto battle = commander_battle76();
    if (battle.environment) {
        const auto returned = commander76::exchange.take_ungroup(battle);
        for (unsigned i = 0; i < returned.count; ++i)
            commander76::ungroup_formation(base, battle, returned.items[i]);
    }
    const auto info = commander76::exchange.state();
    if (!info.rts_wait && input.rts_return) {
        const auto generation = input.rts_return;
        input.rts_return = 0;
        restore_rts_on_window90(uint32_t(generation));
    }
    if (info.formation || !input.count) return;
    const auto count = input.count;
    input.count = 0;
    if (!info.available || !keyboard_fps84() || dead_view() || charge_input_locked146() ||
        input.generation != state->lease_generation || GetForegroundWindow() != window) return;
    for (unsigned i = 0; i < count; ++i) {
        const auto event = input.pending[i];
        if(event.order.sector>=0)command_window_wheel101(event.order);
        else command_window100(window, event.message, event.key, event.flags);
    }
}

