"""Version-locked offline internal geometry oracle. Run in a disposable process."""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
OLD_SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'
PHOTOS_SHA='0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0'
class Vector(C.Structure):
    _fields_=[('begin',C.c_void_p),('end',C.c_void_p),('capacity',C.c_void_p)]
def vector(a):return Vector(a.ctypes.data,a.ctypes.data+a.nbytes,a.ctypes.data+a.nbytes)
def values(v,dtype):
    length=(v.end or 0)-(v.begin or 0)
    if length<0 or length%C.sizeof(dtype) or length>1000000:raise ValueError('Oracle output vector bounds')
    return list((dtype*(length//C.sizeof(dtype))).from_address(v.begin)) if length else []

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--photos',action='store_true')
    args=p.parse_args();binary=ROOT/('bin/photos-2026.11090.22001.0/oneocr.dll' if args.photos else 'bin/oneocr.dll')
    expected=PHOTOS_SHA if args.photos else OLD_SHA
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=expected:raise ValueError('Unsupported oracle DLL')
    offsets=[0x58a380,0x58b580,0x524d80,0x52ab40,0x25732e8] if args.photos else [0xe4ea0,0xe48d0,0xd9c40,0xdc510,0x256b400]
    import os
    directory=os.add_dll_directory(str(binary.parent));dll=C.WinDLL(str(binary))
    fit=C.CFUNCTYPE(C.c_bool,C.c_int,C.c_void_p,C.c_void_p,C.c_int,C.c_void_p)(dll._handle+offsets[0])
    fallback=C.CFUNCTYPE(C.c_bool,C.c_int,C.c_void_p,C.c_void_p,C.c_int,C.c_void_p)(dll._handle+offsets[1])
    records=[]
    for xs,ys in [([0,0,0],[2,4,6]),([1,1,1],[2,4,6]),([2,2,2,2],[1,2,3,4]),([0,0,1,1],[2,4,6,8]),([3,3],[1,5]),([5],[7]),([1000]*4,[4,5,6,7]),([1,1,1,2],[2,3,4,8])]:
        x=np.array(xs,np.float64);y=np.array(ys,np.float64);out=Vector();other=Vector()
        ok=fit(len(x),x.ctypes.data,y.ctypes.data,2,C.byref(out))
        ok2=fallback(len(x),x.ctypes.data,y.ctypes.data,2,C.byref(other))
        records.append({'x':xs,'y':ys,'lu_success':bool(ok),'lu':values(out,C.c_float),'svd_success':bool(ok2),'svd':values(other,C.c_float)})
    widths=[]
    width=C.CFUNCTYPE(None,C.c_void_p,C.c_float,C.c_int64,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+offsets[2])
    polygons=[[[0,5],*[[x,5] for x in distances],*[[x,65] for x in reversed(distances)],[0,65]]
              for distances in [[100,200,300],[100,296,350],[296,350,400],[297,350,400],[298,350,400],[100,200,250]]]
    polygons.append([[10,10],[100,12],[200,6],[310,18],[310,78],[200,66],[100,72],[10,70]])
    for points in polygons:
        poly=np.array(points,np.float32)
        context=C.create_string_buffer(32);struct.pack_into('<i',context,16,60)
        options=C.create_string_buffer(64);struct.pack_into('<ii',options,4,1,1)
        output=Vector();height=C.c_int();target=np.zeros_like(poly);vpoly=vector(poly);vt=vector(target)
        width(context,60,len(poly)//2,C.byref(vpoly),options,C.byref(output),C.byref(height),C.byref(vt))
        widths.append({'polygon':poly.reshape(-1).tolist(),'maximum_width':297,'positions':values(output,C.c_float),'target':target.reshape(-1).tolist(),'height':height.value})
    pixels=[]
    normalizer=C.CFUNCTYPE(None,*([C.c_void_p]*7))(dll._handle+offsets[3])
    construct_mapper=C.CFUNCTYPE(None,*([C.c_void_p]*3))(dll._handle+(0x52fad0 if args.photos else 0xddd60))
    map_point=C.CFUNCTYPE(C.c_void_p,*([C.c_void_p]*3))(dll._handle+(0x52f770 if args.photos else 0xde340))
    def mat(data=None):
        header=C.create_string_buffer(96);address=C.addressof(header)
        struct.pack_into('<QQ',header,64,address+8,address+80)
        struct.pack_into('<i',header,0,0x42ff0000)
        if data is not None:
            h,w,_=data.shape;struct.pack_into('<4i',header,0,0x42ff4010,2,h,w)
            struct.pack_into('<QQQQ',header,16,data.ctypes.data,data.ctypes.data,data.ctypes.data+data.nbytes,data.ctypes.data+data.nbytes)
            struct.pack_into('<QQ',header,80,w*3,3)
        return header
    yy,xx=np.indices((100,420));image=np.stack([(xx+yy)%256,(xx*3+yy*2)%256,(xx*7+yy*5)%256],axis=-1).astype(np.uint8)
    for n,item in enumerate(widths):
        poly=np.array(item['polygon'],np.float32).reshape(-1,2);pv=vector(poly)
        context=C.create_string_buffer(32);struct.pack_into('<Q',context,0,dll._handle+offsets[4]);struct.pack_into('<i',context,16,60)
        options=C.create_string_buffer(128);struct.pack_into('<ii',options,4,1,1)
        source=mat(image);output=mat();height=C.c_float();inverse=Vector()
        normalizer(context,source,C.byref(pv),output,C.byref(height),C.byref(inverse),options)
        rows,cols=struct.unpack_from('<ii',output,8);data=struct.unpack_from('<Q',output,16)[0];step=struct.unpack_from('<Q',output,80)[0]
        if rows!=60 or not 0<cols<=297 or step<cols*3:raise ValueError('Native normalized image bounds')
        payload=b''.join(C.string_at(data+y*step,cols*3) for y in range(rows))
        file=args.output.stem+f'-pixels-{n}.rgb';args.output.parent.mkdir(parents=True,exist_ok=True);(args.output.parent/file).write_bytes(payload)
        mapper=C.create_string_buffer(256);bounds=np.array([0,0,419,0,419,99,0,99],np.float32)
        construct_mapper(mapper,C.byref(inverse),bounds.ctypes.data)
        words=[]
        for start,end in [(0,1),(90,110),(195,250),(275,297)]:
            for flipped in [False,True]:
                points=[[start,0],[end,0],[end,60],[start,60]];result=[]
                for k,(x,y) in enumerate(points):
                    if flipped:x=cols-(1 if k in [0,3] else 0)-x;y=60-y
                    source_point=np.array([x,y],np.float32);mapped=np.zeros(2,np.float32)
                    map_point(mapper,mapped.ctypes.data,source_point.ctypes.data)
                    result.extend([float(np.clip(mapped[0],poly[:,0].min(),poly[:,0].max())),float(np.clip(mapped[1],poly[:,1].min(),poly[:,1].max()))])
                words.append({'start':start,'end':end,'flipped':flipped,'box':result})
        pixels.append({'case':n,'width':cols,'height':rows,'thickness':height.value,'file':file,'sha256':hashlib.sha256(payload).hexdigest(),'inverse':values(inverse,C.c_float),'words':words})
    report={'dll_sha256':expected,'functions':dict(zip(['lu','svd','width','normalize','vtable'],map(hex,offsets))),'polynomials':records,'widths':widths,'pixels':pixels}
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'polynomials':records,'widths':widths},indent=2))
if __name__=='__main__':main()
