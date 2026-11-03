"""Read-only native acceptance verification; WebAssembly is diagnostic only."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from regression_inputs import current_cases
from regression_reference import active_reference
from verification_state import read,sha,contained,signatures,same_json,native_result_path,check_result_files,destination_for_verification

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--reference',type=Path,help='Explicit historical reference snapshot; does not verify the current scope');a=p.parse_args()
    report=a.report.resolve();output=destination_for_verification(report,a.output)
    index=read(report/'FILES.sha256.json')
    files={f.relative_to(report).as_posix() for f in report.rglob('*') if f.is_file() and f!=report/'FILES.sha256.json'}
    if set(index)!=files:raise ValueError('Incomplete report index')
    for name,entry in index.items():
        file=contained(report,name)
        if sha(file)!=entry['sha256'] or file.stat().st_size!=entry['size']:raise ValueError('Archived file changed: '+name)
    reference=active_reference(a.reference)
    cases=current_cases(ROOT,reference);captured=read(report/'evidence/execution-cases.json')
    if signatures(cases)!=signatures(captured):raise ValueError('Native input/option scope changed')
    for case,old in zip(cases,captured):
        if case['name']!=old['name'] or (sha(Path(case['baseline'])) if case.get('baseline') else None)!=old['baseline_sha256']:
            raise ValueError('Native baseline changed: '+case['name'])
    bindings=read(report/'evidence/bindings.json');model_hash=sha(ROOT/'models/manifest.json')
    for name,digest in bindings['original_assets'].items():
        if sha(ROOT/reference['legacy_binary_dir']/name)!=digest:raise ValueError('Original asset changed: '+name)
    for name,entry in read(ROOT/'research/delivery-manifest.json')['files'].items():
        if name.startswith('models/') and sha(ROOT/name)!=entry['sha256']:raise ValueError('Model asset changed: '+name)
    for platform in ['win32-x64','linux-x64']:
        prefix='artifacts/'+platform
        snapshot=json.loads(subprocess.check_output(['git','show',bindings['implementation_commit']+':'+prefix+'/BUILD.json'],cwd=ROOT))
        for name,digest in snapshot['files'].items():
            if name.startswith(('bin/','lib/')) and sha(ROOT/prefix/name)!=digest:raise ValueError('Native runtime changed: '+platform+'/'+name)
    spec=importlib.util.spec_from_file_location('native_strict',ROOT/'scripts/compare-results.py')
    comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
    if sha(ROOT/'scripts/compare-results.py')!=bindings['comparator_sha256']:raise ValueError('Comparator changed')
    environments=[]
    for env in ['windows-native','linux-native']:
        directory=report/env;record=read(directory/'summary.json')
        if not (record['passed'] is True and record['full_regression'] is True and record['cases']==record['passed_cases']==len(cases) and record['model_manifest_sha256']==model_hash):
            raise ValueError('Incomplete native execution: '+env)
        for file in check_result_files(directory,cases,native=True):
            if file.relative_to(report).as_posix() not in index:raise ValueError('Unindexed native output')
        samples={c['name']:c for c in read(directory/'all-samples/suite.json')['cases']}
        parameters={c['name']:c for c in read(directory/'options/suite.json')['cases']}
        results=[]
        for case in cases:
            name=case['name'];actual=native_result_path(directory,name)
            if name.startswith('parameter-'):
                entry=parameters[name.removeprefix('parameter-')]
                if entry['input_sha256']!=case['sha256'] or entry['baseline_sha256']!=sha(Path(case['baseline'])) or not same_json(entry['options'],case['options']):
                    raise ValueError('Native parameter identity changed: '+name)
            if case.get('expected_error'):
                m=samples[name.removeprefix('sample-')]['measurement'];expected=case['expected_error']
                comparison={'passed':m['exit_code']==1 and m['stderr'].replace('\r\n','\n')=='ERROR: '+expected['message']+'\n' and not actual.exists(),'expected_error':expected}
            else:comparison=comparator.compare(read(case['baseline']),read(actual))
            results.append({'case':name,**comparison})
        environments.append({'environment':env,'cases':len(results),'passed_cases':sum(r['passed'] for r in results),'passed':all(r['passed'] for r in results),'results':results})
    summary={'mode':'saved-native-results-verification','fresh_inference':False,'passed':all(e['passed'] for e in environments),
             'cases':sum(e['cases'] for e in environments),'passed_cases':sum(e['passed_cases'] for e in environments),
             'wasm_is_acceptance_gate':False,'execution_index_sha256':sha(report/'FILES.sha256.json'),
             'current_scope':a.reference is None,'reference_snapshot_sha256':sha(a.reference) if a.reference else None,
             'comparator_sha256':bindings['comparator_sha256'],'environments':environments}
    output.mkdir(parents=True,exist_ok=True);(output/'verification.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({k:summary[k] for k in ['passed','cases','passed_cases','wasm_is_acceptance_gate']}))
    return 0 if summary['passed'] else 1

if __name__=='__main__':sys.exit(main())
