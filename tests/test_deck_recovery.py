"""Recovery download/archive boundaries; no network, compiler or game access."""
from pathlib import Path
import importlib.util,io,tarfile,tempfile,unittest
SPEC=importlib.util.spec_from_file_location('deck_recovery',Path(__file__).resolve().parents[1]/'tools/deck/recover_dependencies.py')
MODULE=importlib.util.module_from_spec(SPEC);SPEC.loader.exec_module(MODULE)

class DeckRecoveryTests(unittest.TestCase):
 def archive(self,members):
  data=io.BytesIO()
  with tarfile.open(fileobj=data,mode='w:gz') as stream:
   for name,kind in members:
    item=tarfile.TarInfo(name)
    if kind=='link':item.type=tarfile.SYMTYPE;item.linkname='../../outside'
    else:item.size=3
    stream.addfile(item,io.BytesIO(b'abc') if kind!='link' else None)
  return data.getvalue()
 def test_wrong_dependency_hash_is_refused(self):
  with self.assertRaises(ValueError):MODULE.verified(b'wrong','0'*64,'fixture')
 def test_archive_traversal_rejected_before_any_member_is_written(self):
  with tempfile.TemporaryDirectory() as directory:
   out=Path(directory)
   with self.assertRaises(ValueError):MODULE.extract_lua(self.archive([('lua-5.1.5/COPYRIGHT','file'),('lua-5.1.5/../../outside','file')]),out)
   self.assertEqual(list(out.iterdir()),[])
 def test_archive_links_rejected_before_any_member_is_written(self):
  with tempfile.TemporaryDirectory() as directory:
   out=Path(directory)
   with self.assertRaises(ValueError):MODULE.extract_lua(self.archive([('lua-5.1.5/COPYRIGHT','file'),('lua-5.1.5/src/link','link')]),out)
   self.assertEqual(list(out.iterdir()),[])
 def test_float_patch_requires_original_config_and_updates_number_format(self):
  with self.assertRaises(ValueError):MODULE.patch_float('unexpected configuration')
  text=MODULE.patch_float('#define LUA_NUMBER_DOUBLE\n#define LUA_NUMBER\tdouble\n"%lf" "%.14g"')
  self.assertIn('#define LUA_NUMBER\tfloat',text);self.assertIn('"%f" "%.7g"',text)

if __name__=='__main__':unittest.main()
