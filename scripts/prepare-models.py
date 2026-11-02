"""Prepare independent model data from the user's local OneModel file.

No Microsoft DLL is loaded. Source resource paths, rather than vocabulary
sizes or byte sizes, determine model identities and language names.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import onnx
sys.path.insert(0,str(Path(__file__).resolve().parent))
from model_config import settings

ROOT=Path(__file__).resolve().parents[1]
SCRIPTS={'ArabicMixed':'arabic','CJKPrinted':'cjk','CyrillicMixed':'cyrillic','DevanagariMixed':'devanagari','GreekMixed':'greek','HebrewPrinted':'hebrew','LatinPrintedV2':'latin','TamilPrinted':'tamil','ThaiMixed':'thai'}

def character_type(token):
    special={'<blank>':4,'<eps>':4,'<space>':5,'<trash_as_space>':8}
    if token in special:return special[token]
    if len(token)!=1:return 0
    cp=ord(token)
    if 65<=cp<=90 or 97<=cp<=122:return 1
    ranges=[(0x4e00,0x9fff),(0x3400,0x4dbf),(0x20000,0x2a6df),(0x2a700,0x2ebef),(0x30000,0x3134f),(0x1100,0x11ff),(0x2e80,0x2fdf),(0x3000,0x318f),(0x31a0,0x33ff),(0xac00,0xd7ff),(0xff00,0xffef)]
    if any(first<=cp<=last for first,last in ranges):return 2
    if 0xe00<=cp<=0xe7f:return 3
    if token=="'":return 6
    if cp in [0x2e,0x2a,0x2d,0x3d,0xb7,0x30fb,0x21,0x2026,0x3a,0x3002,0x3f,0x5f,0x3b,0x3001,0x203b,0xa1,0x7e,0xbf,0x2022,0x3c]:return 7
    return 0

def digest(data): return hashlib.sha256(data).hexdigest()
def write(path,data):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(data)
def write_json(path,value): write(path,json.dumps(value,ensure_ascii=False,indent=2).encode('utf8'))

def prepare(source,models,unpacked):
    subprocess.run([sys.executable,str(ROOT/'scripts/unpack-onemodel.py'),str(source),str(unpacked)],check=True)
    resources=json.loads((unpacked/'resources.json').read_text(encoding='utf8'))
    by_name={record['name'].split('Model_Edge\\',1)[1]:record for record in resources['resources']}
    manifest={'schema_version':1,'source_assets':{'oneocr.onemodel':resources['source_sha256']},'preparation':'static container extraction; no Microsoft DLL loaded','models':[],'vocabularies':[],'resources':[],'script_ids':['unknown','cjk','latin','cyrillic','arabic','devanagari','greek','thai','hebrew','tamil']}
    manifest.update(settings((unpacked/'config.pb').read_bytes(),SCRIPTS))
    for name,record in by_name.items():
        data=(unpacked/record['file']).read_bytes()
        if digest(data)!=record['sha256']: raise ValueError('Unpacked resource checksum mismatch')
        raw_file='resources/'+record['file']
        write(models/raw_file,data)
        manifest['resources'].append(dict(record,file=raw_file))
        if not name.endswith('.onnx'): continue
        graph=onnx.load_model_from_string(data)
        onnx.checker.check_model(graph)
        parts=name.split('\\');kind=parts[0];vocab_size=None
        if kind=='Detector':
            role,file='detector','detector/text_detector.onnx'
        elif kind=='AuxMltCls':
            role,file='classifier','classifier/script_classifier.onnx'
        elif kind=='Character':
            language=SCRIPTS[parts[1]]
            role,file='recognizer','recognizers/recognizer_'+language+'.onnx'
            output=next(v for v in graph.graph.output if v.name=='logsoftmax')
            vocab_size=output.type.tensor_type.shape.dim[-1].dim_value
            if vocab_size<=1: raise ValueError('Recognizer vocabulary dimension is not static')
        elif kind=='LineLayout':
            role,file='layout','layout/line_layout.onnx'
        elif kind in ('Confidence','Rejection'):
            role,file=kind.lower(),'auxiliary/'+record['sha256']+'.onnx'
        else: raise ValueError('Unknown ONNX resource category: '+kind)
        write(models/file,data)
        def info(v):
            t=v.type.tensor_type
            return {'name':v.name,'dtype':t.elem_type,'shape':[d.dim_value if d.HasField('dim_value') else d.dim_param for d in t.shape.dim]}
        manifest['models'].append({'file':file,'sha256':record['sha256'],'size':len(data),'role':role,'source_resource':name,'vocab_size':vocab_size,'inputs':[info(v) for v in graph.graph.input],'outputs':[info(v) for v in graph.graph.output],'opsets':{x.domain:x.version for x in graph.opset_import},'operators':sorted({x.domain+'::'+x.op_type for x in graph.graph.node})})
    for folder,language in SCRIPTS.items():
        prefix='Character\\'+folder+'\\'
        dictionary=by_name[prefix+'char2ind.txt']
        lines=(unpacked/dictionary['file']).read_text(encoding='utf8').splitlines()
        indices={}
        for line in lines:
            token,index=line.rsplit(' ',1);index=int(index)
            if index in indices: raise ValueError('Duplicate vocabulary index')
            indices[index]=token
        model=next(x for x in manifest['models'] if x['file']=='recognizers/recognizer_'+language+'.onnx')
        if set(indices)!=set(range(model['vocab_size'])): raise ValueError('Incomplete vocabulary indices for '+language)
        composite={}
        if prefix+'composite_chars_map' in by_name:
            record=by_name[prefix+'composite_chars_map']
            for line in (unpacked/record['file']).read_text(encoding='utf8').splitlines():
                if line.strip() and not line.startswith('//'):
                    key,value=line.split(' ',1);composite[key]=value
        tokens=[indices[i] for i in range(len(indices))]
        replacements={'<space>':' ','<blank>':'','<trash_as_space>':' '}
        vocab=[replacements.get(token,composite.get(token,token)) for token in tokens]
        file='vocab/vocab_'+language+'.json';write_json(models/file,vocab)
        insensitive_record=by_name[prefix+'char2inschar.txt']
        insensitive={}
        for line in (unpacked/insensitive_record['file']).read_text(encoding='utf8').splitlines():
            token,target=line.split(' ',1)
            if token in insensitive:raise ValueError('Duplicate insensitive character mapping')
            insensitive[token]=target
        token_indices={token:index for index,token in indices.items()}
        if set(insensitive)!=set(tokens) or not set(insensitive.values())<=set(tokens):raise ValueError('Incomplete insensitive character mapping')
        info_record=by_name[prefix+'rnn.info']
        info_lines=(unpacked/info_record['file']).read_text(encoding='utf8').splitlines()
        prior_header=info_lines.index('<LogPrior> '+str(len(tokens)))
        phone_header=info_lines.index('<PhoneMap> '+str(len(tokens)))
        log_priors=[float(v) for v in info_lines[prior_header+1:prior_header+1+len(tokens)]]
        phone_indices=[int(v) for v in info_lines[phone_header+1:phone_header+1+len(tokens)]]
        if len(log_priors)!=len(tokens) or len(phone_indices)!=len(tokens):raise ValueError('Incomplete RNN metadata')
        metadata={'tokens':tokens,'composite_characters':composite,'source_resource':dictionary['name'],
                  'logical_indices':[token_indices[insensitive[token]] for token in tokens],
                  'physical_indices':phone_indices,'character_types':[character_type(token) for token in tokens],
                  'log_priors':log_priors,'insensitive_source_sha256':insensitive_record['sha256'],'rnn_info_source_sha256':info_record['sha256']}
        metadata_file='vocab/metadata_'+language+'.json';write_json(models/metadata_file,metadata)
        proof={'file':file,'sha256':digest((models/file).read_bytes()),'entries':len(vocab),'source_sha256':resources['source_sha256'],'source_resource':dictionary['name'],'dictionary_sha256':dictionary['sha256'],'empty_indices':[i for i,x in enumerate(vocab) if not x],'method':'complete original char2ind.txt plus composite_chars_map; preserves special token metadata'}
        write_json(models/('vocab/vocab_'+language+'.provenance.json'),proof)
        manifest['vocabularies'].append(proof)
        manifest.setdefault('character_metadata',[]).append({'file':metadata_file,'sha256':digest((models/metadata_file).read_bytes()),'language':language,'entries':len(tokens)})
    calibration=by_name['AuxMltCls\\handwritten_calibration_map.txt']
    pairs=[[float(v) for v in line.split()] for line in (unpacked/calibration['file']).read_text().splitlines() if line.strip()]
    if any(len(pair)!=2 for pair in pairs) or any(pairs[i][0]>=pairs[i+1][0] for i in range(len(pairs)-1)): raise ValueError('Invalid calibration table')
    file='calibration/handwritten.json';write_json(models/file,pairs)
    manifest['handwritten_calibration']={'file':file,'sha256':digest((models/file).read_bytes()),'source_resource':calibration['name']}
    write(models/'resources/config.pb',(unpacked/'config.pb').read_bytes())
    write_json(models/'manifest.json',manifest)
    print('Prepared',len(manifest['models']),'graphs and',len(manifest['vocabularies']),'complete original vocabularies')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--source',type=Path,default=ROOT/'bin/oneocr.onemodel')
    p.add_argument('--models',type=Path,default=ROOT/'models')
    p.add_argument('--unpacked',type=Path,default=ROOT/'work/oneocr-portable/unpacked')
    p.add_argument('--raw-only',action='store_true',help='Research only: skip portable graph conversion')
    p.add_argument('--evidence-dir',type=Path)
    args=p.parse_args();prepare(args.source,args.models,args.unpacked)
    if not args.raw_only:
        for script in ('inspect-models.py','convert-auxiliary.py'):
            command=[sys.executable,str(ROOT/'scripts'/script),'--models',str(args.models)]
            if args.evidence_dir:command+=['--evidence-dir',str(args.evidence_dir)]
            subprocess.run(command,check=True)
        print('Portable model preparation complete; no Microsoft DLL was loaded')
