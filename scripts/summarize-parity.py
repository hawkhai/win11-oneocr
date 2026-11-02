"""Aggregate strict corpus comparisons without suppressing failed cases."""
import argparse
import importlib.util
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('oneocr_comparison',ROOT/'scripts/compare-results.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--baseline',type=Path,default=ROOT/'work/oneocr-portable/corpus-legacy')
    p.add_argument('--actual',type=Path,default=ROOT/'work/oneocr-portable/corpus-native')
    p.add_argument('--output',type=Path,default=ROOT/'work/oneocr-portable/evidence/corpus-parity.json')
    args=p.parse_args();results=[]
    fixtures=json.loads((ROOT/'work/oneocr-portable/corpus/manifest.json').read_text(encoding='utf8'))['cases']
    measurements={}
    for kind,directory in [('baseline',args.baseline),('actual',args.actual)]:
        path=directory/'measurements.json'
        measurements[kind]={x['name']:x for x in json.loads(path.read_text(encoding='utf8'))} if path.exists() else {}
    for fixture in fixtures:
        failed=[kind for kind in measurements if fixture['name'] in measurements[kind] and measurements[kind][fixture['name']]['exit_code']!=0]
        if failed:
            results.append({'name':fixture['name'],'passed':False,'error':'Backend execution failed: '+', '.join(failed)});continue
        baseline=args.baseline/(fixture['name']+'.json')
        if not baseline.exists():
            results.append({'name':fixture['name'],'passed':False,'error':'Baseline output missing'});continue
        actual=args.actual/baseline.name
        if not actual.exists():
            results.append({'name':baseline.stem,'passed':False,'error':'Native output missing'});continue
        result=module.compare(json.loads(baseline.read_text(encoding='utf8')),json.loads(actual.read_text(encoding='utf8')))
        result.update(name=baseline.stem)
        # Keep the complete differences in each per-case file.
        individual=args.output.parent/'parity-cases'/(baseline.stem+'.json')
        individual.parent.mkdir(parents=True,exist_ok=True)
        individual.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf8')
        results.append({k:v for k,v in result.items() if k!='failures'})
    summary={'passed':bool(results) and all(x['passed'] for x in results),'passed_cases':sum(x['passed'] for x in results),'total_cases':len(results),'cases':results,'coverage_gap':'This 27-case corpus contains no real handwriting; samples/oneocr-parity is reported separately.'}
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(summary,ensure_ascii=False,indent=2),encoding='utf8')
    print(json.dumps({k:v for k,v in summary.items() if k!='cases'},ensure_ascii=False))
    raise SystemExit(0 if summary['passed'] else 1)
