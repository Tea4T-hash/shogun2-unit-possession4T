# Revision22 cursor and binding audit

This private candidate addresses the invisible cursor on the campaign map and in campaign panels/pop-ups. The owner reports that touching the screen restores it; right-trackpad motion still highlights buttons while the cursor is hidden. Y successfully ended the turn in the same retest, so its earlier failure remains intermittent and unproven.

## Evidence and implementation

The bounded, read-only check used the exact revision21 bridge and stock engine in one pinned Deck process. Across 1,198 samples the native cursor-enabled bytes remained `0101`, with a valid cursor handle. Gamescope visibility feedback alternated between shown and hidden; it became hidden at 2.504 seconds while the right pad was touched. There were 951 samples with pad touch and 107 of 120 compositor samples reported hidden. It became shown again at 34.566 seconds. Touchscreen events were not instrumented, so the owner's confirmation establishes the tap behavior separately; do not assign a particular transition to a tap from timestamps alone.

Valve's [cursor warp handling](https://github.com/ValveSoftware/gamescope/blob/309eee74f967d458cf71836619132010f88ee796/src/wlserver.cpp) distinguishes synthetic movement from activity that unhides the cursor. Its [compositor cursor implementation](https://github.com/ValveSoftware/gamescope/blob/309eee74f967d458cf71836619132010f88ee796/src/steamcompmgr.cpp) can have a valid cursor image while hiding it. These pinned sources explain the mechanism; the installed Gamescope build has not been asserted identical to that commit.

`native/deck/cursor194.h` renders the game's actual current cursor image and hotspot through an owned, non-activating, click-through layered window while the right pad is touched, plus a two-second release allowance. It runs only in the existing physical-Deck frontend worker under Proton. It covers both the map and campaign/menu panels. It respects native hidden-cursor state, focus, client bounds, invalid input, help-guide ownership and the battle handoff. It never modifies ShowCursor counters, system cursor settings, input devices or the game's cursor artwork. The desktop profile bypasses this worker; battle cursor and possession camera paths are unchanged. Battle shoulder zoom is the separately requested exception described below.

Hardware verification is still required for compositing, z-order, modal transitions and the moment hardware-cursor visibility returns. The mirrored cursor may coincide with a visible hardware cursor at the same hotspot; no global hiding policy is introduced. The cursor image is refreshed at most 30 times per second, with position updated at the worker cadence. Dimensions are limited to 128 by 128 pixels and graphics objects are released. No routine logs or recorder are added.

## Button audit

The audit follows each active Steam action label through its production handler, including game-native key scans, selection modifiers, queued orders, holds, context changes and release handling. `verify_bindings194.cpp` supplements the existing control, native selection, native keyboard, group, command-wheel, trigger, camera and View/HUD suites. It exercises actual campaign/menu routes, every direct possession key/release, repeated trigger releases, and the cursor renderer/lifetime without sending game or system input.

Campaign Y and R4 still require a 600 ms hold. A native panel previously masked their output after consuming the hold timer. This candidate resets the timer while a panel or the controls guide owns input and requires a fresh complete hold on the map. This is a reproduced policy problem; it does not establish why the owner's first Y attempt failed.

The layout remains `Unit Possession Layout` version3.5. The user subsequently requested inverted campaign right-stick zoom and L3 fast panning. Those two campaign policies change; the accepted native pan/zoom formulas, battle pan/turn/tilt and possession cameras are preserved; the requested shoulder zoom now uses continuous native zoom instead of wheel steps. L3 retains Ctrl selection as well as enabling the existing 2x pan multiplier. Right-trackpad click remains unassigned everywhere. Menu stick/D-pad focus navigation remains deferred. The former RTS shoulder fire/run orders remain unassigned as requested when the shoulders became zoom controls.

## Current mapping

### Menus

| Input | Action |
| --- | --- |
| A | Select |
| B | Back |
| View | Hold: controls guide |
| Menu | Skip intro / back |
| LeftPad | Scroll |
| RightPad | Move cursor |
| L2 | Right click |
| R2 | Select / drag |

### Campaign

| Input | Action |
| --- | --- |
| A | Toggle movement speed (Space) |
| B | Cancel / back |
| X | Zoom home |
| Y | End turn (hold) |
| L1 | Previous selection |
| R1 | Next selection |
| L3 | Hold: faster map pan / CTRL selection |
| Up | Mastery of Arts |
| Down | Finances |
| Left | Diplomacy |
| Right | Clan Management |
| View | Hold: controls guide |
| Menu | Game menu |
| L4 | SHIFT: select card range (hold) |
| L5 | Add / remove selection (hold) |
| R4 | Quick-save (hold) |
| R5 | Toggle character movement speed |
| LeftStick | Pan campaign map |
| RightStick | Turn / zoom camera |
| LeftPad | Scroll / zoom |
| RightPad | Cursor; panel L1 + swipe: scroll |
| L2 | Move / attack |
| R2 | Select / interact |

### Battle (RTS)

| Input | Action |
| --- | --- |
| A | Hold: show formations / range (Space) |
| B | Cancel placement / deselect |
| X | Stop selected units |
| Y | Possess soldier |
| L1 | Zoom in (hold) |
| R1 | Zoom out (hold) |
| L3 | Add / remove selection (hold) |
| Up | Group 1: tap to select / hold to assign |
| Down | Group 2: tap to select / hold to assign |
| Left | Group 3: tap to select / hold to assign |
| Right | Group 4: tap to select / hold to assign |
| View | Hide / cycle HUD; hold: controls guide |
| Menu | Game menu |
| L4 | SHIFT: range / queued orders (hold) |
| L5 | Add selection / group modifier (hold) |
| R4 | Toggle melee mode; L4 + R4 selects all |
| R5 | Group / ungroup; L5: lock formation |
| LeftStick | Pan battle camera |
| RightStick | Turn / tilt camera |
| LeftPad | Zoom camera |
| RightPad | Move cursor |
| L2 | Move / attack / formation; double: run |
| R2 | Select / drag box; double: focus unit |

### Possession

| Input | Action |
| --- | --- |
| A | Climb / interact |
| B | Mount / dismount unit |
| X | Toggle unit melee / ranged |
| Y | Return to RTS |
| L1 | Unit order / formation; double: run |
| R1 | Command wheel (hold) |
| L3 | Run (hold) |
| R3 | Change first / third person (hold) |
| Up | Cycle ability / ammunition |
| Down | Use / end ability |
| Left | Third-person camera closer |
| Right | Third-person camera farther |
| View | Hide / cycle HUD; hold: controls guide |
| Menu | Game menu (remain possessed) |
| L4 | Orbit camera (hold) |
| L5 | Crouch (hold) |
| R4 | Stop unit |
| R5 | Follow Me |
| LeftStick | Move soldier |
| RightStick | Look / command wheel choice |
| LeftPad | Third-person camera distance |
| RightPad | Look / touch for gyro |
| L2 | Ready weapon / secondary melee response |
| R2 | Fire / hold-release bow / primary melee response |

The private validation receipt contains the complete route audit. Offline success does not establish all in-game outcomes, controller feel, or long-session stability. Do not promote this candidate without a new owner test.

## Selection cycling confirmation

The owned game's campaign-root bytecode handles select_next/select_prev by calling
CampaignUI.NextAutoEntitySelection/PreviousAutoEntitySelection. The mod uses these
same functions on the UI callback. A targeted, hash-verified review of cached
native analysis shows dispatch from the current selection object, not a fixed
castle list. The owner then tested an agent/army and confirmed that it follows
the selected type. This path is retained; no unverified category iterator or
additional native engine call was added.

## Battle follow-up in this candidate

The owner reports that the battle cursor is visible and working. The new cursor
presentation stays in menu/campaign; the battle cursor is preserved.

Battle A now holds the original Space scan (`show_allied_units_proxies` in the
owned default key definitions). Hold L4 and press R4 to select all. R4 alone
retains melee mode. The generated Ctrl+A does not add the controller's held
Shift; physical keyboard bits are preserved. Labels and the in-game guide match.

Holding a D-pad direction for550ms still requests one group assignment. The
original engine handler f91e40 chooses assignment from cached Ctrl at1bbe84f,
separately from the DirectInput keys. The old merge did not refresh that cache.
An inert original-engine fixture reproduces a no-op recall branch with a digit
and Ctrl present in DI but cached Ctrl false. The corrected merge calls the
already guarded native cache producer f92ae0 after merging the keys. All four
hold gestures reach the original assignment branch with several selected units
in both stock and reviewed Bugfix engine fixtures. Actual selection allocation
and notification are mocked; this is not live gameplay acceptance or proof that
no other live group issue exists. Physical Ctrl and release behavior are tested.

Battle LB/RB call the original cam_down/cam_up camera methods at vtable slots
a8/a4 instead of accumulating mouse-wheel notches. Native2f7a50 retains its
distance/rate handling, with the existing tick-ratio compensation used by pan
and turn. Original engine code moves on every update across32 rate/direction
combinations. Campaign, stick pan/turn/tilt and possession camera paths retain
their accepted behavior. Native analysis was read from the existing hash-verified
on-demand decoder index; no additional engine hook or copied game code is shipped.

## Revision23 shoulder feel adjustment

Owner reports cursor works everywhere and zoom is smooth, but battle LB/RB is too
fast. Only those shoulders now use a 450 ms smoothstep acceleration curve and an
80% peak multiplier. The curve is integrated per callback for consistent travel
across frame rates. Release, reversal, focus/context/identity changes and missing
fresh input reset it; no zoom coasting. Native distance/rate/limits still apply.
Campaign right-stick zoom and all other camera/input paths remain unchanged.


## Revision23: restoring campaign shortcuts after a battle

The owner reported that B and held Y stopped working only after returning from
battle. A fresh authorized SSH check of the exact revision22 process confirmed
campaign input was active and receiving both buttons. All 1,199 samples still
showed the original keyboard poll (Empire.Retail.dll + 0xbd75f0) jumping into
the gameplay adapter, although the frontend had regained controller ownership.
The camera and cursor use separate paths, explaining why those still worked.

The gameplay adapter previously retained its native keyboard hook for its whole
process lifetime. The campaign bridge deliberately requires the original poll
before enabling its own hook, so it correctly refused to overwrite that site.

The private Deck adapter now returns the original poll bytes **before** releasing
its battle ownership lease. On the next battle it waits for the frontend lease
to end, then reacquires its own hook. Its disabled trampoline stays allocated
so calls already in flight can return. Exact original/owned bytes are checked
before each transition; an unknown hook is preserved and refused, never chained.
Desktop behavior retains its existing path. The regular public repository and
installed engine file are unchanged.

The two-DLL offline fixture uses independent MinHook instances, reproduces the
old retained-hook failure, then checks 30 round trips and 124 Back, Enter and
panel pulses. It also verifies physical input preservation, release edges,
focus/epoch rejection and refusal to overwrite a foreign hook. The compiled
production adapter's yield path is separately checked for 12 ownership cycles.
These checks establish the mechanism offline; the combined candidate still
requires a real campaign-to-battle-to-campaign retest on Deck.
