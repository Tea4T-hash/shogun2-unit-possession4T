# Steam Deck control research — 31 October 2026

This is a private development design and evidence record. The navigation-schemes-5 candidate is not hardware-accepted. The regular Windows mod is unchanged.

## Findings and decisions

Valve recommends complete controller access, simultaneous mouse/trackpad and stick input, and automatic text entry. Its native action system separates named game actions from physical controls and supports automatic action-set changes. We therefore use four automatically selected sets: Menus, Campaign, Battle (RTS), and Possession. Steam owns its controller editor, including page navigation; the mod cannot promise custom L1/R1 editor tabs.

The former layout exposed the full diagnostic input inventory in every context. Many entries genuinely had no implemented action, so they were labelled “Not assigned”. This was unfinished presentation, not a request for players to invent their own bindings. The new manifest declares only implemented actions: 85 localized set/action names, rather than 124. The runtime requires only the channels declared for that context. Physical-origin validation still applies. Steam may still show unused physical buttons as None; we do not attach meaningless actions just to fill them.

Allyn’s Shogun T.W Setup is the basis for RTS feel: left stick pans, L3 accelerates the camera, bumpers rotate, right pad points, LT selects, RT orders and the left pad scrolls/zooms. The earlier preserved VDF review records these bindings. A fresh public Steam API metadata query confirms item 582338806 and its title; the current API did not provide a downloadable file URL, so this investigation does not claim a fresh raw-VDF comparison. No third-party configuration is shipped or installed.

World’s Edge’s official Age of Empires II console guidance uses contextual selection, modifiers, quick selection and a control-group wheel. Those are useful design precedents: keep common actions immediately available and put advanced commands in contextual UI. Our current build retains native Shogun drag selection and formation orders. A real group wheel and gamepad text entry are further work, not buttons or prompts pretending those features already exist.

## Implemented default schemes

| Context | Camera / movement | Selection / action | Other |
| --- | --- | --- | --- |
| Menus | Left stick or D-pad navigates; right pad moves cursor; left pad scrolls | A selects; LT/right-pad click selects or drags; B backs out | Menu sends focused native Escape for intro/back after the frontend has loaded |
| Campaign | Left stick pans; hold L3 for fast pan; right stick turns/zooms; bumpers turn; left pad scrolls/zooms | Right pad cursor; A/LT/right-pad click selects; RT contextual native order | X/Y previous/next selection; D-pad UI navigation; B closes panel; Menu pauses |
| Battle RTS | Left stick pans; hold L3 for fast pan; right stick turns/tilts; bumpers rotate; left pad zooms | Right pad cursor; LT/right-pad click drag selects; A tap selects; RT move/attack/formation drag | Hold L4 or R5 for additive selection; X stops; B cancels; L5 hides HUD; Menu pauses; tap R3 possesses |
| Possession | Left stick moves; right stick/right pad looks; right-pad touch enables gyro; L3 toggles sprint; R4 holds sprint | LT aims/defends/QTE defend; RT fires/attacks/QTE attack; hold RT to draw bow | A interacts/climbs; B cancels active unit placement; X melee toggle; Y selects ability; LB activates; RB holds orbit; left pad changes third-person distance |

Possession D-pad: Up advance/attack (hold for formation placement), Down stop, Left hold here, Right follow. L4 mounts/dismounts, R5 cycles fire policy, L5 hides HUD and Menu pauses. R3 double-tap returns to RTS; holding R3 changes first/third-person view. Gesture windows remain 250 ms double-tap and 350 ms hold. B does not dismiss a combat QTE or eject possession.

These names describe the implemented defaults. This is not yet complete controller access to every Shogun command. Groups, deployment/naval support, text entry, suspend/resume and prompt coverage need further work. Arbitrary physical remapping is also constrained by the existing required-origin safety checks.

## Regression investigation

The native engine’s CameraTarget descriptor explicitly returns X, Y, Z, bearing and usually theatre; Y is height/zoom. The old campaign code read only its first two values as horizontal coordinates. It therefore passed zoom as a map coordinate to SetCameraTargetInstant. The correction uses X and Z, preserves height, and uses the native bearing (radians). Verified owned function bodies: CameraTarget 0x1135690, CameraPosition 0x1135620, SetCameraTargetInstant 0x1149c20, SetCameraHeading 0x11499f0. Hash-indexed exports and assembly are preserved privately under navigation-review-1. Tests now vary height and map Z independently.

The campaign stick was already excluded from menu-direction sampling. An initial explanation suggesting duplicate stick handling was incorrect and was corrected to the owner. We did not change that existing exclusion.

For battle RTS, the earlier native camera algorithm and sensitivity are retained. RTS frames now use their original independent mailbox rather than depending on the possession intent queue. Optional glyph decoding happens after input delivery is posted, at most one image per poll, with bounded retries and cache reuse. These are verified structural corrections, not measured proof of the actual Deck slowdown’s sole cause.

Prompt layout now sizes the parent before positioning children, sizes each child before positioning it, and reapplies layout after text or adoption reflow. Text is changed only when needed. Fixtures exercise parent resize, text reflow, screen resize and invalid native UI values. Actual appearance on the Deck remains to be judged by the owner.

## Primary references

- [Valve: Steam Deck recommendations](https://partner.steamgames.com/doc/steamhardware/recommendations)
- [Valve: Steam Input concepts and action sets](https://partner.steamgames.com/doc/features/steam_controller/concepts)
- [Valve: action manifest, localization and examples](https://partner.steamgames.com/doc/features/steam_controller/iga_file)
- [Valve: ISteamInput API and action-set sampling examples](https://partner.steamgames.com/doc/api/ISteamInput)
- [Allyn’s original Shogun T.W Setup](https://steamcommunity.com/sharedfiles/filedetails/?id=582338806)
- [World’s Edge: basic console controls](https://www.ageofempires.com/learn-to-play/controlling-your-empire-gathering-resources-xbox/)
- [World’s Edge: advanced console controls](https://www.ageofempires.com/learn-to-play/match-goals-advanced-controls-xbox/)

Research was read-only. No external layout, controller driver or community code was executed. The backend continues to use Valve Steam Input through Proton rather than a replacement HID implementation.

## Current offline evidence

The clean navigation-full-2 build passed all 100 adapter, 50 QTE and 8 HUD fixture executables, plus native resource/font/compositor checks. Targeted checks passed 29 possession/R3 cases, 7 independent-RTS/presentation/backlog cases, 7 sparse-action/physical-origin cases, 58 float-Lua UI cases, 27 menu worker cases, 5 compiled frontend cases, 6 actual numeric Lua export cases and 16 guarded Workshop checks. Campaign handoff adds 6 compiled-entry, 8 native-reader and 5 context-backend cases. The 13 maintained contextual/controller files match the completed build snapshot. These are offline results, not evidence of actual Deck frame rate or layout appearance.

The first clean build stopped at DX9 device creation with 88760868. Its isolated compositor retry passed, and the second complete clean build passed that fixture and all remaining checks. No renderer or game setting was changed to bypass the failure. The former campaign source fails the corrected coordinate test; the current source passes. A first negative-fixture invocation had an incorrect backup path and was corrected before recording the real negative result.
