"""Release only the temporary cache validator in an isolated Deck snapshot."""
import hashlib
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
SOURCE='native/work/autoload99-preview/integrated.h'
BEFORE='assets159::Store input;require(input.load(source,kind),"Local game resources missing or corrupt; run the local-assets setup again");'
AFTER=BEFORE+'''
    // The validator is no longer used. Close its map and file before creating
    // or renaming a hard-link alias: Proton sharing applies to the same inode.
    // Keep immutable resource readers and all hash/foreign-file guards intact.
    input.clear();'''
GATE='static bool battle_ready(uintptr_t base){try{auto root=local_u32(base+0x1bc8180);auto env=local_u32(root+0x110);auto common=local_u32(env+8);auto phase=local_u32(common+0xb4);auto id=local_u32(phase+0x14);return id>=5&&id<=8;}catch(...){return false;}}'
DECK_GATE='#include "../../deck/device192.h"\n#include "../../deck/battle_ready181.h"\nstatic bool battle_ready(uintptr_t base){try{if(device192::enabled())return deck181::battle_ready(base,local_u32);auto root=local_u32(base+0x1bc8180);auto env=local_u32(root+0x110);auto common=local_u32(env+8);auto phase=local_u32(common+0xb4);auto id=local_u32(phase+0x14);return id>=5&&id<=8;}catch(...){return false;}}'

def verify(snapshot,baseline=ROOT,legend184=False):
    expected=(Path(baseline)/SOURCE).read_text(encoding='utf-8')
    actual=(Path(snapshot)/SOURCE).read_text(encoding='utf-8')
    if legend184:
        include='#include "../../deck/legend_resources184.h"\n'
        publish='\n    if(device192::enabled())deck184::publish_resources(runtime_folder);'
        if actual.count(include)!=1 or actual.count(publish)!=1:raise ValueError('Unreviewed legend resource handoff')
        actual=actual.replace(include,'').replace(publish,'')
    if expected.count(BEFORE)!=1 or actual.count(AFTER)!=1 or actual.count(DECK_GATE)!=1 or actual.replace(AFTER,BEFORE).replace(DECK_GATE,GATE)!=expected:
        raise ValueError('Unreviewed cache/deployment integration change')
    return dict(temporary_mapping_closed_before_alias_commit=True,initialized_battle_camera_gate=True,exact_reverse_comparison=True,validated_resource_folder_published=legend184,
        original_text_sha256=hashlib.sha256(expected.encode()).hexdigest(),
        corrected_text_sha256=hashlib.sha256(actual.encode()).hexdigest())

def generate(snapshot,legend184=False):
    snapshot=Path(snapshot).resolve()
    if snapshot==ROOT or ROOT.is_relative_to(snapshot):raise ValueError('Use an isolated cache176 snapshot')
    path=snapshot/SOURCE;original=(ROOT/SOURCE).read_bytes()
    if path.read_bytes()!=original:raise ValueError('Cache source differs before adaptation')
    text=(ROOT/SOURCE).read_text(encoding='utf-8')
    if text.count(BEFORE)!=1 or text.count(GATE)!=1:raise ValueError('Cache/deployment anchor differs')
    text=text.replace(BEFORE,AFTER).replace(GATE,DECK_GATE)
    if legend184:
        text='#include \"../../deck/legend_resources184.h\"\n'+text
        needle='    auto adapter=extract('
        if text.count(needle)!=1:raise ValueError('Resource handoff boundary differs')
        text=text.replace(needle,'    if(device192::enabled())deck184::publish_resources(runtime_folder);\n'+needle)
    path.write_text(text,encoding='utf-8')
    return verify(snapshot,legend184=legend184)
