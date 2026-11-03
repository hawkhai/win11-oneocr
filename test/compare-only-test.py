"""Real CLI verification of an indexed Linux record, without running inference."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
REPORT=ROOT/'reports/review-fixes-2026-11-03'


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def tree(path):return {p.relative_to(path).as_posix():sha(p) for p in path.rglob('*') if p.is_file()}


class CompareOnlyCli(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='oneocr-compare-only-')
        cls.root=Path(cls.temp.name);cls.record=cls.root/'linux-execution'
        shutil.copytree(REPORT/'linux-node',cls.record)
        index=json.loads((REPORT/'FILES.sha256.json').read_text(encoding='utf8'))
        subset={name.removeprefix('linux-node/'):value for name,value in index.items() if name.startswith('linux-node/')}
        (cls.record/'FILES.sha256.json').write_text(json.dumps(subset),encoding='utf8')
        cls.before=tree(cls.record)

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def invoke(self,*args):
        return subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/run-wasm-parity.py','--compare-only','--output',self.record,*args],
                              cwd=ROOT,capture_output=True,text=True)

    def test_separate_verification_preserves_all_execution_files(self):
        destination=self.root/'verification'
        result=self.invoke('--verification-output',str(destination))
        self.assertEqual(result.returncode,0,result.stderr)
        data=json.loads((destination/'verification.json').read_text(encoding='utf8'))
        self.assertEqual(data['mode'],'saved-results-verification');self.assertFalse(data['fresh_inference'])
        self.assertEqual(data['passed_cases'],138)
        self.assertEqual(data['original_scheduling']['jobs'],3)
        self.assertIn('longest previous',data['original_scheduling']['method'])
        original=json.loads((self.record/'cases.json').read_text(encoding='utf8'))
        self.assertTrue(original[0]['path'].startswith('/mnt/'))
        self.assertEqual(tree(self.record),self.before)
        self.assertFalse((destination/'summary.json').exists())

    def test_missing_or_nested_or_execution_options_fail_without_writes(self):
        for args in [[],['--verification-output',str(self.record/'nested')],
                     ['--verification-output',str(self.root/'bad-options'),'--jobs','4']]:
            result=self.invoke(*args)
            self.assertNotEqual(result.returncode,0)
            self.assertEqual(tree(self.record),self.before)
        self.assertFalse((self.root/'bad-options').exists())


if __name__=='__main__':unittest.main()
