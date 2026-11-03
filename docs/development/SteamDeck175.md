# Private Steam Deck RTS test 175

This is the first playable **RTS controller increment**, following the owner's
successful native-input diagnostic. Full controller support is still in
development. The released Windows mod and public Workshop item are unchanged.

Use the existing private item **3829785193**, with official **Proton 10.0-4**
(10.0-4b maintenance build is also suitable). Keep the regular possession mod
off while this private full-mod candidate is enabled. Restart the Deck before
testing so the earlier diagnostic's Steam Input session has ended.

## Controls implemented in this increment

| Control | RTS action |
| --- | --- |
| Left stick | Pan the native RTS camera |
| Hold L3 while moving left stick | Faster camera pan |
| Right stick left/right | Rotate camera |
| Right stick up/down | Native wheel zoom |
| Hold R3, right stick up/down | Camera pitch |
| Right trackpad swipe | Move the native game cursor |
| Right trackpad click/hold and swipe | Native unit selection/selection box |
| A | Native selection click at the cursor |
| Hold R5 while selecting | Native additive-selection modifier |
| R2 press, swipe, release | Native contextual order/formation drag and commit |
| B during a drag | Cancel without sending the order |
| X | Native Halt for the current land-unit selection |

The game owns hit-testing, selection, formation preview, placement and order
eligibility. R2 uses the game's normal contextual right-click path. These are
not replacement movement or combat rules. Actual behavior and sensitivity on
the Deck still require the owner test; desktop fixtures do not prove them.

Other controls have not been connected yet. R1 run/walk and queued orders,
selection shortcuts, groups, wheels, menu/campaign/naval control and possession
controller/QTE input are pending. R1+R2 is refused rather than silently issuing
a replacing order. An unsupported wheel cancels owned held gestures. Mouse and
keyboard possession remain available, but are not the Deck test's acceptance.

## Implementation and checks

Valve Steam Input006 supplies native actions inside the Proton game process.
The existing adapter worker samples numeric frames into a bounded32-entry FIFO.
Only the game's owning window thread dispatches them; current battle, camera,
focus, mode and identity are rechecked before native calls. Focus loss, stale
frames, overflow and a changed world cancel owned gestures and require neutral
input before resuming. Continuous camera time is capped at50ms after a stall.

The right pad moves the focused game cursor using SetCursorPos. Selection and
formation gestures call the existing in-process native window procedure, as
the mod's Q placement already does. Owned drag bits are retained on mouse moves;
this does not use SendInput, keyboard bindings, a virtual gamepad or a Steam
radial overlay. Steam and Quick Access retain their system functions.

Camera methods and Halt are recovered from the preserved on-demand decoder,
indexed native analysis and read-only Ghidra exports.31 whole native bodies and
relocations are hash-checked against separately owned stock and supported Bugfix
images. Halt calls the existing UI action dispatcher10dfb80/action4; it does
not construct its own command packet. Land-selection and queue-capacity checks
run first, with native eligibility retained.

The build generates a separate source snapshot. Reversing its adapter/extractor
substitutions recovers all prior text. The normal adapter and engine contract
are not changed. Every new backed numeric reference must already lie in the
accepted immutable regions; unknown images or occupied hooks are not allowed.
No game bytes, SDK header bodies, fonts, artwork or voice are bundled. Required
resources are prepared from the user's owned game using the existing pipeline.

The candidate does not start a recorder, debugger or profiler, create gameplay
logs, change Steam user configuration files, replace the engine or alter CPU
settings. Small native/config/resource caches remain necessary. The Valve
manifest override lasts the Steam session: disable the test and **restart the
Deck before normal play**. Shutdown alone does not prove the old layout returns.

## Build

Run `tools/deck/build175.py --help` for required owned-fixture/tool paths. Use a
new output outside the game folder. `--quick` compiles the adapter and boundary
fixtures only; it does not produce a distributable build. A full build additionally
runs the maintained component tests and Workshop startup/resource/compatibility
checks. No builder installs, launches or uploads. The private publisher filename
must retain `shogun2_unit_possession_deck_probe174` for Update to the same private
item; never stage this candidate as the normal public Workshop entry.

Keep full build folders private: they contain recovered scripts and locally
prepared fixture resources. Stage only the audited pack/preview with rollback
after the required closed-game check. A new live test needs a readiness cue.
