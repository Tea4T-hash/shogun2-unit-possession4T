# Revision32: cavalry movement with a stationary formation

Revision31's owner test accepted infantry walk/run animation and battle cursor
edge suppression, but mounted soldiers could still slide while appearing idle.
The subsequent authorized read-only observation saved87 samples from the same
verified process. Of72 moving samples,46 had the horse in a standing pose; five
also contained an active, owned standing-idle clip. It stopped when the
possession camera disappeared. There is no ongoing capture.

## Cause and correction

The existing completion hook explicitly excluded mounted possession. The horse
is the movement actor: native41fda0 calls the shared3c1d90/366a10 movement path,
then attaches its rider. The captured rider used mode29; the horse used mode0.
Calling infantry locomotion on the rider would address the wrong object.

The hook now admits only the validated movement actor of the current possession
lease. For cavalry, reciprocal horse/rider ownership, serials, roster and native
unit identity must still match. The mounted rider must remain in ordinary
mounted mode29 or30 without its own explicit clip, combat or animation flags.
The rider itself is never given the horse's moving pose.

After completed player movement, the horse uses original3e4850/3e44f0 pose
classification. A verified active standing-idle clip can be interrupted through
original3dd9b0, with the same animation-database ownership checks as infantry.
The original native pose/asset selection handles the horse's animation rig;
this mod does not ship or substitute animations.

No velocity, position, facing, parent-unit orders or animation clocks are changed
by this correction. The existing sprint/run-permission path supplies gait.
Charging, horse jumps, dismounting, ranged actions, combat, recovery, unknown
clips and invalid ownership yield to their existing native paths. Accepted
infantry, camera, climbing and controller behavior are retained.

## Verification

The same isolated mounted fixture fails on revision31 with an actually moving
horse still in idle pose, then passes with this correction. It executes the
original motion-completion, idle-interruption and pose routines in an owned
mapped engine image. The velocity integrator and running-permission result are
explicit fixture providers; this is not a complete battle/rendering test.

Coverage includes36 rider-mode/direction/gait/stop cases,16 moving/stationary
horse idle cases and37 special-state/identity guards. Rider and peer bytes,
physical velocity and animation clocks are checked. Existing infantry cases
and884 native weapon-presentation cases pass. Relevant native disassembly was
compared byte for byte to the pinned engine. The on-demand decoder supplied
owned-game horse/rider fragments for the animation-category investigation.

Evidence: D:/Shogun2Deck/builds/Test181/cavalry206-review-1.
Original observation: D:/Shogun2Deck/diagnostics/revision31-cavalry-1.

Full build/package results are recorded in DECK_STATE.json. In-game mounted
acceptance is still pending; revision31 remains installed until separately
approved staging. Enemy acquisition, campaign deployment controller loss and
the earlier melee/QTE crash remain unresolved.

## Owner acceptance

The owner subsequently reported "cav works" during the Deck trial. Cavalry
movement is accepted from that manual report. Exact revision32 pack and bridge
were verified at the main menu; the separate battle component check was not
completed. This does not establish all-unit coverage or long-term stability.
The installed build and rollback are retained; no publication was performed.
