# Known problems and troubleshooting

The mod is still being worked on. **Visual and gameplay bugs, and some crashes, are expected.**

**Naval battles do not support possession.** Naval support is planned for a later update. **Multiplayer is not implemented:** Versus and campaign drop-in are under consideration.

## Crashes and slowdowns

Crashes can still happen during battles, including with blood turned off. The accepted v0.28 playtest also ended in a recorded crash; its cause remains unresolved. v0.24 corrects one captured fault where a detached soldier's native advance task expected a squad. Other captured failures remain unresolved, and this does not establish a shared cause for every crash.

v0.22 was reported smooth with Large units, one massive army per side and maximum graphics in DX11. Ultra units with substantial army presence can still become very slow, and remain an open performance limitation. The audio guard covers one reproduced fault; it does not resolve all crashes. It can happen without a QTE showing, and sometimes the QTE stays smooth while the battle slows down. Some fights recover on their own. Leaving and re-entering possession isn't a confirmed fix.

If you hit either problem, include the unit, map and what was happening nearby. The [technical notes](https://github.com/logsam58/shogun2-first-person/blob/v0.28/docs/releases/v0.24-technical.md#remaining-limits) contain the crash details and investigation limits.

## Visual problems

v0.24 gameplay was accepted, with first-person weapon/arm poses and clipping left for later work. Melee combat presentation was reported good; ordinary walking and running keep their original movement states.

- **Mounted bows:** the weapon can look wrong while aiming. This is still an open issue.
- **Arms and weapons clipping:** first-person visibility has improved, but some weapons and animations still let you see through parts of the model. Looking sharply up/down can expose it.
- **Missing HUD or odd timing:** include resolution, fullscreen/windowed mode, Windows scaling, monitor refresh rate, battle speed and whether the whole game slowed down. Not every display/overlay combination has been tested.

## Combat and unit orders

- **Bow draw and sound:** bow strength depends on holding LMB, but the native draw animation and sound begin with RMB. Separating sound timing from that animation is deferred.

- **Personal charging has improved:** the new request/contact behavior was accepted by the owner. Use forward sprint plus LMB/RT in melee mode without aiming; intermittent failures can still occur.
- **Enemies targeting a separated player:** enemies still ignored the lone possessed soldier in the latest revision34 test. The included targeting corrections have not resolved autonomous shooting, pursuit or melee acquisition.

- **Interrupted melee prompts:** another hit, death or a change in the fight can cut a prompt short. INTERRUPTED is there to show that it ended early. Crowded fights still need more testing.
- **Shots or volleys not releasing:** include the exact unit, weapon and ammo. Bow, gun and other weapon support is broader than the combinations tested so far. Say whether it was your soldier, the rest of the unit, or both.
- **Unexpected return to the RTS view:** some battle situations still need investigation. Say whether your soldier was alive, wavering, climbing, falling or landing when it happened.
- **Formations:** orders use Shogun 2's own formations and pathfinding. Terrain, casualties and morale can change how a unit responds. Report soldiers staying stuck or failing to reform.
- **Castle wall climbing is intermittent:** it sometimes works and sometimes does not. This remains an open bug in the current build. Include the castle and wall location when reporting it.
- **Friendly-fire voice:** it covers direct hits from your own tracked shots. Splash damage, secondary effects and every weapon combination haven't been tested.
- **Aiming:** the descending dashes help with drop; they aren't a rangefinder. Shot spread follows unit accuracy, but isn't an exact copy of RTS shot scattering.

## Compatibility

The mod is meant for single-player land battles and sieges. Weapons from Shogun 2, Rise of the Samurai and Fall of the Samurai are recognised, but not every unit and ammo combination has been tested.

Multiplayer is not implemented; Versus and campaign drop-in support are under consideration. Naval possession and ship commands do not work and are planned for later. Overhaul compatibility needs separate testing. The original and reviewed
Bugfix engine are supported; neither Bugfix nor Shogun2 Loader is required.
Compatible engine edits may pass the shared checks, while conflicting code or
hooks still refuse. Include the exact engine/startup error in a report. Artillery keeps the game's existing first-person mode.

## Workshop setup and updates

**F9 does nothing:** select a living friendly unit in a land battle, check the
mod is enabled, and restart the game after enabling or updating it. Include the
unit and what it was doing if entry keeps failing. The original supported
Windows Steam game build is required; see the [build guide](../BUILD_AND_TEST.md).

**HUD or local resources fail to prepare:** the mod reads the owned data.pack
and compatible native fonts from the installed language pack on first launch.
English advisor voice and its decoder are optional; the HUD remains English. Missing, changed or foreign cache files are preserved and initialization
may stop; do not delete unfamiliar files blindly.

**Other mods:** the Workshop bootstrap replaces the front-end root script
entry. One enabled UI root can be preserved when our bootstrap runs;
different competing roots can still conflict. Do not
combine Workshop with the older mod DInput loader. If coming from an old
setup.exe installation, close the game, uninstall that version and remove its
camera-helper Steam launch option before enabling Workshop.

**Turning it off:** Close the game and disable the mod in the launcher or
unsubscribe. Necessary local caches may remain; the runtime does not prune old
versions or rollback backups. No recorder or debugger is installed with it,
and routine gameplay/input/QTE disk logs are disabled.

## Steam Deck

Use the Windows game through Proton (tested 9.0-4). Menu directional highlighting
is unfinished; use the right trackpad. Large campaign panels shrink to fit and
can retain uncomfortable small text. Campaign-to-battle/deployment controller
failures remain under investigation even though some subsequent tests worked.
The latest melee/QTE crash remains undiagnosed. Controller support is community
tested, not an official Valve verification status.

## Reporting a problem

Please include:

- Mod version, game/expansion, battle type and map.
- Unit, weapon and ammo.
- What you did, what you expected and what actually happened.
- Other mods and graphics settings, especially for visual or timing problems.
- Army count and unit size for slowdowns, plus whether the same nearby fight is smooth in RTS mode.
- For a crash, any error message or exception/module information you have.

The optional [crash recorder](../tools/diagnostics/README.md) can collect more detail. Full dumps and input recordings can contain private data, so review them before sharing. The [contributing guide](../CONTRIBUTING.md) has more reporting detail.
