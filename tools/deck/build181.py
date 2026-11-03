"""Build an isolated native Deck menu/RTS candidate; never install, launch or publish."""
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
import adapt_cache176
import adapt_frontend181
import adapt_controls181

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('game','bugfix','output','vcvars','lua','luac','abi-reference-dir'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--quick',action='store_true',help='Compile adapter and new fixtures only; not a distributable build')
    p.add_argument('--components-from',type=Path,help='Reuse all four components ONLY after complete dependency closure equality and100/50/8 passed regressions; run every current focused/bridge check fresh')
    a=p.parse_args();game=a.game.resolve();out=a.output.resolve();require_game(game)
    if out.exists() or ROOT.is_relative_to(out) or out.is_relative_to(game):p.error('Use a new isolated output outside the game')
    abi=check_abi174.inspect(a.abi_reference_dir.resolve())
    out.mkdir(parents=True);snapshot=out/'source';snapshot.mkdir();logs=out/'logs';logs.mkdir()
    inputs=[]
    for source in source_files(ROOT):
        rel=source.relative_to(ROOT);target=snapshot/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        inputs.append(dict(path=rel.as_posix(),sha256=digest(source)))
    (out/'source-inputs.json').write_text(json.dumps(inputs,indent=2)+'\n')
    adaptation=dict(RTS175=adapt_runtime175.generate(snapshot),cache176=adapt_cache176.generate(snapshot,legend184=True),frontend181=adapt_frontend181.generate(snapshot),controls181=adapt_controls181.generate(snapshot))
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
        '--snapshot',snapshot,'--output',out/'engine-contract','--cache-fix176','--context-controls181'])
    run('private-engine-contract-check',[sys.executable,'-B',snapshot/'tools/workshop/check_engine_contract172.py'])
    env=dict(os.environ,SHOGUN2_BUILD_OUTPUT=str(out/'components'),SHOGUN2_GAME_DIR=str(game),SHOGUN2_VCVARS=str(a.vcvars.resolve()))
    # Compiler commands execute from the snapshot so build-time transformations
    # cannot enter the normal maintained source or installed game directory.
    reuse=None;inherited_hooks=None
    if a.components_from:
        prior=a.components_from.resolve();component=prior/'components'
        results=json.loads((component/'build-results.json').read_text());assert all(row['passed']for row in results['results'])
        for name,count in [('adapter-tests',100),('qte-tests',50),('hud-tests',8)]:
            assert next(x for x in results['results']if x['name']==name)['executables']==count
        def closure(folder,tag):
            spec=importlib.util.spec_from_file_location(tag,folder/'tools/workshop/generate_engine_contract172.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            entries=['native/outputs/shogun2-accuracy90-preview/src/adapter.cpp','native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp','native/outputs/shogun2-ammunition83-preview/hud-src/hud.cpp','tools/installer/prepare_local_assets159.cpp']
            return {q.relative_to(folder).as_posix():digest(q)for q in module.source_closure(entries)}
        before=closure(component/'source','before185');after=closure(snapshot,'after185')
        if before!=after:raise ValueError('A compiled component dependency changed; fresh full build required')
        # Embedded resource definitions and shared local-assets preparation also
        # must remain exact; C++ closure alone is insufficient for RC payloads.
        for old in (component/'source/native').rglob('*'):
            if old.is_file()and old.suffix.lower()in('.rc','.rcinc','.tga','.ttf','.cuf','.bin'):
                target=snapshot/old.relative_to(component/'source')
                if not target.is_file()or digest(old)!=digest(target):raise ValueError('Compiled resource input differs: '+str(old))
        destination=out/'components';shutil.copytree(component/'bin',destination/'bin');shutil.copyfile(component/'build-results.json',destination/'build-results.json')
        shutil.copyfile(component/'build-inputs.json',destination/'build-inputs.json')
        shutil.copytree(component/'logs',destination/'logs') # unchanged component evidence; explicitly inherited in receipt
        reuse=dict(source_build=str(prior),complete_component_dependency_files=len(before),dependencies_exact=True,resource_inputs_exact=True,
                   inherited_checks='100 adapter/50 QTE/8 HUD plus local resource/font/render checks',fresh_component_checks=False,
                   results_sha256=digest(component/'build-results.json'),binaries={q.name:digest(q)for q in(destination/'bin').iterdir()if q.suffix in('.exe','.dll')})
        (out/'component-reuse.json').write_text(json.dumps(reuse,indent=2)+'\n');inherited_hooks=component/'obj'
    elif a.quick:
        driver=out/'quick.py'
        driver.write_text('import sys,os\nfrom pathlib import Path\nsys.path.insert(0,'+repr(str(snapshot/'tools'))+')\nimport native_build as m\nm.VCVARS=Path(os.environ["SHOGUN2_VCVARS"])\nfor q in [m.BIN,m.OBJ,m.TEST,m.LOG]:q.mkdir(parents=True,exist_ok=True)\nm.adapter(False)\n')
        run('quick-adapter-compile',[sys.executable,'-B',driver],env)
    else:
        run('components',[sys.executable,'-B',snapshot/'tools/build.py','workshop','--candidate','181','--tests',
            '--local-assets','--game',game,'--output-dir',out/'components','--vcvars',a.vcvars.resolve()],env)
    # Actual native guard mapping and portable input/RTS fixtures are mandatory
    # even in a quick compile. None starts a Steam API or any game worker.
    from build174 import batch_script
    bin_dir=out/'fixture-bin';bin_dir.mkdir();common=['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/W4']
    commands=[]
    hook_objects=[(inherited_hooks or out/'components/obj')/name for name in ('buffer.obj','hook.obj','trampoline.obj','hde32.obj')]
    owner196=bin_dir/'keyboard-owner196.dll';return196=bin_dir/'verify-return196.exe'
    commands.extend([[*common,'/LD',snapshot/'tools/deck/verify_keyboard_owner196.cpp','/Fo'+str(bin_dir/'owner196.obj'),'/Fe'+str(owner196),'/link',*hook_objects,'user32.lib'],
                     [*common,snapshot/'tools/deck/verify_return196.cpp','/Fo'+str(bin_dir/'return196.obj'),'/Fe'+str(return196),'/link',*hook_objects,'user32.lib'],[return196,owner196]])
    for source,libs,args in [('verify_edges205.cpp',['user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_wheel197.cpp',[],[]),('verify_zoom195.cpp',[],[]),('verify_battle194.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_bindings194.cpp',['user32.lib','gdi32.lib'],[]),('verify_qol193.cpp',['user32.lib'],[]),('verify_usability190.cpp',['user32.lib'],[]),('verify_selection189.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_native188.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_panels187.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_view186.cpp',['user32.lib'],[]),('verify_campaign_hook185.cpp',['user32.lib'],[]),('verify_native_ui185.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_usability185.cpp',['user32.lib'],[]),('verify_usability184.cpp',[],[]),('verify_bindings183.cpp',['user32.lib'],[]),('verify_zoom181.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()]),('verify_lifecycle181.cpp',['user32.lib'],[]),('verify_controls181.cpp',['user32.lib'],[]),('verify_handoff181.cpp',[],[]),('verify_navigation182.cpp',['user32.lib'],[]),('verify_binding_profiles182.cpp',['user32.lib'],[]),('verify174.cpp',['user32.lib'],[]),('verify_rts175.cpp',[],[]),('verify_orders175.cpp',[],[]),('verify_frontend177.cpp',['user32.lib','gdiplus.lib'],[out/'native-menu-fixture']),('verify_frontend181.cpp',['user32.lib','gdiplus.lib','gdi32.lib','bcrypt.lib'],[]),('verify_controls178.cpp',['user32.lib'],[]),('verify_camera178.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll']),
            ('verify_native175.cpp',['bcrypt.lib','user32.lib'],[game/'Empire.Retail.dll',a.bugfix.resolve()])]:
        exe=bin_dir/(Path(source).stem+'.exe')
        commands.extend([[*common,snapshot/'tools/deck'/source,'/Fo'+str(bin_dir/(Path(source).stem+'.obj')),'/Fe'+str(exe),'/link',*libs,*hook_objects],[exe,*args]])
    fake=bin_dir/'steam_api.dll';device=bin_dir/'verify_device192.exe'
    commands.extend([[*common,'/LD',snapshot/'tools/deck/fake_steam192.cpp','/Fo'+str(bin_dir/'fake-steam192.obj'),'/Fe'+str(fake)],
                     [*common,snapshot/'tools/deck/verify_device192.cpp','/Fo'+str(bin_dir/'verify-device192.obj'),'/Fe'+str(device),'/link','user32.lib'],
                     [device,fake,'0'],[device,fake,'1']])
    dispatch=bin_dir/'controls-boundary181.exe'
    commands.extend([[*common,'/DS2FP_LOCAL_ASSETS','/DNOMINMAX','/I'+str(snapshot/'native/outputs/shogun2-accuracy90-preview/src'),snapshot/'tools/deck/verify_dispatch181.cpp','/Fo'+str(bin_dir/'dispatch.obj'),'/Fe'+str(dispatch),'/link',*hook_objects,'bcrypt.lib','user32.lib','gdi32.lib','gdiplus.lib','d3d11.lib','d3d9.lib','dxgi.lib','d3dcompiler.lib'],[dispatch]])
    batch=out/'new-fixtures.cmd';batch.write_text(batch_script(a.vcvars.resolve(),commands))
    run('new-fixtures',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',batch])
    context_dll=bin_dir/'context-lookup181.dll'
    context_batch=out/'context-lookup.cmd';context_batch.write_text(batch_script(a.vcvars.resolve(),[
        [*common,'/LD',snapshot/'tools/deck/verify_context_lookup181.cpp','/Fo'+str(bin_dir/'context-lookup.obj'),'/Fe'+str(context_dll)]]))
    run('context-lookup-compile',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',context_batch])
    context_env=dict(os.environ,S2UP165_FIXTURE_GAME=str(game))
    run('native-context-lookup',[a.lua.resolve(),snapshot/'tools/deck/verify_context_lookup181.lua',context_dll],context_env)
    if 'PASS 5 native context181' not in (logs/'native-context-lookup.log').read_text():raise ValueError('Native context helper fixture did not complete')
    if 'PASS native string183:' not in (logs/'native-context-lookup.log').read_text():raise ValueError('Missing actual native string argument failure regression')
    text=(logs/'new-fixtures.log').read_text(errors='replace')
    for marker in ('PASS 123 lifecycle181','PASS 30 controls181 context/R3/lifecycle checks','PASS 13 handoff181','PASS 7 navigation182','PASS 7 binding_profiles182','PASS 48 Deck simulation/ABI checks','PASS 22 RTS175 policy/queue checks','PASS 12 RTS175 native Halt boundary checks','PASS RTS175 normalized native guards:'):
        if marker not in text:raise ValueError('A native fixture did not complete: '+marker)
    if text.count('PASS device192: 19 production')!=2 or 'PASS desktop192 adapter bypass' not in text:raise ValueError('Automatic device/profile checks incomplete')
    run('native-menu-Lua-policy',[a.lua.resolve(),snapshot/'tools/deck/verify_frontend181.lua',snapshot/'native/deck/frontend181.lua',snapshot/'native/deck/navigation181.lua',snapshot/'native/deck/frontend179.lua',snapshot/'native/deck/navigation179.lua'])
    if 'PASS 87 frontend181' not in (logs/'native-menu-Lua-policy.log').read_text():raise ValueError('Menu policy fixture did not complete')
    if 'PASS panels191: 44 native whole-window' not in text or '10003 bounded burst' not in text:raise ValueError('Missing actual native panel/burst checks')
    for marker in ['PASS 33 frontend181 native worker boundary checks','PASS 18 controls178','PASS camera178 original native setter/move/turn: 16']:
        if marker not in text:raise ValueError('Missing private check: '+marker)
    if 'PASS 11 frontend177 native lifecycle/glyph checks' not in text:raise ValueError('Native menu fixture did not complete')
    if 'PASS selection189: 30 original native Ctrl producer/query/UI-cache/battlefield-event'not in text:raise ValueError('Missing actual native selection modifier cases')
    if 'PASS usability190: 25 'not in text:raise ValueError('Missing current campaign/click/Back regression checks')
    if 'PASS edges205: 216 original native'not in text:raise ValueError('Missing Deck-only edge-camera regression')
    if 'PASS native188:' not in text:raise ValueError('Missing native modifier/Clan/modal checks')
    if 'PASS HUD198: desktop K and View' not in text:raise ValueError('Missing shared PC/Deck HUD-cycle regression')
    if 'PASS look202: 4296 actual first/third-person camera'not in text:raise ValueError('Missing camera frame timing regressions')
    if 'PASS aim201: 1440 actual camera'not in text:raise ValueError('Missing fractional Deck aiming camera regressions')
    if 'PASS wheel197 policy: 22'not in text or 'PASS wheel197 dispatch: 8'not in text:raise ValueError('Missing wheel input/native-command dispatch checks')
    if 'PASS return196: 30 two-DLL'not in text or 'PASS return196 adapter yield: 12'not in text:raise ValueError('Missing post-battle input ownership regressions')
    if 'PASS zoom195 policy:' not in text or 'PASS zoom195 native ramp: 32'not in text:raise ValueError('Missing battle shoulder ramp/lifecycle checks')
    if 'PASS battle194:' not in text:raise ValueError('Missing original native group/battle binding checks')
    if 'PASS bindings194:' not in text:raise ValueError('Missing binding/cursor checks')
    if 'PASS qol193:' not in text:raise ValueError('Missing QoL control and HUD sampling regressions')
    if a.quick:
        (out/'verification.json').write_text(json.dumps(dict(candidate=181,quick_only=True,
            adapter_compiled=True,new_fixtures_passed=True,abi=abi,game_install_or_launch=False,not_for_distribution=True),indent=2)+'\n')
        return
    run('workshop-package',[sys.executable,'-B',snapshot/'tools/workshop/build_workshop165.py','--candidate','181',
        '--game',game,'--compat-engine',a.bugfix.resolve(),'--components',out/'components/bin','--output',out/'workshop',
        '--vcvars',a.vcvars.resolve(),'--lua',a.lua.resolve(),'--luac',a.luac.resolve()])
    entry_cache=out/'menu-entry-cache';entry_cache.mkdir()
    entry_env=dict(os.environ,LOCALAPPDATA=str(entry_cache),S2UP165_FIXTURE_GAME=str(game))
    run('compiled-menu-bootstrap',[a.lua.resolve(),snapshot/'tools/deck/verify_frontend_entry181.lua',out/'workshop/bootstrap-fixture.luac'],entry_env)
    if 'PASS 6 compiled frontend181 bootstrap integration checks' not in (logs/'compiled-menu-bootstrap.log').read_text():raise ValueError('Compiled menu entry checks did not complete')
    run('actual-Lua-viewport',[a.lua.resolve(),snapshot/'tools/deck/verify_viewport181.lua',out/'workshop/bin/s2up-workshop165-fixture.dll'],entry_env)
    if 'PASS 6 actual float-Lua viewport' not in (logs/'actual-Lua-viewport.log').read_text():raise ValueError('Native viewport export checks did not complete')
    ui_lua_dll=bin_dir/'ui-lua185.dll'
    ui_batch=out/'ui-lua185.cmd';ui_batch.write_text(batch_script(a.vcvars.resolve(),[[*common,'/LD',snapshot/'tools/deck/verify_ui_lua185.cpp','/Fo'+str(bin_dir/'ui-lua185.obj'),'/Fe'+str(ui_lua_dll),'/link',*hook_objects,'bcrypt.lib','user32.lib']]))
    run('native-ui-Lua-compile',[os.environ.get('COMSPEC','cmd.exe'),'/d','/c',ui_batch])
    run('native-ui-float-Lua',[a.lua.resolve(),snapshot/'tools/deck/verify_ui_lua185.lua',ui_lua_dll],entry_env)
    if 'PASS native UI185 float-Lua:3000'not in(logs/'native-ui-float-Lua.log').read_text():raise ValueError('Native tooltip stack checks incomplete')
    component_text=(out/'components/logs/adapter-tests.log').read_text(errors='replace')
    for marker in ('PASS locomotion206: observed horse idle reproduction; 36 mounted directional gait cases; 16 native horse idle cases; 37 special/identity guards.', 'PASS locomotion204: 37 native idle categories', 'PASS framing203: 690','PASS locomotion203: original idle reproduction; 36','PASS acquisition203: actual native omission reproduced'):
        if marker not in component_text:raise ValueError('Missing gameplay203 native regression: '+marker)
    if component_text.count('PASS acquisition207:')!=2:raise ValueError('Missing foot/mounted acquisition regression')
    if component_text.count('PASS acquisition208:')!=2 or component_text.count('PASS acquisition208 distance:')!=2:raise ValueError('Missing shared direct-call and cached-distance acquisition regressions')
    result=dict(candidate=181,revision='acquisition-34',scope='Shared cone/radial/scoring entries include the registered independent soldier even when native special controllers bypass normal search; unit-pair distance also includes the independent member without rewriting the parent cache; live attacks unverified',
        native_inputs='Valve Steam Input006 through Proton; physical Deck required',abi=abi,acquisition207=True,acquisition207_live_verified=False,acquisition208=True,acquisition208_live_verified=False,
        new_simulation_ABI_checks=48,new_RTS_policy_queue_checks=22,native_Halt_boundary_checks=12,native_function_guards=len(metadata['functions']),
        native_UI_scale=1.25,native_UI_modal_policy="Uniform whole-window fit; oversized modals reduce enlargement only as needed; native contents and fonts unchanged",mod_HUD_scale=1.15,permanent_legends=False,header186="Press View: HUD; hold View: controls",controls_overlay="First-party Shogun-styled native-font and Valve-glyph guide",native_UI_projection_points=4488,native_panel_fit_cases=44,rapid_panel_burst_cases=10003,native_keyboard_hook_cases=123,native_Lua_result_cases=3000,native_string_argument_failure_cases=1,Steam_layout_revision=dict(major=3,minor=6),edge205_native_branch_cases=216,edge205_Deck_only=True,locomotion206_mounted_regression=True,locomotion206_directional_cases=36,locomotion206_idle_cases=16,locomotion206_guard_cases=37,locomotion204_idle_categories=37,locomotion204_observed_regression=True,camera_rate202_checks=4296,camera_rate202_concurrent_samples=20000,top_wall_entry_removed201=True,fractional_aim201_checks=1440,fractional_aim201_concurrent_samples=10000,shared_HUD198_checks=True,command_wheel197_checks=True,command_wheel197_deliveries=8,campaign_return196_checks=True,campaign_return196_roundtrips=30,campaign_return196_adapter_cycles=12,battle_zoom195_checks=True,battle_zoom_ramp_seconds=.45,battle_zoom_peak_factor=.8,qol193_checks=True,bindings194_checks=True,battle194_checks=True,cursor194_native_image=True,native188_checks=29,native_selection189_checks=30,usability190_checks=25,menu_RT_primary=True,button_binding_cases=28,source_adaptation=adaptation,frontend_Lua_policy_checks=87,possession_context_checks=30,controls_policy_checks=18,native_camera_rate_cases=16,menu_worker_boundary_checks=33,frontend_native_lifecycle_checks=11,compiled_menu_bootstrap_checks=6,automatic_device192=True,device_fixture_cases=38,component_reuse=reuse,cache_lifetime_fix_verified=True,game_install_or_launch=False,Steam_user_config_changed=False,
        recorder_debugger_started=False,actual_Deck_RTS_gameplay_verified=False,
        supported_increment=['native shared UI sizing/input and touch cursor; directional menu navigation remains deferred','campaign native camera/panels/selection/contextual cursor orders','RTS camera/selection/orders/Halt','automatic possession movement/look/combat/commands/abilities/QTE routing','Y possess/return; R3 hold perspective only'],
        pending=['real Deck menu/campaign/RTS/possession and QTE validation','new campaign panel/save/end-turn buttons on Deck; accepted camera feel preserved','live native shortcut/group and command-wheel effect','deployment/naval controls','timing/focus/suspend-resume validation'])
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print('Private contextual181 build complete; not installed, uploaded or hardware accepted.',flush=True)

if __name__=='__main__':main()
