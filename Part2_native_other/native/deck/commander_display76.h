#pragma once

#include <windows.h>
#include <cstdint>
#include <atomic>
#include "commander_wheel84.h"

namespace commander76 {
inline volatile LONG pc_wheel, deck_rts_wheel;
inline volatile LONG deck_sector = -1;
inline std::atomic<WheelPointer> deck_pointer{WheelPointer{}};
inline constexpr unsigned rts_overlay_key = 0x43525453;
struct Display {
    unsigned pid{}, lease{}, flags{};
    int sector{-1};
    unsigned targets{}, ai{};
    uint64_t tick{};
    volatile LONG sequence{};
    OrderFeedback orders{};
    WheelPointer pointer{};
};
enum : unsigned { Available = 1, ArmyAI = 2, FormationAI = 4, PcWheel = 8, Rts = 16, DeckWheel = 32, ArmyMixed = 64, PcOrders = 128 };

inline void display_name(wchar_t (&name)[64], DWORD pid) {
    swprintf_s(name, L"Local\\S2UP-Commander103-%lu", pid);
}

class DisplayWriter {
    HANDLE mapping_{};
    Display* view_{};
public:
    bool open() {
        if (view_) return true;
        wchar_t name[64];
        display_name(name, GetCurrentProcessId());
        mapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Display), name);
        if (!mapping_) return false;
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            view_ = static_cast<Display*>(MapViewOfFile(mapping_, FILE_MAP_WRITE, 0, 0, sizeof(Display)));
        if (!view_) { CloseHandle(mapping_); mapping_ = nullptr; }
        return view_ != nullptr;
    }
    void publish(unsigned lease, unsigned flags, int sector,
                 unsigned targets = 0, unsigned ai = 0, const OrderFeedback& orders = {}, WheelPointer pointer = {}) {
        if (!view_) return;
        InterlockedIncrement(&view_->sequence);
        MemoryBarrier();
        view_->pid = GetCurrentProcessId();
        view_->lease = lease;
        view_->flags = flags;
        view_->sector = sector;
        view_->targets = targets;
        view_->ai = ai;
        view_->orders = orders;
        view_->pointer = pointer;
        view_->tick = GetTickCount64();
        MemoryBarrier();
        InterlockedIncrement(&view_->sequence);
    }
};

inline bool read_display(DWORD pid, Display& out) {
    struct Reader {
        DWORD pid{};
        HANDLE mapping{};
        const Display* view{};
        uint64_t next{};
        void close() {
            if (view) UnmapViewOfFile(view);
            if (mapping) CloseHandle(mapping);
            view = nullptr;
            mapping = nullptr;
        }
        ~Reader() { close(); }
    };
    static thread_local Reader reader;
    auto now = GetTickCount64();
    if (reader.pid != pid) { reader.close(); reader.pid = pid; reader.next = 0; }
    if (!reader.view && now >= reader.next) {
        reader.next = now + 1000;
        wchar_t name[64];
        display_name(name, pid);
        reader.mapping = OpenFileMappingW(FILE_MAP_READ, FALSE, name);
        if (reader.mapping) {
            reader.view = static_cast<const Display*>(MapViewOfFile(reader.mapping, FILE_MAP_READ, 0, 0, sizeof(Display)));
            if (!reader.view) reader.close();
        }
    }
    if (!reader.view) return false;
    for (unsigned i = 0; i < 3; ++i) {
        const auto sequence = reader.view->sequence;
        if (sequence & 1) continue;
        MemoryBarrier();
        const auto value = *reader.view;
        MemoryBarrier();
        if (reader.view->sequence != sequence) continue;
        if (value.pid != pid || !value.lease || value.flags > 255 || ((value.flags&PcOrders)&&((value.flags&Rts)||!(value.flags&PcWheel))) || value.sector < -1 ||
            value.sector > last_sector || value.targets > valid_targets ||
            (value.ai & ~value.targets) || value.orders.hold_pace > 2 || !value.pointer.valid() || value.tick > now || now - value.tick > 300) return false;
        out = value;
        return true;
    }
    return false;
}
}

