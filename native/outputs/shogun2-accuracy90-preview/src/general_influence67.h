#pragma once
#include <cmath>

using GeneralRingDraw67 = void(__thiscall*)(void*, unsigned, void*);
static GeneralRingDraw67 original_general_ring67;

static bool possessed_general67(PossessionIdentity& identity, float& x, float& z) {
    __try {
        if (!possessed || field<uintptr_t>(possessed, 0) != base + 0x15dbd68 ||
            !friendly_owned109(identity)) return false;
        auto physical = identity.actor;
        if (identity.mount) {
            NativeSubject mount{};
            if (native_subject(identity.actor, identity.unit, identity.view, true, mount) &&
                mount.actor == identity.mount && mount.holder == identity.mount_holder &&
                mount.config == identity.mount_config && mount.serial == identity.mount_id)
                physical = mount.actor;
        }
        x = field<float>(physical, 0x48);
        z = field<float>(physical, 0x50);
        return std::isfinite(x) && std::isfinite(z);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool possessed_general_ring67(uintptr_t ring, float& x, float& z) {
    __try {
        PossessionIdentity identity{};
        return ring && unit && field<uintptr_t>(ring, 0) == base + 0x15d352c &&
            field<uintptr_t>(ring, 0x1c) == unit && possessed_general67(identity, x, z) &&
            field<uintptr_t>(ring, 0x1c) == identity.unit;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static void __fastcall draw_general_ring67(void* ring, void*, unsigned flags, void* renderer) {
    const auto address = reinterpret_cast<uintptr_t>(ring);
    float x{}, z{};
    if (!possessed_general_ring67(address, x, z)) {
        original_general_ring67(ring, flags, renderer);
        return;
    }

    // The original draw reads this 64-byte object without modifying it or
    // retaining its address. Its explicit-center branch is normally used in setup.
    // Keep the live ring and the unit's tactical center untouched.
    alignas(uintptr_t) unsigned char draw_ring[0x40];
    memcpy(draw_ring, ring, sizeof(draw_ring));
    const auto copy = reinterpret_cast<uintptr_t>(draw_ring);
    field<unsigned char>(copy, 0x2c) = 1;
    field<float>(copy, 0x30) = x;
    field<float>(copy, 0x34) = z;
    original_general_ring67(draw_ring, flags, renderer);
}

// 1587d0 is the nearby-general morale modifier. Scope both its candidate
// filter and its exact distance without touching the shared unit-pair cache.
// The original scan still owns alliance, eligibility, radius and effect size.
struct GeneralMorale67 {
    uintptr_t source{}, receiver{}, bounds{};
    float distance{};
};
static thread_local GeneralMorale67 general_morale67;
using GeneralMoraleScan67 = void(__thiscall*)(void*);
using GeneralMoraleFilter67 = bool(__stdcall*)(uintptr_t, uintptr_t, float, unsigned);
using GeneralMoraleDistance67 = float(__thiscall*)(void*, uintptr_t);
static GeneralMoraleScan67 original_general_morale67;
static GeneralMoraleFilter67 original_morale_filter67;
static GeneralMoraleDistance67 original_morale_distance67;

static GeneralMorale67 nearby_general67(void* context) {
    __try {
        PossessionIdentity identity{};
        float x{}, z{};
        if (!possessed_general67(identity, x, z)) return {};
        const auto receiver = field<uintptr_t>(reinterpret_cast<uintptr_t>(context), 4);
        if (!receiver || receiver == identity.unit ||
            field<uintptr_t>(receiver, 0x14) != identity.environment) return {};
        const auto formation = field<uintptr_t>(receiver, 0xb9c);
        if (!formation) return {};
        // Original rectangle-distance ABI. The general is a zero-size box at
        // the same X/Z used by the ring; the recipient keeps its native shape.
        const float general[] = {x, z, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f};
        using SquaredDistance = float(__thiscall*)(void*, const float*);
        const float squared = reinterpret_cast<SquaredDistance>(base + 0x1aaed0)(
            reinterpret_cast<void*>(formation + 0x644), general);
        if (!std::isfinite(squared) || squared < 0.f) return {};
        return {identity.unit, receiver, formation + 0x628, std::sqrt(squared)};
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return {};
    }
}

static void __fastcall scan_general_morale67(void* context, void*) {
    const auto saved = general_morale67;
    general_morale67 = nearby_general67(context);
    __try {
        original_general_morale67(context);
    } __finally {
        general_morale67 = saved;
    }
}

static bool __stdcall filter_general_morale67(uintptr_t bounds, uintptr_t candidate,
                                              float range, unsigned include_radius) {
    const auto& scope = general_morale67;
    if (scope.source && scope.source == candidate && scope.bounds == bounds)
        return scope.distance < range;
    return original_morale_filter67(bounds, candidate, range, include_radius);
}

static float __fastcall distance_general_morale67(void* context, void*, uintptr_t candidate) {
    const auto& scope = general_morale67;
    if (scope.source && scope.source == candidate &&
        field<uintptr_t>(reinterpret_cast<uintptr_t>(context), 4) == scope.receiver)
        return scope.distance;
    return original_morale_distance67(context, candidate);
}

static void create_general_influence67() {
    const unsigned char ring[] = {0x83, 0xec, 0x2c, 0x53, 0x55, 0x56, 0x8b, 0xf1};
    const unsigned char scan[] = {0xb8, 0x14, 0x28, 0, 0, 0xe8, 0xd6, 0x39, 0x3a, 0x01};
    const unsigned char filter[] = {0x8b, 0x44, 0x24, 0x08, 0x83, 0xec, 0x0c};
    const unsigned char distance[] = {0x8b, 0x01, 0xff, 0x74, 0x24, 0x04};
    const struct {
        uintptr_t rva;
        const unsigned char* signature;
        size_t size;
        void* detour;
        void** original;
    } hooks[] = {
        {0x2b68c0, ring, sizeof ring, reinterpret_cast<void*>(&draw_general_ring67),
            reinterpret_cast<void**>(&original_general_ring67)},
        {0x1587d0, scan, sizeof scan, reinterpret_cast<void*>(&scan_general_morale67),
            reinterpret_cast<void**>(&original_general_morale67)},
        {0x459b40, filter, sizeof filter, reinterpret_cast<void*>(&filter_general_morale67),
            reinterpret_cast<void**>(&original_morale_filter67)},
        {0x1b3d30, distance, sizeof distance, reinterpret_cast<void*>(&distance_general_morale67),
            reinterpret_cast<void**>(&original_morale_distance67)}
    };
    for (const auto& hook : hooks) {
        auto entry = reinterpret_cast<void*>(base + hook.rva);
        require(memcmp(entry, hook.signature, hook.size) == 0, "Native general influence signature differs");
        require(MH_CreateHook(entry, hook.detour, hook.original) == MH_OK, "Native general influence hook failed");
    }
}
