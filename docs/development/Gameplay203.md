# Revision30: framing, native movement presentation and target inclusion

Revision30 is installed on PC and Deck, without a new Workshop upload. Exact
loaded components and four hook destinations were verified during a custom siege.
The owner accepts third-person framing. Moving idle animations and isolated enemy
acquisition still fail; see the live follow-up below.

## Player-facing changes

- The actual third-person camera moves over the shoulder. Its sideways and
  vertical offset scale with the existing 1–12m scroll distance. The centre
  marker continues to describe the rendered camera ray used by commands and
  aiming. Existing orbit, scroll direction, first person and camera timing stay.
- An ordinary, manually moved infantry actor refreshes the game's pose classifier
  after native movement speed is computed. Walk/run uses the same input and native
  running permission as locomotion. Zero actual movement produces native idle.
  No new animations are distributed or selected by filename.
- The native unit-target group resolver can consider the possessed soldier's
  existing registered independent group when closer to an enemy than the main
  formation. The native formation-charge selector includes that same real actor
  using its original corridor and nearest-distance rule.
- Deck Battle RTS X toggles the original run/walk shortcut. It no longer issues
  Stop. Campaign X (zoom home) and possession X (weapon mode) remain unchanged.
  Steam Input layout is3.6, with matching guide labels.

## Native evidence and limits

The on-demand decoder read the original katana, bow and musket fragments and
animation table from owned data.pack. Private evidence/payloads stay under
D:/Shogun2Deck/builds/Test181/gameplay203-review-1; no game artwork or animation
payloads enter the source or pack. The existing Ghidra exports were checked
instruction-by-instruction against the pinned original engine. New consumers
remain inside the unchanged engine-contract regions; no unknown engine admitted.

366a10 invokes367600, then calculates1ac. Our former367600 callback finished
before that driver existed. The new completion hook refreshes3dd9d0 only after
our own successful ordinary human intent. Its3e4850/3e44f0 branches choose the
native idle/walk/run classification. Gait244 is0/1 using native15cb80. No motion
speed, animation clock, model facing, root motion or combat state is rewritten.
The hook's fast path leaves other actors untouched. Paired combat, explicit
clips, native combat readiness, mounted actors, aiming, recovery, charge and
climb retain their existing paths. This is a tested ordinary-infantry handoff,
not proof of every renderer/rig state or directional animation variant.

174ec0 scans only target-unit+b9c's members; it misses a soldier in the real
kind2 group. The added candidate uses the identical corridor and squared-distance
comparison and preserves original ties. 1679c0 returns the explicit actor's
group or unit+b9c. For unit-only targets, the added resolver compares current
registered group geometry from the enemy's existing context. It preserves
explicit targets, friendlies, native downstream visibility/range/fire gates,
unit ownership, roster, stats, morale and group lifecycle.

These changes do not create a new battle unit/card or a new strategic AI goal.
Whether an enemy chooses the parent unit at all, and whether actual rendered
shots/charges follow this separated actor, still require a real battle test.
The previous QTE crash remains undiagnosed; this is not a crash fix.

## Verification

Focused original-engine fixtures reproduce the omitted charge candidate and the
stale idle classification. New tests run the actual hooked selectors/caller,
using explicit world/allocation/integrator providers, with guards for release,
identity reuse, friendly targets, original corridor, explicit targets, native
clips and unchanged ownership. Camera coverage executes690 actual camera/aim
updates across every scroll notch, yaw/pitch and collision shortening. It does
not simulate GPU output or prove visual acceptance.

Full build and package results are recorded separately when complete. A real
swap requires explicit staging approval and an exact verified rollback.

## Historical candidate failures

The first broad builds reached older camera fixtures that measured Euclidean
eye-to-pivot distance as exactly the scroll distance. Shoulder composition adds
lateral/vertical components. Those fixture expectations were updated in the
isolated transform, including the death-view X coordinate; the first-person,
orbit, QTE and corpse-retirement assertions remain. The focused original-engine
75 pitched-camera cases and100 QTE/100 death updates passed afterward. Failed
build logs remain under gameplay30-full-1 and gameplay30-final-1/-2.

## Completed offline candidate

Final full build: `D:/Shogun2Deck/builds/Test181/gameplay30-final-3`. All100 adapter,
50 QTE and8 HUD executables, Workshop/bootstrap/profile/resource checks,52 Python
tests and7 native pack-reader cases passed. Exact29 rollback and20 protected PC
paths were verified. Pack SHA256: `784107bdc11bccd008a42712925eaae557c9e4cb9ef57bb6a4e2d24eca313208`.

The candidate was subsequently staged and transferred with exact revision29
rollback. It remains unuploaded. During this work the owner reported
a new controller failure entering a campaign castle defence. Existing saved Steam
logs were read after exit; older layout-load failures do not prove that episode.
This candidate does not claim to fix it. See private diagnostic summary on D.

## Live follow-up

The bounded 120-second read-only observation caught genuine movement while
explicit native STAND_TRAINED_IDLE clips152/153/156 remained active. It also
caught nonzero physical speed with consumed planner intent and idle pose1.
The earlier fixture excluded explicit clips and retained planner intent, so it
did not cover either observed failure. These are addressed by the separately
tested correction in [Locomotion204.md](Locomotion204.md).

A real registered one-soldier group was observed, but the owner still reports
being ignored by castle defenders. The downstream target inclusion is not a
verified solution to initial enemy acquisition. Campaign-transition controller
loss was not reproduced by this custom siege.


## Revision33 acquisition correction

The live castle observation put the independent player about40m from a ranged
formation while its parent formation was about215m away. The enemy's visible
unit list already included the parent. Initial native searches nevertheless
used only the parent's primary group bounds. Earlier203 target resolution did
not cover that initial discovery step.

Acquisition207 includes the existing registered one-member group in the shared
ranged search/cone/radial/scoring paths and melee neighbourhood search. This is
not restricted to castle archers, a unit name, a weapon or a map type. It works
with the validated physical actor, including the horse for mounted possession.
No fake unit card, ownership transfer, morale adjustment or enemy order is added.

The exact owned identity, current independent group, native registration and
enemy-visible list must all agree. A nearer valid independent target may replace
only a local group register during the search. The real unit/roster/primary-group
pointers stay untouched. Native eligibility, fire permission, ammo, range, cone,
contact and downstream combat rules remain responsible for attacks. Invalid
independent candidates fall back to the formation. Ordinary targets take a short
passthrough path without the floating-point save/helper work.

Offline checks execute the original radial/cone functions, complete ranged
search, scorer and melee candidate-distance branch on isolated owned engines.
The melee fixture stops before target-record allocation and strategic orders;
it is not a complete simulated battle. Both foot and mounted fixtures reproduce
the original omission and exercise the correction, including rejection cases.
Stock and the reviewed Bugfix image are checked separately. Installed revision32
is unchanged until a separately approved swap. Actual enemy shots, pursuit and
melee engagement remain an in-game acceptance requirement.


## Revision34 shared targeting and hostile-distance correction

Revision33 failed the owner trial: enemies still ignored the separated soldier,
including inside the castle. Three bounded read-only captures in the same
verified process showed a live registered one-member group and the parent unit
in enemy visibility/neighbour lists, but no chosen target. This establishes
candidate visibility, not native attack selection or actual ammunition counts.

Native177e30 directly calls48d6a0 and1752d0 for its special firing controller.
It bypasses16aa00, so revision33's thread-local normal-search scope was absent.
Acquisition208 scopes shared cone, radial and scoring entries independently.
The cone entry validates its native side and visible-enemy list; scorer/radial
entries validate their observing unit. Finally blocks restore nested state and
callback counts. Exact hook bytes and original functions were compared on both
supported engines. No castle, weapon, troop-name or map filter is present.

Native47efc0 caches distances between primary formation rectangles. The new
hook keeps that original call/cache and returns the smaller original1aaed0
rectangle distance for a hostile observer querying the currently leased,
registered independent player group. It does not overwrite the parent cache.
Native callers pass observer first; queries BY the player's parent formation
remain unchanged to preserve its own movement/firing/ability distances.
Self, ordinary, allied, invisible, stale, dead and released targets fall through.
No fake unit/card, new morale, forced enemy order, contact threshold or native
fire permission is added. Native battle AI still decides whether to engage.

Focused fixtures execute original native routines on isolated stock and reviewed
Bugfix images for both foot and mounted actors. They reproduce direct-call
candidate/scorer omission and primary-only cached distance; cover14 shared-entry
and18 distance cases per actor, plus the existing207 suite. They check nested
scope/stack restoration, candidate guards, original cache values, both parameter
orders and unchanged parent-observer results. This is not an end-to-end battle.
Actual ranged shots, melee pursuit/engagement and cavalry behavior remain
unverified until the owner tests the installed candidate.

Evidence: D:/Shogun2Deck/builds/Test181/acquisition208-review-1 and
D:/Shogun2Deck/builds/Test181/acquisition208-implementation-1.
