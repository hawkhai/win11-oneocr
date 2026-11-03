"""Freeze every default and parameter case using isolated original-DLL workers."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from regression_options import effective_options, requested_options, FIELDS

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def read(path):return json.loads(path.read_text(encoding='utf8'))
def write(path,value):path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--jobs',type=int,default=3);p.add_argument('--binary-dir',type=Path,default=ROOT/'bin/photos-2026.11090.22001.0');a=p.parse_args()
    out=a.output.resolve();binary=a.binary_dir.resolve()
    if out.exists() and any(out.iterdir()):raise ValueError('Use a fresh reference directory')
    from PIL import Image
    from regression_inputs import image_files
    inventory=[]
    for name in image_files(ROOT/'samples'):
        source=ROOT/'samples'/name
        with Image.open(source) as image:w,h=image.size;mode=image.mode
        inventory.append({'name':hashlib.sha256(name.encode()).hexdigest()[:16],'file':name,'sha256':sha(source),'width':w,'height':h,'pixel_mode':mode})
    write(ROOT/'samples/manifest.json',{'schema_version':1,'scope':'Every image recursively below samples, including references and thumbnails','cases':inventory})
    spec=importlib.util.spec_from_file_location('runner',ROOT/'scripts/run-corpus.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
    bindings={name:sha(binary/name) for name in ['oneocr.dll','oneocr.onemodel','onnxruntime.dll','oneocr_wrapper.dll']}
    tasks=[]
    for item in inventory:tasks.append(('sample-'+item['name'],ROOT/'samples'/item['file'],out/'legacy'/(item['name']+'.json'),item,None))
    for item in read(ROOT/'research/case/corpus/manifest.json')['cases']:
        tasks.append(('corpus-'+item['name'],ROOT/'research/case/corpus'/item['file'],out/'corpus-legacy'/(item['name']+'.json'),item,None))
    tasks.append(('book',ROOT/'ocr-book.jpg',out/'book-legacy.json',{},None))
    for item in read(ROOT/'research/parameter-cases.json')['cases']:
        tasks.append(('parameter-'+item['name'],ROOT/item['file'],out/'parameters'/(item['name']+'.json'),item,item['options']))
    def execute(task):
        name,source,result,item,options=task;result.parent.mkdir(parents=True,exist_ok=True)
        command=[sys.executable,'-X','utf8',str(ROOT/'scripts/run-legacy.py'),str(source),str(result),'--binary-dir',str(binary)]
        if options is not None:
            effective=effective_options(options)
            for flag,key in zip(['--max-lines','--resize-width','--resize-height'],FIELDS):command.extend([flag,str(effective[key])])
        measurement=runner.run(command,ROOT)
        contract={'file':item.get('file',source.relative_to(ROOT).as_posix()),'image_sha256':sha(source),'dll_bindings':bindings,
                  'measurement':measurement,'result_sha256':sha(result) if result.exists() else None}
        if options is not None:contract.update(requested_options=requested_options(options),effective_options=effective,original_wrapper_arguments=effective)
        write(result.with_name(result.stem+'.contract.json'),contract)
        expected=measurement['exit_code']==0
        if name.startswith('sample-') and measurement['exit_code']==1:
            expected=('Original wrapper OCR failed: -18' in measurement['stderr'] and not (50<=item['width']<=10000 and 50<=item['height']<=10000)) or ('Original wrapper OCR failed: -15' in measurement['stderr'] and item['pixel_mode']=='LA')
        print(name,'captured' if expected else 'UNCLASSIFIED FAILURE',flush=True)
        return {'name':name,'captured':bool(expected),'contract':result.with_name(result.stem+'.contract.json').relative_to(out).as_posix()}
    with ThreadPoolExecutor(max_workers=a.jobs) as pool:results=list(pool.map(execute,tasks))
    if bindings!={name:sha(binary/name) for name in bindings}:raise ValueError('Original assets changed during capture')
    summary={'reference_complete':all(x['captured'] for x in results),'mode':'original-DLL-capture; not a portable-engine parity claim',
             'model_source_sha256':bindings['oneocr.onemodel'],'new_reference_assets':binary.relative_to(ROOT).as_posix(),
             'sample_cases':len(inventory),'parameter_cases':len(tasks)-len(inventory)-28,'total_cases':len(tasks),'dll_bindings':bindings,'cases':results,
             'parameter_manifest':'research/parameter-cases.json','parameter_manifest_sha256':sha(ROOT/'research/parameter-cases.json')}
    write(out/'summary.json',summary)
    (out/'README.md').write_text('Fresh isolated original DLL capture for every registered sample, corpus, book and parameter case. Capture completeness is separate from portable parity. Zero aliases use the explicitly recorded effective 1000 / 1152x768 defaults; raw wrapper zero-resize skips its resize setter.\n',encoding='utf8')
    return 0 if summary['reference_complete'] else 1

if __name__=='__main__':sys.exit(main())
