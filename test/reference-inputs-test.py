"""Reference cache reuse must never mix unverified weights with current vocabularies."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from reference_inputs import REFERENCE_LABELS, stage_reference_inputs
from verification_state import sha


def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,ensure_ascii=False),encoding='utf8')


def tree(root):return {p.relative_to(root).as_posix():p.read_bytes() for p in root.rglob('*') if p.is_file()}


class ReferenceInputs(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.models=self.root/'models';self.target=self.root/'cache'
        self.manifest={'source_assets':{'oneocr.onemodel':'frozen-source'},'models':[],'vocabularies':[]}
        for role,name in [('detector','detector/text_detector.onnx'),('classifier','classifier/script_classifier.onnx')]:
            path=self.models/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(role.encode())
            self.manifest['models'].append({'role':role,'file':name,'sha256':sha(path)})
        for size,alias in REFERENCE_LABELS.items():
            name='recognizers/recognizer_'+alias+'.onnx';path=self.models/name
            path.parent.mkdir(exist_ok=True);path.write_bytes(('weights-'+alias).encode())
            self.manifest['models'].append({'role':'recognizer','file':name,'sha256':sha(path),
                                           'vocab_size':size,'source_resource':alias})
            name='vocab/vocab_'+alias+'.json';path=self.models/name;write(path,['x']*(size-1)+[''])
            self.manifest['vocabularies'].append({'file':name,'sha256':sha(path)})
        write(self.models/'manifest.json',self.manifest)

    def test_fresh_and_same_source_reuse_bind_every_asset(self):
        provenance=stage_reference_inputs(self.models,self.target);before=tree(self.target)
        self.assertEqual(len(provenance['assets']),20)
        self.assertEqual(provenance['model_manifest_sha256'],sha(self.models/'manifest.json'))
        self.assertEqual(stage_reference_inputs(self.models,self.target),provenance)
        self.assertEqual(before,tree(self.target))

    def test_stale_cached_weights_are_rejected_before_any_writes(self):
        path=self.target/'classifier/script_classifier.onnx';path.parent.mkdir(parents=True);path.write_bytes(b'previous model')
        before=tree(self.target)
        with self.assertRaisesRegex(ValueError,'cache checksum mismatch'):stage_reference_inputs(self.models,self.target)
        self.assertEqual(before,tree(self.target))

    def test_changed_cached_vocabulary_is_rejected(self):
        stage_reference_inputs(self.models,self.target)
        path=self.target/'vocab/vocab_latin.txt';path.write_bytes(b'0: different token\n');before=tree(self.target)
        with self.assertRaisesRegex(ValueError,'cached vocabulary mismatch'):stage_reference_inputs(self.models,self.target)
        self.assertEqual(before,tree(self.target))

    def test_changed_source_model_or_vocabulary_is_rejected(self):
        for name in ['classifier/script_classifier.onnx','vocab/vocab_latin.json']:
            path=self.models/name;original=path.read_bytes();path.write_bytes(b'changed source')
            try:
                with self.assertRaisesRegex(ValueError,'source checksum mismatch'):stage_reference_inputs(self.models,self.target)
                self.assertFalse(self.target.exists())
            finally:path.write_bytes(original)

    def test_manifest_upgrade_cannot_reuse_previous_weights(self):
        stage_reference_inputs(self.models,self.target);before=tree(self.target)
        path=self.models/'classifier/script_classifier.onnx';path.write_bytes(b'new weights')
        self.manifest['models'][1]['sha256']=sha(path);self.manifest['source_assets']['oneocr.onemodel']='new-source'
        write(self.models/'manifest.json',self.manifest)
        with self.assertRaisesRegex(ValueError,'cache checksum mismatch'):stage_reference_inputs(self.models,self.target)
        self.assertEqual(before,tree(self.target))

    def test_new_cache_files_do_not_share_writable_model_storage(self):
        stage_reference_inputs(self.models,self.target)
        source=self.models/'classifier/script_classifier.onnx';before=source.read_bytes()
        (self.target/'classifier/script_classifier.onnx').write_bytes(b'cache damaged')
        self.assertEqual(source.read_bytes(),before)

    def test_incomplete_manifest_does_not_reuse_unlisted_cached_models(self):
        stage_reference_inputs(self.models,self.target);before=tree(self.target)
        self.manifest['models'].pop(1);write(self.models/'manifest.json',self.manifest)
        with self.assertRaisesRegex(ValueError,'Incomplete reference model inventory'):stage_reference_inputs(self.models,self.target)
        self.assertEqual(before,tree(self.target))


if __name__=='__main__':unittest.main()
