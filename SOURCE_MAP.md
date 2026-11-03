> Current maintained source: v0.34 combines PC and Steam Deck. Use [BUILD_STEAM_DECK.md](BUILD_STEAM_DECK.md) and [README](README.md). Earlier checkpoint and release instructions below are historical.

# Source map

[Release identity](docs/current-release.json) is authoritative. Start with the
[developer reading path](docs/README.md), [architecture](docs/ARCHITECTURE.md),
[native guide](native/README.md), or [tool guide](tools/README.md).

Directory suffixes are historical implementation names. `native/outputs` and
`native/work` contain source inputs, not disposable build artifacts. The current
Workshop route shares some code with older installers; the responsibility column
distinguishes those uses.

| Component | Active source | Responsibility |
| --- | --- | --- |
| Runtime worker / legacy loader | native/work/autoload99-preview | Shared delayed startup, payload checks, cache and game-process audio. DInput forwarding and launch161.cpp belong to older installer builds |
| Workshop entry | tools/workshop and native/work/autoload99-preview/workshop_assets165.h | Current Test173 entry, owned-script recovery, automatic local resources and existing worker; no chooser or Steam setting edits |
| Adapter | native/outputs/shogun2-accuracy90-preview/src | Actor lease, native movement/weapons/orders/camera/charge/climbing |
| Shared | native/outputs/shogun2-possession-cleanup-v46/src and native/common | Identity, protocols, visibility and final-human roles |
| QTE | native/outputs/shogun2-authentic-hud80-preview/qte-src | Native pairs, response/input timing, animation camera |
| HUD | native/outputs/shogun2-ammunition83-preview/hud-src | Read-only measurement, artwork and controls; friendly-fire playback belongs to the game loader worker |
| Installer | native/work/installer-siege99 | Fingerprints, durable journals, repair, conservative retention |
| Player setup/local assets | tools/installer, native/common/local_assets159.h | Inno wizard, read-only native extraction, six-target rollback, external resources; steam_launch162/steam_session163 manage the owned Steam connection and normal install-time client lifecycle |
| Diagnostics | tools/diagnostics/source | Optional external capture, telemetry and dump inspection |
| Player storage | native/common/runtime_storage166.h and tools/workshop | Disabled gameplay/input disk logs, ephemeral live-check receipts, shared local resources and conservative temporary-file recovery |
| Build | tools/build.py, build_frontend.py, native_build.py | Isolated snapshot, explicit inputs, native checks |
| Resources | tools/prepare_resources.py, game_packs.py, resource-recipe.json | Owned-game extraction/trim/packing/fonts/icons/voice |
| Derived records | tools/data, generate_tables.py, generate_roles.py | Canonical observations and repeatable headers |

The playable QTE DLL is compiled from `qte-src/playable_runtime.cpp`; its worker
owns gameplay initialization. `qte-src/observer.cpp` is a separate observer,
not that worker. Check the actual build entry point when changing startup gates.

The current [engine contract introduced in Test172](docs/public-release/WORKSHOP172.md) centralizes engine
admission in `native/common/engine_contract172.h`, with its maintained registry
in `tools/data/engine-contract172.json`. Its generator and source audit are in
`tools/workshop`. The resource extractor is an admission consumer as well.

Build-generated hashes belong in the build snapshot. See [native contracts](docs/public-release/NATIVE_CONTRACTS.md) and [derived data](docs/public-release/DERIVED_DATA.md).

The [development](docs/development/README.md) and [release](docs/releases/README.md)
indexes explain historical evidence. Full development notebooks and older
release notes are excluded from the curated source kit; the indexes and current
v0.33 notes are included. Earlier machine paths, limits and session requests do
not override current instructions. The original implementation/research remains
in baseline commit 24d31a2f5e5c2cf84a42c459a69bdcd9bd86c600.
