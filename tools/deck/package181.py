"""Prepare the existing PRIVATE Deck item's publisher package; never stage or upload."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'tools/deck')]
import adapt_frontend181
from game_packs import Pack,digest

NAME='shogun2_unit_possession_deck_probe174'

def package(build,output,preview,luac):
    build=Path(build).resolve();output=Path(output).resolve();preview=Path(preview).resolve()
    if output.exists() or ROOT.is_relative_to(output):raise ValueError('Use a new isolated output')
    if any((parent/'Empire.Retail.dll').is_file() for parent in (output,*output.parents)):
        raise ValueError('Packaging cannot write into a game folder')
    result=json.loads((build/'verification.json').read_text())
    verified=json.loads((build/'workshop/verification.json').read_text())
    candidate=result.get('candidate')
    if candidate!=181 or result.get('quick_only') or result.get('possession_context_checks')!=30 or result.get('frontend_Lua_policy_checks')!=(93 if result.get('revision')=='menu-parent-36'else 90 if result.get('revision')=='menu-fit-35'else 87 if result.get('revision')in('usability-17','usability-18','usability-19','automatic-device-20','usability-21','bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')else 85 if result.get('revision')=='usability-16'else 80 if result.get('revision')=='usability-15'else 76 if result.get('revision')=='usability-14'else 75 if result.get('revision')=='usability-13'else 73 if result.get('revision')=='campaign-buttons-12'else 72 if result.get('revision')=='button-bindings-11'else 66) or result.get('menu_worker_boundary_checks')!=33:
        raise ValueError('Use a completed181 contextual controller build')
    if 'PASS 13 handoff181' not in (build/'logs/new-fixtures.log').read_text():
        raise ValueError('Missing owner/deployment/timeout boundary checks')
    if 'PASS 5 native context181' not in (build/'logs/native-context-lookup.log').read_text():
        raise ValueError('Missing actual native context helper checks')
    expected_checks={'compile','lua-bridge','menu-failure-recovery','enabled-ui-mod',
        'renamed-duplicate-bootstrap','repeat-startup','runtime-resources','storage-lifetime',
        'mod-coexistence','actual-qte-stock-startup','actual-qte-occupied-live-hook',
        'actual-qte-independent-identity','actual-qte-critical-engine-refusal',
        'actual-qte-reviewed-engine-startup','engine-contract-controls','actual-qte-independent-code'}
    if result.get('revision')in('usability-14','usability-15','usability-16','usability-17','usability-18','usability-19','automatic-device-20','usability-21','bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36'):
        expected_checks.update('bridge-'+name for name in ('buffer','hook','trampoline','hde32'))
    if result.get('revision')in('hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and (not result.get('shared_HUD198_checks')or 'PASS HUD198: desktop K and View'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing PC/Deck HUD-cycle checks')
    checks=verified['fresh_checks']
    if verified.get('test')!=candidate or len(checks)!=len(expected_checks) or {x['name']for x in checks}!=expected_checks or not all(x['passed']for x in checks):
        raise ValueError('Incomplete Workshop checks')
    if result.get('revision')in('automatic-device-20','usability-21','bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36'):
        if not result.get('automatic_device192') or result.get('device_fixture_cases')!=38 or result.get('compiled_menu_bootstrap_checks')!=6:raise ValueError('Missing automatic device checks')
        if (build/'logs/new-fixtures.log').read_text().count('PASS device192: 19 production')!=2:raise ValueError('Missing both native device profiles')
    if result.get('revision')in('usability-21','bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (not result.get('qol193_checks') or 'PASS qol193:' not in (build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing QoL checks')
    if result.get('revision')in('bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (not result.get('bindings194_checks') or not result.get('cursor194_native_image') or 'PASS bindings194:' not in (build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing binding/cursor checks')
    if result.get('revision')in('bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (not result.get('battle194_checks') or 'PASS battle194:' not in (build/'logs/new-fixtures.log').read_text() or 'PASS battle194 continuous native zoom: 32' not in (build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing native battle group/zoom checks')
    if result.get('revision')in('zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (not result.get('battle_zoom195_checks') or 'PASS zoom195 policy:'not in(build/'logs/new-fixtures.log').read_text() or 'PASS zoom195 native ramp: 32'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing battle shoulder acceleration checks')
    if result.get('revision')in('wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and (not result.get('command_wheel197_checks')or 'PASS wheel197 dispatch: 8'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing native wheel command checks')
    if result.get('revision')in('return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and (not result.get('campaign_return196_checks')or 'PASS return196: 30 two-DLL'not in(build/'logs/new-fixtures.log').read_text()or 'PASS return196 adapter yield: 12'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing campaign return checks')
    if result.get('revision')=='wall-27' and 'PASS wall200: 432 original per-actor request cases' not in (build/'components/logs/adapter-tests.log').read_text(errors='replace'):raise ValueError('Missing native personal-wall permission/admission/cancellation checks')
    if result.get('revision')in('aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36'):
        if result.get('fractional_aim201_checks')!=1440 or 'PASS aim201: 1440 actual camera'not in(build/'logs/new-fixtures.log').read_text():raise ValueError('Missing fractional aiming checks')
        wall_log=(build/'components/logs/adapter-tests.log').read_text(errors='replace')
        if 'PASS wall201: 96 top-entry offers/requests refused'not in wall_log or 'PASS wall200:'not in wall_log:raise ValueError('Missing upward-wall and removed-down-entry checks')
    if result.get('revision')in('look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (result.get('camera_rate202_checks')!=4296 or 'PASS look202: 4296 actual first/third-person camera'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing frame-timed analog camera checks')
    if result.get('revision')in('movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36') and (result.get('locomotion204_idle_categories')!=37 or 'PASS locomotion204: 37 native idle categories'not in(build/'components/logs/adapter-tests.log').read_text(errors='replace')):raise ValueError('Missing native standing-idle regression checks')
    if result.get('revision')in('movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and(result.get('edge205_native_branch_cases')!=216 or not result.get('edge205_Deck_only')or 'PASS edges205: 216 original native'not in(build/'logs/new-fixtures.log').read_text()):raise ValueError('Missing Deck-only native edge-camera checks')
    if result.get('revision')in('cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and(not result.get('locomotion206_mounted_regression')or 'PASS locomotion206: observed horse idle reproduction; 36 mounted directional gait cases; 16 native horse idle cases; 37 special/identity guards.'not in(build/'components/logs/adapter-tests.log').read_text(errors='replace')):raise ValueError('Missing native mounted movement regression')
    if result.get('revision')in('acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')and(not result.get('acquisition207')or(build/'components/logs/adapter-tests.log').read_text(errors='replace').count('PASS acquisition207:')!=2):raise ValueError('Missing foot/mounted native acquisition checks')
    if result.get('revision')in('acquisition-34','menu-fit-35','menu-parent-36')and(not result.get('acquisition208')or(build/'components/logs/adapter-tests.log').read_text(errors='replace').count('PASS acquisition208:')!=2 or(build/'components/logs/adapter-tests.log').read_text(errors='replace').count('PASS acquisition208 distance:')!=2):raise ValueError('Missing shared native acquisition208 regressions')
    if result.get('revision')=='menu-fit-35'and(not result.get('frontend_campaign_fit209')or result.get('frontend_fit209_original_engine_cases')!=8 or (build/'logs/new-fixtures.log').read_text().count('PASS menu209: 4 native frontend campaign fitting cases')!=2):raise ValueError('Missing native frontend setup fitting checks')
    if result.get('revision')=='menu-parent-36'and(not result.get('frontend_parent_fit210')or result.get('frontend_parent210_original_engine_cases')!=16 or (build/'logs/new-fixtures.log').read_text().count('PASS menu210: 8 native campaign parent clipping cases')!=2):raise ValueError('Missing actual campaign parent clipping checks')
    source=build/'workshop/package'/verified['pack']
    if digest(source)!=verified['pack_sha256']:raise ValueError('Built pack differs')
    parsed=Pack(source)
    expected={'ui/frontend ui/layout_scripts/root.luac','possession/workshop165.json',
              'possession/licenses/LICENSE.txt','possession/licenses/THIRD_PARTY_NOTICES.md',
              'possession/licenses/MinHook-LICENSE.txt','possession/PLAYING.md'}
    if set(parsed.entries)!=expected:raise ValueError('Unexpected input pack closure')
    raw=source.read_bytes();entries={}
    for name,(offset,size) in parsed.entries.items():
        data=raw[offset:offset+size]
        entries[name]=parsed.read(name,size,hashlib.sha256(data).hexdigest())
    original_root=entries['ui/frontend ui/layout_scripts/root.luac']
    # No game script is copied into the pack. This first-party wrapper requests
    # the exact owned campaign root through the existing guarded native reader.
    output.mkdir(parents=True)
    campaign_source=output/'campaign181.lua';campaign_compiled=output/'campaign181.luac'
    campaign_source.write_text(adapt_frontend181.campaign_wrapper(build/'source',verified['native_sha256']),encoding='utf-8')
    subprocess.run([str(Path(luac).resolve()),'-s','-o',str(campaign_compiled),str(campaign_source)],check=True,cwd=output)
    campaign=campaign_compiled.read_bytes()
    if campaign[:12]!=bytes.fromhex('1b4c75615100010404040400'):raise ValueError('Matching float Lua compiler required')
    entries['ui/campaign ui/layout.root.luac']=campaign
    playing=ROOT/('docs/development/SteamDeck'+str(candidate)+'.md')
    entries['possession/PLAYING.md']=playing.read_bytes()
    metadata=json.loads(entries['possession/workshop165.json'])
    metadata.update(private_Deck_test=candidate,private_item='3829785193',private_Deck_revision=result.get('revision','initial'),
                    controller_scope='RTS camera/cursor/selection/contextual orders/Halt only',
                    actual_Deck_RTS_validation=False)
    metadata.update(automatic_device192=result.get('automatic_device192',False),desktop_profile='Normal keyboard/mouse controls and original UI; no Deck prompts',controller_scope='Automatic menu/campaign/RTS/possession action sets with contextual Valve glyphs; Deck validation pending',
        R3_gestures=(dict(hold='first/third person',tap='unassigned',double_tap='unassigned')if result.get('revision')in('campaign-buttons-12','usability-13','usability-14','usability-15','usability-16','usability-17','usability-18','usability-19','automatic-device-20','usability-21','bindings-22','zoom-23','return-23','wheel-24','hud-25','gameplay-26','wall-27','aim-28','look-29','gameplay-30','movement-31','cavalry-32','acquisition-33','acquisition-34','menu-fit-35','menu-parent-36')else dict(tap='possess',double_tap='return to RTS',hold='first/third person')),actual_Deck_menu_validation=False)
    entries['possession/workshop165.json']=(json.dumps(metadata,indent=2)+'\n').encode()
    image=preview.read_bytes()
    if not image.startswith(b'\x89PNG\r\n\x1a\n') or len(image)>=1000000:
        raise ValueError('Use the existing private PNG preview below1MB')
    index=b''.join(struct.pack('<I',len(value))+name.replace('/','\\').encode()+b'\0' for name,value in entries.items())
    stamp=struct.unpack_from('<Q',raw,24)[0]
    target=output/(NAME+'.pack')
    with target.open('xb') as stream:
        stream.write(struct.pack('<4s5IQ',b'PFH3',3,0,0,len(entries),len(index),stamp)+index+b''.join(entries.values()))
    delivered=Pack(target)
    for name,value in entries.items():
        if delivered.read(name,len(value),hashlib.sha256(value).hexdigest())!=value:raise ValueError('Private pack roundtrip differs')
    if entries['ui/frontend ui/layout_scripts/root.luac']!=original_root:raise ValueError('Gameplay/bootstrap payload changed')
    (output/(NAME+'.png')).write_bytes(image)
    shutil.copyfile(playing,output/'PLAYING.md')
    for name in ('LICENSE','THIRD_PARTY_NOTICES.md','MinHook-LICENSE.txt'):
        shutil.copyfile(build/'workshop/package'/name,output/name)
    value=dict(packager_sha256=digest(Path(__file__)),candidate=candidate,revision=result.get('revision','initial'),item='3829785193',pack=target.name,pack_sha256=digest(target),pack_bytes=target.stat().st_size,
               preview_sha256=digest(output/(NAME+'.png')),source_pack_sha256=digest(source),
               native_bridge_sha256=verified['native_sha256'],components=verified['components'],
               entries=len(entries),changed_entries=['possession/PLAYING.md','possession/workshop165.json','ui/campaign ui/layout.root.luac'],campaign_wrapper_sha256=digest(campaign_compiled),
               bootstrap_and_gameplay_payload_byte_exact=True,first_party_code_only=True,
               recorder_debugger=False,installed=False,uploaded=False,live_Deck_RTS_validation=False,
               normal_public_item_3825625050_untouched=True)
    (output/'verification.json').write_text(json.dumps(value,indent=2)+'\n')
    return value

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('build','output','preview','luac'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();print(json.dumps(package(a.build,a.output,a.preview,a.luac),indent=2))
