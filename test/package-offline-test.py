"""A failed refresh must preserve the current index and historical packages."""
import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('package_offline',ROOT/'scripts/package-offline.py')
pack=importlib.util.module_from_spec(spec);spec.loader.exec_module(pack)


class VersionedPackages(unittest.TestCase):
    def test_failed_refresh_preserves_current_index_and_previous_package(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);artifacts=root/'artifacts';artifacts.mkdir()
            index=artifacts/'OFFLINE_PACKAGE.json';index.write_bytes(b'previous index')
            legacy=artifacts/'win11-oneocr-offline.bundle';legacy.write_bytes(b'previous package')
            target=artifacts/'releases/new'
            def git(*args):
                return '' if args[0]=='status' else ('refs/heads/research' if args[0]=='symbolic-ref' else 'a'*40)
            with patch.object(pack,'ROOT',root),patch.object(pack,'git',git),\
                 patch.object(sys,'argv',['package-offline.py','--output',str(target)]),\
                 patch.object(pack.subprocess,'run',side_effect=RuntimeError('review rejected')):
                with self.assertRaisesRegex(RuntimeError,'review rejected'):pack.main()
                self.assertEqual(index.read_bytes(),b'previous index')
                self.assertEqual(legacy.read_bytes(),b'previous package')
                # A failed directory is retained as evidence; retry uses a new one.
                with self.assertRaises(FileExistsError):pack.main()


if __name__=='__main__':unittest.main()
