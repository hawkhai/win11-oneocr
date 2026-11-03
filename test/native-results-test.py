"""The native acceptance entrypoint enforces native output, not Wasm parity."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class NativeAcceptance(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='oneocr-native-review-');cls.work=Path(cls.temp.name)
        cls.report=cls.work/'report';shutil.copytree(ROOT/'reports/photos-intersection-p2-2026-11-03',cls.report)
    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()
    def invoke(self,name,current=False):
        args=[] if current else ['--reference',ROOT/'research/references/options-handwriting-2026-11-03.json']
        return subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/verify-native-results.py','--report',self.report,'--output',self.work/name,*args],cwd=ROOT,capture_output=True,text=True)
    def test_historical_execution_cannot_pass_as_expanded_current_scope(self):
        result=self.invoke('stale-current-scope',current=True)
        self.assertNotEqual(result.returncode,0);self.assertIn('Native input/option scope changed',result.stderr)
        self.assertFalse((self.work/'stale-current-scope').exists())
    def seal(self):
        index=self.report/'FILES.sha256.json'
        index.write_text(json.dumps({p.relative_to(self.report).as_posix():{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'size':p.stat().st_size} for p in self.report.rglob('*') if p.is_file() and p!=index}),encoding='utf8')
    def test_wasm_diagnostic_difference_does_not_block_native(self):
        p=self.report/'diagnostics/wasm/nonblocking-test.json';p.parent.mkdir(parents=True,exist_ok=True)
        p.write_text('{"passed":false,"diagnostic_only":true}',encoding='utf8');self.seal()
        try:
            result=self.invoke('wasm-difference');self.assertEqual(result.returncode,0,result.stderr)
            value=json.loads((self.work/'wasm-difference/verification.json').read_text(encoding='utf8'))
            self.assertEqual(value['passed_cases'],442);self.assertFalse(value['wasm_is_acceptance_gate'])
            self.assertFalse(value['current_scope'])
        finally:p.unlink();self.seal()
    def test_native_coordinate_deviation_remains_failure(self):
        p=self.report/'windows-native/book.json';original=p.read_bytes();value=json.loads(original)
        value['lines'][0]['words'][0]['bounding_box'][0]+=2.1;p.write_text(json.dumps(value),encoding='utf8');self.seal()
        try:
            result=self.invoke('native-deviation');self.assertEqual(result.returncode,1,result.stderr)
            value=json.loads((self.work/'native-deviation/verification.json').read_text(encoding='utf8'))
            self.assertFalse(value['passed']);self.assertEqual(value['passed_cases'],441)
        finally:p.write_bytes(original);self.seal()
    def test_conflicting_native_error_is_rejected_before_output(self):
        p=self.report/'windows-native/book.error.json';p.write_text('{}',encoding='utf8');self.seal()
        try:
            result=self.invoke('native-conflict');self.assertNotEqual(result.returncode,0)
            self.assertIn('Conflicting error file',result.stderr);self.assertFalse((self.work/'native-conflict').exists())
        finally:p.unlink();self.seal()

if __name__=='__main__':unittest.main()
