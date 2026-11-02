"""Capture the fixed DLL's UTF-8 edit-distance feature primitive offline."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
ROOT=Path(__file__).resolve().parents[1]
SHA='0dbbec98e5e33de0be9b12d5f286011a6ea8f18e40baae092542b38ff00e7c7b'
def main():
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,required=True);args=parser.parse_args()
    path=ROOT/'bin/oneocr.dll'
    if hashlib.sha256(path.read_bytes()).hexdigest()!=SHA:raise ValueError('Unexpected DLL version')
    dll=C.WinDLL(str(path));distance=C.CFUNCTYPE(C.c_double,C.c_void_p,C.c_void_p)(dll._handle+0xd45d0)
    keep=[]
    def string(text):
        data=text.encode('utf8');obj=C.create_string_buffer(32)
        if len(data)<=15:C.memmove(obj,data,len(data))
        else:
            payload=C.create_string_buffer(data);keep.append(payload);struct.pack_into('<Q',obj,0,C.addressof(payload))
        struct.pack_into('<QQ',obj,16,len(data),max(15,len(data)));keep.append(obj);return obj
    cases=[]
    texts=['','ASCII','ascii','0.8×25=','0.8x25=','你好','你号','αβγ','αBγ','😀','😃','مرحبا','مرحبا عالم','LONG TEXT LONG TEXT','LONG TEXT LONG TEST']
    for a in texts:
        for b in texts:cases.append({'first':a,'second':b,'distance':distance(string(a),string(b))})
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps({'source_sha256':SHA,'function_rva':'0xd45d0','cases':cases},ensure_ascii=False,indent=2),encoding='utf8');print({'cases':len(cases)})
if __name__=='__main__':main()
