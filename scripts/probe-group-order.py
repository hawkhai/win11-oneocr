"""Observe the fixed DLL's two unordered-map passes used for detector groups."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'
def main():
    import argparse
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);a=p.parse_args();binary=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(binary.read_bytes()).hexdigest()!=SHA:raise ValueError('Unexpected DLL version')
    dll=C.WinDLL(str(binary));allocate=C.CFUNCTYPE(C.c_void_p,C.c_size_t)(dll._handle+0x2b510);insert=C.CFUNCTYPE(C.c_void_p,C.c_void_p,C.c_void_p,C.c_void_p)(dll._handle+0xfeb20)
    def order(keys):
        obj=C.create_string_buffer(64);head=allocate(48);buckets=allocate(128);struct.pack_into('<f',obj,0,1.);struct.pack_into('<7Q',obj,8,head,0,buckets,buckets+128,buckets+128,7,8)
        C.c_uint64.from_address(head).value=head;C.c_uint64.from_address(head+8).value=head
        for i in range(16):C.c_uint64.from_address(buckets+8*i).value=head
        for key in keys:
            result=C.create_string_buffer(16);value=C.c_int32(key);insert(obj,result,C.byref(value))
        result=[];node=C.c_uint64.from_address(head).value
        while node!=head:
            if len(result)>len(keys):raise ValueError('Invalid map iteration')
            result.append(C.c_int32.from_address(node+16).value);node=C.c_uint64.from_address(node).value
        return result
    cases=[]
    for n in (0,1,7,8,9,16,64,65,100,124,130,256,300):
        keys=list(range(n));first=order(keys);cases.append({'keys':keys,'first':first,'second':order(first)})
    keys=list(range(124))+list(range(1317,1323));first=order(keys);cases.append({'keys':keys,'first':first,'second':order(first)})
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps({'source_sha256':SHA,'function_rva':'0xfeb20','cases':cases},indent=2),encoding='utf8');print(cases[-1])
if __name__=='__main__':main()
