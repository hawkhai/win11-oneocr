"""Integration checks for ownership, raw stride, threads and corrupt assets."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes as C
import json
import os
from pathlib import Path
import tempfile
import struct
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--image',type=Path,default=ROOT/'test.png')
    p.add_argument('--output',type=Path,default=ROOT/'work/oneocr-portable/evidence/api-integration.json')
    p.add_argument('--pixel-decoder',type=Path,help='Optional native decoder; avoids a Pillow test dependency')
    args=p.parse_args();library=args.library.resolve();models=args.models.resolve()
    directory=os.add_dll_directory(str(library.parent)) if os.name=='nt' else None
    dll=C.CDLL(str(library))
    ptr=C.c_void_p
    class Options(C.Structure):_fields_=[('max_lines',C.c_int32),('width',C.c_int32),('height',C.c_int32)]
    dll.oneocr_create.argtypes=[C.c_char_p,C.POINTER(ptr),C.POINTER(ptr)];dll.oneocr_create.restype=C.c_int
    dll.oneocr_destroy.argtypes=[ptr];dll.oneocr_destroy.restype=None
    dll.oneocr_free.argtypes=[ptr];dll.oneocr_free.restype=None
    dll.oneocr_recognize_file.argtypes=[ptr,C.c_char_p,C.POINTER(Options),C.POINTER(ptr),C.POINTER(ptr)];dll.oneocr_recognize_file.restype=C.c_int
    dll.oneocr_recognize_bgra.argtypes=[ptr,ptr,C.c_size_t,C.c_int32,C.c_int32,C.c_int64,C.POINTER(Options),C.POINTER(ptr),C.POINTER(ptr)];dll.oneocr_recognize_bgra.restype=C.c_int
    def text(pointer):
        try:return C.string_at(pointer).decode('utf8') if pointer else ''
        finally:dll.oneocr_free(pointer)
    def create(path=models):
        context,error=ptr(),ptr();code=dll.oneocr_create(str(path).encode('utf8'),C.byref(context),C.byref(error));message=text(error)
        if code:raise RuntimeError(message)
        return context
    def recognize(context,path=args.image,options=None):
        result,error=ptr(),ptr();code=dll.oneocr_recognize_file(context,str(path).encode('utf8'),C.byref(options) if options else None,C.byref(result),C.byref(error))
        value,message=text(result),text(error)
        if code:raise RuntimeError(message)
        return json.loads(value)
    context=create()
    checks=[]
    try:
        baseline=recognize(context)
        if args.pixel_decoder:
            with tempfile.TemporaryDirectory(prefix='oneocr-api-pixels-') as temporary:
                decoded=Path(temporary)/'pixels.bgra'
                subprocess.run([str(args.pixel_decoder.resolve()),str(args.image.resolve()),str(decoded)],check=True,timeout=30)
                payload=decoded.read_bytes();width,height=struct.unpack_from('<ii',payload);raw=payload[8:]
                if width<1 or height<1 or len(raw)!=width*height*4:raise ValueError('Invalid native pixel payload')
        else:
            from PIL import Image
            image=Image.open(args.image).convert('RGBA');width,height=image.size;raw=image.tobytes('raw','BGRA')
        stride=width*4+16
        padded=bytearray(stride*height)
        for y in range(height):padded[y*stride:y*stride+width*4]=raw[y*width*4:(y+1)*width*4]
        buffer=(C.c_ubyte*len(padded)).from_buffer(padded)
        result,error=ptr(),ptr()
        code=dll.oneocr_recognize_bgra(context,buffer,len(padded),width,height,stride,None,C.byref(result),C.byref(error))
        value,message=text(result),text(error)
        if code:raise RuntimeError(message)
        actual=json.loads(value);baseline['file']=actual['file'];baseline['image']['step']=stride
        if baseline!=actual:raise AssertionError('File and padded raw BGRA results differ')
        checks.append('file versus padded BGRA exact result equivalence')
        with ThreadPoolExecutor(max_workers=3) as executor:
            repeated=list(executor.map(lambda _:recognize(context),range(6)))
        if any(x!=repeated[0] for x in repeated):raise AssertionError('Same-context concurrency is nondeterministic')
        checks.append('six concurrent calls on one context')
        def independent(_):
            other=create()
            try:return recognize(other)
            finally:dll.oneocr_destroy(other)
        with ThreadPoolExecutor(max_workers=2) as executor:separate=list(executor.map(independent,range(2)))
        if separate[0]!=separate[1]:raise AssertionError('Independent contexts differ')
        checks.append('two concurrent independent contexts')
        result,error=ptr(),ptr()
        code=dll.oneocr_recognize_file(context,b'__missing_image__.png',None,C.byref(result),C.byref(error))
        text(error)
        if code!=-13 or result.value:raise AssertionError('Missing-image error contract')
        checks.append('missing image leaves JSON null with image error')
        # The original DLL has a soft batch budget, not a hard line cap.
        # Exercise the multi-line fixture and compare the whole actual output;
        # a one-line image cannot detect a mistaken max_lines truncation.
        from regression_reference import active_reference
        import importlib.util
        reference=active_reference()
        contract='twelve-printed-lines-m1-r1152x768'
        inventory=json.loads((ROOT/reference['parameter_manifest']).read_text(encoding='utf8'))
        case=next(c for c in inventory['cases'] if c['name']==contract)
        limited=recognize(context,path=ROOT/case['file'],options=Options(1,1152,768))
        expected=json.loads((ROOT/reference['parameter_baseline']/(contract+'.json')).read_text(encoding='utf8'))
        spec=importlib.util.spec_from_file_location('api_strict_compare',ROOT/'scripts/compare-results.py')
        comparator=importlib.util.module_from_spec(spec);spec.loader.exec_module(comparator)
        comparison=comparator.compare(expected,limited)
        if not comparison['passed']:raise AssertionError('Soft line-budget parity: '+json.dumps(comparison))
        checks.append('max_lines=1 multi-line soft batch budget strictly matches original DLL')
    finally:dll.oneocr_destroy(context)
    workspace=(ROOT/'work/oneocr-portable').resolve();workspace.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='api-corrupt-',dir=workspace) as temp:
        target=Path(temp).resolve()
        if not target.is_relative_to(workspace):raise RuntimeError('Temporary model directory escaped workspace')
        invalid=target/'invalid-image.bin';invalid.write_bytes(b'This is not an encoded image')
        valid_context=create()
        try:
            result,error=ptr(),ptr()
            code=dll.oneocr_recognize_file(valid_context,str(invalid).encode('utf8'),None,C.byref(result),C.byref(error))
            text(error)
            if code!=-13 or result.value:raise AssertionError('Invalid-image error contract')
            checks.append('existing invalid image leaves JSON null with image error')
        finally:dll.oneocr_destroy(valid_context)
        manifest=json.loads((models/'manifest.json').read_text(encoding='utf8'))
        (target/'manifest.json').write_text(json.dumps(manifest),encoding='utf8')
        assets=manifest['models']+manifest['vocabularies']+[manifest['portable_detector'],manifest['handwritten_calibration']]+manifest.get('portable_auxiliary',[])
        for asset in assets:
            destination=target/asset['file'];destination.parent.mkdir(parents=True,exist_ok=True)
            os.link(models/asset['file'],destination)
        corrupt=target/manifest['vocabularies'][0]['file']
        corrupt.unlink() # Break our hard link before changing any bytes.
        corrupt.write_text('[]',encoding='utf8')
        context,error=ptr(),ptr();code=dll.oneocr_create(str(target).encode('utf8'),C.byref(context),C.byref(error));message=text(error)
        if not code or context.value or 'checksum' not in message.lower():raise AssertionError('Corrupt asset was not rejected by SHA-256')
        checks.append('corrupt vocabulary rejected before inference; source assets unchanged')
    report={'passed':True,'library':str(library),'checks':checks}
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps(report,ensure_ascii=False))
