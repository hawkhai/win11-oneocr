"""Run the ORIGINAL Windows wrapper in its own process (Unicode paths work)."""
import argparse
import ctypes as C
import json
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from regression_reference import active_reference
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('image',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--binary-dir',type=Path,default=ROOT/active_reference().get('legacy_binary_dir','bin'))
    p.add_argument('--model-dir',type=Path)
    p.add_argument('--max-lines',type=int)
    p.add_argument('--resize-width',type=int)
    p.add_argument('--resize-height',type=int)
    args=p.parse_args();binary=args.binary_dir.resolve();model=(args.model_dir or binary).resolve()
    import os
    directory=os.add_dll_directory(str(binary))
    kernel=C.WinDLL('kernel32',use_last_error=True)
    kernel.SetDllDirectoryW.argtypes=[C.c_wchar_p];kernel.SetDllDirectoryW.restype=C.c_int
    if not kernel.SetDllDirectoryW(str(binary)):raise C.WinError(C.get_last_error())
    dll=C.CDLL(str(binary/'oneocr_wrapper.dll'))
    dll.initModel.argtypes=[C.c_wchar_p];dll.initModel.restype=C.c_int
    dll.releaseModel.argtypes=[];dll.releaseModel.restype=C.c_int
    allocator_type=C.CFUNCTYPE(C.c_void_p,C.c_size_t)
    dll.ocrImage.argtypes=[C.c_wchar_p,C.POINTER(C.c_void_p),allocator_type];dll.ocrImage.restype=C.c_int
    dll.ocrImageEx.argtypes=[C.c_wchar_p,C.POINTER(C.c_void_p),allocator_type,C.c_int,C.c_int,C.c_int];dll.ocrImageEx.restype=C.c_int
    crt=C.CDLL('msvcrt');crt.malloc.argtypes=[C.c_size_t];crt.malloc.restype=C.c_void_p;crt.free.argtypes=[C.c_void_p]
    allocator=allocator_type(lambda size:crt.malloc(size))
    status=dll.initModel(str(model))
    if status:raise SystemExit(f'Original model initialization failed: {status}')
    output=C.c_void_p()
    try:
        values=(args.max_lines,args.resize_width,args.resize_height)
        if any(v is not None for v in values):
            if not all(v is not None for v in values):p.error('Extended recognition requires all three option values')
            status=dll.ocrImageEx(str(args.image.resolve()),C.byref(output),allocator,*values)
        else:status=dll.ocrImage(str(args.image.resolve()),C.byref(output),allocator)
        if status:raise SystemExit(f'Original wrapper OCR failed: {status}')
        result=json.loads(C.string_at(output).decode('utf8'))
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf8')
    finally:
        if output.value:crt.free(output)
        dll.releaseModel()
