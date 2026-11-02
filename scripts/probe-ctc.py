"""Replay the fixed local DLL's sparse CTC preparation in an isolated process.

Consumes previously recorded logits. No files in bin/ are changed, and no
model, library or executable is downloaded. Native temporary allocations die
with this disposable process.
"""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'
class Vector(C.Structure):
    _fields_=[('begin',C.c_uint64),('end',C.c_uint64),('capacity',C.c_uint64)]
def u64(address):return C.c_uint64.from_address(address).value
def i32(address):return C.c_int32.from_address(address).value
def f32(address):return C.c_float.from_address(address).value
def native_string(address):
    size=u64(address+16)
    if size>4096:raise ValueError('Native name bounds')
    return C.string_at(u64(address) if u64(address+24)>15 else address,size).decode('utf8')
def character_info(info):
    result=[];character_array=u64(info+8)
    count=i32(info)
    if not(1<count<=50000):raise ValueError('Character metadata count')
    for i in range(count):
        address=character_array+i*80
        result.append({'id':i,'logical_id':i32(address),'physical_id':i32(address+4),'type':i32(address+8),'logical':native_string(address+16),'physical':native_string(address+48)})
    return result
def read_csc(address):
    rows=i32(address+16);cols=i32(address+8)
    if not(1<=rows<=50000 and 1<=cols<=100000):raise ValueError('Native CSC dimensions: '+str((rows,cols)))
    offsets=np.ctypeslib.as_array(C.cast(u64(address+24),C.POINTER(C.c_int32)),shape=(cols+1,)).copy()
    size=int(offsets[-1])
    if not(0<=size<=rows*cols):raise ValueError('Native CSC count')
    values=np.ctypeslib.as_array(C.cast(u64(address+40),C.POINTER(C.c_float)),shape=(size,)).copy()
    indices=np.ctypeslib.as_array(C.cast(u64(address+48),C.POINTER(C.c_int32)),shape=(size,)).copy()
    return {'shape':np.array([rows,cols]),'offsets':offsets,'values':values,'indices':indices}
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--trace',type=Path)
    parser.add_argument('--metadata-only',action='store_true')
    parser.add_argument('--call',type=int,default=0,help='Recognizer call ordinal')
    parser.add_argument('--language',default='LatinPrintedV2')
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args();binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise SystemExit('Unsupported sample hash')
    kernel=C.WinDLL('kernel32',use_last_error=True);kernel.SetDllDirectoryW.argtypes=[C.c_wchar_p]
    if not kernel.SetDllDirectoryW(str(binary.parent)):raise C.WinError(C.get_last_error())
    dll=C.WinDLL(str(binary));options=C.c_int64();pipeline=C.c_int64()
    dll.CreateOcrInitOptions.argtypes=[C.POINTER(C.c_int64)];dll.CreateOcrInitOptions.restype=C.c_int64
    dll.CreateOcrPipeline.argtypes=[C.c_char_p,C.c_char_p,C.c_int64,C.POINTER(C.c_int64)];dll.CreateOcrPipeline.restype=C.c_int64
    dll.ReleaseOcrPipeline.argtypes=[C.c_int64];dll.ReleaseOcrInitOptions.argtypes=[C.c_int64]
    try:
        if dll.CreateOcrInitOptions(C.byref(options)):raise RuntimeError('Options initialization failed')
        code=dll.CreateOcrPipeline(str(ROOT/'bin/oneocr.onemodel').encode(),b'kj)TGtrK>f]b[Piow.gU+nC@s""""""4',options,C.byref(pipeline))
        if code:raise RuntimeError('Pipeline initialization failed: '+str(code))
        recognizer=u64(pipeline.value+48);head=u64(recognizer+0x3f8)
        queue=[u64(head+8)];seen=set();model=None
        while queue:
            node=queue.pop()
            if node in seen or C.c_uint8.from_address(node+25).value:continue
            seen.add(node)
            if len(seen)>100:raise ValueError('Native character model tree bounds')
            if native_string(node+32)==args.language:model=u64(node+128);break
            queue.extend([u64(node),u64(node+16)])
        if not model:raise ValueError('Character model unavailable')
        info=model+16
        if args.metadata_only:
            args.output.parent.mkdir(parents=True,exist_ok=True)
            args.output.write_text(json.dumps({'source_sha256':SHA,'model':args.language,'characters':character_info(info)},ensure_ascii=False,indent=2),encoding='utf8')
            print(json.dumps({'output':str(args.output),'characters':i32(info)}));return
        if not args.trace:parser.error('--trace is required unless --metadata-only is specified')
        entries=[x for x in json.loads((args.trace/'trace.json').read_text(encoding='utf8')) if x['model'].startswith('recognizers/')]
        logits=np.load(args.trace/entries[args.call]['outputs']['logsoftmax']['file'])[:,0,:]
        probabilities=np.ascontiguousarray(np.exp(logits),dtype=np.float32)
        data=(C.c_uint64*2)(probabilities.size,probabilities.ctypes.data);view=C.create_string_buffer(32)
        sparse=C.CFUNCTYPE(None,C.c_void_p,C.c_int32,C.c_int32,C.c_void_p,C.c_float,C.c_int32,C.c_int32)(dll._handle+0xc9710)
        sparse(view,logits.shape[1],logits.shape[0],data,f32(info+0xb4),i32(info+0xb8),i32(info+0x30))
        result=C.create_string_buffer(208)
        prepare=C.CFUNCTYPE(C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p,C.c_float,C.c_float,C.c_float,C.c_int32,C.c_uint8)(dll._handle+0x97cf0)
        prepare(result,view,info,f32(info+0xbc),f32(info+0xc0),f32(info+0xb0),i32(info+0xb8),C.c_uint8.from_address(info+0xc4).value)
        arrays={}
        for name,offset in [('physical',0x28),('logical',0x70)]:
            for key,value in read_csc(C.addressof(result)+offset).items():arrays[name+'_'+key]=value
        arrays['discard']=np.ctypeslib.as_array(C.cast(u64(C.addressof(result)+0xb8),C.POINTER(C.c_int32)),shape=(len(logits),)).copy()
        argmax=C.CFUNCTYPE(C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0xca950);indices=Vector();argmax(result,C.byref(indices))
        arrays['argmax']=np.ctypeslib.as_array(C.cast(indices.begin,C.POINTER(C.c_int32)),shape=((indices.end-indices.begin)//4,)).copy()
        args.output.parent.mkdir(parents=True,exist_ok=True);np.savez(args.output,**arrays)
        characters=character_info(info)
        args.output.with_suffix('.json').write_text(json.dumps({'source_sha256':SHA,'model':args.language,'call':args.call,'logits_shape':list(logits.shape),'info':{'blank':i32(info+0x30),'space':i32(info+0x34),'trash':i32(info+0x38),'minimum_probability':f32(info+0xb4),'log_minimum_probability':f32(info+0xb0),'top_k':i32(info+0xb8),'log_blank_threshold':f32(info+0xbc),'log_character_threshold':f32(info+0xc0)},'characters':characters},indent=2),encoding='utf8')
        print(json.dumps({'output':str(args.output),'physical_shape':arrays['physical_shape'].tolist(),'logical_shape':arrays['logical_shape'].tolist()}))
    finally:
        if pipeline.value:dll.ReleaseOcrPipeline(pipeline)
        if options.value:dll.ReleaseOcrInitOptions(options)
if __name__=='__main__':main()
