"""Build an isolated native Deck RTS candidate; never install, launch or publish."""
import argparse
import hashlib
import importlib.util
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
import adapt_runtime175
import check_abi174

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('game','bugfix','output','vcvars','lua','luac','abi-reference-dir'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--quick',action='store_true',help='Compile adapter and new fixtures only; not a distributable build')
    a=p.parse_args();game=a.game.resolve();out=a.output.resolve();require_game(game)
    if out.exists() or ROOT.is_relative_to(out) or out.is_relative_to(game):p.error('Use a new isolated output outside the game')
    abi=check_abi174.inspect(a.abi_reference_dir.resolve())
    out.mkdir(parents=True);snapshot=out/'source';snapshot.mkdir();logs=out/'logs';logs.mkdir()
    inputs=[]
    for source in source_files(ROOT):
        rel=source.relative_to(ROOT);target=snapshot/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        inputs.append(dict(path=rel.as_posix(),sha256=digest(source)))
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    adaptation=adapt_runtime175.generate(snapshot)
    (out/'adaptation.json').write_text(json.dumps(adaptation,indent=2)+'\n')
    metadata=json.loads((snapshot/'tools/data/deck175-native-bindings.json').read_text())
    stock=(game/'Empire.Retail.dll').read_bytes();other=a.bugfix.read_bytes()
    if hashlib.sha256(stock).hexdigest()!=metadata['stock_sha256'] or hashlib.sha256(other).hexdigest()!=metadata['reviewed_bugfix_sha256']:
        raise ValueError('Use exact owned input fixtures')
    from review_bindings175 import pe
    raw,_=pe(stock);raw_other,_=pe(other)
    for item in metadata['functions']:
        rv,size=item['rva'],item['size']
        if hashlib.sha256(stock[raw(rv):raw(rv)+size]).hexdigest()!=item['sha256'] or stock[raw(rv):raw(rv)+size]!=other[raw_other(rv):raw_other(rv)+size]:
            raise ValueError('Reviewed native body no longer matches fixtures')
    def run(name,command,env=None):
        print('Running '+name,flush=True)
        with (logs/(name+'.log')).open('w') as stream:
            result=subprocess.run(list(map(str,command)),cwd=snapshot,env=env,stdout=stream,stderr=subprocess.STDOUT)
        if result.returncode:
            print((logs/(name+'.log')).read_text(errors='replace')[-6000:]);raise SystemExit(result.returncode)
        print('PASS '+name,flush=True)
    # All current RTS references were verified to fall within the accepted
    # immutable regions. Review only the isolated source dependency extension;
    # keep the entire header, region hashes and compatibility admission exact.
    run('private-engine-contract',[sys.executable,'-B',ROOT/'tools/deck/extend_contract175.py',
        '--snapshot',snapshot,'--output',out/'engine-contract'])
    run('private-engine-contract-check',[sys.executable,'-B',snapshot/'tools/workshop/check_engine_contract172.py'])
    env=dict(os.environ,SHOGUN2_BUILD_OUTPUT=str(out/'components'),SHOGUN2_GAME_DIR=str(game),SHOGUN2_VCVARS=str(a.vcvars.resolve()))
    # Compiler commands execute from the snapshot so build-time transformations
    # cannot enter the normal maintained source or installed game directory.
    if a.quick:
        driver=out/'quick.py'
        driver.write_text('import sys,os\nfrom pathlib import Path\nsys.path.insert(0,'+repr(str(snapshot/'tools'))+')\nimport native_build as m\nm.VCVARS=Path(os.environ["SHOGUN2_VCVARS"])\nfor q in [m.BIN,m.OBJ,m.TEST,m.LOG]:q.mkdir(parents=True,exist_ok=True)\nm.adapter(False)\n')
        run('quick-adapter-compile',[sys.executable,'-B',driver],env)
    else:
        run('components',[sys.executable,'-B',snapshot/'tools/build.py','workshop','--candidate','175','--tests',
            '--local-assets','--game',game,'--output-dir',out/'components','--vcvars',a.vcvars.resolve()],env)
    # Actual native guard mapping and portable input/RTS fixtures are mandatory
    # even in a quick compile. None starts a Steam API or any game worker.
    from build174 import batch_script
    bin_dir=out/'fixture-bin';bin_dir.mkdir();common=['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/W4']
    commands=[]
    for source,libs,args in [('verify174.cpp',['user32.lib'],[]),('verify_rts175.cpp',[],[]),('verify_orders175.cpp',[],[]),
            ('verify_native175.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()])]:
        exe=bin_dir/(Path(source).stem+'.exe')
        commands.extend([[*common,snapshot/'tools/deck'/source,'/Fo'+str(bin_dir/(Path(source).stem+'.obj')),'/Fe'+str(exe),'/link',*libs],[exe,*args]])
    batch=out/'new-fixtures.cmd';batch.write_text(batch_script(a.vcvars.resolve(),commands))
    run('new-fixtures',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',batch])
    text=(logs/'new-fixtures.log').read_text(errors='replace')
    for marker in ('PASS 48 Deck simulation/ABI checks','PASS 22 RTS175 policy/queue checks','PASS 12 RTS175 native Halt boundary checks','PASS RTS175 normalized native guards:'):
        if marker not in text:raise ValueError('A native fixture did not complete: '+marker)
    if a.quick:
        (out/'verification.json').write_text(json.dumps(dict(candidate=175,quick_only=True,
            adapter_compiled=True,new_fixtures_passed=True,abi=abi,game_install_or_launch=False,not_for_distribution=True),indent=2)+'\n')
        return
    run('workshop-package',[sys.executable,'-B',snapshot/'tools/workshop/build_workshop165.py','--candidate','175',
        '--game',game,'--compat-engine',a.bugfix.resolve(),'--components',out/'components/bin','--output',out/'workshop',
        '--vcvars',a.vcvars.resolve(),'--lua',a.lua.resolve(),'--luac',a.luac.resolve()])
    result=dict(candidate=175,scope='First private native RTS integration; full controller support unfinished',
        native_inputs='Valve Steam Input006 through Proton; physical Deck required',abi=abi,
        new_simulation_ABI_checks=48,new_RTS_policy_queue_checks=22,native_Halt_boundary_checks=12,native_function_guards=len(metadata['functions']),
        source_adaptation=adaptation,game_install_or_launch=False,Steam_user_config_changed=False,
        recorder_debugger_started=False,actual_Deck_RTS_gameplay_verified=False,
        supported_increment=['camera pan/fast pan/rotate/zoom/pitch','native focused cursor','selection click/drag/confirm','native contextual order drag/commit/cancel','native halt'],
        pending=['remaining RTS selection shortcuts and run/walk toggle','native groups/wheels/ability UI','possession/QTE controller dispatch','campaign/deployment/naval contexts','real Deck timing/focus/suspend-resume validation'])
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print('Private RTS175 build complete; not installed, uploaded or hardware accepted.',flush=True)

if __name__=='__main__':main()
