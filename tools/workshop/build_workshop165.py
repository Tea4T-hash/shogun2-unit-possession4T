"""Build/test a first-party-only Workshop candidate. Never install, launch or upload."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
for path in [ROOT / 'tools', ROOT / 'tools/installer', Path(__file__).resolve().parent]:
    sys.path.insert(0, str(path))
from game_packs import Pack, digest, require_game
from build_frontend import source_files
from audit_local_payload159 import audit, resources


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', type=int, default=165)
    parser.add_argument('--compat-engine', type=Path, help='Optional exact reviewed ModDB DLL for isolated actual-QTE startup verification; never installed')
    for name in ['game', 'components', 'output', 'vcvars', 'lua', 'luac']:
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    from check_engine_contract172 import check as check_engine172
    check_engine172()
    game = args.game.resolve()
    components = args.components.resolve()
    out = args.output.resolve()
    if args.candidate < 165:
        parser.error('Workshop candidate must be at least 165')
    require_game(game)
    if (
        out.exists()
        or out == game
        or out.is_relative_to(game)
        or ROOT.is_relative_to(out)
    ):
        parser.error('Use a new output outside game/source ancestors')
    for tool in [args.vcvars, args.lua, args.luac]:
        if not tool.is_file():
            parser.error('Missing build tool: ' + str(tool))
    receipt = json.loads((components.parent / 'build-results.json').read_text())
    names = [
        'possession-v46.dll',
        'possession-hud.exe',
        'qte-playable-v79.dll',
        'prepare-local-assets159.exe',
    ]
    hashes = {
        row['file'].replace('\\', '/'): row['sha256'] for row in receipt['binaries']
    }
    for name in names:
        if digest(components / name) != hashes['bin/' + name]:
            raise ValueError('Component differs from its build receipt: ' + name)
    prior = json.loads((components.parent / 'build-inputs.json').read_text())
    protected = []
    for row in prior['source_files']:
        if row['path'].startswith(('native/outputs/', 'native/common/')) and Path(
            row['path']
        ).suffix in {'.cpp', '.c', '.h', '.def', '.rc'}:
            if digest(ROOT / row['path']) != row['sha256']:
                raise ValueError('Protected component source changed: ' + row['path'])
            protected.append(row)
    checked = {'passed': True, 'game_images_fonts_audio_embedded': False, 'files': {}}
    for name in names:
        leaves = resources(components / name)
        if any(key[0] not in (16, 24) for key in leaves):
            raise ValueError('Unexpected component resource: ' + name)
        checked['files'][name] = {
            'sha256': digest(components / name),
            'resource_keys': [str(key) for key in leaves],
        }
    out.mkdir(parents=True)
    snapshot = out / 'source'
    bin_dir = out / 'bin'
    bin_dir.mkdir()
    logs = out / 'logs'
    logs.mkdir()
    inputs = []
    paths = {p.relative_to(ROOT): p for p in source_files(ROOT)}
    for p in (ROOT / 'tools/workshop').glob('*.lua'):
        paths[p.relative_to(ROOT)] = p
    for relative, path in sorted(paths.items()):
        target = snapshot / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
        inputs.append({'path': relative.as_posix(), 'sha256': digest(path)})
    for name in names:
        shutil.copy2(components / name, bin_dir / name)
    auto = snapshot / 'native/work/autoload99-preview'
    work = snapshot / 'tools/workshop'
    # Documentation/bootstrap-only revisions reuse the same component cache.
    identity = hashlib.sha256(
        json.dumps({n: digest(bin_dir / n) for n in names}, sort_keys=True).encode()
    ).hexdigest()[:16]
    build_id = str(args.candidate) + '-' + identity
    header = '#pragma once\n' + ''.join(
        'static const char* EMBEDDED_'
        + label
        + '_SHA="'
        + digest(bin_dir / name)
        + '";\n'
        for label, name in zip(['ADAPTER', 'HUD', 'QTE', 'PREPARER'], names)
    )
    header += (
        'static const char* EMBEDDED_PACK_SHA="";\nstatic const wchar_t* EMBEDDED_BUILD=L"'
        + build_id
        + '";\n'
    )
    header += (
        'static constexpr unsigned EMBEDDED_WORKSHOP_TEST='
        + str(args.candidate)
        + ';\n'
    )
    (auto / 'embedded_hashes.h').write_text(header)
    rc = out / 'payload165.rc'
    rc.write_text(
        ''.join(
            f'{i} RCDATA "{(bin_dir/name).as_posix()}"\n'
            for i, name in zip([101, 102, 103, 104], names)
        )
    )
    results = []

    def command(values):
        if any(any(c in str(v) for c in '\r\n%&|<>^!') for v in values):
            raise ValueError('Unsupported batch argument')
        return subprocess.list2cmdline(list(map(str, values)))

    def run(label, commands, env=None):
        batch = logs / (label + '.cmd')
        log = logs / (label + '.log')
        lines = [
            '@echo off',
            'call ' + command([args.vcvars.resolve()]),
            'if errorlevel 1 exit /b %errorlevel%',
        ]
        for values in commands:
            lines += [command(values), 'if errorlevel 1 exit /b %errorlevel%']
        batch.write_text('\r\n'.join(lines) + '\r\n')
        print('Running ' + label, flush=True)
        started = time.monotonic()
        with log.open('w') as stream:
            r = subprocess.run(
                [os.environ.get('COMSPEC', 'cmd.exe'), '/d', '/c', batch],
                cwd=out,
                env=env,
                stdout=stream,
                stderr=subprocess.STDOUT,
            )
        row = {
            'name': label,
            'passed': r.returncode == 0,
            'seconds': round(time.monotonic() - started, 3),
            'log': str(log.relative_to(out)),
        }
        results.append(row)
        (out / 'checks.json').write_text(json.dumps(results, indent=2) + '\n')
        if r.returncode:
            raise RuntimeError(
                label + ' failed:\n' + log.read_text(errors='replace')[-5000:]
            )
        print('PASS ' + label, flush=True)

    common = [
        'cl',
        '/nologo',
        '/std:c++17',
        '/EHsc',
        '/O2',
        '/MT',
        '/W4',
        '/DNOMINMAX',
        '/DS2FP_LOCAL_ASSETS',
        '/DPOSSESSION_WORKSHOP_BUILD',
    ]
    libs = ['bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib', 'gdiplus.lib', 'gdi32.lib']
    res = out / 'payload165.res'
    dll = bin_dir / 's2up-workshop165.dll'
    fixture = bin_dir / 's2up-workshop165-fixture.dll'
    vendor=ROOT/'native/outputs/shogun2-accuracy90-preview/vendor'
    hooks=[]
    for source in ['buffer.c','hook.c','trampoline.c','hde/hde32.c']:
        obj=out/('bridge-'+Path(source).stem+'.obj');hooks.append(obj)
        run('bridge-'+Path(source).stem, [['cl','/nologo','/O2','/MT','/W3','/I'+str(vendor/'include'),'/c',vendor/'src'/source,'/Fo'+str(obj)]])
    run(
        'compile',
        [
            ['rc', '/nologo', '/fo', res, rc],
            [
                *common,
                '/LD',
                work / 'entry165.cpp',
                res,*hooks,
                '/Fo' + str(out / 'entry165.obj'),
                '/Fe' + str(dll),
                '/link',
                *libs,
            ],
            [
                *common,
                '/DS2UP165_LUA_FIXTURE',
                '/LD',
                work / 'entry165.cpp',
                res,*hooks,
                '/Fo' + str(out / 'entry165-fixture.obj'),
                '/Fe' + str(fixture),
                '/link',
                *libs,
            ],
            [
                *common,
                work / 'verify_runtime165.cpp',
                res,
                '/Fo' + str(out / 'verify165.obj'),
                '/Fe' + str(bin_dir / 'verify-runtime165.exe'),
                '/link',
                *libs,
            ],
            [
                'cl',
                '/nologo',
                '/O2',
                '/MT',
                work / 'owned_toy165.cpp',
                '/Fo' + str(out / 'toy165.obj'),
                '/Fe' + str(bin_dir / 'owned-toy165.exe'),
            ],
            [
                *common,
                work / 'verify_storage166.cpp',
                '/Fo' + str(out / 'storage166.obj'),
                '/Fe' + str(bin_dir / 'verify-storage166.exe'),
                '/link',
                *libs,
            ],
            [*common, work / 'verify_compat168.cpp',
             '/Fo' + str(out / 'compat168.obj'),
             '/Fe' + str(bin_dir / 'verify-compat168.exe'), '/link', *libs],
            [*common, work / 'verify_qte_startup171.cpp',
             '/Fo' + str(out / 'qte-startup171.obj'),
             '/Fe' + str(bin_dir / 'verify-qte-startup171.exe')],
            [*common, work / 'verify_engine_contract172.cpp',
             '/Fo' + str(out / 'engine-contract172.obj'),
             '/Fe' + str(bin_dir / 'verify-engine-contract172.exe'), '/link', *libs],
        ],
    )
    leaves = resources(dll)
    embedded = {key[1]: value for key, value in leaves.items() if key[0] == 10}
    if set(embedded) != {101, 102, 103, 104} or any(
        key[0] not in (10, 16, 24) for key in leaves
    ):
        raise ValueError('Unexpected loader resource')
    for i, name in zip([101, 102, 103, 104], names):
        if embedded[i] != (bin_dir / name).read_bytes():
            raise ValueError('Embedded component closure differs')

    def script(native, target):
        payload = native.read_bytes()
        sha = digest(native)
        encoded = ',\n'.join(
            '"' + ''.join('\\%03d' % b for b in payload[i : i + 1024]) + '"'
            for i in range(0, len(payload), 1024)
        )
        text = (
            (work / 'bootstrap165.lua')
            .read_text()
            .replace('__NATIVE_SHA165__', sha)
            .replace('-- __NATIVE_PAYLOAD165__', encoded)
        )
        source = out / (target + '.lua')
        source.write_text(text)
        compiled = out / (target + '.luac')
        subprocess.run(
            [args.luac.resolve(), '-s', '-o', compiled, source], check=True, cwd=out
        )
        if compiled.read_bytes()[:12] != bytes.fromhex('1b4c75615100010404040400'):
            raise ValueError('Matching float Lua compiler required')
        return compiled

    delivered = script(dll, 'bootstrap-production')
    fixture_script = script(fixture, 'bootstrap-fixture')
    stock = Pack(game / 'data/data.pack').read(
        'ui/frontend ui/layout_scripts/root.luac',
        13739,
        '255b305cefb24a68cf6d2fc79d07efad7c0252acfd98ecd3ad10fa55c959cae8',
    )
    reference = out / 'private-owned-root.luac'
    reference.write_bytes(stock)
    lua_env = dict(
        os.environ,
        LOCALAPPDATA=str(out / 'lua cache with spaces'),
        S2UP165_FIXTURE_GAME=str(game),
    )
    Path(lua_env['LOCALAPPDATA']).mkdir()
    lua_cache = Path(lua_env['LOCALAPPDATA'])
    pending_native = lua_cache / ('S2UP-Workshop165-' + digest(fixture) + '.dll.new.1')
    pending_native.write_bytes(fixture.read_bytes())
    run(
        'lua-bridge',
        [
            [
                args.lua.resolve(),
                work / 'verify_bootstrap165.lua',
                reference,
                fixture_script,
                dll,
                fixture,
            ]
        ],
        lua_env,
    )
    if pending_native.exists():
        raise ValueError('Completed interrupted native staging was not reused')
    run(
        'menu-failure-recovery',
        [[args.lua.resolve(), work / 'verify_fallback167.lua', reference,
          fixture_script, game / 'data/data.pack', fixture]],
        lua_env,
    )
    ui_fixtures = out / 'ui-mod-fixtures'
    ui_fixtures.mkdir()
    other_root = (
        'local c=assert(loadfile(' + json.dumps(reference.as_posix()) + '));'
        'setfenv(c,getfenv(1));c(...);'
        'function CompatibilityMenu168() return "other menu mod" end\n'
    ).encode()
    ui_name = b'ui\\frontend ui\\layout_scripts\\root.luac\0'
    ui_index = struct.pack('<I', len(other_root)) + ui_name
    (ui_fixtures / 'other-ui.pack').write_bytes(
        struct.pack('<4s5IQ', b'PFH3', 3, 0, 0, 1, len(ui_index), 1)
        + ui_index + other_root
    )
    selected_ui = ui_fixtures / 'used_mods.txt'
    selected_ui.write_text(
        'add_working_directory "' + ui_fixtures.as_posix() + '";\n'
        'mod "other-ui.pack";\n', encoding='utf-16'
    )
    ui_env = dict(lua_env, S2UP168_FIXTURE_SELECTION=str(selected_ui))
    run('enabled-ui-mod', [[args.lua.resolve(), work / 'verify_ui168.lua', fixture_script]], ui_env)
    # Real compiled bootstrap and native bridge, with renamed duplicate copies.
    # The self archive must not be mistaken for another menu and re-entered.
    self_meta = json.dumps({'first_party_bootstrap': True, 'native_sha256': digest(fixture)}).encode()
    self_root = fixture_script.read_bytes()
    self_index = (struct.pack('<I', len(self_root)) + ui_name
                  + struct.pack('<I', len(self_meta)) + b'possession\\workshop165.json\0')
    self_pack = (struct.pack('<4s5IQ', b'PFH3', 3, 0, 0, 2, len(self_index), 1)
                 + self_index + self_root + self_meta)
    duplicate_folder = ui_fixtures / 'second copy'
    duplicate_folder.mkdir()
    (ui_fixtures / 'renamed-possession.pack').write_bytes(self_pack)
    (duplicate_folder / 'renamed-possession.pack').write_bytes(self_pack)
    renamed_selection = ui_fixtures / 'renamed_used_mods.txt'
    renamed_selection.write_text(
        'add_working_directory "' + ui_fixtures.as_posix() + '";\n'
        'add_working_directory "' + duplicate_folder.as_posix() + '";\n'
        'mod "renamed-possession.pack";\nmod "other-ui.pack";\n', encoding='utf-16'
    )
    renamed_env = dict(lua_env, S2UP168_FIXTURE_SELECTION=str(renamed_selection))
    run('renamed-duplicate-bootstrap', [[args.lua.resolve(), work / 'verify_ui168.lua', fixture_script]], renamed_env)
    if any(p.name.endswith('.root.luac') for p in lua_cache.iterdir()):
        raise ValueError('Transient UI handoff accumulated')

    def cache_state():
        return {
            p.name: (p.stat().st_size, digest(p))
            for p in lua_cache.iterdir()
            if p.is_file()
        }

    initial_cache = cache_state()
    run(
        'repeat-startup',
        [
            [
                args.lua.resolve(),
                work / 'verify_bootstrap165.lua',
                reference,
                fixture_script,
                dll,
                fixture,
            ]
            for _ in range(8)
        ],
        lua_env,
    )
    if cache_state() != initial_cache:
        raise ValueError('Repeated startup accumulated or changed cached files')
    if any(p.suffix in ('.log', '.jsonl', '.json') for p in lua_cache.iterdir()):
        raise ValueError('Startup persisted logs or per-process receipts')
    run(
        'runtime-resources',
        [
            [
                bin_dir / 'verify-runtime165.exe',
                game,
                out / 'runtime-fixtures',
                bin_dir / 'owned-toy165.exe',
                components,
            ]
        ],
    )
    run(
        'storage-lifetime',
        [[bin_dir / 'verify-storage166.exe', out / 'storage-fixtures']],
    )
    run('mod-coexistence', [[bin_dir / 'verify-compat168.exe', game, out / 'compatibility-fixtures']])
    run('actual-qte-stock-startup', [[bin_dir / 'verify-qte-startup171.exe',
        game / 'Empire.Retail.dll', bin_dir / 'qte-playable-v79.dll', 'Possession module unavailable']])
    run('actual-qte-occupied-live-hook', [[bin_dir / 'verify-qte-startup171.exe',
        game / 'Empire.Retail.dll',bin_dir / 'qte-playable-v79.dll',
        'Engine compatibility: occupied/changed hook at RVA 0x3e5d40*','0x3e5d40']])
    # Build identity alone no longer decides admission. The actual worker must
    # accept an identity-only edit, but reject a changed required native region.
    unknown_folder = out / 'unknown-engine-fixture'
    unknown_folder.mkdir()
    unknown_image = unknown_folder / 'Empire.Retail.dll'
    unknown_bytes = bytearray((game / 'Empire.Retail.dll').read_bytes())
    pe_offset = struct.unpack_from('<I', unknown_bytes, 0x3c)[0]
    unknown_bytes[pe_offset + 8] ^= 1
    unknown_image.write_bytes(unknown_bytes)
    run('actual-qte-independent-identity', [[bin_dir / 'verify-qte-startup171.exe',
        unknown_image, bin_dir / 'qte-playable-v79.dll', 'Possession module unavailable']])
    conflict_folder=out / 'critical-engine-fixture'
    conflict_folder.mkdir()
    conflict_image=conflict_folder / 'Empire.Retail.dll'
    conflict=bytearray((game / 'Empire.Retail.dll').read_bytes())
    section_count=struct.unpack_from('<H',conflict,pe_offset+6)[0]
    section_table=pe_offset+24+struct.unpack_from('<H',conflict,pe_offset+20)[0]
    for i in range(section_count):
        at=section_table+i*40
        rva,raw_size,raw=struct.unpack_from('<III',conflict,at+12)
        if rva<=0x3e5d40<rva+raw_size:
            conflict[raw+0x3e5d40-rva]^=1
            break
    else:
        raise ValueError('Critical fixture RVA not mapped')
    conflict_image.write_bytes(conflict)
    run('actual-qte-critical-engine-refusal', [[bin_dir / 'verify-qte-startup171.exe',
        conflict_image, bin_dir / 'qte-playable-v79.dll', 'Engine compatibility: required native region differs at RVA *']])
    if args.compat_engine:
        reviewed = re.search(r'bugfix_sha\[\]="([a-f0-9]{64})"',
            (snapshot / 'native/common/engine_profiles170.h').read_text()).group(1)
        if digest(args.compat_engine) != reviewed:
            raise ValueError('Optional compatibility image differs from the exact reviewed ModDB profile')
        image_folder = out / 'reviewed-engine-fixture'
        image_folder.mkdir()
        image = image_folder / 'Empire.Retail.dll'
        shutil.copyfile(args.compat_engine, image)
        if digest(image) != reviewed:
            raise ValueError('Optional compatibility image copy differs')
        run('actual-qte-reviewed-engine-startup', [[bin_dir / 'verify-qte-startup171.exe',
            image, bin_dir / 'qte-playable-v79.dll', 'Possession module unavailable']])
        run('engine-contract-controls', [[bin_dir / 'verify-engine-contract172.exe',
            game / 'Empire.Retail.dll',args.compat_engine.resolve(),out / 'engine-contract-fixtures']])
        independent_folder=out / 'independent-code-engine-fixture'
        independent_folder.mkdir()
        independent_image=independent_folder / 'Empire.Retail.dll'
        shutil.copyfile(out / 'engine-contract-fixtures/independent.dll',independent_image)
        run('actual-qte-independent-code', [[bin_dir / 'verify-qte-startup171.exe',
            independent_image, bin_dir / 'qte-playable-v79.dll', 'Possession module unavailable']])
    payload = delivered.read_bytes()
    if stock in payload:
        raise ValueError('Original stock script leaked into delivery')
    metadata = json.dumps(
        {
            'test': args.candidate,
            'native_sha256': digest(dll),
            'first_party_bootstrap': True,
            'resources': 'prepared from owned game',
            'chooser': False,
            'default_view': 'third-person',
            'routine_disk_logging': False,
            'external_recorder_debugger': False,
            'gameplay_components': {name: digest(bin_dir / name) for name in names},
        }
    ).encode()
    entries = {
        'ui\\frontend ui\\layout_scripts\\root.luac': payload,
        'possession\\workshop165.json': metadata,
    }
    for path, name in [
        (ROOT / 'LICENSE', 'LICENSE.txt'),
        (ROOT / 'THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_NOTICES.md'),
        (
            ROOT / 'native/outputs/shogun2-possession-cleanup-v46/vendor/LICENSE.txt',
            'MinHook-LICENSE.txt',
        ),
    ]:
        entries['possession\\licenses\\' + name] = path.read_bytes()
    playing = work / ('PLAYING173.md' if args.candidate >= 173 else 'PLAYING172.md' if args.candidate >= 172 else 'PLAYING171.md' if args.candidate >= 171 else 'PLAYING170.md' if args.candidate >= 170 else 'PLAYING169.md' if args.candidate >= 169 else 'PLAYING168.md' if args.candidate >= 168 else 'PLAYING167.md' if args.candidate >= 167 else 'PLAYING165.md')
    entries['possession\\PLAYING.md'] = playing.read_bytes()
    index = b''.join(
        struct.pack('<I', len(value)) + name.encode() + b'\0'
        for name, value in entries.items()
    )
    with (game / 'data/data.pack').open('rb') as stream:
        stamp = struct.unpack_from('<Q', stream.read(32), 24)[0]
    if not stamp:
        raise ValueError('Owned-stock timestamp is zero')
    package = out / 'package'
    package.mkdir()
    pack = package / 'shogun2_unit_possession_workshop165.pack'
    pack.write_bytes(
        struct.pack('<4s5IQ', b'PFH3', 3, 0, 0, len(entries), len(index), stamp)
        + index
        + b''.join(entries.values())
    )
    parsed = Pack(pack)
    for name, value in entries.items():
        if (
            parsed.read(
                name.replace('\\', '/'), len(value), hashlib.sha256(value).hexdigest()
            )
            != value
        ):
            raise ValueError('Pack roundtrip differs')
    for name in ['LICENSE', 'THIRD_PARTY_NOTICES.md']:
        shutil.copy2(ROOT / name, package / name)
    shutil.copy2(playing, package / 'PLAYING.md')
    shutil.copy2(
        ROOT / 'native/outputs/shogun2-possession-cleanup-v46/vendor/LICENSE.txt',
        package / 'MinHook-LICENSE.txt',
    )
    manifest = {
        'test': args.candidate,
        'pack_sha256': digest(pack),
        'pack': pack.name,
        'native_sha256': digest(dll),
        'native_bytes': dll.stat().st_size,
        'embedded_build': build_id,
        'first_party_code_only': True,
        'game_script_artwork_fonts_audio_shipped': False,
        'owned_script_read_locally': True,
        'chooser': False,
        'starting_view': 'third-person',
        'P_switches_view': True,
        'component_asset_audit': checked,
        'components': {name: digest(bin_dir / name) for name in names},
        'protected_component_source_count': len(protected),
        'retained_component_build': str(components.parent),
        'source_files': inputs,
        'fresh_checks': results,
        'routine_disk_logging': False,
        'external_recorder_debugger': False,
        'receipts_delete_on_process_exit': True,
        'resources_shared_by_recipe': True,
        'live_validation': False,
        'game_installed_or_launched': False,
        'game_changed': False,
        'public_package_ready': False,
        'steam_settings_changed': False,
        'uploaded': False,
    }
    (out / 'verification.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(
        json.dumps(
            {
                k: manifest[k]
                for k in [
                    'pack',
                    'pack_sha256',
                    'native_sha256',
                    'embedded_build',
                    'first_party_code_only',
                    'live_validation',
                ]
            },
            indent=2,
        ),
        flush=True,
    )


if __name__ == '__main__':
    main()
