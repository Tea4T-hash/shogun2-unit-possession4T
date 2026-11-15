# Revision38: possession-scoped melee reservations

The owner reports late-battle melee stopping after returning from possession to
RTS, with about a dozen survivors, including two samurai defending a castle.
They appeared to acquire opponents and turn away. Arrows still killed them.
The battle ended before a failing-state memory read; there is no captured native
state proving this report's complete cause. A community report describes the
same low-survivor symptom across unit types.

## Reproduced defect

The QTE conflict wrappers previously accepted a retained encounter address and
living saved actor identities as authority to block other melee preparation,
contact, target assignment, actor release and pre-play avoidance. They did not
require the original possession lease to remain active. An isolated test of the
actual production wrappers reproduces all five suppressions after the player
returns to RTS. This is a code defect consistent with the report, not a complete
reproduction of the finished battle or proof that low troop count causes it.

The original engine removal/release/contact instructions at 3e1930, 3cea10,
3e1970, 3e5aa0 and 3eaa10 were compared byte-for-byte with the owned stock image.
An additional mapped-image test executes original3e1930 actor release and
original3f4f30 melee eligibility: the old wrapper preserves busy bits/backlinks
in RTS and native melee stays refused; the corrected wrapper permits original
cleanup and native eligibility returns. Stock and reviewed Bugfix each pass
eight cases, at represented counts240/12/2/1 for both former participants.
The image is mapped with DONT_RESOLVE_DLL_REFERENCES; no game initialization,
third-party code, orders, animation playback or live process is run.
There is no new animation, forced casualty, altered routing behavior or native
engine API. Bow/projectile code is unchanged.

## Correction

A reservation now also requires a fresh stable possession status, the matching
lease, the current physical camera actor, and the exact still-registered native
encounter generation. RTS, lease release/replacement, stale status and witness
removal/address reuse immediately stop this extra suppression. Existing active
QTE identities and mounted rider/horse proofs remain required. Checks read
state only; original native callbacks perform combat and cleanup.

This common QTE path covers PC and Deck and is not restricted to a faction,
weapon, siege, unit type or survivor threshold. No special last-man-standing
exception is invented. Current personal charges and the Renderer211 coexistence
correction are retained.

## Verification and limits

The new production-hook regression verifies active protection, RTS prepare and
update, contact preparation/update, target assignment, release and avoidance,
full native return values, exact call counts, replaced leases and reused native
encounters. It includes 4,000 RTS cycles at represented counts240,12,2,1. Counts
are test data, not a complete simulation. Active foot/mounted reservation and
charge tests remain separate. Updated full-build results are recorded privately.

No live files were replaced and nothing was uploaded by this source correction.
Actual end-of-battle melee, castle last-man-standing and repeated possession/RTS
transitions still require owner gameplay testing. Enemy acquisition while
separated remains an independently unresolved issue.

## Subsequent live result

Revision38 did not resolve the owner's siege failure. A later bounded read-only
check captured a lone defender with an orphaned native busy bit and empty
encounter queues. Revision39 corrects the separately reproduced candidate-list
cleanup leak while retaining this current-possession guard. See
[MeleeCandidates213.md](MeleeCandidates213.md) and the current validation record.
The owner reported revision39 successful after installation.
