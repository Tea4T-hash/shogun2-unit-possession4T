"""Build only the private Deck cache-lifetime correction; no install or upload."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'tools/deck')]
from build_frontend import source_files
from game_packs import digest,require_game
from build174 import batch_script
import adapt_runtime175
import adapt_cache176
import extend_contract175

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('previous','game','bugfix','output','vcvars','lua','luac'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();previous=a.previous.resolve();game=a.game.resolve();out=a.output.resolve()
    require_game(game)
    if out.exists() or ROOT.is_relative_to(out) or out.is_relative_to(game):p.error('Use a new isolated output outside the game')
    prior=json.loads((previous/'verification.json').read_text())
    workshop=json.loads((previous/'workshop/verification.json').read_text())
    if prior.get('candidate')!=175 or prior.get('quick_only') or len(workshop['fresh_checks'])!=16 or not all(r['passed'] for r in workshop['fresh_checks']):
        raise ValueError('Use the completed RTS175 build')
    out.mkdir(parents=True);snapshot=out/'source';snapshot.mkdir();logs=out/'logs';logs.mkdir()
    inputs=[]
    for source in source_files(ROOT):
        rel=source.relative_to(ROOT);target=snapshot/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        inputs.append(dict(path=rel.as_posix(),sha256=digest(source)))
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    adaptation=adapt_runtime175.generate(snapshot);cache=adapt_cache176.generate(snapshot)
    (out/'adaptation.json').write_text(json.dumps(dict(RTS175=adaptation,cache176=cache),indent=2)+'\n')
    contract=extend_contract175.extend(snapshot,cache_fix176=True)
    (snapshot/'tools/data/engine-contract172.json').write_text(json.dumps(contract,indent=2)+'\n')
    (out/'engine-contract-review.json').write_text(json.dumps(dict(
        RTS=contract['deck175_dependency_review'],cache=contract['deck176_cache_lifetime_review']),indent=2)+'\n')

    def run(name,command):
        print('Running '+name,flush=True)
        with (logs/(name+'.log')).open('w') as stream:
            result=subprocess.run(list(map(str,command)),cwd=snapshot,stdout=stream,stderr=subprocess.STDOUT)
        if result.returncode:
            print((logs/(name+'.log')).read_text(errors='replace')[-5000:]);raise SystemExit(result.returncode)
        print('PASS '+name,flush=True)
    run('private-engine-contract-check',[sys.executable,'-B',snapshot/'tools/workshop/check_engine_contract172.py'])
    # Prove reuse against actual source text used for the compiled components,
    # including private Deck headers and generated extractor path integration.
    old_inputs=json.loads((previous/'components/build-inputs.json').read_text())
    reused=[]
    for row in old_inputs['source_files']:
        name=row['path']
        if name.startswith(('native/outputs/','native/common/','native/deck/','tools/installer/')) and Path(name).suffix in ('.cpp','.c','.h','.def','.rc','.ini'):
            if digest(snapshot/name)!=row['sha256']:raise ValueError('Component dependency changed: '+name)
            reused.append(row)
    for name in ('contract_id','regions','sections','generated_header_sha256'):
        before=json.loads((previous/'source/tools/data/engine-contract172.json').read_text())[name]
        if contract[name]!=before:raise ValueError('Engine admission changed')
    components=out/'retained-components';(components/'bin').mkdir(parents=True)
    for name,expected in workshop['components'].items():
        source=previous/'components/bin'/name
        if digest(source)!=expected:raise ValueError('Retained component differs: '+name)
        shutil.copyfile(source,components/'bin'/name)
        if digest(components/'bin'/name)!=expected:raise ValueError('Retained component copy differs')
    for name in ('hud-assets159.s2fp','qte-assets159.s2fp'):
        shutil.copyfile(previous/'components/bin'/name,components/'bin'/name)
    for name in ('build-inputs.json','build-results.json'):
        shutil.copyfile(previous/'components'/name,components/name)
    (out/'component-reuse.json').write_text(json.dumps(dict(passed=True,previous=str(previous),
        dependencies=reused,component_hashes=workshop['components'],components_recompiled=False,
        inherited_adapter_QTE_HUD_tests=[100,50,8]),indent=2)+'\n')

    fixtures=out/'cache-fixtures';fixtures.mkdir();common=['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/W4',
        '/DNOMINMAX','/DS2FP_LOCAL_ASSETS','/DPOSSESSION_WORKSHOP_BUILD',
        '/I'+str(previous/'workshop/source/native/work/autoload99-preview')]
    libs=['bcrypt.lib','shell32.lib','user32.lib','winmm.lib'];commands=[]
    for label,source,options in [('old',ROOT/'tools/deck/verify_cache176.cpp',['/DS2UP_CACHE_EXPECT_OLD']),
                                  ('corrected',snapshot/'tools/deck/verify_cache176.cpp',[])]:
        exe=fixtures/(label+'.exe')
        commands.extend([[*common,*options,source,'/Fo'+str(fixtures/(label+'.obj')),'/Fe'+str(exe),'/link',*libs],
            [exe,components/'bin/hud-assets159.s2fp',components/'bin/qte-assets159.s2fp',fixtures/(label+'-data')]])
    batch=out/'cache-fixtures.cmd';batch.write_text(batch_script(a.vcvars.resolve(),commands))
    run('cache-old-and-corrected',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',batch])
    text=(logs/'cache-old-and-corrected.log').read_text(errors='replace')
    for marker in ('PASS 8 cache176 controlled OLD-path checks','PASS 8 cache176 corrected-path checks'):
        if marker not in text:raise ValueError('Cache fixture did not finish: '+marker)
    run('workshop-package',[sys.executable,'-B',snapshot/'tools/workshop/build_workshop165.py','--candidate','176',
        '--game',game,'--compat-engine',a.bugfix.resolve(),'--components',components/'bin','--output',out/'workshop',
        '--vcvars',a.vcvars.resolve(),'--lua',a.lua.resolve(),'--luac',a.luac.resolve()])
    value=dict(candidate=176,scope='Private cache startup correction; exact RTS175 gameplay components retained',
        cache_lifetime_fix_verified=True,old_path_controlled_checks=8,corrected_path_checks=8,
        actual_Proton_execution=False,actual_Deck_startup_verified=False,
        component_build_reused=str(previous/'components'),component_dependencies_exact=len(reused),
        component_hashes=workshop['components'],component_regression_tests_inherited_from175=[100,50,8],
        native_engine_admission_unchanged=True,cache_source_reverse_comparison=cache,
        engine_bytes_fonts_artwork_voice_shipped=False,installed=False,uploaded=False,
        game_started=False,recorder_debugger_started=False,normal_public_mod_changed=False,
        limitations=prior['pending'])
    (out/'verification.json').write_text(json.dumps(value,indent=2)+'\n')
    print('Private cache176 build complete; no install, upload or actual Deck verification.',flush=True)

if __name__=='__main__':main()
