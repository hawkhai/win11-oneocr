"""Probe fixed local classifier decisions in a disposable Python process.

The synthetic objects contain only the recovered classifier configuration;
all decision functions run from the original hash-checked DLL. No model changes.
"""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--trace',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unexpected DLL hash')
    dll=C.WinDLL(str(binary));allocations=[]
    def buffer(size):
        value=C.create_string_buffer(size);allocations.append(value);return C.addressof(value)
    def pack(address,fmt,*values):struct.pack_into(fmt,(C.c_char*struct.calcsize(fmt)).from_address(address),0,*values)
    def tree(entries,key_format,value_format,key_offset,value_offset):
        head=buffer(56);pack(head+24,'<BB',1,1)
        def node(items):
            if not items:return head
            middle=len(items)//2;address=buffer(56);left=node(items[:middle]);right=node(items[middle+1:]);pack(address,'<QQQ',left,head,right);pack(address+key_offset,key_format,items[middle][0]);pack(address+value_offset,value_format,items[middle][1]);return address
        root=node(sorted(entries));pack(head,'<QQQ',head,root,head);return head
    classifier=buffer(0xb0);pack(classifier+16,'<dd',4.1,3)
    # +0x18 is an integer count, not a double.
    pack(classifier+24,'<Q',3)
    pack(classifier+32,'<Q',tree([(2,3),(4,1),(6,2),(7,3),(8,3),(9,3),(10,3)],'<i','<Q',32,40))
    pack(classifier+48,'<d',5)
    pack(classifier+56,'<Q',tree([(-2.,1),(0.,0),(float('inf'),2)],'<d','<i',32,40));pack(classifier+136,'<B',1)
    script=C.CFUNCTYPE(C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x86f80)
    style=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_float,C.c_void_p)(dll._handle+0x87750)
    resolve=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x7c580)
    def span(values):
        values=np.ascontiguousarray(values,dtype=np.float32);return values,(C.c_uint64*2)(values.size,values.ctypes.data)
    cases=[]
    for entry in json.loads((args.trace/'trace.json').read_text(encoding='utf8')):
        if not entry['model'].startswith('classifier/'):continue
        load=lambda key:np.load(args.trace/entry['outputs'][key]['file']).reshape(-1)
        cases.append({'scores':load('script_id_score').tolist(),'script_frames':load('script_id_perframe_predict').tolist(),'handwriting':float(load('hwpc_score')[0]),'handwriting_frames':load('hwpc_perframe_predict').tolist()})
    for winning in range(10):
        for margin in (0.,4.099,4.1,4.101,5.):
            scores=[0.]*10;scores[winning]=margin
            cases.append({'scores':scores,'script_frames':scores*5,'handwriting':-3.,'handwriting_frames':[0.,3.]*5})
    for score in (-2.001,-2.,-1.999,-.001,0.,.001,2.):
        for run in (0,2,3,4,6):
            cases.append({'scores':[0.,0.,20.]+[0.]*7,'script_frames':[],'handwriting':score,'handwriting_frames':[2.,0.]*run+[0.,2.]})
    global_cases=[]
    for case in cases:
        values,view=span(case['scores']);frames,frame_view=span(case['script_frames']);result=buffer(8)
        script(classifier,result,view,frame_view);case['script']=C.c_int.from_address(result).value
        handwriting,hw_view=span(case['handwriting_frames']);case['style']=style(classifier,case['handwriting'],hw_view)
        probabilities=np.exp(values);probabilities/=np.sum(probabilities,dtype=np.float32)
        if len(global_cases)<80:
            counts=np.zeros(12,dtype=np.uint64);counts[0]=5;counts[2]=len(global_cases)%5;counts[3]=3;counts[11]=6
            data,view=span(probabilities);vector=(C.c_uint64*3)(data.ctypes.data,data.ctypes.data+data.nbytes,data.ctypes.data+data.nbytes)
            count_vector=(C.c_uint64*3)(counts.ctypes.data,counts.ctypes.data+counts.nbytes,counts.ctypes.data+counts.nbytes)
            global_cases.append({'probabilities':probabilities.tolist(),'counts':counts.tolist(),'script':resolve(None,vector,count_vector)})
    result={'source_sha256':SHA,'functions':['0x86f80','0x87750','0x7c580'],'cases':cases,'global_cases':global_cases}
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2),encoding='utf8');print(len(cases),'native classification probes;',len(global_cases),'global script probes')

if __name__=='__main__':main()
