"""Verify the compact recorded flip/layout witness without NumPy or inference."""
import argparse
import hashlib
import json
import math
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--witness',type=Path,default=ROOT/'reports/options-handwriting-2026-11-03/evidence/flip-layout-witness')
    args=parser.parse_args();directory=args.witness.resolve()
    manifest=json.loads((directory/'manifest.json').read_text(encoding='utf8'));arrays={}
    for name,record in manifest['files'].items():
        path=(directory/name).resolve()
        if not path.is_relative_to(directory):raise ValueError('Witness path escapes directory')
        data=path.read_bytes()
        if (record['dtype']!='float32-little-endian' or len(data)!=record['bytes'] or
            len(data)!=math.prod(record['shape'])*4 or hashlib.sha256(data).hexdigest()!=record['sha256']):
            raise ValueError('Witness changed: '+name)
        arrays[name]=data
    shape=manifest['files']['classifier-input.f32']['shape']
    if len(shape)!=4 or shape[:2]!=[1,3]:raise ValueError('Unexpected classifier shape')
    source=arrays['classifier-input.f32'];plane=shape[2]*shape[3]*4
    rotated=b''.join(source[channel*plane+offset:channel*plane+offset+4]
                     for channel in range(3) for offset in range(plane-4,-1,-4))
    if rotated!=arrays['photos-layout-input.f32']:raise ValueError('Recorded layout input is not the rotated classifier input')
    if arrays['photos-layout-input.f32']!=arrays['cpp-layout-input.f32']:raise ValueError('CPP layout input differs')
    if arrays['photos-layout-score.f32']!=arrays['cpp-layout-score.f32']:raise ValueError('CPP layout output differs')
    print(json.dumps({'passed':True,'mode':'recorded-tensor-witness','fresh_inference':False,'shape':shape,'files':len(arrays)}))

if __name__=='__main__':main()
