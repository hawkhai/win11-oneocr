"""A self-consistent ZIP manifest must not conceal missing report evidence."""
import contextlib
import gzip
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('package_source',ROOT/'scripts/package-source.py')
pack=importlib.util.module_from_spec(spec);spec.loader.exec_module(pack)


class SourceReports(unittest.TestCase):
    def setUp(self):
        temp=tempfile.TemporaryDirectory();self.addCleanup(temp.cleanup);self.root=Path(temp.name)
        self.report=self.root/'reports/sealed';self.report.mkdir(parents=True)
        (self.root/'artifacts').mkdir();(self.root/'CMakeLists.txt').write_text('project(fixture)')
        self.payload={'evidence/build.log.gz':gzip.compress(b'compiler and tests passed\n',mtime=0),
                      'evidence/observations.evidence':b'future evidence extension\n'}
        for name,data in self.payload.items():
            path=self.report/name;path.parent.mkdir(exist_ok=True);path.write_bytes(data)
        self.index={n:{'sha256':hashlib.sha256(data).hexdigest(),'size':len(data)} for n,data in self.payload.items()}
        self.save_index();self.destination=self.root/'artifacts/source.zip'
        self.addCleanup(patch.stopall)
        patch.object(pack,'ROOT',self.root).start();patch.object(pack,'ROOT_FILES',('CMakeLists.txt',)).start()

    def save_index(self):
        (self.report/'FILES.sha256.json').write_text(json.dumps(self.index),encoding='utf8')

    def package(self):
        with contextlib.redirect_stdout(io.StringIO()):pack.package(self.destination)

    def old_style_archive(self,omit=False,corrupt=False):
        # Mimic the original bug: SOURCE_BUILD agrees with the ZIP, while the
        # included sealed report index still requires missing/original evidence.
        files={'reports/sealed/FILES.sha256.json':(self.report/'FILES.sha256.json').read_bytes()}
        for name,data in self.payload.items():
            if omit and name.endswith('.gz'):continue
            files['reports/sealed/'+name]=b'changed' if corrupt and name.endswith('.gz') else data
        manifest={'files':{n:hashlib.sha256(data).hexdigest() for n,data in files.items()}}
        with zipfile.ZipFile(self.destination,'w') as archive:
            for n,data in files.items():archive.writestr(n,data)
            archive.writestr('SOURCE_BUILD.json',json.dumps(manifest))

    def test_all_indexed_extensions_are_included(self):
        self.package();result=pack.verify_source_archive(self.destination)
        self.assertEqual(result['report_indexes'],1);self.assertEqual(result['indexed_report_files'],2)
        with zipfile.ZipFile(self.destination) as archive:
            for name,data in self.payload.items():self.assertEqual(archive.read('reports/sealed/'+name),data)
            self.assertIsNone(archive.testzip())

    def test_missing_disk_evidence_rejects_before_zip(self):
        (self.report/'evidence/build.log.gz').unlink()
        with self.assertRaisesRegex(ValueError,'Missing'):self.package()
        self.assertFalse(self.destination.exists())

    def test_legacy_bytes_index_is_preserved_and_verified(self):
        for record in self.index.values():record['bytes']=record.pop('size')
        self.save_index();original=(self.report/'FILES.sha256.json').read_bytes()
        self.package();self.assertEqual(pack.verify_source_archive(self.destination)['indexed_report_files'],2)
        with zipfile.ZipFile(self.destination) as archive:
            self.assertEqual(archive.read('reports/sealed/FILES.sha256.json'),original)
        self.assertEqual((self.report/'FILES.sha256.json').read_bytes(),original)

    def test_conflicting_size_and_bytes_reject(self):
        self.index['evidence/build.log.gz']['bytes']=self.index['evidence/build.log.gz']['size']+1;self.save_index()
        with self.assertRaisesRegex(ValueError,'Invalid indexed report digest/size'):self.package()

    def test_boolean_length_cannot_alias_integer(self):
        name='evidence/observations.evidence';(self.report/name).write_bytes(b'x')
        self.index[name]={'sha256':hashlib.sha256(b'x').hexdigest(),'size':1,'bytes':True};self.save_index()
        with self.assertRaisesRegex(ValueError,'Invalid indexed report digest/size'):self.package()

    def test_changed_disk_digest_rejects_before_zip(self):
        self.index['evidence/build.log.gz']['sha256']='0'*64;self.save_index()
        with self.assertRaisesRegex(ValueError,'content changed'):self.package()
        self.assertFalse(self.destination.exists())

    def test_changed_disk_size_rejects_before_zip(self):
        self.index['evidence/build.log.gz']['size']+=1;self.save_index()
        with self.assertRaisesRegex(ValueError,'content changed'):self.package()
        self.assertFalse(self.destination.exists())

    def test_self_consistent_archive_with_missing_evidence_rejects(self):
        self.old_style_archive(omit=True)
        with self.assertRaisesRegex(ValueError,'Missing indexed report file'):pack.verify_source_archive(self.destination)

    def test_self_consistent_archive_with_changed_evidence_rejects(self):
        self.old_style_archive(corrupt=True)
        with self.assertRaisesRegex(ValueError,'Indexed report content differs'):pack.verify_source_archive(self.destination)

    def test_index_cannot_leave_its_report(self):
        self.index['../outside.log']={'sha256':'0'*64,'size':0};self.save_index()
        with self.assertRaisesRegex(ValueError,'Invalid indexed report path'):self.package()

    def test_index_cannot_add_model_or_compiled_files(self):
        self.index['evidence/model.onnx']={'sha256':'0'*64,'size':0};self.save_index()
        with self.assertRaisesRegex(ValueError,'Compiled/model asset'):self.package()


if __name__=='__main__':unittest.main()
