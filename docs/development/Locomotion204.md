# Revision31: movement during formation idle animations

This correction follows a loaded revision30 custom-siege observation. The
possessed soldier moved while the parent formation stayed still, but native
standing-idle clips remained active. The owner also saw the idle animation near
the end of the observation. The accepted camera framing and input feel stay.

## What changed

After the native movement integrator completes our own ordinary infantry intent,
the completion hook interrupts only a verified standing-idle clip using the
game's own interruption routine. The clip must belong to the current soldier's
animation database and agree with the active animation ID. It then uses native
idle/walk/run classification from the completed physical movement. Consumed
planner intent no longer makes an actually moving soldier look stationary.

There are37 native standing-idle categories. They were identified in the pinned
engine's enum and compared with already decoded owned-game animation fragments.
No animation assets are shipped. No animation clock, position, velocity, facing
or parent-unit order is changed. Native gait selection remains the existing
input/run-permission path. Non-possessed actors are untouched.

Combat pairs, readiness transitions, ranged/weapon phases, mounted actors, charge,
climb, recovery, invalid identity and unknown clips remain excluded. This is an
ordinary-infantry correction; it is not proof that every mounted rig or rendered
animation issue is solved.

## Reproduction and verification

The previous source fails the new reproduction with the observed active idle
clip; the corrected source passes. The isolated fixture executes the original
366a10 caller,3dd9b0 interruption checks/cleanup and3e4850/3e44f0 pose routines.
Integration velocity and running capability are explicit fixture providers.
Every executed native routine's disassembly is checked against the pinned
stock engine. No live process is modified by these tests.

Coverage includes37 idle categories while moving and standing, seven non-idle
clip categories, mismatched clip ownership, weapon-phase exclusion, consumed
intent while walking/running/stopping, unchanged velocity/animation clock and
non-possessed peers. Existing36 directional pose cases and884 projectile/native
state cases still pass. Full build results are recorded separately.

Evidence: D:/Shogun2Deck/builds/Test181/gameplay204-review-1. Original bounded
observation: D:/Shogun2Deck/diagnostics/revision30-siege-1/actor-long.jsonl.
The observation completed; no recorder/debugger or continuing capture is active.

## Remaining work

Actual in-game rendering needs owner validation. Initial enemy acquisition of
the lone soldier, the campaign-to-siege layout failure and the earlier QTE crash
remain unresolved. This correction does not manufacture a battle unit or force
enemy orders. Installation/transfer requires separate approval and verified
rollback; no new Workshop or GitHub publication is part of this pass.

## Additional targeting investigation, not a shipped fix

Original native15d1c0 applies phase/roster and unit-dependent range conditions,
then calls180cd0 with the observer and target primary-group rectangles at
unit+b9c/group+644. The caller1bde40 uses it when building an opponent-unit list
for units gated by the existing eb5 flag. No semantic name is assumed for that
flag. These bodies were checked against the pinned engine instructions.

An11-case isolated experiment reproduces refusal with a main formation300m away,
even when a separate fixture group is20m away. Supplying the near group's geometry
only in the isolated fixture makes the original routine accept. Dead/empty roster
and non-battle-phase exclusions remain. This identifies an upstream omission,
not the entire live AI decision, ranged line of sight, or a safe separate-unit
lifecycle. No runtime hook or unit-pointer substitution was added.

The first fixture declared a C++ bool return; its assertion failed. The corrected
fixture reads the native AL byte explicitly as unsigned char and passes. Both
logs and sources are retained. Evidence is under the private review folder's
acquisition-range directory.

## Deck cursor-edge camera suppression (205, same revision31)

The owner additionally requested stick-only map movement/rotation in campaign
and battle. Original battle handler304550 queries ui_mouse_scroll (option48)
at30481b; the campaign handler queries the same option at8fb626. Both immediately
compare the returned byte and branch around their cursor-edge camera steps.
The exact query windows match stock and reviewed Bugfix and already lie within
the unchanged engine contract regions. On Valve-identified Deck only, the pinned
bridge replaces those two consumers with a false byte, preserving their original
CMP/JZ and four-byte callee cleanup. The preference itself, cursor coordinates,
selection/dragging and direct stick camera methods are not changed. Desktop does
not install the hooks. No saved settings or Steam configuration are edited.

verify_edges205 executes both ORIGINAL native query/CMP/JZ sequences in owned
mapped engine fixtures:216 cases across stock/Bugfix, preference true/false,
nine cursor locations, unmodified/Deck/rollback states. It checks stack and
nonvolatile registers, untouched cursor and preference, ordinary option getter
behavior, exact restoration, and refusal of either changed/occupied site before
modifying the other. Scene callbacks and rendering are not executed; this is not
hardware acceptance. The first harness compile had assembly syntax errors;
correcting the harness yielded the passing run. Production scope is two native
option consumers, not all mouse input or the whole camera update.

Valve's [device-query documentation](https://partner.steamgames.com/doc/api/isteamutils#IsSteamRunningOnSteamDeck)
confirms the existing physical-Deck selection API. The original engine evidence,
not a generic camera recipe, establishes the two edge-input checks. The game's
[manual](https://www.feralinteractive.com/en/manuals/shogun2tw/latest/steam/)
provides the separate ordinary camera controls; the Windows native addresses
above come exclusively from the locally owned engine analysis.
