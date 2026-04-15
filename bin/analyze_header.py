"""分析 onnx_dump 文件的 header 结构，寻找 ONNX protobuf 的真正偏移"""
import os, struct, sys

DUMP_DIR = sys.argv[1] if len(sys.argv) > 1 else "onnx_dump"

# ONNX ModelProto 的 protobuf 起始特征：
#   field 1 (ir_version) = varint, tag = 0x08
#   field 7 (opset_import) tag = 0x3a
#   field 8 (producer_name) tag = 0x42 或 0x0a (graph)
# 常见开头字节: 08 xx (ir_version)

def find_onnx_start(data):
    """在 data 中搜索可能的 ONNX protobuf 起始位置"""
    # 方法1: 搜索 ONNX 的 magic pattern: 0x08 后跟小 varint (ir_version 3-9)
    # 然后几字节后出现 field tag 0x12 (producer_name) 或 0x1a (domain)
    candidates = []
    for i in range(min(len(data) - 4, 256)):
        if data[i] == 0x08 and 3 <= data[i+1] <= 9:
            # 可能是 ir_version field
            # 检查后续是否有合理的 protobuf field tags
            j = i + 2
            if j < len(data) and data[j] in (0x12, 0x1a, 0x22, 0x2a, 0x3a, 0x42):
                candidates.append(i)
    return candidates

files = sorted(os.listdir(DUMP_DIR))
for fname in files:
    fpath = os.path.join(DUMP_DIR, fname)
    if not os.path.isfile(fpath):
        continue
    sz = os.path.getsize(fpath)
    with open(fpath, "rb") as f:
        head = f.read(min(sz, 512))

    # 显示前 64 字节 hex
    hex_line = " ".join(f"{b:02x}" for b in head[:64])
    print(f"\n{'='*80}")
    print(f"{fname}  ({sz:,} bytes)")
    print(f"  HEX[0:64]: {hex_line}")

    # 前8字节解析为两个 uint32 LE
    if len(head) >= 8:
        u32_0, u32_1 = struct.unpack_from("<II", head, 0)
        print(f"  u32[0]={u32_0:#x} ({u32_0})  u32[1]={u32_1:#x} ({u32_1})")
    if len(head) >= 16:
        u64_0, u64_1 = struct.unpack_from("<QQ", head, 0)
        print(f"  u64[0]={u64_0:#x} ({u64_0})  u64[1]={u64_1:#x} ({u64_1})")

    # 搜索 ONNX protobuf 起始
    candidates = find_onnx_start(head)
    if candidates:
        print(f"  ONNX candidates at offsets: {candidates}")
        for off in candidates[:3]:
            snippet = " ".join(f"{b:02x}" for b in head[off:off+16])
            print(f"    @{off}: {snippet}")
    else:
        print(f"  No ONNX start found in first {min(len(head),256)} bytes")

    # 也尝试查找 "pytorch" 或 "onnx" 字符串
    for pattern, label in [(b"pytorch", "pytorch"), (b"onnx", "onnx"), (b"ONNX", "ONNX"), (b"\x08\x06\x12", "ir_v6+producer")]:
        idx = head.find(pattern)
        if idx >= 0:
            print(f"  Found '{label}' at offset {idx}")
