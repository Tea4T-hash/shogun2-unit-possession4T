"""Isolated native build orchestration. Defaults never install or launch."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from game_packs import digest, require_game
from native_build import find_vcvars


def source_files(root):
    """Enumerate portable snapshot inputs without requiring a Git checkout."""
    # Works from a source archive as well as a Git checkout. Exclude all build,
    # game, vendor-runtime and local configuration material independently of Git.
    tops = ('native', 'tools', 'tests', 'docs')
    suffixes = {
        '.cpp',
        '.c',
        '.h',
        '.py',
        '.lua',
        '.ps1',
        '.cmd',
        '.def',
        '.rc',
        '.md',
        '.txt',
        '.json',
        '.csv',
        '.mjs',
        '.js',
        '.iss',
    }
    for folder in tops:
        for path in sorted((root / folder).rglob('*')):
            relative = path.relative_to(root)
            if any(
                part in {'artifacts', 'node_modules', '__pycache__', 'private', 'build'}
                or part.startswith(('fixture-', 'embedded-fixture-'))
                for part in relative.parts
            ):
                continue
            if (
                path.is_file()
                and path.suffix in suffixes
                and path.name
                not in {
                    'local.json',
                    'embedded_hashes.h',
                    'setup_hashes.h',
                    'cache_catalog.h',
                    'config.json',
                    'baseline.json',
                }
            ):
                yield path
    for name in (
        'AGENTS.md',
        'README.md',
        'BUILD_AND_TEST.md',
        'SOURCE_MAP.md',
        'VALIDATION.md',
        'THIRD_PARTY_NOTICES.md',
        'LICENSE',
        'CONTRIBUTING.md',
        'VERSION',
        'CHANGELOG.md',
        '.gitignore',
        '.gitattributes',
    ):
        if (root / name).is_file():
            yield root / name
    # This first-party default is a build input, not a personal configuration.
    # Keep the exception exact rather than exporting arbitrary .ini files.
    deck_defaults = root / 'native/deck/controller174.ini'
    if deck_defaults.is_file():
        yield deck_defaults


def resource_inputs(folder):
    """Validate a resource-reference receipt without installing its contents."""
    receipt = json.loads(
        (folder / 'resource-manifest.json').read_text(encoding='utf-8')
    )
    if receipt['profile'] != 'local-game' or receipt['old_release_resources_used']:
        raise ValueError('Use locally regenerated resources for the public build')
    if receipt['recipe_sha256'] != digest(ROOT / 'tools/resource-recipe.json'):
        raise ValueError(
            'Resource recipe changed; regenerate in a fresh output directory'
        )
    for row in receipt['files']:
        relative = Path(row['path'])
        if (
            relative.is_absolute()
            or '..' in relative.parts
            or relative.parts[0] != 'native'
        ):
            raise ValueError('Unsafe prepared-resource path')
        original = folder / 'prepared' / relative
        if (
            not original.is_file()
            or original.stat().st_size != row['bytes']
            or digest(original) != row['sha256']
        ):
            raise ValueError('Prepared resource changed: ' + str(relative))
    return receipt


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        'target',
        choices=['adapter', 'qte', 'hud', 'workshop', 'all'],
        help='Standalone DLL/HUD target, or all (includes loader and installer)',
    )
    parser.add_argument('--game', type=Path)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--resources', type=Path)
    parser.add_argument(
        '--local-assets',
        action='store_true',
        default=True,
        help='Local resources are always enabled',
    )
    parser.add_argument(
        '--release',
        action='store_true',
        help='Use release metadata without installing or publishing',
    )
    parser.add_argument('--vcvars', type=Path)
    parser.add_argument('--tests', action='store_true')
    parser.add_argument('--candidate', type=int, default=163)
    args = parser.parse_args()
    sys.path.insert(0,str(ROOT/'tools/workshop'))
    from check_engine_contract172 import check as check_engine172
    check_engine172()
    config = ROOT / 'tools/local.json'
    settings = (
        json.loads(config.read_text(encoding='utf-8')) if config.is_file() else {}
    )
    game = args.game or (Path(settings['game']) if settings.get('game') else None)
    if args.tests and not game:
        parser.error('--tests requires --game <supported legitimate installation>')
    if game:
        game = game.resolve()
        require_game(game)
    resources = args.resources or (
        Path(settings['resources']) if settings.get('resources') else None
    )
    if args.target != 'adapter' and not resources and not args.local_assets:
        parser.error('Run tools/prepare_resources.py, then supply --resources <output>')
    if args.local_assets:
        resources = None
        if not game:
            parser.error(
                '--local-assets requires the supported --game for offline extraction/tests'
            )
    resource_receipt = None
    vcvars = (args.vcvars or find_vcvars()).resolve()
    if not vcvars.is_file():
        parser.error('--vcvars must name vcvars32.bat')
    output = args.output_dir.resolve()
    if (
        output == ROOT
        or ROOT.is_relative_to(output)
        or (game and (output == game or output.is_relative_to(game)))
    ):
        parser.error(
            'Build into a separate directory outside the game and repository ancestors'
        )
    if output.exists():
        parser.error('Use a new output directory; existing builds are preserved')
    output.mkdir(parents=True)
    snapshot = output / 'source'
    inputs = []
    for path in source_files(ROOT):
        relative = path.relative_to(ROOT)
        target = snapshot / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
        inputs.append({'path': relative.as_posix(), 'sha256': digest(path)})
    if resource_receipt:
        for row in resource_receipt['files']:
            target = snapshot / row['path']
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(resources / 'prepared' / row['path'], target)
    source_digest = hashlib.sha256(
        json.dumps(inputs, sort_keys=True).encode()
    ).hexdigest()
    manifest = {
        'schema': 1,
        'source_digest': source_digest,
        'source_files': inputs,
        'resources': resource_receipt,
        'local_assets': args.local_assets,
        'candidate': args.candidate,
        'game_changed': False,
        'installed': False,
        'game_launched': False,
        'historical_compatibility': {
            'status': 'skipped',
            'mandatory': False,
            'reason': 'No optional archive requested',
        },
        'live_validation': 'not performed; requires fresh separate authorization',
    }
    (output / 'build-inputs.json').write_text(
        json.dumps(manifest, indent=2) + '\n', encoding='utf-8'
    )
    environment = dict(
        os.environ,
        SHOGUN2_BUILD_OUTPUT=str(output),
        SHOGUN2_VCVARS=str(vcvars),
        SHOGUN2_ISOLATED_BUILD='1',
    )
    if args.local_assets:
        environment['SHOGUN2_LOCAL_ASSETS'] = '1'
    if game:
        environment['SHOGUN2_GAME_DIR'] = str(game)
    if resources:
        environment['SHOGUN2_PREPARED_MANIFEST'] = str(
            resources.resolve() / 'resource-manifest.json'
        )
    command = [
        sys.executable,
        '-B',
        str(snapshot / 'tools/native_build.py'),
        args.target,
        '--candidate',
        str(args.candidate),
    ]
    if args.release:
        command += ['--release']
    if args.tests:
        command += ['--tests']
    result = subprocess.run(command, cwd=snapshot, env=environment)
    manifest['native_build_returncode'] = result.returncode
    manifest['native_build_passed'] = result.returncode == 0
    (output / 'build-inputs.json').write_text(
        json.dumps(manifest, indent=2) + '\n', encoding='utf-8'
    )
    raise SystemExit(result.returncode)
