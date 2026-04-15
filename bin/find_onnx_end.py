"""计算 ONNX protobuf 的精确结束位置，截断后尝试加载"""
import os, struct, sys

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
        if shift > 63:
            return -1, pos
    return -1, pos

def skip_field(data, pos, wire_type):
    """跳过一个 protobuf field 的 value 部分"""
    if wire_type == 0:  # varint
        _, pos = decode_varint(data, pos)
    elif wire_type == 1:  # 64-bit
        pos += 8
    elif wire_type == 2:  # length-delimited
        length, pos = decode_varint(data, pos)
        pos += length
    elif wire_type == 5:  # 32-bit
        pos += 4
    else:
        return -1  # unknown wire type
    return pos

# ONNX ModelProto 有效的 field numbers:
# 1=ir_version, 2=producer_name, 3=producer_version, 4=domain,
# 5=model_version, 6=doc_string, 7=graph, 8=opset_import,
# 14=training_info, 25=functions
VALID_ONNX_FIELDS = {1, 2, 3, 4, 5, 6, 7, 8, 14, 25}

def find_onnx_end(data):
    """解析 protobuf fields，遇到无效 field number 时停止，返回有效部分的结束位置"""
    pos = 0
    last_valid_pos = 0
    while pos < len(data):
        tag_start = pos
        tag, pos = decode_varint(data, pos)
        if tag == -1:
            break
        field_num = tag >> 3
        wire_type = tag & 0x07
        
        if field_num not in VALID_ONNX_FIELDS or wire_type > 5:
            # 遇到非 ONNX field，前面的数据是有效 ONNX
            return tag_start
        
        pos = skip_field(data, pos, wire_type)
        if pos == -1 or pos > len(data):
            return tag_start
        last_valid_pos = pos
    return last_valid_pos


DUMP_DIR = "onnx_dump"
MAGIC = b'\x4a\x1a\x08\x2b\x25\x00\x00\x00'

try:
    import onnx
    HAS_ONNX = True
except ImportError:
    HAS_ONNX = False

samples = sorted(os.listdir(DUMP_DIR))
ok = 0
fail = 0
for fname in samples:
    fpath = os.path.join(DUMP_DIR, fname)
    if not os.path.isfile(fpath):
        continue
    with open(fpath, "rb") as f:
        raw = f.read()
    
    # 跳过 header
    if raw[:8] == MAGIC:
        data = raw[8:]
    else:
        data = raw
    
    # 检查是否以 ONNX 典型的 ir_version field 开头 (tag=0x08, value=3..9)
    if len(data) < 2 or data[0] != 0x08 or not (3 <= data[1] <= 9):
        print(f"{fname}: NOT ONNX (first bytes: {' '.join('%02x'%b for b in data[:8])})")
        continue
    
    onnx_end = find_onnx_end(data)
    onnx_data = data[:onnx_end]
    tail_size = len(data) - onnx_end
    
    status = ""
    if HAS_ONNX:
        try:
            model = onnx.load_from_string(onnx_data)
            status = f"OK ir={model.ir_version} producer={model.producer_name}"
            ok += 1
        except Exception as e:
            status = f"FAIL: {e}"
            fail += 1
    
    print(f"{fname}: total={len(raw)} onnx={len(onnx_data)} tail={tail_size}  {status}")

print(f"\n总计: {ok} OK, {fail} FAIL")
