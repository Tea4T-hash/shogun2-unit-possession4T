# Revision39: release excluded QTE candidate reservations

## Live evidence

The owner's revision38 siege still failed: the last spear levy defender was
alive, not routing, and approached by opposing soldiers who turned away.
Read-only SSH snapshots in verified PID7214 showed actor serial542/index541
with no witness688, animation owner18 or marker1e8. The common native busy bit
remained set across advancing model ticks; both Combat and Avoidance Witness
Managers had zero active and pending encounters. Nearby attackers were at the
same height, within about1.7m, with no active witnesses. This is an orphaned
combat reservation. Revision38 is recorded as a failed live fix, not accepted.

## Reproduced native lifecycle error

Original3c0b10 reserves a busy bit for each initial candidate before preparation.
Original3bf170/3bf780 destructors release the actors remaining in that list.
The QTE's retain_selected changed a three-to-five-member native list to the
selected two but never released excluded reservations. Those actors vanished
from the destructor's list; their busy flags survived without a witness and
original3f4f30 continued refusing them for melee. The old native-fixture control
with an unmodified list releases everybody. The current pair-narrowing baseline
reproduces six orphaned actors per engine. Stock and reviewed Bugfix both match.
This reproduces a direct source mechanism for the captured orphaned flag; the
original creation moment in the real battle was not instrumented.

## Correction

After successful list writes, excluded initial candidates are released through
the existing original3e1930 actor-release port on the native callback thread.
Both selected fighters remain reserved for original preparation. The helper
requires all initial backlinks to be zero and the candidate list to be unique.
A missing cleanup port refuses narrowing. Failed list/count writes retain or
restore the full original list, with no premature release. The legacy unused
nearest-pair helper shares the same cleanup rule.

No direct global-bit clearing, live repair, new AI orders, changed casualty or
routing rules, new animations or native RNG rerolls are added. The existing
native selection chooses its original pair once. PC and Deck use this common
path. Revision38's current-lease guard and Renderer211 coexistence stay intact.
Movement, camera, controls, HUD and local-resource source are unchanged.

## Verification

verify_melee213.cpp executes original candidate registration, release,
eligibility and destruction in isolated mapped owned images, without game
initialization. Sixty cases cover foot/mounted actors,3/4/5 candidates, original
control, successful retention, missing cleanup and both failed-write boundaries.
Selected fighters keep their reservations; excluded fighters are eligible
again, and native destruction leaves every actor eligible. The native flags
are not mocked. Multi-participant QTE, native-selected target/roles, existing
active reservation, charge and RTS regressions remain required.

Private native evidence, before/after runs and live snapshots stay on D:.
The full revision39 build passed 100 adapter, 51 QTE and 8 HUD executables,
including the 60 original-engine candidate cases. Startup, campaign handoff,
seven actual package-reader cases and exact PC/Deck rollback fixtures passed.
Revision39 was installed and hash-verified on PC and Deck with exact revision38
rollback preserved. The owner reported “success”; exact scenario and feature
coverage were not specified. A formal revision39 loaded-hash/hook gate was not
completed. No Workshop upload is part of the GitHub source publication.
