"""Replay fragment chain construction in a disposable, version-locked DLL process."""
import argparse
import ctypes as C
import importlib.util
import json
from pathlib import Path
import numpy as np

spec=importlib.util.spec_from_file_location('detector_probe',Path(__file__).with_name('probe-detector.py'))
probe=importlib.util.module_from_spec(spec);spec.loader.exec_module(probe)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--segments',type=Path,action='append',default=[]);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    import hashlib
    binary=probe.ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=probe.SAMPLE_SHA:raise ValueError('Unexpected DLL version')
    kernel=C.WinDLL('kernel32');kernel.SetDllDirectoryW.argtypes=[C.c_wchar_p];kernel.SetDllDirectoryW(str(binary.parent));dll=C.WinDLL(str(binary))
    initialize=C.CFUNCTYPE(C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_float,C.c_uint8)(dll._handle+0x15d380)
    chains=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_void_p,C.c_uint8)(dll._handle+0x15d8a0)
    groups=[]
    for source in a.segments:
        for group in json.loads(source.read_text(encoding='utf8')):groups.append({**group,'source':str(source)})
    rng=np.random.default_rng(20261102)
    for horizontal in (True,False):
        for n in (1,2,3,5,12):
            for trial in range(10):
                segments=[]
                for i in range(n):
                    x=float(rng.uniform(0,100));y=float(rng.uniform(0,15));w=float(rng.uniform(12,40));h=float(rng.uniform(10,20));sl=float(rng.uniform(-.3,.3))
                    q=np.array([[x,y],[x+w,y+sl*w],[x+w,y+sl*w+h],[x,y+h]],np.float32)
                    if not horizontal:q=q[:,::-1][[0,3,2,1]]
                    segments.append({'cell':i,'score':float(np.float32(rng.uniform(.7,1))),'box':q.reshape(-1).tolist()})
                groups.append({'vertical':not horizontal,'segments':segments,'source':'synthetic','trial':trial})
    records=[]
    for group in groups:
        horizontal=not group['vertical'];objects=[];keys=[];addresses={}
        for i,s in enumerate(group['segments']):
            q=np.array(s['box'],np.float32).reshape(4,2);b=np.array([*q.min(axis=0),*q.max(axis=0)],np.float32);obj=C.create_string_buffer(88)
            initialize(obj,q.ctypes.data,b.ctypes.data,s['score'],horizontal);objects.append(obj);addresses[C.addressof(obj)]=i
            keys.append(float((q[0,0]+q[3,0])*.5) if horizontal else float((q[0,1]+q[1,1])*.5))
        pointers=np.array([C.addressof(o)for o in objects],np.uint64);order=np.argsort(np.array(keys,np.float32),kind='stable').astype(np.int64)
        pv=probe.Vector(pointers.ctypes.data,pointers.ctypes.data+pointers.nbytes,pointers.ctypes.data+pointers.nbytes);ov=probe.Vector(order.ctypes.data,order.ctypes.data+order.nbytes,order.ctypes.data+order.nbytes);output=probe.Vector()
        chains(C.byref(pv),C.byref(ov),C.byref(output),horizontal)
        count=((output.end or 0)-(output.begin or 0))//24;result=[]
        for i in range(count):
            v=probe.Vector.from_address(output.begin+i*24);result.append([addresses[C.c_uint64.from_address(j).value]for j in range(v.begin,v.end,8)])
        records.append({'horizontal':horizontal,'segments':group['segments'],'order':order.tolist(),'chains':result,'source':group['source']})
    constants={hex(rva):C.c_float.from_address(dll._handle+rva).value for rva in (0x2576324,0x2576350,0x2576368,0x2576544,0x25765b4)}
    report={'source_sha256':probe.SAMPLE_SHA,'function_rva':'0x15d8a0','constants':constants,'records':records};a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,indent=2),encoding='utf8');print(json.dumps({'records':len(records),'constants':constants}))
if __name__=='__main__':main()
