"""Probe the fixed DLL's complete reading sort, including large-vector paths."""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import random
ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--book',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unexpected DLL hash')
    dll=C.WinDLL(str(binary));sort=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_int64,C.c_uint64)(dll._handle+0x72040)
    rng=random.Random(20261102);cases=[]
    for count in (2,20,32,33,41,72,105,160):
        boxes=[]
        for i in range(count):
            x,y,w,h=rng.uniform(0,900),rng.uniform(0,1000),rng.uniform(20,350),rng.uniform(8,45);boxes.append([x,y,x+w,y,x+w,y+h,x,y+h])
        cases.append({'boxes':boxes,'angle':0,'rtl':False});cases.append({'boxes':boxes,'angle':3,'rtl':True})
    book=json.loads(args.book.read_text(encoding='utf8'));cases.append({'boxes':[x['bounding_box']for x in book['lines']],'angle':book['image_angle'],'rtl':False})
    for case in cases:
        count=len(case['boxes']);buffer=C.create_string_buffer(count*152)
        for i,box in enumerate(case['boxes']):struct.pack_into('<Q8f',buffer,i*152,i,*box)
        flags=struct.unpack('<Q',struct.pack('<If',int(case['rtl']),case['angle']))[0]
        sort(buffer,C.addressof(buffer)+count*152,count,flags)
        case['order']=[struct.unpack_from('<Q',buffer,i*152)[0]for i in range(count)]
        if sorted(case['order'])!=list(range(count)):raise ValueError('Invalid native sort result')
    args.output.write_text(json.dumps({'source_sha256':SHA,'function':'0x72040','cases':cases},indent=2));print(len(cases),'native complete sort probes')
if __name__=='__main__':main()
