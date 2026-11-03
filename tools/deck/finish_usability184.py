"""Finish a bridge-only correction after verified full181 component tests.

Keeps the failed build immutable. Reuse requires identical complete dependency
closures for all four components, every passing full component regression and
all earlier focused181 markers. Rebuilds and tests Workshop bridge separately.
Never installs, launches a game, starts Steam or publishes.
"""
from pathlib import Path
import argparse,hashlib,importlib.util,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'tools/deck')]
from game_packs import digest
import check_abi174

def module(path,name):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('from-build','output','game','bugfix','vcvars','lua','luac','abi-reference-dir'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();prior=a.from_build.resolve();out=a.output.resolve();game=a.game.resolve()
 if out.exists()or out.is_relative_to(game)or ROOT.is_relative_to(out):p.error('Use a new isolated output outside the game')
 results=json.loads((prior/'components/build-results.json').read_text());assert all(row['passed']for row in results['results'])
 for name,count in [('adapter-tests',100),('qte-tests',50),('hud-tests',8)]:assert next(x for x in results['results']if x['name']==name)['executables']==count
 required={'new-fixtures':['PASS 16 usability184','PASS 28 bindings183','PASS 30 controls181','PASS 33 frontend181','PASS 123 lifecycle181','PASS smooth zoom181: 2004 actual native float/ABI calls'],'native-context-lookup':['PASS 5 native context181','PASS native string183:'],'native-menu-Lua-policy':['PASS 75 frontend181']}
 for name,markers in required.items():
  text=(prior/'logs'/(name+'.log')).read_text()
  for marker in markers:assert marker in text,(name,marker)
 out.mkdir();snapshot=out/'source';shutil.copytree(prior/'source',snapshot);logs=out/'logs';logs.mkdir()
 path=snapshot/'native/work/autoload99-preview/integrated.h';s=path.read_text();assert s.count('#include "../../../deck/legend_resources184.h"')==1;s=s.replace('#include "../../../deck/legend_resources184.h"','#include "../../deck/legend_resources184.h"');path.write_text(s)
 for name in ['tools/deck/adapt_controls181.py','tools/deck/adapt_cache176.py','tools/deck/finish_usability184.py','tools/deck/verify_usability184.cpp','tools/deck/verify_legend184.cpp','tools/deck/verify_campaign_entry181.lua']:shutil.copyfile(ROOT/name,snapshot/name)
 before=module(prior/'source/tools/workshop/generate_engine_contract172.py','before184');after=module(snapshot/'tools/workshop/generate_engine_contract172.py','after184')
 entries=['native/outputs/shogun2-accuracy90-preview/src/adapter.cpp','native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp','native/outputs/shogun2-ammunition83-preview/hud-src/hud.cpp','tools/installer/prepare_local_assets159.cpp']
 left={q.relative_to(prior/'source').as_posix():digest(q)for q in before.source_closure(entries)};right={q.relative_to(snapshot).as_posix():digest(q)for q in after.source_closure(entries)}
 assert left==right,'A compiled gameplay dependency changed; full rebuild required'
 component=out/'components';shutil.copytree(prior/'components/bin',component/'bin');shutil.copyfile(prior/'components/build-results.json',component/'build-results.json')
 inherited=json.loads((prior/'components/build-inputs.json').read_text());inherited['component_inherited_from']=str(prior/'components');inherited['source_inputs_describe']=str(prior/'components/source')
 (component/'build-inputs.json').write_text(json.dumps(inherited,indent=2)+'\n')
 reuse=dict(source_build=str(prior),dependency_files_exact=len(left),component_regressions='100 adapter /50 QTE /8 HUD inherited after exact complete closure comparison',binaries={q.name:digest(q)for q in (component/'bin').iterdir()if q.suffix in('.dll','.exe')},base_results_sha256=digest(prior/'components/build-results.json'),not_fresh_component_tests=True)
 (out/'component-reuse.json').write_text(json.dumps(reuse,indent=2)+'\n')
 for name in required:shutil.copyfile(prior/'logs'/(name+'.log'),logs/(name+'.log'))
 def run(name,command,env=None):
  print('Running '+name,flush=True)
  with(logs/(name+'.log')).open('w')as log:r=subprocess.run(list(map(str,command)),cwd=snapshot,env=env,stdout=log,stderr=subprocess.STDOUT)
  if r.returncode:print((logs/(name+'.log')).read_text()[-6000:]);raise SystemExit(r.returncode)
  print('PASS '+name,flush=True)
 run('private-engine-contract',[sys.executable,'-B',ROOT/'tools/deck/extend_contract175.py','--snapshot',snapshot,'--output',out/'engine-contract','--cache-fix176','--context-controls181'])
 run('private-engine-contract-check',[sys.executable,'-B',snapshot/'tools/workshop/check_engine_contract172.py'])
 run('workshop-package',[sys.executable,'-B',snapshot/'tools/workshop/build_workshop165.py','--candidate','181','--game',game,'--compat-engine',a.bugfix.resolve(),'--components',component/'bin','--output',out/'workshop','--vcvars',a.vcvars.resolve(),'--lua',a.lua.resolve(),'--luac',a.luac.resolve()])
 cache=out/'menu-entry-cache';cache.mkdir();env=dict(os.environ,LOCALAPPDATA=str(cache),S2UP165_FIXTURE_GAME=str(game))
 run('compiled-menu-bootstrap',[a.lua.resolve(),snapshot/'tools/deck/verify_frontend_entry181.lua',out/'workshop/bootstrap-fixture.luac'],env)
 assert 'PASS 5 compiled frontend181'in(logs/'compiled-menu-bootstrap.log').read_text()
 run('actual-Lua-viewport',[a.lua.resolve(),snapshot/'tools/deck/verify_viewport181.lua',out/'workshop/bin/s2up-workshop165-fixture.dll'],env)
 assert 'PASS 6 actual float-Lua viewport'in(logs/'actual-Lua-viewport.log').read_text()
 value=dict(candidate=181,revision='usability-13',scope='Held L3, consistent RT menu select, uniform top legends, 30% mod HUD/QTE, native card proportions; base-game global UI reflow pending',abi=check_abi174.inspect(a.abi_reference_dir.resolve()),component_reuse=reuse,Steam_layout_revision=dict(major=2,minor=13),possession_context_checks=30,frontend_Lua_policy_checks=75,menu_worker_boundary_checks=33,button_binding_cases=28,native_string_argument_failure_cases=1,compiled_menu_bootstrap_checks=5,game_install_or_launch=False,recorder_debugger_started=False,uploaded=False,actual_Deck_gameplay_verified=False)
 (out/'verification.json').write_text(json.dumps(value,indent=2)+'\n');print('PASS bridge-only finishing build; not installed or uploaded.')
if __name__=='__main__':main()
