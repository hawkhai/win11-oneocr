# SPDX-License-Identifier: AGPL-3.0-only
"""Unpack the local OneModel container without loading any Microsoft DLL.

Container-format reference: shiyori/oneocr-native, commit
6020d4339ce2feb835393e2512e433baec29f8d3, internal/engine/onemodel_container.go
(AGPL-3.0-only). This standalone preparation utility is distributed under
that license; it is not linked into the C++ runtime. See THIRD_PARTY_NOTICES.
Model payloads retain their original ownership and are never downloaded.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad

LIMIT=2*1024*1024*1024
PASSWORD=b'kj)TGtrK>f]b[Piow.gU+nC@s""""""4'

class Reader:
    def __init__(self,data): self.data=data;self.position=0
    def read(self,length):
        end=self.position+length
        if length<0 or end>len(self.data): raise ValueError('Truncated container field')
        value=self.data[self.position:end];self.position=end;return value
    def integer(self): return struct.unpack('<Q',self.read(8))[0]
    def blob(self,limit=LIMIT):
        length=self.integer()
        if length>limit: raise ValueError('Container field exceeds configured limit')
        return self.read(length)
    def finish(self):
        if self.position!=len(self.data): raise ValueError('Unexpected trailing container bytes')

def decrypt(record,password=b'',salted=True):
    r=Reader(record)
    salt=r.read(16) if salted else b''
    header=b'' if password else r.read(16)
    key=hashlib.sha256((password or header)+salt).digest()
    encrypted=r.read(len(record)-r.position)
    matches=[]
    # The local 9.0.103 sample uses AES-CFB128, confirmed against the DLL's
    # BCrypt plaintext captures. The format reference also documents CBC.
    for mode in ('CFB128','CBC-PKCS7'):
        try:
            if mode=='CFB128':
                plain=AES.new(key,AES.MODE_CFB,b'Copyright @ OneO',segment_size=128).decrypt(encrypted)
            else:
                plain=unpad(AES.new(key,AES.MODE_CBC,b'Copyright @ OneO').decrypt(encrypted),16)
            p=Reader(plain)
            size,total=struct.unpack('<QQ',header) if header else (p.integer(),p.integer())
            if p.integer()!=0x252b081a4a or size>LIMIT or total!=size+24:
                continue
            payload=p.read(size);p.finish();matches.append(payload)
        except ValueError:
            continue
    if len(matches)!=1: raise ValueError('Unknown or ambiguous container encoding, inconsistent lengths, or wrong key')
    return matches[0]

def unpack(source,destination):
    if source.stat().st_size>LIMIT: raise ValueError('Model exceeds 2 GiB limit')
    raw=source.read_bytes();r=Reader(raw)
    index=Reader(decrypt(r.blob(),PASSWORD));body=r.blob();r.finish()
    config=decrypt(index.blob());count=index.integer()
    if not 1<=count<=4096: raise ValueError('Invalid resource count')
    destination.mkdir(parents=True,exist_ok=True)
    (destination/'config.pb').write_bytes(config)
    records=[];names=set();covered=0
    for i in range(count):
        name=decrypt(index.blob(16384),salted=False).decode('utf8',errors='strict')
        if not name or '\0' in name or name in names: raise ValueError('Invalid or duplicate resource name')
        names.add(name)
        offset,length=index.integer(),index.integer()
        if index.read(1)!=b'\1' or offset!=covered or length>len(body)-offset:
            raise ValueError('Unsupported resource flags, ordering or bounds')
        payload=decrypt(body[offset:offset+length]);covered+=length
        digest=hashlib.sha256(payload).hexdigest()
        # Keep embedded paths as metadata; never trust them as output paths.
        file=f'{i:04d}-{digest}.bin';(destination/file).write_bytes(payload)
        records.append({'name':name,'file':file,'size':len(payload),'sha256':digest})
    index.finish()
    if covered!=len(body): raise ValueError('Resource index does not cover container body')
    report={'source_sha256':hashlib.sha256(raw).hexdigest(),'config_sha256':hashlib.sha256(config).hexdigest(),'resources':records}
    (destination/'resources.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
    print('Unpacked',len(records),'resources without loading any DLL')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('source',type=Path);p.add_argument('destination',type=Path)
    args=p.parse_args();unpack(args.source,args.destination)
