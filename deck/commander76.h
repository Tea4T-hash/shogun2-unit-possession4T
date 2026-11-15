#pragma once

#include <windows.h>
#include <intrin.h>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "commander_wheel84.h"
#include "../outputs/shogun2-accuracy90-preview/vendor/include/MinHook.h"

namespace commander76 {

struct Member {
    uintptr_t unit{};
    unsigned id{};
    bool operator==(const Member& other) const { return unit == other.unit && id == other.id; }
};
struct Selection {
    Member units[128]{};
    unsigned count{};
    bool contains(Member unit) const {
        for (unsigned i = 0; i < count; ++i)
            if (units[i] == unit) return true;
        return false;
    }
    bool add(Member unit) {
        if (!unit.unit || !unit.id || contains(unit)) return true;
        if (count == 128) return false;
        units[count++] = unit;
        return true;
    }
};
struct Request {
    int sector{-1};
    bool give{};
    uint64_t stamp{};
    Member unit{};
};
struct AuthorityChanges {
    Selection before{};
    Request request{};
    Selection after{};
};

struct Battle {
    uintptr_t environment{}, common{}, simulation{}, formation{};
    unsigned formation_id{}, generation{};
    bool same_world(const Battle& other) const {
        return environment == other.environment && common == other.common &&
               simulation == other.simulation;
    }
};
struct GroupReturn {
    Member unit{};
    uintptr_t group{};
    unsigned number{};
};
struct GroupReturns {
    GroupReturn items[128]{};
    unsigned count{};
};
struct State {
    bool available{}, army{}, formation{}, returning{};
    unsigned generation{};
    bool possessed{}, rts_wait{};
    bool army_mixed{};
    unsigned targets{}, ai{};
    uint64_t stamps[group_count + 1]{};
};

class Exchange {
    SRWLOCK lock_ = SRWLOCK_INIT;
    Battle wanted_{};
    State current_{};
    Request request_{};
    Selection before_{}, manual_{};
    GroupReturns ungroup_{};
    unsigned next_{};

    void update(Battle battle) {
        battle.generation = ++next_;
        if (!battle.generation) battle.generation = ++next_;
        const bool rts_wait = !battle.formation && (current_.rts_wait ||
            (wanted_.formation && current_.returning));
        if (!wanted_.same_world(battle)) {
            request_ = {};
            before_ = manual_ = {};
            ungroup_ = {};
            current_ = {};
        }
        wanted_ = battle;
        current_.available = current_.formation = false;
        current_.generation = battle.generation;
        current_.possessed = battle.formation != 0;
        current_.rts_wait = rts_wait;
        current_.targets &= ~unit_bit;
        current_.ai &= ~unit_bit;
        current_.stamps[group_count] = 0;
    }
    void clear() {
        wanted_ = {};
        request_ = {};
        before_ = manual_ = {};
        ungroup_ = {};
        const bool returning = current_.returning;
        current_ = {};
        current_.returning = current_.rts_wait = returning;
    }

public:
    void enter(Battle battle) {
        AcquireSRWLockExclusive(&lock_);
        update(battle);
        ReleaseSRWLockExclusive(&lock_);
    }
    void watch_rts(Battle battle) {
        AcquireSRWLockExclusive(&lock_);
        if (!wanted_.formation) {
            if (!battle.environment) {
                if (wanted_.generation) clear();
            } else if (!wanted_.generation || !wanted_.same_world(battle)) update(battle);
        }
        ReleaseSRWLockExclusive(&lock_);
    }
    void leave_possession() {
        AcquireSRWLockExclusive(&lock_);
        if (wanted_.formation) {
            Battle battle = wanted_;
            battle.formation = 0;
            battle.formation_id = 0;
            update(battle);
        }
        ReleaseSRWLockExclusive(&lock_);
    }
    void leave() {
        AcquireSRWLockExclusive(&lock_);
        clear();
        ReleaseSRWLockExclusive(&lock_);
    }
    Battle wanted() {
        AcquireSRWLockShared(&lock_);
        auto battle = wanted_;
        ReleaseSRWLockShared(&lock_);
        return battle;
    }
    State state() {
        // Tea4T fork (kit v21): the AI Commander is switched off in this build. An empty state has
        // no generation and is never available, so no wheel opens and no order is ever requested.
        return State{};
    }
    void request(int sector, unsigned generation) {
        AcquireSRWLockExclusive(&lock_);
        const bool was_pending = request_.sector >= 0;
        if (generation && wanted_.generation == generation && current_.generation == generation &&
            current_.available && request_.sector < 0 && sector >= army_sector && sector <= last_sector) {
            if (sector == army_sector) request_ = {sector, !(current_.army || current_.army_mixed)};
            else {
                const bool unit = sector == unit_sector;
                const unsigned bit = unit ? group_count : unsigned(sector - first_group);
                if ((current_.targets & (1u << bit)) && (!unit || wanted_.formation)) {
                    request_ = {sector, !(current_.ai & (1u << bit)), current_.stamps[bit]};
                    if (unit) request_.unit = {wanted_.formation, wanted_.formation_id};
                }
            }
        }
        if (!was_pending && request_.sector >= 0) {
            // Move only selections that preceded this confirmation.
            before_ = manual_;
            manual_ = {};
        }
        ReleaseSRWLockExclusive(&lock_);
    }
    void take_for_order() {
        AcquireSRWLockExclusive(&lock_);
        if (wanted_.generation && current_.available && current_.formation)
            manual_.add({wanted_.formation, wanted_.formation_id});
        ReleaseSRWLockExclusive(&lock_);
    }
    void select(Battle battle, const Selection& selected) {
        AcquireSRWLockExclusive(&lock_);
        if (wanted_.generation && !wanted_.formation && wanted_.same_world(battle) &&
            current_.available && (current_.returning || request_.give)) {
            for (unsigned i = 0; i < selected.count; ++i) manual_.add(selected.units[i]);
        }
        ReleaseSRWLockExclusive(&lock_);
    }
    bool pending(Battle battle) {
        AcquireSRWLockShared(&lock_);
        const bool queued = wanted_.generation == battle.generation &&
            (request_.sector >= 0 || before_.count || manual_.count);
        ReleaseSRWLockShared(&lock_);
        return queued;
    }
    AuthorityChanges receive(unsigned generation) {
        AcquireSRWLockExclusive(&lock_);
        AuthorityChanges changes;
        if (wanted_.generation == generation) {
            changes = {before_, request_, manual_};
            before_ = manual_ = {};
            request_ = {};
        }
        ReleaseSRWLockExclusive(&lock_);
        return changes;
    }
    bool queue_ungroup(const Battle& battle, GroupReturn item) {
        AcquireSRWLockExclusive(&lock_);
        const bool queued = wanted_.generation && wanted_.same_world(battle) &&
            ungroup_.count < 128;
        if (queued) ungroup_.items[ungroup_.count++] = item;
        ReleaseSRWLockExclusive(&lock_);
        return queued;
    }
    GroupReturns take_ungroup(const Battle& battle) {
        AcquireSRWLockExclusive(&lock_);
        GroupReturns result{};
        if (wanted_.generation && wanted_.same_world(battle)) {
            result = ungroup_;
            ungroup_ = {};
        }
        ReleaseSRWLockExclusive(&lock_);
        return result;
    }
    void publish(State value) {
        AcquireSRWLockExclusive(&lock_);
        if (wanted_.generation == value.generation) current_ = value;
        else {
            current_.returning = value.returning;
            current_.rts_wait |= value.rts_wait && !current_.possessed;
        }
        ReleaseSRWLockExclusive(&lock_);
    }
};

inline Exchange exchange;

template<class T> inline T& field(uintptr_t object, size_t offset) {
    return *reinterpret_cast<T*>(object + offset);
}
using Tick = void(__thiscall*)(uintptr_t);
using UnitCall = void(__thiscall*)(uintptr_t, uintptr_t);

// The native script enrollment/release methods use only this embedded vector.
// Its capacity covers the admitted roster, so they never replace our storage.
struct Transfer {
    uintptr_t reserved[6]{};
    unsigned capacity{128}, count{};
    uintptr_t* members{};
};
static_assert(offsetof(Transfer, capacity) == 0x18 &&
              offsetof(Transfer, count) == 0x1c &&
              offsetof(Transfer, members) == 0x20, "Commander transfer layout");

struct Roster {
    uintptr_t army{};
    uintptr_t members[128]{};
    unsigned ids[128]{};
    unsigned count{};
    int find(uintptr_t unit, unsigned id) const {
        for (unsigned i = 0; i < count; ++i)
            if (members[i] == unit && ids[i] == id) return int(i);
        return -1;
    }
};

// The classic land card manager is root+5b4 (110fee00). Its card map
// links native unit serials to widgets (110cf3a0, 110ff5b0). Numbered
// groups own those same widgets (f843c0, f91e40), not simulation units.
inline uintptr_t land_cards(uintptr_t image, const Battle& battle) {
    const auto root = field<uintptr_t>(image, 0x1bc8180);
    if (!root || field<uintptr_t>(root, 0) != image + 0x170d284 ||
        field<uintptr_t>(root, 0x110) != battle.environment) return 0;
    const auto cards = field<uintptr_t>(root, 0x5b4);
    return cards && field<uintptr_t>(cards, 0) == image + 0x170d184 &&
        field<uintptr_t>(cards, 0x98) == root &&
        field<uintptr_t>(cards, 0x64) == battle.environment ? cards : 0;
}
inline uintptr_t card_for(uintptr_t manager, unsigned id) {
    const auto buckets = field<unsigned>(manager, 0x70);
    const auto rows = field<uintptr_t>(manager, 0x74);
    if (!buckets || buckets > 8192 || !rows) return 0;
    const auto bucket = rows + ((id ^ 0x4a545eedu) % buckets) * 16;
    auto node = field<uintptr_t>(bucket, 0);
    const auto end = field<uintptr_t>(bucket, 4);
    for (unsigned i = 0; node && node != end && i < 512; ++i) {
        if (field<unsigned>(node, 8) == id) return field<uintptr_t>(node, 0xc);
        node = field<uintptr_t>(node, 4);
    }
    return 0;
}
inline bool owned_land_group(uintptr_t image, uintptr_t manager, uintptr_t group) {
    // The land manager factory 10e0130 calls the derived constructor 10d6fc0.
    // It replaces the base group table (16e320c) before publishing the group.
    return group && field<uintptr_t>(group, 0) == image + 0x170d0e4 &&
        field<uintptr_t>(group, 4) == manager;
}
inline uintptr_t group_for(uintptr_t image, uintptr_t manager, unsigned number) {
    const auto count = field<unsigned>(manager, 0x40);
    const auto rows = field<uintptr_t>(manager, 0x44);
    if (number >= group_count || !rows || count > group_count ||
        field<int>(image, 0x17b8954 + number * 4) == -1) return 0;
    for (unsigned i = 0; i < count; ++i) {
        const auto group = field<uintptr_t>(rows, i * 4);
        if (owned_land_group(image, manager, group) &&
            field<unsigned>(group, 0x14) == number)
            return group;
    }
    return 0;
}
inline bool group_has_card(uintptr_t group, uintptr_t card) {
    if (!group || !card) return false;
    const auto count = field<unsigned>(group, 0xc);
    const auto cards = field<uintptr_t>(group, 0x10);
    if (count > 128 || !cards) return false;
    for (unsigned i = 0; i < count; ++i)
        if (field<uintptr_t>(cards, i * 4) == card) return true;
    return false;
}
inline GroupReturn numbered_group(uintptr_t image, const Battle& battle, Member unit) {
    __try {
        const auto manager = land_cards(image, battle);
        if (!manager) return {};
        const auto card = card_for(manager, unit.id);
        for (unsigned number = 0; number < group_count; ++number) {
            const auto group = group_for(image, manager, number);
            if (group_has_card(group, card)) return {unit, group, number};
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return {};
}
inline bool ungroup_formation(uintptr_t image, const Battle& battle, GroupReturn item) {
    __try {
        const auto unit = item.unit.unit;
        const auto manager = land_cards(image, battle);
        if (!unit || !manager || field<unsigned>(unit, 4) != item.unit.id) return false;
        const auto link = field<uintptr_t>(unit, 0x14);
        if (!link || field<uintptr_t>(link, 8) != battle.common) return false;
        const auto group = group_for(image, manager, item.number);
        const auto card = card_for(manager, item.unit.id);
        if (group != item.group || !group_has_card(group, card) ||
            field<uintptr_t>(field<uintptr_t>(group, 0), 8) != image + 0x10e2120) return false;
        // Ghidra 111010a0 uses the native UI manager removal. Its return
        // value reports group destruction, not whether this card was removed.
        using RemoveCard = bool(__thiscall*)(uintptr_t, uintptr_t, int);
        const auto id = field<int>(image, 0x17b8954 + item.number * 4);
        reinterpret_cast<RemoveCard>(image + 0xf97c20)(manager, card, id);
        return !group_has_card(group_for(image, manager, item.number), card);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
inline bool map_cards(uintptr_t manager, const Roster& roster, uintptr_t cards,
                      unsigned count, Selection& selection) {
    if (count > 128 || (count && !cards)) return false;
    uintptr_t identities[128]{};
    for (unsigned i = 0; i < roster.count; ++i) identities[i] = card_for(manager, roster.ids[i]);
    for (unsigned i = 0; i < count; ++i) {
        const auto card = field<uintptr_t>(cards, i * 4);
        unsigned match = roster.count;
        for (unsigned j = 0; card && j < roster.count; ++j)
            if (identities[j] == card) {
                if (match != roster.count) return false;
                match = j;
            }
        if (match == roster.count || !selection.add({roster.members[match], roster.ids[match]}))
            return false;
    }
    return true;
}
inline uint64_t target_members(uintptr_t image, const Battle& battle, const Roster& roster,
                               unsigned slot, Selection& out) {
    __try {
        if (slot == group_count) {
            if (!battle.formation || roster.find(battle.formation, battle.formation_id) < 0) return 0;
            out.add({battle.formation, battle.formation_id});
        } else {
            const auto manager = land_cards(image, battle);
            if (!manager) return 0;
            const auto group = group_for(image, manager, slot);
            if (!group) return 0;
            const auto count = field<unsigned>(group, 0xc);
            const auto cards = field<uintptr_t>(group, 0x10);
            if (!count || !map_cards(manager, roster, cards, count, out)) return 0;
        }
        uint64_t stamp = 14695981039346656037ull;
        for (unsigned i = 0; i < out.count; ++i) {
            stamp = (stamp ^ out.units[i].unit) * 1099511628211ull;
            stamp = (stamp ^ out.units[i].id) * 1099511628211ull;
        }
        return stamp ? stamp : 1;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

struct Signature { unsigned rva; const char* bytes; size_t size; };
#define COMMANDER_SIGNATURE(rva, bytes) {rva, bytes, sizeof(bytes) - 1}
inline constexpr Signature signatures[] = {
    COMMANDER_SIGNATURE(0xf91810, "\x83\xec\x2c\x55\x8b\x6c\x24\x34\x57\x8b\xf9"),
    COMMANDER_SIGNATURE(0xf91500, "\x8a\x44\x24\x08\x83\xec\x28"),
    COMMANDER_SIGNATURE(0xf9a320, "\x83\xec\x28\x53\x8b\xd9\x89\x5c\x24\x04"),
    COMMANDER_SIGNATURE(0xf97c20, "\x51\x53\x8b\x5c\x24\x10\x32\xc0\x88\x44\x24\x07"),
    COMMANDER_SIGNATURE(0x10e2120, "\x53\x8b\x5c\x24\x08\x56\x57\x53\x8b\xf9\xe8\x11\x57\xeb\xff"),
    COMMANDER_SIGNATURE(0xf97840, "\x8b\x54\x24\x04\x53\x56\x57\x8b\xf9\x8b\x77\x10"),
    COMMANDER_SIGNATURE(0x303430, "\xb8\x1c\x50\x00\x00\xe8\x76\x8d\x1f\x01"),
    COMMANDER_SIGNATURE(0x506ef0, "\x53\x56\x8b\xf1\x57\x8b\xbe\x5c\x06\x00\x00\x8b\x9e\x60\x06\x00\x00"),
    COMMANDER_SIGNATURE(0x506f01, "\xe8\xfa\x47\xfd\xff\x80\xbe\x34\x06\x00\x00\x00"),
    COMMANDER_SIGNATURE(0x4db700, "\x56\x57\x8b\xf9\x33\xd2\x8b\x47\x24\xc6\x87\x34\x06\x00\x00\x01"),
    COMMANDER_SIGNATURE(0x5465f0, "\x56\x8b\x74\x24\x08\xc6\x86\x01\x0c\x00\x00\x01\xc7\x86\xa8\x0e\x00\x00\x01\x00\x00\x00"),
    COMMANDER_SIGNATURE(0x546630, "\x8b\x41\x20\x8b\x51\x1c\x56\x8b\x74\x24\x08\x8d\x14\x90\x3b\xc2"),
    COMMANDER_SIGNATURE(0x525cb0, "\x53\x56\x57\xff\x74\x24\x10\x8b\xf9\x8d\x9f\x8c\x00\x00\x00"),
    COMMANDER_SIGNATURE(0x506fc2, "\xe8\xa9\x86\x01\x00"),
    COMMANDER_SIGNATURE(0x51f670, "\x81\xec\xb4\x00\x00\x00\x53\x55\x56\x57\x8b\xe9"),
    COMMANDER_SIGNATURE(0x51ffd8, "\xe8\xd3\x5c\x00\x00"),
    COMMANDER_SIGNATURE(0x514680, "\x83\xec\x08\x33\xc0\x89\x0c\x24\x89\x44\x24\x04\x39\x41\x48"),
    COMMANDER_SIGNATURE(0x523cf0, "\x53\x56\x8b\xf1\x57\x33\xff\x8b\x8e\xc0\x00\x00\x00"),
    COMMANDER_SIGNATURE(0x51a7a0, "\x56\x8b\x74\x24\x08\x57\x8b\xf9\x8d\x8e\xc4\x0c\x00\x00"),
    COMMANDER_SIGNATURE(0x21af60, "\x83\xec\x08\x53\x55\x8b\x6c\x24\x14\x33\xd2\x56"),
    COMMANDER_SIGNATURE(0x21b040, "\x51\x53\x8b\xc1\x55\x8b\x6c\x24\x10\x56"),
    COMMANDER_SIGNATURE(0x228e00, "\x56\x8b\xf1\x33\xc0\x8b\x4e\x10\xc6\x46\x18\x01"),
    COMMANDER_SIGNATURE(0x228d50, "\x57\x8b\xf9\x33\xc0\x8b\x57\x3c\x85\xd2\x74\x5e"),
    COMMANDER_SIGNATURE(0x50b1b0, "\x53\x55\x8b\x6c\x24\x0c\x33\xdb\x56\x57"),
    COMMANDER_SIGNATURE(0x1b9be0, "\x8b\x49\x0c\x85\xc9\x0f\x85\x05\xd3\x34\x00\xc3"),
};
inline constexpr Signature resume_signatures[] = {
    COMMANDER_SIGNATURE(0x509b10, "\x56\x8b\xf1\x57\x8b\x86\xc0\x00\x00\x00"),
    COMMANDER_SIGNATURE(0x520140, "\x56\x8b\xf1\x8d\x8e\x4c\x04\x00\x00\xe8\xb2\x6c\xfe\xff"),
    COMMANDER_SIGNATURE(0x50700b, "\x55\xbd\x0a\x00\x00\x00\x3b\xfb\x75\x1f\x8b\x86\x24\x06\x00\x00\x99\xf7\xfd\x83\xfa\x01\x74\x0a\x8a\x86\x37\x06\x00\x00\x84\xc0\x74\x07\x8b\xce\xe8\x0c\x91\x01\x00\x8b\x4e\x28\xe8\xd4\x2a\x00\x00"),
};
#undef COMMANDER_SIGNATURE

class Controller {
    uintptr_t image_{}, ai_{}, army_{};
    Battle battle_{};
    uintptr_t members_[128]{};
    unsigned serials_[128]{};
    Transfer delegates_{};
    bool yielded_{}, scope_ready_{}, refresh_on_resume_{};
    State display_{};
    HWND notification_{};
    UINT wake_{};
    bool prune_pending_{}, parent_return_pending_{}, admitted_{}, ungroup_pending_{};

    static bool live_world(uintptr_t image, const Battle& b) {
        if (!b.environment) return false;
        const auto root = field<uintptr_t>(image, 0x1bc8180);
        return root && field<uintptr_t>(root, 0) == image + 0x170d284 &&
            field<uintptr_t>(root, 0x110) == b.environment &&
            field<uintptr_t>(b.environment, 8) == b.common &&
            field<uintptr_t>(b.common, 0xb4) == b.simulation;
    }
    static bool playable(const Battle& b) {
        const auto phase = field<unsigned>(b.simulation, 0x14);
        return phase >= 5 && phase <= 8 &&
            !field<uintptr_t>(b.environment, 0x28248) &&
            !field<unsigned char>(b.environment, 0x28254);
    }
    bool read_roster(uintptr_t ai, const Battle& b, Roster& result) const {
        if (field<uintptr_t>(ai, 0) != b.environment) return false;
        const auto alliance = field<uintptr_t>(ai, 0x24);
        if (!alliance || field<uintptr_t>(alliance, 0xc) != ai) return false;
        const auto army_count = field<unsigned>(alliance, 0x20);
        const auto armies = field<uintptr_t>(alliance, 0x24);
        if (!armies || !army_count || army_count > 16 || (!army_ && army_count != 1)) return false;
        uintptr_t army = 0;
        for (unsigned i = 0; i < army_count; ++i) {
            const auto item = field<uintptr_t>(armies, 4 * i);
            if (!army_ || item == army_) army = item;
        }
        if (!army || field<unsigned char>(army, 0x270) != 1 ||
            field<unsigned>(army, 0x12c) || !field<uintptr_t>(ai, 0x28) ||
            !field<uintptr_t>(ai, 0x34)) return false;
        const auto count = field<unsigned>(army, 0x114);
        const auto units = field<uintptr_t>(army, 0x118);
        if (count > 128 || (count && !units)) return false;
        result.army = army;
        for (unsigned i = 0; i < count; ++i) {
            const auto u = field<uintptr_t>(units, i * 4);
            if (!u || field<uintptr_t>(u, 0x2a4) != army) return false;
            const auto link = field<uintptr_t>(u, 0x14);
            const auto id = field<unsigned>(u, 4);
            if (!link || field<uintptr_t>(link, 8) != b.common || !id || result.find(u, id) >= 0)
                return false;
            result.members[result.count] = u;
            result.ids[result.count++] = id;
        }
        return true;
    }
    static bool sole_human_army(const Battle& battle, uintptr_t army) {
        const auto count = field<unsigned>(battle.simulation, 0x88);
        const auto sides = field<uintptr_t>(battle.simulation, 0x8c);
        if (!count || count > 16 || !sides) return false;
        uintptr_t human{};
        for (unsigned i = 0; i < count; ++i) {
            const auto side = field<uintptr_t>(sides, i * 4);
            if (!side) return false;
            const auto size = field<unsigned>(side, 0x20);
            const auto rows = field<uintptr_t>(side, 0x24);
            if (size > 16 || (size && !rows)) return false;
            for (unsigned j = 0; j < size; ++j) {
                const auto member = field<uintptr_t>(rows, j * 4);
                if (!member) return false;
                if (!field<unsigned char>(member, 0x270)) continue;
                if (human) return false;
                human = member;
            }
        }
        return human == army;
    }
    bool scripted(uintptr_t ai) const {
        return field<uintptr_t>(ai, 0x65c) != field<uintptr_t>(ai, 0x660);
    }
    int owned(uintptr_t unit) const {
        for (unsigned i = 0; i < delegates_.count; ++i)
            if (members_[i] == unit) return int(i);
        return -1;
    }
    static bool transferred(uintptr_t unit) {
        return field<unsigned>(unit, 0xea8) == 1 && field<unsigned char>(unit, 0xc01) == 1;
    }
    static bool idle_authority(uintptr_t unit) {
        return !field<unsigned>(unit, 0xea8) && !field<unsigned char>(unit, 0xc01) &&
            !field<uintptr_t>(unit, 0xbec) && !field<uintptr_t>(unit, 0xbf0);
    }
    void forget(unsigned i) {
        for (unsigned j = i; j + 1 < delegates_.count; ++j) {
            members_[j] = members_[j + 1];
            serials_[j] = serials_[j + 1];
        }
        --delegates_.count;
    }
    void reset() {
        ai_ = army_ = 0;
        prune_pending_ = parent_return_pending_ = admitted_ = ungroup_pending_ = false;
        battle_ = {};
        delegates_.count = 0;
        yielded_ = scope_ready_ = refresh_on_resume_ = false;
        display_ = {};
    }
    bool script_owns(uintptr_t unit) const {
        auto node = field<uintptr_t>(ai_, 0x65c);
        const auto end = field<uintptr_t>(ai_, 0x660);
        for (unsigned n = 0; node != end; ++n) {
            if (!node || n >= 128) return true;
            const auto transfer = field<uintptr_t>(node, 0xc);
            if (transfer) {
                const auto size = field<unsigned>(transfer, 0x1c);
                const auto rows = field<uintptr_t>(transfer, 0x20);
                if (size > 128 || (size && !rows)) return true;
                for (unsigned i = 0; i < size; ++i)
                    if (field<uintptr_t>(rows, i * 4) == unit) return true;
            }
            node = field<uintptr_t>(node, 4);
        }
        return false;
    }
    bool synchronize(const Roster& roster) {
        for (unsigned i = delegates_.count; i > 0; --i) {
            const auto index = i - 1;
            if (roster.find(members_[index], serials_[index]) < 0)
                forget(index); // Removed object is no longer in this battle roster.
            else if (!transferred(members_[index]) || script_owns(members_[index])) {
                forget(index); // A different controller now owns it.
                yielded_ = true;
            }
        }
        for (unsigned i = 0; i < roster.count; ++i)
            if (owned(roster.members[i]) < 0 && !idle_authority(roster.members[i]))
                return false;
        return true;
    }
    void remove(unsigned i) {
        const auto u = members_[i];
        const auto count = delegates_.count;
        reinterpret_cast<UnitCall>(image_ + 0x546630)(reinterpret_cast<uintptr_t>(&delegates_), u);
        // Native erase compacts members. The parallel serials follow that erase.
        if (delegates_.count == count - 1) {
            for (unsigned j = i; j < delegates_.count; ++j) serials_[j] = serials_[j + 1];
            prune_pending_ = true;
            if (u == battle_.formation) parent_return_pending_ = true;
        }
    }
    void enroll(uintptr_t unit, unsigned id) {
        if (delegates_.count >= delegates_.capacity || owned(unit) >= 0 || !idle_authority(unit)) return;
        const auto count = delegates_.count;
        reinterpret_cast<UnitCall>(image_ + 0x5465f0)(reinterpret_cast<uintptr_t>(&delegates_), unit);
        if (delegates_.count == count + 1) {
            serials_[count] = id;
            if (!count) refresh_on_resume_ = true;
        }
    }
    void apply_manual(const Roster& roster, const Selection& selected) {
        for (unsigned i = 0; i < selected.count; ++i) {
            const auto member = selected.units[i];
            const int index = owned(member.unit);
            if (index >= 0 && serials_[index] == member.id && roster.find(member.unit, member.id) >= 0)
                remove(unsigned(index));
        }
    }
    void apply_request(const Roster& roster, Request request) {
        if (request.sector < 0) return;
        Selection selected;
        if (request.sector == army_sector) {
            for (unsigned i = 0; i < roster.count; ++i)
                selected.add({roster.members[i], roster.ids[i]});
        } else if (request.sector == unit_sector) {
            if (roster.find(request.unit.unit, request.unit.id) < 0) return;
            selected.add(request.unit);
        } else {
            const unsigned slot = unsigned(request.sector - first_group);
            if (!request.stamp || request.stamp != target_members(image_, battle_, roster, slot, selected)) return;
        }
        if (request.give) {
            for (unsigned i = 0; i < selected.count; ++i)
                enroll(selected.units[i].unit, selected.units[i].id);
        } else {
            apply_manual(roster, selected);
            if (request.sector == unit_sector && owned(request.unit.unit) < 0 &&
                !field<unsigned>(request.unit.unit, 0xea8) && !field<unsigned char>(request.unit.unit, 0xc01)) {
                const auto group = numbered_group(image_, battle_, request.unit);
                if (group.group && exchange.queue_ungroup(battle_, group)) ungroup_pending_ = true;
            }
        }
    }
    void update_display(const Roster& roster) {
        display_ = {};
        display_.army = roster.count && delegates_.count == roster.count;
        display_.army_mixed = delegates_.count && !display_.army;
        for (unsigned slot = 0; slot <= group_count; ++slot) {
            Selection selected;
            const auto stamp = target_members(image_, battle_, roster, slot, selected);
            if (!stamp) continue;
            const unsigned bit = 1u << slot;
            display_.targets |= bit;
            display_.stamps[slot] = stamp;
            for (unsigned i = 0; i < selected.count; ++i) {
                if (owned(selected.units[i].unit) < 0) continue;
                display_.ai |= bit;
                break;
            }
        }
    }
    void notify(HWND window, UINT message) const {
        if (window && message) PostMessageW(window, message, 0, 0);
    }

public:
    void configure(uintptr_t image, UINT wake) {
        image_ = image;
        wake_ = wake;
        delegates_.members = members_;
    }
    bool prepare_commands(const Battle& next) {
        return ai_ && next.generation && next.same_world(battle_) && exchange.pending(next) &&
            prepare(ai_, next, notification_);
    }
    uintptr_t retiring_ai() const { return prune_pending_ ? ai_ : 0; }
    bool admit_tick(uintptr_t ai) {
        admitted_ = ai_ == ai && delegates_.count && !scripted(ai);
        return admitted_;
    }
    bool admitted() const { return admitted_; }
    bool take_resume_refresh(uintptr_t ai, uintptr_t root) {
        if (!refresh_on_resume_ || !admitted_ || ai != ai_ ||
            field<uintptr_t>(ai, 0x28) != root) return false;
        refresh_on_resume_ = false;
        // The original scheduler already evaluates on opening and every tenth tick.
        return !field<unsigned char>(ai, 0x637) && field<int>(ai, 0x624) % 10 != 1;
    }
    bool needs_final_roster(uintptr_t ai) const {
        return ai_ == ai && !delegates_.count && prune_pending_ && !scripted(ai);
    }
    bool needs_prune(uintptr_t root) const {
        return prune_pending_ && ai_ && field<uintptr_t>(ai_, 0x28) == root;
    }
    void prune(uintptr_t root) {
        const auto operations = field<uintptr_t>(root, 0xc4);
        const auto count = field<unsigned>(root, 0xc0);
        for (unsigned i = 0; i < count; ++i)
            reinterpret_cast<Tick>(image_ + 0x514680)(field<uintptr_t>(operations, i * 4));
        prune_pending_ = parent_return_pending_ = false;
    }
    bool prepare(uintptr_t ai, Battle want, HWND window) {
        notification_ = window;
        if (ai_ && !live_world(image_, battle_)) {
            reset();
            exchange.publish({false, false, false, false, want.generation});
            notify(window, wake_);
        }
        if (ai_ && ai_ != ai) return false;
        const auto candidate = ai_ ? battle_ : want;
        if (!live_world(image_, candidate)) return false;
        Roster roster;
        if (!read_roster(ai, candidate, roster)) return false;
        const bool owns_army = sole_human_army(candidate, roster.army);
        const bool valid_want = want.generation && candidate.same_world(want) &&
            playable(want) && owns_army;
        if (!ai_) {
            if (!valid_want) return false;
            ai_ = ai;
            army_ = roster.army;
        }
        if (roster.army != army_) return false;
        const bool uncontested = synchronize(roster) && !scripted(ai) && owns_army &&
            field<unsigned>(field<uintptr_t>(ai, 0x24), 0x20) == 1;
        if (!uncontested) yielded_ = true;
        if (valid_want && want.generation != battle_.generation) battle_ = want;
        scope_ready_ = valid_want && !yielded_;
        display_ = {};
        if (!scope_ready_) {
            while (delegates_.count) {
                const unsigned count = delegates_.count;
                remove(count - 1);
                if (delegates_.count == count) break;
            }
            return true;
        }

        const auto changes = exchange.receive(want.generation);
        apply_manual(roster, changes.before);
        apply_request(roster, changes.request);
        apply_manual(roster, changes.after);
        update_display(roster);
        return true;
    }
    void finish() {
        const auto previous = exchange.state();
        const auto next = exchange.wanted();
        const bool continuing = next.generation && next.generation == battle_.generation;
        const bool formation = owned(battle_.formation) >= 0 || parent_return_pending_;
        const bool returning = delegates_.count != 0 || prune_pending_;
        admitted_ = false;
        const bool rts_wait = !next.formation && (prune_pending_ || (!continuing && returning));
        State value = continuing && scope_ready_ ? display_ : State{};
        value.available = continuing && scope_ready_;
        value.formation = continuing && formation;
        value.returning = returning;
        value.generation = next.same_world(battle_) && !continuing ? battle_.generation : next.generation;
        value.possessed = next.formation != 0;
        value.rts_wait = rts_wait;
        const bool ungroup = ungroup_pending_;
        ungroup_pending_ = false;
        if (!continuing && !returning) reset();
        exchange.publish(value);
        if (ungroup || previous.formation != value.formation || previous.returning != value.returning ||
            previous.available != value.available || previous.rts_wait != value.rts_wait)
            notify(notification_, wake_);
    }
};

inline Controller controller;
inline Tick scheduler{}, eligibility{};
inline UnitCall roster_update{};
inline uintptr_t engine{};
inline volatile LONG installed;
inline __declspec(thread) uintptr_t scoped_ai;
inline Tick update_operations{};

inline void __fastcall native_update_operations(uintptr_t root, void*) {
    // The native collector and context refresh have run. Evaluate once at the
    // original strategy boundary before advancing the existing operations.
    const auto ai = scoped_ai;
    if (ai && uintptr_t(_ReturnAddress()) == engine + 0x50703c &&
        controller.take_resume_refresh(ai, root))
        reinterpret_cast<Tick>(engine + 0x520140)(ai);
    update_operations(root);
}

inline void __fastcall native_eligibility(uintptr_t ai, void*) {
    const bool scoped = scoped_ai == ai && uintptr_t(_ReturnAddress()) == engine + 0x506f06;
    eligibility(ai);
    if (!scoped) return;
    if (controller.admit_tick(ai)) field<unsigned char>(ai, 0x634) = 0;
    else if (field<unsigned char>(ai, 0x634) && controller.needs_final_roster(ai)) {
        // Publish the native roster and prune released assignments without a
        // strategy tick against an empty army. Native eligibility stays human.
        reinterpret_cast<Tick>(engine + 0x51f670)(ai);
    }
}
inline void __fastcall native_roster_update(uintptr_t root, void*, uintptr_t roster) {
    roster_update(root, roster);
    if (scoped_ai && uintptr_t(_ReturnAddress()) == engine + 0x51ffdd && controller.needs_prune(root))
        controller.prune(root);
}
using Window = HWND(*)();
inline Window input_window;
inline void __fastcall native_scheduler(uintptr_t ai, void*) {
    if (!installed || scoped_ai) { scheduler(ai); return; }
    const auto next = exchange.wanted();
    bool prepared = false;
    __try { prepared = controller.prepare(ai, next, input_window()); }
    __except (EXCEPTION_EXECUTE_HANDLER) { exchange.leave(); }
    if (!prepared) { scheduler(ai); return; }
    scoped_ai = ai;
    __try { scheduler(ai); }
    __finally { scoped_ai = 0; }
    if (controller.admitted()) eligibility(ai);
    controller.finish();
}
using SelectCard = unsigned char(__thiscall*)(uintptr_t, uintptr_t, unsigned);
using SelectCards = void(__thiscall*)(uintptr_t, uintptr_t);
using CommandBatch = void(__thiscall*)(uintptr_t, uintptr_t, uintptr_t);
inline SelectCard select_card{}, select_group{};
inline SelectCards select_cards{};
inline CommandBatch command_batch{};
inline __declspec(thread) unsigned group_selection_depth;

inline bool chosen_card(uintptr_t manager, uintptr_t card) {
    const auto count = field<unsigned>(manager, 0x28);
    const auto rows = field<uintptr_t>(manager, 0x2c);
    if (count > 128 || (count && !rows)) return false;
    for (unsigned i = 0; i < count; ++i)
        if (field<uintptr_t>(rows, i * 4) == card) return true;
    return false;
}

inline void capture_selection(uintptr_t manager, uintptr_t rows, unsigned count, bool group) {
    const auto wanted = exchange.wanted();
    if (!installed || !wanted.environment || wanted.formation || !count || count > 128 || !rows)
        return;
    __try {
        const auto window = input_window();
        if (!window || GetForegroundWindow() != window ||
            GetCurrentThreadId() != GetWindowThreadProcessId(window, nullptr) ||
            land_cards(engine, wanted) != manager) return;
        Selection selection;
        const auto sides = field<uintptr_t>(wanted.simulation, 0x8c);
        const auto side_count = field<unsigned>(wanted.simulation, 0x88);
        if (!sides || !side_count || side_count > 16) return;
        uintptr_t army = 0;
        for (unsigned i = 0; i < side_count; ++i) {
            const auto side = field<uintptr_t>(sides, i * 4);
            if (!side) return;
            const auto armies = field<uintptr_t>(side, 0x24);
            const auto army_count = field<unsigned>(side, 0x20);
            if (army_count > 16 || (army_count && !armies)) return;
            for (unsigned j = 0; j < army_count; ++j) {
                const auto item = field<uintptr_t>(armies, j * 4);
                if (!item) return;
                if (!field<unsigned char>(item, 0x270)) continue;
                if (army) return;
                army = item;
            }
        }
        if (!army || field<unsigned>(army, 0x12c)) return;
        const auto size = field<unsigned>(army, 0x114);
        const auto units = field<uintptr_t>(army, 0x118);
        if (size > 128 || (size && !units)) return;
        for (unsigned i = 0; i < size; ++i) {
            const auto unit = field<uintptr_t>(units, i * 4);
            if (!unit || field<uintptr_t>(unit, 0x2a4) != army) return;
            const unsigned id = field<unsigned>(unit, 4);
            const auto card = id ? card_for(manager, id) : 0;
            if (!card || (!group && !chosen_card(manager, card))) continue;
            for (unsigned j = 0; j < count; ++j)
                if (field<uintptr_t>(rows, j * 4) == card) {
                    selection.add({unit, id});
                    break;
                }
        }
        exchange.select(wanted, selection);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}
inline unsigned char __fastcall native_select_card(uintptr_t manager, void*, uintptr_t card, unsigned mode) {
    const auto from = uintptr_t(_ReturnAddress()) - engine;
    const auto result = select_card(manager, card, mode);
    if (!group_selection_depth && (from == 0x10ff59f || from == 0x10b084f ||
        from == 0x10b09f3 || from == 0x11d97b8 || from == 0x11d9898 ||
        from == 0xf9a49c || from == 0xf9a4f8 || from == 0xf9a540 || from == 0xf9a5a4))
        capture_selection(manager, uintptr_t(&card), 1, false);
    return result;
}
inline void capture_group(uintptr_t manager, uintptr_t group) {
    __try {
        const auto wanted = exchange.wanted();
        if (!wanted.environment || land_cards(engine, wanted) != manager ||
            !owned_land_group(engine, manager, group)) return;
        capture_selection(manager, field<uintptr_t>(group, 0x10), field<unsigned>(group, 0xc), true);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}
inline unsigned char __fastcall native_select_group(uintptr_t manager, void*, uintptr_t group, unsigned mode) {
    const auto from = uintptr_t(_ReturnAddress()) - engine;
    ++group_selection_depth;
    unsigned char result{};
    __try { result = select_group(manager, group, mode); }
    __finally { --group_selection_depth; }
    if (result && !(mode & 0xff) && (from == 0xf91e91 || from == 0xf917e8))
        capture_group(manager, group);
    return result;
}
inline void __fastcall native_select_cards(uintptr_t manager, void*, uintptr_t list) {
    const auto from = uintptr_t(_ReturnAddress()) - engine;
    select_cards(manager, list);
    if (from != 0x10ff75c && from != 0xf9a2f5 && from != 0x124efe5) return;
    __try {
        if (list) capture_selection(manager, field<uintptr_t>(list, 8), field<unsigned>(list, 4), false);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}
inline void __fastcall native_command_batch(uintptr_t environment, void*, uintptr_t packets, uintptr_t context) {
    const auto wanted = exchange.wanted();
    if (installed && !scoped_ai && environment == wanted.environment && wanted.generation) {
        bool prepared = false;
        __try { prepared = controller.prepare_commands(wanted); }
        __except (EXCEPTION_EXECUTE_HANDLER) { exchange.leave(); }
        if (prepared) {
            // 303430 is the original drain of the frame's copied command packets.
            // Release native assignments before that drain can apply the first order.
            const auto ai = controller.retiring_ai();
            if (ai) {
                scoped_ai = ai;
                __try { reinterpret_cast<Tick>(engine + 0x51f670)(ai); }
                __finally { scoped_ai = 0; }
            }
            controller.finish();
        }
    }
    command_batch(environment, packets, context);
}
inline constexpr unsigned hook_targets[] = {
    0x506ef0, 0x4db700, 0x525cb0, 0xf91810, 0xf91500, 0xf9a320, 0x303430
};
inline bool disable() {
    if (!installed || exchange.state().returning) return true;
    bool complete = true;
    for (auto rva : hook_targets)
        if (MH_DisableHook(reinterpret_cast<void*>(engine + rva)) != MH_OK) complete = false;
    if (update_operations &&
        MH_DisableHook(reinterpret_cast<void*>(engine + 0x509b10)) != MH_OK) complete = false;
    if (complete) InterlockedExchange(&installed, 0);
    return complete;
}
inline bool install(uintptr_t image, Window window, UINT wake) {
    if (installed) return false;
    for (const auto& sig : signatures)
        if (std::memcmp(reinterpret_cast<const void*>(image + sig.rva), sig.bytes, sig.size)) return false;
    void* detours[] = {reinterpret_cast<void*>(&native_scheduler),
        reinterpret_cast<void*>(&native_eligibility), reinterpret_cast<void*>(&native_roster_update),
        reinterpret_cast<void*>(&native_select_card), reinterpret_cast<void*>(&native_select_group),
        reinterpret_cast<void*>(&native_select_cards), reinterpret_cast<void*>(&native_command_batch)};
    void** originals[] = {reinterpret_cast<void**>(&scheduler), reinterpret_cast<void**>(&eligibility),
        reinterpret_cast<void**>(&roster_update), reinterpret_cast<void**>(&select_card),
        reinterpret_cast<void**>(&select_group), reinterpret_cast<void**>(&select_cards),
        reinterpret_cast<void**>(&command_batch)};
    constexpr unsigned count = sizeof(hook_targets) / sizeof(*hook_targets);
    unsigned created = 0;
    for (; created < count; ++created)
        if (MH_CreateHook(reinterpret_cast<void*>(image + hook_targets[created]), detours[created], originals[created]) != MH_OK) break;
    if (created != count) {
        while (created) MH_RemoveHook(reinterpret_cast<void*>(image + hook_targets[--created]));
        return false;
    }
    engine = image;
    input_window = window;
    controller.configure(image, wake);
    update_operations = nullptr;
    // Keep the base Commander path if an AI overhaul owns this optional site.
    bool compatible = true;
    for (const auto& sig : resume_signatures)
        if (std::memcmp(reinterpret_cast<const void*>(image + sig.rva), sig.bytes, sig.size))
            compatible = false;
    if (compatible)
        MH_CreateHook(reinterpret_cast<void*>(image + 0x509b10),
            reinterpret_cast<void*>(&native_update_operations),
            reinterpret_cast<void**>(&update_operations));
    InterlockedExchange(&installed, 1);
    return true;
}

} // namespace commander76

