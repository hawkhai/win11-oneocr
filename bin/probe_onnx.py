"""深入探查 dump 文件的 protobuf 结构"""
import struct, sys, os

def decode_varint(data, pos):
    result = 0
    shift = 0
    while pos < len(data):
        b = data[pos]
        result |= (b & 0x7F) << shift
        pos += 1
        if (b & 0x80) == 0:
            return result, pos
        shift += 7
    return result, pos

def probe_fields(data, max_fields=20):
    """解析 protobuf 顶层字段"""
    pos = 0
    fields = []
    for _ in range(max_fields):
        if pos >= len(data):
            break
        tag, pos = decode_varint(data, pos)
        field_num = tag >> 3
        wire_type = tag & 0x07
        
        if wire_type == 0:  # varint
            val, pos = decode_varint(data, pos)
            fields.append((field_num, 'varint', val, None))
        elif wire_type == 1:  # 64-bit
            val = struct.unpack_from('<Q', data, pos)[0] if pos + 8 <= len(data) else 0
            pos += 8
            fields.append((field_num, 'fixed64', val, None))
        elif wire_type == 2:  # length-delimited
            length, pos = decode_varint(data, pos)
            snippet = data[pos:pos+min(length, 60)]
            fields.append((field_num, 'bytes', length, snippet))
            pos += length
        elif wire_type == 5:  # 32-bit
            val = struct.unpack_from('<I', data, pos)[0] if pos + 4 <= len(data) else 0
            pos += 4
            fields.append((field_num, 'fixed32', val, None))
        else:
            fields.append((field_num, f'wire{wire_type}', 0, None))
            break  # unknown wire type
    return fields

DUMP_DIR = "onnx_dump"
# 分析几个代表性文件
samples = [
    "0003checkpoint.040.onnx",    # 大 ONNX (11MB)
    "0004_decrypt.bin",           # decrypt.bin (3.4MB)
    "0008_decrypt.bin",           # 小 (5KB)
    "0052rejection_model_2023_09_01_14_00_33.onnx",  # 27KB
    "0063confidence_model_calibrated_2023_09_03_02_20_39.onnx",  # 29KB
    "0001_decrypt.bin",           # 目录表
]

for fname in samples:
    fpath = os.path.join(DUMP_DIR, fname)
    if not os.path.isfile(fpath):
        print(f"\n{fname}: NOT FOUND")
        continue
    
    with open(fpath, "rb") as f:
        raw = f.read()
    
    print(f"\n{'='*70}")
    print(f"{fname}  ({len(raw):,} bytes)")
    
    # 跳过 8 字节 header
    MAGIC = b'\x4a\x1a\x08\x2b\x25\x00\x00\x00'
    if raw[:8] == MAGIC:
        data = raw[8:]
        print(f"  [Skipped 8-byte oneocr header]")
    else:
        data = raw
        print(f"  [No oneocr header]")
        # 0001 有不同 header
        print(f"  First 16 bytes: {' '.join('%02x'%b for b in raw[:16])}")
        # 尝试跳过 16 字节
        if len(raw) > 16:
            u0, u1 = struct.unpack_from('<QQ', raw, 0)
            print(f"  u64[0]={u0:#x}  u64[1]={u1:#x}")
            data = raw[16:]
            print(f"  [Trying skip 16 bytes]")
    
    print(f"  Payload size: {len(data):,} bytes")
    print(f"  Payload[0:32]: {' '.join('%02x'%b for b in data[:32])}")
    
    # 解析 protobuf 顶层字段
    fields = probe_fields(data)
    print(f"  Protobuf top-level fields:")
    for fnum, wtype, val, snippet in fields:
        if snippet is not None:
            # 尝试解码为 UTF-8
            try:
                txt = snippet.decode('utf-8', errors='replace')
                if all(0x20 <= ord(c) < 0x7f or c == '\ufffd' for c in txt):
                    print(f"    field {fnum} ({wtype}): len={val}  text={txt!r}")
                else:
                    print(f"    field {fnum} ({wtype}): len={val}  hex={' '.join('%02x'%b for b in snippet[:40])}")
            except:
                print(f"    field {fnum} ({wtype}): len={val}  hex={' '.join('%02x'%b for b in snippet[:40])}")
        else:
            print(f"    field {fnum} ({wtype}): {val}")
