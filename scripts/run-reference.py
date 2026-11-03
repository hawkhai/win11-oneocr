"""Benchmark the pinned reference core without its downloader or extractor.

The input layout matches that reference's extractor vocabulary-size labels.
Those labels are known to disagree with the original resource identities;
bindings.json records the distinction. No reference code is patched.
"""
import argparse
import importlib
import json
from pathlib import Path
import subprocess
import sys
import types
import onnxruntime as ort
from PIL import Image
sys.path.insert(0,str(Path(__file__).resolve().parent))
from reference_inputs import stage_reference_inputs

ROOT=Path(__file__).resolve().parents[1]

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('image',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--cache-dir',type=Path,default=ROOT/'work/oneocr-portable/reference-inputs',
                   help='Verified reference inputs; use a fresh directory after a model change')
    args=p.parse_args();source=ROOT/'work/vendor/oneocr-onnx-python'
    commit=subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip()
    if commit!='75cc12666503425ffd6ea0cb052c0bcaaff21058':raise SystemExit('Reference revision mismatch')
    models=ROOT/'models';target=args.cache_dir.resolve()
    provenance=stage_reference_inputs(models,target)
    # Import only the core. The package __init__ imports Windows extraction
    # code and a Store downloader, neither of which belongs in this benchmark.
    package=types.ModuleType('oneocr_reference');package.__path__=[str(source/'oneocr')];sys.modules['oneocr_reference']=package
    ort.set_default_logger_severity(3)
    engine=importlib.import_module('oneocr_reference.engine').OneOCR
    image=Image.open(args.image).convert('RGB')
    def box(q):return [q.x1,q.y1,q.x2,q.y2,q.x3,q.y3,q.x4,q.y4] if q else None
    with engine(config_dir=target) as ocr:result=ocr.recognize(image)
    lines=[]
    for i,line in enumerate(result.lines):
        words=[{'index':j,'text':word.text,'bounding_box':box(word.bbox),'confidence':word.confidence} for j,word in enumerate(line.words)]
        lines.append({'index':i,'text':line.text,'bounding_box':box(line.bbox),'style':{'type':'unknown','confidence':None},'word_count':len(words),'words':words})
    output={'file':str(args.image),'image':{'width':image.width,'height':image.height,'step':image.width*4},'image_angle':result.image_angle,'line_count':len(lines),'lines':lines,'engine':{'backend':'pinned-python-reference','parity':'incomplete','style':'unavailable; reference style is horizontal/vertical, not handwriting','bindings':str(target/'bindings.json'),'model_source_sha256':provenance['model_source_sha256'],'model_manifest_sha256':provenance['model_manifest_sha256']}}
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(output,ensure_ascii=False,indent=2),encoding='utf8')
    args.output.with_suffix('.inputs.json').write_text(json.dumps(provenance,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Reference lines:',len(lines))
