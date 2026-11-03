# Native source guide

The directory names below are retained build identifiers. Despite the names
`outputs`, `work`, and `preview`, these folders contain maintained source inputs.
Renaming them requires coordinated include, build, fixture and manifest changes.
Generated binaries belong in a new isolated output directory, not here.

## Current Workshop runtime

| Component | Entry points and supporting code |
| --- | --- |
| Workshop bridge | [`entry165.cpp`](../tools/workshop/entry165.cpp) exports the Lua prepare/start calls; [`bootstrap165.lua`](../tools/workshop/bootstrap165.lua) restores the owned game's front-end script before requesting gameplay startup |
| Startup worker | [`integrated.h`](work/autoload99-preview/integrated.h) checks payloads, starts the adapter/QTE/HUD and services friendly-fire audio; [`workshop_assets165.h`](work/autoload99-preview/workshop_assets165.h) prepares and shares local resources |
| Possession adapter | [`adapter.cpp`](outputs/shogun2-accuracy90-preview/src/adapter.cpp) coordinates actor ownership, camera, movement, weapons, orders and native hooks |
| Melee QTE | [`playable_runtime.cpp`](outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp), `qte_core.h`, `qte_session.h`, and `qte_native_bridge.h` manage admission, deadlines, input and native forwarding |
| HUD | [`hud.cpp`](outputs/shogun2-ammunition83-preview/hud-src/hud.cpp) and `hud_renderer.h` consume validated publications and render in a separate process |
| Shared contracts | [`common`](common) and [`shared headers`](outputs/shogun2-possession-cleanup-v46/src) define roles, resource/storage policy, engine identity and protocols |
| Resource extraction | [`prepare_local_assets159.cpp`](../tools/installer/prepare_local_assets159.cpp) is shared by Workshop and the historical installer; its folder name does not make it installer-only |

The adapter vendor tree supplies the MinHook objects used by the component
builds. Retain both vendor copies and their notices until all include/build and
licensing references have been reconciled.

`deck` contains local Steam Deck input, routing and UI-model development work.
It is not linked into the current Workshop gameplay build. See the
[input proof guide](../tools/deck/README.md); real Deck/Proton validation and
gameplay dispatch are pending.

## Tests and generated files

The native builder discovers adapter `verify.cpp` / `verify_*.cpp`, QTE
`verify_*.cpp`, and HUD `verify_hud_*.cpp` beside their implementations. Old
numeric suffixes do not make these regression tests obsolete. Workshop fixtures
live in [`tools/workshop`](../tools/workshop).

Generated tables and headers have declared inputs in [derived data](../docs/public-release/DERIVED_DATA.md).
Embedded payload hashes and cache catalogs are produced in the build snapshot.
Do not edit expected tables or fingerprints simply to make a check pass.

## Historical and shared code

`work/autoload99-preview/bootstrap.cpp`, the DInput exports, `launch161.cpp`, and
the installer transaction code support older setup builds. Workshop reuses the
worker and resource code, but does not load through that DInput bootstrap or
run the camera chooser. Smaller earlier preview trees retain shared headers and
fixture dependencies; they are not disposable build output.

Read the [architecture](../docs/ARCHITECTURE.md) and [native contracts](../docs/public-release/NATIVE_CONTRACTS.md)
before changing image gates, ABI, ownership, lifetime, threading or forwarding.
Offline fixtures cannot establish all live-game behavior.
