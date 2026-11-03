"""Generate a diagnostic-only path-policy adapter around unchanged pack readers.

The shipping Windows sources remain exact. A pinned copy in the isolated Deck
snapshot delegates only path checks; pack parsing/root fingerprints/ownership
and menu coexistence logic remain unchanged. No SDK/game source is generated.
"""
from pathlib import Path
import hashlib
import json

PINNED={
    'pack165.h':'c586f77379fda3336618ff7a94f66d4cd8860c3a723a96c75c4d8aec8694f79f',
    'ui_compat168.h':'7155ccb453564edd6ef0b4447a44e821af43458301759bf24db47104e5834216',
}


def generate(snapshot):
    snapshot=Path(snapshot)
    files={name:(snapshot/'tools/workshop'/name).read_bytes() for name in PINNED}
    for name,expected in PINNED.items():
        if hashlib.sha256(files[name]).hexdigest()!=expected:
            raise ValueError('Review changed shared reader before adapting: '+name)
    pack=files['pack165.h'].decode().replace('\r\n','\n')
    ui=files['ui_compat168.h'].decode().replace('\r\n','\n')
    begin=pack.index('inline void plain(')
    end=pack.index('\ninline std::vector<unsigned char> ui_root168',begin)
    original_guard=pack[begin:end]
    new_guard='inline void plain(const std::filesystem::path& path){deck174::paths::plain(path);}'
    adapted=pack[:begin]+new_guard+pack[end:]
    if adapted.count('    plain(pack);')!=1:
        raise ValueError('Reader path boundary differs')
    adapted=adapted.replace('    plain(pack);','    deck174::paths::read_file(pack);',1)
    adapted=adapted.replace('#pragma once\n','#pragma once\n#include "../../native/deck/proton_paths174.h"\n',1)
    if ui.count('#include "pack165.h"')!=1 or ui.count('workshop165::plain(p);')!=1:
        raise ValueError('UI reader path boundary differs')
    adapted_ui=ui.replace('#include "pack165.h"','#include "pack174_generated.h"',1)
    adapted_ui=adapted_ui.replace('workshop165::plain(p);','deck174::paths::read_directory(p);',1)
    # Restore the deliberate substitutions and compare every other character.
    restored=adapted.replace('#include "../../native/deck/proton_paths174.h"\n','',1)
    restored=restored.replace('    deck174::paths::read_file(pack);','    plain(pack);',1)
    restored=restored.replace(new_guard,original_guard,1)
    restored_ui=adapted_ui.replace('#include "pack174_generated.h"','#include "pack165.h"',1)
    restored_ui=restored_ui.replace('deck174::paths::read_directory(p);','workshop165::plain(p);',1)
    if restored!=pack or restored_ui!=ui:
        raise ValueError('Non-policy reader bytes changed')
    folder=snapshot/'tools/deck'
    targets={'pack174_generated.h':adapted,'ui_compat174_generated.h':adapted_ui}
    for name,body in targets.items():
        with (folder/name).open('x',encoding='utf-8',newline='\n') as stream:
            stream.write(body)
    return {'shared_inputs':PINNED,'parser_and_fingerprint_logic_unchanged':True,
        'generated':{name:hashlib.sha256((folder/name).read_bytes()).hexdigest() for name in targets},
        'normal_Windows_runtime_sources_modified':False}
