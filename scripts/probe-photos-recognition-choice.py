"""Replay recorded candidates through fixed Photos orientation NMS, offline.

Synthetic native structs exist only in this disposable process. Input byte
buffers remain alive throughout the call; original files are never modified.
"""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import math

ROOT=Path(__file__).resolve().parents[1]
SHA='0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0'
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--candidates',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    binary=ROOT/'bin/photos-2026.11090.22001.0/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unsupported DLL')
    dll=C.WinDLL(str(binary));lines=json.loads(a.candidates.read_text(encoding='utf8'));allocations=[]
    def buffer(n):v=C.create_string_buffer(n);allocations.append(v);return v
    def ptr(v):return C.addressof(v)
    count=len(lines);text_lines=(C.c_uint64*count)();results=buffer(count*144)
    for i,line in enumerate(lines):
        points=line['polygon'];n=len(points)//2;half=n//2
        raw=(C.c_float*len(points))(*points);allocations.append(raw);bp=ptr(raw);size=half*8
        poly=buffer(512);bounds=[math.floor(min(points[::2])),math.ceil(max(points[::2])),math.floor(min(points[1::2])),math.ceil(max(points[1::2]))]
        area=abs(sum(points[k*2]*points[((k+1)%n)*2+1]-points[((k+1)%n)*2]*points[k*2+1]for k in range(n)))*.5
        struct.pack_into('<4f',poly,4,*bounds);struct.pack_into('<fffiB',poly,20,.99,area,(bounds[1]-bounds[0]+1)*(bounds[3]-bounds[2]+1),i,line['horizontal'])
        struct.pack_into('<3Q',poly,64,bp,bp+size,bp+size);struct.pack_into('<3Q',poly,88,bp+size,bp+size*2,bp+size*2)
        poly[0]=bytes([n>4]);poly[1]=bytes([line.get('ambiguous',False)])
        text=buffer(0x200);struct.pack_into('<Q',text,0x148,ptr(poly));struct.pack_into('<B',text,0x15a,line['horizontal']);struct.pack_into('<i',text,0x170,line['style']);struct.pack_into('<i',text,0x17c,line['script']);text_lines[i]=ptr(text)
        word=buffer(224);data=line['text'].encode('utf8');destination=(C.c_char*32).from_buffer(word,128)
        struct.pack_into('<QQ',destination,16,len(data),15 if len(data)<=15 else len(data))
        if len(data)<=15:destination[:len(data)]=data
        else:storage=buffer(len(data)+1);storage[:len(data)]=data;struct.pack_into('<Q',destination,0,ptr(storage))
        struct.pack_into('<Q',results,i*144,i);struct.pack_into('<3Q',results,i*144+64,ptr(word),ptr(word)+224,ptr(word)+224);struct.pack_into('<ff',results,i*144+92,line['confidence_score'],line['language_score'])
    text_vector=(C.c_uint64*3)(ptr(text_lines),ptr(text_lines)+count*8,ptr(text_lines)+count*8)
    result_vector=(C.c_uint64*3)(ptr(results),ptr(results)+count*144,ptr(results)+count*144);out=(C.c_uint64*6)();options=buffer(256)
    function=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x491610);function(out,text_vector,result_vector,options)
    if out[1]<out[0] or (out[1]-out[0])%144 or (out[1]-out[0])//144>count:raise ValueError('Original result bounds')
    kept=[C.c_uint64.from_address(out[0]+i*144).value for i in range((out[1]-out[0])//144)]
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps({'dll_sha256':SHA,'function_rva':'0x491610','input_sha256':hashlib.sha256(a.candidates.read_bytes()).hexdigest(),'lines':lines,'kept':kept},ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Original kept',kept,'CPP kept',[i for i,x in enumerate(lines)if not x['suppressed']])
if __name__=='__main__':main()
