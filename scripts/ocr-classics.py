"""Fetch, verify and browse the frozen, independently sourced OCR classics corpus.

Only Pillow is required. No OCR engine is run and no recognition ground truth is
inferred. Paths in the manifest are relative to its directory.
"""
from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import io
import itertools
import json
from pathlib import Path
import re
import tempfile
import time
import urllib.error
import urllib.request
import zipfile

from PIL import Image, ImageDraw, ImageFont, ImageOps

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST = ROOT / "samples/ocr-classics/manifest.json"
MAX_DOWNLOAD = 64 * 1024 * 1024


def digest(data):
    return hashlib.sha256(data).hexdigest()


def local_path(root, relative):
    path = (root / relative).resolve()
    path.relative_to(root.resolve())
    if Path(relative).is_absolute() or ".." in Path(relative).parts:
        raise ValueError(f"Unsafe corpus path: {relative}")
    return path


def image_metadata(data):
    with Image.open(io.BytesIO(data)) as image:
        image.verify()
    with Image.open(io.BytesIO(data)) as image:
        image.load()
        return {"width": image.width, "height": image.height,
                "format": image.format, "bytes": len(data), "sha256": digest(data)}


def validate_bytes(data, expected, image=True):
    if digest(data) != expected["sha256"]:
        raise ValueError("SHA-256 mismatch")
    if len(data) != expected["bytes"]:
        raise ValueError("Byte length mismatch")
    if image:
        actual = image_metadata(data)
        for key in ("width", "height", "format"):
            if actual[key] != expected[key]:
                raise ValueError(f"Image {key} mismatch: {actual[key]} != {expected[key]}")


def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, suffix=".part", delete=False) as f:
            temporary = Path(f.name)
            f.write(data)
        temporary.replace(path)
    finally:
        if temporary is not None:
            try:
                temporary.unlink()
            except FileNotFoundError:
                pass


class Fetcher:
    def __init__(self, downloads, timeout=25, retries=2):
        self.downloads = downloads
        self.timeout = timeout
        self.retries = retries
        self.archive_cache = {}

    def request(self, url):
        for attempt in range(self.retries + 1):
            try:
                request = urllib.request.Request(url, headers={"User-Agent": "OCR-Classics/1.0"})
                with urllib.request.urlopen(request, timeout=self.timeout) as response:
                    body = response.read(MAX_DOWNLOAD + 1)
                    length = response.headers.get("Content-Length")
                    if length is not None and len(body) != int(length):
                        raise ValueError("Interrupted download: Content-Length mismatch")
                if len(body) > MAX_DOWNLOAD:
                    raise ValueError("Download exceeds 64 MiB")
                return body
            except Exception:
                if attempt == self.retries:
                    raise
                time.sleep(min(2 ** attempt, 3))
        raise RuntimeError("Unreachable")

    def acquire(self, asset):
        if "archive_id" not in asset:
            return self.request(asset["image_url"])
        archive_id = asset["archive_id"]
        if archive_id not in self.archive_cache:
            archive = self.downloads[archive_id]
            body = self.request(archive["url"])
            validate_bytes(body, archive, image=False)
            self.archive_cache[archive_id] = body
        # Read one named member without extracting paths, fonts or other files.
        with zipfile.ZipFile(io.BytesIO(self.archive_cache[archive_id])) as archive:
            info = archive.getinfo(asset["archive_member"])
            if info.file_size > MAX_DOWNLOAD:
                raise ValueError("Archive member exceeds 64 MiB")
            return archive.read(info)

    def ensure(self, asset, root):
        path = local_path(root, asset["file"])
        if path.exists():
            try:
                validate_bytes(path.read_bytes(), asset)
                return "skipped"
            except Exception:
                pass
        body = self.acquire(asset)
        validate_bytes(body, asset)
        # Existing data is replaced only after the entire new payload is valid.
        atomic_write(path, body)
        return "downloaded"


def all_assets(manifest):
    return manifest["cases"] + manifest.get("references", [])


def validate_schema(manifest, root):
    cases = manifest["cases"]
    names, files = set(), set()
    for source in manifest["sources"].values():
        for key in ("title", "author", "article_url", "license"):
            if not source.get(key):
                raise ValueError(f"Missing source field: {key}")
    for asset in all_assets(manifest):
        if asset["name"] in names or asset["file"] in files:
            raise ValueError(f"Duplicate name/path: {asset['name']}")
        names.add(asset["name"])
        files.add(asset["file"])
        local_path(root, asset["file"])
        if asset["source_id"] not in manifest["sources"]:
            raise ValueError(f"Unknown source: {asset['source_id']}")
        if not re.fullmatch(r"[a-f0-9]{64}", asset["sha256"]):
            raise ValueError(f"Invalid SHA-256: {asset['name']}")
        if not asset.get("acquired_on") or not asset.get("image_url"):
            raise ValueError(f"Missing acquisition provenance: {asset['name']}")
        if "archive_id" in asset and asset["archive_id"] not in manifest["downloads"]:
            raise ValueError(f"Unknown archive: {asset['name']}")
        match = re.search(r"raw\.githubusercontent\.com/[^/]+/[^/]+/([^/]+)/", asset["image_url"])
        if match and not re.fullmatch(r"[a-f0-9]{40}", match[1]):
            raise ValueError(f"GitHub asset is not pinned: {asset['name']}")
    reference_names = {x["name"] for x in manifest.get("references", [])}
    for case in cases:
        if not case["file"].startswith("images/"):
            raise ValueError("Input images must live in images/")
        for key in ("title", "tags", "source", "challenge", "approach", "why_classic", "article_result"):
            if not case.get(key):
                raise ValueError(f"Missing case field {key}: {case['name']}")
        if case["source"] != manifest["sources"][case["source_id"]]["article_url"]:
            raise ValueError(f"Case source URL mismatch: {case['name']}")
        if case["group"] not in manifest["groups"]:
            raise ValueError(f"Unknown group: {case['group']}")
        if case["annotation_status"] != "unannotated" or case["visual_review"]["status"] != "reviewed_by_agent":
            raise ValueError(f"Unexpected annotation/review state: {case['name']}")
        if not set(case.get("references", [])).issubset(reference_names):
            raise ValueError(f"Unknown reference: {case['name']}")
    for ref in manifest.get("references", []):
        if not ref["file"].startswith("references/") or not ref.get("role") or not ref.get("caption"):
            raise ValueError(f"Invalid reference: {ref['name']}")
    counts = Counter(x["source_id"] for x in cases)
    chinese = sum("zh" in x["languages"] for x in cases)
    requirements = manifest["acceptance"]
    if len(cases) != requirements["input_count"]:
        raise ValueError("Input count does not meet the frozen selection")
    if len(counts) < requirements["min_input_sources"] or chinese < requirements["min_chinese_inputs"]:
        raise ValueError("Source/Chinese coverage does not meet acceptance")
    if max(counts.values()) > requirements["max_inputs_per_source"]:
        raise ValueError("Too many inputs from one article")


def fingerprint(path):
    with Image.open(path) as image:
        image = ImageOps.exif_transpose(image).convert("L").resize((17, 16), Image.Resampling.LANCZOS)
        pixels = list(image.getdata())
        return sum((pixels[y * 17 + x] > pixels[y * 17 + x + 1]) << (y * 16 + x)
                   for y in range(16) for x in range(16))


def verify(manifest, root):
    errors, suspects = [], []
    try:
        validate_schema(manifest, root)
    except Exception as exc:
        return {"passed": False, "errors": [str(exc)], "near_duplicates": []}
    hashes, fingerprints = {}, {}
    for asset in all_assets(manifest):
        try:
            path = local_path(root, asset["file"])
            validate_bytes(path.read_bytes(), asset)
            if asset in manifest["cases"]:
                if asset["sha256"] in hashes:
                    errors.append(f"Duplicate input: {asset['name']} / {hashes[asset['sha256']]}")
                hashes[asset["sha256"]] = asset["name"]
                fingerprints[asset["name"]] = fingerprint(path)
        except Exception as exc:
            errors.append(f"{asset['name']}: {exc}")
    for a, b in itertools.combinations(manifest["cases"], 2):
        if a["name"] not in fingerprints or b["name"] not in fingerprints:
            continue
        ratio_a, ratio_b = a["width"] / a["height"], b["width"] / b["height"]
        distance = bin(fingerprints[a["name"]] ^ fingerprints[b["name"]]).count("1")
        if 0.9 < ratio_a / ratio_b < 1.1 and distance <= 10:
            suspects.append({"a": a["name"], "b": b["name"], "dhash_distance_256": distance})
    if suspects:
        errors.append("Near-duplicate candidates need visual review")
    return {"passed": not errors, "inputs": len(manifest["cases"]),
            "chinese_or_mixed": sum("zh" in x["languages"] for x in manifest["cases"]),
            "input_sources": len({x["source_id"] for x in manifest["cases"]}),
            "references": len(manifest.get("references", [])),
            "errors": errors, "near_duplicates": suspects}


def load_font(size):
    for font in ("C:/Windows/Fonts/msyh.ttc", "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
                 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"):
        if Path(font).exists():
            return ImageFont.truetype(font, size)
    return ImageFont.load_default()


def thumbnail(path, size):
    with Image.open(path) as image:
        # EXIF orientation is applied only to previews; original bytes are untouched.
        return ImageOps.contain(ImageOps.exif_transpose(image).convert("RGB"), size, Image.Resampling.LANCZOS)


def contact_sheet(cases, root, title, output):
    width, height, columns = 400, 276, 3
    sheet = Image.new("RGB", (width * columns, 65 + height * ((len(cases) + columns - 1) // columns)), "#eef2f6")
    draw = ImageDraw.Draw(sheet)
    draw.text((20, 15), title, font=load_font(25), fill="#15243b")
    for i, case in enumerate(cases):
        x, y = (i % columns) * width, 65 + (i // columns) * height
        draw.rounded_rectangle((x + 7, y + 3, x + width - 7, y + height - 8), radius=8, fill="white")
        preview = thumbnail(local_path(root, case["file"]), (375, 210))
        sheet.paste(preview, (x + (width - preview.width) // 2, y + 9 + (210 - preview.height) // 2))
        draw.text((x + 16, y + 223), f"{case['number']:02d}  {case['title']}", font=load_font(17), fill="#15243b")
        draw.text((x + 16, y + 247), case["name"], font=load_font(12), fill="#5e6c7c")
    output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(output, quality=90)


def build(manifest, root):
    result = verify(manifest, root)
    if not result["passed"]:
        raise ValueError(json.dumps(result, ensure_ascii=False))
    previews = root / "thumbnails"
    previews.mkdir(parents=True, exist_ok=True)
    for case in manifest["cases"]:
        thumbnail(local_path(root, case["file"]), (360, 230)).save(previews / (case["name"] + ".jpg"), quality=90)
    contact_sheet(manifest["cases"], root, "传统 OCR 经典案例 · 30 张独立输入", previews / "overview.jpg")
    lines = ["# 传统 OCR 经典案例样本库", "",
             f"30 张独立输入图 · {result['chinese_or_mixed']} 张中文及中英混排 · {result['input_sources']} 篇输入来源 · {result['references']} 张辅助图。", "",
             "用于本地研究的精选案例集，覆盖 OpenCV、Tesseract、PaddleOCR、EasyOCR 和 CnOCR；未运行引擎评测，所有样本均未精标。", "",
             "输入原图位于 `images/`，保留下载或官方压缩包中的原始字节与分辨率。`references/` 中的原文结果、处理对照和拼图仅作参考，不计入输入数量，不充当文字真值。", "",
             "![30 张输入总览](thumbnails/overview.jpg)", "",
             "## 使用与重建", "", "在仓库根目录执行（Python 3.7+，依赖 Pillow 9.1+）：", "", "```powershell",
             "python scripts/ocr-classics.py verify", "python scripts/ocr-classics.py fetch",
             "python scripts/ocr-classics.py build", "python scripts/ocr-classics.py check-links", "```", "",
             "`fetch` 按固定清单获取缺失或损坏的材料，校验通过后才替换文件；重复运行跳过已校验文件。失败返回非零状态，不会以失败材料生成新索引。`verify` 可离线检查输入、辅助图、数量和重复候选。", "",
             "可用 `--output-dir <目录>` 在新目录重建；`--manifest <文件>` 指定清单。下载可设置 `--timeout`、`--retries`，并用 `--report <文件>` 保存结果。", "",
             "`manifest.json` 是固定清单：保留 `cases/name/file/tags/source` 字段；`sources` 记录文章和许可，`downloads` 锁定官方压缩包，图片记录校验值、尺寸、直链及解读。GitHub 素材固定到提交。", "",
             "## 筛选与审阅", "",
             "每张输入均经助手视觉检查；`visual_review` 明确记录为 `reviewed_by_agent`，不冒充人工精标。文字方向、表格线、商品设计及科学论文插图属于原图内容；原文绘制的 OCR 框、识别答案或处理拼图不作为独立输入。", "",
             "明显的同图旋转、裁剪和处理版本不重复计数。近似重复采用 256 位 dHash 筛查并结合视觉审阅；该检查不能保证发现所有变形副本。", "",
             "来源仓库的代码许可与其中第三方图片的权利分别记录；未明确找到素材许可的项目标为未单独声明，不套用本仓库 MIT 许可。", "",
             "## 分组案例", ""]
    refs = {x["name"]: x for x in manifest.get("references", [])}
    for group, group_title in manifest["groups"].items():
        cases = [x for x in manifest["cases"] if x["group"] == group]
        if not cases:
            continue
        contact_sheet(cases, root, group_title, previews / (group + ".jpg"))
        lines += [f"### {group_title}", "", f"![{group_title}](thumbnails/{group}.jpg)", ""]
        for case in cases:
            source = manifest["sources"][case["source_id"]]
            lines += [f"#### {case['number']:02d} · {case['title']}", "",
                      f"[![{case['title']}](thumbnails/{case['name']}.jpg)]({case['file']})", "",
                      f"[输入原图]({case['file']}) · {case['width']} × {case['height']} · [原文：{source['title']}]({source['article_url']})", "",
                      f"- 难点：{case['challenge']}", f"- 原文方案：{case['approach']}",
                      f"- 收录理由：{case['why_classic']}", f"- 原文结果（非真值）：{case['article_result']}",
                      f"- 标签：{'、'.join(case['tags'])}；标注：未精标。", ""]
            for name in case.get("references", []):
                ref = refs[name]
                lines += [f"辅助材料：[ {ref['caption']} ]({ref['file']})（{ref['role']}，不计入输入）。", ""]
    lines += ["## 来源与许可", "", "| 来源 | 作者 | 输入数 | 已查到的许可信息 |", "|---|---|---:|---|"]
    counts = Counter(x["source_id"] for x in manifest["cases"])
    for key, source in manifest["sources"].items():
        license_info = source["license"]
        lines.append(f"| [{source['title']}]({source['article_url']}) | {source['author']} | {counts[key]} | {license_info['note']} |")
    lines += ["", "GitHub 原文快照、素材地址、素材获取日期及许可证据地址详见 `manifest.json`。", "",
              "## 补充辅助材料", ""]
    linked = {name for case in manifest["cases"] for name in case.get("references", [])}
    for ref in manifest.get("references", []):
        if ref["name"] not in linked:
            source = manifest["sources"][ref["source_id"]]
            lines += [f"- [{ref['caption']}]({ref['file']})：{ref['role']}；[原文]({source['article_url']})。"]
    lines += ["", "小票扫描教程公开配图为处理拼图，作为补充辅助材料保存；独立小票输入取自 PSM 教程。二者不作为独立重复样本计数。", ""]
    atomic_write(root / "README.md", "\n".join(lines).encode("utf-8"))
    # A reconstruction directory should also contain its source of truth.
    atomic_write(root / "manifest.json", (json.dumps(manifest, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
    return result


def check_links(manifest, timeout):
    urls = {x["image_url"] for x in all_assets(manifest)}
    for source in manifest["sources"].values():
        urls.add(source["article_url"])
        for key in ("snapshot_url",):
            if source.get(key):
                urls.add(source[key])
        if source["license"].get("evidence_url"):
            urls.add(source["license"]["evidence_url"])

    def check(url):
        try:
            request = urllib.request.Request(url, method="HEAD", headers={"User-Agent": "Mozilla/5.0"})
            try:
                response = urllib.request.urlopen(request, timeout=timeout)
            except urllib.error.HTTPError as exc:
                if exc.code not in (403, 405, 501):
                    raise
                response = urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"}), timeout=timeout)
            with response:
                return {"url": url, "status": response.status, "ok": 200 <= response.status < 300}
        except Exception as exc:
            return {"url": url, "ok": False, "error": str(exc)}

    with ThreadPoolExecutor(max_workers=6) as pool:
        results = list(pool.map(check, sorted(urls)))
    return {"passed": all(x["ok"] for x in results), "checked": len(results), "links": results}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("fetch", "verify", "build", "check-links"))
    parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--timeout", type=float, default=25)
    parser.add_argument("--retries", type=int, default=2)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    if args.timeout <= 0 or args.retries < 0:
        parser.error("timeout must be positive and retries must be nonnegative")
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    root = (args.output_dir or args.manifest.parent).resolve()
    try:
        validate_schema(manifest, root)
        if args.command == "fetch":
            fetcher = Fetcher(manifest.get("downloads", {}), args.timeout, args.retries)
            assets = []
            for asset in all_assets(manifest):
                try:
                    status = fetcher.ensure(asset, root)
                    assets.append({"name": asset["name"], "status": status})
                    print(f"{status}: {asset['name']}", flush=True)
                except Exception as exc:
                    assets.append({"name": asset["name"], "status": "failed", "error": str(exc)})
                    print(f"FAILED: {asset['name']}: {exc}", flush=True)
            result = {"passed": all(x["status"] != "failed" for x in assets),
                      "counts": dict(Counter(x["status"] for x in assets)), "assets": assets}
            if result["passed"]:
                result["verification"] = build(manifest, root)
        elif args.command == "verify":
            result = verify(manifest, root)
        elif args.command == "build":
            result = build(manifest, root)
        else:
            result = check_links(manifest, args.timeout)
    except Exception as exc:
        result = {"passed": False, "error": str(exc)}
    if args.report:
        atomic_write(args.report.resolve(), (json.dumps(result, ensure_ascii=False, indent=2) + "\n").encode("utf-8"))
    print(json.dumps(result, ensure_ascii=False, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
