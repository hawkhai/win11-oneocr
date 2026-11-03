"""Saved verification must preserve history and reject stale or unidentified records."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from verification_state import checkpoint,destination_for_verification,load_execution,verify_execution,sha,check_result_files


def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')


class ExecutionRecords(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.directory=self.root/'run';self.models=self.root/'models'
        self.models.mkdir();(self.models/'weight.bin').write_bytes(b'original weight')
        write(self.models/'manifest.json',{'models':[{'file':'weight.bin','sha256':sha(self.models/'weight.bin')}]})
        self.build={'model_manifest_sha256':sha(self.models/'manifest.json'),'model_source_sha256':'source',
                    'files':{'oneocr-core.wasm':'original-wasm','oneocr.mjs':'original-loader','share/doc.md':'old-doc'}}
        self.golden=self.root/'baseline.json';write(self.golden,{'baseline':'original'})
        self.cases=[{'name':'one','path':'/original-linux/input.png','sha256':'input','baseline':str(self.golden)}]
        self.schedule={'method':'longest previous duration first; scheduling only','jobs':3}
        for name,value in [('cases.json',self.cases),('execution-cases.json',self.cases),('execution-build.json',self.build),
                           ('actual/measurements.json',{'jobs':3,'cases':[{'name':'one','milliseconds':123}]}),('actual/one.json',{'result':'original'})]:
            write(self.directory/name,value)
        write(self.directory/'summary.json',{'mode':'fresh-inference','fresh_inference':True,
                                           'model_manifest_sha256':self.build['model_manifest_sha256'],'wasm_build':self.build,
                                           'scheduling':self.schedule,'results':[{'name':'one','baseline_sha256':sha(self.golden)}]})
        write(self.directory/'execution.json',checkpoint(self.directory,self.build,self.cases,self.schedule,None))

    def test_read_only_verification_preserves_original_context(self):
        before={p.relative_to(self.directory):p.read_bytes() for p in self.directory.rglob('*') if p.is_file()}
        record=load_execution(self.directory);current=copy.deepcopy(self.cases);current[0]['path']='F:/another-host/input.png'
        verify_execution(record,current,self.build,self.models)
        self.assertEqual(record['state']['scheduling'],self.schedule)
        self.assertEqual(record['state']['cases'][0]['path'],'/original-linux/input.png')
        self.assertEqual(before,{p.relative_to(self.directory):p.read_bytes() for p in self.directory.rglob('*') if p.is_file()})

    def test_runtime_change_requires_new_inference(self):
        record=load_execution(self.directory)
        for name in ['oneocr-core.wasm','oneocr.mjs']:
            current=copy.deepcopy(self.build);current['files'][name]='another runtime'
            with self.assertRaises(ValueError):verify_execution(record,self.cases,current,self.models)
        current=copy.deepcopy(self.build);current['files']['new-module.mjs']='new'
        with self.assertRaises(ValueError):verify_execution(record,self.cases,current,self.models)
        current=copy.deepcopy(self.build);current['files']['share/doc.md']='updated-doc'
        verify_execution(record,self.cases,current,self.models)

    def test_changed_execution_output_or_inventory_is_rejected(self):
        for name in ['actual/one.json','cases.json','summary.json','actual/measurements.json']:
            path=self.directory/name;before=path.read_bytes();path.write_bytes(before+b' ')
            with self.assertRaises(ValueError):load_execution(self.directory)
            path.write_bytes(before)

    def test_model_and_baseline_and_input_scope_are_bound(self):
        record=load_execution(self.directory)
        current=copy.deepcopy(self.build);current['model_manifest_sha256']='another'
        with self.assertRaises(ValueError):verify_execution(record,self.cases,current,self.models)
        (self.models/'weight.bin').write_bytes(b'changed')
        with self.assertRaises(ValueError):verify_execution(record,self.cases,self.build,self.models)
        (self.models/'weight.bin').write_bytes(b'original weight')
        self.golden.write_text('{}')
        with self.assertRaises(ValueError):verify_execution(record,self.cases,self.build,self.models)
        changed=copy.deepcopy(self.cases);changed[0]['sha256']='different input'
        with self.assertRaises(ValueError):verify_execution(record,changed,self.build,self.models)

    def test_execution_json_cannot_override_sealed_metadata(self):
        path=self.directory/'execution.json';original=path.read_bytes()
        changes=[lambda x:x['build']['files'].update({'oneocr-core.wasm':'never executed'}),
                 lambda x:x['cases'][0].update(path='F:/rewritten.png'),
                 lambda x:x['scheduling'].update(method='round-robin'),
                 lambda x:x['scheduling'].update(jobs=3.0),
                 lambda x:x['baselines'].update(one='different baseline'),
                 lambda x:x.update(browser_request_timeout_ms=1),lambda x:x.update(mode='saved-results-verification')]
        for change in changes:
            with self.subTest(change=change):
                state=json.loads(original);change(state);write(path,state)
                with self.assertRaisesRegex(ValueError,'metadata disagree'):load_execution(self.directory)
                path.write_bytes(original)

    def test_conflicting_and_extra_results_are_rejected(self):
        for name in ['one.error.json','unknown.json']:
            path=self.directory/'actual'/name;write(path,{'code':-31})
            with self.assertRaises(ValueError):check_result_files(self.directory,self.cases)
            with self.assertRaises(ValueError):load_execution(self.directory)
            path.unlink()

    def test_schema_version_does_not_accept_boolean_or_float_aliases(self):
        path=self.directory/'execution.json';original=path.read_bytes()
        for version in [True,1.0,'1',2]:
            state=json.loads(original);state['schema_version']=version;write(path,state)
            with self.assertRaisesRegex(ValueError,'Unsupported execution record'):load_execution(self.directory)
            path.write_bytes(original)

    def test_native_result_inventory_uses_the_same_conflict_rule(self):
        native=self.root/'native';write(native/'book.json',{'result':'ok'})
        cases=[{'name':'book','sha256':'input'}];check_result_files(native,cases,native=True)
        write(native/'book.error.json',{'code':-31})
        with self.assertRaises(ValueError):check_result_files(native,cases,native=True)

    def test_verification_destination_must_be_separate_and_fresh(self):
        for destination in [self.directory,self.directory/'verify',self.root]:
            with self.assertRaises(ValueError):destination_for_verification(self.directory,destination)
        self.assertEqual(destination_for_verification(self.directory,self.root/'new'),self.root/'new')
        occupied=self.root/'occupied';occupied.mkdir();(occupied/'data').write_bytes(b'keep')
        with self.assertRaises(ValueError):destination_for_verification(self.directory,occupied)

    def test_legacy_requires_a_complete_report_index(self):
        (self.directory/'execution.json').unlink()
        with self.assertRaises(ValueError):load_execution(self.directory)
        files={p.relative_to(self.directory).as_posix():{'sha256':sha(p)} for p in self.directory.rglob('*') if p.is_file()}
        write(self.directory/'FILES.sha256.json',files)
        record=load_execution(self.directory);verify_execution(record,self.cases,self.build,self.models)
        self.assertIn('legacy',record['integrity'])


if __name__=='__main__':unittest.main()
