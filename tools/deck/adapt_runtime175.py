"""Generate the optional Deck build in an isolated source snapshot only.

Known anchors join the existing owning-window callback and worker. The original
maintained adapter/extractor files are never edited. Reversing every substitution
must recover their entire prior text; offsets/asset decoders are not rewritten.
"""
import hashlib
import json
from pathlib import Path
import native_defaults174
from review_bindings175 import header

ROOT=Path(__file__).resolve().parents[2]
ADAPTER='native/outputs/shogun2-accuracy90-preview/src/adapter.cpp'
EXTRACTOR='tools/installer/prepare_local_assets159.cpp'

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def substitute(path,changes):
    original=path.read_text(encoding='utf-8');text=original
    for before,after in changes:
        if text.count(before)!=1:raise ValueError('Unreviewed integration anchor in '+str(path)+': '+before[:70])
        text=text.replace(before,after)
    restored=text
    for before,after in reversed(changes):
        if restored.count(after)!=1:raise ValueError('Ambiguous reverse integration anchor')
        restored=restored.replace(after,before)
    if restored!=original:raise ValueError('Integration modified unrelated text')
    path.write_text(text,encoding='utf-8')
    return dict(original_text_sha256=hashlib.sha256(original.encode()).hexdigest(),
        transformed_text_sha256=hashlib.sha256(text.encode()).hexdigest(),
        exact_reverse_comparison=True,substitutions=len(changes))

def generate(snapshot):
    snapshot=Path(snapshot).resolve()
    if snapshot==ROOT or ROOT.is_relative_to(snapshot):raise ValueError('Use an isolated source snapshot')
    for name in (ADAPTER,EXTRACTOR):
        if (snapshot/name).read_bytes()!=(ROOT/name).read_bytes():raise ValueError('Snapshot differs before integration')
    prototype='static LRESULT CALLBACK weapon_window_proc(HWND window,UINT message,WPARAM w,LPARAM l){'
    worker='static DWORD WINAPI worker(void*) {'
    focus='if(state&&(message==WM_KILLFOCUS||(message==WM_ACTIVATEAPP&&!w))){'
    body=substitute(snapshot/ADAPTER,[
        (prototype,'static bool deck175_window(HWND,UINT,WPARAM,LPARAM);\nstatic void deck175_focus_lost(HWND);\n'+prototype+'\n    if(deck175_window(window,message,w,l))return 0;'),
        (focus,focus+'\n        deck175_focus_lost(window);'),
        (worker,'#include "../../../deck/adapter175.h"\n'+worker+'\n    deck175::Lifetime deck_lifetime175;'),
        ('state->heartbeat=GetTickCount64();unsigned input=0;',
         'state->heartbeat=GetTickCount64();unsigned input=0;\n            deck175::poll(self_module);')])
    source=(snapshot/EXTRACTOR).read_text()
    first=source.index('static void plain159(');last=source.index('\nstatic unsigned u16_159',first)
    old=source[first:last]
    extractor=substitute(snapshot/EXTRACTOR,[
        ('#include "localization167.h"','#include "localization167.h"\n#include "../../native/deck/proton_paths174.h"'),
        (old,'static void plain159(const std::filesystem::path& p){deck174::paths::plain(p);}'),
        ('plain159(english);','deck174::paths::read_file(english);'),
        ('plain159(entry.path());','deck174::paths::read_file(entry.path());'),
        ('plain159(game);plain159(out);plain159(game/L"Empire.Retail.dll");',
         'deck174::paths::read_directory(game);plain159(out);deck174::paths::read_file(game/L"Empire.Retail.dll");'),
        ('plain159(game/L"data"/spec.pack);','deck174::paths::read_file(game/L"data"/spec.pack);')])
    native=snapshot/'native/deck'
    values={'manifest':native_defaults174.manifest(),'bindings':native_defaults174.configuration(),
            'settings':(snapshot/'native/deck/controller174.ini').read_text()}
    native_defaults174.validate(values['bindings'])
    defaults=native/'defaults175_generated.h'
    with defaults.open('x',encoding='utf-8') as stream:
        stream.write('#pragma once\nnamespace defaults175 {\n'+''.join('inline constexpr char '+name+'[]='+json.dumps(text)+';\n' for name,text in values.items())+'}\n')
    metadata=json.loads((snapshot/'tools/data/deck175-native-bindings.json').read_text())
    if metadata['schema']!=175:raise ValueError('Unknown native binding metadata')
    with (native/'bindings175_generated.h').open('x',encoding='utf-8') as stream:stream.write(header(metadata))
    # Apply the separately tested Proton pack/UI reader adapter in this snapshot.
    import adapt_paths174
    paths=adapt_paths174.generate(snapshot)
    entry=snapshot/'tools/workshop/entry165.cpp'
    entry_changes=substitute(entry,[('#include "pack165.h"','#include "../deck/pack174_generated.h"'),
        ('#include "ui_compat168.h"','#include "../deck/ui_compat174_generated.h"')])
    return dict(adapter=body,extractor=extractor,entry=entry_changes,paths=paths,
        original_runtime_sources_modified=False,ordinary_Windows_build_modified=False,
        generated_defaults_sha256=digest(defaults),generated_guards_sha256=digest(native/'bindings175_generated.h'))
