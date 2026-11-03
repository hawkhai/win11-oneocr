"""Version-specific, process-local observation of the Photos orientation call.

The caller verifies the DLL SHA-256 first. A single E8 call is redirected to a
nearby relay; the callee itself is untouched and is called with original args.
Restore must run before releasing the pipeline. No on-disk patch is made.
"""
import ctypes as C
import struct

def install(dll,kernel,vector,errors):
    address=dll._handle+0x49079b;old=C.string_at(address,5)
    expected=b'\xe8'+struct.pack('<i',0x491610-(0x49079b+5))
    if old!=expected:raise ValueError('Photos callsite bytes differ from reviewed Ghidra assembly')
    original=C.CFUNCTYPE(None,*([C.c_void_p]*4))(dll._handle+0x491610);records=[]
    def string(address):
        size=C.c_uint64.from_address(address+16).value;capacity=C.c_uint64.from_address(address+24).value
        if size>1000000 or size>capacity:raise ValueError('Original string bounds')
        pointer=C.c_uint64.from_address(address).value if capacity>15 else address
        return C.string_at(pointer,size).decode('utf8')
    def hook(output,texts,results,options):
        record={'lines':[]}
        try:
            tb,tn=vector(texts,8);rb,rn=vector(results,144)
            for i in range(rn):
                row=rb+i*144;index=C.c_uint64.from_address(row).value
                if i>=tn:raise ValueError('Original line vector bounds')
                line=C.c_uint64.from_address(tb+i*8).value;poly=C.c_uint64.from_address(line+0x148).value
                ub,un=vector(poly+64,8);lb,ln=vector(poly+88,8);wb,wn=vector(row+64,224)
                text=''.join(string(wb+j*224+128) for j in range(wn))
                record['lines'].append({'source_index':index,'polygon':list((C.c_float*(un*2)).from_address(ub))+list((C.c_float*(ln*2)).from_address(lb)),
                    'text':text,'horizontal':bool(C.c_uint8.from_address(line+0x15a).value),'ambiguous':bool(C.c_uint8.from_address(poly+1).value),
                    'min_rectangle':bool(C.c_uint8.from_address(poly+2).value),'script':C.c_int32.from_address(line+0x17c).value,'style':C.c_int32.from_address(line+0x170).value,
                    'confidence_score':C.c_float.from_address(row+92).value,'language_score':C.c_float.from_address(row+96).value})
        except BaseException as error:errors.append('Choice input: '+repr(error))
        original(output,texts,results,options)
        try:
            rb,rn=vector(output,144);record['kept_source_indices']=[C.c_uint64.from_address(rb+i*144).value for i in range(rn)];records.append(record)
        except BaseException as error:errors.append('Choice output: '+repr(error))
    callback=C.CFUNCTYPE(None,*([C.c_void_p]*4))(hook)
    kernel.VirtualAlloc.argtypes=[C.c_void_p,C.c_size_t,C.c_ulong,C.c_ulong];kernel.VirtualAlloc.restype=C.c_void_p
    kernel.VirtualFree.argtypes=[C.c_void_p,C.c_size_t,C.c_ulong]
    kernel.GetCurrentProcess.restype=C.c_void_p;kernel.FlushInstructionCache.argtypes=[C.c_void_p,C.c_void_p,C.c_size_t]
    relay=None
    for offset in range(0x4000000,0x70000000,0x10000):
        relay=kernel.VirtualAlloc((dll._handle+offset)&~0xffff,4096,0x3000,0x40)
        if relay:break
    if not relay:raise RuntimeError('No nearby observation relay allocation')
    jump=b'\x48\xb8'+struct.pack('<Q',C.cast(callback,C.c_void_p).value)+b'\xff\xe0';C.memmove(relay,jump,len(jump))
    protect=C.c_ulong();ignored=C.c_ulong()
    if not kernel.VirtualProtect(address,5,0x40,C.byref(protect)):raise C.WinError(C.get_last_error())
    C.memmove(address,b'\xe8'+struct.pack('<i',relay-(address+5)),5)
    kernel.VirtualProtect(address,5,protect.value,C.byref(ignored));kernel.FlushInstructionCache(kernel.GetCurrentProcess(),address,5)
    def restore():
        # Capture callback in this closure so it cannot be collected while live.
        _=callback
        kernel.VirtualProtect(address,5,0x40,C.byref(ignored));C.memmove(address,old,5)
        kernel.VirtualProtect(address,5,protect.value,C.byref(ignored));kernel.FlushInstructionCache(kernel.GetCurrentProcess(),address,5)
        kernel.VirtualFree(relay,0,0x8000)
    return records,restore
