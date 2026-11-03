"""Reproduce version-locked geometry, grouping and orientation observations."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from verification_state import read,sha
from regression_inputs import current_cases
from regression_reference import active_reference

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    if a.output.exists() and any(a.output.iterdir()):raise ValueError('Use a fresh oracle directory')
    a.output.mkdir(parents=True);scope=current_cases(ROOT,active_reference());records=[]
    spec=importlib.util.spec_from_file_location('comparator',ROOT/'scripts/compare-results.py');comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
    inputs=[('calligraphy','red-calligraphy-jason.jpg',1000,1152,768),('calligraphy-90','red-calligraphy-jason-rotate-90.png',1000,1152,768),
        ('calligraphy-270','red-calligraphy-jason-rotate-270.png',1000,1152,768),('grid-10','red-grid-jason-rotate-10.png',1000,1152,768),
        ('calligraphy-640','red-calligraphy-jason.jpg',1,640,384),('grid-budget','red-grid-jason.jpg',5,1152,768)]
    for name,file,n,w,h in inputs:
        image=ROOT/'samples/options-handwriting'/file;output=a.output/(name+'.json')
        subprocess.run([sys.executable,'-X','utf8',ROOT/'scripts/probe-photos-regions.py',image,'--max-lines',str(n),'--resize',f'{w}x{h}','--observe-choice','--output',output],check=True,cwd=ROOT)
        if n==1000 and (w,h)==(1152,768):case=next(x for x in scope if x['name'].startswith('sample-') and Path(x['path'])==image)
        else:case=next(x for x in scope if x['name'].startswith('parameter-') and Path(x['path'])==image and x['options']=={'max_lines':n,'resize_width':w,'resize_height':h})
        comparison=comparator.compare(read(Path(case['baseline'])),read(output)['result'])
        if not comparison['passed']:raise ValueError('Observation changed original output: '+name)
        records.append({'file':output.name,'sha256':sha(output),'case':case['name'],'baseline_sha256':sha(Path(case['baseline'])),'unmodified_output_strictly_equal':True})
    index={'schema_version':1,'files':records,'source_files':{name:sha(ROOT/'scripts'/name) for name in ['probe-photos-regions.py','photos_choice_observer.py','probe-photos-recognition-choice.py','capture-options-oracles.py']}}
    (a.output/'index.json').write_text(json.dumps(index,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
