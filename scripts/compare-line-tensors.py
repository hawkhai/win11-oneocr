"""Check one-to-one, byte-exact classifier/recognizer tensor observations.

Pairs are assigned by complete input equality, never by guessed model size or
recognized text. Unmatched inputs remain failures in the report.
"""
import argparse
import json
from pathlib import Path
import numpy as np

def read_cpp(directory,name):
    metadata=json.loads((directory/(name+'.json')).read_text(encoding='utf8'))
    return np.fromfile(directory/(name+'.f32'),np.float32).reshape(metadata['shape'])

def compare(native,cpp):
    trace=json.loads((native/'trace.json').read_text(encoding='utf8'))
    records=[]
    for prefix,input_name,outputs in [
        ('classifier/','classifier_input',['script_id_score','hwpc_score','flip_score']),
        ('recognizers/','recognizer_input',['logsoftmax'])]:
        available=[path for path in sorted(cpp.glob('line-*')) if (path/(input_name+'.json')).exists()]
        for index,entry in enumerate(trace):
            if not entry['model'].startswith(prefix):continue
            expected=np.load(native/entry['inputs']['data']['file'])
            match=next((path for path in available if np.array_equal(expected,read_cpp(path,input_name))),None)
            record={'native_call':index,'model':entry['model'],'shape':list(expected.shape),'input_exact':match is not None,'outputs':[]}
            if match:
                record['cpp_line']=match.name;available.remove(match)
                for name in outputs:
                    baseline=np.load(native/entry['outputs'][name]['file']);actual=read_cpp(match,name)
                    same=baseline.shape==actual.shape
                    item={'name':name,'same_shape':same,'exact':same and bool(np.array_equal(baseline,actual))}
                    if same:item['max_absolute_error']=float(abs(baseline-actual).max())
                    record['outputs'].append(item)
            records.append(record)
        for path in available:records.append({'model':prefix,'cpp_line':path.name,'input_exact':False,'error':'Unmatched C++ input'})
    return {'exact':bool(records) and all(r['input_exact'] and all(x['exact'] for x in r['outputs']) for r in records),'calls':records}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native',type=Path,required=True)
    parser.add_argument('--cpp',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();report=compare(args.native,args.cpp)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps({'exact':report['exact'],'calls':len(report['calls'])}))
    raise SystemExit(0 if report['exact'] else 1)
