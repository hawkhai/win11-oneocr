"""Both saved-results entrypoints must reject conflicting or extra outcomes."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]


class ArchiveVerification(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='oneocr-archive-test-')
        cls.root=Path(cls.temp.name);cls.report=cls.root/'report'
        shutil.copytree(ROOT/'reports/review-fixes-2026-11-03',cls.report)

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def invoke(self,kind,output):
        if kind=='all':args=['scripts/verify-recorded-parity.py','--report',self.report,'--output',output]
        else:args=['scripts/run-wasm-parity.py','--compare-only','--output',self.report/'windows-node','--verification-output',output]
        return subprocess.run([sys.executable,'-X','utf8',*args],cwd=ROOT,capture_output=True,text=True)

    def test_unmodified_archive_passes_both_entrypoints(self):
        for kind,count in [('all',690),('single',138)]:
            output=self.root/('valid-'+kind);result=self.invoke(kind,output)
            self.assertEqual(result.returncode,0,result.stderr)
            data=json.loads((output/'verification.json').read_text(encoding='utf8'))
            self.assertEqual(data['passed_cases'],count);self.assertFalse(data['fresh_inference'])

    def test_conflicting_and_extra_json_fail_without_publishing(self):
        for filename in ['corpus-latin-36.error.json','unregistered.json']:
            path=self.report/'windows-node/actual'/filename;path.write_text(json.dumps({'code':-31,'message':'later failure'}))
            try:
                for kind in ['all','single']:
                    output=self.root/(filename+'-'+kind)
                    result=self.invoke(kind,output)
                    self.assertNotEqual(result.returncode,0,result.stdout)
                    self.assertFalse(output.exists())
            finally:path.unlink()

    def test_native_conflict_is_rejected_too(self):
        path=self.report/'linux-native/book.error.json';path.write_text('{"code":-31}')
        try:
            output=self.root/'bad-native';result=self.invoke('all',output)
            self.assertNotEqual(result.returncode,0,result.stdout);self.assertFalse(output.exists())
        finally:path.unlink()


if __name__=='__main__':unittest.main()
