# Personal wall climbing (revision27 candidate)

Charge199 is owner-accepted and unchanged. The owner reported that climbing still
needed a unit wall order and could lock movement until leaving possession.

The original per-actor request builder at RVA3ccd00 reads two permissions from
unit state: portal traversal (+34,3c1fb0/14f750) and castle climbing
(+35,3c1f80/14f720). It can also attach the unit's selected wall list at+40.
Original371b60 rejects pipe traversal without the first permission;384120 rejects
castle pipe kinds11–14 without the second and consults the unit list via1be3c0.
Those dependencies explain a concrete way that a personal request can stall
without a unit climb order. They are verified in exact owned engine bytes, not
inferred from an animation name.

For an explicit F/A request only, the soldier moves to the existing registered
native independent subgroup at the unit update boundary. Ownership, master
roster, stats and peers remain intact. The route starts only after proving that
membership. Its native request receives portal/castle permission and no unit
wall-list restriction. Identity, generation, current wall and exact goal must
all match. Other requests remain unchanged. Native wall availability, target
surface and region restrictions remain enforced. Native pathfinding aligns,
approaches and attaches the soldier; no teleport or direct premature binding.
Formation idle/move tasks cannot replace this personal route during approach,
climbing or landing. Normal proximity policy resumes afterward.

A pending approach cancels through original3df380 after two seconds without
position/heading progress, or eight seconds overall, measured in simulation
time. Pausing does not spend that budget. A second F/A cancels a pending
approach. A replaced goal is left untouched. Attached traversal is not forcibly
cancelled or teleported by the approach timeout.

Offline coverage uses the original request builder, portal-admission predicate,
castle-kind predicate, cancellation, native group factory/transfer/retirement
and real hook trampolines. It covers kinds11–14, ordinary/officer/banner human
roles, both endpoints, rotated geometry, elevated/ground surfaces, stale identity,
disabled walls/surfaces, changed goals and stalled routes. Terrain and the async
route solver are fixture providers; this is not an end-to-end siege simulation.
Actual castle maps, admission/arrival timing and crest completion remain live
validation. No claim of universal live wall success is made.
