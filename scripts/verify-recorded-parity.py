"""Recheck the sealed five-environment archive; never perform or claim new inference."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from verification_state import contained,destination_for_verification,signatures,runtime_assets,check_result_files,load_execution,verify_execution
from regression_inputs import current_cases
from regression_reference import active_reference


def read(p):return json.loads(p.read_text(encoding='utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    sys.stdout.reconfigure(encoding='utf8')
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();report=args.report.resolve();output=destination_for_verification(report,args.output)
    index=read(report/'FILES.sha256.json')
    for name,entry in index.items():
        if sha(contained(report,name))!=entry['sha256']:raise ValueError('Archived report changed: '+name)
    captured=read(report/'windows-node/execution-cases.json')
    reference=active_reference();cases=current_cases(ROOT,reference)
    if signatures(captured)!=signatures(cases):
        raise ValueError('Verification input scope differs from the recorded execution')
    recorded=read(report/'windows-node/summary.json')
    baselines={c['name']:c['baseline_sha256'] for c in recorded['results']}
    if sha(ROOT/'models/manifest.json')!=recorded['model_manifest_sha256']:raise ValueError('Model changed')
    for model in read(report/'evidence/model-preparation.json')['files']:
        if sha(contained(ROOT/'models',model['file']))!=model['sha256']:
            raise ValueError('Recorded model asset changed: '+model['file'])
    commit=read(report/'evidence/verification.json')['implementation_commit']
    if not re.fullmatch('[0-9a-f]{40}',commit):raise ValueError('Invalid execution commit')
    # Match native code/CRT files to their actual execution commit, allowing doc updates.
    for platform in ['win32-x64','linux-x64']:
        prefix='artifacts/'+platform
        snapshot=json.loads(subprocess.check_output(['git','show',commit+':'+prefix+'/BUILD.json'],cwd=ROOT))
        for name,expected in snapshot['files'].items():
            if name.startswith(('bin/','lib/')) and sha(ROOT/prefix/name)!=expected:
                raise ValueError('Native runtime changed: '+platform+'/'+name)
    build=read(ROOT/'artifacts/wasm32/BUILD.json');execution_build=read(report/'windows-node/execution-build.json')
    if runtime_assets(build)!=runtime_assets(execution_build):raise ValueError('Wasm runtime file set changed')
    for name,expected in execution_build['files'].items():
        if name.endswith(('.mjs','.wasm','.d.mts')) and (build['files'].get(name)!=expected or sha(ROOT/'artifacts/wasm32'/name)!=expected):
            raise ValueError('Wasm runtime changed: '+name)
    spec=importlib.util.spec_from_file_location('strict_compare',ROOT/'scripts/compare-results.py')
    comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
    environments=[]
    for env in ['windows-native','linux-native','windows-node','chrome','linux-node']:
        directory=report/env;old=read(directory/'summary.json');results=[]
        if not old['passed'] or old['passed_cases']!=len(cases):raise ValueError('Original execution was incomplete: '+env)
        native=env.endswith('-native')
        result_files=check_result_files(directory,cases,native=native)
        for path in result_files:
            if path.relative_to(report).as_posix() not in index:raise ValueError('Unindexed result file: '+str(path))
        if not native:
            execution=load_execution(directory)
            verify_execution(execution,cases,build,ROOT/'models')
        suite={c['name']:c for c in read(directory/'all-samples/suite.json')['cases']} if native else {}
        for case in cases:
            name=case['name']
            if native:
                actual=directory/'book.json' if name=='book' else directory/'corpus/native'/(name.removeprefix('corpus-')+'.json') if name.startswith('corpus-') else directory/'all-samples/native'/(name.removeprefix('sample-')+'.json')
            else:actual=directory/'actual'/(name+'.json')
            expected=case.get('expected_error')
            if expected:
                if native:
                    measurement=suite[name.removeprefix('sample-')]['measurement']
                    passed=measurement['exit_code']==1 and measurement['stderr'].replace('\r\n','\n')=='ERROR: '+expected['message']+'\n' and not actual.exists()
                else:
                    error=read(actual.with_name(name+'.error.json'))
                    passed=all(error.get(k)==v for k,v in expected.items()) and not actual.exists()
                result={'passed':passed,'expected_error':expected}
            else:
                golden=Path(case['baseline'])
                if sha(golden)!=baselines[name]:raise ValueError('Baseline changed: '+name)
                result=comparator.compare(read(golden),read(actual))
            results.append({'case':name,**result})
        environments.append({'environment':env,'passed':all(r['passed'] for r in results),'cases':len(results),
                             'passed_cases':sum(r['passed'] for r in results),'results':results})
    summary={'mode':'saved-results-verification','fresh_inference':False,'passed':all(e['passed'] for e in environments),
             'cases':sum(e['cases'] for e in environments),'passed_cases':sum(e['passed_cases'] for e in environments),
             'execution_index_sha256':sha(report/'FILES.sha256.json'),'execution_commit':commit,
             'comparator_sha256':sha(ROOT/'scripts/compare-results.py'),'runtime_identity_checked':True,
             'input_scope_checked':True,'verified_input_sha256':{c['name']:c['sha256'] for c in cases},
             'input_inventory_sha256':{n:sha(ROOT/n) for n in ['samples/manifest.json','research/case/corpus/manifest.json']},
             'verifier_source_sha256':{n:sha(ROOT/'scripts'/n) for n in ['verify-recorded-parity.py','verification_state.py','regression_inputs.py']},
             'tolerances':{'coordinates_px':1,'angle_degrees':.1,'confidence':1e-3},'environments':environments}
    output.mkdir(parents=True,exist_ok=True)
    (output/'verification.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({k:summary[k] for k in ['passed','fresh_inference','passed_cases','cases']}));return 0 if summary['passed'] else 1


if __name__=='__main__':sys.exit(main())
