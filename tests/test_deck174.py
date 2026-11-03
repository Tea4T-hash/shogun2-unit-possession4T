"""Deck defaults remain native, complete, and independent of user Steam files."""
from pathlib import Path
import importlib.util
import tempfile
import unittest
import sys
import os
import subprocess
import re

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('deck_defaults174',ROOT/'tools/deck/native_defaults174.py')
defaults=importlib.util.module_from_spec(spec);spec.loader.exec_module(defaults)
sys.path.insert(0,str(ROOT/'tools'))
from build_frontend import source_files
from package_source import selected
build_spec=importlib.util.spec_from_file_location('deck_build174',ROOT/'tools/deck/build174.py')
builder=importlib.util.module_from_spec(build_spec);build_spec.loader.exec_module(builder)


class DeckDefaultsTests(unittest.TestCase):
    def test_stick_clicks_use_digital_presses_and_pads_keep_pressure(self):
        # Parse the shipped text and follow each active physical source to its
        # actual group. A flat action-name check missed the old Soft_Press bug.
        tokens=iter(re.findall(r'"(?:\\.|[^"\\])*"|[{}]',defaults.configuration()))
        def pairs(nested=False):
            result=[]
            for key in tokens:
                if key=='}':
                    self.assertTrue(nested)
                    return result
                self.assertTrue(key.startswith('"'))
                value=next(tokens)
                result.append((key[1:-1],pairs(True) if value=='{' else value[1:-1]))
            self.assertFalse(nested)
            return result
        def one(nodes,key):
            found=[v for k,v in nodes if k==key]
            self.assertEqual(len(found),1)
            return found[0]
        mapping=one(pairs(),'controller_mappings')
        groups={one(v,'id'):v for k,v in mapping if k=='group'}
        sources={v:k for k,v in one(one(mapping,'preset'),'group_source_bindings')}
        for source,action,activator in (
            ('joystick','L3','Full_Press'),('right_joystick','R3','Full_Press'),
            ('left_trackpad','LeftPadClick','Soft_Press'),('right_trackpad','RightPadClick','Soft_Press')):
            group=groups[sources[source+' active']]
            self.assertEqual(one(group,'mode'),'joystick_move')
            click=one(one(group,'inputs'),'click')
            activators=one(click,'activators')
            self.assertEqual([k for k,v in activators],[activator])
            bindings=one(one(activators,activator),'bindings')
            self.assertEqual(bindings,[('binding','game_action '+defaults.SET+' '+action)])

    def test_all_four_grips_are_distinct_native_actions(self):
        s=defaults.configuration();defaults.validate(s)
        for action in ('L4','L5','R4','R5'):
            self.assertEqual(s.count('"game_action DeckHardware174 '+action+'"'),1)
        self.assertNotIn('xinput',s.lower())

    def test_system_buttons_are_not_bound(self):
        s=defaults.configuration()
        for token in ('button_steam','button_guide','button_quick_access','steam_action'):
            self.assertNotIn(token,s.lower())

    def test_no_emulation_or_external_radial(self):
        s=defaults.configuration()
        for value in ('key_press W','mouse_button LEFT','xinput_button A','steam_action 1','radial_menu'):
            with self.assertRaises(ValueError):defaults.validate(s+'\n'+value)

    def test_manifest_is_deck_only_and_uses_relative_native_config(self):
        s=defaults.manifest()
        self.assertIn('"controller_neptune"',s)
        self.assertNotIn('controller_xbox',s)
        self.assertIn('"path" "deck-native174.vdf"',s)
        for action in defaults.ANALOG+defaults.BUTTONS:
            self.assertIn('"'+action+'"',s)

    def test_new_output_preserves_existing_files(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'new';first=defaults.write(p)
            with self.assertRaises(FileExistsError):defaults.write(p)
            for name,text in first.items():self.assertEqual((p/name).read_text(),text)

    def test_portable_source_contains_exact_default_and_build_tools(self):
        default=ROOT/'native/deck/controller174.ini'
        self.assertIn(default,set(source_files(ROOT)))
        exported=set(selected(ROOT))
        for path in (default,ROOT/'tools/deck/build174.py',ROOT/'tools/deck/check_abi174.py',
                     ROOT/'docs/development/SteamDeck174.md'):
            self.assertIn(path,exported)

    def test_path_adapter_preserves_shared_parser_and_refuses_unreviewed_source(self):
        import shutil
        import adapt_paths174
        with tempfile.TemporaryDirectory() as folder:
            snapshot=Path(folder);(snapshot/'tools/workshop').mkdir(parents=True)
            (snapshot/'tools/deck').mkdir()
            for name in adapt_paths174.PINNED:
                shutil.copyfile(ROOT/'tools/workshop'/name,snapshot/'tools/workshop'/name)
            receipt=adapt_paths174.generate(snapshot)
            self.assertTrue(receipt['parser_and_fingerprint_logic_unchanged'])
            self.assertFalse(receipt['normal_Windows_runtime_sources_modified'])
            for name in adapt_paths174.PINNED:
                self.assertEqual((snapshot/'tools/workshop'/name).read_bytes(),(ROOT/'tools/workshop'/name).read_bytes())
            with self.assertRaises(FileExistsError):adapt_paths174.generate(snapshot)
            with (snapshot/'tools/workshop/pack165.h').open('ab') as f:f.write(b'// changed')
            with self.assertRaises(ValueError):adapt_paths174.generate(snapshot)

    @unittest.skipUnless(os.name=='nt','Tests actual Windows command exit propagation')
    def test_native_runner_refuses_positive_and_signed_fault_exit_codes(self):
        with tempfile.TemporaryDirectory() as d:
            folder=Path(d);vcvars=folder/'fixture.cmd';vcvars.write_text('@exit /b 0\n')
            shell=os.environ.get('COMSPEC','cmd.exe')
            for code in (0,1,-1073741819):
                script=folder/('case'+str(code)+'.cmd')
                script.write_text(builder.batch_script(vcvars,[
                    [shell,'/d','/c','exit','/b',str(code)],
                    [shell,'/d','/c','echo','SECOND_COMMAND_REACHED']]))
                result=subprocess.run([shell,'/d','/c',script],capture_output=True,text=True)
                self.assertEqual(result.returncode==0,code==0)
                self.assertEqual('SECOND_COMMAND_REACHED' in result.stdout,code==0)
