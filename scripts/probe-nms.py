"""Isolated secondary-NMS oracle for the fixed local DLL; no network access."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'

def main():
    import argparse
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--geometry',type=Path,required=True);p.add_argument('--output',type=Path,required=True);args=p.parse_args()
    binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unsupported DLL sample')
    dll=C.WinDLL(str(binary))
    initialize=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_uint8,C.c_float,C.c_int32)(dll._handle+0xe1200)
    intersection=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_void_p)(dll._handle+0xe2f60)
    possible=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_int,C.c_double)(dll._handle+0xdff10)
    iou=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_void_p,C.c_int,C.c_void_p)(dll._handle+0xdfea0)
    covered=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_float,C.c_void_p)(dll._handle+0xe2380)
    join=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_double,C.c_double,C.c_void_p)(dll._handle+0xe1ea0)
    polygons=[]
    for batch in json.loads(args.geometry.read_text()):
        for item in batch['polygons']:polygons.append({'points':item['upper']+item['lower'],'horizontal':item['horizontal']})
    # Include containment, partial overlap, crossing, disjoint, and both axes.
    for horizontal in (False,True):
        for x,y,w,h in [(0,0,100,20),(20,2,40,15),(80,0,100,20),(200,30,50,20),(20,0,20,100),(0,15,100,20)]:
            polygons.append({'points':[x,y,x+w,y,x+w,y+h,x,y+h],'horizontal':horizontal})
    objects=[];keep=[]
    for i,item in enumerate(polygons):
        points=np.array(item['points'],np.float32);vector=(C.c_uint64*3)(points.ctypes.data,points.ctypes.data+points.nbytes,points.ctypes.data+points.nbytes)
        obj=C.create_string_buffer(512);initialize(obj,vector,item['horizontal'],.99,2);struct.pack_into('<i',obj,32,i);objects.append(obj);keep.extend([points,vector])
    comparisons=[]
    for i,a in enumerate(objects):
        for j,b in enumerate(objects):
            if i==j:continue
            area=float(intersection(a,b))
            # A read-only one-entry MSVC map supplies the original intersection
            # cache's result; it never contains or calls a foreign allocation.
            head=C.create_string_buffer(48);node=C.create_string_buffer(48);hp=C.addressof(head);np_=C.addressof(node)
            struct.pack_into('<3Q',head,0,np_,np_,np_);struct.pack_into('<H',head,24,0x101)
            struct.pack_into('<3Q',node,0,hp,hp,hp);struct.pack_into('<iif',node,28,min(i,j),max(i,j),area)
            cache=(C.c_uint64*2)(hp,1)
            record={'first':i,'second':j,'intersection':area,'covered':bool(covered(a,b,.4,cache))}
            record['primary_possible']=bool(possible(a,b,0,float(np.float32(.2))))
            record['iou']=float(iou(a,b,0,cache))
            points=np.array(polygons[i]['points'],np.float32);vector=(C.c_uint64*3)(points.ctypes.data,points.ctypes.data+points.nbytes,points.ctypes.data+points.nbytes)
            fresh=C.create_string_buffer(512);initialize(fresh,vector,polygons[i]['horizontal'],.99,2);struct.pack_into('<i',fresh,32,i)
            record['joinable']=bool(join(fresh,b,4.,2.,cache))
            if record['joinable']:
                merged=[]
                for start,end in [(64,72),(88,96)]:
                    begin=struct.unpack_from('<Q',fresh,start)[0];last=struct.unpack_from('<Q',fresh,end)[0]
                    merged.extend(list((C.c_float*((last-begin)//4)).from_address(begin)))
                record['joined_polygon']=merged
            comparisons.append(record)
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps({'source_sha256':SHA,'functions':['0xe1200','0xe2f60','0xe2380'],'polygons':polygons,'comparisons':comparisons},indent=2))
    print({'polygons':len(polygons),'comparisons':len(comparisons)})

if __name__=='__main__':main()
