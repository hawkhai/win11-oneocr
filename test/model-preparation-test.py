"""Verify version selection and rejection before any existing model is written."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from model_preparation import source_for_preparation


class PreparationSource(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name);self.source=self.root/'bin/photos/oneocr.onemodel'
        self.source.parent.mkdir(parents=True);self.source.write_bytes(b'current')
        self.old=self.root/'bin/oneocr.onemodel';self.old.write_bytes(b'legacy')
        self.sha=hashlib.sha256(b'current').hexdigest()
        self.reference={'legacy_binary_dir':'bin/photos','model_source_sha256':self.sha}
        self.models=self.root/'models';self.models.mkdir()
        (self.models/'manifest.json').write_text(json.dumps({'source_assets':{'oneocr.onemodel':self.sha}}))

    def test_default_is_current_bound_model(self):
        self.assertEqual(source_for_preparation(self.root,None,self.models,self.reference),self.source.resolve())

    def test_legacy_cannot_replace_active_model(self):
        before=(self.models/'manifest.json').read_bytes()
        with self.assertRaises(ValueError):source_for_preparation(self.root,self.old,self.models,self.reference)
        self.assertEqual((self.models/'manifest.json').read_bytes(),before)

    def test_default_needs_real_binding(self):
        with self.assertRaises(ValueError):source_for_preparation(self.root,None,self.models,{})
        self.source.write_bytes(b'changed')
        with self.assertRaises(ValueError):source_for_preparation(self.root,None,self.models,self.reference)

    def test_explicit_other_version_in_fresh_directory(self):
        fresh=self.root/'work/legacy'
        self.assertEqual(source_for_preparation(self.root,self.old,fresh,self.reference),self.old.resolve())
        self.assertFalse(fresh.exists())

    def test_existing_other_version_is_preserved(self):
        with self.assertRaises(ValueError):source_for_preparation(self.root,self.old,self.models,{})

    def test_unidentified_nonempty_directory_is_preserved(self):
        target=self.root/'work/partial';(target/'detector').mkdir(parents=True)
        graph=target/'detector/text_detector.onnx';graph.write_bytes(b'existing partial graph')
        for source in [None,self.old]:
            with self.assertRaises(ValueError):source_for_preparation(self.root,source,target,self.reference)
            self.assertEqual(graph.read_bytes(),b'existing partial graph')
        self.assertFalse((target/'manifest.json').exists())

    def test_existing_empty_directory_is_accepted(self):
        target=self.root/'work/empty';target.mkdir(parents=True)
        self.assertEqual(source_for_preparation(self.root,None,target,self.reference),self.source.resolve())


if __name__=='__main__':unittest.main()
