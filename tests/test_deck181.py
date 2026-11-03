"""Contextual native action-set boundaries; no Steam/game/hardware access."""
from pathlib import Path
import sys,unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools/deck'))
import native_defaults181 as native
class ContextBindingsTests(unittest.TestCase):
 def setUp(self):self.manifest,self.bindings=native.values();self.mapping=native.parse(self.bindings)[0][1]
 def test_four_distinct_complete_action_sets(self):
  actions=dict(dict(self.mapping)['actions']);self.assertEqual(set(actions),{n+'181'for n in native.SETS})
  keys=[]
  for values in actions.values():
   for kind,items in values:
    if kind=='Button':keys.extend(k for k,v in items)
  self.assertEqual(len(keys),len(set(keys)))
 def test_each_preset_references_only_its_own_groups(self):
  groups={dict(v)['id']:v for k,v in self.mapping if k=='group'}
  presets=[dict(v)for k,v in self.mapping if k=='preset'];self.assertEqual(len(presets),4)
  seen=set()
  for p in presets:
   ids={k for k,v in p['group_source_bindings']};self.assertTrue(ids);self.assertTrue(ids<=set(groups));self.assertFalse(ids&seen);seen|=ids
 def test_generated_binding_is_native_only(self):
  native.validate(self.manifest,self.bindings)
  self.assertNotIn('key_press ',self.bindings.lower());self.assertNotIn('xinput_',self.bindings.lower())
 def test_manifest_round_trip_does_not_lose_repeated_groups(self):
  from native_defaults174 import node
  self.assertEqual(native.parse(node(native.parse(self.bindings))),native.parse(self.bindings))
 def test_all_localization_tokens_resolve_to_readable_labels(self):
  import re
  for text in (self.manifest,self.bindings):
   body=native.parse(text)[0][1]
   english=dict(dict(dict(body)['localization'])['english'])
   self.assertEqual(len(english),sum(len(native.labels(c))+1 for c in native.SETS))
   for token in re.findall(r'"#([^"\\]+)"',text):
    self.assertIn(token,english);self.assertTrue(english[token].strip())
   for context in native.SETS:
    self.assertEqual(english[context+'181'],native.SET_TITLES[context])
    for name in native.labels(context):
     self.assertNotEqual(english[context+'_'+name],name)
 def test_context_specific_labels_do_not_promise_unused_commands(self):
  labels={c:native.labels(c)for c in native.SETS}
  self.assertNotIn('Y',labels['Menu'])
  self.assertEqual(labels['Possession']['B'],'Mount / dismount unit')
  self.assertEqual(labels['Campaign']['X'],'Zoom home')
  self.assertEqual(labels['RTS']['X'],'Toggle run / walk')
  self.assertNotIn('R3',labels['RTS']);self.assertEqual(labels['RTS']['Y'],'Possess soldier')
  self.assertEqual(labels['Possession']['R3'],'Change first / third person (hold)')
  self.assertEqual(labels['Possession']['Left'],'Third-person camera closer')
 def test_display_only_semantic_filter_retains_bindings(self):
  changed=self.bindings.replace('"Select"','"Choose"')
  self.assertEqual(native.binding_semantics(changed),native.binding_semantics(self.bindings))
  changed=self.bindings.replace('game_action Menu181 Menu_A','game_action Menu181 Menu_B')
  self.assertNotEqual(native.binding_semantics(changed),native.binding_semantics(self.bindings))
 def test_no_dummy_gameplay_actions_are_advertised(self):
  self.assertNotIn('Not assigned',self.manifest)
  self.assertNotIn('Menu_RightStick',self.manifest)
  self.assertNotIn('Campaign_R3',self.manifest)
  self.assertIn('Run (hold)',self.manifest)
 def test_updated_layout_revisions_match_and_do_not_force_legacy_layouts(self):
  manifest=dict(native.parse(self.manifest)[0][1]);config=dict(self.mapping)
  self.assertEqual(config['title'],'Unit Possession Layout')
  for key,wanted in [('major_revision','3'),('minor_revision','6')]:
   self.assertEqual(manifest[key],wanted);self.assertEqual(config[key],wanted)
  self.assertEqual(manifest['major_revision_affects_xinput'],'0');self.assertEqual(manifest['major_revision_affects_mousekb'],'0')
 def test_y_is_sole_possession_entry_and_r3_label_only_changes_view(self):
  self.assertNotIn('RTS_R3',self.manifest)
  self.assertEqual(native.labels('RTS')['Y'],'Possess soldier')
  self.assertEqual(native.labels('Possession')['Y'],'Return to RTS')
  self.assertEqual(native.labels('Possession')['R3'],'Change first / third person (hold)')
 def test_right_pad_click_is_unbound_in_every_context(self):
  for context in native.SETS:
   self.assertNotIn('RightPadClick',native.labels(context))
   self.assertNotIn('game_action '+context+'181 '+context+'_RightPadClick',self.bindings)
  self.assertNotIn('RightPadClick',self.manifest)
 def test_cpp_required_channels_match_real_action_declarations(self):
  import re
  text=(ROOT/'native/deck/binding_profiles182.h').read_text()
  buttons={c:int(v)for c,v in re.findall(r'case deck174::Context::(\w+):return (\d+)ull;',text)}
  analogs={c:int(v)for c,v in re.findall(r'case deck174::Context::(\w+):return (\d+)u;',text)}
  for c in native.SETS:
   self.assertEqual(buttons[c],sum(1<<i for i,k in enumerate(native.old.BUTTONS)if k in native.labels(c)))
   self.assertEqual(analogs[c],sum(1<<i for i,k in enumerate(native.old.ANALOG)if k in native.labels(c)))
if __name__=='__main__':unittest.main()
