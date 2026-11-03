"""Private RTS integration must not weaken the released engine admission."""
from pathlib import Path
import json
import shutil
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'tools/deck')]
from build_frontend import source_files
import adapt_runtime175
import extend_contract175


class PrivateIntegrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder=tempfile.TemporaryDirectory()
        cls.snapshot=Path(cls.folder.name)/'source'
        for source in source_files(ROOT):
            target=cls.snapshot/source.relative_to(ROOT)
            target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(source,target)
        cls.adaptation=adapt_runtime175.generate(cls.snapshot)

    @classmethod
    def tearDownClass(cls):
        cls.folder.cleanup()

    def test_private_pins_keep_every_existing_guard(self):
        before=json.loads((ROOT/'tools/data/engine-contract172.json').read_text())
        after=extend_contract175.extend(self.snapshot)
        for name in ('contract_id','regions','sections','generated_header_sha256'):
            self.assertEqual(before[name],after[name])
        self.assertTrue(after['deck175_dependency_review']['added_sources'])
        self.assertEqual(before['bindings'],after['bindings'][:len(before['bindings'])])

    def test_new_unprotected_reference_is_refused(self):
        value=json.loads((ROOT/'tools/data/engine-contract172.json').read_text())
        missing=next(address for section in value['sections']
                     for address in range(section['rva'],section['rva']+section['size'],16)
                     if not extend_contract175.protected(address,value))
        path=self.snapshot/'native/deck/adapter175.h';original=path.read_bytes()
        try:
            path.write_bytes(original+('\n// deliberate new reference '+hex(missing)+'\n').encode())
            with self.assertRaisesRegex(ValueError,'outside existing immutable guards'):
                extend_contract175.extend(self.snapshot)
        finally:path.write_bytes(original)

    def test_changed_existing_native_guard_is_refused(self):
        path=self.snapshot/'native/common/engine_contract172_generated.h';original=path.read_bytes()
        try:
            path.write_bytes(original+b'\n// changed\n')
            with self.assertRaisesRegex(ValueError,'guard header differs'):
                extend_contract175.extend(self.snapshot)
        finally:path.write_bytes(original)

    def test_unrelated_existing_gameplay_change_is_refused(self):
        path=self.snapshot/'native/outputs/shogun2-accuracy90-preview/src/unit_weapon131.h'
        original=path.read_bytes()
        try:
            path.write_bytes(original+b'\n// changed\n')
            with self.assertRaisesRegex(ValueError,'Unreviewed source integration scope'):
                extend_contract175.extend(self.snapshot)
        finally:path.write_bytes(original)

    def test_normal_source_cannot_be_the_snapshot(self):
        with self.assertRaisesRegex(ValueError,'separate private source snapshot'):
            extend_contract175.extend(ROOT)
        with self.assertRaisesRegex(ValueError,'isolated source snapshot'):
            adapt_runtime175.generate(ROOT)

    def test_adapter_and_extractor_substitutions_are_fully_reversible(self):
        for name in ('adapter','extractor','entry'):
            self.assertTrue(self.adaptation[name]['exact_reverse_comparison'])
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'source.cpp';path.write_text('anchor\nanchor\n')
            with self.assertRaisesRegex(ValueError,'Unreviewed integration anchor'):
                adapt_runtime175.substitute(path,[('anchor','replacement')])
            self.assertEqual(path.read_text(),'anchor\nanchor\n')

    def test_cache176_review_accepts_only_validator_release_and_exact_deployment_gate(self):
        import adapt_cache176
        path=self.snapshot/adapt_cache176.SOURCE;original=path.read_bytes()
        try:
            review=adapt_cache176.generate(self.snapshot)
            self.assertTrue(review['exact_reverse_comparison'])
            self.assertTrue(review['initialized_battle_camera_gate'])
            with self.assertRaisesRegex(ValueError,'Unreviewed source integration scope'):
                extend_contract175.extend(self.snapshot)
            result=extend_contract175.extend(self.snapshot,cache_fix176=True)
            self.assertEqual(result['deck176_cache_lifetime_review'],review)
            valid=path.read_bytes()
            path.write_text(path.read_text().replace('return deck181::battle_ready(base,local_u32);','return true;'))
            with self.assertRaisesRegex(ValueError,'Unreviewed cache/deployment'):
                extend_contract175.extend(self.snapshot,cache_fix176=True)
            path.write_bytes(valid)
            path.write_bytes(path.read_bytes()+b'\n// unrelated change\n')
            with self.assertRaisesRegex(ValueError,'Unreviewed cache/deployment'):
                extend_contract175.extend(self.snapshot,cache_fix176=True)
        finally:path.write_bytes(original)


if __name__=='__main__':unittest.main()
