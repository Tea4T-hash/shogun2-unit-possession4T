# Deck renderer frame-owner coexistence — revision37

Shogun2 Loader0.5.2 replaces renderer vtable slot161 at0x17547f8 with its frame
statistics callback, even when frame-file logging is disabled. The old Deck UI
admission hash covered the complete784-byte renderer table at0x1754574. That
unrelated entry therefore blocked initialization with Native Deck UI scaling
unavailable. The original engine file and Unit Possession pack were unchanged.

The UI contract now has two renderer table spans around exactly that four-byte
entry. All195 other entries, every original native code span and every hook
entry remain guarded. The viewport/projection/inverse-input methods use the
protected renderer methods, not the per-frame callback. Unit Possession never
calls, rewrites, chains, disables or restores the other mod's frame entry.
This is a dependency-scope correction, not a general occupied-hook bypass.

`tools/deck/generate_native211.py` regenerates the hashes/relocations from exact
owned stock and reviewed-Bugfix images. It asserts that the only difference in
covered bytes is that four-byte slot; no game code is emitted. Normal builds
check the maintained generated header against both owned input files.

The isolated native fixture reproduces the old full-table refusal, exercises
both installation orders, refuses880 mutations to code spans and other table
entries, and executes4896 original projection/inverse points. Providers for the
render device are mocked. Optional `--loader-fixture` checks the exact0.5.2 file
hash and decoded installer/hook addresses in a mapped DLL without executing
third-party code, DllMain, frame statistics, threads or game startup. Without
that optional file the same contract tests use an owned callback only.

The exact external DLL is not built into or redistributed with this mod. The
candidate retains the accepted menu footer correction and gameplay components.
Tests do not establish actual Proton startup, FPS, loading speed, battle safety
or universal loader/mod support. Both mods subsequently initialized in the
verified revision38 Deck session. Revision39 retains this correction and the
owner reported success after installation; no universal mod guarantee follows.
