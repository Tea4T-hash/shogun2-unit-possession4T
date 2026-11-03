# Steam Deck development — Test174, local only

This work follows the owner's Deck V3.1 brief. It is not a new public release.
v0.33/Test173, its installed pack and all remote refs remain untouched.

## Architecture audit before implementation

The mod is C++17/x86 plus float Lua 5.1, built with MSVC. Its Workshop wrapper
recovers the owned game's front-end script and then starts the native worker.
The adapter uses scoped engine hooks, a game-window procedure, raw mouse input
and keyboard polling. Native movement/weapon commands remain authoritative.
The in-game D3D9/D3D11 overlay restores render state; the existing HUD process
supplies numeric publications and pixels. Controller gameplay UI must eventually
draw through the in-game renderer, not a Steam radial or external overlay.

| Existing implementation | Reuse boundary |
| --- | --- |
| `adapter.cpp`: `request_possession`, `enter_selected_soldier`, `restore_rts_on_window90` | Existing entry/exit, identity and owning-thread checks |
| `raw_mouse.h`, worker input, `view152.h`, `steering.h` | Look/movement intent; preserve keyboard/mouse input |
| `weapon_mouse143`, `equipment_input81.h` | Actual ranged draw/release, reload and native mode behavior |
| `native_commands100.h`, `command_input100.h`, `native_context103.h` | Existing parent-unit commands, native gesture placement and targeting |
| `ability_catalog141.h` and existing ability publications | Actual unit ability definitions/artwork; no invented abilities |
| `playable_runtime.cpp` and QTE session/authorization | Existing timestamp/deadline/oscillator judgment, unchanged difficulty |
| `native_overlay106.h` and local resource bundles | In-game render-state preservation and native artwork/fonts |
| `entry165.cpp`, `pack165.h`, `ui_compat168.h`, `bootstrap165.lua` | Known startup/root recovery; diagnostic must not load gameplay |

The existing command path is scoped to the possessed parent unit. Arbitrary
allied selection, native ten-group operations, RTS camera actions and campaign
dispatch are not established merely by having controller events. Those actions
must stay disabled until their engine interfaces and contexts are verified.

## Input backend decision and proof gate

The installed game `steam_api.dll` exports `SteamClient`, handle accessors and
the flat `GetISteamGenericInterface` accessor. Valve's published Proton source
defines the versioned SteamInput006 ABI. The first route to test is therefore
in-process native Steam Input, using the game's existing loaded Steam API.
Do not replace Steam DLLs, initialize another Steam account/pipe, or use XInput
aliases for the four grips. Bundled action defaults are native API actions,
with no keyboard/mouse bindings and no Steam radial menu.

SDL's direct HID Deck driver changes lizard-mode state and uses a watchdog.
That route is not proof of safe coexistence with Steam in Gaming Mode. A Linux
helper is not selected while the versioned in-process route remains testable.

The diagnostic first tests interface availability, all grips independently,
both pad positions/touches, both sticks, analog trigger ranges and motion.
Steam/QAM are never assigned. Hardware sampling timestamps are host timestamps,
not a claim of hardware-to-display latency. Touchscreen and pad pressure have
separate capability states; neither may be fabricated from another control.

Only a local diagnostic pack is built initially. It restores the native menu,
starts a bounded input-only worker and does not load adapter/QTE/HUD gameplay.
It is separate from the accepted possession pack. All hardware and Gaming Mode
claims remain pending until the owner tests on their Deck.

## Subsequent dependencies

1. Compile/test normalized events, tunable thresholds, context priority,
   release/disconnect handling, tap/hold/chord arbitration and menu geometry.
2. Verify native controls and duplicate-input behavior on the actual Deck.
3. Feed existing possession/QTE functions through semantic actions on their
   owning threads; preserve mouse/keyboard and native combat math.
4. Investigate RTS selection/camera/groups and campaign interfaces using the
   preserved on-demand decoder; no guessed offsets or keyboard preset substitute.
5. Render Deck wheels/prompts in process from actual unit data and owned assets.
6. Validate each required RTS, possession, campaign and suspend/resume scenario.

## Implemented locally and remaining work

The input-only pack and modules below are development work. They are not
included by the normal Workshop builder and do not change the installed mod.

| Area | Local implementation | Validation / remaining dependency |
| --- | --- | --- |
| Versioned native Steam Input | Existing Steam interface; 24 digital and 6 analog actions, motion, physical-origin checks | MSVC x86 simulation and pinned declarations pass; actual Deck pending |
| Grips, sticks, pads, triggers | Distinct native actions and normalized frame fields | All four grips and analog ranges need the owner's Deck report |
| Gyro / accelerometer | Unit conversion; gyro bias, smoothing, deadzone and time scaling | Synthetic checks pass; real axes, drift and latency pending |
| Stick capacitive touch | Optional capability, independent actions | Hardware pending; not required to accept other controls |
| Trackpad pressure / touchscreen | Capability fields reserved; no backend implementation | Unavailable through this selected API path so far; separate native investigation needed |
| Device, focus and sample lifecycle | Sequence/epoch guards, stale/focus clear, neutral digital/trigger reacquisition | Simulated; Gaming Mode, suspend/resume and reconnect pending |
| Semantic router | Context priority, button edges, chords, tap/hold, camera/pointer/combat requests | Simulation only; no game dispatch is enabled |
| Wheels / ten groups | Sector geometry, availability models, two banks and overwrite hold | Pure models tested; actual engine groups, content and in-process rendering pending |
| RTS camera, cursor, selection and formation placement | Action requests and existing native reuse map | Owning-thread native RTS interface work pending; no fake keyboard preset |
| Possession / QTE / ranged / mounted input | Semantic requests and existing-function map | Not connected until native input proof; combat and oscillator code unchanged |
| Allied commands while possessed | Separate command-selection design required | Existing parent-unit orders cannot be assumed to control arbitrary allied groups |
| Campaign / deployment / naval | Explicit context and permission boundaries | Actual native navigation/dispatch not implemented; naval possession remains unsupported |
| Controller HUD and prompts | No new render hook installed | Mod-owned in-process rendering and 1280×800 hardware review pending |
| Keyboard/mouse coexistence | Existing implementation unchanged; no emulated outputs in defaults | Real simultaneous-input and duplicate-event test pending |

The router's `DispatchGate` defaults to denying every game call. Context Facts
currently come from a simulation fixture, not a completed engine context
resolver. Tuning defaults are editable through a bounded INI reader. A single
L4 tap waits for the 250 ms double-tap window; this explicit arbitration delay
needs handheld review. Per-axis gyro tuning, camera acceleration, full nested
wheels and campaign confirmations remain future implementation work.

The diagnostic collects aggregates for 90 seconds and stops. It creates at
most four reports of 16 KiB per content-addressed development binary. It does
not attach a debugger, start the old recorder, load gameplay components or
install engine hooks. A scoped exception boundary protects only its new worker;
it does not suppress crashes in other game threads. Shutdown and actual
Steam Input coexistence still require hardware testing.

Valve documents the action-manifest override as lasting for the Steam session.
The diagnostic calls Shutdown on its owned initialization, but does not claim
that call clears the client's override. The owner test therefore ends with a
Deck restart before normal play. Automatic restoration and a suitable final
player lifecycle remain unresolved; no guessed reset call or Steam-config
rewrite is used. See the linked primary manifest documentation.

See [the Deck test instructions](../../tools/deck/README.md) before launching.
No hardware, Proton, Gaming Mode or suspend/resume success is claimed yet.

## Offline test evidence and a corrected test-runner defect

The corrected native runner requires a zero exit code and explicit completion
markers. Earlier development outputs could miss a signed Windows access
violation because `if errorlevel 1` checks only positive codes. Those early
outputs are preserved privately but are **not valid overall passes**.

The corrected runner exposed a new-bridge ABI fault: MSVC x86 cannot safely
substitute an ordinary function-pointer wrapper for every virtual method
returning these structures. The bridge now uses a typed versioned virtual
interface and trivial, one-byte-packed POD return types, matching Valve's
declarations. All 34 simulation/ABI checks then completed. The production
resource/report fixture completes 9 checks, and real float Lua exercises all
37 stock callbacks and failure recovery. None of this executes a real Steam
Input implementation or establishes hardware support.

A Python regression executes real Windows batch processes returning zero,
positive failure and a signed access-violation code; only zero may proceed to
the next command. The initial 27-test Python suite passed. The existing 196-source
engine contract, role policy and six canonical table checks remain unchanged.

The final build receipt and exact pack/native hashes accompany the local test
package. No GitHub ref or release is changed. The owner subsequently authorized
a separate private Workshop diagnostic for transfer; see the next section.

## First Deck startup failure and path-policy revision 1

The owner uploaded diagnostic item **3829785193** and supplied a Deck photograph
showing `Reparse cache/input refused`. The string maps to the shared reader's
strict Windows path guard, before the input worker starts. The exact rejected
Deck path is not yet known. The first upload is a failed startup, not hardware
input validation. Steam separately confirms its upload and matching download.

Fresh CLOSED authorized the correction. The Deck snapshot now generates a
path-policy adapter around pinned copies of the shared pack/UI readers.
Only guard delegation changes; parser, owned-root hash, script lifetime,
self-identification and UI coexistence logic remain exact. The source Windows
readers, gameplay modules, engine contract and installed main mod are untouched.

Only under Proton may existing ancestor directory aliases and read-only input
aliases be followed. The followed handle must be a disk file/directory of the
required kind, with a resolved local DOS name and an available non-network
volume. Output leaves still reject links, and exclusive/content-checked writes
are retained. This does not claim to defeat same-user Unix filesystem races.
The normal Windows path still rejects reparse aliases.

The native path fixture exercises real NTFS junctions, the unchanged stock-root
reader through a linked game directory, temporary root cleanup, invalid paths,
linked-output refusal and dangling links. Actual file-symlink creation was
skipped with Windows error 1314; its read/write role predicate is tested. Real
Proton symlink behavior remains pending. A Python adapter-boundary test brings
the full suite to **28 passing tests**. No controller gameplay is enabled.

The fixed pack updates the same private diagnostic item, never public possession
item 3825625050. The test baseline requested for the Deck is official Proton
10.0-4 / maintenance 10.0-4b. New upload and owner hardware results must be
recorded separately; offline success is not a successful Deck launch.

## Primary references

- [Valve hardware specifications](https://www.steamdeck.com/en/tech)
- [Steam Input API](https://partner.steamgames.com/doc/api/ISteamInput)
- [Bundled action manifests](https://partner.steamgames.com/doc/features/steam_controller/action_manifest_file)
- [Valve Proton SteamInput006 declarations](https://github.com/ValveSoftware/Proton/blob/e91ca2be0df2cef4c230cbbc0b86604d73a0bbf6/lsteamclient/steamworks_sdk_162/isteaminput.h)
- [SDL Deck driver](https://github.com/libsdl-org/SDL/blob/main/src/joystick/hidapi/SDL_hidapi_steamdeck.c)

Private pinned reference hashes and the owner brief are kept under the new
Test174 audit output. SDK source/game assets are not added to the mod payload.


## First native-input report and diagnostic revision 2

The owner photograph of the revision-1 report confirms its new DLL identity
3edb029f... on the Deck, path-policy execution, Steam Input manifest/handles,
one controller and all 24 active digital bindings. It shows zero accepted
frames, roughly ten thousand rejected reads, state WaitingBindings, and
expected origin357/actual358. This establishes startup/interface progress,
not successful physical-control coverage. Device epochs and press counts were
zero because Proof only observed fully accepted frames.

The pinned Valve declarations identify357 as SteamDeck R2_SoftPull and358 as
SteamDeck R2. The bridge now accepts either or both origins of each same analog
trigger, preserving left/right separation and exact checks for digital,
stick and pad origins. The ABI audit now checks the six analog and four trigger
origin constants against Valve source. Regression tests reproduce full-only
and combined origins, reject opposite triggers and unrelated aliases, preserve
analog values, and clear stale mismatch information after recovery.

Revision2 waits for valid bindings, then uses an in-process native ready prompt.
Touching OK starts the complete90-second measurement; time spent waiting for
that confirmation is excluded. Failure before readiness does not ask the owner
to perform a useless timed exercise. Cancel skips measurement. A native results
message shows the duration, counts and missing controls and waits for a photo.
A bounded JSON remains optional. No separate script/UI process is added.
The exact measurement flow is tested with a delayed prompt and a fake clock;
42 native simulation/ABI checks now pass. Dialog layout/touch interaction and
real Deck input remain pending; no full controller gameplay is enabled.

The backend is Valve Steam Input under Proton. The SDL HID driver linked in the
brief was research, not the selected backend. Only the mod bridge, normalization
and semantic control layer are first-party development; raw hardware protocol
handling is Valve's existing implementation. Normal Windows gameplay and the
public/main Workshop item remain untouched.

## Thumbstick-click correction - diagnostic revision 3

The owner's revision-2 photograph shows the full 90-second measurement,
10,583 accepted reads and zero rejections. Independent grips, stick/pad ranges,
analog trigger ranges and gyro movement are recorded. L3/R3 are the only two
unrecorded required actions. The owner first thought those clicks were skipped,
then repeated the test and reported neither was recognized. The later report
is user-reported; no second original JSON/photo was supplied.

Read-only review of Valve's installed
`controller_base/templates/controller_neptune_gamepad_joystick.vdf` follows
the preset's active physical-source bindings: actual `joystick` and
`right_joystick` groups use `Full_Press` for their `click` inputs. Trackpads use
`Soft_Press`. Our generator had used `Soft_Press` for all four. This concrete
configuration difference fits the two working pad clicks and two missing stick
clicks. Valve's [activator documentation](https://partner.steamgames.com/doc/features/steam_controller/activators)
describes ordinary button activation; the installed templates establish the
serialized `Full_Press` names for the actual stick sources. Private review
evidence pins the local template hashes and extracted source/group paths.

Revision 3 changes only L3/R3 to regular digital activation, increments the
bundled configuration revision and displays explicit click counts. All other
native bindings, analog settings, API origins/ABI, input coverage requirements,
normal Windows gameplay and the accepted mod remain unchanged. A generated-VDF
regression follows active physical sources to their actual click activators;
the former flat action-name checks could not catch this mistake. Native tests
also retain separate click press/release counting and require both clicks
before marking coverage complete. These are offline checks; a real Deck retest
is still needed to establish whether this correction resolves the symptom.

No Steam user configuration or installed Valve template is edited. A new native
resource payload gives the diagnostic its own content-addressed configuration
directory. Restart the Deck before retesting to end the previous manifest's
Steam session, and after disabling the probe before normal play. Only the same
private item is prepared for updating; full controller gameplay remains gated.
