"""Validate captured graphs against stock ORT, preserving raw models."""
import argparse
import hashlib
import json
from pathlib import Path
import numpy as np
import onnx
import onnxruntime as ort

ROOT = Path(__file__).resolve().parents[1]

def inspect(models,evidence_dir=None):
    manifest = json.loads((models/'manifest.json').read_text(encoding='utf8'))
    options = ort.SessionOptions()
    options.intra_op_num_threads = 1
    options.log_severity_level = 3
    report = []
    for record in manifest['models']:
        path = models/record['file']
        entry = {'file':record['file']}
        try:
            ort.InferenceSession(str(path),sess_options=options,providers=['CPUExecutionProvider'])
            entry['load'] = True
        except Exception as error:
            entry.update(load=False,error=str(error))
        report.append(entry)
    # The native default detector requests the 18 FPN outputs, never feature_p2.
    # Slice away the optional, vendor-specific feature extraction branch. No
    # weights or computations contributing to these 18 outputs are changed.
    detector = onnx.load(str(models/'detector/text_detector.onnx'))
    outputs = [x.name for x in detector.graph.output if x.name!='feature_p2']
    standard = onnx.utils.Extractor(detector).extract_model(['data','im_info'],outputs)
    onnx.checker.check_model(standard)
    target = models/'detector/text_detector.portable.onnx'
    onnx.save(standard,str(target))
    session = ort.InferenceSession(str(target),sess_options=options,providers=['CPUExecutionProvider'])
    transformation = {'file':'detector/text_detector.portable.onnx','sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'source_sha256':hashlib.sha256((models/'detector/text_detector.onnx').read_bytes()).hexdigest(),'transformation':'backward graph slice for the 18 FPN outputs; optional feature_p2 branch removed','outputs':outputs,'replay':{}}
    trace = ROOT/'work/oneocr-portable/traces/ocr-book'
    binding=trace/'model-source.json'
    bound_source=json.loads(binding.read_text(encoding='utf8')).get('oneocr.onemodel') if binding.is_file() else None
    if (trace/'trace.json').exists() and bound_source==manifest['source_assets']['oneocr.onemodel']:
        entry = json.loads((trace/'trace.json').read_text(encoding='utf8'))[0]
        inputs = {k:np.load(trace/v['file']) for k,v in entry['inputs'].items()}
        values = session.run(outputs,inputs)
        for name,value in zip(outputs,values):
            expected = np.load(trace/entry['outputs'][name]['file'])
            transformation['replay'][name] = {'max_absolute_error':float(np.max(np.abs(value-expected))), 'shape':list(value.shape)}
    else:transformation['replay_status']='not_replayed: historical trace is unbound or belongs to another model'
    manifest['portable_detector'] = transformation
    # Only assets selected by the preparation manifest are authoritative.
    # Older experiments can leave files under vocab/; never auto-enroll them.
    for record in manifest.get('vocabularies',[]):
        path=models/record['file']
        if hashlib.sha256(path.read_bytes()).hexdigest()!=record['sha256']:
            raise ValueError('Vocabulary checksum mismatch: '+record['file'])
    (models/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf8')
    output = (evidence_dir or ROOT/'work/oneocr-portable/evidence')/'standard-ort-load.json'
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text(json.dumps({'raw_models':report,'portable_detector':transformation},indent=2),encoding='utf8')
    print('Stock ORT loaded',sum(x['load'] for x in report),'/',len(report),'raw models')
    print('Portable detector replay:',{k:x['max_absolute_error'] for k,x in transformation['replay'].items()})

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--evidence-dir',type=Path)
    args=p.parse_args();inspect(args.models,args.evidence_dir)
