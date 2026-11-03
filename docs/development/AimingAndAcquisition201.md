# Deck aiming and lone-soldier targeting investigation (201)

## Scope

The owner reports right-stick microstutter while holding LT on Deck, accepts charging, and says wall climbing is improved but still imperfect. They propose a temporary one-soldier unit when separated, with independent enemy acquisition, isolation morale and merging back. They also request removal of the "Climb Down Wall" option/prompt.

This pass changes fractional look handling and removes top-of-wall descent entry. It does not implement a temporary battle unit, new morale behavior or an enemy acquisition fix. No installation, Workshop upload, live SSH, recorder or debugger is part of the offline work.

## Aiming correction

Steam Input supplies floating-point analog values. The existing controller code produced floating-point mouse-equivalent motion, but the queue delivered whole counts and retained a fractional remainder for later frames; the camera/pitch consumers accepted integers. This quantized slow motion into stop/jump steps rather than losing the total accumulated motion. A quarter-count passed directly to the original actual camera fixture produced zero movement. That fixture demonstrates the integer camera boundary, not permanent loss of the earlier queue remainder. The accepted sensitivity at full ADS is 0.0025 * 0.45 radians/count.

The combined build now keeps both axes as fractions through a bounded, synchronized queue and the real yaw/pitch consumer. The keyboard/mouse sensitivity, stick curve, deadzone, camera limits and aim blend are unchanged. No interpolation or smoothing delay is added. The isolated build adapter supplies the float signatures; the preserved base adapter source remains usable independently.

The fixture checks 1,440 individual yaw/pitch advances over hip/partial/full ADS, cancellation, invalid inputs, queue bounds and 10,000 concurrent samples. It also runs the existing desktop bypass, PC/Deck HUD-cycle, command wheel and post-battle return checks. This proves the precision correction, not that every source of visible Deck jitter is gone.

## Climb-down removal

Discovery offers only the lower wall endpoint. A direct top-entry request refuses before creating a path or taking input control. The exact "Climb down wall" prompt is removed. Existing in-progress traversal, upward entry, crest and landing behavior remain intact. The numeric channel layout is unchanged. The native fixture verifies 96 top-entry refusal cases and the retained original upward request/admission/cancellation cases.

## Enemy acquisition: reproduced omission

Original native function RVA174ec0 reads the target unit's primary group at +b9c, then searches that group's member list (+18/+1c) for the closest soldier within the charge approach corridor. It does not enumerate all the unit's independent kind2 groups.

The existing proximity implementation moves the possessed soldier into a registered kind2 group when away, but preserves the original unit owner and master roster. A fixture using the actual original174ec0 reproduced an omission with the player 20m from their formation: a nearby attacker whose corridor contains the player finds no target. The same routine selects that exact player at the same position when an isolated fixture target points to the one-member group. No real unit ownership was changed in this experiment.

This establishes one charge-target selection gap. It does NOT prove the entire strategic acquisition or fire-at-will decision path, and it does not establish a safe constructor/registration/retirement lifecycle for a new battle unit. The fixture-only target copy is diagnostic data, not a proposed runtime implementation.

A separate card alone will not solve this. A real separate unit would require verified army registration, spatial queries, ownership and member transfer, ammunition/abilities, casualty accounting and merge/exit cleanup. These are especially important for generals, officers, mounted pairs and death during separation. No synthetic unit copy or temporary actor-owner substitution is shipped.

The original on-demand decoder extracted and byte-roundtripped the owned game's _kv_morale and _kv_rules tables against official authoring data. They include unit morale thresholds and general/flank effects; they do not establish a per-possessed-soldier isolation state. The proposed lone-soldier wavering behavior is not applied to the parent formation.

Next native boundary to verify: enemy army/unit target eligibility and location evaluation before a soldier target is selected, then ranged and melee assignment against a separated actor. A complete split/merge implementation remains unproven; enemy acquisition is explicitly unresolved in this candidate.

## Evidence and sources

Private evidence lives at D:/Shogun2Deck/builds/Test181/aim201-review-1: source-before.zip, before.log, camera201.log, acquisition201.log, enemy-native201.json, enemy-decoder201.json, enemy-morale201.json and wall-focused-source/verify_siege97-200.log. Stock native image SHA256: 022fae2e2da92b59b8b1b8ee52887003c624e3c8eca060b209f5d792a7f874f5. Executed target-selection instructions were checked against that preserved image.

- [Valve Steam Input API](https://partner.steamgames.com/doc/api/ISteamInput): analog action values are floats.
- [Valve input source modes](https://partner.steamgames.com/doc/features/steam_controller/input_source_modes): distinguishes position and mouse-like analog behavior.
- [CA battle scripting documentation](https://wiki.totalwar.com/w/Battle_Script_Documentation.html): army/unit controllers operate existing units; the listed reinforcement deployment is not a documented soldier split/merge API.
