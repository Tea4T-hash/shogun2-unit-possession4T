# Developer tools

Run commands from the repository root. [Build and test](../BUILD_AND_TEST.md)
contains the complete Workshop commands and dependencies. Tools do not install
or launch the game as part of a normal build; staging, recording and publishing
are separate operator actions.

## Routine development

| Tool | Purpose |
| --- | --- |
| `build.py` → `build_frontend.py` → `native_build.py` | Snapshot explicit source inputs, build x86 components, and optionally run native fixtures in isolated output folders |
| `workshop/build_workshop165.py` | Package the component build, compile matching float Lua, and run Workshop startup/resource/storage fixtures; pass `--candidate 173` for the current implementation |
| `configure.py` | Save ignored local game/tool paths in `tools/local.json`; command-line build paths remain explicit alternatives |
| `package_source.py` | Audit and export the source ZIP, then verify archive closure, sizes, hashes and CRC |
| `audit_repository.py` | Inspect the Git index for excluded files, binary signatures and recognized credential patterns; stage changes before running it |
| `generate_roles.py --check` | Compare diagnostic role constants with the native human-role policy |
| `generate_tables.py --check` | Compare six retained derived headers with canonical records |
| `generate_native_inventory.py --check` | Compare the generated source location index with native source |

Python tests run with `py -3.12 -m unittest discover -s tests -v`. They use
synthetic files/memory and safe entry-point help checks. A passing Python suite
does not mean the native or live-game suites ran.

## Resources and retained observations

`game_packs.py` supplies bounded read-only pack access for Python tools.
The native Workshop reader supports the defined PFH2/PFH3 layouts and uses
the shared engine contract; the separate resource-reference pipeline keeps
its own stock-input checks. `prepare_resources.py`, `prepare_audio109.py`, `assets`, and their
recipes support the independent resource-reference pipeline. Ordinary Workshop
preparation uses the native extractor in `installer` and needs no Node,
ImageMagick or FFmpeg installation.

`data` holds canonical roles/tables and historical fixture fingerprints.
Regenerating retained observations is distinct from sampling new game data.
See [derived data](../docs/public-release/DERIVED_DATA.md) and [resources](../docs/public-release/RESOURCES.md).

## Optional and historical tools

| Area | Scope |
| --- | --- |
| `deck` | Local input-only Steam Deck/Proton development probe and simulation tests; hardware proof and gameplay integration pending. Read its [test guide](deck/README.md). Not included in the normal Workshop payload |
| `installer` | Shared resource extraction plus historical Inno setup, Steam-launch and transaction fixtures; see its [scope note](installer/README.md) |
| `check_historical.py`, `run_historical.py` | Optional exact-archive preflight and compatibility tests; missing archives are skips, not passes |
| `diagnostics` | Optional external recording/debugging and dump analysis; read its [guide](diagnostics/README.md) before explicit operator use |
| Workshop `probe164`, `stage_*`, `prepare_upload165.py`, `subscription_swap165.py` and related helpers | Historical startup probes or operator staging; not required player steps and not part of the curated source kit |
| `package_release.py`, `github_release.py` | Historical installer-oriented release helpers; their fixed asset lists do not package the current Workshop release |

Use the [release procedure](../docs/public-release/RELEASING.md) for v0.33's
Workshop/source archive format. Builds and source exports do not authorize
publication, game-file changes, Steam configuration edits or capture.
