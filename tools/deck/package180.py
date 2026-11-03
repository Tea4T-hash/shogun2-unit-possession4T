"""Prepare the existing PRIVATE Deck item's publisher package; never stage or upload."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
from game_packs import Pack,digest

NAME='shogun2_unit_possession_deck_probe174'

def package(build,output,preview):
    build=Path(build).resolve();output=Path(output).resolve();preview=Path(preview).resolve()
    if output.exists() or ROOT.is_relative_to(output):raise ValueError('Use a new isolated output')
    if any((parent/'Empire.Retail.dll').is_file() for parent in (output,*output.parents)):
        raise ValueError('Packaging cannot write into a game folder')
    result=json.loads((build/'verification.json').read_text())
    verified=json.loads((build/'workshop/verification.json').read_text())
    candidate=result.get('candidate')
    if candidate not in (180,) or result.get('quick_only'):
        raise ValueError('Use a completed private RTS build')
    if candidate==175 and not result.get('native_Halt_boundary_checks'):raise ValueError('Missing RTS boundary checks')
    if candidate==176 and not result.get('cache_lifetime_fix_verified'):raise ValueError('Missing cache lifetime checks')
    if candidate==177 and (result.get('frontend_Lua_policy_checks')!=18 or result.get('frontend_native_lifecycle_checks')!=11 or result.get('new_simulation_ABI_checks')!=48):
        raise ValueError('Missing native menu/controller checks')
    if candidate==178 and (result.get('frontend_Lua_policy_checks')!=12 or result.get('controls_policy_checks')!=16 or result.get('native_camera_rate_cases')!=16 or result.get('menu_worker_boundary_checks')!=6):
        raise ValueError('Missing corrected menu/cursor/camera checks')
    if candidate==179 and (result.get('frontend_Lua_policy_checks')!=16 or result.get('menu_worker_boundary_checks')!=12):
        raise ValueError('Missing179 native-widget/directional checks')
    if candidate==180 and (result.get('frontend_Lua_policy_checks')!=23 or result.get('menu_worker_boundary_checks')!=12):
        raise ValueError('Missing180 hierarchy regression checks')
    if verified.get('test')!=candidate or len(verified['fresh_checks'])!=16 or not all(x['passed'] for x in verified['fresh_checks']):
        raise ValueError('Incomplete Workshop checks')
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
    playing=ROOT/('docs/development/SteamDeck'+str(candidate)+'.md')
    entries['possession/PLAYING.md']=playing.read_bytes()
    metadata=json.loads(entries['possession/workshop165.json'])
    metadata.update(private_Deck_test=candidate,private_item='3829785193',
                    controller_scope='RTS camera/cursor/selection/contextual orders/Halt only',
                    actual_Deck_RTS_validation=False)
    if candidate>=177:
        metadata.update(controller_scope='Native menu cursor/prompts plus RTS camera/cursor/selection/orders/Halt; incomplete controller support',actual_Deck_menu_validation=False)
    if candidate==180:
        metadata.update(controller_scope='Native menu directional focus, prompt legend and RTS core; full controller support unfinished')
    entries['possession/workshop165.json']=(json.dumps(metadata,indent=2)+'\n').encode()
    image=preview.read_bytes()
    if not image.startswith(b'\x89PNG\r\n\x1a\n') or len(image)>=1000000:
        raise ValueError('Use the existing private PNG preview below1MB')
    output.mkdir(parents=True)
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
    value=dict(candidate=candidate,item='3829785193',pack=target.name,pack_sha256=digest(target),pack_bytes=target.stat().st_size,
               preview_sha256=digest(output/(NAME+'.png')),source_pack_sha256=digest(source),
               native_bridge_sha256=verified['native_sha256'],components=verified['components'],
               entries=len(entries),changed_entries=['possession/PLAYING.md','possession/workshop165.json'],
               bootstrap_and_gameplay_payload_byte_exact=True,first_party_code_only=True,
               recorder_debugger=False,installed=False,uploaded=False,live_Deck_RTS_validation=False,
               normal_public_item_3825625050_untouched=True)
    (output/'verification.json').write_text(json.dumps(value,indent=2)+'\n')
    return value

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('build','output','preview'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();print(json.dumps(package(a.build,a.output,a.preview),indent=2))
