"""Add rotated-handwriting, script and layout option cases without rewriting history."""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def expanded_manifest():
    original=ROOT/'research/parameter-cases.json'
    if sha(original)!='b9ccc9bdaad3c0037bd0384b586a800228ff61d99a2956e565447bbb07b0e326':
        raise ValueError('Historical parameter manifest changed')
    value=json.loads(original.read_text(encoding='utf8'));cases=value['cases']
    def add(file,n,w,h,purpose):
        source=ROOT/file
        cases.append({'name':f'{source.stem}-m{n}-r{w}x{h}','file':file,'sha256':sha(source),
                      'options':{'max_lines':n,'resize_width':w,'resize_height':h},'purpose':purpose})
    for stem in ['red-grid-jason','red-calligraphy-jason']:
        for angle in [90,180,270,10]:
            for n,w,h in [(1,1152,768),(5,640,384),(6,768,512),(1000,33,65)]:
                add(f'samples/options-handwriting/{stem}-rotate-{angle}.png',n,w,h,
                    'Real handwriting rotation combined with soft line budgets and resize; retained source provenance')
    for script in ['arabic','hebrew','devanagari','thai']:
        for n,w,h in [(1,640,384),(2,1151,767),(0,320,192)]:
            add(f'research/case/corpus/{script}-36.png',n,w,h,
                'Existing printed script control with nondefault options; not a handwriting or accuracy claim')
    for layout in ['multi-column','vertical-cjk']:
        for n,w,h in [(1,640,384),(5,1151,767)]:
            add(f'research/case/corpus/{layout}.png',n,w,h,
                'Reading order and intersecting layout components combined with soft line budget and nonmultiple resize')
    value['scope']=f'{len(cases)} finite reproducible combinations, not all possible options'
    value['previous_manifest']={'file':'research/parameter-cases.json','sha256':sha(original),'cases':67}
    value['added_coverage']={'rotated_handwriting':32,'printed_scripts':12,'layout':4,
                           'new_writers':0,'new_source_photographs':0}
    return value

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'research/parameter-cases-2026-11-04.json')
    args=parser.parse_args()
    if args.output.exists():raise ValueError('Use a new manifest path; historical manifests must not be overwritten')
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(expanded_manifest(),ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print(args.output)

if __name__=='__main__':main()
