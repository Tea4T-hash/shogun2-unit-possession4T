# Current public-main build

This guide builds the combined runtime through revision36. It automatically selects
PC or Deck behavior. The supported personal charge uses forward sprint plus primary
attack; enemy acquisition remains unresolved. See the current release identity.

The build pipeline retains historical Test181/private publisher names. Do not upload
its private-name pack to a new item: public promotion retains item3825625050 and
shogun2_unit_possession_workshop165.pack, changes only first-party PLAYING.md/JSON
publication metadata, and preserves the native/bootstrap/campaign payload bytes.
The current public pack identity is in docs/public-release/V0_34_MENU36_VALIDATION.json.
Never distribute complete build outputs or recovered game resources.

# Revision24 combined candidate

Includes post-battle campaign input restoration, gentler battle LB/RB zoom,
and a four-sector R1 command wheel using locally prepared game resources.
The wheel has focused input/dispatch tests and a standalone real-resource
renderer check at 1280x800 and 1280x720. No game artwork is distributed.
Offline checks and owner gameplay acceptance are recorded separately.

# Build the combined PC and Steam Deck runtime

This is Test181 on the regular v0.33 source base. Building does not install or launch the game, edit Steam settings, or upload anything. The older Test173 instructions in BUILD_AND_TEST.md describe the preceding v0.33 runtime.

## Working while the game is running

Offline investigation, editing, builds and fixtures may run while the owner plays.
Builds use isolated copies and do not alter the running game. Close the game and
launcher only on the device whose active mod files will be replaced, after explicit
staging approval and verified rollback. Restart that game to load an update.

## Dependencies

Windows, Python 3.12+, MSVC 2022 x86 Build Tools with Windows SDK, an owned original Windows Shogun 2 installation, and the exact separately obtained reviewed Bugfix engine fixture are required. The build validates both engines against existing hashes. No decoding database is needed to compile.

Create the float-number Lua 5.1 tools and pinned Valve reference headers using:

```powershell
py -3.12 -B tools/deck/recover_dependencies.py --output D:\Shogun2Deck\dependencies --vcvars C:\BuildTools2022\VC\Auxiliary\Build\vcvars32.bat
```

The script fetches official upstream sources (including the pinned SteamUtils010
device-detection declaration), checks their hashes, builds only the development Lua tools, and validates the exact float-bytecode header. It needs no game access. The Lua copyright remains in the downloaded source. Valve headers are audit inputs and are not shipped in the mod.

## Full clean build

Adjust input/tool paths for the machine. Use an unmodified owned game as --game; if the actual installation has the Bugfix engine, use a separate legitimate stock fixture for these checks. The existing game remains untouched.

```powershell
py -3.12 -B tools/deck/build181.py --game "D:\Owned Stock Game" --bugfix "D:\Reviewed Bugfix\Empire.Retail.dll" --output D:\Shogun2Deck\builds\test181-clean --vcvars C:\BuildTools2022\VC\Auxiliary\Build\vcvars32.bat --lua D:\Shogun2Deck\dependencies\lua51.exe --luac D:\Shogun2Deck\dependencies\luac51.exe --abi-reference-dir D:\Shogun2Deck\dependencies\valve
```

A full clean build remains available. For bridge-only changes, --components-from may reuse a completed build only after exact comparison of all four component dependency closures and embedded resource inputs; its inherited component results are identified explicitly. Any compiled component change requires a fresh full component build. Source snapshots, compiled game-independent fixtures, generated local resources and logs stay inside the output directory. Set TEMP/TMP to a D: scratch directory too.

Fresh full build outputs contain private recovered game-script/resource fixtures. Do not upload the whole output folder. Only the audited Workshop package is a mod payload. tools/deck/package181.py produces the historical staging identity described below. Supply an authorized preview image separately.

## Source checks

```powershell
py -3.12 -B -m unittest discover -s tests -v
py -3.12 -B tools/generate_roles.py --check
py -3.12 -B tools/generate_tables.py --check
```

Offline checks do not establish native Deck rendering, display latency or full controller support. See docs/current-release.json and docs/public-release/V0_34_MENU36_VALIDATION.json before testing.

## Campaign checks and historical staging package

After the full build passes, run the additional owned campaign-root checks. They exercise the native reader, inert native export fixture and compiled first-party wrappers; they never start a game or Steam.

```powershell
py -3.12 -B tools/deck/check_campaign181.py --build D:\Shogun2Deck\builds\test181-clean --game "D:\Owned Stock Game" --output D:\Shogun2Deck\builds\test181-campaign-check --vcvars C:\BuildTools2022\VC\Auxiliary\Build\vcvars32.bat --lua D:\Shogun2Deck\dependencies\lua51.exe --luac D:\Shogun2Deck\dependencies\luac51.exe
py -3.12 -B tools/deck/package181.py --build D:\Shogun2Deck\builds\test181-clean --output D:\Shogun2Deck\builds\test181-publisher --preview D:\Shogun2Deck\private-preview.png --luac D:\Shogun2Deck\dependencies\luac51.exe
```

The final private pack has seven entries, including the first-party campaign wrapper. No owned campaign script is embedded. This intermediate output uses the historical private publisher stem. For the main Workshop package, retain item3825625050 / shogun2_unit_possession_workshop165.pack and the native payload; do not publish the intermediate as a new player item. Staging/upload require separate explicit authorization and rollback. The maintained public item is 3825625050; the historical private identity is a staging output, not a new player subscription. Controls and pending hardware checks are in [SteamDeck181.md](docs/development/SteamDeck181.md).

## Display-label-only rebuild

For a completed Test181 clean build, `tools/deck/rebuild_menu181.py` also accepts `--controller-labels` alongside its existing input/output/tool arguments. It compares every compiled gameplay dependency with that build. The only permitted component-source difference is the generated controller-default header, and its parsed action definitions, presets, physical bindings and settings must remain identical after removing display metadata. The adapter is recompiled because it embeds that header; QTE, HUD and the extractor are reused with exact hashes. The menu bridge, input-policy and Workshop checks run freshly. Prior full gameplay regression results are inherited evidence, not rerun or hardware acceptance.

Use a new output directory and run the campaign checks and private packaging step afterward. This option does not install, upload or edit a Steam user layout.

## Revision21 QoL checks

The full build runs `verify_qol193.cpp` through the real Controls and RTS policy
paths, alongside the existing input, native-hook, device-profile and HUD suites.
See docs/development/QoL193.md for exact behavior and live acceptance limits.

`tools/deck/verify_help_header186.cpp` is a separate native-font visual fixture.
Compile it as x86 C++17 with GDI+, User32 and BCrypt; pass the locally generated
hud-assets159.s2fp, Steam's own sd_button_view_md.png, and a new output directory.
It checks eight layouts, transparent background and shared left anchor, and saves
PNG previews. Do not distribute the owned fonts, glyph input or complete build
fixtures. The private package contains only first-party code and existing notices.

## Revision22 cursor and campaign bindings

The full build also runs `verify_bindings194.cpp`: actual campaign/menu button
routing, native key queues, possession press/release sequences, and the owned
cursor renderer/lifetimes. The frontend boundary fixture verifies inverted
campaign stick zoom and L3 fast panning while preserving the pan/turn vectors.
The displayed Steam layout revision is 3.5; its campaign L3 label includes both
fast panning and Ctrl selection. See [CursorAndBindings194.md](docs/development/CursorAndBindings194.md)
for the complete mapping, observed Gamescope cursor evidence and hardware limits.

These checks never start Steam or a game and do not inject system input. Actual
cursor visibility and compositing on the Deck require a subsequent owner test.

Revision22 battle additions: hold A for native Space formations/range; L4 + R4
selects all (R4 alone keeps melee mode). Hold D-pad assignments refresh the native
modifier cache. LB/RB zoom is continuous through the original native camera
methods. In-game controls and Steam layout labels reflect these bindings.
Battle cursor is owner-confirmed working and unchanged. Hardware validation of
the new candidate remains pending; no public/Workshop update is implied.
