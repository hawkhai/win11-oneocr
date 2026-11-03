"""Stage only manifest-verified inputs for the pinned Python research reference."""
import json
from pathlib import Path
import shutil

from verification_state import contained, read, sha

REFERENCE_LABELS={32632:'cjk',548:'cyrillic',415:'latin',221:'arabic',237:'devanagari',244:'hebrew',199:'thai',201:'bengali',179:'greek'}


def stage_reference_inputs(models,target):
    models,target=Path(models).resolve(),Path(target).resolve()
    if target.is_relative_to(models) or models.is_relative_to(target):
        raise ValueError('Reference cache must be separate from source models')
    manifest=read(models/'manifest.json');plan=[];bindings=[];destinations=set()
    def add(source,relative,expected,payload=None):
        source=contained(models,source);destination=contained(target,relative)
        if sha(source)!=expected:raise ValueError('Reference source checksum mismatch: '+str(source))
        if relative in destinations:raise ValueError('Duplicate reference asset: '+relative)
        destinations.add(relative)
        if destination.exists():
            if payload is None:
                if sha(destination)!=expected:raise ValueError('Reference cache checksum mismatch: '+relative)
            elif destination.read_bytes().replace(b'\r\n',b'\n')!=payload:
                raise ValueError('Reference cached vocabulary mismatch: '+relative)
        plan.append((source,destination,relative,expected,payload))
    vocabularies={v['file']:v for v in manifest['vocabularies']}
    for record in manifest['models']:
        if record['role'] in ('detector','classifier'):relative=record['file']
        elif record['role']=='recognizer':
            alias=REFERENCE_LABELS[record['vocab_size']]
            relative='recognizers/recognizer_'+alias+'.onnx'
            actual=Path(record['file']).stem.removeprefix('recognizer_')
            name='vocab/vocab_'+actual+'.json';vocabulary=vocabularies[name]
            source=contained(models,name)
            if sha(source)!=vocabulary['sha256']:raise ValueError('Reference source checksum mismatch: '+name)
            vocab=read(source)
            if len(vocab)!=record['vocab_size']:raise ValueError('Reference vocabulary size mismatch: '+name)
            payload=''.join(f'{i}: {x}\n' for i,x in enumerate(vocab)).encode('utf8')
            add(name,'vocab/vocab_'+alias+'.txt',vocabulary['sha256'],payload)
            bindings.append({'reference_label':alias,'actual_resource':record['source_resource'],
                             'vocab_size':record['vocab_size'],'model_sha256':record['sha256'],
                             'vocabulary_source_sha256':vocabulary['sha256']})
        else:continue
        add(record['file'],relative,record['sha256'])
    expected={'detector/text_detector.onnx','classifier/script_classifier.onnx'}
    expected.update('recognizers/recognizer_'+alias+'.onnx' for alias in REFERENCE_LABELS.values())
    expected.update('vocab/vocab_'+alias+'.txt' for alias in REFERENCE_LABELS.values())
    if destinations!=expected:raise ValueError('Incomplete reference model inventory')
    # Validate the entire existing cache before creating or replacing any file.
    for source,destination,relative,expected,payload in plan:
        if not destination.exists():
            destination.parent.mkdir(parents=True,exist_ok=True)
            if payload is None:shutil.copyfile(source,destination)
            else:destination.write_bytes(payload)
    assets=[]
    for source,destination,relative,expected,payload in plan:
        # Also bind the bytes actually handed to the reference after staging.
        if sha(source)!=expected:raise ValueError('Reference source changed during staging: '+str(source))
        if (sha(destination)!=expected if payload is None else destination.read_bytes().replace(b'\r\n',b'\n')!=payload):
            raise ValueError('Reference cache changed during staging: '+relative)
        assets.append({'file':relative,'sha256':sha(destination),'source_file':source.relative_to(models).as_posix(),
                       'source_sha256':expected,'transformation':'unchanged' if payload is None else 'reference indexed UTF-8 vocabulary'})
    provenance={'schema_version':1,'model_source_sha256':manifest['source_assets']['oneocr.onemodel'],
                'model_manifest_sha256':sha(models/'manifest.json'),'assets':assets,'bindings':bindings}
    (target/'bindings.json').write_text(json.dumps(bindings,indent=2)+'\n',encoding='utf8')
    (target/'reference-inputs.json').write_text(json.dumps(provenance,indent=2)+'\n',encoding='utf8')
    return provenance
