"""Strict DLL/native result comparison. A mismatch always exits nonzero."""
import argparse
import json
from pathlib import Path

def compare(expected,actual):
    failures=[]
    def check(condition,field,expected_value,actual_value):
        if not condition:
            failures.append({'field':field,'expected':expected_value,'actual':actual_value})
    def close(a,b,tolerance,field):
        numeric=isinstance(a,(int,float)) and isinstance(b,(int,float))
        check(numeric and abs(a-b)<=tolerance,field,a,b)
    def box(a,b,field):
        if not isinstance(a,list) or not isinstance(b,list) or len(a)!=8 or len(b)!=8:
            check(False,field,a,b)
        else:
            for i,(x,y) in enumerate(zip(a,b)):
                close(x,y,1.0,f'{field}[{i}]')
    a,b=expected.get('lines',[]),actual.get('lines',[])
    check(len(a)==len(b),'line_count',len(a),len(b))
    ea,aa=expected.get('image_angle'),actual.get('image_angle')
    numeric=isinstance(ea,(int,float)) and isinstance(aa,(int,float))
    difference=abs((ea-aa+180)%360-180) if numeric else None
    check(numeric and difference<=.1,'image_angle',ea,aa)
    check(expected.get('image')==actual.get('image'),'image',expected.get('image'),actual.get('image'))
    matched_text=0
    for i,(x,y) in enumerate(zip(a,b)):
        prefix=f'lines[{i}]'
        check(x.get('text')==y.get('text'),prefix+'.text',x.get('text'),y.get('text'))
        matched_text+=x.get('text')==y.get('text')
        box(x.get('bounding_box'),y.get('bounding_box'),prefix+'.bounding_box')
        xs,ys=x.get('style',{}),y.get('style',{})
        check(xs.get('type')==ys.get('type'),prefix+'.style.type',xs.get('type'),ys.get('type'))
        close(xs.get('confidence'),ys.get('confidence'),1e-3,prefix+'.style.confidence')
        aw,bw=x.get('words',[]),y.get('words',[])
        check(len(aw)==len(bw),prefix+'.word_count',len(aw),len(bw))
        for j,(u,v) in enumerate(zip(aw,bw)):
            wp=f'{prefix}.words[{j}]'
            check(u.get('text')==v.get('text'),wp+'.text',u.get('text'),v.get('text'))
            box(u.get('bounding_box'),v.get('bounding_box'),wp+'.bounding_box')
            close(u.get('confidence'),v.get('confidence'),1e-3,wp+'.confidence')
    return {'passed':not failures,'expected_lines':len(a),'actual_lines':len(b),'same_position_text_matches':matched_text,'failure_count':len(failures),'failures':failures,'tolerances':{'coordinates_px':1,'angle_degrees':.1,'confidence':1e-3}}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline',type=Path);p.add_argument('actual',type=Path);p.add_argument('--output',type=Path)
    args=p.parse_args()
    report=compare(json.loads(args.baseline.read_text(encoding='utf8')),json.loads(args.actual.read_text(encoding='utf8')))
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
    print(json.dumps({k:v for k,v in report.items() if k!='failures'},ensure_ascii=False))
    raise SystemExit(0 if report['passed'] else 1)
