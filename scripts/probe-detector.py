"""Isolated, version-locked probes of the local DLL's detector arithmetic.

Run this script as a separate process. It calls only the fixed local sample;
it never patches or replaces a binary or downloads models.
"""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
SAMPLE_SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'

class Matrix(C.Structure):
    _fields_=[('data',C.c_void_p),('rows',C.c_int64),('cols',C.c_int64)]

def matrix(value):
    return Matrix(value.ctypes.data,*value.shape)

class Vector(C.Structure):
    _fields_=[('begin',C.c_void_p),('end',C.c_void_p),('capacity',C.c_void_p)]

class Tensor(C.Structure):
    _fields_=[('data',C.c_void_p),('dimensions',Vector)]

def proposal_probe(dll,trace,level,direction):
    entry=next(e for e in json.loads((trace/'trace.json').read_text(encoding='utf8')) if e['model'].startswith('detector/'))
    tensors=[];keep=[]
    for kind in ('scores','bbox_deltas','link_scores'):
        value=np.ascontiguousarray(np.load(trace/entry['outputs'][f'{kind}_{direction}_fpn{level}']['file']))
        dims=np.array(value.shape[1:],dtype=np.int64);keep.extend((value,dims))
        tensors.append(Tensor(value.ctypes.data,Vector(dims.ctypes.data,dims.ctypes.data+dims.nbytes,dims.ctypes.data+dims.nbytes)))
    info=np.ascontiguousarray(np.load(trace/entry['inputs']['im_info']['file']))
    image_info=Vector(info.ctypes.data,info.ctypes.data+info.nbytes,info.ctypes.data+info.nbytes)
    import struct
    config=C.create_string_buffer(256)
    stride=2**level
    struct.pack_into('<5q4fq',config,0,stride,stride-1,32*2**(level-2)-1,8,5,.7,.8,.7,.5,5)
    output=Vector()
    function=C.CFUNCTYPE(None,*([C.c_void_p]*5),C.c_uint8,C.c_uint8,*([C.c_void_p]*5))(dll._handle+0x156fc0)
    function(config,*[C.byref(t) for t in tensors],C.byref(image_info),direction=='hori',0,C.byref(output),None,None,None,None)
    length=(output.end or 0)-(output.begin or 0)
    if length<0 or length%48 or length>48000000:raise ValueError('Invalid proposal output size')
    values=np.ctypeslib.as_array(C.cast(output.begin,C.POINTER(C.c_float)),shape=(length//4,)).reshape(-1,12).copy() if length else np.empty((0,12),np.float32)
    return {'function_rva':'0x156fc0','direction':direction,'level':level,'rows':values.tolist()}

def curve_probe(dll,proposals,scale_x,scale_y):
    source=json.loads(proposals.read_text(encoding='utf8'))
    rows=np.array(source['rows'],dtype=np.float32)
    function=C.CFUNCTYPE(C.c_uint64,C.c_void_p,C.c_float,C.c_void_p)(dll._handle+0xe6620)
    records=[]
    for group in sorted(set(rows[:,0])):
        data=np.ascontiguousarray(rows[rows[:,0]==group,2:])
        data[:,2::2]/=np.float32(scale_x);data[:,3::2]/=np.float32(scale_y)
        vector=Vector(data.ctypes.data,data.ctypes.data+data.nbytes,data.ctypes.data+data.nbytes)
        output=Vector()
        flags=function(C.byref(vector),1 if source['direction']=='hori' else 0,C.byref(output))
        length=(output.end or 0)-(output.begin or 0)
        if length!=18*4:raise ValueError('Unexpected native line curve size')
        result=np.ctypeslib.as_array(C.cast(output.begin,C.POINTER(C.c_float)),shape=(18,)).copy()
        records.append({'group':int(group),'flags':flags,'curve':result.tolist(),'source_segments':data.tolist()})
    return {'function_rva':'0xe6620','direction':source['direction'],'records':records}

def pipeline_probe(dll):
    options,pipeline=C.c_int64(),C.c_int64()
    dll.CreateOcrInitOptions.argtypes=[C.POINTER(C.c_int64)]
    dll.CreateOcrInitOptions.restype=C.c_int64
    dll.CreateOcrPipeline.argtypes=[C.c_char_p,C.c_char_p,C.c_int64,C.POINTER(C.c_int64)]
    dll.CreateOcrPipeline.restype=C.c_int64
    dll.ReleaseOcrPipeline.argtypes=[C.c_int64]
    dll.ReleaseOcrInitOptions.argtypes=[C.c_int64]
    try:
        if dll.CreateOcrInitOptions(C.byref(options)):raise ValueError('Cannot initialize original options')
        code=dll.CreateOcrPipeline(str(ROOT/'bin/oneocr.onemodel').encode('utf8'),b'kj)TGtrK>f]b[Piow.gU+nC@s""""""4',options,C.byref(pipeline))
        if code:raise ValueError('Original pipeline creation failed: '+str(code))
        table=C.c_uint64.from_address(pipeline.value).value
        slots=[hex(C.c_uint64.from_address(table+i*8).value-dll._handle) for i in range(16)]
        return {'direction':'n/a','vtable_rva':hex(table-dll._handle),'slots':slots}
    finally:
        if pipeline.value:dll.ReleaseOcrPipeline(pipeline)
        if options.value:dll.ReleaseOcrInitOptions(options)

def ordering_probe(dll):
    compare=C.CFUNCTYPE(C.c_bool,C.c_void_p,C.c_void_p,C.c_uint8)(dll._handle+0x188600)
    rotate=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_float)(dll._handle+0x641f0)
    rng=np.random.default_rng(20261102)
    cases=[]
    for i in range(32):
        x,y,w,h=rng.uniform([0,0,20,10],[600,300,400,70])
        a=np.array([x,y,x+w,y,x+w,y+h,x,y+h],np.float32)
        x,y,w,h=rng.uniform([0,0,20,10],[600,300,400,70])
        b=np.array([x,y,x+w,y,x+w,y+h,x,y+h],np.float32)
        for rtl in (False,True):cases.append({'first':a.tolist(),'second':b.tolist(),'rtl':rtl,'before':bool(compare(a.ctypes.data,b.ctypes.data,rtl))})
    rotations=[]
    source=np.array([28,42,494,42,494,78,28,78],np.float32)
    for angle in (0,.5,90,180,270,359.5):
        target=np.zeros(8,np.float32);rotate(target.ctypes.data,source.ctypes.data,angle)
        rotations.append({'source':source.tolist(),'angle':angle,'target':target.tolist()})
    return {'direction':'n/a','functions':['0x188600','0x641f0'],'comparisons':cases,'rotations':rotations}

def geometry_probe(dll,source):
    """Replay polygon-to-line geometry in this disposable process only."""
    import struct
    initialize=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_uint8,C.c_float,C.c_int32)(dll._handle+0xe1200)
    line_geometry=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p)(dll._handle+0xd8e70)
    angle=C.CFUNCTYPE(C.c_double,C.c_void_p)(dll._handle+0x59230)
    records=[]
    for batch in json.loads(source.read_text(encoding='utf8')):
        for item in batch['polygons']:
            points=np.array(item['upper']+item['lower'],dtype=np.float32)
            polygon=C.create_string_buffer(160);line=C.create_string_buffer(360)
            vector=Vector(points.ctypes.data,points.ctypes.data+points.nbytes,points.ctypes.data+points.nbytes)
            initialize(polygon,C.byref(vector),item['horizontal'],item['score'],item['level'])
            # The curve detector sets this flag before TextLine construction.
            polygon[0]=bytes([item.get('curved',True)])
            polygon[0x2c]=bytes([item.get('min_rectangle',False)])
            line_geometry(line,polygon)
            box=list(struct.unpack_from('<8f',line.raw,0xc0))
            records.append({**item,'box':box,'angle':angle(C.addressof(line)+0xc0),
                            'bounds':list(struct.unpack_from('<8f',line.raw,0xe0))})
    return {'direction':'n/a','function_rva':'0xd8e70','records':records}

def quad_probe(dll,direction):
    count=18
    boxes=np.tile(np.array([[0,0,31,3]],dtype=np.float32),(count,1))
    if direction=='vert':boxes=np.tile(np.array([[0,0,3,31]],dtype=np.float32),(count,1))
    deltas=np.zeros((count,16),dtype=np.float32)
    for i in range(16):deltas[i+1,i]=1
    deltas[-1,:]=.25
    labels=np.full((count,1),1 if direction=='hori' else 0,dtype=np.float32)
    source,delta,label,result=matrix(boxes),matrix(deltas),matrix(labels),Matrix()
    function=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x158c40)
    function(C.byref(result),C.byref(source),C.byref(delta),C.byref(label))
    if result.rows!=count or result.cols!=8 or not result.data:raise ValueError('Unexpected native quad probe shape')
    output=np.ctypeslib.as_array(C.cast(result.data,C.POINTER(C.c_float)),shape=(count*8,)).reshape(count,8).copy()
    if not np.isfinite(output).all():raise ValueError('Non-finite native quad probe')
    return {'function_rva':'0x158c40','direction':direction,'boxes':boxes.tolist(),'deltas':deltas.tolist(),'labels':labels.tolist(),'quads':output.tolist()}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--direction',choices=['hori','vert'],default='hori')
    parser.add_argument('--mode',choices=['quad','proposals','curve','pipeline','ordering','geometry'],default='quad')
    parser.add_argument('--geometry',type=Path)
    parser.add_argument('--trace',type=Path)
    parser.add_argument('--level',type=int,choices=[2,3,4],default=2)
    parser.add_argument('--proposals',type=Path)
    parser.add_argument('--scale-x',type=float,default=1)
    parser.add_argument('--scale-y',type=float,default=1)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SAMPLE_SHA:raise SystemExit('Unsupported DLL sample hash')
    kernel=C.WinDLL('kernel32',use_last_error=True)
    kernel.SetDllDirectoryW.argtypes=[C.c_wchar_p]
    if not kernel.SetDllDirectoryW(str(binary.parent)):raise C.WinError(C.get_last_error())
    dll=C.WinDLL(str(binary))
    if args.mode=='proposals' and not args.trace:parser.error('--trace is required for proposals')
    if args.mode=='curve' and not args.proposals:parser.error('--proposals is required for curve')
    if args.mode=='quad':report=quad_probe(dll,args.direction)
    elif args.mode=='proposals':report=proposal_probe(dll,args.trace,args.level,args.direction)
    elif args.mode=='curve':report=curve_probe(dll,args.proposals,args.scale_x,args.scale_y)
    elif args.mode=='pipeline':report=pipeline_probe(dll)
    elif args.mode=='geometry':
        if not args.geometry:parser.error('--geometry is required')
        report=geometry_probe(dll,args.geometry)
    else:report=ordering_probe(dll)
    report['source_sha256']=SAMPLE_SHA
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps({'output':str(args.output),'direction':report['direction'],'rows':len(report.get('quads',report.get('rows',report.get('records',[]))))}))
