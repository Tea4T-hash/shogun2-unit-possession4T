"""Failure/preservation cases for public boundaries; no game or tool invocation."""
import hashlib
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from game_packs import Pack,require_game
from package_source import audit,selected,verify_archive
from generate_tables import render

class ResourceBoundaryTests(unittest.TestCase):
    def pack(self,folder,entries):
        index=b''.join(struct.pack('<I',len(data))+name+b'\0' for name,data in entries)
        path=folder/'toy.pack';path.write_bytes(struct.pack('<4s5IQ',b'PFH3',1,0,0,len(entries),len(index),0)+index+b''.join(data for _,data in entries));return path

    def test_pack_integrity_and_preservation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);path=self.pack(root,[(b'ui/icon',b'abc')]);before=path.read_bytes();pack=Pack(path)
            self.assertEqual(pack.read('ui/icon',3,hashlib.sha256(b'abc').hexdigest()),b'abc')
            for size,digest in ((2,hashlib.sha256(b'abc').hexdigest()),(3,'0'*64)):
                with self.assertRaises(ValueError):pack.read('ui/icon',size,digest)
            self.assertEqual(path.read_bytes(),before)
            for rows in ([(b'../escape',b'x')],[(b'A',b'x'),(b'a',b'y')],[(b'C:/escape',b'x')]):
                with self.assertRaises(ValueError):Pack(self.pack(root,rows))
            path=self.pack(root,[(b'a',b'abc')]);path.write_bytes(path.read_bytes()[:-1])
            with self.assertRaises(ValueError):Pack(path)

    def test_unsupported_game_is_read_only(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/'Empire.Retail.dll';path.write_bytes(b'toy, not a game')
            with self.assertRaises(ValueError):require_game(path.parent)
            self.assertEqual(path.read_bytes(),b'toy, not a game')

    def test_public_payload_audit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);source=root/'safe.py';source.write_text('print("safe")\n')
            self.assertEqual(len(audit([source],root)),1)
            for raw in (b'MZfake',b'PK\x03\x04fake',('C:'+'\\Users\\private-user\\config').encode(),('ghp_'+'A'*35).encode()):
                source.write_bytes(raw)
                with self.assertRaises(ValueError):audit([source],root)
            asset=root/'asset.png';asset.write_bytes(b'text does not excuse asset extension')
            with self.assertRaises(ValueError):audit([asset],root)

    def test_canonical_observations(self):
        data=json.loads((ROOT/'tools/data/derived-records.json').read_text(encoding='utf-8'))
        for row in data['headers']:
            self.assertEqual((ROOT/row['path']).read_text(encoding='utf-8'),render(row))
        broken=dict(data['headers'][0]);broken['template']+='changed'
        with self.assertRaises(ValueError):render(broken)

    def test_source_kit_contains_developer_guides(self):
        paths = {p.relative_to(ROOT).as_posix() for p in selected()}
        self.assertTrue({
            'docs/README.md', 'native/README.md', 'tools/README.md',
            'docs/development/README.md', 'docs/releases/README.md',
            'docs/public-release/README.md',
        } <= paths)
        self.assertIn('docs/releases/v' + (ROOT / 'VERSION').read_text().strip() + '.md', paths)
        self.assertNotIn('docs/development/Test157.md', paths)
        self.assertNotIn('docs/releases/v0.31.md', paths)
        self.assertIn('docs/development/CampaignSetup209.md', paths)
        self.assertNotIn('tools/github_release.py', paths)

    def test_source_selection_uses_requested_root_version(self):
        # An exported or alternate source tree must select its own release notes,
        # rather than accidentally using this checkout's product version.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'VERSION').write_text('9.99\n')
            releases = root / 'docs/releases'
            releases.mkdir(parents=True)
            (releases / 'v9.99.md').write_text('fixture release')
            (releases / 'v0.32.md').write_text('other release')
            paths = {p.relative_to(root).as_posix() for p in selected(root)}
            self.assertIn('docs/releases/v9.99.md', paths)
            self.assertNotIn('docs/releases/v0.32.md', paths)

if __name__=='__main__':unittest.main()
