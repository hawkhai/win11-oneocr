"""Read-only verification of saved clean-build executions, not staged-runtime claims."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'scripts'))
from regression_inputs import current_cases
from regression_reference import active_reference
from verification_state import read,sha,contained,signatures,same_json,native_result_path,check_result_files,destination_for_verification

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    report=a.report.resolve();output=destination_for_verification(report,a.output)
    index=read(report/'FILES.sha256.json')
    if set(index)!={f.relative_to(report).as_posix() for f in report.rglob('*') if f.is_file() and f!=report/'FILES.sha256.json'}:
        raise ValueError('Incomplete clean-build report index')
    for name,entry in index.items():
        f=contained(report,name)
        if sha(f)!=entry['sha256'] or f.stat().st_size!=entry['size']:raise ValueError('Archived build record changed: '+name)
    for name,entry in read(report/'evidence/source-bindings.json')['files'].items():
        data=contained(ROOT,name).read_bytes()
        if entry['format']=='lf':data=data.replace(b'\r\n',b'\n')
        if hashlib.sha256(data).hexdigest()!=entry['sha256']:raise ValueError('Clean-build source changed: '+name)
    bindings=read(report/'evidence/bindings.json');reference=active_reference()
    if sha(ROOT/'scripts/compare-results.py')!=bindings['comparator_sha256']:raise ValueError('Comparator changed')
    for name,digest in bindings['original_assets'].items():
        if sha(ROOT/reference['legacy_binary_dir']/name)!=digest:raise ValueError('Original asset changed: '+name)
    for name,entry in read(ROOT/'research/delivery-manifest.json')['files'].items():
        if sha(contained(ROOT,name))!=entry['sha256']:raise ValueError('Model/staged runtime asset changed: '+name)
    cases=current_cases(ROOT,reference);captured=read(report/'evidence/execution-cases.json')
    if signatures(cases)!=signatures(captured):raise ValueError('Clean-build input/option scope changed')
    for case,old in zip(cases,captured):
        if case['name']!=old['name'] or (sha(case['baseline']) if case.get('baseline') else None)!=old['baseline_sha256']:
            raise ValueError('Clean-build baseline changed: '+case['name'])
    spec=importlib.util.spec_from_file_location('strict',ROOT/'scripts/compare-results.py')
    comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
    environments=[]
    for env in ['windows-native','linux-native']:
        directory=report/env;record=read(directory/'summary.json')
        if not (record['passed'] and record['full_regression'] and record['cases']==record['passed_cases']==len(cases)):
            raise ValueError('Incomplete clean-build execution: '+env)
        if record['binary_sha256']!=bindings['rebuilt_outputs'][env]['cli_sha256']:
            raise ValueError('Rebuilt CLI identity changed: '+env)
        check_result_files(directory,cases,native=True)
        samples={c['name']:c for c in read(directory/'all-samples/suite.json')['cases']}
        parameters={c['name']:c for c in read(directory/'options/suite.json')['cases']}
        results=[]
        for case in cases:
            name=case['name'];actual=native_result_path(directory,name)
            if name.startswith('parameter-'):
                entry=parameters[name.removeprefix('parameter-')]
                if entry['input_sha256']!=case['sha256'] or entry['baseline_sha256']!=sha(case['baseline']) or not same_json(entry['options'],case['options']):
                    raise ValueError('Clean-build parameter identity changed: '+name)
            if case.get('expected_error'):
                expected=case['expected_error'];measurement=samples[name.removeprefix('sample-')]['measurement']
                result={'passed':measurement['exit_code']==1 and measurement['stderr'].replace('\r\n','\n')=='ERROR: '+expected['message']+'\n' and not actual.exists(),'expected_error':expected}
            else:result=comparator.compare(read(case['baseline']),read(actual))
            results.append({'case':name,**result})
        environments.append({'environment':env,'cases':len(results),'passed_cases':sum(c['passed'] for c in results),'passed':all(c['passed'] for c in results),'results':results})
    summary={'mode':'saved-clean-build-results-verification','fresh_inference':False,'current_scope':True,
             'passed':all(e['passed'] for e in environments),'cases':sum(e['cases'] for e in environments),'passed_cases':sum(e['passed_cases'] for e in environments),
             'source_identity_checked':True,'wasm_is_acceptance_gate':False,'rebuilt_outputs_are_not_claimed_to_be_staged_runtime':True,
             'execution_index_sha256':sha(report/'FILES.sha256.json'),'environments':environments}
    output.mkdir(parents=True,exist_ok=True);(output/'verification.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(json.dumps({k:summary[k] for k in ['passed','cases','passed_cases','fresh_inference']}))
    return 0 if summary['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
