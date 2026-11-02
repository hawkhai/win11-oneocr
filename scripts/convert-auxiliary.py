"""Replace private feature layers using validated standard ONNX operations.

Fresh preparation uses checked-in hashes and needs no Microsoft DLL.
--record-contracts requires the locally captured native probes/replays.
"""
import argparse
import hashlib
import json
from pathlib import Path
import numpy as np
import onnx
from onnx import helper,numpy_helper
import onnxruntime as ort

ROOT=Path(__file__).resolve().parents[1]
CONTRACTS=ROOT/'research/portable-model-contracts.json'
def sha(data): return hashlib.sha256(data).hexdigest()

def transform(original,activation):
    model=onnx.ModelProto();model.CopyFrom(original)
    custom_nodes=[(i,n) for i,n in enumerate(model.graph.node) if n.domain=='com.microsoft.oneocr']
    if len(custom_nodes)!=1: raise ValueError('Expected one private feature layer')
    index,custom=custom_nodes[0]
    config=next(x for x in model.graph.initializer if x.name==custom.input[1])
    values=np.frombuffer(config.string_data[0],dtype='>f4').astype(np.float32)
    if len(values)==1123 and values[50:52].tolist()==[50,1] and values[1102:1104].tolist()==[21,50]:
        weights=values[52:1102][::-1].copy().reshape(50,21)
        bias=values[:50][::-1].copy()
        projection=helper.make_node('Gemm',[custom.input[0],'portable_weights','portable_bias'],['portable_projection'],transB=1)
        kind='dense-21x50'
    elif len(values)==4137 and values[16:18].tolist()==[16,1] and values[4114:4118].tolist()==[1,1,256,16]:
        weights=values[18:4114][::-1].copy().reshape(16,256,1,1)
        bias=values[:16][::-1].copy()
        projection=helper.make_node('Conv',[custom.input[0],'portable_weights','portable_bias'],['portable_projection'],kernel_shape=[1,1])
        kind='conv-256x16-1x1'
    else: raise ValueError('Unknown private feature serialization; refusing to guess')
    model.graph.initializer.extend([numpy_helper.from_array(weights,'portable_weights'),numpy_helper.from_array(bias,'portable_bias')])
    replacement=[projection,helper.make_node(activation,['portable_projection'],list(custom.output))]
    del model.graph.node[index]
    for node in reversed(replacement):model.graph.node.insert(index,node)
    for i in reversed(range(len(model.graph.initializer))):
        if model.graph.initializer[i].name==custom.input[1]:del model.graph.initializer[i]
    onnx.checker.check_model(model)
    return model,kind

def replay(session,raw_hash,kind):
    if kind=='dense-21x50':
        probe=ROOT/'work/oneocr-portable/auxiliary-probes'/(raw_hash+'.npz')
        samples=np.load(probe)
        predictions=np.stack([session.run(None,{'data':x})[0] for x in samples['inputs']])
        return float(np.max(np.abs(predictions-samples['outputs']))),len(samples['inputs'])
    trace=ROOT/'work/oneocr-portable/traces/ocr-book'
    entries=json.loads((trace/'trace.json').read_text(encoding='utf8'))
    errors=[];samples=0
    for entry in entries:
        if Path(entry['model']).stem!=raw_hash:continue
        samples+=1
        inputs={name:np.load(trace/value['file']) for name,value in entry['inputs'].items()}
        if any(list(inputs[k].shape)!=v['shape'] for k,v in entry['inputs'].items()):raise ValueError('Native trace tensor shape mismatch')
        names=list(entry['outputs'])
        for name,prediction in zip(names,session.run(names,inputs)):
            expected=np.load(trace/entry['outputs'][name]['file'])
            if list(expected.shape)!=entry['outputs'][name]['shape']:raise ValueError('Native trace output shape mismatch')
            errors.append(float(np.max(np.abs(prediction-expected))))
    if not errors:raise ValueError('Native layout replays missing')
    return max(errors),samples

def convert(models,verify,record_contracts):
    manifest=json.loads((models/'manifest.json').read_text(encoding='utf8'))
    contracts=json.loads(CONTRACTS.read_text(encoding='utf8')) if CONTRACTS.exists() else {}
    options=ort.SessionOptions();options.intra_op_num_threads=1;options.log_severity_level=3
    report=[]
    for record in manifest['models']:
        if record['role'] not in ('confidence','rejection','layout'):continue
        path=models/record['file'];raw=path.read_bytes();raw_hash=sha(raw)
        if raw_hash!=record['sha256']:raise ValueError('Source graph checksum mismatch')
        original=onnx.load_model_from_string(raw)
        contract=contracts.get(raw_hash)
        if not verify and not contract:raise ValueError('No validated conversion contract for '+record['file'])
        activations=['Relu','Identity'] if record['role']=='layout' and verify else [contract['activation'] if contract else 'Relu']
        candidates=[]
        for activation in activations:
            model,kind=transform(original,activation)
            session=ort.InferenceSession(model.SerializeToString(),sess_options=options,providers=['CPUExecutionProvider'])
            error,samples=replay(session,raw_hash,kind) if verify else (contract['max_absolute_error'],contract['native_samples'])
            candidates.append((error,activation,model,kind,samples))
        error,activation,model,kind,samples=min(candidates,key=lambda x:x[0])
        # The approved confidence tolerance is 1e-3. Fused vs serial float
        # reductions differ; this does not certify end-to-end equivalence.
        if error>1e-3:raise ValueError(f'Native conversion mismatch: {record["file"]}: {error}')
        encoded=model.SerializeToString()
        if not verify and sha(encoded)!=contract['sha256']:raise ValueError('Converted graph differs from its validated contract')
        target='portable_auxiliary/'+raw_hash+'.onnx'
        (models/target).parent.mkdir(exist_ok=True);(models/target).write_bytes(encoded)
        entry={'source':record['file'],'file':target,'source_sha256':raw_hash,'sha256':sha(encoded),'kind':kind,'activation':activation,'native_samples':samples,'max_absolute_error':error,'accepted':True}
        report.append(entry)
        contracts[raw_hash]={k:entry[k] for k in ('sha256','kind','activation','native_samples','max_absolute_error')}
        print(record['role'],raw_hash[:12],activation,error,flush=True)
    manifest['portable_auxiliary']=report
    (models/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf8')
    if record_contracts:
        CONTRACTS.parent.mkdir(exist_ok=True)
        CONTRACTS.write_text(json.dumps(contracts,indent=2),encoding='utf8')
    output=ROOT/'work/oneocr-portable/evidence/auxiliary-conversion.json'
    output.parent.mkdir(parents=True,exist_ok=True);output.write_text(json.dumps(report,indent=2),encoding='utf8')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--verify',action='store_true')
    p.add_argument('--record-contracts',action='store_true')
    args=p.parse_args();convert(args.models,args.verify or args.record_contracts,args.record_contracts)
