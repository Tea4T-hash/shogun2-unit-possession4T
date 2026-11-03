"""Review private source pins while preserving every existing engine guard.

This is allowed only when every new backed numeric reference is ALREADY inside
an immutable region of the accepted contract. It cannot enlarge compatibility,
remove a guard, accept a new image, or regenerate hashes to hide a mismatch.
"""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/deck'))
CHANGED={'native/outputs/shogun2-accuracy90-preview/src/adapter.cpp',
         'tools/installer/prepare_local_assets159.cpp','tools/workshop/entry165.cpp'}
REPLACED={'tools/workshop/pack165.h','tools/workshop/ui_compat168.h'}

def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def protected(value,manifest):return any(r['rva']<=value<r['rva']+r['size'] for r in manifest['regions'])

def extend(snapshot,baseline=ROOT,cache_fix176=False,context_controls181=False):
    snapshot=Path(snapshot).resolve();baseline=Path(baseline).resolve()
    if snapshot==baseline or baseline.is_relative_to(snapshot):raise ValueError('Use a separate private source snapshot')
    allowed_changes=set(CHANGED);cache_review=None
    if context_controls181:
        allowed_changes.update({
            "native/outputs/shogun2-possession-cleanup-v46/src/hud_layout108.h",
            "native/outputs/shogun2-accuracy90-preview/src/ability_input141.h",
            "native/outputs/shogun2-accuracy90-preview/src/equipment_input81.h",
            "native/outputs/shogun2-accuracy90-preview/src/native_bow.h",
            "native/outputs/shogun2-accuracy90-preview/src/soldier_pose.h",
            "native/outputs/shogun2-accuracy90-preview/src/native_keyboard84.h",
            "native/outputs/shogun2-accuracy90-preview/src/hud_visibility148.h",
            "native/outputs/shogun2-accuracy90-preview/src/look_pitch96.h",
            "native/outputs/shogun2-accuracy90-preview/src/third_person152.h",
            "native/outputs/shogun2-accuracy90-preview/src/proximity147.h",
            "native/outputs/shogun2-accuracy90-preview/src/unit_detach130.h",
            "native/outputs/shogun2-authentic-hud80-preview/qte-src/playable_runtime.cpp",
            "native/outputs/shogun2-authentic-hud80-preview/src/hud_renderer.h",
            "native/outputs/shogun2-ammunition83-preview/hud-src/hud.cpp",
            "native/outputs/shogun2-ammunition83-preview/hud-src/hud_renderer.h",
            "native/work/autoload99-preview/integrated.h",
            "native/outputs/shogun2-possession-cleanup-v46/src/overlay_surface106.h",
            "native/outputs/shogun2-accuracy90-preview/src/native_overlay106.h"})
    if cache_fix176:
        import adapt_cache176
        cache_review=adapt_cache176.verify(snapshot,baseline,legend184=context_controls181)
        allowed_changes.add(adapt_cache176.SOURCE)
    manifest=json.loads((baseline/'tools/data/engine-contract172.json').read_text())
    generated='native/common/engine_contract172_generated.h'
    if digest(baseline/generated)!=manifest['generated_header_sha256'] or (snapshot/generated).read_bytes()!=(baseline/generated).read_bytes():
        raise ValueError('Existing native guard header differs; preserved')
    for name,sha in manifest['source_files'].items():
        if digest(baseline/name)!=sha:raise ValueError('Baseline dependency changed: '+name)
    spec=importlib.util.spec_from_file_location('private_deck175_contract',snapshot/'tools/workshop/generate_engine_contract172.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    sources=module.source_closure();actual={p.relative_to(snapshot).as_posix():digest(p) for p in sources}
    old=manifest['source_files'];changed={n for n in old.keys()&actual.keys() if old[n]!=actual[n]}
    removed=old.keys()-actual.keys();added=actual.keys()-old.keys()
    if not changed<=allowed_changes or not removed<=REPLACED or any(not n.startswith(('native/deck/','tools/deck/')) for n in added):
        raise ValueError('Unreviewed source integration scope')
    result=copy.deepcopy(manifest);registered={(v['source'],int(v['value'],16)) for v in manifest['bindings']};new=[]
    for path,text in module.source_closure(module.NATIVE_ENTRIES).items():
        if path.name=='engine_contract172.h':continue
        name=path.relative_to(snapshot).as_posix()
        for line,textline in enumerate(text.splitlines(),1):
            for token in re.findall(r'\b0x[0-9a-fA-F]+\b',textline):
                value=int(token,16)
                if not any(s['rva']<=value<s['rva']+s['size'] for s in manifest['sections']):continue
                if name=='native/deck/native_input185.h'and value in (0x1b30f58,0x1b31058):continue # exact relocated keyboard globals, guarded full poll body
                if name=='native/deck/frontend181.h'and value==0x1b30f58:continue # same guarded keyboard current buffer
                if name=='native/deck/native_ui185.h'and value in(0x1bbe364,0x1bc73e0,0x1bc73e4):continue # read-only current UI root/PanelManager and native reserved extent; exact native geometry bodies guarded
                if value==0x1bc8fe4 and name=='native/deck/campaign181.h':continue # reviewed mutable campaign manager pointer, never code
                if not protected(value,manifest):raise ValueError('New reference is outside existing immutable guards: '+name+' '+hex(value))
                if (name,value) not in registered:
                    registered.add((name,value));new.append(dict(source=name,line=line,value=hex(value),function=None,
                        classification='Reviewed Deck175 dependency already in unchanged immutable engine region; native call bodies separately byte-verified'))
    result['source_files']=actual;result['bindings']+=new
    if cache_review:result['deck176_cache_lifetime_review']=cache_review
    result['deck175_dependency_review']=dict(base_contract_id=manifest['contract_id'],
        immutable_header_exact=True,regions_unchanged=True,engine_admission_unchanged=True,
        changed_sources=sorted(changed),replaced_with_reversible_path_adapters=sorted(removed),
        added_sources=sorted(added),new_reviewed_references=len(new),
        native_body_evidence='tools/data/deck175-native-bindings.json')
    # These four admission fields must remain exact. A new native function with
    # unprotected bytes requires the full evidence generator, not this helper.
    for key in ('contract_id','generated_header_sha256','regions','sections'):
        if result[key]!=manifest[key]:raise ValueError('Engine guards changed')
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--snapshot',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--cache-fix176',action='store_true');p.add_argument('--context-controls181',action='store_true');a=p.parse_args()
    result=extend(a.snapshot,cache_fix176=a.cache_fix176,context_controls181=a.context_controls181);a.output.mkdir(parents=True,exist_ok=False)
    (a.output/'review.json').write_text(json.dumps(result['deck175_dependency_review'],indent=2)+'\n')
    (a.snapshot/'tools/data/engine-contract172.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS reviewed private source pins; '+str(len(result['regions']))+' immutable regions and admission/header remain exact')

if __name__=='__main__':main()
