> Historical development/checkpoint notes. The combined implementation is now
> maintained in public `main`; use the root README and BUILD_STEAM_DECK.md for
> current play/build instructions. Earlier private-item state below is historical.

# Current status: device profiles accepted

The owner accepted the automatic profile on PC and Deck in revision20. It is
retained in accepted revision24/v0.34-deck.2. The candidate-stage notes below
are historical. Public Workshop promotion has not happened. No separate claim
is made for every Big Picture/controller configuration or a fresh PC replay of24.

# Automatic desktop and Steam Deck profile — private candidate

One Workshop pack chooses its controls and presentation at game startup. This is
being developed in the private Deck fork. The public Workshop item is unchanged.

| Where the game runs | Controls | Presentation |
| --- | --- | --- |
| Steam Deck identified by Steam | Existing Deck menu, campaign, battle and possession bindings | Accepted Deck UI sizing, prompts and View controls guide |
| Desktop, including a desktop using Proton or a controller | Existing keyboard and mouse controls | Normal PC HUD/QTE sizing; no Deck header, glyphs or controls guide |
| Device lookup unavailable | Desktop fallback | No controller takeover or Deck scaling hooks |

This is a hardware choice for the game process, not a switch whenever the mouse
moves. The profile is fixed until the game restarts. A generic controller,
1280×800 resolution, Big Picture mode, or Proton alone does not identify a Deck.

## Implementation

The bridge obtains SteamUtils010 from the already loaded game's steam_api.dll
using its existing Steam client and pipe. It calls Valve's
`IsSteamRunningOnSteamDeck` method. The exact x86 interface layout is checked
against Valve's pinned Proton SDK162 header, including its two private slots.
The newer SDK deprecates this method but the pinned version remains the interface
used here; do not mix a newer interface's slots into this adapter.

- [Valve API reference](https://partner.steamgames.com/doc/api/ISteamUtils#IsSteamRunningOnSteamDeck)
- [Pinned Valve declaration](https://github.com/ValveSoftware/Proton/blob/e91ca2be0df2cef4c230cbbc0b86604d73a0bbf6/lsteamclient/steamworks_sdk_162/isteamutils.h)

No Steam library is replaced or loaded from another directory, and the detector
does not initialize, shut down or reconfigure Steam Input. Only a positive Deck
result enables Deck workers, native UI scaling and layout provisioning.
The normal local-game resource preparation remains common to both profiles.

A 32-byte process-lifetime memory record contains the device choice, reason,
PID and process creation time. Adapter, QTE and external HUD read this same
record; the HUD does not start a Steam client. No diagnostic files or background
recorder are added. The record is released with its owning processes.

Desktop HUD sizing and QTE position use the regular v0.33 formulas. Deck uses
revision19's +15% mod HUD, normal +25% native map UI, and whole-window fitting.
Large campaign panels may still reduce to approximately original size to fit;
that accepted readability compromise is not fixed by device detection.

## Validation boundary

Offline checks cover actual device-query ABI against a deliberately fake Steam
interface, unavailable/failed API fallback, immutable process choice, mismatched
process identity rejection, cross-process HUD reading, desktop input bypass,
stock menu callbacks without Deck children, and both scale formulas.
The complete gameplay and Deck regressions must also pass before staging.
These fixtures do not constitute real Steam Deck or desktop gameplay acceptance.

Before merging into the public Workshop item, test the SAME private pack on a
normal PC and on the Deck. Check main menu, campaign, deployment, possession,
QTE, pause/return transitions and HUD visibility. Also verify desktop remains
normal in Big Picture and with a controller connected. Proton 9.0-4 remains the
existing Deck comparison baseline. Publication is not authorized by this pass.

## Completed private offline candidate

Revision `automatic-device-20` was packaged on 2026-11-01 for the existing private
item 3829785193. It is staged on the PC after explicit owner approval, but not uploaded or
hardware-accepted. The accepted
v0.34-deck.1 release remains unchanged.

- Fresh component checks: 100 adapter, 50 QTE, 8 HUD, plus local resource/font/render suites.
- Device query/profile and cross-process checks: 38; compiled menu cases: 6.
- Campaign wrapper: 8 cases; bounded reader: 8; context backend: 5.
- Workshop startup/resource checks: 20; maintained Python tests: 52.
- Twelve accepted camera/control source files match revision19 exactly.
- Exact accepted19-to-candidate-to-accepted19 rollback passed in isolated fixtures,
  including restoration with the source and candidate paths unavailable.

The initial full build stopped in a test-only fake Steam pointer expression after
its component checks passed. That fixture was corrected; the final build reused
only the exact verified component dependency/resource closure and reran the
remaining checks successfully. Earlier evidence was retained.

Pack SHA256: `7f7dcc83252b16c5cb8fcf1b9ebd77119facafe6ad73fb457d5df1c70628592b`.
Bridge SHA256: `a608c108f07c20aaa7b7d112f37b066c8d0bda26a18d62451d278c67f59dc625`.
Private evidence is under `D:/Shogun2Deck/builds/Test181/automatic20-review-1`;
the package is `private-package-21`, with prepared rollback in `private-update-21`.
