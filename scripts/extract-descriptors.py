"""Read embedded Protobuf descriptors for offline configuration evidence."""
from pathlib import Path
import json
from google.protobuf import descriptor_pb2,descriptor_pool,message_factory,text_format

ROOT=Path(__file__).resolve().parents[1]
data=(ROOT/'bin/oneocr.dll').read_bytes()
output=ROOT/'work/oneocr-portable/descriptors';output.mkdir(exist_ok=True)

def integer(position):
    value=0
    for shift in range(0,70,7):
        byte=data[position];position+=1;value|=(byte&127)<<shift
        if byte<128:return value,position
    raise ValueError('Invalid descriptor varint')

def extract(name):
    needle=b'\x0a'+bytes([len(name)])+name.encode()
    start=data.index(needle);position=start
    while position<len(data) and data[position]!=0:
        tag,position=integer(position);wire=tag&7
        if tag>>3<1:break
        if wire==0:_,position=integer(position)
        elif wire==1:position+=8
        elif wire==2:
            length,position=integer(position);position+=length
        elif wire==5:position+=4
        else:raise ValueError('Invalid descriptor wire type')
    raw=data[start:position]
    proto=descriptor_pb2.FileDescriptorProto.FromString(raw)
    if proto.name!=name:raise ValueError('Wrong descriptor boundary')
    (output/(name+'.descriptor.pb')).write_bytes(raw)
    (output/(name+'.descriptor.txt')).write_text(text_format.MessageToString(proto),encoding='utf8')
    return raw,proto

pool=descriptor_pool.DescriptorPool()
pool.AddSerializedFile(descriptor_pb2.DESCRIPTOR.serialized_pb)
pending=[extract(name) for name in ('oneocr.proto','oneocr_interop.proto')]
while pending:
    progress=False
    for raw,proto in pending[:]:
        try:
            pool.AddSerializedFile(raw);pending.remove((raw,proto));progress=True
            print('Descriptor',proto.name,'package',proto.package,'messages',len(proto.message_type))
        except TypeError:
            continue
    if not progress:raise ValueError('Unresolved descriptor dependencies')
config=(ROOT/'work/oneocr-portable/unpacked/config.pb').read_bytes()
message=message_factory.GetMessageClass(pool.FindMessageTypeByName('OneOCR.PipelineProto'))()
message.ParseFromString(config)
(output/'pipeline.prototxt').write_text(text_format.MessageToString(message),encoding='utf8')
print('Wrote decoded pipeline configuration')
