"""Build the complete Unit Possession Workshop pack on a GitHub Windows runner.

Compiles the four components (possession, HUD, QTE, local-asset preparer), the Workshop
bridge that embeds them, a float Lua 5.1 compiler, the bootstrap script, and finally
shogun2_unit_possession_workshop165.pack. It needs no game files and installs nothing.
Mirrors tools/native_build.py and tools/workshop/build_workshop165.py (candidate 173),
without the fixture tests that need an owned game.
"""
import hashlib, io, json, struct, subprocess, tarfile, urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'gh-out'; OBJ = OUT / 'obj'; BIN = OUT / 'bin'
for d in (OBJ, BIN): d.mkdir(parents=True, exist_ok=True)
CAND = 173
NAMES = ['possession-v46.dll', 'possession-hud.exe', 'qte-playable-v79.dll', 'prepare-local-assets159.exe']

def sh(cmd, cwd=ROOT):
    cmd = [str(c) for c in cmd]; print('>', ' '.join(cmd), flush=True)
    subprocess.run(cmd, cwd=cwd, check=True)

def digest(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()

COMMON = ['cl', '/nologo', '/std:c++17', '/EHsc', '/O2', '/MT', '/W4', '/DS2FP_LOCAL_ASSETS', '/DNOMINMAX']
N = ROOT / 'native/outputs'

def cpp(src, target, extra=(), libs=(), opts=()):
    sh([*COMMON, *opts, src, *extra, '/Fo' + str(OBJ / (Path(target).stem + '.obj')), '/Fe' + str(target), '/link', *libs])

# 1. MinHook
V = N / 'shogun2-accuracy90-preview/vendor'; hooks = []
for f in ['buffer.c', 'hook.c', 'trampoline.c', 'hde/hde32.c']:
    o = OBJ / (Path(f).stem + '.obj')
    sh(['cl', '/nologo', '/O2', '/MT', '/W3', '/I' + str(V / 'include'), '/c', V / 'src' / f, '/Fo' + str(o)]); hooks.append(o)

# 2. The four components
base = ['bcrypt.lib', 'user32.lib', 'gdi32.lib']
cpp(N / 'shogun2-accuracy90-preview/src/adapter.cpp', BIN / NAMES[0], hooks, base + ['d3d11.lib', 'd3d9.lib', 'dxgi.lib', 'd3dcompiler.lib'], ['/LD'])
cpp(N / 'shogun2-ammunition83-preview/hud-src/hud.cpp', BIN / NAMES[1], (), base + ['gdiplus.lib', 'ole32.lib', 'winmm.lib'])
cpp(N / 'shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp', BIN / NAMES[2], hooks, base + ['gdiplus.lib', 'ole32.lib'], ['/LD'])
cpp(ROOT / 'tools/installer/prepare_local_assets159.cpp', BIN / NAMES[3], (),
    ['bcrypt.lib', 'ole32.lib', 'mfplat.lib', 'mfreadwrite.lib', 'mfuuid.lib', 'shell32.lib', 'delayimp.lib',
     '/DELAYLOAD:mfplat.dll', '/DELAYLOAD:mfreadwrite.dll'], ['/DUNICODE', '/D_UNICODE'])

# 3. Workshop bridge with the components embedded and their fingerprints compiled in
identity = hashlib.sha256(json.dumps({n: digest(BIN / n) for n in NAMES}, sort_keys=True).encode()).hexdigest()[:16]
header = '#pragma once\n' + ''.join(f'static const char* EMBEDDED_{l}_SHA="{digest(BIN / n)}";\n'
                                    for l, n in zip(['ADAPTER', 'HUD', 'QTE', 'PREPARER'], NAMES))
header += f'static const char* EMBEDDED_PACK_SHA="";\nstatic const wchar_t* EMBEDDED_BUILD=L"{CAND}-{identity}";\n'
header += f'static constexpr unsigned EMBEDDED_WORKSHOP_TEST={CAND};\n'
(ROOT / 'native/work/autoload99-preview/embedded_hashes.h').write_text(header)
rc = OUT / 'payload165.rc'
rc.write_text(''.join(f'{i} RCDATA "{(BIN / n).as_posix()}"\n' for i, n in zip([101, 102, 103, 104], NAMES)))
res = OUT / 'payload165.res'; sh(['rc', '/nologo', '/fo', res, rc])
dll = BIN / 's2up-workshop165.dll'
sh(['cl', '/nologo', '/std:c++17', '/EHsc', '/O2', '/MT', '/W4', '/DNOMINMAX', '/DS2FP_LOCAL_ASSETS', '/DPOSSESSION_WORKSHOP_BUILD',
    '/LD', ROOT / 'tools/workshop/entry165.cpp', res, '/Fo' + str(OBJ / 'entry165.obj'), '/Fe' + str(dll),
    '/link', 'bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib'])

# 4. Float Lua 5.1 compiler (official source, fingerprint-checked)
data = urllib.request.urlopen('https://www.lua.org/ftp/lua-5.1.5.tar.gz', timeout=60).read()
assert hashlib.sha256(data).hexdigest() == '2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333', 'Lua source fingerprint'
with tarfile.open(fileobj=io.BytesIO(data)) as t: t.extractall(OUT / 'lua', filter='data')
src = OUT / 'lua/lua-5.1.5/src'; conf = src / 'luaconf.h'; text = conf.read_text()
assert '#define LUA_NUMBER_DOUBLE' in text and '#define LUA_NUMBER\tdouble' in text
conf.write_text(text.replace('#define LUA_NUMBER_DOUBLE', '#define LUA_NUMBER_FLOAT').replace('#define LUA_NUMBER\tdouble', '#define LUA_NUMBER\tfloat')
                .replace('"%lf"', '"%f"').replace('"%.14g"', '"%.7g"'))
SOURCES = ('lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c '
           'ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c').split()
sh(['cl', '/nologo', '/O2', '/MT', '/D_CRT_SECURE_NO_WARNINGS', '/c', *SOURCES], cwd=src)
sh(['lib', '/nologo', '/out:lua51.lib', *[s[:-2] + '.obj' for s in SOURCES]], cwd=src)
sh(['cl', '/nologo', '/O2', '/MT', '/D_CRT_SECURE_NO_WARNINGS', 'luac.c', 'print.c', 'lua51.lib', '/Feluac51.exe'], cwd=src)

# 5. Bootstrap script carrying the bridge, compiled to root.luac
payload = dll.read_bytes()
encoded = ',\n'.join('"' + ''.join('\\%03d' % b for b in payload[i:i + 1024]) + '"' for i in range(0, len(payload), 1024))
lua = OUT / 'bootstrap-production.lua'
lua.write_text((ROOT / 'tools/workshop/bootstrap165.lua').read_text()
               .replace('__NATIVE_SHA165__', digest(dll)).replace('-- __NATIVE_PAYLOAD165__', encoded))
luac = OUT / 'root.luac'; sh([src / 'luac51.exe', '-s', '-o', luac, lua], cwd=OUT)
assert luac.read_bytes()[:12] == bytes.fromhex('1b4c75615100010404040400'), 'Matching float Lua compiler required'

# 6. The Workshop pack (same layout as the released one)
meta = json.dumps({'test': CAND, 'native_sha256': digest(dll), 'first_party_bootstrap': True, 'resources': 'prepared from owned game',
                   'chooser': False, 'default_view': 'third-person', 'routine_disk_logging': False, 'external_recorder_debugger': False,
                   'gameplay_components': {n: digest(BIN / n) for n in NAMES}}).encode()
entries = {'ui\\frontend ui\\layout_scripts\\root.luac': luac.read_bytes(), 'possession\\workshop165.json': meta}
for p, n in [(ROOT / 'LICENSE', 'LICENSE.txt'), (ROOT / 'THIRD_PARTY_NOTICES.md', 'THIRD_PARTY_NOTICES.md'),
             (N / 'shogun2-possession-cleanup-v46/vendor/LICENSE.txt', 'MinHook-LICENSE.txt')]:
    entries['possession\\licenses\\' + n] = p.read_bytes()
entries['possession\\PLAYING.md'] = (ROOT / 'tools/workshop/PLAYING173.md').read_bytes()
index = b''.join(struct.pack('<I', len(v)) + k.encode() + b'\0' for k, v in entries.items())
STAMP = 0x01d98509c46b5c1c  # timestamp from the released Workshop pack header
pack = OUT / 'package/shogun2_unit_possession_workshop165.pack'; pack.parent.mkdir(exist_ok=True)
pack.write_bytes(struct.pack('<4s5IQ', b'PFH3', 3, 0, 0, len(entries), len(index), STAMP) + index + b''.join(entries.values()))
(OUT / 'package/fingerprints.json').write_text(json.dumps({'pack': digest(pack), 'bridge': digest(dll),
    'components': {n: digest(BIN / n) for n in NAMES}}, indent=2))
print(open(OUT / 'package/fingerprints.json').read())
