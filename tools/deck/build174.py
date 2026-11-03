"""Build an input-only Deck proof pack. Never install, launch a game or publish."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import time
import zipfile

ROOT=Path(__file__).resolve().parents[2]
for path in (ROOT/'tools',ROOT/'tools/installer',ROOT/'tools/deck'):
    sys.path.insert(0,str(path))
from build_frontend import source_files
from game_packs import Pack,require_game,digest
from audit_local_payload159 import resources
import native_defaults174
import check_abi174
import adapt_paths174


def batch_script(vcvars, commands):
    def quote(args):
        if any(any(c in str(x) for c in '\r\n%&|<>^!') for x in args):
            raise ValueError('Unsafe batch argument')
        return subprocess.list2cmdline(list(map(str,args)))
    # Native faults can be negative signed exit codes. `if errorlevel 1`
    # misses those and can turn an access violation into a false pass.
    guard='if not %errorlevel%==0 exit /b %errorlevel%'
    lines=['@echo off','call '+quote([vcvars]),guard]
    for args in commands:
        lines.extend([quote(args),guard])
    return '\r\n'.join(lines)+'\r\n'


def encode(data):
    return ',\n'.join('"'+''.join('\\%03d'%b for b in data[i:i+1024])+'"' for i in range(0,len(data),1024))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game',type=Path,required=True,help='Owned stock input for offline root/menu fixtures')
    p.add_argument('--output',type=Path,required=True)
    for name in ('vcvars','lua','luac','abi-reference-dir'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();game=a.game.resolve();out=a.output.resolve()
    require_game(game)
    if out.exists() or out==ROOT or ROOT.is_relative_to(out) or out.is_relative_to(game):
        p.error('Use a new isolated output directory outside the game')
    abi=check_abi174.inspect(a.abi_reference_dir.resolve())
    for tool in (a.vcvars,a.lua,a.luac):
        if not tool.is_file():p.error('Required compiler/interpreter missing')
    out.mkdir(parents=True);snapshot=out/'source';snapshot.mkdir();logs=out/'logs';logs.mkdir();bin_dir=out/'bin';bin_dir.mkdir()
    inputs=[];paths=set(source_files(ROOT))|set((ROOT/'native/deck').glob('*.ini'))
    for source in sorted(paths):
        relative=source.relative_to(ROOT);target=snapshot/relative;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        inputs.append({'path':relative.as_posix(),'sha256':digest(source)})
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    policy=adapt_paths174.generate(snapshot)
    (out/'path-policy-adaptation.json').write_text(json.dumps(policy,indent=2)+'\n')
    defaults=native_defaults174.write(out/'native-defaults')
    rc=out/'probe174.rc';rc.write_text(''.join(f'{i} RCDATA "{path.as_posix()}"\n' for i,path in [
        (201,out/'native-defaults/deck-actions174.vdf'),(202,out/'native-defaults/deck-native174.vdf'),
        (203,snapshot/'native/deck/controller174.ini')]))
    checks=[]
    def run(name,commands,env=None):
        script=logs/(name+'.cmd');log=logs/(name+'.log')
        script.write_text(batch_script(a.vcvars.resolve(),commands));start=time.monotonic();print('Running '+name,flush=True)
        with log.open('w') as f:
            result=subprocess.run([os.environ.get('COMSPEC','cmd.exe'),'/d','/c',script],cwd=out,env=env,stdout=f,stderr=subprocess.STDOUT)
        row={'name':name,'passed':result.returncode==0,'seconds':round(time.monotonic()-start,3),'log':str(log.relative_to(out))};checks.append(row)
        (out/'checks.json').write_text(json.dumps(checks,indent=2)+'\n')
        if result.returncode:raise ValueError('Check failed; inspect '+str(log))
        print('PASS '+name,flush=True)
    common=['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/W4']
    exe=bin_dir/'verify-deck174.exe';lifecycle=bin_dir/'verify-lifecycle174.exe';paths_exe=bin_dir/'verify-paths174.exe';dll=bin_dir/'s2up-deck-probe174.dll';fixture=bin_dir/'s2up-deck-probe174-fixture.dll';res=out/'probe174.res'
    run('compile',[
        ['rc','/nologo','/fo',res,rc],
        [*common,snapshot/'tools/deck/verify174.cpp','/Fo'+str(out/'verify174.obj'),'/Fe'+str(exe),'/link','user32.lib'],
        [*common,snapshot/'tools/deck/verify_lifecycle174.cpp',res,'/Fo'+str(out/'lifecycle174.obj'),'/Fe'+str(lifecycle),'/link','bcrypt.lib','user32.lib'],
        [*common,snapshot/'tools/deck/verify_paths174.cpp','/Fo'+str(out/'paths174.obj'),'/Fe'+str(paths_exe),'/link','bcrypt.lib','user32.lib'],
        [*common,'/LD',snapshot/'tools/deck/entry174.cpp',res,'/Fo'+str(out/'entry174.obj'),'/Fe'+str(dll),'/link','bcrypt.lib','user32.lib'],
        [*common,'/DS2UP165_LUA_FIXTURE','/LD',snapshot/'tools/deck/entry174.cpp',res,'/Fo'+str(out/'entry174-fixture.obj'),'/Fe'+str(fixture),'/link','bcrypt.lib','user32.lib']])
    run('simulation-and-native-ABI',[[exe]])
    if 'PASS 48 Deck simulation/ABI checks' not in (logs/'simulation-and-native-ABI.log').read_text():
        raise ValueError('Native verification did not finish every expected test')
    run('production-resource-report-boundaries',[[lifecycle,out/'resource-report-fixture']])
    if 'PASS 9 production resource/report/scoped-fault checks' not in (logs/'production-resource-report-boundaries.log').read_text():
        raise ValueError('Production resource verification did not complete')
    run('Proton-path-policy',[[paths_exe,out/'path-fixture',game]])
    if 'PASS diagnostic Proton path policy:' not in (logs/'Proton-path-policy.log').read_text():
        raise ValueError('Production path-policy verification did not complete')
    for report in (out/'resource-report-fixture').glob('owned*.json'):
        data=json.loads(report.read_text())
        if not data['diagnostic_only'] or data['gameplay_loaded'] or not data['hardware_validation_pending']:
            raise ValueError('Diagnostic report claims unsupported execution')
    expected={201:(out/'native-defaults/deck-actions174.vdf').read_bytes(),202:(out/'native-defaults/deck-native174.vdf').read_bytes(),203:(snapshot/'native/deck/controller174.ini').read_bytes()}
    for binary in (dll,fixture):
        leaves=resources(binary);actual={k[1]:v for k,v in leaves.items() if k[0]==10}
        if actual!=expected or any(k[0] not in (10,16,24) for k in leaves):raise ValueError('Probe resource closure differs')
    template=(snapshot/'tools/workshop/bootstrap165.lua').read_text()
    def wrapper(binary,name):
        text=template.replace('-- __NATIVE_PAYLOAD165__',encode(binary.read_bytes())).replace('__NATIVE_SHA165__',digest(binary))
        source=out/(name+'.lua');source.write_text(text,encoding='utf-8');target=out/(name+'.luac')
        subprocess.run([a.luac.resolve(),'-s','-o',target,source],check=True,cwd=out)
        if target.read_bytes()[:12]!=bytes.fromhex('1b4c75615100010404040400'):raise ValueError('Wrong float Lua compiler')
        return target
    delivered=wrapper(dll,'root174');fixture_script=wrapper(fixture,'root174-fixture')
    stock=Pack(game/'data/data.pack').read('ui/frontend ui/layout_scripts/root.luac',13739,
        '255b305cefb24a68cf6d2fc79d07efad7c0252acfd98ecd3ad10fa55c959cae8')
    reference=out/'private-owned-root.luac';reference.write_bytes(stock)
    cache=out/'isolated-lua-cache';cache.mkdir()
    env=dict(os.environ,LOCALAPPDATA=str(cache),S2UP165_FIXTURE_GAME=str(game))
    run('real-float-Lua-and-menu',[[a.lua.resolve(),snapshot/'tools/workshop/verify_bootstrap165.lua',reference,fixture_script,dll,fixture]],env)
    run('menu-failure-recovery',[[a.lua.resolve(),snapshot/'tools/workshop/verify_fallback167.lua',reference,fixture_script,game/'data/data.pack',fixture]],env)
    if list(cache.glob('*.deck174-report*.json')) or list(cache.glob('*.deck174')):
        raise ValueError('Non-game fixtures started the hardware worker')
    payload=delivered.read_bytes()
    if stock in payload:raise ValueError('Owned original script leaked into delivery')
    package=out/'diagnostic-package';package.mkdir()
    metadata={'test':174,'first_party_bootstrap':True,'native_sha256':digest(dll),
        'path_policy_revision':1,'diagnostic_revision':3,'diagnostic_only':True,'gameplay':False,'hardware_verification_pending':True,'not_for_publication':True}
    entries={'ui\\frontend ui\\layout_scripts\\root.luac':payload,'possession\\workshop165.json':json.dumps(metadata).encode(),
        'possession\\licenses\\LICENSE.txt':(ROOT/'LICENSE').read_bytes()}
    index=b''.join(struct.pack('<I',len(v))+k.encode()+b'\0' for k,v in entries.items())
    with (game/'data/data.pack').open('rb') as f:stamp=struct.unpack_from('<Q',f.read(32),24)[0]
    if not stamp:raise ValueError('Stock timestamp unavailable')
    pack=package/'shogun2_unit_possession_deck_probe174.pack'
    pack.write_bytes(struct.pack('<4s5IQ',b'PFH3',3,0,0,len(entries),len(index),stamp)+index+b''.join(entries.values()))
    parsed=Pack(pack)
    for name,data in entries.items():
        if parsed.read(name.replace('\\','/'),len(data),hashlib.sha256(data).hexdigest())!=data:raise ValueError('Pack roundtrip differs')
    report={'candidate':174,'path_policy_revision':1,'diagnostic_revision':3,'scope':'Native Deck input proof only; gameplay dispatch disabled',
        'pack':pack.name,'pack_sha256':digest(pack),'native_sha256':digest(dll),'native_bytes':dll.stat().st_size,
        'abi':abi,'checks':checks,'game_assets_or_SDK_shipped':False,'gameplay_components_loaded':False,
        'actual_SteamDeck_Proton_or_GamingMode_tested':False,'game_install_or_launch':False,
        'Steam_configuration_or_remote_repository_changed':False,'recording_debugger_started':False,
        'development_report_limit':'four reports <=16KiB each per content-addressed diagnostic build'}
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    shutil.copyfile(ROOT/'tools/deck/README.md',package/'README.md')
    shutil.copyfile(ROOT/'LICENSE',package/'LICENSE.txt')
    manifest=dict(report,simulation_checks=48,production_fixture_checks=9,
        report_basename='S2UP-Workshop165-'+digest(dll)+'.dll.deck174-report0.json',
        manifest_override_lifetime='Steam session per Valve documentation; restart Deck after diagnostic',
        automatic_manifest_restoration_verified=False,
        sampling_seconds=90,initialization_wait_seconds=15,binding_warmup_wait_seconds=15,
        measurement_starts_after_user_OK=True,onscreen_results=True,
        axes=['left_stick_x','left_stick_y','right_stick_x','right_stick_y',
              'left_pad_x','left_pad_y','right_pad_x','right_pad_y','L2','R2'],
        gyro_units='radians/second',accelerometer_units='metres/second squared',
        backend_states=['Idle','WaitingSteam','NoInterface','UnsupportedEnvironment',
            'InitializationFailed','ManifestRejected','WaitingDevice','AmbiguousDevice',
            'RemoteDevice','WaitingBindings','Ready','WrongThread','Failed'])
    (package/'test-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    selected=sorted(p for p in package.iterdir() if p.is_file())
    (package/'SHA256SUMS.txt').write_text(''.join(digest(p)+'  '+p.name+'\n' for p in selected))
    selected.append(package/'SHA256SUMS.txt')
    archive=out/'Shogun2-Deck-Native-Input-Test174.zip'
    with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for path in selected:z.write(path,path.name)
    with zipfile.ZipFile(archive) as z:
        if z.testzip() or set(z.namelist())!={p.name for p in selected}:
            raise ValueError('Diagnostic ZIP closure or CRC differs')
        for path in selected:
            if hashlib.sha256(z.read(path.name)).hexdigest()!=digest(path):
                raise ValueError('Diagnostic ZIP readback differs')
    (out/'archive-verification.json').write_text(json.dumps(dict(passed=True,
        file=archive.name,sha256=digest(archive),bytes=archive.stat().st_size,
        files=[p.name for p in selected],stock_script_or_SDK_or_gameplay_payload=False),indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
