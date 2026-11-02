"""Create an explicitly diagnostic ONNX graph exposing existing node outputs.

Uses a native ONEOCR_TRACE_DIR line to fix seq_lengths; never replaces a model
in models/. Requires the research environment's onnx and numpy packages.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil
import numpy as np
import onnx
from onnx import numpy_helper

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--model', type=Path, required=True)
    parser.add_argument('--trace-line', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    destination = args.output.resolve()
    if not destination.is_relative_to(ROOT/'work'):
        raise ValueError('Diagnostic artifacts must stay under project work/, outside production models/')
    if destination.exists() and any(destination.iterdir()):
        raise ValueError('Use a fresh diagnostic directory')
    trace = args.trace_line
    data_shape = json.loads((trace/'recognizer_input.json').read_text())['shape']
    frames = json.loads((trace/'logsoftmax.json').read_text())['shape'][0]
    model = onnx.load(args.model)
    names = {i.name:i for i in model.graph.input}
    if set(names) != {'data', 'seq_lengths'} or names['seq_lengths'].type.tensor_type.elem_type != onnx.TensorProto.INT32:
        raise ValueError('Unexpected recognizer inputs; inspect the model instead of guessing')
    if len(data_shape) != 4 or data_shape[0] != 1:
        raise ValueError('Expected one traced NCHW recognition region')
    model.graph.input.remove(names['seq_lengths'])
    model.graph.initializer.append(numpy_helper.from_array(np.array([frames],np.int32),'seq_lengths'))
    inferred = onnx.shape_inference.infer_shapes(model)
    info = {v.name:v for v in [*inferred.graph.value_info,*inferred.graph.output]}
    exposed = []
    for node in inferred.graph.node:
        for name in node.output:
            if not name:continue
            value = info.get(name)
            if value is None or not value.type.tensor_type.elem_type:
                raise ValueError('No proven output type for '+name)
            exposed.append(copy.deepcopy(value))
    del model.graph.output[:]
    model.graph.output.extend(exposed)
    onnx.checker.check_model(model)
    (destination/'input').mkdir(parents=True)
    onnx.save(model,destination/'diagnostic.onnx')
    shutil.copyfile(trace/'recognizer_input.f32',destination/'input/data.f32')
    (destination/'input/data.json').write_text(json.dumps({'shape':data_shape})+'\n')
    provenance = {'production_model':args.model.as_posix(),'production_model_sha256':sha(args.model),
                  'diagnostic_model_sha256':sha(destination/'diagnostic.onnx'),
                  'input_sha256':sha(trace/'recognizer_input.f32'),'shape':data_shape,
                  'fixed_seq_lengths':[frames],'exposed_outputs':[v.name for v in exposed],
                  'purpose':'Research only: original operations/weights retained, intermediate outputs exposed; not a production model'}
    (destination/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n')
    print(json.dumps({'outputs':len(exposed),'fixed_seq_lengths':[frames],'diagnostic':str(destination)}))


if __name__ == '__main__':
    main()
