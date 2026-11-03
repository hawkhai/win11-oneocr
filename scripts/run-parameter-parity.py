"""Strict CLI regression for every frozen nondefault parameter combination."""
import argparse
import importlib.util
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from regression_inputs import current_cases
from regression_reference import active_reference
from regression_options import cli_options
from verification_state import read,sha

def module(name,file):
    spec=importlib.util.spec_from_file_location(name,ROOT/'scripts'/file);value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
def write(path,value):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--binary',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    cases=[c for c in current_cases(ROOT,active_reference()) if c['name'].startswith('parameter-')]
    if not cases:raise ValueError('No registered parameter cases')
    runner=module('runner','run-corpus.py');comparator=module('comparator','compare-results.py');binary=a.binary.resolve();results=[]
    if a.output.exists() and any(a.output.iterdir()):raise ValueError('Use a fresh output directory')
    for case in cases:
        name=case['name'].removeprefix('parameter-');destination=(a.output/'native'/(name+'.json')).resolve();destination.parent.mkdir(parents=True,exist_ok=True)
        measurement=runner.run([str(binary),case['path'],'--model-dir',str(ROOT/'models'),'--quiet','--output',str(destination),*cli_options(case['options'])],binary.parent)
        report=comparator.compare(read(case['baseline']),read(destination)) if measurement['exit_code']==0 else {'passed':False,'process_error':measurement['stderr']}
        write(a.output/(name+'.parity.json'),report)
        results.append({'name':name,'input_sha256':case['sha256'],'options':case['options'],'baseline_sha256':sha(Path(case['baseline'])),
                        'measurement':measurement,**{k:v for k,v in report.items() if k!='failures'}})
        print(name,report['passed'],flush=True)
        write(a.output/'suite.json',{'passed':all(x['passed'] for x in results),'total_cases':len(cases),'completed_cases':len(results),
              'passed_cases':sum(x['passed'] for x in results),'cases':results,'binary_sha256':sha(binary),'comparator_sha256':sha(ROOT/'scripts/compare-results.py')})
    return 0 if all(x['passed'] for x in results) else 1

if __name__=='__main__':sys.exit(main())
