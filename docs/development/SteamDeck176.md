# Private Deck test176: resource-cache startup correction

This updates the existing private item **3829785193**. It retains the exact
Test175 adapter, HUD, QTE and extractor binaries. The Workshop startup bridge
releases its temporary resource-validation mapping before committing a hard
link. Integrity checks, immutable resource readers and foreign-file preservation
remain unchanged. The normal Windows release is unchanged.

The owner reported `Cannot commit local resource cache; existing files preserved
(Win32=32)` on the Deck. The old helper keeps its source mapped without delete
sharing while renaming an alias of that source. Proton tracks sharing across
open handles to the same inode. The corrected helper closes only its temporary
validator before the rename; it does not add broad delete/write sharing or
disable hard-link storage reuse.

The targeted desktop fixture uses real resource bundles, mappings, hard links
and Windows handle sharing. At the rename boundary it applies the same-inode
sharing check: the old production helper fails with32 for both HUD/QTE and an
existing valid pending link; the corrected helper commits them. Copy fallback,
repeat use without growth, foreign targets/stages and independent reader locks
are also checked. This is a controlled sharing-policy reproduction, not actual
Proton execution or proof that every possible Deck startup issue is solved.

Primary references: [Valve's Proton10 Wine file-descriptor implementation](https://github.com/ValveSoftware/wine/blob/proton_10.0/server/fd.c)
and [Microsoft's CreateFile sharing contract](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew).

## Owner test

1. On PC, Update the **same private Deck item**, keeping Hidden/Private visibility.
2. Exit the game/launcher on the Deck. Let Steam finish downloading the update.
   Restart the Deck to end the earlier native-input manifest session.
3. Use official **Proton10.0-4** (10.0-4b maintenance build is suitable), with
   only the private test enabled. Try startup and then a land battle.

No cache deletion, manual DLL copying, extra recording or READY message is
needed for this manual Deck test. The developer cannot inspect the Deck remotely.
Report whether startup succeeds and how the controls behave. An owner report
is distinct from an independently read loaded-module/hash/hook check.

If another error appears, send its screenshot; do not delete preserved cache
files. When returning to normal play, disable the private test and restart the
Deck to end the Steam Input manifest override.

## RTS controls retained from175

| Control | Action |
| --- | --- |
| Left stick; hold L3 for faster pan | Native RTS camera pan |
| Right stick horizontal/vertical | Camera rotation/native wheel zoom |
| Hold R3 + right stick vertical | Camera pitch |
| Right pad swipe | Native game cursor |
| Right pad click/hold + swipe | Native selection/selection box |
| A; hold R5 to add to selection | Native selection click |
| R2 press, swipe, release | Contextual native order/formation drag |
| B during a drag | Cancel without committing |
| X | Native Halt |

Controller possession/QTE, groups/wheels, remaining shortcuts/run-walk,
campaign/naval and suspend/resume validation remain pending. No90-second input
diagnostic runs in this full-mod test. No recorder/debugger/profiler starts.
Required artwork/fonts/icons/voice still come from the user's owned game.

`tools/deck/build176.py` creates a private snapshot, verifies exact component
source/binary reuse, applies the reversible cache-only correction, preserves
engine guards, runs targeted old/new fixtures and the16 Workshop checks.
It never installs, starts the game or uploads. Full fixture folders contain
owned-game resources and must remain private.
