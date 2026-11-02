"""Exercise the original post-recognition orientation selection, offline."""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import random
import struct
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unexpected DLL hash')
    dll=C.WinDLL(str(binary));initialize=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_uint8,C.c_float,C.c_int32)(dll._handle+0xe1200)
    nms=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x5f040)
    rng=random.Random(20261102);cases=[]
    for count in (2,3,6):
        for trial in range(24):
            lines=[]
            for i in range(count):
                horizontal=i%2==0;x,y=rng.randrange(0,40),rng.randrange(0,20);w,h=rng.randrange(10,110),rng.randrange(10,30)
                polygon=[x,y,x+w,y,x+w,y+h,x,y+h]
                if not horizontal:polygon=[x+w,y,x+w,y+h,x,y+h,x,y]
                if trial%3==0:
                    # A three-point edge activates the reliable-line vote.
                    a=polygon;polygon=a[:2]+[(a[0]+a[2])/2,(a[1]+a[3])/2]+a[2:6]+[(a[4]+a[6])/2,(a[5]+a[7])/2]+a[6:]
                lines.append({'polygon':polygon,'horizontal':horizontal,'text':rng.choice(['A','AB','123','中文','+','XY']), 'confidence_score':rng.random(),'language_score':rng.random()/10,'script':rng.choice([2,3]),'style':rng.choice([0,1,2])})
            cases.append({'lines':lines})
    for case in cases:
        allocations=[]
        def buffer(size):
            value=C.create_string_buffer(size);allocations.append(value);return value
        def ptr(value):return C.addressof(value)
        def string(destination,value):
            data=value.encode('utf8');struct.pack_into('<QQ',destination,16,len(data),15 if len(data)<=15 else len(data))
            if len(data)<=15:destination[:len(data)]=data
            else:
                storage=buffer(len(data)+1);storage[:len(data)]=data;struct.pack_into('<Q',destination,0,ptr(storage))
        count=len(case['lines']);text_lines=(C.c_uint64*count)();results=buffer(count*152)
        for i,line in enumerate(case['lines']):
            points=np.array(line['polygon'],np.float32);allocations.append(points);view=(C.c_uint64*3)(points.ctypes.data,points.ctypes.data+points.nbytes,points.ctypes.data+points.nbytes)
            polygon=buffer(512);initialize(polygon,view,line['horizontal'],.99,2);line['min_rectangle']=bool(polygon[2][0])
            text=buffer(0x170);struct.pack_into('<Q',text,0x118,ptr(polygon));struct.pack_into('<B',text,299,line['horizontal']);struct.pack_into('<ii',text,0x13c,line['style'],0);struct.pack_into('<i',text,0x148,line['script']);text_lines[i]=ptr(text)
            word=buffer(224);name=(C.c_char*32).from_buffer(word,128);string(name,line['text'])
            struct.pack_into('<Q',results,i*152,i);struct.pack_into('<QQQ',results,i*152+64,ptr(word),ptr(word)+224,ptr(word)+224)
            struct.pack_into('<ff',results,i*152+92,line['confidence_score'],line['language_score'])
        text_vector=(C.c_uint64*3)(ptr(text_lines),ptr(text_lines)+count*8,ptr(text_lines)+count*8)
        result_vector=(C.c_uint64*3)(ptr(results),ptr(results)+count*152,ptr(results)+count*152);output=(C.c_uint64*6)();options=buffer(256)
        nms(output,text_vector,result_vector,options)
        case['kept']=[C.c_uint64.from_address(output[0]+j*152).value for j in range((output[1]-output[0])//152)]
        if any(i>=count for i in case['kept']):raise ValueError('Native result bounds')
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps({'source_sha256':SHA,'function':'0x5f040','cases':cases},ensure_ascii=False,indent=2),encoding='utf8');print(len(cases),'native post-recognition orientation cases')
if __name__=='__main__':main()
