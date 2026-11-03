# Current implementation plan: independent wall climbing

Charge199 is accepted by the owner. Its source remains exact. The remaining
wall failure is that the soldier needs a formation climb order and can remain
movement-locked until leaving possession.

Revision27 gives an explicit personal climb its own native subgroup and route
permissions, without changing the formation's order. Formation movement tasks
cannot overwrite the approach, climb or landing. Native wall availability and
route solving remain in control. A stalled pending approach releases movement;
F on PC / A on Deck can also cancel the pending approach.

The four native castle wall kinds, original request/admission/cancel functions,
independent-group lifecycle and hook ABI pass offline checks. The full build,
PC/Deck integration, existing charge checks, 52 Python tests and exact rollback
also pass. See [native evidence and limits](WallClimbing200.md).

Candidate: `D:/Shogun2Deck/promotion/wall27-workshop-1`.
Evidence: `D:/Shogun2Deck/builds/Test181/wall200-review-1/final-verification.json`.
The owner approved staging; revision27 is now active in the main local mod entry.
The exact revision26 backup and tested rollback remain on D:. Nothing uploaded.
Actual castle maps and approach/attachment/crest timing still need live testing.

After explicit staging approval, test F/A at an eligible wall while the unit
has no climb order, including an inner castle wall. Verify approach, attachment,
crest and landing, and that a rejected/stalled attempt returns movement without
F10. Preserve the unit's own orders. No new charge test is required solely for
this change; its source and regression checks are retained.

Enemy acquisition remains unresolved. No new AI override, SSH session,
recorder/debugger, GitHub publication or Workshop upload is part of this pass.
Only the PC receiving the eventual staging swap needs its game/launcher closed.

# Previous revision26 implementation and evidence

Scope: the owner accepted combined PC/Deck HUD198 (revision25), then requested
charge, climbing and enemy acquisition fixes. Use the longer workflow only
where useful. Multiplayer, naval work and control redesign are outside this pass.
The accepted dirty baseline is preserved in the private gameplay199 review archive.

| Area | Confirmed gap and correction | Evidence / next step |
|---|---|---|
| Personal charge | Sprint alone admitted a charge; entry cleared Follow Me. Require primary attack + forward sprint through the existing PC/Deck input reader; retain Follow Me and the unit's orders. | Old code fails both new regression cases. Corrected native foot/horse/rider, nearest-forward target, collision/recovery/contact and release fixtures pass. Full native/integration build passed; owner subsequently accepted charge in game; source retained exactly in revision27. |
| Individual climb | First request could be erased by lease reset; new aiming input could abandon an admitted native route. Consume a short-lived, lease-bound request before reset; retain a valid native approach, reject stale/blocked requests, publish approach/unavailable state. | Old code fails first-request regression. Native wall fixture passes; repeated/rejected input cannot queue a delayed climb. Native routing/attachment/crest requirements retained. Completed climbs still need live acceptance. |
| Enemy acquisition | Existing tests preassigned the target. That does not prove autonomous enemy selection. | Added reverse native perception check: an enemy with no target enumerates the separated player with the formation far away. Existing native group registration/update/target resolution work in fixtures. Tactical selection, ranged-task assignment and actual attack remain unresolved; no speculative AI override made. |
| PC/Deck and HUD | Accepted K/View parity, camera, bindings, wheel and device selection. | Preserve revision25 behavior and run its existing integration gates with the gameplay candidate. |

Personal charge uses LMB + forward sprint on PC and RT + forward sprint on Deck,
without ready/aim held. The nearest eligible forward enemy is selected by the
existing native perception/eligibility path; native outcome decides the result.
QTE remains edge-triggered: holding attack before a new prompt cannot answer it.

Private evidence: `D:/Shogun2Deck/builds/Test181/gameplay199-review-1`.
The original on-demand asset reader checked four owned animation entries;
byte-verified saved Ghidra exports cover the native calls inspected in this pass.
These are engine/asset evidence, not rendered-game or autonomous-AI acceptance.
Full engine code, original assets and decoded exports stay out of the package.

Online cross-checks: [Feral's original-game manual](https://www.feralinteractive.com/en/manuals/shogun2tw/latest/steam/)
and [the No Matched Combat author's description](https://steamcommunity.com/sharedfiles/filedetails/?id=2849860490).
Neither publishes the engine's acquisition ABI; local byte-verified engine evidence
is required for native calls. No invented charge kill, defense or leap animations.

Keep all work on D:. Offline work while playing is authorized. No live memory,
SSH, recorder/debugger, installation, upload or GitHub publication in this pass.
Any staging requires the concrete candidate, exact rollback and explicit approval;
only the device receiving a file swap needs its game/launcher closed.

## Next focused play check

Use the new candidate only after approval and exact rollback preparation. PC
charge input is LMB + W + Shift; Deck is RT + forward stick + held L3, without
ready/aim. Try an infantry approach and a mounted approach toward a stationary
enemy. Native eligibility/outcome still decides whether a charge encounter occurs.
For climbing, press F / A once at a suitable wall: note whether the approach
message appears, whether the soldier attaches, and whether crest/landing finish.

For isolated-player acquisition, keep the original formation outside the enemy's
local encounter and move the player into a normal, unobstructed attack opportunity.
If ignored, inspect that actor's current native subgroup/registry and enemy
perception/decision/task state under a separately authorized live check. Do not
preassign an enemy target, issue enemy orders, change aggro priority or count a
passing fixture as autonomous attack evidence. Recording/debugger stay off unless
explicitly requested. The existing offline checks do not establish the reported live failure.

## Candidate staged locally; upload deferred by owner

Test181 revision26 / gameplay199 is built: 100 adapter, 50 QTE, 8 HUD,
20 Workshop checks, PC/Deck integration fixtures and 52 Python tests passed.
Original engine guard header/regions/admission and native Deck source remain
exact to accepted revision25. Public candidate and tested rollback are at
`D:/Shogun2Deck/promotion/gameplay26-workshop-1`. Owner subsequently approved staging;
only the main publisher pack was replaced and independently verified. Nothing uploaded.
Enemy acquisition is not marked fixed; no new AI override is in this candidate.
