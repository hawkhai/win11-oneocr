"""Offline OneOCR evidence collection. No downloads or automatic installations.

The extraction approach follows bropines/oneocr-onnx-python (MIT), pinned
at 75cc12666503425ffd6ea0cb052c0bcaaff21058. This implementation identifies
graphs by their public tensor contracts, never their serialized byte sizes.
Run with the project-local research Python; see bootstrap-research.ps1.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
CASE = ROOT / 'work/oneocr-portable'
MODEL_NAMES = {32632:'cjk',548:'latin',415:'cyrillic',221:'arabic',237:'devanagari',244:'greek',199:'thai',201:'hebrew',179:'tamil'}

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding='utf-8')

def inventory():
    import pefile
    evidence = {'source_commit': subprocess.check_output(['git','rev-parse','HEAD'], cwd=ROOT, text=True).strip(), 'assets':[]}
    for path in sorted((ROOT/'bin').glob('*')):
        if path.name not in ('oneocr.dll','onnxruntime.dll','oneocr.onemodel'):
            continue
        entry = {'path':str(path), 'sha256':sha(path), 'size':path.stat().st_size}
        if path.suffix == '.dll':
            pe = pefile.PE(str(path))
            entry.update(machine=hex(pe.FILE_HEADER.Machine), image_base=hex(pe.OPTIONAL_HEADER.ImageBase), imports={}, exports=[])
            for dll in getattr(pe, 'DIRECTORY_ENTRY_IMPORT', []):
                entry['imports'][dll.dll.decode()] = [x.name.decode() if x.name else f'ordinal:{x.ordinal}' for x in dll.imports]
            entry['exports'] = [{'name':x.name.decode() if x.name else None,'rva':hex(x.address)} for x in pe.DIRECTORY_ENTRY_EXPORT.symbols]
            entry['sections'] = [{'name':x.Name.rstrip(b'\0').decode(), 'entropy':x.get_entropy()} for x in pe.sections]
            entry['debug'] = [pe.get_data(x.struct.AddressOfRawData,x.struct.SizeOfData).hex() for x in getattr(pe,'DIRECTORY_ENTRY_DEBUG',[]) if x.struct.Type == 2]
        evidence['assets'].append(entry)
    write_json(CASE/'evidence/inventory.json', evidence)
    print('Inventory:', [(x['path'],x['sha256']) for x in evidence['assets']])

def capture_worker(args):
    if os.name != 'nt' or struct.calcsize('P') != 8:
        raise RuntimeError('Extraction requires a 64-bit Windows research process')
    import onnx
    import numpy as np
    binary = Path(args.bin).resolve()
    if args.trace_geometry and sha(binary/'oneocr.dll')!='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b':
        raise ValueError('Geometry layouts require the fixed 9.0.103 DLL sample')
    trace_name=args.trace_label or (Path(args.trace).stem if args.trace else '')
    destination = Path(args.models).resolve()
    destination.mkdir(parents=True, exist_ok=True)
    directory = os.add_dll_directory(str(binary))
    ort_dll = C.WinDLL(str(binary/'onnxruntime.dll'))
    ptr = C.c_void_p
    get_api_t = C.WINFUNCTYPE(ptr, C.c_uint32)
    version_t = C.WINFUNCTYPE(C.c_char_p)
    class ApiBase(C.Structure):
        _fields_ = [('GetApi',get_api_t), ('GetVersionString',version_t)]
    ort_dll.OrtGetApiBase.restype = C.POINTER(ApiBase)
    api_base = ort_dll.OrtGetApiBase().contents
    api = api_base.GetApi(23)
    if not api:
        raise RuntimeError('ORT C API v23 unavailable')
    table = C.cast(api,C.POINTER(ptr))
    def fn(index, restype, *types):
        return C.WINFUNCTYPE(restype,*types)(table[index])
    get_message = fn(2,C.c_char_p,ptr)
    tensor_data = fn(51,ptr,ptr,C.POINTER(ptr))
    shape_info = fn(65,ptr,ptr,C.POINTER(ptr))
    dim_count = fn(61,ptr,ptr,C.POINTER(C.c_size_t))
    dimensions = fn(62,ptr,ptr,C.POINTER(C.c_int64),C.c_size_t)
    release_shape = fn(99,None,ptr)
    records, sessions, callbacks, errors = {}, {}, [], []
    forcing = {'script':None,'offset':0,'vocab_size':None,'chunk':0,'limit':100}
    traces = []
    def tensor_array(value, dtype):
        info, count, data = ptr(),C.c_size_t(),ptr()
        status = shape_info(value,C.byref(info))
        if status:
            raise RuntimeError(get_message(status).decode())
        try:
            dim_count(info,C.byref(count))
            dims = (C.c_int64*count.value)()
            dimensions(info,dims,count.value)
        finally:
            release_shape(info)
        tensor_data(value,C.byref(data))
        c_type = {1:C.c_float,6:C.c_int32,7:C.c_int64,9:C.c_bool}[dtype]
        shape = tuple(dims)
        size = int(np.prod(shape))
        return np.ctypeslib.as_array(C.cast(data,C.POINTER(c_type)),shape=(size,)).reshape(shape).copy()
    def save(data, length, handle):
        blob = C.string_at(data,length)
        digest = hashlib.sha256(blob).hexdigest()
        if digest in records:
            sessions[handle] = records[digest]
            return
        model = onnx.load_model_from_string(blob)
        onnx.checker.check_model(model)
        def value_info(v):
            t = v.type.tensor_type
            return {'name':v.name,'dtype':t.elem_type,'shape':[d.dim_value if d.HasField('dim_value') else d.dim_param for d in t.shape.dim]}
        ins, outs = [value_info(x) for x in model.graph.input], [value_info(x) for x in model.graph.output]
        names = [x['name'] for x in outs]
        vocab_size = None
        if 'script_id_score' in names:
            role, rel = 'classifier', 'classifier/script_classifier.onnx'
        elif any('fpn' in n for n in names):
            role, rel = 'detector', 'detector/text_detector.onnx'
        elif 'logsoftmax' in names:
            shape = next(x['shape'] for x in outs if x['name']=='logsoftmax')
            vocab_size = shape[-1]
            if vocab_size not in MODEL_NAMES:
                raise ValueError(f'Unknown recognizer contract: {shape}')
            role, rel = 'recognizer', f'recognizers/recognizer_{MODEL_NAMES[vocab_size]}.onnx'
        else:
            role, rel = 'auxiliary', f'auxiliary/{digest}.onnx'
        output = destination/rel
        if output.exists() and sha(output) != digest:
            raise ValueError(f'Conflicting model role: {rel}')
        output.parent.mkdir(parents=True,exist_ok=True)
        output.write_bytes(blob)
        record = {'file':rel,'sha256':digest,'size':length,'role':role,'vocab_size':vocab_size,'inputs':ins,'outputs':outs,'opsets':{x.domain:x.version for x in model.opset_import},'operators':sorted({f'{x.domain}::{x.op_type}' for x in model.graph.node})}
        records[digest] = record
        sessions[handle] = record
        print('Captured',rel, length, flush=True)
    signatures = {
        8:C.WINFUNCTYPE(ptr,ptr,ptr,C.c_size_t,ptr,C.POINTER(ptr)),
        151:C.WINFUNCTYPE(ptr,ptr,ptr,C.c_size_t,ptr,ptr,C.POINTER(ptr)),
        9:C.WINFUNCTYPE(ptr,ptr,ptr,C.POINTER(C.c_char_p),C.POINTER(ptr),C.c_size_t,C.POINTER(C.c_char_p),C.c_size_t,C.POINTER(ptr)),
    }
    originals = {i:signatures[i](table[i]) for i in signatures}
    def session_hook(slot):
        def hook(*parameters):
            call_parameters = list(parameters)
            augmented = None
            if args.probe_aux:
                model = onnx.load_model_from_string(C.string_at(parameters[1],parameters[2]))
                if len(model.graph.input)==1 and [d.dim_value for d in model.graph.input[0].type.tensor_type.shape.dim]==[1,21,1,1]:
                    model.graph.output.extend([onnx.helper.make_tensor_value_info('oneocr_feature',onnx.TensorProto.FLOAT,[1,50]),onnx.helper.make_tensor_value_info('24',onnx.TensorProto.FLOAT,[1,21])])
                    blob = model.SerializeToString()
                    augmented = C.create_string_buffer(blob)
                    call_parameters[1],call_parameters[2] = C.cast(augmented,ptr),len(blob)
            status = originals[slot](*call_parameters)
            if not status:
                try:
                    save(parameters[1],parameters[2],parameters[-1][0])
                except BaseException as error:
                    errors.append(repr(error))
                    print('CAPTURE ERROR',repr(error),flush=True)
            return status
        return signatures[slot](hook)
    def run_hook(session, options, in_names, inputs, n_in, out_names, n_out, outputs):
        status = originals[9](session,options,in_names,inputs,n_in,out_names,n_out,outputs)
        if not status and args.trace and session in sessions:
            try:
                record = sessions[session]
                trace_dir = CASE/'traces'/trace_name
                trace_dir.mkdir(parents=True,exist_ok=True)
                index = len(traces)
                entry = {'model':record['file'],'inputs':{},'outputs':{}}
                # Reserve the unique index before numpy.save releases the GIL;
                # native pipeline threads may enter another callback meanwhile.
                traces.append(entry)
                for direction,names,values,num in [('inputs',in_names,inputs,n_in),('outputs',out_names,outputs,n_out)]:
                    for i in range(num):
                        name = names[i].decode()
                        dtype = next(x['dtype'] for x in record[direction] if x['name']==name)
                        path = trace_dir/f'{index:03d}_{direction}_{name}.npy'
                        array = tensor_array(values[i],dtype)
                        np.save(path,array,allow_pickle=False)
                        entry[direction][name] = {'file':path.name,'shape':list(array.shape),'min':float(array.min()),'max':float(array.max())}
            except BaseException as error:
                errors.append(f'Trace: {error!r}')
        if status or forcing['script'] is None:
            return status
        try:
            record = sessions.get(session)
            if not record:
                raise ValueError('Uncaptured session')
            if record['role']=='classifier':
                index = [out_names[i].decode() for i in range(n_out)].index('script_id_score')
                data = ptr()
                tensor_data(outputs[index],C.byref(data))
                array = np.ctypeslib.as_array(C.cast(data,C.POINTER(C.c_float)),shape=(10,))
                array.fill(-999)
                array[forcing['script']] = 10
            elif record['role']=='recognizer':
                info, data, count = ptr(),ptr(),C.c_size_t()
                shape_info(outputs[0],C.byref(info))
                try:
                    dim_count(info,C.byref(count))
                    dims = (C.c_int64*count.value)()
                    dimensions(info,dims,count.value)
                finally:
                    release_shape(info)
                t,b,v = dims
                if b != 1 or v != record['vocab_size']:
                    raise ValueError(f'Unexpected recognizer shape: {list(dims)}')
                tensor_data(outputs[0],C.byref(data))
                array = np.ctypeslib.as_array(C.cast(data,C.POINTER(C.c_float)),shape=(t,1,v))
                array.fill(-999)
                array[:,:,v-1] = 0
                # Explicit space tokens delimit vocabulary entries, including
                # multi-codepoint entries. Adjacent CJK/Latin entries otherwise
                # acquire word-boundary spaces in GetOcrLineContent.
                chunk = min((t-2)//2,forcing['limit'],v-2-forcing['offset'])
                for i in range(chunk):
                    pos = 2*i+1
                    array[pos,0,v-1] = -999
                    array[pos,0,forcing['offset']+1+i] = 0
                    if i+1 < chunk:
                        array[pos+1,0,v-1] = -999
                        array[pos+1,0,0] = 0
                forcing.update(vocab_size=v,chunk=chunk)
        except BaseException as error:
            errors.append(repr(error))
        return status
    callbacks.extend([session_hook(8),session_hook(151),signatures[9](run_hook)])
    kernel = C.WinDLL('kernel32',use_last_error=True)
    kernel.VirtualProtect.argtypes = [ptr,C.c_size_t,C.c_ulong,C.POINTER(C.c_ulong)]
    saved = {i:table[i] for i in signatures}
    old = C.c_ulong()
    def patch(values):
        if not kernel.VirtualProtect(api,1600,0x04,C.byref(old)):
            raise C.WinError(C.get_last_error())
        try:
            for index,value in values.items():
                table[index] = value
        finally:
            ignored = C.c_ulong()
            kernel.VirtualProtect(api,1600,old.value,C.byref(ignored))
    patch({8:C.cast(callbacks[0],ptr).value,151:C.cast(callbacks[1],ptr).value,9:C.cast(callbacks[2],ptr).value})
    dll = C.WinDLL(str(binary/'oneocr.dll'))
    geometry_restore=[];geometry=[];lattices=[];recognition_records=[];native_model_metadata={};quad_normalizations=[];analyzed_lines=[];detector_selection=[]
    if args.trace_geometry:
        signature=C.WINFUNCTYPE(C.c_uint64,*([C.c_uint64]*8))
        address=dll._handle+0x256ba60+3*8
        slot=C.cast(address,C.POINTER(ptr));previous=slot[0];original=signature(previous)
        def read_vector(address,stride):
            begin,end=C.c_uint64.from_address(address).value,C.c_uint64.from_address(address+8).value
            if end<begin or (end-begin)%stride or end-begin>16000000:raise ValueError('Native geometry vector bounds')
            return begin,(end-begin)//stride
        def geometry_hook(*parameters):
            result=original(*parameters)
            try:
                begin,count=read_vector(parameters[1],16);polygons=[]
                for i in range(count):
                    polygon=C.c_uint64.from_address(begin+i*16).value
                    upper,n_upper=read_vector(polygon+0x40,8);lower,n_lower=read_vector(polygon+0x58,8)
                    polygons.append({'score':C.c_float.from_address(polygon+0x14).value,'horizontal':bool(C.c_uint8.from_address(polygon+0x24).value),'level':C.c_int32.from_address(polygon+0x28).value,'curved':bool(C.c_uint8.from_address(polygon).value),'ambiguous':bool(C.c_uint8.from_address(polygon+1).value),'min_rectangle':bool(C.c_uint8.from_address(polygon+0x2c).value),'upper':list((C.c_float*(n_upper*2)).from_address(upper)),'lower':list((C.c_float*(n_lower*2)).from_address(lower))})
                geometry.append({'polygons':polygons,'count':count})
            except BaseException as error:errors.append('Geometry capture: '+repr(error))
            return result
        callback=signature(geometry_hook);callbacks.append(callback)
        protection=C.c_ulong()
        if not kernel.VirtualProtect(address,8,0x04,C.byref(protection)):raise C.WinError(C.get_last_error())
        slot[0]=C.cast(callback,ptr).value
        ignored=C.c_ulong();kernel.VirtualProtect(address,8,protection.value,C.byref(ignored))
        geometry_restore.append((address,slot,previous))
        width_signature=C.WINFUNCTYPE(None,ptr,C.c_float,C.c_float,ptr,ptr,ptr,ptr)
        width_address=dll._handle+0x256b400+8;width_slot=C.cast(width_address,C.POINTER(ptr));width_previous=width_slot[0];width_original=width_signature(width_previous)
        def width_hook(this,width,height,options,output_width,output_height,output_scale):
            width_original(this,width,height,options,output_width,output_height,output_scale)
            try:quad_normalizations.append({'crop_width':width,'crop_height':height,'normalized_width':C.c_int32.from_address(output_width).value,'normalized_height':C.c_int32.from_address(output_height).value,'scale':C.c_double.from_address(output_scale).value})
            except BaseException as error:errors.append('Quad normalization capture: '+repr(error))
        width_callback=width_signature(width_hook);callbacks.append(width_callback);width_protection=C.c_ulong()
        if not kernel.VirtualProtect(width_address,8,0x04,C.byref(width_protection)):raise C.WinError(C.get_last_error())
        width_slot[0]=C.cast(width_callback,ptr).value
        width_ignored=C.c_ulong();kernel.VirtualProtect(width_address,8,width_protection.value,C.byref(width_ignored));geometry_restore.append((width_address,width_slot,width_previous))
        analyzer_signature=C.WINFUNCTYPE(C.c_uint64,*([C.c_uint64]*8))
        analyzer_address=dll._handle+0x2568138+3*8;analyzer_slot=C.cast(analyzer_address,C.POINTER(ptr));analyzer_previous=analyzer_slot[0];analyzer_original=analyzer_signature(analyzer_previous)
        def analyzer_hook(*parameters):
            result=analyzer_original(*parameters)
            try:
                begin,count=read_vector(parameters[1],8)
                batch=[]
                for i in range(count):
                    line=C.c_uint64.from_address(begin+i*8).value
                    batch.append({'index':i,'box':list((C.c_float*8).from_address(line+0xc0)),'horizontal':bool(C.c_uint8.from_address(line+299).value),'flip':C.c_int32.from_address(line+0x134).value,'flip_ambiguous':bool(C.c_uint8.from_address(line+300).value),'script':C.c_int32.from_address(line+0x148).value,'style':C.c_int32.from_address(line+0x13c).value,'style_confidence':C.c_float.from_address(line+0x140).value,'layout':C.c_int32.from_address(line+0x154).value})
                analyzed_lines.append(batch)
            except BaseException as error:errors.append('Analyzer capture: '+repr(error))
            return result
        analyzer_callback=analyzer_signature(analyzer_hook);callbacks.append(analyzer_callback);analyzer_protection=C.c_ulong()
        if not kernel.VirtualProtect(analyzer_address,8,0x04,C.byref(analyzer_protection)):raise C.WinError(C.get_last_error())
        analyzer_slot[0]=C.cast(analyzer_callback,ptr).value
        analyzer_ignored=C.c_ulong();kernel.VirtualProtect(analyzer_address,8,analyzer_protection.value,C.byref(analyzer_ignored));geometry_restore.append((analyzer_address,analyzer_slot,analyzer_previous))
        def read_native_string(address):
            length=C.c_uint64.from_address(address+16).value
            capacity=C.c_uint64.from_address(address+24).value
            if length>1000000 or length>capacity:raise ValueError('Native string bounds')
            data=C.c_uint64.from_address(address).value if capacity>15 else address
            return C.string_at(data,length).decode('utf8')
        def words_from_vector(address,stride):
            begin,count=read_vector(address,stride);words=[]
            for i in range(count):
                word=begin+i*stride
                candidate_vector=word+(8 if stride==32 else 16)
                candidates,n=read_vector(candidate_vector,72)
                text=read_native_string(candidates) if n else ''
                words.append({'start':C.c_int32.from_address(word+(0 if stride==32 else 8)).value,
                              'end':C.c_int32.from_address(word+(4 if stride==32 else 12)).value,'text':text,
                              'pixel_start':C.c_int32.from_address(word).value,'pixel_end':C.c_int32.from_address(word+4).value,
                              'candidate_confidence':C.c_float.from_address(candidates+0x20).value if n else None,
                              'candidate_language_score':C.c_float.from_address(candidates+0x24).value if n else None})
            return words
        def install_observation(vtable_rva,index,observe,before=None):
            address=dll._handle+vtable_rva+index*8
            slot=C.cast(address,C.POINTER(ptr));previous=slot[0];original=signature(previous)
            def hook(*parameters):
                if before:
                    try:before(parameters)
                    except BaseException as error:errors.append('Preprocess capture: '+repr(error))
                result=original(*parameters)
                try:observe(parameters)
                except BaseException as error:errors.append('Postprocess capture: '+repr(error))
                return result
            callback=signature(hook);callbacks.append(callback)
            protection=C.c_ulong()
            if not kernel.VirtualProtect(address,8,0x04,C.byref(protection)):raise C.WinError(C.get_last_error())
            slot[0]=C.cast(callback,ptr).value
            ignored=C.c_ulong();kernel.VirtualProtect(address,8,protection.value,C.byref(ignored))
            geometry_restore.append((address,slot,previous))
        def detector_selection_snapshot(parameters,phase):
            begin,count=read_vector(parameters[2],16);polygons=[]
            for i in range(count):
                polygon=C.c_uint64.from_address(begin+i*16).value
                upper,n_upper=read_vector(polygon+0x40,8);lower,n_lower=read_vector(polygon+0x58,8)
                polygons.append({'score':C.c_float.from_address(polygon+0x14).value,'horizontal':bool(C.c_uint8.from_address(polygon+0x24).value),'curved':bool(C.c_uint8.from_address(polygon).value),'level':C.c_int32.from_address(polygon+0x28).value,'id':C.c_int32.from_address(polygon+0x20).value,'upper':list((C.c_float*(n_upper*2)).from_address(upper)),'lower':list((C.c_float*(n_lower*2)).from_address(lower))})
            record={'phase':phase,'polygons':polygons,'nms_threshold':C.c_float.from_address(parameters[0]+0x74).value,'coverage_threshold':C.c_float.from_address(parameters[0]+0x78).value}
            record['vtable_rva']=hex(C.c_uint64.from_address(parameters[0]).value-dll._handle)
            record['ambiguity']=bool(C.c_uint8.from_address(parameters[0]+0x238).value)
            record['ambiguity_thresholds']=list((C.c_float*4).from_address(parameters[0]+0x23c))
            head=C.c_uint64.from_address(parameters[0]+0x80).value
            record['level_thresholds']={}
            def visit_threshold(node):
                if C.c_uint8.from_address(node+25).value:return
                visit_threshold(C.c_uint64.from_address(node).value)
                record['level_thresholds'][C.c_int32.from_address(node+28).value]=C.c_float.from_address(node+32).value
                visit_threshold(C.c_uint64.from_address(node+16).value)
            visit_threshold(C.c_uint64.from_address(head+8).value)
            if phase=='after':
                begin,count=read_vector(parameters[1],4);record['selected']=list((C.c_int32*count).from_address(begin))
            detector_selection.append(record)
        for nms_table in (0x256be60,0x256c990,0x256c188,0x256c1f8,0x256cbe0,0x256d050):
            install_observation(nms_table,0,lambda parameters:detector_selection_snapshot(parameters,'after'),lambda parameters:detector_selection_snapshot(parameters,'before'))
        def observe_lattice(parameters):
            begin,count=read_vector(parameters[2],4)
            lattices.append({'phones':list((C.c_int32*count).from_address(begin)),
                             'words':words_from_vector(parameters[3],32)})
        def observe_recognition(parameters):
            begin,count=read_vector(parameters[1],176)
            recognition_records.append([{'text':read_native_string(begin+i*176),
                                          'words':words_from_vector(begin+i*176+32,40)} for i in range(count)])
        install_observation(0x256a180,2,observe_lattice)
        install_observation(0x2569388,5,observe_recognition)
        def observe_single(parameters):
            address=parameters[1]
            begin,count=read_vector(address+0x98,8)
            recognition_records.append([{'text':read_native_string(address),
                                          'box':list((C.c_float*8).from_address(parameters[2]+0xc0)),
                                          'language_score':C.c_double.from_address(address+0x38).value,
                                          'rejected':bool(C.c_uint8.from_address(address+0x50).value),
                                          'words':words_from_vector(address+32,40),
                                          'blank_intervals':[list((C.c_int32*2).from_address(begin+i*8)) for i in range(count)]}])
            head=C.c_uint64.from_address(parameters[0]+0x3f8).value
            queue=[C.c_uint64.from_address(head+8).value];visited=set()
            while queue:
                node=queue.pop()
                if node in visited or C.c_uint8.from_address(node+0x19).value:continue
                visited.add(node)
                if len(visited)>100:raise ValueError('Character model tree bounds')
                queue.extend([C.c_uint64.from_address(node).value,C.c_uint64.from_address(node+16).value])
                model=C.c_uint64.from_address(node+0x80).value
                if model:
                    name=read_native_string(node+0x20)
                    native_model_metadata[name]={'words_u32':list((C.c_uint32*52).from_address(model))}
        install_observation(0x2569388,3,observe_single)
        install_observation(0x2569388,4,observe_single)
        def observe_mbr(parameters):
            sentence=C.c_uint64.from_address(parameters[2]+64).value
            record={'kind':'mbr','words':words_from_vector(parameters[3],32),'sentence_words':[]}
            for offset in (0xe0,0xf8):
                begin,count=read_vector(sentence+offset,120)
                for i in range(count):
                    word=begin+i*120;characters,n=read_vector(word+0x28,40)
                    chars=[]
                    for j in range(n):
                        item=characters+j*40;character=C.c_uint64.from_address(item).value
                        chars.append({'text':read_native_string(character+0x30),
                                      'frames':list((C.c_int32*2).from_address(item+8)),
                                      'scores':list((C.c_float*6).from_address(item+16))})
                    record['sentence_words'].append({'list_offset':offset,'text':read_native_string(word),'characters':chars})
            lattices.append(record)
        install_observation(0x25696b8,2,observe_mbr)
        def observe_scope(parameters):
            sentence=C.c_uint64.from_address(parameters[0]+0x28).value
            record={'kind':'sentence','sentence_words':[]}
            for offset in (0xe0,0xf8):
                begin,count=read_vector(sentence+offset,120)
                for i in range(count):
                    word=begin+i*120;characters,n=read_vector(word+0x28,40);chars=[]
                    for j in range(n):
                        item=characters+j*40;character=C.c_uint64.from_address(item).value
                        chars.append({'text':read_native_string(character+0x30),
                                      'frames':list((C.c_int32*2).from_address(item+8)),
                                      'scores':list((C.c_float*4).from_address(item+16))})
                    record['sentence_words'].append({'list_offset':offset,'text':read_native_string(word),'characters':chars})
            lattices.append(record)
        install_observation(0x2569648,2,observe_scope)
    crypto_restore = None
    crypto_records = []
    if args.capture_records:
        import pefile
        pe = pefile.PE(str(binary/'oneocr.dll'))
        symbol = next(symbol for library in pe.DIRECTORY_ENTRY_IMPORT for symbol in library.imports if symbol.name==b'BCryptDecrypt')
        slot_address = dll._handle+symbol.address-pe.OPTIONAL_HEADER.ImageBase
        slot = C.cast(slot_address,C.POINTER(ptr))
        previous = slot[0]
        decrypt_type = C.WINFUNCTYPE(C.c_long,ptr,ptr,C.c_ulong,ptr,ptr,C.c_ulong,ptr,C.c_ulong,C.POINTER(C.c_ulong),C.c_ulong)
        decrypt_original = decrypt_type(previous)
        source_model = (binary/'oneocr.onemodel').read_bytes()
        record_dir = CASE/'decrypted-records'
        record_dir.mkdir(exist_ok=True)
        def decrypt_hook(key,encrypted,length,padding,iv,iv_length,output,capacity,written,flags):
            source_offset = source_model.find(C.string_at(encrypted,min(32,length))) if encrypted and length else -1
            iv_bytes = C.string_at(iv,iv_length) if iv else b''
            status = decrypt_original(key,encrypted,length,padding,iv,iv_length,output,capacity,written,flags)
            if status==0 and output and written:
                try:
                    plain = C.string_at(output,written[0])
                    digest = hashlib.sha256(plain).hexdigest()
                    file = f'{len(crypto_records):04d}-{digest}.bin'
                    (record_dir/file).write_bytes(plain)
                    crypto_records.append({'file':file,'sha256':digest,'size':len(plain),'encrypted_length':length,'source_offset':source_offset,'iv':iv_bytes.hex(),'flags':flags})
                except BaseException as error:
                    errors.append(f'Crypto capture: {error!r}')
            return status
        callback = decrypt_type(decrypt_hook)
        callbacks.append(callback)
        protection = C.c_ulong()
        if not kernel.VirtualProtect(slot_address,8,0x04,C.byref(protection)):
            raise C.WinError(C.get_last_error())
        slot[0] = C.cast(callback,ptr).value
        ignored = C.c_ulong()
        kernel.VirtualProtect(slot_address,8,protection.value,C.byref(ignored))
        crypto_restore = (slot_address,slot,previous)
    def bind(name,*types):
        f = getattr(dll,name)
        f.argtypes, f.restype = list(types), C.c_int64
        return f
    def check(code):
        if code:
            raise RuntimeError(f'OneOCR status {code:#x}')
    create_options = bind('CreateOcrInitOptions',C.POINTER(C.c_int64))
    delay = bind('OcrInitOptionsSetUseModelDelayLoad',C.c_int64,C.c_char)
    create_pipeline = bind('CreateOcrPipeline',C.c_char_p,C.c_char_p,C.c_int64,C.POINTER(C.c_int64))
    options, pipeline = C.c_int64(),C.c_int64()
    try:
        check(create_options(C.byref(options)))
        check(delay(options, b'\0'))
        check(create_pipeline(str(binary/'oneocr.onemodel').encode('utf8'),b'kj)TGtrK>f]b[Piow.gU+nC@s""""""4',options,C.byref(pipeline)))
        if args.vocab:
            extract_vocabs(dll,bind,check,pipeline,forcing,destination)
        if args.trace:
            trace_pipeline(bind,check,pipeline,args.trace,CASE/'traces'/trace_name,args.pixel_decoder,args.trace_max_lines)
        if args.probe_aux:
            create_memory = fn(69,ptr,C.c_int,C.c_int,C.POINTER(ptr))
            create_tensor = fn(49,ptr,ptr,ptr,C.c_size_t,C.POINTER(C.c_int64),C.c_size_t,C.c_int,C.POINTER(ptr))
            release_memory,release_value = fn(94,None,ptr),fn(96,None,ptr)
            memory = ptr()
            status = create_memory(1,0,C.byref(memory))
            if status:
                raise RuntimeError(get_message(status).decode())
            rng = np.random.default_rng(20261102)
            probe_dir = CASE/'auxiliary-probes'
            probe_dir.mkdir(exist_ok=True)
            try:
                for handle,record in sessions.items():
                    if record['inputs'][0]['shape'] != [1,21,1,1]:
                        continue
                    xs,ys,features,normalized = [],[],[],[]
                    for sample in range(64):
                        x = rng.uniform(-1,1,(1,21,1,1)).astype(np.float32)
                        tensor,result = ptr(),(ptr*3)()
                        dims = (C.c_int64*4)(1,21,1,1)
                        status = create_tensor(memory,x.ctypes.data,x.nbytes,dims,4,1,C.byref(tensor))
                        if status:
                            raise RuntimeError(get_message(status).decode())
                        try:
                            status = originals[9](handle,None,(C.c_char_p*1)(b'data'),(ptr*1)(tensor),1,(C.c_char_p*3)(b'softmax',b'oneocr_feature',b'24'),3,result)
                            if status:
                                raise RuntimeError(get_message(status).decode())
                            xs.append(x)
                            ys.append(tensor_array(result[0],1))
                            features.append(tensor_array(result[1],1))
                            normalized.append(tensor_array(result[2],1))
                        finally:
                            for value in result:
                                if value:
                                    release_value(value)
                            release_value(tensor)
                    np.savez(probe_dir/(Path(record['file']).stem+'.npz'),inputs=np.stack(xs),outputs=np.stack(ys),features=np.stack(features),normalized=np.stack(normalized))
                    print('Probed auxiliary',record['file'],flush=True)
            finally:
                release_memory(memory)
    finally:
        if pipeline.value:
            bind('ReleaseOcrPipeline',C.c_int64)(pipeline)
        if options.value:
            bind('ReleaseOcrInitOptions',C.c_int64)(options)
        patch(saved)
        for address,slot,previous in reversed(geometry_restore):
            protection=C.c_ulong()
            if not kernel.VirtualProtect(address,8,0x04,C.byref(protection)):raise C.WinError(C.get_last_error())
            slot[0]=previous
            ignored=C.c_ulong();kernel.VirtualProtect(address,8,protection.value,C.byref(ignored))
        if crypto_restore:
            address,slot,previous = crypto_restore
            protection = C.c_ulong()
            if not kernel.VirtualProtect(address,8,0x04,C.byref(protection)):
                raise C.WinError(C.get_last_error())
            slot[0] = previous
            ignored = C.c_ulong()
            kernel.VirtualProtect(address,8,protection.value,C.byref(ignored))
            write_json(CASE/'decrypted-records/records.json',crypto_records)
    manifest = {'schema_version':1,'source_assets':{x.name:sha(x) for x in binary.iterdir() if x.name in ('oneocr.dll','oneocr.onemodel','onnxruntime.dll')},'ort_version':api_base.GetVersionString().decode(),'models':sorted(records.values(),key=lambda x:x['file']),'errors':errors}
    write_json(destination/'manifest.json',manifest)
    if args.trace:
        write_json(CASE/'traces'/trace_name/'trace.json',traces)
        if args.trace_geometry:
            write_json(CASE/'traces'/trace_name/'geometry.json',geometry)
            write_json(CASE/'traces'/trace_name/'lattices.json',lattices)
            write_json(CASE/'traces'/trace_name/'recognition.json',recognition_records)
            write_json(CASE/'traces'/trace_name/'native-model-metadata.json',native_model_metadata)
            write_json(CASE/'traces'/trace_name/'quad-normalizations.json',quad_normalizations)
            write_json(CASE/'traces'/trace_name/'analyzed-lines.json',analyzed_lines)
            write_json(CASE/'traces'/trace_name/'detector-selection.json',detector_selection)
    if errors:
        raise RuntimeError(f'Extraction errors: {errors}')
    print('Captured models:',len(records),flush=True)

def extract_vocabs(dll,bind,check,pipeline,forcing,destination):
    from PIL import Image,ImageDraw,ImageFont
    image = Image.new('RGBA',(2540,200),'white')
    ImageDraw.Draw(image).text((30,60),'This is a long line for complete vocabulary extraction',fill='black',font=ImageFont.truetype('arial.ttf',60))
    raw = image.tobytes('raw','BGRA')
    data = (C.c_ubyte*len(raw)).from_buffer_copy(raw)
    class ImageStruct(C.Structure):
        _fields_ = [('type',C.c_int32),('width',C.c_int32),('height',C.c_int32),('reserved',C.c_int32),('stride',C.c_int64),('data',C.c_void_p)]
    frame = ImageStruct(3,image.width,image.height,0,image.width*4,C.cast(data,C.c_void_p))
    opts = C.c_int64()
    check(bind('CreateOcrProcessOptions',C.POINTER(C.c_int64))(C.byref(opts)))
    run = bind('RunOcrPipeline',C.c_int64,C.POINTER(ImageStruct),C.c_int64,C.POINTER(C.c_int64))
    get_count = bind('GetOcrLineCount',C.c_int64,C.POINTER(C.c_int64))
    get_line = bind('GetOcrLine',C.c_int64,C.c_int64,C.POINTER(C.c_int64))
    get_content = bind('GetOcrLineContent',C.c_int64,C.POINTER(C.c_char_p))
    vocab_dir = destination/'vocab'
    vocab_dir.mkdir(exist_ok=True)
    try:
        seen = set()
        for script in range(10):
            forcing.update(script=script,offset=0,vocab_size=None,chunk=0,limit=100)
            vocab = None
            while True:
                result = C.c_int64()
                check(run(pipeline,C.byref(frame),opts,C.byref(result)))
                try:
                    v = forcing['vocab_size']
                    if v is None or (forcing['offset']==0 and v in seen):
                        break
                    cached = vocab_dir/f'vocab_{MODEL_NAMES[v]}.json'
                    proof = cached.with_suffix('.provenance.json')
                    if forcing['offset']==0 and cached.exists() and proof.exists():
                        provenance = json.loads(proof.read_text(encoding='utf8'))
                        if provenance['source_sha256']==sha(ROOT/'bin/oneocr.onemodel') and provenance['sha256']==sha(cached):
                            seen.add(v)
                            print('Reused verified vocabulary',MODEL_NAMES[v],flush=True)
                            break
                    if vocab is None:
                        vocab = [None]*v
                        vocab[0],vocab[-1] = ' ',''
                    count, line, text = C.c_int64(),C.c_int64(),C.c_char_p()
                    check(get_count(result,C.byref(count)))
                    if count.value==0:
                        if forcing['chunk'] != 1:
                            forcing['limit'] = 1
                            continue
                        decoded = ''
                    elif count.value != 1:
                        raise ValueError(f'Vocabulary probe needs one line; got {count.value}')
                    else:
                        check(get_line(result,0,C.byref(line)))
                        check(get_content(line,C.byref(text)))
                        decoded = (text.value or b'').decode('utf8',errors='strict')
                    expected = forcing['chunk']
                    chars = [decoded] if expected==1 else [x for x in decoded.split(' ') if x]
                    if len(chars) != expected:
                        # Empty entries and language-specific joining cannot be
                        # split safely. Re-probe each remaining token alone.
                        print(f'Vocabulary singleton fallback script={script} offset={forcing["offset"]}',flush=True)
                        forcing['limit'] = 1
                        continue
                    for i,char in enumerate(chars):
                        vocab[forcing['offset']+1+i] = char
                    forcing['offset'] += expected
                    if forcing['offset'] >= v-2:
                        if any(x is None for x in vocab):
                            raise ValueError('Incomplete vocabulary')
                        lang = MODEL_NAMES[v]
                        write_json(vocab_dir/f'vocab_{lang}.json',vocab)
                        write_json(vocab_dir/f'vocab_{lang}.provenance.json',{'source_sha256':sha(ROOT/'bin/oneocr.onemodel'),'entries':len(vocab),'sha256':sha(vocab_dir/f'vocab_{lang}.json'),'empty_indices':[i for i,x in enumerate(vocab) if x==''],'method':'delimited CTC probes, singleton fallback for ambiguous batches'})
                        (vocab_dir/f'vocab_{lang}.txt').write_text(''.join(f'{i}: {x}\n' for i,x in enumerate(vocab)),encoding='utf8')
                        seen.add(v)
                        print('Vocabulary',lang,len(vocab),flush=True)
                        break
                finally:
                    bind('ReleaseOcrResult',C.c_int64)(result)
        if seen != set(MODEL_NAMES):
            raise ValueError(f'Missing vocabulary groups {set(MODEL_NAMES)-seen}')
    finally:
        forcing['script'] = None
        bind('ReleaseOcrProcessOptions',C.c_int64)(opts)

def trace_pipeline(bind,check,pipeline,path,directory,pixel_decoder=None,max_lines=1000):
    if pixel_decoder:
        import struct
        pixels=directory/'source-pixels.bgra';directory.mkdir(parents=True,exist_ok=True)
        decoder=Path(pixel_decoder).resolve()
        subprocess.run([str(decoder),str(Path(path).resolve()),str(pixels.resolve())],check=True,timeout=30)
        payload=pixels.read_bytes();width,height=struct.unpack_from('<ii',payload)
        if width<=0 or height<=0 or len(payload)!=8+width*height*4:raise ValueError('Invalid isolated pixel decoder output')
        raw=payload[8:];decoder_record={'implementation':'project stb_image child process','binary_sha256':sha(decoder)}
    else:
        from PIL import Image
        image=Image.open(path)
        if image.format!='PNG':raise ValueError('Non-PNG traces require --pixel-decoder to match the original wrapper pixels')
        image=image.convert('RGBA');width,height=image.size;raw=image.tobytes('raw','BGRA');decoder_record={'implementation':'Pillow RGBA (PNG only)'}
    write_json(directory/'pixel-source.json',dict(decoder_record,width=width,height=height,bgra_sha256=hashlib.sha256(raw).hexdigest()))
    data = C.create_string_buffer(raw)
    class Frame(C.Structure):
        _fields_ = [('type',C.c_int32),('width',C.c_int32),('height',C.c_int32),('reserved',C.c_int32),('stride',C.c_int64),('data',C.c_void_p)]
    frame = Frame(3,width,height,0,width*4,C.cast(data,C.c_void_p))
    opts,result = C.c_int64(),C.c_int64()
    check(bind('CreateOcrProcessOptions',C.POINTER(C.c_int64))(C.byref(opts)))
    try:
        check(bind('OcrProcessOptionsSetMaxRecognitionLineCount',C.c_int64,C.c_int64)(opts,max_lines))
        check(bind('RunOcrPipeline',C.c_int64,C.POINTER(Frame),C.c_int64,C.POINTER(C.c_int64))(pipeline,C.byref(frame),opts,C.byref(result)))
        count=C.c_int64();check(bind('GetOcrLineCount',C.c_int64,C.POINTER(C.c_int64))(result,C.byref(count)))
        metadata=[]
        for index in range(count.value):
            line=C.c_int64();check(bind('GetOcrLine',C.c_int64,C.c_int64,C.POINTER(C.c_int64))(result,index,C.byref(line)))
            metadata.append({'index':index,'angle':C.c_float.from_address(line.value+0x58).value,'script':C.c_int32.from_address(line.value+0x7c).value,'box':list((C.c_float*8).from_address(line.value+8))})
        write_json(directory/'native-lines.json',metadata)
    finally:
        if result.value:
            bind('ReleaseOcrResult',C.c_int64)(result)
        bind('ReleaseOcrProcessOptions',C.c_int64)(opts)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('command',choices=['inventory','extract','capture-worker'])
    p.add_argument('--bin',default=str(ROOT/'bin'))
    p.add_argument('--models',default=str(ROOT/'models'))
    p.add_argument('--vocab',action='store_true')
    p.add_argument('--trace',help='Capture unmodified native model inputs/outputs for this local image')
    p.add_argument('--trace-label',help='Case-local trace directory name; keeps earlier snapshots intact')
    p.add_argument('--trace-geometry',action='store_true',help='Observe fixed DLL TextDetector polygon outputs in this isolated process')
    p.add_argument('--pixel-decoder',help='Research-only oneocr_decode_image executable, required for non-PNG traces')
    p.add_argument('--trace-max-lines',type=int,default=1000,help='Match the original wrapper line limit (default: 1000)')
    p.add_argument('--probe-aux',action='store_true',help='Collect 64 deterministic native samples per 21-feature model')
    p.add_argument('--capture-records',action='store_true',help='Capture plaintext model records at the local DLL BCrypt boundary')
    args = p.parse_args()
    if args.trace_label and (Path(args.trace_label).name!=args.trace_label or args.trace_label in ('.','..')):p.error('trace-label must be a directory name')
    if args.trace_geometry and not args.trace:p.error('trace-geometry requires trace')
    if args.command == 'inventory':
        inventory()
    elif args.command == 'extract':
        command = [sys.executable,str(Path(__file__).resolve()),'capture-worker','--bin',args.bin,'--models',args.models]
        if args.vocab:
            command.append('--vocab')
        if args.trace:
            command.extend(['--trace',str(Path(args.trace).resolve())])
        if args.trace_label:command.extend(['--trace-label',args.trace_label])
        if args.trace:command.extend(['--trace-max-lines',str(args.trace_max_lines)])
        if args.trace_geometry:command.append('--trace-geometry')
        if args.pixel_decoder:command.extend(['--pixel-decoder',str(Path(args.pixel_decoder).resolve())])
        if args.probe_aux:
            command.append('--probe-aux')
        if args.capture_records:
            command.append('--capture-records')
        suffix = '-vocab' if args.vocab else ('-trace-'+(args.trace_label or Path(args.trace).stem) if args.trace else ('-aux' if args.probe_aux else ('-records' if args.capture_records else '')))
        log = CASE/f'evidence/extraction{suffix}.log'
        log.parent.mkdir(parents=True,exist_ok=True)
        with log.open('w',encoding='utf8') as output:
            code = subprocess.call(command,stdout=output,stderr=subprocess.STDOUT,timeout=1800)
        print(log.read_text(encoding='utf8'))
        if code:
            raise SystemExit(code)
    else:
        capture_worker(args)

if __name__ == '__main__':
    main()
