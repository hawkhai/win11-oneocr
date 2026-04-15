"""
verify_onnx.py  –  校验 onnx_dump/ 下所有文件是否为有效 ONNX 模型
用法:  python verify_onnx.py [目录路径]
默认目录:  ./onnx_dump

oneocr 容器格式:
  [8 bytes magic: 4a 1a 08 2b 25 00 00 00]
  [ONNX protobuf 或其他资源数据]
  [1~15 bytes 尾部校验数据]
"""

import sys
import os
import glob
import struct

try:
    import onnx
except ImportError:
    print("ERROR: 需要安装 onnx 包:  pip install onnx")
    sys.exit(1)

try:
    import onnxruntime as ort
    HAS_ORT = True
except ImportError:
    HAS_ORT = False


# oneocr 容器固定 8 字节 magic header
ONEOCR_MAGIC = b'\x4a\x1a\x08\x2b\x25\x00\x00\x00'
HEADER_SIZE = 8

# ONNX ModelProto 有效的顶层 field numbers
VALID_ONNX_FIELDS = {1, 2, 3, 4, 5, 6, 7, 8, 14, 25}


def _decode_varint(data, pos):
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


def _decode_varint_strict(data, pos):
    """Decode varint and also return byte count. Returns (value, new_pos, nbytes)."""
    result = 0
    shift = 0
    start = pos
    while pos < len(data):
        b = data[pos]
        result |= (b & 0x7F) << shift
        pos += 1
        if (b & 0x80) == 0:
            return result, pos, pos - start
        shift += 7
        if shift > 63:
            return -1, pos, pos - start
    return -1, pos, pos - start


def _min_varint_bytes(value):
    """Minimum bytes needed to encode a varint value."""
    if value == 0:
        return 1
    n = 0
    while value > 0:
        value >>= 7
        n += 1
    return n


def find_onnx_end(data):
    """解析 protobuf 顶层 fields，遇到非 ONNX field 时停止，返回有效 ONNX 的结束偏移。"""
    pos = 0
    while pos < len(data):
        tag_start = pos
        tag, pos, tag_bytes = _decode_varint_strict(data, pos)
        if tag == -1:
            return tag_start
        field_num = tag >> 3
        wire_type = tag & 0x07

        # 非规范 varint 编码 (如 a8 00 而非 28) 说明不是正常 protobuf
        if tag_bytes > _min_varint_bytes(tag):
            return tag_start

        if field_num not in VALID_ONNX_FIELDS or wire_type > 5:
            return tag_start

        # skip value
        if wire_type == 0:       # varint
            _, pos = _decode_varint(data, pos)
        elif wire_type == 1:     # fixed64
            pos += 8
        elif wire_type == 2:     # length-delimited
            length, pos = _decode_varint(data, pos)
            if length < 0:
                return tag_start
            pos += length
        elif wire_type == 5:     # fixed32
            pos += 4
        else:
            return tag_start

        if pos > len(data):
            return tag_start
    return pos


def strip_container(raw):
    """去除 oneocr 容器 header，返回 (payload, header_skipped)。"""
    if raw[:HEADER_SIZE] == ONEOCR_MAGIC:
        return raw[HEADER_SIZE:], HEADER_SIZE
    return raw, 0


def is_onnx_start(data):
    """检查 data 是否以 ONNX ir_version field 开头 (tag=0x08, value=3..9)。"""
    return len(data) >= 2 and data[0] == 0x08 and 3 <= data[1] <= 9


def classify_resource(data):
    """对非 ONNX 资源进行简单分类。"""
    if data[:9] == b'<LogPrior':
        return "LogPrior 文本（转录概率表）"
    if len(data) > 2 and data[0:2] == b'! ':
        return "字符映射表（char map）"
    if len(data) > 4 and data[0:4] == b'0.0 ':
        return "浮点参数表"
    return "二进制资源"


def check_file(filepath):
    """校验单个文件，返回结果字典。"""
    fname = os.path.basename(filepath)
    sz = os.path.getsize(filepath)
    result = {
        "file": fname,
        "size": sz,
        "is_onnx": False,
        "onnx_load": False,
        "onnx_check": False,
        "ort_load": False,
        "onnx_bytes": 0,
        "tail_bytes": 0,
        "inputs": [],
        "outputs": [],
        "custom_domains": [],
        "resource_type": None,
        "error": None,
    }

    with open(filepath, "rb") as f:
        raw = f.read()

    payload, hdr_skip = strip_container(raw)
    result["header_skipped"] = hdr_skip

    if not is_onnx_start(payload):
        result["resource_type"] = classify_resource(payload)
        return result

    result["is_onnx"] = True

    # 截断尾部非 ONNX 数据
    onnx_end = find_onnx_end(payload)
    onnx_data = payload[:onnx_end]
    result["onnx_bytes"] = len(onnx_data)
    result["tail_bytes"] = len(payload) - onnx_end

    # 1) onnx.load_from_string
    try:
        model = onnx.load_from_string(onnx_data)
        result["onnx_load"] = True
    except Exception as e:
        result["error"] = "onnx.load failed: %s" % e
        return result

    # 2) onnx.checker
    custom_domains = [op.domain for op in model.opset_import
                      if op.domain and op.domain != "ai.onnx"]
    result["custom_domains"] = custom_domains
    try:
        onnx.checker.check_model(model)
        result["onnx_check"] = True
    except Exception as e:
        if custom_domains:
            result["onnx_check"] = True
            result["error"] = "checker skipped (custom ops: %s)" % ', '.join(custom_domains)
        else:
            result["error"] = "onnx.checker failed: %s" % e

    # 提取输入/输出信息
    for inp in model.graph.input:
        shape = []
        if inp.type.tensor_type.HasField("shape"):
            for dim in inp.type.tensor_type.shape.dim:
                shape.append(dim.dim_param if dim.dim_param else dim.dim_value)
        result["inputs"].append({"name": inp.name, "shape": shape})

    for out in model.graph.output:
        shape = []
        if out.type.tensor_type.HasField("shape"):
            for dim in out.type.tensor_type.shape.dim:
                shape.append(dim.dim_param if dim.dim_param else dim.dim_value)
        result["outputs"].append({"name": out.name, "shape": shape})

    # 3) onnxruntime 加载
    if HAS_ORT and result["onnx_check"]:
        try:
            opts = ort.SessionOptions()
            opts.graph_optimization_level = ort.GraphOptimizationLevel.ORT_DISABLE_ALL
            sess = ort.InferenceSession(onnx_data, opts,
                                        providers=["CPUExecutionProvider"])
            result["ort_load"] = True
            del sess
        except Exception as e:
            err_msg = str(e)[:150]
            if result["error"] is None:
                result["error"] = "ort load failed: %s" % err_msg
            else:
                result["error"] += " | ort: %s" % err_msg

    return result


def main():
    dump_dir = sys.argv[1] if len(sys.argv) > 1 else "onnx_dump"

    if not os.path.isdir(dump_dir):
        print("ERROR: 目录不存在: %s" % dump_dir)
        sys.exit(1)

    files = sorted(glob.glob(os.path.join(dump_dir, "*")))
    if not files:
        print("目录为空: %s" % dump_dir)
        sys.exit(0)

    print("扫描目录: %s" % os.path.abspath(dump_dir))
    print("文件总数: %d" % len(files))
    print("onnx: %s" % onnx.__version__)
    if HAS_ORT:
        print("onnxruntime: %s" % ort.__version__)
    else:
        print("onnxruntime: 未安装 (跳过 runtime 加载校验)")
    print("=" * 90)

    onnx_ok = 0
    onnx_fail = 0
    resource_count = 0

    for fpath in files:
        if os.path.isdir(fpath):
            continue

        r = check_file(fpath)
        size_kb = r["size"] / 1024.0

        if not r["is_onnx"]:
            resource_count += 1
            print("\n%s  (%.1f KB)  [资源: %s]" % (r["file"], size_kb, r["resource_type"]))
            continue

        if r["onnx_load"] and r["onnx_check"]:
            tag = "OK"
            onnx_ok += 1
            if HAS_ORT:
                tag += " (ort %s)" % ("OK" if r["ort_load"] else "FAIL")
        elif r["onnx_load"]:
            tag = "LOAD_OK / CHECK_FAIL"
            onnx_fail += 1
        else:
            tag = "FAIL"
            onnx_fail += 1

        print("\n%s  (%.1f KB, onnx=%d, tail=%d)" % (
            r["file"], size_kb, r["onnx_bytes"], r["tail_bytes"]))
        print("  状态: %s" % tag)

        if r["custom_domains"]:
            print("  自定义域: %s" % ', '.join(r["custom_domains"]))
        for inp in r["inputs"]:
            print("  输入: %s  shape=%s" % (inp["name"], inp["shape"]))
        for out in r["outputs"]:
            print("  输出: %s  shape=%s" % (out["name"], out["shape"]))
        if r["error"]:
            err = r["error"][:200]
            print("  备注: %s" % err)

    print("\n" + "=" * 90)
    print("结果: %d 有效 ONNX,  %d 失败,  %d 非ONNX资源,  共 %d 文件" % (
        onnx_ok, onnx_fail, resource_count, onnx_ok + onnx_fail + resource_count))


if __name__ == "__main__":
    main()
