> Historical development/checkpoint notes. The combined implementation is now
> maintained in public `main`; use the root README and BUILD_STEAM_DECK.md for
> current play/build instructions. Earlier private-item state below is historical.

# Pending revision30: shared gameplay corrections

Revision29 camera feel remains accepted. This candidate adds scroll-aware
third-person shoulder framing, an ordinary-infantry walk/run pose handoff,
and two native selectors that include the isolated possessed soldier.
Deck Battle RTS **X toggles run/walk**; the Steam layout/guide label is3.6.
Campaign and possession X retain their existing actions.

See [Gameplay203](Gameplay203.md) for evidence and limits. In-game enemy
acquisition and the new visual changes remain unverified. No Workshop update
has been made for this candidate.

## Pending revision25: PC HUD parity

PC keyboard K now uses the same three-step possession HUD cycle as Deck View.
See [HUD198](HudVisibility198.md) for the cause, checks and pending game test.
This correction is staged in the main Unit Possession entry; it is not uploaded or in-game accepted yet.

# Current acceptance: v0.34-deck.2, revision24

The owner accepted battle play and confirmed campaign B-close and held Y end-turn
still work after returning from battle. Exact subscribed and loaded24 hashes were
verified. Cursor visibility was separately accepted on22; automatic profiles were
accepted on PC and Deck on20. Per-revision test-pending notes below are historical;
every wheel quadrant/clan and long-session stability remain individually unproven.
The public Workshop item has not been updated. See the release validation record.

# Private command wheel and campaign return correction (revision24)

Hold R1 during possession, choose a direction with the right stick or D-pad,
then release R1 to order. Up follows, Down stops, Left holds at your position,
and Right cycles fire orders. B cancels without issuing an order or dismounting.
Opening the controls guide, pausing, returning to RTS or losing input focus
cancels a pending choice.

The wheel has four curved sectors around the possessed unit's clan mon. Its
paper, panel textures, fonts and mon come from the existing resources prepared
from each player's installed Shogun 2. Button glyphs come from that player's
Steam client. No game artwork, fonts or audio are bundled with this update.
Unsupported custom clan artwork is left absent rather than showing another clan.

Also retains the post-battle B/End Turn correction and gentler battle LB/RB zoom
below. This private candidate requires a Deck test; offline command delivery
checks do not prove the resulting formation movement in a live battle.

# Private post-battle input correction (revision23)

B and End Turn failed after battle because the gameplay component kept the
keyboard poll hook after handing controller ownership back to the campaign.
The adapter now restores that site before releasing its battle lease and
reacquires it only after the campaign owner has left. Exact hook guards and
existing engine admission remain in force. No engine file is replaced.
The private candidate also includes the gentler battle shoulder zoom below.

# Private battle zoom tuning (revision23)

Battle LB/RB now builds speed over 450 ms and reaches 80% of the previous maximum.
Release stops immediately; changing direction starts gently again.
Campaign right-stick zoom, cursor, other cameras and button bindings are unchanged.
The owner accepted revision22 cursor visibility and smooth zoom; this new zoom
feel still needs a Deck test.

# Private cursor and campaign correction (revision22)

Adds the game-native cursor image during right-trackpad use in campaign and
in-game panels/menus under Proton. The new cursor presentation needs a Deck
trial; offline checks cannot establish Gamescope compositing.

Campaign right-stick zoom is reversed as requested. Hold L3 for faster map
panning; its Ctrl multi-selection behavior remains. L1/R1 keep the existing
native next/previous selection behavior, which the owner confirmed follows the
selected entity type. Campaign Y end-turn and R4 quick-save still require600ms;
blocked panels and the controls guide now reset their hold timers.

Steam labels are3.5 under **Unit Possession Layout**. All86 active labels were
traced to their actual control routes; see [CursorAndBindings194.md](CursorAndBindings194.md).
Battle pan/turn/tilt, possession cameras, triggers and card selection are preserved.
Group modifier delivery, shoulder zoom, A and L4+R4 are corrected below.
No public Workshop update or recorder is included.

# Private QoL candidate (revision21)

Builds on the owner-accepted automatic PC/Steam Deck candidate. Desktop keeps
normal controls, sizing and no Deck hints. On Deck: a small transparent hint
sits beside the top-left native menu buttons in campaign, battle and possession.
Campaign shows Hold View: Controls; battle/possession also show Tap View: HUD.
Main menu remains free of the hint.

Possession gives forward/back/strafe movement more tolerance for thumb wobble;
deliberate diagonals still work. Campaign and RTS camera axes/speeds are unchanged.
In battle, tap D-pad to recall group 1/2/3/4; hold 0.55 seconds to assign the current
selection. L5 + direction remains available. Double RT on a unit sends the native
mouse double-click, with the system double-click time and pixel-distance limits;
Ctrl/Shift selection and dragging remain separate. Steam labels use revision3.4
and the layout is named **Unit Possession Layout**.

The HUD validates each status after reading it, fixing a reproduced timing race
that could label freshly written fire orders unknown. Identity/freshness checks
remain enforced. This candidate still needs an actual Deck test; offline checks
are not proof of gameplay appearance or of every possible flicker cause.

# Automatic-device private candidate (revision20)

This candidate adds automatic Steam Deck detection to the accepted revision19
controls. A desktop gets normal keyboard/mouse input and normal HUD/QTE sizing,
with no Deck prompts, View header, or native UI enlargement. A positively
identified Deck retains the accepted behavior below. Lookup failure uses the
desktop fallback. See [AutomaticDevice192.md](AutomaticDevice192.md).

Revision20 was accepted by the owner on both PC and Deck; revision21 still needs
live testing. The public Workshop item is unchanged. Oversized Deck campaign panels still use the accepted smaller-scale
fitting workaround; that readability limitation is preserved.

# Private Steam Deck controls — v0.34-deck.1 (Test181 revision19)

Use Windows Shogun 2 under Proton 9.0-4 with only private item3829785193 enabled.
This includes Unit Possession. Keep public and multiplayer possession entries off.
This candidate preserves the accepted cameras, LB/RB zoom and L3 selection.
It replaces frame-only resizing with uniform fitting of the complete native
window. The owner accepted this checkpoint on the Deck, with the panel-scale compromise below.

Hold View for600ms to open the in-game controls guide. It uses Shogun fonts,
parchment styling and Valve button glyphs, with L1/R1 pages and B to close.
It does not open Steam. In Battle RTS, tap View to hide/show the native HUD.
In possession, successive taps hide the RTS HUD, then both HUDs, then restore
both. QTE and response prompts remain available. The main menu has no permanent hint. Revision21 adds the small transparent
Hold View hint to campaign and the two-instruction hint to battle/possession.

The map UI retains25% enlargement and the mod HUD/QTE15%. An oversized campaign
window uses only as much enlargement as fits its complete frame on screen.
Native child sizes, fonts, tabs and close buttons stay together; no separate
frame compression or additional Clan initialization is used. The technology
window keeps its own research data and connections, with a complete inset frame
like the agent skill tree; it is not replaced with the agent skill system.

The owner reports that the correction works and accepts it for now. Large panels
return toward the original small UI scale to fit; this remains uncomfortable to
read and is an acknowledged temporary workaround. A properly enlarged, reflowed
panel layout remains unfinished. This acceptance does not establish every panel,
control or long-session stability. Offline geometry/render-provider fixtures and
manual Deck acceptance are recorded separately in recovery/test181.

## Campaign

| Input | Action |
|---|---|
| R2 / L2 | Select or interact / move or attack |
| R1 / L1 | Next / previous selection; in a panel, hold L1 and swipe right pad to scroll |
| A / B | Toggle movement speed (Space) / cancel or back |
| X | Zoom home |
| Y held | End turn, once after600ms |
| D-pad up / down / left / right | Mastery of Arts / Finances / Diplomacy / Clan Management |
| L3 held | Faster map pan; Ctrl add/remove selection |
| L5 held / L4 held | Ctrl add/remove selection / Shift range selection |
| R4 held | Quick-save, once after600ms; native CanSave required |
| R5 | Toggle character movement speed |
| View / Menu | Hold: in-game controls / game menu |

## Battle RTS, including deployment

| Input | Action |
|---|---|
| R2 | Select; hold and drag for box selection; double-tap a unit to focus it |
| L2 | Move or attack; hold and drag formation; double-tap for run or charge |
| R1 / L1 | Zoom out / zoom in |
| A / B / X / Y | Hold: formations and range / cancel or deselect / toggle run-walk / possess |
| D-pad up / down / left / right | Tap: select group1 /2 /3 /4; hold: assign current selection |
| L4 held | Select card range or queue orders |
| L3 or L5 held + R2 | Add/remove one unit at the cursor; the point is held still until R2 releases |
| L5 + D-pad | Assign selected units to corresponding native group |
| R4 / R5 | Toggle melee mode / group or ungroup |
| L4 + R4 | Select all units |
| L5 + R5 | Lock or unlock group formation |
| View / Menu | Tap: hide/show HUD; hold: controls / game menu |

## Possession

| Input | Action |
|---|---|
| R2 | Fire; hold/release bow; primary melee or QTE response |
| L2 | Ready ranged weapon; secondary melee or QTE response |
| R1 held | Command wheel: right stick or D-pad chooses; release commits |
| Wheel up / down / left / right | Follow Me / Stop / Hold Here / Cycle Fire Orders |
| L1 | Unit move/attack; hold for formation; double-tap for run/charge |
| A / B | Climb/interact / mount or dismount unit |
| X / Y | Native unit melee/ranged toggle / return to RTS |
| D-pad up / down | Cycle available ability/ammunition / use or end selected ability |
| D-pad left / right | Third-person distance closer / farther |
| L3 held / L5 held | Run / crouch |
| L4 held | Orbit the third-person camera |
| R4 / R5 | Stop unit / Follow Me |
| View / Menu | Tap: cycle HUD; hold: controls / game menu while remaining possessed |

## Accepted camera controls

Left stick retains campaign/RTS pan and possessed movement. Right stick retains
campaign turn and continuous native zoom, RTS turn/tilt, and possessed look.
Trackpad touch cursor/look and left-trackpad scroll/zoom remain as accepted.
Right-trackpad click remains unassigned. L3 adds to selection in RTS and holds
sprint in possession. Campaign L3 accelerates map panning and also adds to selection. L4 holds possession
orbit. The underlying accepted directional and camera-rate functions are exact.
Y is the sole possess/return button. R3 is unassigned in RTS; holding R3 in
possession switches between first and third person. Campaign native camera functions and sensitivities stay exact; revision22 reverses its stick zoom and enables the existing L3 pan boost. Battle shoulder zoom now uses continuous native camera steps.
The main-menu legend is absent. Campaign has the compact controls hint; battle/possession also show the View HUD shortcut.
Steam action-set labels and Valve-provided shaped/coloured glyphs match this map.

## Engineering and test limits

All native ability, movement, melee/QTE, charge, morale, weapon and AI mechanics
remain the regular mod's code. The former RTS shoulder orders are unassigned
for now; they are not moved to a trackpad. RTS shortcuts merge the shipped default
key chords into the game's existing private native keyboard polling path;
no Steam keyboard preset, SendInput or global OS key-state changes are made.
Custom in-game keyboard remaps need separate validation. Native groups retain
native membership and behavior; no separate controller group store is created.

Campaign panel shortcuts use the original native shortcut path. Clan Management
uses the original Clan button handler, including its native Initialise dispatch.
B queues Escape in the native keyboard phase; it no longer closes all panels
inside our draw callback. The original PanelManager.OpenPanels result gates map
panning, turning and zoom so the map cannot move behind a modal panel. Trackpad
cursor and panel scrolling remain available. Rapid D-pad requests coalesce and
allow500ms for panel transitions; focus/root changes discard pending requests.

Battle RTS L3 supplies native Ctrl through both mouse messages and the engine's
own GetAsyncKeyState import. It preserves physical Ctrl, clears when unfocused
or stale and refuses an already replaced import. No global keyboard mutation
is used. Campaign Ctrl and Shift merge into the original native keyboard poll,
with focus, epoch and freshness checks, then refresh the original UI selection
cache on its native input thread. Possession L3 remains held sprint.
Ordinary trigger clicks absorb small pad jitter, then allow deliberate drags;
Ctrl/Shift selection clicks stay pinned until release.

Visible campaign cursor placement is preserved during left-stick panning. It
can park at center only after1500ms idle AND the OS reports it hidden. The game
owns cursor visibility; the mod does not change ShowCursor's global counter.

The native Steam API configuration is now3.5, refreshing labels for the new
shoulder zoom, campaign A, L3 fast pan and selection modifiers. Legacy keyboard/mouse and
XInput layouts are excluded from that forced refresh. No Steam configuration
file is edited by the mod.

The regular/public mod and multiplayer build are not updated by this candidate.
No recorder, debugger, continuous trace or growing log is included or started.

The controls guide uses the native action-set labels and Valve-supplied glyphs;
local Shogun fonts are prepared from the player's own game. No game artwork or
fonts are distributed. The old Steam controls-screen route is unused.

## Revision17 verification limits

The original native Ctrl producer, UI cached-Ctrl getter and battlefield event
constructor are exercised in inert owned images. Tests reproduce the old cache
mismatch and check restoration of physical keys. The unused GetAsyncKeyState
import hook is no longer installed. Ctrl-selection pins the chosen point during
R2 presses so pad motion cannot turn that click into a card drag. Ordinary box
selection keeps its existing drag behavior.

The original Clan toolbar component/controller and nested window dimensions
were read with the established MIT Total War UI decoder and checked against
owned game assets. Clan is activated through its normal game-window button
events, outside the Lua draw callback and keyboard poll. Other panel shortcuts
and the accepted cameras remain unchanged. No original layouts are distributed.

Offline checks cover nested placement, stock fixed-size resize refusal and
restoring authored flags. They do not prove that every actual panel child fits,
that Clan contents populate, or that in-game multi-selection succeeds. Those
remain the next focused Deck test. Diplomacy B closed cleanly once in revision16;
this is not a crash-stability claim. The first-party controls guide was accepted.

The external SSH helper records bounded read-only input/status samples only
when separately authorized. It is not shipped. Its revision16 capture stopped
with that game process; no recorder or debugger starts with this build.

Revision22 battle additions: hold A for native Space formations/range; L4 + R4
selects all (R4 alone keeps melee mode). Hold D-pad assignments refresh the native
modifier cache. LB/RB zoom is continuous through the original native camera
methods. In-game controls and Steam layout labels reflect these bindings.
Battle cursor is owner-confirmed working and unchanged. Hardware validation of
the new candidate remains pending; no public/Workshop update is implied.
