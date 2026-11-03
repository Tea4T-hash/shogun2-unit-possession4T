# Changelog

## v0.34 - 2026-11-02

Changes since public v0.33. Public main now contains the combined implementation;
this source push does not upload Steam or create a binary release.

### Steam Deck integration + possession improvements
One Unit Possession mod for PC and Steam Deck. The mod now detects your device and loads the appropriate controls and UI. Existing subscribers use this same Workshop item.

### Steam Deck
- Fixed the clipped Start Game footer in Steam Deck campaign setup by fitting the native campaign screen as well as its outer canvas. Owner accepted the correction.
- Separate, automatic control schemes for menus, campaign, Battle RTS and possession, labelled in Steam as “Unit Possession Layout.”
- Thumbstick camera controls, smooth campaign and battle zoom, faster campaign panning with L3, and a visible right-trackpad cursor in campaign menus.
- Y enters/leaves possession. Hold L3 to run; hold R3 to switch first/third person. RT selects in menus, campaign and battles.
- Hold View for an in-game controls guide, with labelled Deck buttons and pages. Compact hints replace the large permanent legend.
- A four-section R1 command wheel for Follow Me, Stop, Hold Here and Fire Orders, using local Shogun artwork and your unit’s clan mon.
- Larger Deck HUD, melee prompts and map UI. Large campaign windows fit to the screen, although some still use smaller text.
- Battle selection improvements: steadier clicks, L3/L5 add/remove selection, L4 range selection/queued orders, hold D-pad to assign groups, double RT to focus a unit, and L4 + R4 to select all.
- Battle A shows formations/ranges, X toggles run/walk, and LB/RB smoothly zoom in/out. Right-trackpad click is disabled.
- Campaign shortcuts for Arts, Finances, Diplomacy and Clan Management; held Y ends the turn. Campaign controls recover after returning from battle. The redundant news panel is removed on Deck.
- Moving the cursor to the screen edges no longer moves or rotates the map on Deck.


### Possession, camera and combat
- PC K and Deck View now share the same HUD cycle: hide RTS HUD, hide both HUDs, restore both. Melee timing prompts stay available.
- Reworked personal infantry/cavalry charging and native contact animations. PC: hold W + Shift + LMB. Deck: push forward with L3 + RT held. Use melee mode and do not hold aim/ready.
- Wall climbing can be requested by your soldier without first ordering the whole unit up the wall. Stalled approaches cancel instead of trapping you; the unsupported “Climb Down Wall” entry/prompt is removed.
- Smoother right-stick looking in first and third person, including aiming, plus steadier forward movement.
- Third-person framing moves the soldier away from the aiming marker and adjusts with camera distance and scrolling.
- Fixed reproduced infantry and cavalry movement-animation cases where the soldier or horse slid while appearing idle.
- Corrected possession unit-card proportions and reduced weapon/fire-order display flicker.
- Additional enemy-targeting work is included, but the latest test still left a lone possessed soldier ignored. This issue remains unresolved.


**Still known:** intermittent wall-climb/charge failures, small text in some Deck campaign panels, occasional controller failures during campaign-to-battle transitions, visual bugs, crashes and severe slowdown in very large battles. Naval possession and multiplayer are not supported.

[Build and validation](docs/releases/v0.34.md) · [Steam copy](docs/workshop/README.md)


## v0.33 - 2026-10-28

- Support the reviewed Bugfix engine and compatible engine edits through shared native checks; conflicting code or hooks still refuse safely.
- Keep the HUD visible for custom units without stock artwork.
- Read supported older PFH2/PFH3 packs and improve enabled UI-mod handling.
- Allow compatible localized fonts and make the English friendly-fire voice optional.
- Restore stock menu callbacks after an early startup failure and show a bounded startup error.
- Remove F12 as a mod-disable/possession-release key. F10 and Escape still return to RTS.

Packages the exact Test173 build; no new gameplay rebuild or Steam upload.
Test172 was accepted in game. Test173 has offline checks, with live acceptance
still pending. No installer downloads. [Release notes](docs/releases/v0.33.md).

## v0.32 - 2026-10-24

- Move the player release to Steam Workshop with automatic owned-game resource preparation.
- Start in third person, with P to switch views; remove the installer and startup chooser from the current player route.
- Disable routine gameplay/input/QTE/startup disk logs and remove live receipts on process exit.
- Share compatible resource caches with verified copy fallback and preserve unknown files.
- Retain the accepted gameplay, HUD, unit controls, abilities, cropped voice and Miles audio guard.

Uses the exact accepted Test166 Workshop pack. Charging, climbing, crashes,
clipping and Ultra crowd slowdowns remain open. [Release notes](docs/releases/v0.32.md).

## v0.28 - 2026-10-21

- Add third person with P, camera distance on the wheel, and middle-mouse orbit while moving.
- Pick a specific soldier under the mouse cursor at closest RTS zoom, including Generals, officers, flag bearers and musicians with their own equipment.
- Improve native wall approach/climbing and whole-unit mount/dismount with M.
- Keep third-person camera position through melee encounters and death; limit first-person combat look around the animated face.
- Keep possession through broken/shattered morale until death and carry K Hide UI across RTS/possession.
- Retain unit orders, formation firing, abilities, bow strength, charges and performance/audio guards.

Crashes remain under investigation. [Player notes](docs/releases/v0.28.md) - [Technical details](docs/releases/v0.28-technical.md).

## v0.27 - 2026-10-21

- Complete the charging checkpoint with the user-accepted Test149 gameplay.
- Start a personal native charge with W+Shift, retaining the game's target approach, paired attacks and defensive outcomes.
- Fix native charge alignment and stop ordinary melee prompts from taking over that contact.
- Carry K Hide UI between RTS and first person while keeping melee prompts visible.
- Preserve v0.26 unit abilities, bow strength, native audio and all existing unit controls.

[Player notes](docs/releases/v0.27.md) - [Technical details](docs/releases/v0.27-technical.md).

## v0.26 - 2026-10-20

- Add whole-unit T/G abilities, friendly-target Inspire and variable bow strength.
- Restore native bow audio and retain the melee aiming dot, dismount continuity and Hold Here corrections.

[Player notes](docs/releases/v0.26.md).

## v0.25 - 2026-10-20

- Join native infantry and cavalry charges while possessed.
- Let native charge contact and horse jumps take priority over the ordinary melee prompt.
- Keep the accepted HUD, unit orders, independent formation firing, native melee toggle and Rapid Advance behavior.

Charging is improved but still buggy; the player accepted it with that caveat. First-person weapon clipping, Ultra crowd slowdowns and other crashes remain open. [Player notes](docs/releases/v0.25.md) - [Technical details](docs/releases/v0.25-technical.md).

## v0.24 - 2026-10-20

- Match Rapid Advance sprinting to the unit's boosted pace while keeping walking available.
- Target enemy units through friendly troops more reliably.
- Toggle the whole unit's native melee mode with **1**, using the game's own mode indicator. **2** no longer switches weapons.
- Keep the accepted melee combat presentation and restore ordinary movement animations.
- Correct one captured crash in a detached soldier's native advance task.
- Preserve independent formation firing, unit orders, HUD and QTE behavior.

The player accepted this checkpoint after a verified in-battle test. First-person weapon/arm poses and clipping still need work. Other crashes and Ultra crowd slowdowns remain unresolved. [Player notes](docs/releases/v0.24.md) - [Technical details](docs/releases/v0.24-technical.md).

## v0.23 â€” 2026-10-19

- Let the rest of your unit form up and fire while your possessed soldier moves independently. Leaving possession restores normal squad control.
- Face the soldier's direction on entry and carry walking/running movement into possession.
- Follow the opening combat animation before allowing limited head-look during melee prompts.
- Stop repeated Follow Me voice acknowledgements as the unit moves and catches up.
- Retain the HUD, QTE, weapon behavior and crowd optimizations.

The player accepted Test130 gameplay and reported a crash in the same session. Its cause remains unresolved. [Player notes](docs/releases/v0.23.md) Â· [Technical notes](docs/releases/v0.23-technical.md).

## v0.22 â€” 2026-10-19

- Keep the latest crowd-performance improvements, cleaner HUD and friendly-fire callout.
- Skip more work for unrelated melee encounters while preserving full checks for the player's combat.
- Guard a reproduced zero-sample audio fault. Other crash causes remain under investigation.
- Preserve the player-confirmed smooth Large-unit, one-army-per-side, maximum-graphics DX11 configuration. Ultra units with heavy army presence can still cause severe slowdown.

See [release notes](docs/releases/v0.22.md) for controls and installation, and [technical notes](docs/releases/v0.22-technical.md) for implementation and evidence.

## v0.21 - 19 October 2026

- Improved first-person performance in large infantry and cavalry fights by reducing repeated checks for unrelated soldiers.
- Keeps the updated HUD, melee prompts, unit controls and cropped "SHAMEFUL DISPLAY!" friendly-fire callout.
- Packages the exact Test122 gameplay selected after a successful playtest. That session ended in a crash; crash safety remains unresolved.
- Updated installer with verified upgrades and interrupted-install rollback from v0.20 and Test122.

[Release notes](docs/releases/v0.21.md) Â· [Technical details](docs/releases/v0.21-technical.md).

## v0.20 - 18 October 2026

- Cleaner HUD, with commands down the right side and less clutter around the weapon information.
- Smaller top-centre melee meter and larger mouse-button prompts.
- Tighter QTE click handling and fewer repeated checks in crowded melee. The occasional severe battle slowdown is still being investigated.
- Cropped original SHAMEFUL DISPLAY callout when you shoot an ally.
- Updated installer, with the existing possession, weapons, unit orders, cavalry and siege features.

Crashes, mounted-bow visuals and some clipping remain open problems. [Player release notes](docs/releases/v0.20.md) Â· [Technical notes and test details](docs/releases/v0.20-technical.md).

## v0.19 - 18 October 2026

Promotes the exact accepted Test109 gameplay with the existing possession, combat, command and siege systems.

- Less cluttered native-style HUD: unchanged bottom-left soldier card, vertical right-edge Q/E/Z/C/R legend and relevant-only bottom-centre context.
- Compact top-centre QTE with a larger mouse-button cue, retained INTERRUPTED feedback and unchanged accepted timing/input authority.
- Original cropped SHAMEFUL DISPLAY callout for verified direct personal projectile hits on allies; the routing lead-in is excluded. Playback runs asynchronously in the HUD process.
- Exact gameplay binaries/source retained. Fresh release loader and full installer fixtures cover Test109 and published-v0.18 upgrades, seven interrupted rollbacks each and repeat-install retention.

The player accepted Test109 without sending READY; automatic recording verifies the build and capture retrospectively. That session crashed with blood disabled through a distinct invalid DirectSound virtual call. Originating corruption, possible mod involvement and the relationship to earlier effects/Miles crashes remain unresolved. No crash fix is included. [Release notes](docs/releases/v0.19.md) and [validation](docs/releases/v0.19-validation.json).

## v0.18 - 18 October 2026

Promotes the accepted Test107 gameplay with the full v0.17 possession and unit-control feature set.

- Existing HUD, QTE and death artwork now has native DirectX9/11 presentation, retaining the Windows fallback and 1080p layouts. HUD visibility was accepted in1920Ã—1080 fullscreen DirectX11 play.
- Current proven simulation-clock anchors keep the QTE marker moving between combat callbacks. Pause, native authority, response deadlines and submitted-image click judgement are preserved.
- A shared owned-infantry animation reset correction and consistent current aim identity address reproduced personal firing lifecycle failures while retaining independent unit firing.
- Exact accepted gameplay binaries/source are retained. Fresh release loader/installer checks include exact Test107 and published-v0.17 upgrades, interruption rollback and repeat-install backup retention.

The user reported Test107 "worked great" after verified loaded hashes and actual recording. Full roster/device/death-overlay coverage, hardware latency and the effects crash remain open. [Release notes](docs/releases/v0.18.md) and [validation](docs/releases/v0.18-validation.json).

## v0.17 - 17 October 2026

Promotes the accepted Test104 gameplay into a versioned release package.

- Native Q/E/Z/C/R unit commands, contextual walk/run/charge orders and stock formation dragging.
- Follow Me catches up at a run, then matches walking/running pace without stationary mouse-look restarting orders; Z captures one position/facing for walk or double-tap run.
- Improved owned first-person near clipping and eye-relative infantry weapon/arm look, with preserved native pose caches and exact restoration.
- Current native Q targeting excludes the possessed formation from the aim ray. Manual movement stays locked through knockdown and the complete get-up animation.
- FPS roster overrides stay inside camera work, protecting ordinary native unit consumers; exact current holder lookup and a guarded UI followed-actor write reduce reproduced possession-drop paths.
- Horizontal and vertical head-look continue through QTE and native kill-animation handoff.
- The exact accepted adapter, HUD and QTE binaries are retained. Release metadata is 0.17; loader/installer integrity and exact Test104 upgrade/rollback checks are freshly run.

The user accepted Test104 after verified loaded hashes and automatic crash/input/memory/hardware capture. This does not certify all maps/units or fix the unresolved effects crash and mounted-bow presentation. [Release notes](docs/releases/v0.17.md) and [validation](docs/releases/v0.17-validation.json). The GitHub release includes the installer, Windows package, source archive and checksums.

## v0.16 - unpublished development series, 17 October 2026

- Q remaps native contextual move/attack: single-tap walk, double-tap run/charge, and hold/drag native formation placement with first-person aim active.
- E stops; Z captures Hold Here with guard, single-tap walk or double-tap run; C follows beside/around the player and runs to catch up before matching the player's native pace.
- R cycles Hold Fire, Fire at Will and Fire on Command after an actual successful personal shot. Independent native AI/firing remains in charge outside that policy.
- Stationary mouse-look no longer restarts follow goals. Near clipping and infantry arm/weapon look use the owned first-person camera/current eye, with exact restoration and preserved native head/shoulder caches.
- Native unit pathfinding, formation geometry, run eligibility, fatigue, equipment, accuracy, QTE, RTS return and siege foundations are retained.

Historical development builds Test102 through Test104; superseded by v0.17. Test104 received user playtest acceptance before promotion. [Release notes](docs/releases/v0.16.md).

## v0.15 - 17 October 2026

First packaged GitHub release of Shogun 2 First-Person Possession. This release brings the complete native possession experience together:

- First-person control of a selected living soldier, native movement/running/fatigue and animated camera/death behavior.
- Personal bow, firearm and recognized projectile handling, native ammunition/reload, melee/ranged equipment selection and supported crouched firing.
- Native effective-accuracy muzzle cone, dynamic firearm spread reticles and descending non-firearm aiming dashes.
- Native paired melee QTEs with readable left/right prompts, at least three active seconds plus one full cycle, timestamped input and interruption feedback.
- Cavalry possession with separate steering/look, independent unit AI/firing and halt/advance/follow orders.
- Native-speed siege wall ascent/descent/hold, crest animation and possession continuity into landing.
- Saved RTS camera restoration and a contextual HUD using the game's visual language.
- Integrated loader/setup with exact predecessor recognition, transactional interruption rollback, reinstall handling and guarded cache cleanup.
- Maintained C++ source, build/regression tooling, diagnostic source, component manifests and player checksums.

The release retains the verified gameplay component build; release-specific code changes are product metadata and exact predecessor installer support. See [release notes](docs/releases/v0.15.md), [validation](docs/releases/v0.15-validation.json) and [known issues](docs/KNOWN_ISSUES.md). Extended-play crashes and mounted-bow presentation remain unresolved.
