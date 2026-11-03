# Historical private Deck usability pass

This page records the earlier revision13 design. Current bindings, sizing,
workflow and validation limits are in [SteamDeck181.md](SteamDeck181.md) and
DECK_STATE.json. In particular native UI is now +25%, mod HUD +15%; old
permanent legends and the old CLOSED-for-offline rule below are superseded.

The owner approved offline work with Deck and PC closed. This candidate has not
been installed, uploaded or accepted on the Deck. The public Workshop item is
unchanged. Regular mod source has only the separately requested portrait fix.

- Hold L3 to sprint in possession; hold L4 to orbit. L5 remains crouch.
- Hold L3 or L5 for additive RTS selection, for cards and battlefield units.
  L3 no longer also selects the faster RTS camera rate. Campaign L3 is unchanged.
- RT selects in menus as in campaign and RTS. LT keeps the alternate click.
- Campaign, RTS and possession use the same top legend geometry, native fonts
  and Steam-supplied glyphs. Main-menu visual legend is absent; its invisible
  controller lifetime owner remains. The campaign texture is generated locally,
  in one fixed file per bridge cache, without recordings or glyph history.
- Mod HUD and QTE use 30% larger presentation. Context panels are separated;
  active QTE placement avoids the new legend. Timing, input clocks, judgement,
  possession camera and native camera rates are unchanged.
- Fire-order text may reuse an already-valid, same-PID/lease/actor/unit sample
  during an unstable seqlock read. Stable invalid/reset data clears it at once.
  Original 1000ms freshness remains; no stale state drives commands or AI.
- Unit portraits in both source trees fit their original logical canvas rather
  than stretching into 148 by 190. Native transparent padding, backing and rim
  retain alignment. No artwork or native unit data is replaced.

Campaign Finance/Diplomacy formerly opened panels directly inside a frame
callback. The candidate queues the original on-screen button route, separating
hover, press and release across UI frames, with a 500ms lifetime and cancellation
on focus/ownership loss. It also removes an incorrect requirement that the Lua
UI callback thread equal the Win32 window thread; current-process window and
UI-owner validation remain. This is an investigated correction, not proof of
the reported SteamOS exit's cause or actual Arts/Clan success. Rapid panel use
and all four directions still need an owner Deck test.

## Remaining base-game UI sizing work

This candidate enlarges the mod HUD, QTE and controller legends. It does not yet
scale every original game menu, unit-card row and tooltip. No original-game
UI-scale preference or global scale method was found. Exact native Resize and
text-detail analysis shows docking/resizable-child logic and alignment/colour
fields; it does not establish a complete font/layout scaler. A blanket geometry
resize would risk clipped text, off-screen controls and incorrect hit targets.
A verified per-layout or render/input scaling implementation remains pending.
Do not present this as completion of the requested all-game UI enlargement.

## Validation

The full usability component build runs 100 adapter, 50 QTE and eight HUD tests,
plus native font/resource rendering. A bridge-only include correction retains
those binaries only after exact comparison of all four dependency closures.
The Workshop bridge has its own fresh startup/lifetime/compatibility checks.
Additional float-Lua and native fixtures check binding isolation, queued clicks,
identity expiry and native portrait aspect ratios. Offline PNG previews use the
production renderer. These do not replace real Proton/Deck acceptance.
