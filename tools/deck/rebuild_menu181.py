"""Rebuild only the private menu bridge after exact gameplay-closure comparison."""
import argparse,hashlib,importlib.util,json,os,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'tools/deck'),str(ROOT/'tools')]
from game_packs import digest
from build174 import batch_script
import adapt_frontend181
import native_defaults181

def module(path,name):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ['from-build','output','game','bugfix','vcvars','lua','luac']:p.add_argument('--'+n,type=Path,required=True)
 p.add_argument('--rebindings',action='store_true',help='Bridge-only finishing changes after completed full rebinding build; all four components exact')
 p.add_argument('--smooth-zoom',action='store_true',help='Bridge-only verified native float campaign zoom; all gameplay binaries exact')
 p.add_argument('--camera-legend',action='store_true',help='Reviewed campaign RS turn/zoom and top/no-campaign legend correction; rebuild HUD and adapter labels')
 p.add_argument('--controller-labels',action='store_true',help='Allow display-only controller metadata changes and rebuild the adapter consumer')
 a=p.parse_args();prior=a.from_build.resolve();out=a.output.resolve();game=a.game.resolve()
 old=json.loads((prior/'verification.json').read_text());w=json.loads((prior/'workshop/verification.json').read_text())
 if a.rebindings and(a.smooth_zoom or a.controller_labels or a.camera_legend or old.get('revision')!='button-bindings-11'):raise ValueError('Use completed full button rebinding build with unchanged component closure')
 if a.smooth_zoom and(a.controller_labels or a.camera_legend or old.get('revision')!='campaign-turn-zoom-9'):raise ValueError('Smooth zoom uses exact accepted revision9 with unchanged component binaries')
 if a.camera_legend and(not a.controller_labels or old.get('revision')!='controller-lifecycle-8'):raise ValueError('Use exact completed revision8 plus --controller-labels for the reviewed UI correction')
 if out.exists()or out.is_relative_to(game)or ROOT.is_relative_to(out):p.error('Use a new isolated D: output outside game')
 if old.get('candidate')!=181 or old.get('quick_only')or not all(x['passed']for x in w['fresh_checks']):raise ValueError('Use a completed181 base')
 out.mkdir(parents=True);snapshot=out/'source';shutil.copytree(prior/'source',snapshot);logs=out/'logs';logs.mkdir()
 before=module(prior/'source/tools/deck/adapt_frontend181.py','previous_frontend181').injection(snapshot,'menu')
 changed=['tools/deck/adapt_frontend181.py','native/deck/viewport181.h','tools/deck/verify_viewport181.lua','native/deck/frontend181.h','native/deck/frontend181.lua','native/deck/navigation181.lua','tools/deck/verify_frontend181.cpp','tools/deck/verify_frontend181.lua','tools/deck/verify_frontend_entry181.lua','tools/deck/verify_campaign_entry181.lua','tools/deck/verify_campaign181.cpp','tools/deck/verify_context_backend181.cpp','tools/deck/check_campaign181.py','tools/deck/package181.py','tools/deck/rebuild_menu181.py','tools/deck/native_defaults181.py','tests/test_deck181.py','docs/development/SteamDeck181.md']
 if a.rebindings:changed+=['native/deck/bindings183.h','native/deck/campaign181.h','native/deck/zoom181.h','native/deck/zoom181_generated.h','tools/data/deck181-native-zoom.json','tools/deck/verify_zoom181.cpp','tools/deck/verify_bindings183.cpp','tools/deck/verify_lifecycle181.cpp']
 if a.smooth_zoom:changed+=['native/deck/campaign181.h','native/deck/zoom181.h','native/deck/zoom181_generated.h','tools/data/deck181-native-zoom.json','tools/deck/verify_zoom181.cpp','tools/deck/verify_lifecycle181.cpp']
 if a.camera_legend:changed+=['native/deck/campaign181.h','native/deck/legend181.h','tools/deck/verify_lifecycle181.cpp']
 for name in changed:shutil.copyfile(ROOT/name,snapshot/name)
 bootstrap=snapshot/'tools/workshop/bootstrap165.lua';text=bootstrap.read_text();assert text.count(before)==1;bootstrap.write_text(text.replace(before,adapt_frontend181.injection(snapshot,'menu')),encoding='utf-8')
 label_review=None
 if a.controller_labels:
  import re
  header=snapshot/'native/deck/defaults175_generated.h'
  prior_defaults={name:bytes(int(n)for n in data.split(',')[:-1]).decode('utf-8')for name,data in re.findall(r'inline constexpr char (\w+)\[\]=\{([0-9,]+)\};',header.read_text())}
  if set(prior_defaults)!={'manifest','bindings','settings'}:raise ValueError('Unexpected embedded default closure')
  manifest,bindings=native_defaults181.values();values=dict(manifest=manifest,bindings=bindings,settings=prior_defaults['settings'])
  for name in ('manifest','bindings'):
   if native_defaults181.binding_semantics(values[name])!=native_defaults181.binding_semantics(prior_defaults[name]):raise ValueError('Controller bindings changed, not just labels')
  header.write_text('#pragma once\nnamespace defaults175 {\n'+''.join('inline constexpr char '+n+'[]='+'{'+','.join(str(b)for b in v.encode('utf-8'))+',0};\n'for n,v in values.items())+'}\n',encoding='utf-8')
  label_review=dict(display_metadata_only=True,actions_presets_physical_bindings_exact=True,settings_exact=True,localized_tokens=sum(len(native_defaults181.labels(c))+1 for c in native_defaults181.SETS),prior_generated_sha256=digest(prior/'source/native/deck/defaults175_generated.h'),new_generated_sha256=digest(header))
 previous=module(prior/'source/tools/workshop/generate_engine_contract172.py','previous181');current=module(snapshot/'tools/workshop/generate_engine_contract172.py','current181')
 entries=['native/outputs/shogun2-accuracy90-preview/src/adapter.cpp','native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp','native/outputs/shogun2-ammunition83-preview/hud-src/hud.cpp','tools/installer/prepare_local_assets159.cpp']
 paths=current.source_closure(entries);old_paths=previous.source_closure(entries)
 assert {x.relative_to(snapshot)for x in paths}=={x.relative_to(prior/'source')for x in old_paths}
 for path in paths:
  if digest(path)!=digest(prior/'source'/path.relative_to(snapshot)):
   name=path.relative_to(snapshot).as_posix()
   if a.rebindings and name=='native/deck/rts_schema178.h':
    original=(prior/'source'/name).read_text();clean='\n'.join(line.rstrip()if not line.strip()else line for line in original.split('\n'))
    if path.read_text()!=clean:raise ValueError('Only blank-line whitespace cleanup permitted')
    continue
   if a.controller_labels and name=='native/deck/defaults175_generated.h':continue
   if a.camera_legend and name=='native/deck/legend181.h':
    original=(prior/'source'/name).read_text();assert original.count('client.bottom-int(h)-4')==1
    if path.read_text()!=original.replace('client.bottom-int(h)-4','4'):raise ValueError('Unreviewed HUD source change')
    continue
   raise ValueError('Gameplay dependency changed: '+str(path))
 reuse=dict(previous_build=str(prior),byte_exact_dependency_files=len(paths),component_hashes=w['components'],original_component_checks='100 adapter /50 QTE /8 HUD plus resources, inherited only after closure comparison')
 if label_review:reuse.update(controller_labels=label_review,adapter_rebuilt=True,byte_exact_dependency_files=len(paths)-1,reviewed_dependency_files=len(paths),component_hashes={n:sha for n,sha in w['components'].items()if n!='possession-v46.dll'},original_component_checks='Prior full-4 regression evidence; not rerun here. Adapter display metadata rebuilt; other three components exact.')
 reuse['byte_exact_dependency_files']=sum(digest(q)==digest(prior/'source'/q.relative_to(snapshot))for q in paths)
 if a.camera_legend:reuse.update(hud_rebuilt=True,component_hashes={n:sha for n,sha in w['components'].items()if n in ('qte-playable-v79.dll','prepare-local-assets159.exe')},HUD_layout_only=True,original_component_checks='Revision8 full100 adapter/50QTE regression evidence retained after exact dependency comparison. Adapter labels rebuilt; HUD position change plus8 HUD tests fresh; QTE/extractor bytes exact.')
 if a.rebindings:reuse['blank_line_only_source_cleanup']=['native/deck/rts_schema178.h']
 (out/'component-reuse.json').write_text(json.dumps(reuse,indent=2)+'\n')
 def run(name,command,env=None,cwd=None):
  print('Running '+name,flush=True)
  with(logs/(name+'.log')).open('w')as log:r=subprocess.run(list(map(str,command)),cwd=cwd or snapshot,env=env,stdout=log,stderr=subprocess.STDOUT)
  if r.returncode:print((logs/(name+'.log')).read_text(errors='replace')[-4500:]);raise SystemExit(r.returncode)
  print('PASS '+name,flush=True)
 run('private-engine-contract',[sys.executable,'-B',ROOT/'tools/deck/extend_contract175.py','--snapshot',snapshot,'--output',out/'engine-contract','--cache-fix176','--context-controls181'])
 run('private-engine-contract-check',[sys.executable,'-B',snapshot/'tools/workshop/check_engine_contract172.py'])
 from build181 import check_abi174
 abi=check_abi174.inspect(Path(r'D:\Shogun2Deck\github-recovery-20261030-1\dependency-check\valve'))
 commands=[];bin_dir=out/'fixture-bin';bin_dir.mkdir()
 for source,libs,args in [('verify_frontend181.cpp',['user32.lib','gdiplus.lib','bcrypt.lib'],[]),('verify_controls181.cpp',['user32.lib'],[]),('verify_lifecycle181.cpp',['user32.lib'],[]),('verify_handoff181.cpp',[],[]),('verify_binding_profiles182.cpp',['user32.lib'],[]),('verify_navigation182.cpp',['user32.lib'],[]),('verify_native175.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()])]:
  exe=bin_dir/(Path(source).stem+'.exe');commands.extend([['cl','/nologo','/std:c++17','/EHsc','/O2','/MT',snapshot/'tools/deck'/source,'/Fo'+str(bin_dir/(Path(source).stem+'.obj')),'/Fe'+str(exe),'/link',*libs],[exe,*args]])
 if a.smooth_zoom or a.rebindings:
  source=snapshot/'tools/deck/verify_zoom181.cpp';exe=bin_dir/'zoom181-check.exe';commands.extend([['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/DNOMINMAX',source,'/Fo'+str(bin_dir/'zoom.obj'),'/Fe'+str(exe),'/link','bcrypt.lib','user32.lib'],[exe,game/'Empire.Retail.dll',a.bugfix.resolve()]])
 if a.rebindings:
  source=snapshot/'tools/deck/verify_bindings183.cpp';exe=bin_dir/'bindings183-check.exe';commands.extend([['cl','/nologo','/std:c++17','/EHsc','/O2','/MT',source,'/Fo'+str(bin_dir/'bindings183.obj'),'/Fe'+str(exe),'/link','user32.lib'],[exe]])
 cmd=out/'native-menu.cmd';cmd.write_text(batch_script(a.vcvars.resolve(),commands))
 run('new-fixtures',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',cmd])
 text=(logs/'new-fixtures.log').read_text()
 for marker in ['PASS 33 frontend181','PASS 30 controls181','PASS 123 lifecycle181','PASS 13 handoff181','PASS 7 binding_profiles182','PASS 7 navigation182','PASS RTS175 normalized native guards:']:assert marker in text,marker
 context_dll=bin_dir/'context-lookup181.dll';context_cmd=out/'context-lookup.cmd';context_cmd.write_text(batch_script(a.vcvars.resolve(),[['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/LD',snapshot/'tools/deck/verify_context_lookup181.cpp','/Fo'+str(bin_dir/'context.obj'),'/Fe'+str(context_dll)]]))
 run('context-lookup-compile',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',context_cmd]);run('native-context-lookup',[a.lua.resolve(),snapshot/'tools/deck/verify_context_lookup181.lua',context_dll],dict(os.environ,S2UP165_FIXTURE_GAME=str(game)))
 if a.smooth_zoom or a.rebindings:assert 'PASS smooth zoom181: 2004 actual native float/ABI calls' in text
 assert 'PASS 5 native context181' in(logs/'native-context-lookup.log').read_text()
 run('native-menu-Lua-policy',[a.lua.resolve(),snapshot/'tools/deck/verify_frontend181.lua',snapshot/'native/deck/frontend181.lua',snapshot/'native/deck/navigation181.lua',snapshot/'native/deck/frontend179.lua',snapshot/'native/deck/navigation179.lua']);assert ('PASS 72 frontend181'if a.rebindings else'PASS 66 frontend181') in (logs/'native-menu-Lua-policy.log').read_text()
 components=prior/'components/bin'
 if a.controller_labels:
  component_out=out/'components';components=component_out/'bin';components.mkdir(parents=True)
  for name,sha in w['components'].items():
   source=prior/'components/bin'/name
   if digest(source)!=sha:raise ValueError('Prior component differs: '+name)
   if name not in (('possession-v46.dll','possession-hud.exe')if a.camera_legend else('possession-v46.dll',)):shutil.copyfile(source,components/name)
  env=dict(os.environ,SHOGUN2_BUILD_OUTPUT=str(component_out),SHOGUN2_VCVARS=str(a.vcvars.resolve()))
  driver=out/'adapter-labels.py'
  driver.write_text('import sys,os,json,hashlib,shutil\nfrom pathlib import Path\nsys.path.insert(0,'+repr(str(snapshot/'tools'))+')\nimport native_build as m\nm.VCVARS=Path(os.environ["SHOGUN2_VCVARS"])\nfor p in (m.BIN,m.OBJ,m.TEST,m.LOG):p.mkdir(parents=True,exist_ok=True)\nm.adapter(False)\n'+('\nfor name in [\"hud-assets159.s2fp\",\"qte-assets159.s2fp\"]:\n source=Path('+repr(str(prior/'components/local-assets'))+')/name\n expected=m.sha(source)\n for target in [m.BIN/name,m.TEST/name]:\n  shutil.copyfile(source,target)\n  assert m.sha(target)==expected\nm.hud(True)\n'if a.camera_legend else'')+'binaries=[dict(file=str(p.relative_to(m.OUT)),bytes=p.stat().st_size,sha256=m.sha(p))for p in m.BIN.iterdir()if p.suffix in (".exe",".dll")]\n(m.OUT/"build-results.json").write_text(json.dumps(dict(candidate=181,results=m.receipts,binaries=binaries,display_metadata_rebuild=True,game_installed_or_launched=False),indent=2)+"\\n")\n')
  run('adapter-labels-compile',[sys.executable,'-B',driver],env)
  inherited=json.loads((prior/'components/build-inputs.json').read_text())
  for row in inherited['source_files']:row['sha256']=digest(snapshot/row['path'])
  inherited['source_digest']=hashlib.sha256(json.dumps(inherited['source_files'],sort_keys=True).encode()).hexdigest()
  inherited.update(controller_label_review=label_review,reused_components={n:sha for n,sha in w['components'].items()if n not in (('possession-v46.dll','possession-hud.exe')if a.camera_legend else('possession-v46.dll',))},adapter_compiled=True,hud_compiled=a.camera_legend,game_changed=False,installed=False,game_launched=False)
  (component_out/'build-inputs.json').write_text(json.dumps(inherited,indent=2)+'\n')
  for name,sha in w['components'].items():
   if name not in (('possession-v46.dll','possession-hud.exe')if a.camera_legend else('possession-v46.dll',))and digest(components/name)!=sha:raise ValueError('Unchanged component drift: '+name)
  fixture=out/'verify-label-controls.exe';batch=out/'label-controls.cmd'
  batch.write_text(batch_script(a.vcvars.resolve(),[['cl','/nologo','/std:c++17','/EHsc','/O2','/MT',snapshot/'tools/deck/verify_controls181.cpp','/Fo'+str(out/'controls.obj'),'/Fe'+str(fixture),'/link','user32.lib'],[fixture]]))
  run('context-controls',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',batch]);assert 'PASS 30 controls181' in (logs/'context-controls.log').read_text()
 run('workshop-package',[sys.executable,'-B',snapshot/'tools/workshop/build_workshop165.py','--candidate','181','--game',game,'--compat-engine',a.bugfix.resolve(),'--components',components,'--output',out/'workshop','--vcvars',a.vcvars.resolve(),'--lua',a.lua.resolve(),'--luac',a.luac.resolve()])
 cache=out/'menu-entry-cache';cache.mkdir();env=dict(os.environ,LOCALAPPDATA=str(cache),S2UP165_FIXTURE_GAME=str(game))
 run('compiled-menu-bootstrap',[a.lua.resolve(),snapshot/'tools/deck/verify_frontend_entry181.lua',out/'workshop/bootstrap-fixture.luac'],env);assert 'PASS 5 compiled frontend181' in (logs/'compiled-menu-bootstrap.log').read_text()
 run('actual-Lua-viewport',[a.lua.resolve(),snapshot/'tools/deck/verify_viewport181.lua',out/'workshop/bin/s2up-workshop165-fixture.dll'],env);assert 'PASS 6 actual float-Lua viewport' in (logs/'actual-Lua-viewport.log').read_text()
 result=dict(old);result.update(revision='button-bindings-11'if a.rebindings else'campaign-smooth-zoom-10'if a.smooth_zoom else'campaign-turn-zoom-9'if a.camera_legend else'controller-labels-3'if a.controller_labels else'ui-boundary-4',frontend_Lua_policy_checks=72 if a.rebindings else 66,menu_worker_boundary_checks=33,compiled_menu_bootstrap_checks=5,actual_native_viewport_export_cases=6,component_reuse=reuse,game_install_or_launch=False,uploaded=False,Deck_validation=False)
 if a.smooth_zoom:result.update(fresh_native_zoom_ABI_calls=2004,all_gameplay_components_byte_exact=True,source_adaptation=dict(base_build=str(prior),frontend_injection_refreshed=True,all_component_dependency_files_exact=True),fresh_checks_revision10=dict(native_zoom_ABI_calls=2004,controls=30,native_worker=33,lifecycle_camera=123,Lua_UI=66,native_context=5,compiled_frontend=5,native_viewport=6,Workshop=16),inherited_component_checks=dict(source_build=str(prior),all_binaries_exact=True,previous_full_adapter=100,previous_full_QTE=50,HUD=8))
 if a.rebindings:
  assert 'PASS 28 bindings183' in text
  result.update(fresh_button_map_checks=28,fresh_native_zoom_ABI_calls=2004,all_four_components_exact_to_full_rebinding_build=True,full_component_evidence=str(prior/'components/build-results.json'),actual_Deck_button_effects_verified=False)
 (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n');print('Private181 correction built; '+('adapter labels/HUD placement rebuilt; QTE and extractor exact'if a.camera_legend else'display metadata rebuilt in adapter; three other components exact'if a.controller_labels else'gameplay components byte-exact')+'; not installed or uploaded.')
if __name__=='__main__':main()
