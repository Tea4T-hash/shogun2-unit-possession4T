# Build and test

**v0.34:** use [BUILD_STEAM_DECK.md](BUILD_STEAM_DECK.md) for the maintained combined PC/Deck runtime. The Test173 commands below describe the preceding v0.33 build and do not include Deck integration.

v0.33 packages **Test173**, based on the owner-accepted Test172 implementation.
Test173 has offline checks; its live acceptance remains pending. Directory suffixes such as
45, 99, 159 and 165 are historical implementation names, not product versions.
A build never installs, launches the game, edits Steam settings or publishes.
Use the [developer guide](docs/README.md) for the reading path and the
[tool guide](tools/README.md) to distinguish current commands from operator and
historical helpers.

## Build the Workshop pack

Developers need Windows 10/11, Python 3.12, MSVC x86 Build Tools with the Windows
SDK, and an x86 Lua 5.1 interpreter/compiler configured with float numbers.
The Lua chunk header must be `1b4c75615100010404040400`; the usual double-number
compiler is incompatible. Players need none of these development tools.

Use an owned original Windows Steam installation, including data.pack and the
compatible native fonts from local_en.pack or a supported local_*.pack.
English voice is optional. The owned stock Empire.Retail.dll SHA-256 is:

    022fae2e2da92b59b8b1b8ee52887003c624e3c8eca060b209f5d792a7f874f5

In PowerShell, replace the example paths and use new output directories:

    $game = 'D:\Steam Library\steamapps\common\Total War SHOGUN 2'
    $work = 'D:\Shogun Mod Build'
    $vc = 'C:\BuildTools\VC\Auxiliary\Build\vcvars32.bat'
    $lua = 'C:\Tools\Lua51Float\lua51.exe'
    $luac = 'C:\Tools\Lua51Float\luac51.exe'
    py -3.12 tools/build.py workshop --tests --candidate 173 --game $game --output-dir "$work\native" --vcvars $vc
    py -3.12 tools/workshop/build_workshop165.py --candidate 173 --game $game --components "$work\native\bin" --output "$work\workshop" --vcvars $vc --lua $lua --luac $luac

The distributable folder is `workshop/package`. Its filename retains
`shogun2_unit_possession_workshop165.pack` to update the existing Workshop item.
Keep the rest of the build private: fixtures contain recovered game scripts
and locally generated resource bundles. Never upload complete build folders.
The Workshop target excludes the legacy loader, camera chooser and installer.

The native bootstrap recovers the original UI script and prepares required
artwork/audio locally. It does not modify Steam settings. See the [bootstrap
design](docs/public-release/WORKSHOP165.md) and [storage changes](docs/public-release/WORKSHOP166.md).

## Checks

    py -3.12 -m unittest discover -s tests -v
    py -3.12 tools/generate_roles.py --check
    py -3.12 tools/generate_tables.py --check
    py -3.12 tools/generate_native_inventory.py --check

These Python and metadata checks need no installed game or native compiler.
The inventory is a source index; after relevant native source changes, regenerate
it with `py -3.12 tools/generate_native_inventory.py` and review its diff. Do not
regenerate factual tables merely to make a regression pass.

Test173 passed 100 adapter, 50 QTE and 8 HUD executables plus local-resource
format/renderer checks. Sixteen Workshop checks cover Lua/menu recovery,
coexistence, resource/storage lifetimes, actual QTE startup and the shared
engine contract. These are offline fixtures, not a live gameplay result.

    py -3.12 tools/workshop/check_engine_contract172.py

The source registry must match all compiled dependencies before a native build.
If changing a native binding, review and regenerate the contract using separately
obtained engine files and preserved analysis; see [engine compatibility](docs/public-release/WORKSHOP172.md).
Do not regenerate a contract merely to hide a mismatch. Normal builds need no
decoder database. Use `--compat-engine <reviewed-Bugfix-DLL>` on the Workshop
builder only when that separately obtained fixture is available.

See [validation](VALIDATION.md). Fixtures cannot establish every unit/map
combination, stability or real display/input latency.

## Resources and source export

The extractor stores entry hashes and atlas coordinates, not game asset bytes.
It reads each player's owned game and prepares the HUD/QTE resources there.
The independent [resource pipeline](docs/public-release/RESOURCES.md) can be used
for comparisons and metadata regeneration. Do not distribute generated .s2fp
bundles, fonts, art, audio or recovered stock scripts.

    py -3.12 tools/package_source.py --output-dir "$work\source-export"

The source kit contains the Workshop builder and native sources, a file/hash
manifest, and no compiled payloads or game resources. Optional diagnostic
source is included for developers; those tools are not in the player pack and
are not started by building or playing.

## Older setup builds

Legacy Inno setup and DInput-loader sources remain for historical compatibility
and transaction tests. They are not the current player route. Inno Setup is not a
Workshop build dependency. Do not install the old loader alongside Workshop.
Older versioned validation documents describe those earlier builds only.
