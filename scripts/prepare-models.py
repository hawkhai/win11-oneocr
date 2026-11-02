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

ROOT=Path(__file__).resolve().parents[1]
SCRIPTS={'ArabicMixed':'arabic','CJKPrinted':'cjk','CyrillicMixed':'cyrillic','DevanagariMixed':'devanagari','GreekMixed':'greek','HebrewPrinted':'hebrew','LatinPrintedV2':'latin','TamilPrinted':'tamil','ThaiMixed':'thai'}

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
        write_json(models/('vocab/metadata_'+language+'.json'),{'tokens':tokens,'composite_characters':composite,'source_resource':dictionary['name']})
        proof={'file':file,'sha256':digest((models/file).read_bytes()),'entries':len(vocab),'source_sha256':resources['source_sha256'],'source_resource':dictionary['name'],'dictionary_sha256':dictionary['sha256'],'empty_indices':[i for i,x in enumerate(vocab) if not x],'method':'complete original char2ind.txt plus composite_chars_map; preserves special token metadata'}
        write_json(models/('vocab/vocab_'+language+'.provenance.json'),proof)
        manifest['vocabularies'].append(proof)
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
    args=p.parse_args();prepare(args.source,args.models,args.unpacked)
    if not args.raw_only:
        for script in ('inspect-models.py','convert-auxiliary.py'):
            subprocess.run([sys.executable,str(ROOT/'scripts'/script),'--models',str(args.models)],check=True)
        print('Portable model preparation complete; no Microsoft DLL was loaded')
