# PC K / Deck View HUD parity — revision25

This is a local candidate based on accepted revision24, not a published update.

## Behavior

While possessed, K on a keyboard and a tap of View on Deck share this cycle:

1. Hide the native RTS HUD; keep the possession HUD.
2. Hide both HUDs.
3. Restore both HUDs.

QTE and response prompts remain independent. RTS keyboard K keeps its original
native toggle. Held K is drained on possession exit, preventing a delayed RTS
command; keyboard autorepeat does not advance the cycle.

## Cause and correction

Revision24's View dispatcher kept a three-stage state, but the HUD only consumed
that state from a controller presentation publication. Desktop mode deliberately
never starts the controller worker. Keyboard K continued through the original
native two-state toggle and its visibility channel hid the possession HUD too.

Both routes now use `native/deck/hud_cycle198.h`, scoped to the current native
root and possession lease. The original HUD channel publishes the resolved
possession-HUD flag, without a controller-sample freshness dependency. Native
Hide UI still owns the RTS flag. A changed native flag or lease rebases the
cycle instead of retaining another actor's state. The shared mapping ABI is
unchanged; existing identity, size and sequence checks remain.

The owning-window callback in `adapter175.h` admits keyboard K even when the
Steam Input worker is bypassed. `battle181.h` calls the same guarded native
Hide UI routine as View. The native keyboard poll masks duplicate K handling
while possessed and drains its held state on exit. No camera, combat, timing,
UI sizing, Steam layout or resource-extraction behavior is changed.

## Evidence and next test

The actual revision24 dispatch fixture reproduced the desktop visibility-channel
failure before modification (exit77). The focused regression runs the compiled
adapter dispatcher and real HUD publication/reader with Steam Input bypassed.
Only the native UI callee and game objects are inert fixture substitutes; this
does not establish on-screen game rendering. The actual keyboard hook fixture
also exercises current/previous native buffers, repeats, release, RTS return,
lease changes and native visibility changes.

Build/check identity and final results belong in DECK_STATE.json and the bounded
D: review directory. Hardware acceptance of K's new cycle remains pending.
Smallest test: in a PC battle possess a soldier, press and release K three times,
then hold it once; verify the three views and no repeated changes. Return with
F10 and confirm ordinary K toggles the RTS HUD. A later Deck View check should
confirm the accepted cycle remains unchanged.

Installation requires explicit staging approval, a closed PC game/launcher,
and an exact rollback. Building does not upload or replace an active pack.

## Completed candidate

Final build: `D:/Shogun2Deck/builds/Test181/hud25-full-3`.
100 adapter, 50 QTE and 8 HUD checks passed in the fresh component build;
the final build inherited those only after exact component dependency checks.
It ran the corrected focused fixtures and 20 Workshop checks freshly. 52 Python
checks, roles/six tables, seven public-reader cases and exact rollback passed.
See `D:/Shogun2Deck/builds/Test181/hud25-review-1/final-verification.json`.

The owner approved staging with the PC game/launcher closed. Main item3825625050
now has the candidate locally; its previous combined build is backed up.
No upload or GitHub publication occurred. The new K behavior still needs its
small in-game check. Rollback requires a closed PC game/launcher:

    python D:/Shogun2Deck/promotion/hud25-workshop-1/manage.py rollback --closed
