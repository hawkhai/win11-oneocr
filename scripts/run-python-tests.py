"""Run hyphenated test modules explicitly; unittest discovery skips these names."""
import argparse
import importlib.util
import json
from pathlib import Path
import sys
import unittest

ROOT=Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--without-archives',action='store_true',help='Diagnostic only while fresh full inference is still being collected');p.add_argument('--report',type=Path);a=p.parse_args()
    files=sorted((ROOT/'test').glob('*-test.py'))
    if a.without_archives:files=[f for f in files if f.name not in ['archive-verification-test.py','compare-only-test.py']]
    suite=unittest.TestSuite()
    for file in files:
        spec=importlib.util.spec_from_file_location(file.stem.replace('-','_'),file);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        suite.addTests(unittest.defaultTestLoader.loadTestsFromModule(module))
    if not suite.countTestCases():raise RuntimeError('No tests loaded')
    result=unittest.TextTestRunner(verbosity=2).run(suite)
    if a.report:
        a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps({'passed':result.wasSuccessful(),'full_suite':not a.without_archives,'tests':result.testsRun,'failures':len(result.failures),'errors':len(result.errors),'files':[f.name for f in files]},indent=2)+'\n',encoding='utf8')
    return 0 if result.wasSuccessful() else 1
if __name__=='__main__':sys.exit(main())
