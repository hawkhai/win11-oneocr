"""Compare explicit floating-point accumulation orders with recorded native Conv tensors.

Research only: does not change an OCR model, weight, threshold or result.
"""
import argparse
import ctypes
import ctypes.util
import json
from pathlib import Path
import sys
import numpy as np
import onnx
from onnx import numpy_helper, helper


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--model',type=Path,required=True)
    parser.add_argument('--native',type=Path,required=True)
    parser.add_argument('--input',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    model=onnx.load(args.model);initializers={x.name:numpy_helper.to_array(x) for x in model.graph.initializer}
    library=ctypes.CDLL('ucrtbase.dll' if sys.platform=='win32' else ctypes.util.find_library('m'))
    fma=library.fmaf;fma.argtypes=[ctypes.c_float]*3;fma.restype=ctypes.c_float
    def tensor(name):
        path=args.native/(name.replace('/','_')+'.bin')
        if not path.exists():path=args.input/(name+'.f32')
        shape=json.loads(path.with_suffix('.json').read_text())['shape']
        return np.fromfile(path,np.float32).reshape(shape)
    records=[]
    for node in model.graph.node:
        if node.op_type!='Conv':continue
        x=tensor(node.input[0]);golden=tensor(node.output[0]);w=initializers[node.input[1]]
        bias=initializers[node.input[2]] if len(node.input)>2 else np.zeros(w.shape[0],np.float32)
        attributes={a.name:helper.get_attribute_value(a) for a in node.attribute}
        pads=attributes.get('pads',[0,0,0,0]);stride=attributes.get('strides',[1,1]);dilation=attributes.get('dilations',[1,1]);group=attributes.get('group',1)
        ci,kh,kw=w.shape[1:];co=w.shape[0]//group
        positions=[(c,y,z) for c in sorted({0,min(3,w.shape[0]-1),w.shape[0]-1})
                   for y,z in [(0,0),(golden.shape[2]//2,golden.shape[3]//2),(max(0,golden.shape[2]-2),max(0,golden.shape[3]-3))]]
        candidates=[]
        for block in sorted({1,8,16,32,128,ci}):
            for order in ('channel-spatial','spatial-channel'):
                exact=0;maximum=0.
                for oc,oy,ox in positions:
                    total=np.float32(0)
                    for begin in range(0,ci,block):
                        partial=np.float32(0);channels=range(begin,min(begin+block,ci))
                        triples=((c,y,z) for c in channels for y in range(kh) for z in range(kw)) if order=='channel-spatial' else ((c,y,z) for y in range(kh) for z in range(kw) for c in channels)
                        for c,y,z in triples:
                            iy=oy*stride[0]-pads[0]+y*dilation[0];ix=ox*stride[1]-pads[1]+z*dilation[1]
                            value=x[0,(oc//co)*ci+c,iy,ix] if 0<=iy<x.shape[2] and 0<=ix<x.shape[3] else np.float32(0)
                            partial=fma(float(value),float(w[oc,c,y,z]),float(partial))
                        total=np.float32(total+partial)
                    actual=np.float32(total+bias[oc]);expected=golden[0,oc,oy,ox]
                    exact+=int(actual==expected);maximum=max(maximum,float(abs(actual-expected)))
                candidates.append({'channel_block':block,'order':order,'exact':exact,'total':len(positions),'max_absolute_error':maximum})
        record={'node':node.name,'weight_shape':list(w.shape),'group':group,'candidates':candidates};records.append(record)
        print(node.name,[x for x in candidates if x['exact']==x['total']],flush=True)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(records,indent=2)+'\n',encoding='utf8')


if __name__=='__main__':main()
