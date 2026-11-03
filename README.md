# Shogun 2 Unit Possession

**NOW WITH STEAM DECK INTEGRATION**

One build detects PC or Steam Deck automatically. PC keeps normal keyboard/mouse controls and UI sizing. Deck gets contextual campaign, battle and possession controls, larger UI, a command wheel and an in-game controls guide.

Take control of a soldier in **Total War: SHOGUN 2**, fight in first or third person, and command your unit while the battle continues around you. Select a friendly unit and press **F9** to enter possession. Press **F10** or **Escape** to return to your RTS camera.

**For single-player land battles and sieges.** Naval possession and multiplayer are not implemented. Visual bugs, gameplay bugs and occasional crashes remain.

[Download](#download) · [Requirements](#requirements) · [Installation](#installation) · [Controls](#controls) · [Compatibility](#compatibility) · [Known limitations](#known-limitations) · [Troubleshooting](#troubleshooting)

## Features

- **First- and third-person play:** start in third person, switch with **P**, adjust camera distance, or orbit your soldier.
- **Soldier selection:** at the closest RTS zoom, point at a soldier in your selected unit and press **F9**. Generals, officers, flag bearers and musicians use their own equipment. From further away, the mod picks a soldier for you.
- **Ranged combat:** aim and fire bows, guns and thrown weapons. Hold and release bow shots to vary their strength. Unit accuracy affects shot spread; sights help you allow for projectile drop.
- **Melee combat:** time the displayed mouse-button prompts during the game's combat animations. Personal charges use forward sprint plus the primary attack input, with native charge contact and paired outcomes.
- **Unit commands:** issue move and attack orders, drag formations, stop, hold position, follow, and choose fire orders. The game's formations, pathfinding, morale and fatigue still apply.
- **Abilities and ammunition:** select the options available to your unit, including targeted friendly abilities such as Inspire, subject to the game's restrictions and cooldowns.
- **Cavalry and sieges:** ride cavalry, order supported units to mount or dismount, and attempt castle-wall climbing. Climbing is intermittent.
- **Possession HUD:** see unit condition, weapon, ammunition, abilities and orders. **K** cycles the RTS and possession HUD visibility while keeping melee timing prompts visible. Direct hits on allies from your tracked shots can trigger the “Shameful display!” voice.

## Download

Use the existing [Steam Workshop item](https://steamcommunity.com/sharedfiles/filedetails/?id=3825625050). The combined build uses that same item; a separate Deck subscription is unnecessary.

**Maintained source: v0.34, PC and Steam Deck.** Public `main` contains the combined implementation through revision34. The latest tagged GitHub download remains [v0.33](https://github.com/logsam58/shogun2-unit-possession/releases/tag/v0.33), which predates Deck integration. No new binary release or Steam upload is performed by this source update. [Current source/build identity](docs/current-release.json) records the distinction.

No installer is provided. Fonts, artwork and audio are prepared from your own installed game.

## Requirements

For Steam Deck, use the Windows game through Proton as described below. Desktop resolution alone does not enable the Deck profile.

- An owned installation of the **original Windows Steam version of Total War: SHOGUN 2**, using the supported game build.
- The installed game's `data.pack` and compatible native fonts from `local_en.pack` or a supported `local_*.pack`. HUD text is English; not every localization has been tested.
- English advisor audio and the Windows media components are optional. If unavailable, the mod omits the friendly-fire voice.

No compiler or Python installation is needed to play. The mod prepares its resources from your installed game on first launch and caches them for reuse; those resources are not bundled in the download.

The original engine and the reviewed Shogun 2 Bugfix DLL are supported. Neither the Bugfix mod nor Shogun2 Loader is required. Other modified engines are checked against the functions and layouts this mod needs; a conflicting change can still be refused. See [engine compatibility](docs/public-release/WORKSHOP172.md) for the technical limits.


## Installation

1. Close the game. If you used an older setup.exe release, follow the upgrade instructions below first.
2. [Subscribe to Shogun 2 Unit Possession on Steam Workshop](https://steamcommunity.com/sharedfiles/filedetails/?id=3825625050).
3. Enable **Shogun 2 Unit Possession** in the launcher and start the game through Steam.
4. In a single-player land battle or siege, select a living friendly unit and press **F9**.
5. Possession starts in **third person**. Press **P** for first person; **F10** or **Escape** returns to RTS control.

The Workshop version needs no setup program, external script, camera chooser or Steam launch-option edit. **Restart the game after enabling, disabling or updating the mod.**

### Upgrading from an older setup.exe release

Close the game, uninstall the older version, and remove its old camera-helper Steam launch option before enabling Workshop. Do not run the older DInput mod loader alongside the Workshop version.

### Disabling or uninstalling

To stop loading the mod, close the game and disable it in the launcher or unsubscribe from the Workshop item.

Local resource caches and rollback backups may remain. Do not delete unfamiliar files blindly.

## Controls

These controls apply during possession unless stated otherwise. **LMB** means left mouse button; **RMB** means right mouse button.

### Your soldier and camera

| Key | Action |
| --- | --- |
| **F9** | Possess a soldier in the selected unit; at closest RTS zoom, point the mouse cursor at who you want |
| **F10 / Escape** | Return to your previous RTS camera |
| **WASD** | Move |
| **Shift** | Run; hold **W + Shift + LMB** toward an eligible enemy to request a charge. Use melee mode; release RMB. |
| **Mouse** | Look around |
| **P** | Switch first/third person |
| **Mouse wheel** in third person | Move the camera closer or further away |
| **Hold middle mouse** in third person | Orbit your soldier while keeping your movement direction |
| **M** | Order the unit to mount/dismount, where supported |
| **Hold RMB** | Ready your ranged weapon |
| **LMB** | Fire guns or thrown weapons; hold and release for bow draw strength |
| **1** | Toggle the whole unit's native melee/ranged mode, where supported |
| **T / G** | Select an ability or special ammunition / use or end it; aim at friendlies for targeted abilities |
| **Hold Ctrl** | Crouch to fire, where supported |
| **Shown LMB / RMB prompt** | Click the indicated button as the marker reaches the highlighted zone |
| **F** near a wall | Request an independent climb; press again to cancel a pending approach |
| **W / S** while climbing | Climb up / down; release both to hold position |
| **K** | During possession: hide RTS HUD → hide both HUDs → restore both. In RTS: ordinary HUD toggle. Melee prompts remain visible. |

### Your unit

| Key | Order |
| --- | --- |
| **Q — Move / Attack** | Point at the ground to move, or an enemy or supported breakable target to attack. Tap to walk; double-tap to run or charge. Hold Q and drag left to right to set the formation, then release. |
| **E — Stop** | Stop the unit. |
| **Z — Hold Here** | Form up at your current position, facing the direction you're looking when you press Z. Tap to walk there; double-tap to run. |
| **C — Follow Me** | Form up beside you. Run to catch up when needed, then match your pace. When you stop, your men finish forming up around you. |
| **R — Fire orders** | Cycle **Hold Fire → Fire at Will → Fire on Command**. Fire on Command tells the unit to fire when you fire. |

Unit orders are unavailable while you're locked in a melee animation, climbing or landing.

## Steam Deck controls

Use the **Windows** Steam game through Proton; **Proton 9.0-4** is the tested version. The native Linux build is not supported by these Windows hooks. Use the right trackpad in menus; RT selects and B backs out. Directional menu highlighting is unfinished.

Steam controller settings label the action sets as **Unit Possession Layout**. Hold **View** for the in-game guide; L1/R1 change pages and B closes it. Tap View in battle RTS to toggle the HUD; in possession it shares K’s three-step cycle.

Left stick pans the map or moves your soldier. Right stick turns/zooms in campaign, turns/tilts in battle and looks around during possession. Hold R3 while possessed to switch first/third person. Cursor position does not drive map movement on Deck. Right-trackpad click is unassigned.

To charge, hold **forward + L3 + RT**, with your unit in melee mode and LT released. Sprinting alone does not request a charge. Hold R1 for the four-section command wheel and release to issue the selected order.

### Campaign

| Input | Action |
|---|---|
| R2 / L2 | Select or interact / move or attack |
| R1 / L1 | Next / previous selection; in a panel, hold L1 and swipe right pad to scroll |
| A / B | Toggle movement speed (Space) / cancel or back |
| X | Zoom home |
| Y held | End turn, once after600ms |
| D-pad up / down / left / right | Mastery of Arts / Finances / Diplomacy / Clan Management |
| L3 held | Faster map pan; Ctrl add/remove selection |
| L5 held / L4 held | Ctrl add/remove selection / Shift range selection |
| R4 held | Quick-save, once after600ms; native CanSave required |
| R5 | Toggle character movement speed |
| View / Menu | Hold: in-game controls / game menu |

### Battle RTS, including deployment

| Input | Action |
|---|---|
| R2 | Select; hold and drag for box selection; double-tap a unit to focus it |
| L2 | Move or attack; hold and drag formation; double-tap for run or charge |
| R1 / L1 | Zoom out / zoom in |
| A / B / X / Y | Hold: formations and range / cancel or deselect / toggle run-walk / possess |
| D-pad up / down / left / right | Tap: select group1 /2 /3 /4; hold: assign current selection |
| L4 held | Select card range or queue orders |
| L3 or L5 held + R2 | Add/remove one unit at the cursor; the point is held still until R2 releases |
| L5 + D-pad | Assign selected units to corresponding native group |
| R4 / R5 | Toggle melee mode / group or ungroup |
| L4 + R4 | Select all units |
| L5 + R5 | Lock or unlock group formation |
| View / Menu | Tap: hide/show HUD; hold: controls / game menu |

### Possession

| Input | Action |
|---|---|
| R2 | Fire; hold/release bow; primary melee or QTE response |
| L2 | Ready ranged weapon; secondary melee or QTE response |
| R1 held | Command wheel: right stick or D-pad chooses; release commits |
| Wheel up / down / left / right | Follow Me / Stop / Hold Here / Cycle Fire Orders |
| L1 | Unit move/attack; hold for formation; double-tap for run/charge |
| A / B | Climb/interact / mount or dismount unit |
| X / Y | Native unit melee/ranged toggle / return to RTS |
| D-pad up / down | Cycle available ability/ammunition / use or end selected ability |
| D-pad left / right | Third-person distance closer / farther |
| L3 held / L5 held | Run / crouch |
| L4 held | Orbit the third-person camera |
| R4 / R5 | Stop unit / Follow Me |
| View / Menu | Tap: cycle HUD; hold: controls / game menu while remaining possessed |


Large campaign panels currently reduce their scale to fit. Fully enlarged, reflowed layouts remain unfinished. Occasional campaign-to-battle controller failures remain under investigation.

## Compatibility

| Game mode or mod | Status |
| --- | --- |
| Single-player land battles and sieges | Intended use; bugs and crashes remain |
| Shogun 2, Rise of the Samurai and Fall of the Samurai weapons | Recognised, but not every unit, weapon or ammunition combination has been tested |
| Artillery | Uses the game's existing first-person mode |
| Naval battles | Possession and ship commands are not available |
| Multiplayer, Versus and campaign drop-in | Not implemented |
| Unit and overhaul mods | Missing stock unit artwork no longer hides the entire HUD; individual combinations still need testing |
| Shogun 2 Bugfix engine / Shogun2 Loader | Reviewed compatibility checks and an owner-accepted Test172 combination; neither mod is required |
| Mods replacing the front-end root script entry | Can preserve one enabled root when our bootstrap runs; different competing roots can still conflict |
| Older DInput installation of this mod | Do not combine with Workshop |

## Known limitations

- **Crashes remain unresolved**, including crashes with blood turned off.
- **Large battles can slow down severely**, especially with Ultra unit size, several armies or dense fighting. Lower graphics settings do not necessarily prevent this.
- **Charging was improved and accepted in owner testing; intermittent failures remain possible. Castle-wall climbing is better and independent of unit orders, but remains intermittent.**
- **First-person arms and weapons can clip.** Mounted bows also have presentation problems.
- **Bow draw sound follows the native animation**, which begins with RMB, rather than the variable LMB draw input.
- **Enemies can still ignore your soldier away from the formation.** Revision34’s targeting changes failed the latest owner test; this is unresolved.
- Not every unit, ability, castle layout or display configuration has been tested.

See [known issues and reporting details](docs/KNOWN_ISSUES.md) for additional combat, camera and targeting limitations.

## Troubleshooting

| Problem | What to check |
| --- | --- |
| **F9 does nothing** | Select a living friendly unit in a single-player land battle or siege. Check that the mod is enabled, restart after enabling/updating, and confirm you have the supported Windows Steam game build. |
| **HUD or resources fail to load** | Check `data.pack` and the installed language pack. Compatible native fonts are required; English audio is optional. Include the exact startup warning. Changed or unfamiliar cache files are preserved; do not delete them blindly. |
| **Problems after upgrading** | Check that the older setup.exe version is uninstalled and its camera-helper launch option removed. Do not run both loaders. |
| **Shots or volleys do not fire** | Note the exact unit, weapon and ammunition, and whether the problem affects your soldier, the rest of the unit, or both. Not all combinations have been tested. |
| **Missing HUD or unusual timing** | Report resolution, fullscreen/windowed mode, Windows scaling, monitor refresh rate, battle speed and whether the whole game slowed down. |
| **Crashes or severe slowdown** | Report the map, unit size, army count and nearby fighting. Leaving and re-entering possession is not a confirmed fix. |

To [report a problem](https://steamcommunity.com/workshop/filedetails/discussion/3825625050/564798191048897978/), include the mod version, game/expansion, battle type, map, unit, weapon, ammunition, other mods, and what you did before the problem occurred. Add graphics settings for visual issues and any error message for crashes.

## Credits and license

This is an unofficial mod and is not affiliated with or endorsed by Creative Assembly or SEGA. Total War: SHOGUN 2 and its game material belong to their respective rights holders.

First-party code is available under the [MIT license](LICENSE). See [third-party notices](THIRD_PARTY_NOTICES.md) for other components. Community contributions are welcome; ongoing maintenance, updates and individual support are not promised.
