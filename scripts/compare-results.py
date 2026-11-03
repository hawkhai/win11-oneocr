"""Strict DLL/native result comparison. A mismatch always exits nonzero."""
import argparse
import json
import math
from pathlib import Path

def compare(expected,actual):
    failures=[]
    def check(condition,field,expected_value,actual_value):
        if not condition:
            failures.append({'field':field,'expected':expected_value,'actual':actual_value})
    def close(a,b,tolerance,field):
        numeric=number(a) and number(b)
        check(numeric and abs(a-b)<=tolerance,field,a,b)
    def number(value):
        try:return type(value) in (int,float) and math.isfinite(value)
        except OverflowError:return False
    def bounded(value,lower,upper,field,side,exclusive_upper=False):
        valid=number(value) and lower<=value and (value<upper if exclusive_upper else value<=upper)
        check(valid,side+'.'+field+'.range',f'[{lower},{upper}{")" if exclusive_upper else "]"}',value)
    def objects(value,field,side):
        check(isinstance(value,list),side+'.'+field+'.type','array',type(value).__name__)
        return value if isinstance(value,list) else []
    def metadata(value,expected_value,field,side):
        check(type(value) is int and value==expected_value,side+'.'+field,expected_value,value)
    def record(value,field,side):
        check(isinstance(value,dict),side+'.'+field+'.type','object',type(value).__name__)
        return value if isinstance(value,dict) else {}
    def text(a,b,field):
        check(isinstance(a,str) and isinstance(b,str) and a==b,field,a,b)
    def box(a,b,field):
        if not isinstance(a,list) or not isinstance(b,list) or len(a)!=8 or len(b)!=8:
            check(False,field,a,b)
        else:
            for i,(x,y) in enumerate(zip(a,b)):
                close(x,y,1.0,f'{field}[{i}]')
    expected=record(expected,'result','baseline');actual=record(actual,'result','actual')
    for side,value in [('baseline',expected),('actual',actual)]:
        check(isinstance(value.get('file'),str),side+'.file.type','string',value.get('file'))
        image=record(value.get('image'),'image',side)
        for key in ('width','height','step'):
            item=image.get(key)
            check(type(item) is int and item>0,side+'.image.'+key,'positive integer',item)
    # File paths and backend-specific engine metadata intentionally differ by host.
    a=objects(expected.get('lines'),'lines','baseline');b=objects(actual.get('lines'),'lines','actual')
    metadata(expected.get('line_count'),len(a),'line_count','baseline')
    metadata(actual.get('line_count'),len(b),'line_count','actual')
    check(len(a)==len(b),'line_count',len(a),len(b))
    ea,aa=expected.get('image_angle'),actual.get('image_angle')
    bounded(ea,0,360,'image_angle','baseline',True)
    bounded(aa,0,360,'image_angle','actual',True)
    numeric=number(ea) and number(aa)
    difference=abs((ea-aa+180)%360-180) if numeric else None
    check(numeric and difference<=.1,'image_angle',ea,aa)
    check(expected.get('image')==actual.get('image'),'image',expected.get('image'),actual.get('image'))
    matched_text=0
    for i,(x,y) in enumerate(zip(a,b)):
        prefix=f'lines[{i}]'
        x=record(x,prefix,'baseline');y=record(y,prefix,'actual')
        metadata(x.get('index'),i,prefix+'.index','baseline')
        metadata(y.get('index'),i,prefix+'.index','actual')
        text(x.get('text'),y.get('text'),prefix+'.text')
        matched_text+=isinstance(x.get('text'),str) and x.get('text')==y.get('text')
        box(x.get('bounding_box'),y.get('bounding_box'),prefix+'.bounding_box')
        xs=record(x.get('style'),prefix+'.style','baseline');ys=record(y.get('style'),prefix+'.style','actual')
        text(xs.get('type'),ys.get('type'),prefix+'.style.type')
        bounded(xs.get('confidence'),0,1,prefix+'.style.confidence','baseline')
        bounded(ys.get('confidence'),0,1,prefix+'.style.confidence','actual')
        close(xs.get('confidence'),ys.get('confidence'),1e-3,prefix+'.style.confidence')
        aw=objects(x.get('words'),prefix+'.words','baseline');bw=objects(y.get('words'),prefix+'.words','actual')
        metadata(x.get('word_count'),len(aw),prefix+'.word_count','baseline')
        metadata(y.get('word_count'),len(bw),prefix+'.word_count','actual')
        check(len(aw)==len(bw),prefix+'.word_count',len(aw),len(bw))
        for j,(u,v) in enumerate(zip(aw,bw)):
            wp=f'{prefix}.words[{j}]'
            u=record(u,wp,'baseline');v=record(v,wp,'actual')
            metadata(u.get('index'),j,wp+'.index','baseline')
            metadata(v.get('index'),j,wp+'.index','actual')
            text(u.get('text'),v.get('text'),wp+'.text')
            box(u.get('bounding_box'),v.get('bounding_box'),wp+'.bounding_box')
            bounded(u.get('confidence'),0,1,wp+'.confidence','baseline')
            bounded(v.get('confidence'),0,1,wp+'.confidence','actual')
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
