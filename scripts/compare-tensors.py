"""Compare recorded original DLL ONNX tensors with opt-in C++ tensor traces."""
import argparse
import json
from pathlib import Path
import numpy as np

def compare(native,cpp):
    detector=next(e for e in json.loads((native/'trace.json').read_text(encoding='utf8')) if e['model'].startswith('detector/'))
    records=[]
    for direction in ('inputs','outputs'):
        for name,entry in detector[direction].items():
            if name=='feature_p2':continue
            expected=np.load(native/entry['file'])
            metadata=json.loads((cpp/(name+'.json')).read_text(encoding='utf8'))
            actual=np.fromfile(cpp/(name+'.f32'),dtype=np.float32).reshape(metadata['shape'])
            record={'name':name,'kind':direction,'expected_shape':list(expected.shape),'actual_shape':list(actual.shape),'same_shape':expected.shape==actual.shape}
            if record['same_shape']:
                errors=abs(expected-actual)
                record.update(max_absolute_error=float(errors.max()),mean_absolute_error=float(errors.mean()),changed_elements=int(np.count_nonzero(errors)),exact=bool(np.array_equal(expected,actual)))
            records.append(record)
    return {'detector_outputs_exact':all(r.get('exact',False) for r in records if r['kind']=='outputs'),'tensors':records}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native',type=Path,required=True)
    parser.add_argument('--cpp',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();report=compare(args.native,args.cpp)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps({'detector_outputs_exact':report['detector_outputs_exact'],'tensors':len(report['tensors'])}))
    raise SystemExit(0 if report['detector_outputs_exact'] else 1)
