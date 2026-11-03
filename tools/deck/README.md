# Maintained combined PC/Deck tools

The public main branch now uses this pipeline for both devices. Read
[BUILD_STEAM_DECK.md](../../BUILD_STEAM_DECK.md) and the current validation.
Workshop item3825625050 remains the player subscription. Private-item deployment
and diagnostics below are historical developer workflows, not installation steps.
Never commit local connection profiles or distribute complete build outputs.

# Steam Deck development

Current checkpoint: **v0.34-deck.1 / Test181 revision19**, accepted on Deck with
small oversized-panel UI scale retained as a temporary workaround.
See [current build instructions](../../BUILD_STEAM_DECK.md), [current validation](../../docs/public-release/V0_34_VALIDATION.json)
and [current controls and limits](../../docs/development/SteamDeck181.md).

The following older diagnostic instructions are historical, not the current
menu test or proof of full Deck gameplay support.

---

# Steam Deck development test 174

Current work: [Test177 menus-first correction](../../docs/development/SteamDeck177.md).
The owner reports that176 menu and battle controls did not work. Only the
earlier input diagnostic is accepted. No READY cue is needed for manual Deck
tests; the developer cannot inspect the Deck remotely.

The owner reports that revision3 fixed the stick clicks. The next private
increment connects native RTS controls; see [test175](../../docs/development/SteamDeck175.md).
The instructions below describe the earlier input-only diagnostic.

This is an **input diagnostic**, not playable controller support or a release.
It uses the existing mod's Workshop startup, then reads native Steam Input
actions inside the Windows game process under Proton. It does not start the
possession adapter, HUD or QTE. There is no Linux helper, keyboard/mouse preset,
Steam radial menu, recorder, debugger or network listener.

The owner's implementation brief requires a real SteamOS/Proton input test
before connecting the controller to gameplay. The scoped owner input test is accepted;
full gameplay validation remains pending.
See the [architecture and status](../../docs/development/SteamDeck174.md).

## Private Workshop test - diagnostic revision 3

Use the separate private item **3829785193**, on the same Steam account on the
Deck. Keep it Hidden/Private. It is separate from the normal possession mod;
the main mod is not updated by this diagnostic. Use the original Windows game
through official Valve **Proton 10.0-4**, including maintenance build 10.0-4b.
See Valve's [release](https://github.com/ValveSoftware/Proton/releases/tag/proton-10.0-4b).
This is a chosen test baseline, not a claim that full Deck gameplay is verified.

The first upload failed the linked-folder guard. Path-policy revision 1 got
past startup on the Deck and initialized Steam Input. The owner's results
photograph then showed zero accepted reads and a right-trigger origin mismatch:
357 versus 358. Those are the same trigger's valid soft/full-pull origins. An
overly strict check in our bridge rejected every complete frame, making all
recorded control counts zero. Diagnostic revision 2 corrects that check while
still refusing opposite-trigger, unrelated-control and digital aliases.

Revision 2 then ran for the full 90 seconds on the owner's Deck, accepting
10,583 reads with zero rejected reads. All displayed coverage passed except
L3/R3. The owner repeated the test with those clicks and reported neither was
recognized. The defaults incorrectly used `Soft_Press` for these two digital
stick buttons. Valve's installed Deck templates use `Full_Press` for actual
stick clicks and `Soft_Press` for pressure-sensitive trackpad clicks. Revision
3 follows that distinction, preserving the other bindings and physical-origin
checks. The results now explicitly show separate L3/R3 counts. The owner reports
the correction worked; no full revision3 count report was supplied.

Valve's existing Steam Input implementation handles the Deck hardware. The
mod uses a small versioned bridge checked against Valve's Proton declarations;
it does not implement the Deck's HID protocol. SDL's Deck HID driver was
researched but is not the selected backend. The native controller defaults
contain API actions, not keyboard/mouse or virtual-gamepad bindings.

## Run the test

1. Allow Steam to download the latest update to private item **3829785193**.
   Restart the Deck once to end the previous diagnostic's Steam Input session.
   Enable only `shogun2_unit_possession_deck_probe174`; keep normal Unit
   Possession and other mods disabled for this diagnostic.
2. Launch on the Deck through Proton to the main menu. Native initialization
   and binding checks run first. A failed binding shows a failure message
   without wasting a 90-second control exercise.
3. A **ready message** appears. Touch **OK** when ready. Waiting on that message
   does not consume the test time. Cancel skips the measurement.
4. For the next **90 seconds**, press L4/L5/R4/R5 separately; circle both sticks
   and press each stick straight down until it clicks (L3/R3), several times;
   touch, swipe and click both pads across all edges; squeeze each trigger
   slowly through its whole range; press the face/shoulder/D-pad/Menu/View/stick
   buttons; and tilt/rotate the Deck. Remain at the main menu. Gameplay commands
   are deliberately disabled. Steam and Quick Access keep their system roles.
5. A **RESULTS message** appears automatically and remains until dismissed.
   Photograph that screen and send it to the owner chat. There is no need to
   locate a report file. It shows measured duration, accepted/rejected reads,
   grip counts, exercised/missing controls, axes, triggers, gyro and any binding
   mismatch. Do not exit at a guessed deadline; wait for the results message.
6. Exit, disable the diagnostic, restore the normal mods and **restart the
   Deck before normal play**, as described below.

The messages are compiled into the diagnostic DLL and run inside the game
process. There is no separate UI script/helper. Revision 2's results screen was
photographed on the actual Deck; revision3's click correction is owner-accepted.

## Optional developer report

The same summary is saved as a small JSON beside the content-addressed native
DLL inside the game's Proton prefix, under:

    steamapps/compatdata/34330/pfx/drive_c/users/steamuser/AppData/Local/

Its name ends in `.deck174-report0.json` (or report1-3). The test manifest gives
the exact DLL hash for each build. Some installations keep compatdata in
another Steam library. Reports include `diagnostic_revision`,
`measurement_started`, `user_cancelled` and `measured_elapsed_ms`. The on-screen
summary is the normal owner test result; the file is optional.

At most four reports of 16 KiB each are created per diagnostic binary. Existing
files are preserved, and no report4 is created. The onscreen summary still works
when those slots are full. This is aggregate data, not a frame-by-frame log.
Small native/default files are cached. No recorder, debugger or crash collector
is started. Host sample timestamps are not physical display-latency measurements.
`native_inputs_observed` means coverage thresholds were met, not that gameplay,
all duplicate-input cases, suspend/resume or every Gaming Mode interaction passed.

For an optional manual transfer, with the game/launcher closed, copy only the
new diagnostic `.pack` into the Windows game's `data` folder. Do not duplicate
that manual copy with the Workshop subscription. The private Workshop route
requires no manual copy. No DLL replaces a game or Steam DLL.

## Restore normal play

With the game and launcher closed, disable the private test entry. Remove only an exact manual probe copy if you
previously created one in `data`. Re-enable your normal Unit Possession entry
and previous mods. **Restart the Deck once before normal play.** Valve documents
that this native action-manifest override lasts for the rest of the Steam
session; calling Steam Input Shutdown is not proof it restores the previous
configuration. The restart ends that test session. See Valve's
[manifest documentation](https://partner.steamgames.com/doc/features/steam_controller/action_manifest_file).
Automatic restoration without a restart remains part of the investigation.

Do not restore a legacy DInput loader. The normal Workshop
download, game executable, engine DLL, saves and graphics settings are not
replaced by this test.

Keep the generated report for review. The diagnostic never prunes user files.
There is no reason to remove the normal mod's resource cache.

## Build locally

Dependencies are the existing Windows x86 MSVC/SDK tools, Python 3.12, matching
float Lua 5.1 tools and a legitimate stock-engine game fixture described in
[BUILD_AND_TEST](../../BUILD_AND_TEST.md). This builder does not install or
launch the game. Use a new output directory.

Separately obtain `isteaminput.h` and `steam_api_flat.h` from Valve's Proton
repository, commit `e91ca2be0df2cef4c230cbbc0b86604d73a0bbf6`, directory
`lsteamclient/steamworks_sdk_162`. Their hashes are checked by `check_abi174.py`.
They are audit inputs, not shipped SDK headers. No Steam SDK DLL is needed.

    py -3.12 tools/deck/build174.py --game "D:\Owned Stock Fixture" --output "D:\Deck Work\test174" --vcvars "C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat" --lua "C:\Tools\Lua51Float\lua51.exe" --luac "C:\Tools\Lua51Float\luac51.exe" --abi-reference-dir "D:\Valve Reference"

Only `diagnostic-package` is intended for the owner test. Build outputs contain
private recovered stock-script fixtures; never distribute the whole build.
The separate normal Workshop builder does not include these Deck sources.

`adapt_paths174.py` produces two first-party headers inside the isolated source
snapshot. It checks the exact shared reader hashes, substitutes only path-policy
calls, and verifies all parser/root-fingerprint/coexistence logic is unchanged.
The normal shared Windows sources are not edited. Do not add generated adapters
to the repository; the Deck builder recreates them.

Under Proton, existing directory aliases and read-only file aliases are followed
using disk handles and a resolved local name. Output leaves must still be plain,
and exclusive creation/content checks remain active. This is not a guarantee
against a malicious process running as the same Unix user changing paths in a
race. On Windows the strict no-reparse policy remains. The tests use real NTFS
directory junctions; real Windows file-symlink creation was skipped for lack of
privilege, and actual Proton behavior remains an owner-test requirement.

Tests cover the actual MSVC x86 virtual-call/packed-return ABI against a fake
Steam implementation, normalized input and semantic routing, bounded production
resources/reports, real float-Lua loading, all 37 stock menu callbacks and menu
failure recovery. Those are desktop/offline checks, not a Deck test.
