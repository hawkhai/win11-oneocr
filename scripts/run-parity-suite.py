"""Replay a local image manifest against frozen DLL outputs, without changing tolerances."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[1]
def module(name,file):
    spec=importlib.util.spec_from_file_location(name,Path(__file__).with_name(file));value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value

def main():
    sys.stdout.reconfigure(encoding='utf8')
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',type=Path,required=True);p.add_argument('--baseline',type=Path,required=True);p.add_argument('--error-baseline',type=Path,help='Explicit frozen original-DLL error contracts, bound to image hashes');p.add_argument('--binary',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    runner=module('corpus_runner','run-corpus.py');comparator=module('strict_comparator','compare-results.py')
    cases=json.loads(args.manifest.read_text(encoding='utf8'))['cases'];args.output.mkdir(parents=True,exist_ok=True);native=args.output/'native';native.mkdir(exist_ok=True);results=[]
    for case in cases:
        name=case['name']
        if Path(name).name!=name or name in ('.','..'):raise ValueError('Invalid case name')
        source=(args.manifest.parent/case['file']).resolve()
        if 'sha256' in case and hashlib.sha256(source.read_bytes()).hexdigest()!=case['sha256']:raise ValueError('Input fingerprint changed: '+name)
        destination=(native/(name+'.json')).resolve();binary=args.binary.resolve()
        measurement=runner.run([str(binary),str(source),'--model-dir',str(ROOT/'models'),'--quiet','--output',str(destination)],binary.parent)
        result={'name':name,'measurement':measurement,'passed':False}
        baseline=args.baseline/(name+'.json')
        error_baseline=args.error_baseline/(name+'.json') if args.error_baseline else None
        if not baseline.is_file() and error_baseline and error_baseline.is_file():
            contract=json.loads(error_baseline.read_text(encoding='utf8'))
            if contract['image_sha256']!=hashlib.sha256(source.read_bytes()).hexdigest():raise ValueError('Error contract image changed: '+name)
            result['passed']=measurement['exit_code']==contract['native_cli_exit_code'] and measurement['stderr'].replace('\r\n','\n')==contract['native_cli_stderr'] and not destination.exists()
            result['status']='expected_dll_error' if result['passed'] else 'error_contract_mismatch'
            result['original_dll_status']=contract['original_dll_status'];result['original_wrapper_status']=contract['original_wrapper_status']
        elif not baseline.is_file():result['status']='missing_dll_baseline'
        elif measurement['exit_code']:result['status']='native_process_failed'
        else:
            try:report=comparator.compare(json.loads(baseline.read_text(encoding='utf8')),json.loads(destination.read_text(encoding='utf8')))
            except (ValueError,KeyError) as error:result['status']='invalid_baseline_or_output';result['error']=str(error)
            else:
                result.update({k:v for k,v in report.items() if k!='failures'});result['status']='passed' if report['passed'] else 'mismatch'
                (args.output/(name+'.parity.json')).write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
        results.append(result);print(name,result['status'],round(measurement['seconds'],3),flush=True)
    report={'passed':all(x['passed']for x in results),'passed_cases':sum(x['passed']for x in results),'strict_recognition_passes':sum(x['status']=='passed'for x in results),'expected_error_passes':sum(x['status']=='expected_dll_error'for x in results),'total_cases':len(results),'baseline':str(args.baseline.resolve()),'binary_sha256':hashlib.sha256(args.binary.read_bytes()).hexdigest(),'cases':results}
    (args.output/'suite.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8');return 0 if report['passed'] else 1
if __name__=='__main__':sys.exit(main())
