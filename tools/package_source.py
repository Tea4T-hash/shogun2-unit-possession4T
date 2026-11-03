"""Create a source/local-resource kit. Never include locally built binaries or publish."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from build_frontend import source_files

TOOLS = {
    'README.md',
    'build.py',
    'build_frontend.py',
    'native_build.py',
    'configure.py',
    'game_packs.py',
    'prepare_resources.py',
    'prepare_audio109.py',
    'audio109.json',
    'resource-recipe.json',
    'generate_roles.py',
    'generate_tables.py',
    'generate_native_inventory.py',
    'package_source.py',
    'check_historical.py',
    'run_historical.py',
}
WORKSHOP = {
    'build_workshop165.py',
    'bootstrap165.lua',
    'entry165.cpp',
    'pack165.h',
    'ui_compat168.h',
    'verify_bootstrap165.lua',
    'verify_runtime165.cpp',
    'verify_storage166.cpp',
    'verify_compat168.cpp',
    'verify_qte_startup171.cpp',
    'generate_engine_contract172.py',
    'check_engine_contract172.py',
    'verify_engine_contract172.cpp',
    'verify_ui168.lua',
    'verify_loader168.cpp',
    'check_loader168.py',
    'verify_fallback167.lua',
    'verify_locales167.py',
    'owned_toy165.cpp',
    'PLAYING165.md',
    'PLAYING167.md',
    'PLAYING168.md',
    'PLAYING169.md',
    'PLAYING170.md',
    'PLAYING171.md',
    'PLAYING172.md',
    'PLAYING173.md',
}
DIAGNOSTICS_EXCLUDE = {
    'verify_watcher.py',
    'preserve_qte_log.py',
    'deploy110.py',
    'verify_hardening111.py',
}
ROOT_DOCS = {
    'AGENTS.md',
    'README.md',
    'BUILD_AND_TEST.md',
    'BUILD_STEAM_DECK.md',
    'SOURCE_MAP.md',
    'VALIDATION.md',
    'CONTRIBUTING.md',
    'THIRD_PARTY_NOTICES.md',
    'LICENSE',
    'VERSION',
    'CHANGELOG.md',
    '.gitignore',
    '.gitattributes',
}
DOCS = {
    'README.md',
    'ARCHITECTURE.md',
    'LOCAL_DEPENDENCIES.md',
    'STATUS.md',
    'KNOWN_ISSUES.md',
    'current-release.json',
}


def selected(root=ROOT):
    """Select portable source and guides; retain full notebooks only in Git."""
    version = (root / 'VERSION').read_text().strip()
    paths = list(source_files(root))
    deck_guide = root / 'BUILD_STEAM_DECK.md'
    if deck_guide.is_file() and deck_guide not in paths:
        paths.append(deck_guide)
    for path in paths:
        rel = path.relative_to(root)
        parts = rel.parts
        if len(parts) == 1:
            if path.name in ROOT_DOCS:
                yield path
        elif parts[0] == 'native':
            if path.suffix != '.cmd':
                yield path
        elif parts[0] == 'tests':
            yield path
        elif parts[0] == 'docs':
            if (
                (len(parts) == 2 and path.name in DOCS)
                or (len(parts) > 2 and parts[1] == 'public-release')
                or rel.as_posix()
                in {
                    'docs/development/README.md',
                    'docs/development/SteamDeck174.md',
                    'docs/development/SteamDeck175.md',
                    'docs/development/SteamDeck176.md',
                    'docs/development/SteamDeck177.md',
                    'docs/development/SteamDeck181.md',
                    'docs/development/HudVisibility198.md',
                    'docs/development/WallClimbing200.md',
                    'docs/development/AimingAndAcquisition201.md',
                    'docs/development/PossessionLook202.md',
                    'docs/development/Gameplay203.md',
                    'docs/development/Locomotion204.md',
                    'docs/development/Cavalry206.md',
                    'docs/development/AutomaticDevice192.md',
                    'docs/development/QoL193.md',
                    'docs/development/CursorAndBindings194.md',
                    'docs/releases/README.md',
                    'docs/releases/v' + version + '.md',
                }
            ):
                yield path
        elif parts[0] == 'tools':
            if len(parts) == 2 and path.name in TOOLS:
                yield path
            elif parts[1] == 'installer':
                yield path
            elif parts[1] == 'workshop' and path.name in WORKSHOP:
                yield path
            elif parts[1] == 'deck':
                yield path
            elif parts[1] in ('data', 'assets'):
                yield path
            elif (
                parts[1] == 'diagnostics'
                and path.name not in DIAGNOSTICS_EXCLUDE
                and path.suffix != '.cmd'
            ):
                yield path


def audit(files, root=ROOT):
    """Reject known private/payload patterns before creating the source ZIP."""
    forbidden = {
        '.exe',
        '.dll',
        '.pack',
        '.png',
        '.tga',
        '.cuf',
        '.wav',
        '.mp3',
        '.ogg',
        '.dmp',
        '.zip',
        '.pyc',
    }
    personal = re.compile(
        r'[A-Za-z]:[/\\]+Users[/\\]+[^\s/\\]+|'
        + 'codex'
        + '-runtimes'
        + r'|D:[/\\]+Shogun2Recordings',
        re.I,
    )
    secrets = re.compile(
        r'gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH )?PRIVATE KEY-----'
    )
    rows = []
    for path in files:
        rel = path.relative_to(root).as_posix()
        raw = path.read_bytes()
        if path.suffix.lower() in forbidden or raw.startswith(
            (b'MZ', b'PFH3', b'\x89PNG', b'PK\x03\x04')
        ):
            raise ValueError('Forbidden source payload: ' + rel)
        text = raw.decode('utf-8')
        if personal.search(text) or secrets.search(text):
            raise ValueError('Personal path or credential pattern: ' + rel)
        if path.name in ('config.json', 'baseline.json', 'local.json'):
            raise ValueError('Private configuration: ' + rel)
        rows.append(
            {'path': rel, 'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest()}
        )
    return rows


def verify_archive(path):
    """Check archive CRC, file closure, sizes and hashes against its manifest."""
    with zipfile.ZipFile(path) as archive:
        if archive.testzip():
            raise ValueError('Source archive CRC failure')
        manifest = json.loads(archive.read('source-manifest.json'))
        if set(archive.namelist()) != {r['path'] for r in manifest['files']} | {
            'source-manifest.json'
        }:
            raise ValueError('Archive file closure differs')
        for row in manifest['files']:
            raw = archive.read(row['path'])
            if (
                len(raw) != row['bytes']
                or hashlib.sha256(raw).hexdigest() != row['sha256']
            ):
                raise ValueError('Archive hash differs')
    return manifest


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output-dir', type=Path)
    p.add_argument('--verify', type=Path)
    o = p.parse_args()
    if o.verify:
        m = verify_archive(o.verify)
        print('PASS source archive ' + str(len(m['files'])) + ' files')
        return
    if not o.output_dir:
        p.error('--output-dir is required')
    out = o.output_dir.resolve()
    if out.exists() or out == ROOT or ROOT.is_relative_to(out):
        p.error('Use a new output directory')
    paths = list(selected())
    rows = audit(paths)
    revision = None
    dirty = None
    if (ROOT / '.git').exists():
        revision = subprocess.check_output(
            ['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True
        ).strip()
        dirty = bool(
            subprocess.check_output(
                ['git', 'status', '--porcelain'], cwd=ROOT, text=True
            ).strip()
        )
    manifest = {
        'schema': 1,
        'format': 'source/local-resource-kit',
        'git_revision': revision,
        'working_tree_changes': dirty,
        'content_sha256': hashlib.sha256(
            json.dumps(rows, sort_keys=True).encode()
        ).hexdigest(),
        'files': rows,
        'game_assets_included': False,
        'compiled_binaries_included': False,
        'published': False,
        'license': 'MIT for first-party code; vendor/game-material exclusions in THIRD_PARTY_NOTICES.md',
        'validation': 'See docs/current-release.json and VALIDATION.md; build, owner gameplay acceptance and release packaging checks are recorded separately',
        'excluded': [
            'prepared resources',
            'all compiled payloads',
            'private diagnostics/configuration/dumps',
            'historical operator docs/receipts',
            'legacy owner publication/deployment helpers',
        ],
    }
    out.mkdir(parents=True)
    archive = out / (
        'Shogun2-Unit-Possession-v'
        + (ROOT / 'VERSION').read_text().strip()
        + '-Source.zip'
    )
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
        for path in paths:
            z.write(path, path.relative_to(ROOT).as_posix())
        z.writestr('source-manifest.json', json.dumps(manifest, indent=2) + '\n')
    verify_archive(archive)
    (out / 'source-manifest.json').write_text(
        json.dumps(manifest, indent=2) + '\n', encoding='utf-8'
    )
    (out / 'artifact.json').write_text(
        json.dumps(
            {
                'file': archive.name,
                'bytes': archive.stat().st_size,
                'sha256': hashlib.sha256(archive.read_bytes()).hexdigest(),
                'audit_passed': True,
                'published': False,
            },
            indent=2,
        )
        + '\n'
    )
    print(
        'PASS source kit: '
        + str(archive)
        + '; '
        + str(len(rows))
        + ' audited source files; no publication'
    )


if __name__ == '__main__':
    main()
