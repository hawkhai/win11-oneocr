"""Observe fixed Photos detector regions in one disposable offline process.

The vtable/layout addresses are version locked and recovered with Ghidra.
Only this subprocess's vtable slot is temporarily redirected; disk assets
and model/pipeline inputs are unchanged. Do not import into application code.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
import struct
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
SHA='0a5e99041af902d6825f0af04ab25e62d2e2378ace69d10175f0dcc22816aae0'
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('image',type=Path);p.add_argument('--slot',type=int,default=4);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--max-lines',type=int,default=1000);p.add_argument('--resize',default='1152x768');p.add_argument('--observe-choice',action='store_true');a=p.parse_args()
    binary=ROOT/'bin/photos-2026.11090.22001.0'
    if hashlib.sha256((binary/'oneocr.dll').read_bytes()).hexdigest()!=SHA:raise ValueError('Unsupported DLL hash')
    directory=os.add_dll_directory(str(binary));kernel=C.WinDLL('kernel32',use_last_error=True)
    kernel.SetDllDirectoryW.argtypes=[C.c_wchar_p];kernel.SetDllDirectoryW(str(binary))
    wrapper=C.CDLL(str(binary/'oneocr_wrapper.dll'));wrapper.initModel.argtypes=[C.c_wchar_p];wrapper.initModel.restype=C.c_int
    if wrapper.initModel(str(binary)):raise RuntimeError('Cannot initialize original wrapper')
    dll=C.WinDLL(str(binary/'oneocr.dll'));slot=C.c_void_p.from_address(dll._handle+0x25738e0+a.slot*8);previous=slot.value
    signature=C.WINFUNCTYPE(C.c_uint64,*([C.c_uint64]*8));original=signature(previous);observations=[];errors=[]
    def vector(address,stride):
        begin=C.c_uint64.from_address(address).value;end=C.c_uint64.from_address(address+8).value
        if end<begin or (end-begin)%stride or end-begin>16000000:raise ValueError('Geometry vector bounds')
        return begin,(end-begin)//stride
    def polygons_at(address):
        begin,count=vector(address,16);polygons=[]
        for i in range(count):
            region=C.c_uint64.from_address(begin+i*16).value;upper,nu=vector(region+0x40,8);lower,nl=vector(region+0x58,8)
            polygons.append({'score':C.c_float.from_address(region+0x14).value,'area':C.c_float.from_address(region+0x18).value,'id':C.c_int32.from_address(region+0x20).value,
                'ambiguous':bool(C.c_uint8.from_address(region+1).value),'min_rectangle':bool(C.c_uint8.from_address(region+2).value),'horizontal':bool(C.c_uint8.from_address(region+0x24).value),
                'level':C.c_int32.from_address(region+0x28).value,'upper':list((C.c_float*(nu*2)).from_address(upper)),
                'lower':list((C.c_float*(nl*2)).from_address(lower))})
        return polygons
    def hook(*parameters):
        result=original(*parameters)
        try:
            begin,count=vector(parameters[1],16);polygons=[]
            for i in range(count):
                region=C.c_uint64.from_address(begin+i*16).value
                upper,nu=vector(region+0x40,8);lower,nl=vector(region+0x58,8)
                polygons.append({'score':C.c_float.from_address(region+0x14).value,'horizontal':bool(C.c_uint8.from_address(region+0x24).value),
                    'level':C.c_int32.from_address(region+0x28).value,'upper':list((C.c_float*(nu*2)).from_address(upper)),
                    'lower':list((C.c_float*(nl*2)).from_address(lower))})
            observations.append({'polygons':polygons})
        except BaseException as error:errors.append(repr(error))
        return result
    callback=signature(hook);protection=C.c_ulong();kernel.VirtualProtect.argtypes=[C.c_void_p,C.c_size_t,C.c_ulong,C.POINTER(C.c_ulong)]
    if not kernel.VirtualProtect(C.addressof(slot),8,4,C.byref(protection)):raise C.WinError(C.get_last_error())
    slot.value=C.cast(callback,C.c_void_p).value;ignored=C.c_ulong();kernel.VirtualProtect(C.addressof(slot),8,protection.value,C.byref(ignored))
    recognition=[];recognition_slot=C.c_void_p.from_address(dll._handle+0x2572218);recognition_previous=recognition_slot.value
    recognition_original=signature(recognition_previous)
    def recognition_hook(*parameters):
        try:
            begin,count=vector(parameters[2],8);addresses=list((C.c_uint64*count).from_address(begin));records=[]
            for line in addresses:
                region=C.c_uint64.from_address(line+0x148).value;upper,nu=vector(region+0x40,8);lower,nl=vector(region+0x58,8)
                records.append({'box':list((C.c_float*8).from_address(line+0xf0)),'horizontal':bool(C.c_uint8.from_address(line+0x15a).value),
                                'ambiguous':bool(C.c_uint8.from_address(region+1).value),'min_rectangle':bool(C.c_uint8.from_address(region+2).value),
                                'polygon':list((C.c_float*(nu*2)).from_address(upper))+list((C.c_float*(nl*2)).from_address(lower))})
            # Sort only a private array of pointers. The live pipeline is unchanged.
            array=(C.c_uint64*count)(*addresses);array_begin=C.addressof(array);array_end=array_begin+count*8
            sort=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_uint64,C.c_void_p)(dll._handle+0x485630)
            sort(array_begin,array_end,count,dll._handle+0x490ca0)
            values=(C.c_uint64*3)(array_begin,array_end,array_end);callback_object=C.create_string_buffer(64);own=C.addressof(callback_object)
            struct.pack_into('<2Q',callback_object,0,dll._handle+0x2572258,C.addressof(values));struct.pack_into('<Q',callback_object,56,own)
            grouped=(C.c_uint64*3)();group=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0x4a4c90);group(grouped,values,callback_object)
            gb,gn=vector(C.addressof(grouped),24);groups=[];by_address={value:index for index,value in enumerate(addresses)}
            for i in range(gn):
                ib,inum=vector(gb+i*24,8);indices=list((C.c_uint64*inum).from_address(ib));groups.append([by_address[array[index]] for index in indices])
            possible=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_int,C.c_double)(dll._handle+0x5841e0)
            intersect=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_void_p)(dll._handle+0x584560);edges=[]
            for i,first in enumerate(addresses):
                for j in range(i+1,len(addresses)):
                    if records[i]['horizontal']==records[j]['horizontal']:continue
                    x=C.c_uint64.from_address(first+0x148).value;y=C.c_uint64.from_address(addresses[j]+0x148).value
                    if possible(x,y,1,0.) and intersect(x,y)>0:edges.append([i,j])
            recognition.append({'regions':records,'sorted': [by_address[x] for x in array],'groups':groups,'opposite_axis_intersections':edges})
        except BaseException as error:errors.append('Recognition geometry: '+repr(error))
        return recognition_original(*parameters)
    recognition_callback=signature(recognition_hook);recognition_protection=C.c_ulong()
    kernel.VirtualProtect(C.addressof(recognition_slot),8,4,C.byref(recognition_protection));recognition_slot.value=C.cast(recognition_callback,C.c_void_p).value
    kernel.VirtualProtect(C.addressof(recognition_slot),8,recognition_protection.value,C.byref(ignored))
    nms=[];nms_hooks=[]
    def install_nms(rva):
        entry=C.c_void_p.from_address(dll._handle+rva);old=entry.value;function=signature(old)
        def observe(*parameters):
            before=polygons_at(parameters[2]);result=function(*parameters)
            try:
                begin,count=vector(parameters[1],4)
                record={'vtable_entry_rva':hex(rva),'before':before,'after':polygons_at(parameters[2]),'selected':list((C.c_int32*count).from_address(begin)),
                        'ordinary_thresholds':list((C.c_float*2).from_address(parameters[0]+0x74)),
                        'ambiguity_enabled':C.c_uint8.from_address(parameters[0]+0x218).value,'ambiguity_thresholds':list((C.c_float*4).from_address(parameters[0]+0x21c))}
                rb,rn=vector(parameters[2],16);objects=[C.c_uint64.from_address(rb+i*16).value for i in range(rn)];pairs=[]
                intersection=C.CFUNCTYPE(C.c_float,C.c_void_p,C.c_void_p)(dll._handle+0x584560)
                possible=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_int,C.c_double)(dll._handle+0x5841e0)
                covered=C.CFUNCTYPE(C.c_uint8,C.c_void_p,C.c_void_p,C.c_float,C.c_void_p)(dll._handle+0x5861a0)
                for i,first in enumerate(objects):
                    for j,second in enumerate(objects):
                        if i==j:continue
                        area=float(intersection(first,second))
                        head=C.create_string_buffer(48);node=C.create_string_buffer(48);hp=C.addressof(head);np=C.addressof(node)
                        ids=[C.c_int32.from_address(x+0x20).value for x in [first,second]]
                        struct.pack_into('<3Q',head,0,np,np,np);struct.pack_into('<H',head,24,0x101)
                        struct.pack_into('<3Q',node,0,hp,hp,hp);struct.pack_into('<iif',node,28,min(ids),max(ids),area);cache=(C.c_uint64*2)(hp,1)
                        pairs.append({'first':i,'second':j,'intersection':area,'covered':bool(covered(first,second,.4,cache)),
                                      'primary_possible':bool(possible(first,second,0,float(C.c_float(.3).value)))})
                record['pairs']=pairs;nms.append(record)
            except BaseException as error:errors.append('NMS: '+repr(error))
            return result
        cb=signature(observe);protect=C.c_ulong();kernel.VirtualProtect(C.addressof(entry),8,4,C.byref(protect));entry.value=C.cast(cb,C.c_void_p).value
        kernel.VirtualProtect(C.addressof(entry),8,protect.value,C.byref(ignored));nms_hooks.append((entry,old,protect.value,cb))
    for rva in [0x25739b0,0x2573cb0,0x2573ac0,0x2573ef8,0x2573f90]:install_nms(rva)
    choices=[];restore_choice=None
    if a.observe_choice:
        sys.path.insert(0,str(Path(__file__).resolve().parent));import photos_choice_observer
        choices,restore_choice=photos_choice_observer.install(dll,kernel,vector,errors)
    crt=C.CDLL('msvcrt');crt.malloc.argtypes=[C.c_size_t];crt.malloc.restype=C.c_void_p;crt.free.argtypes=[C.c_void_p]
    alloc_type=C.CFUNCTYPE(C.c_void_p,C.c_size_t);alloc=alloc_type(lambda n:crt.malloc(n));output=C.c_void_p()
    wrapper.ocrImageEx.argtypes=[C.c_wchar_p,C.POINTER(C.c_void_p),alloc_type,C.c_int,C.c_int,C.c_int];wrapper.ocrImageEx.restype=C.c_int
    try:
        width,height=map(int,a.resize.split('x'));status=wrapper.ocrImageEx(str(a.image.resolve()),C.byref(output),alloc,a.max_lines,width,height)
        if status:raise RuntimeError('Original wrapper failed: '+str(status))
        result=json.loads(C.string_at(output).decode('utf8'))
    finally:
        kernel.VirtualProtect(C.addressof(slot),8,4,C.byref(ignored));slot.value=previous;kernel.VirtualProtect(C.addressof(slot),8,protection.value,C.byref(ignored))
        kernel.VirtualProtect(C.addressof(recognition_slot),8,4,C.byref(ignored));recognition_slot.value=recognition_previous;kernel.VirtualProtect(C.addressof(recognition_slot),8,recognition_protection.value,C.byref(ignored))
        for entry,old,protect,cb in nms_hooks:
            kernel.VirtualProtect(C.addressof(entry),8,4,C.byref(ignored));entry.value=old;kernel.VirtualProtect(C.addressof(entry),8,protect,C.byref(ignored))
        if restore_choice:restore_choice()
        if output.value:crt.free(output)
        wrapper.releaseModel()
    if errors or not observations:raise RuntimeError('Geometry observation failed: '+repr(errors))
    bindings={name:hashlib.sha256((binary/name).read_bytes()).hexdigest() for name in ['oneocr.dll','oneocr.onemodel','oneocr_wrapper.dll','onnxruntime.dll']}
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps({'dll_sha256':SHA,'bindings':bindings,'image_sha256':hashlib.sha256(a.image.read_bytes()).hexdigest(),
        'options':{'max_lines':a.max_lines,'resize_width':width,'resize_height':height},'vtable_rva':'0x25738e0','slot':a.slot,'observations':observations,'recognition':recognition,'nms':nms,'choices':choices,'result':result},ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print({'batches':len(observations),'counts':[len(x['polygons']) for x in observations]})

if __name__=='__main__':main()
