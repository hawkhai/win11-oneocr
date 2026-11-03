"""Both saved-results entrypoints must reject conflicting or extra outcomes."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from regression_reference import active_archive

ROOT=Path(__file__).resolve().parents[1]


class ArchiveVerification(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='oneocr-archive-test-')
        cls.root=Path(cls.temp.name);cls.report=cls.root/'report'
        shutil.copytree(ROOT/active_archive(),cls.report)
        cls.case_count=json.loads((cls.report/'windows-node/summary.json').read_text(encoding='utf8'))['cases']
        # Only inputs are redirected. Both real CLI entrypoints still use the
        # original sealed outputs, Git history, runtime assets and comparator.
        cls.inputs=cls.root/'inputs'
        for name in ['samples','research/case/corpus']:
            shutil.copytree(ROOT/name,cls.inputs/name)
        (cls.inputs/'models').mkdir()
        shutil.copyfile(ROOT/'models/manifest.json',cls.inputs/'models/manifest.json')
        shutil.copyfile(ROOT/'ocr-book.jpg',cls.inputs/'ocr-book.jpg')
        manifest=json.loads((ROOT/'research/delivery-manifest.json').read_text(encoding='utf8'))
        reference=manifest['reference']
        for name in {reference['all_samples_baseline'],reference['corpus_baseline']}:
            shutil.copytree(ROOT/name,cls.inputs/name)
        if 'parameter_manifest' in reference:
            shutil.copytree(ROOT/reference['parameter_baseline'],cls.inputs/reference['parameter_baseline'])
            target=cls.inputs/reference['parameter_manifest'];target.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(ROOT/reference['parameter_manifest'],target)
        target=cls.inputs/reference['book_baseline'];target.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(ROOT/reference['book_baseline'],target)

    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()

    def invoke(self,kind,output,inputs=False):
        if kind=='all':args=['scripts/verify-recorded-parity.py','--report',self.report,'--output',output]
        else:args=['scripts/run-wasm-parity.py','--compare-only','--output',self.report/'windows-node','--verification-output',output]
        if inputs:
            wrapper="""import importlib.util,sys
from pathlib import Path
script,inputs,*args=sys.argv[1:]
sys.path.insert(0,str(Path(script).parent))
spec=importlib.util.spec_from_file_location('real_cli',script)
tool=importlib.util.module_from_spec(spec);spec.loader.exec_module(tool)
original=tool.current_cases
tool.current_cases=lambda root,reference:original(Path(inputs),reference)
sys.argv=[script,*args]
sys.exit(tool.main())
"""
            return subprocess.run([sys.executable,'-X','utf8','-c',wrapper,str(ROOT/args[0]),str(self.inputs),*args[1:]],
                                  cwd=ROOT,capture_output=True,text=True)
        return subprocess.run([sys.executable,'-X','utf8',*args],cwd=ROOT,capture_output=True,text=True)

    def test_current_input_snapshot_passes_both_entrypoints(self):
        for kind in ['all','single']:
            result=self.invoke(kind,self.root/('snapshot-'+kind),inputs=True)
            self.assertEqual(result.returncode,0,result.stderr)

    def test_parameter_inventory_or_frozen_options_drift_is_rejected(self):
        manifest=json.loads((ROOT/'research/delivery-manifest.json').read_text(encoding='utf8'));reference=manifest['reference']
        path=self.inputs/reference['parameter_manifest'];original=path.read_bytes();value=json.loads(original)
        value['cases'][0]['options']['max_lines']=2;path.write_text(json.dumps(value),encoding='utf8')
        try:
            for kind in ['all','single']:
                output=self.root/('parameter-inventory-'+kind);result=self.invoke(kind,output,inputs=True)
                self.assertNotEqual(result.returncode,0);self.assertIn('Parameter inventory fingerprint changed',result.stderr);self.assertFalse(output.exists())
        finally:path.write_bytes(original)
        first=json.loads(original)['cases'][0];path=self.inputs/reference['parameter_baseline']/(first['name']+'.contract.json');original=path.read_bytes()
        value=json.loads(original);value['requested_options']['max_lines']=True;path.write_text(json.dumps(value),encoding='utf8')
        try:
            for kind in ['all','single']:
                output=self.root/('parameter-options-'+kind);result=self.invoke(kind,output,inputs=True)
                self.assertNotEqual(result.returncode,0);self.assertIn('Frozen parameter options changed',result.stderr);self.assertFalse(output.exists())
        finally:path.write_bytes(original)

    def test_changed_current_images_fail_without_publishing(self):
        inventory=json.loads((self.inputs/'samples/manifest.json').read_text(encoding='utf8'))
        for name,path in [('sample',self.inputs/'samples'/inventory['cases'][0]['file']),
                          ('corpus',self.inputs/'research/case/corpus/latin-36.png'),('book',self.inputs/'ocr-book.jpg')]:
            original=path.read_bytes();path.write_bytes(b'changed original input')
            try:
                for kind in ['all','single']:
                    output=self.root/('changed-'+name+'-'+kind);result=self.invoke(kind,output,inputs=True)
                    self.assertNotEqual(result.returncode,0,result.stdout)
                    self.assertRegex(result.stderr,'fingerprint changed|input scope differs')
                    self.assertFalse(output.exists())
            finally:path.write_bytes(original)

    def test_extra_or_removed_inputs_fail_without_publishing(self):
        for name in ['samples']:
            directory=self.inputs/name;extra=directory/'review-extra.png';extra.write_bytes(b'new sample')
            try:
                for kind in ['all','single']:
                    output=self.root/(directory.name+'-extra-'+kind);result=self.invoke(kind,output,inputs=True)
                    self.assertNotEqual(result.returncode,0,result.stdout);self.assertIn('inventory changed',result.stderr)
                    self.assertFalse(output.exists())
            finally:extra.unlink()
        path=self.inputs/'research/case/corpus/manifest.json';original=path.read_bytes()
        try:
            inventory=json.loads(original)
            for action in ['add','remove']:
                changed=json.loads(original)
                if action=='add':changed['cases'].append({**inventory['cases'][0],'name':'new-corpus-case'})
                else:changed['cases'].pop()
                path.write_text(json.dumps(changed,ensure_ascii=False),encoding='utf8')
                for kind in ['all','single']:
                    output=self.root/(action+'-case-'+kind);result=self.invoke(kind,output,inputs=True)
                    self.assertNotEqual(result.returncode,0,result.stdout);self.assertIn('input scope differs',result.stderr)
                    self.assertFalse(output.exists())
        finally:path.write_bytes(original)

    def test_unmodified_archive_passes_both_entrypoints(self):
        for kind,count in [('all',self.case_count*5),('single',self.case_count)]:
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
