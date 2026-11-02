"""Build a self-contained illustrated HTML report from measured OCR and review.json.

This renderer never runs OCR, changes raw results, or assigns quality ratings.
"""
import argparse
import base64
from collections import Counter
import hashlib
import html
import io
import json
import math
from pathlib import Path
import statistics

from PIL import Image, ImageDraw, ImageFont, ImageOps

ROOT = Path(__file__).resolve().parents[1]
CORPUS = ROOT / 'samples/ocr-classics'
DEFAULT_REPORT = ROOT / 'reports/oneocr-classics-2026-11-02'
LABELS = {'good': '较好', 'mixed': '部分可用', 'problem': '明显问题', 'failed': '执行失败'}
COLORS = {'good': '#087f64', 'mixed': '#af720c', 'problem': '#c34242', 'failed': '#655085'}
LEVELS = {'failed': -1, 'problem': 0, 'mixed': 1, 'good': 2}


def read(path):
    return json.loads(path.read_text(encoding='utf-8'))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def esc(value):
    return html.escape(str(value), quote=True)


def font(size, latin=False):
    path = 'C:/Windows/Fonts/arial.ttf' if latin else 'C:/Windows/Fonts/msyh.ttc'
    if Path(path).exists():
        return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def picture(source, result=None, color='#dc4444'):
    # Engine decoders ignore EXIF. Keep that exact pixel coordinate system in
    # report previews too, including the sideways cooking page (EXIF=6).
    with Image.open(source) as image:
        original_width, original_height = image.size
        preview = ImageOps.contain(image.convert('RGB'), (1200, 1050), Image.Resampling.LANCZOS)
    if result is not None:
        scale = preview.width / original_width
        draw = ImageDraw.Draw(preview)
        label_font = font(11, latin=True)
        for index, line in enumerate(result.get('lines', [])):
            box = line.get('bounding_box')
            if not isinstance(box, list) or len(box) != 8 or not all(isinstance(v, (int, float)) and math.isfinite(v) for v in box):
                continue
            points = [(box[i] * scale, box[i + 1] * preview.height / original_height) for i in range(0, 8, 2)]
            draw.line(points + [points[0]], fill=color, width=2)
            label = 'L' + str(index)
            x = max(0, min(preview.width - 28, min(p[0] for p in points)))
            y = max(0, min(preview.height - 13, min(p[1] for p in points) - 12))
            draw.rectangle((x, y, x + draw.textlength(label, font=label_font) + 3, y + 12), fill=color)
            draw.text((x + 1, y - 1), label, font=label_font, fill='white')
    return preview


def image_data(image):
    out = io.BytesIO()
    image.save(out, format='JPEG', quality=88)
    return 'data:image/jpeg;base64,' + base64.b64encode(out.getvalue()).decode('ascii')


def wrap(draw, text, maximum, selected_font):
    lines, line = [], ''
    for char in text:
        if char == '\n':
            lines.append(line)
            line = ''
        elif line and draw.textlength(line + char, font=selected_font) > maximum:
            lines.append(line)
            line = char
        else:
            line += char
    if line:
        lines.append(line)
    return lines


def summary_image(folder, cases, review, counts):
    canvas = Image.new('RGB', (1440, 1170), '#edf2f7')
    draw = ImageDraw.Draw(canvas)
    draw.text((32, 22), '30 张经典 OCR 样本 · OneOCR 实测', font=font(34), fill='#17304b')
    draw.text((33, 75), '当前默认原生版本为主测，原版 DLL 同图参照  |  2026-11-02', font=font(20), fill='#57677b')
    x = 34
    for key in ('good', 'mixed', 'problem'):
        title = f"{LABELS[key]}  {counts['native'].get(key, 0)} 张"
        draw.rounded_rectangle((x, 116, x + 325, 170), radius=9, fill=COLORS[key])
        draw.text((x + 18, 125), title, font=font(25), fill='white')
        x += 350
    examples = [
        ('cn-wrapped-paragraph', '干净中文段落：较好', '7 行正文整体连贯，主要文字完整。'),
        ('easy-yuyuan-road', '中文路名：主名称错误', '原图：愚园路\n原生：馬园路\nDLL：愚园路'),
        ('digits-thermostat', '数码管：核心读数错误', '原图：34.5 °C\n原生：3450\nDLL：345'),
        ('dewarp-cooking-page', '弯曲书页：阅读顺序错乱', '英文多数可读，但原生输出把段首、段尾和标题穿插在一起。'),
        ('cn-single-line-mixed', '极矮单行：重复与执行失败', '原图主测重复、DLL 返回 -18；诊断加白边后，两版均读对。'),
        ('pp-lab-table', '表格：数字与列关系问题', '原生：58.9 → 58.；表头乱序。DLL 更完整，仍未恢复单元格。')
    ]
    by_name = {x['name']: x for x in cases}
    for i, (name, title, text) in enumerate(examples):
        x, y = 25 + (i % 3) * 470, 200 + (i // 3) * 440
        draw.rounded_rectangle((x, y, x + 452, y + 420), radius=12, fill='white')
        case = by_name[name]
        with Image.open(CORPUS / case['file']) as image:
            image = image.convert('RGB')
            if name == 'digits-thermostat':
                image = image.crop((220, 190, 530, 410))
            elif name == 'dewarp-cooking-page':
                image = image.rotate(270, expand=True)
            image = ImageOps.contain(image, (420, 224), Image.Resampling.LANCZOS)
            canvas.paste(image, (x + (452 - image.width) // 2, y + 12 + (224 - image.height) // 2))
        selected_font = font(21)
        for j, line in enumerate(wrap(draw, title, 420, selected_font)):
            draw.text((x + 16, y + 247 + j * 27), line, font=selected_font, fill='#17304b')
        for j, line in enumerate(wrap(draw, text, 418, font(18))):
            draw.text((x + 16, y + 305 + j * 25), line, font=font(18), fill='#4f6072')
    draw.text((32, 1104), '定性审阅，不是字符准确率。主测 30/30 获得结果；DLL 29/30，另 1 张执行失败。',
              font=font(20), fill='#43566d')
    draw.text((32, 1136), '完整报告包含 30 张原图、两版行框、全部识别文本、逐例证据与运行指纹。',
              font=font(18), fill='#657487')
    canvas.save(folder / 'summary.jpg', quality=93)


CSS = r'''
:root{--ink:#182e48;--muted:#62748a;--bg:#edf2f7;--line:#dbe3ed;--good:#087f64;--mixed:#af720c;--problem:#c34242;--failed:#655085}
*{box-sizing:border-box}html{scroll-behavior:smooth}body{margin:0;background:var(--bg);color:var(--ink);font:15px/1.65 "Segoe UI","Microsoft YaHei",sans-serif}
main{max-width:1360px;margin:auto;padding:32px 28px 70px}a{color:#225da6}h1{font-size:35px;line-height:1.3;margin:8px 0 16px}h2{font-size:23px;margin:28px 0 15px}h3{font-size:17px;margin:12px 0}p{margin:8px 0 14px}.eyebrow{font-size:13px;letter-spacing:2px;color:var(--muted)}.lead{max-width:1000px;font-size:17px}
.panel{background:white;border:1px solid var(--line);border-radius:14px;padding:24px;margin:18px 0}.muted{color:var(--muted)}.small{font-size:13px}.stats{display:grid;grid-template-columns:repeat(4,1fr);gap:14px;margin:22px 0}.stat{padding:18px;border-radius:12px;background:white;border:1px solid var(--line)}.stat strong{display:block;font-size:29px;line-height:1.3}.stat span{color:var(--muted);font-size:13px}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:22px}.rating-bars{display:flex;gap:8px;flex-wrap:wrap}.badge{border-radius:30px;padding:3px 10px;background:#f0f3f7;white-space:nowrap;font-size:13px;font-weight:600}.good{color:var(--good);background:#e6f5ef}.mixed{color:var(--mixed);background:#fff3da}.problem{color:var(--problem);background:#ffeded}.failed{color:var(--failed);background:#eee9f6}
.hero-image{width:100%;max-width:1060px;display:block;margin:auto;border-radius:10px}.notice{border-left:4px solid #b27a21;background:#fff7e7;padding:12px 17px;border-radius:5px}.findings{padding-left:23px}.findings li{margin:10px 0}.table-scroll{overflow:auto}table{border-collapse:collapse;width:100%;font-size:14px}th,td{border-bottom:1px solid var(--line);padding:10px 11px;text-align:left;vertical-align:top}th{background:#f5f8fb;color:#536980}.evidence th:first-child{width:12%}.evidence td{min-width:105px}.evidence td:last-child{min-width:190px}
.controls{position:sticky;top:0;z-index:5;background:#eef2f7ee;backdrop-filter:blur(10px);border:1px solid var(--line);border-radius:10px;padding:12px;display:flex;flex-wrap:wrap;align-items:center;gap:8px;margin:25px 0 15px}.controls button,.controls input{border:1px solid #cbd7e5;border-radius:7px;background:white;color:var(--ink);font:inherit;padding:7px 11px}.controls button{cursor:pointer}.controls button.active{color:white;background:#244c78;border-color:#244c78}.controls input{min-width:220px;flex:1}button:hover{filter:brightness(.97)}
.case{background:white;border:1px solid var(--line);border-radius:12px;margin:13px 0;overflow:hidden;scroll-margin-top:110px}.case>summary{cursor:pointer;list-style:none;display:flex;align-items:center;gap:16px;padding:16px 20px}.case>summary::-webkit-details-marker{display:none}.case>summary:after{content:'＋';font-size:22px;color:#7890a8;margin-left:auto}.case[open]>summary:after{content:'－'}.case>summary img{width:86px;height:62px;object-fit:contain;background:#f2f4f7;border-radius:5px}.case-title{font-size:17px;font-weight:650}.case-brief{color:var(--muted);font-size:13px;max-width:840px}.case-body{border-top:1px solid var(--line);padding:22px}.case-meta{display:flex;gap:14px;flex-wrap:wrap;color:var(--muted);font-size:13px;margin-bottom:15px}.figures{display:grid;grid-template-columns:repeat(3,1fr);gap:14px;margin:18px 0}.figures figure{margin:0;border:1px solid var(--line);border-radius:9px;padding:12px;min-width:0;background:#fafcfe}.figures img{display:block;width:100%;height:270px;object-fit:contain;cursor:zoom-in;background:white}.figures figcaption{font-size:13px;color:var(--muted);padding:10px 0 2px}.failed-figure{height:270px;display:flex;align-items:center;justify-content:center;text-align:center;background:#f1edf8;color:var(--failed);border-radius:6px}.engine-note{border-radius:9px;background:#f5f8fb;padding:14px;margin-bottom:14px}.engine-note strong{display:block;margin-bottom:6px}.text-panes{display:grid;grid-template-columns:1fr 1fr;gap:18px;margin-top:18px}.text-panes details{border:1px solid var(--line);border-radius:9px;overflow:hidden}.text-panes summary{padding:12px;background:#f4f7fb;cursor:pointer;font-weight:600}.ocr{max-height:480px;overflow:auto;background:#fcfdff;padding:12px 15px;font:13px/1.8 Consolas,"Microsoft YaHei",monospace;white-space:pre-wrap;overflow-wrap:anywhere;margin:0}.error-log{max-height:230px}.links{font-size:13px;margin-top:10px}.link-table td{font:12px/1.5 Consolas,monospace;word-break:break-all}.link-table th{width:150px}.hidden{display:none!important}.zoom{border:0;border-radius:12px;padding:16px;max-width:96vw;max-height:96vh;box-shadow:0 20px 80px #122b4c88}.zoom::backdrop{background:#14283dcc}.zoom img{max-width:90vw;max-height:84vh;display:block}.zoom button{float:right;background:#18344f;color:white;border:0;padding:6px 12px;border-radius:6px;cursor:pointer}.footer{color:var(--muted);font-size:13px;margin:30px 0}
@media(max-width:900px){main{padding:20px 13px}h1{font-size:27px}.grid2,.text-panes{grid-template-columns:1fr}.stats{grid-template-columns:repeat(2,1fr)}.figures{grid-template-columns:1fr}.figures img{height:320px}.panel,.case-body{padding:16px}.case>summary{padding:12px;gap:9px}.case>summary img{width:55px;height:45px}.case-title{font-size:15px}.controls{top:0}.case-brief{font-size:12px}}
@media print{body{background:white}main{max-width:none;padding:0}.controls,.zoom,.case>summary:after,.no-print{display:none!important}.case{break-inside:auto}.case>summary{break-after:avoid}.case-body,.panel{padding:12px}.figures{grid-template-columns:repeat(3,1fr)}.figures img{height:180px}.text-panes{grid-template-columns:1fr 1fr}.ocr{max-height:none;font-size:10px}.panel{break-inside:avoid}.case:not([open]) .case-body{display:block}}
'''

JS = r'''
const cards=[...document.querySelectorAll('.case')];let mode='all';
function applyFilter(){const term=document.querySelector('#search').value.trim().toLowerCase();let count=0;cards.forEach(card=>{const match=(mode==='all'||(mode==='better'?card.dataset.better==='true':card.dataset.rating===mode))&&card.dataset.search.toLowerCase().includes(term);card.classList.toggle('hidden',!match);if(match)count++});document.querySelector('#shown').textContent=`显示 ${count} / ${cards.length}`}
document.querySelectorAll('[data-filter]').forEach(button=>button.addEventListener('click',()=>{mode=button.dataset.filter;document.querySelectorAll('[data-filter]').forEach(b=>b.classList.toggle('active',b===button));applyFilter()}));document.querySelector('#search').addEventListener('input',applyFilter);
document.querySelector('#expand').addEventListener('click',()=>cards.filter(c=>!c.classList.contains('hidden')).forEach(c=>c.open=true));document.querySelector('#collapse').addEventListener('click',()=>cards.forEach(c=>c.open=false));
document.querySelectorAll('[data-case-link]').forEach(a=>a.addEventListener('click',()=>{mode='all';document.querySelector('#search').value='';document.querySelectorAll('[data-filter]').forEach(b=>b.classList.toggle('active',b.dataset.filter==='all'));applyFilter();document.querySelector(a.getAttribute('href')).open=true}));
const dialog=document.querySelector('#zoom');document.querySelectorAll('.figures img').forEach(img=>img.addEventListener('click',()=>{dialog.querySelector('img').src=img.src;dialog.querySelector('img').alt=img.alt;dialog.showModal()}));dialog.querySelector('button').addEventListener('click',()=>dialog.close());dialog.addEventListener('click',e=>{if(e.target===dialog)dialog.close()});
window.addEventListener('beforeprint',()=>{cards.forEach(c=>c.open=true);document.querySelectorAll('.text-panes details').forEach(d=>d.open=true)});
'''


def badge(key):
    return f'<span class="badge {key}">{LABELS[key]}</span>'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report-dir', type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()
    folder = args.report_dir.resolve()
    run = read(folder / 'run.json')
    review = read(folder / 'review.json')
    snapshot = folder / 'corpus-manifest-at-run.json'
    assert sha(snapshot) == run['fingerprints']['corpus_manifest']['sha256'], 'Run manifest snapshot changed'
    assert sha(folder / 'runner-at-run.py') == run['fingerprints']['runner']['sha256'], 'Runner snapshot changed'
    manifest = read(snapshot)
    cases = manifest['cases']
    for case in cases:
        if case['name'] == 'quality-noisy-fraktur':
            case['title'] = '旧希腊文扫描：噪声与密集字形'
        assert sha(CORPUS / case['file']) == case['sha256'], case['name']
    assert set(review['cases']) == {x['name'] for x in cases}
    measures = {(x['name'], x['backend']): x for x in run['measurements']}
    assert len(measures) == 60 and len(cases) == 30
    counts = {backend: dict(Counter(review['cases'][c['name']][backend]['rating'] for c in cases))
              for backend in ('native', 'legacy')}
    summary_image(folder, cases, review, counts)
    figure_dir = folder / 'figures'
    figure_dir.mkdir(exist_ok=True)
    images, data = {}, {}
    for case in cases:
        name = case['name']
        source = CORPUS / case['file']
        image = picture(source)
        image.save(figure_dir / (name + '-input.jpg'), quality=90)
        images[(name, 'input')] = image_data(image)
        for backend, color in (('native', '#dc4444'), ('legacy', '#2475b6')):
            measurement = measures[(name, backend)]
            rating = review['cases'][name][backend]['rating']
            if measurement['status'] != 'success':
                assert rating == 'failed'
                data[(name, backend)] = None
                continue
            assert rating != 'failed'
            path = folder / backend / (name + '.json')
            assert sha(path) == measurement['result_sha256'], path
            result = read(path)
            data[(name, backend)] = result
            overlay = picture(source, result, color)
            overlay.save(figure_dir / (name + '-' + backend + '.jpg'), quality=90)
            images[(name, backend)] = image_data(overlay)
    native_success = sum(x['status'] == 'success' for x in run['measurements'] if x['backend'] == 'native')
    legacy_success = sum(x['status'] == 'success' for x in run['measurements'] if x['backend'] == 'legacy')
    better = [c['name'] for c in cases if LEVELS[review['cases'][c['name']]['legacy']['rating']] > LEVELS[review['cases'][c['name']]['native']['rating']]]
    out = ['<!doctype html><html lang="zh-CN"><head><meta charset="utf-8">',
           '<meta name="viewport" content="width=device-width, initial-scale=1"><title>OneOCR · 30 张经典样本实测报告</title>',
           '<style>' + CSS + '</style></head><body><main>',
           '<div class="eyebrow">ONEOCR / CLASSIC CORPUS / 2026-11-02</div>',
           '<h1>哪些能直接读，哪些还需要复核？</h1>',
           '<p class="lead">30 张真实输入图的逐例图文报告。以项目当前默认的原生 OneOCR 为主测，同时运行原版 Windows DLL 作参照；所有主测保留原图输入，没有外加旋转、补边或展平。</p>',
           '<div class="stats">',
           f'<div class="stat"><strong>{native_success}/30</strong><span>当前原生 · 执行得到结果</span></div>',
           f'<div class="stat"><strong>{counts["native"].get("good", 0)} 张</strong><span>主要内容识别较好</span></div>',
           f'<div class="stat"><strong>{counts["native"].get("problem", 0)} 张</strong><span>存在明显问题</span></div>',
           f'<div class="stat"><strong>{legacy_success}/30</strong><span>原 DLL · 执行得到结果</span></div></div>',
           '<div class="notice">以上是这批样本的定性审阅数量，不是字符准确率。样本库没有整图精标；“较好”也可能含局部错误。评级依据原图及具体输出，不以置信度或与 DLL 一致程度自动判分。</div>',
           '<section class="panel"><h2>先看最有代表性的结果</h2>',
           '<img class="hero-image" alt="六个代表案例与主测质量分布" src="' + image_data(Image.open(folder / 'summary.jpg')) + '">',
           '<ol class="findings"><li><strong>干净段落、大字和单词是亮点。</strong>中文段落、商品核心信息、Designer、THE OLD ENGINE. 基本可读。卡片的主卡号也完整读出，但小字仍有明显错误。</li>',
           '<li><strong>定位与排序是当前原生版本的重要短板。</strong>繁体段落、弯曲书页、词表、检验表和小票中，许多文字已被读出，却以错误的顺序或过碎的片段返回。</li>',
           '<li><strong>主字段错误必须单独关注。</strong>路名“愚园路”变成“馬园路”；液晶“34.5 °C”变成“3450”；检验表“58.9”变成“58.”。这些不能被总体可读性掩盖。</li>',
           '<li><strong>原 DLL 参照能帮助定位差距。</strong>同一张路牌它读对了“愚园路”，食谱页排序也更连贯；但数码管小数点、细字和表格结构仍有问题。</li></ol></section>',
           '<section class="panel"><h2>整体分布与评价口径</h2><div class="grid2">']
    for backend, title in (('native', '当前默认原生版本'), ('legacy', '原版 DLL 参照')):
        out.append(f'<div><h3>{title}</h3><div class="rating-bars">')
        for key in LABELS:
            out.append(f'<span class="badge {key}">{LABELS[key]} {counts[backend].get(key,0)} 张</span>')
        out.append('</div></div>')
    out.append('</div><ul>')
    for description in review['ratings'].values():
        out.append('<li>' + esc(description) + '</li>')
    out += ['</ul><p class="muted small">检查维度：主要文字、显著漏读/重复、阅读顺序、字段对应及运行状态。局部转录是审阅证据，不是整图 ground truth；没有计算 CER/WER。原生 CTC 置信度尚未校准，不与 DLL 置信度作数值比较。</p></section>',
            '<section class="panel"><h2>30 张样本一览</h2><div class="table-scroll"><table><thead><tr><th>编号 / 样本</th><th>当前原生</th><th>原 DLL</th><th>主要观察</th></tr></thead><tbody>']
    for case in cases:
        item = review['cases'][case['name']]
        out.append(f'<tr><td><a data-case-link href="#case-{case["number"]}">{case["number"]:02d} · {esc(case["title"])}</a></td>'
                   f'<td>{badge(item["native"]["rating"])}</td><td>{badge(item["legacy"]["rating"])}</td><td>{esc(item["native"]["verdict"])}</td></tr>')
    out += ['</tbody></table></div></section>',
            '<h2>逐张对照：原图、行框与全部文本</h2><p class="muted small">红框＝当前原生，蓝框＝原 DLL。L0、L1… 对应原始 JSON 的零起始行号及下方文本编号；图可点击放大。这里的预览不应用 EXIF 旋转，以保持与引擎解码及框坐标一致。</p>',
            '<div class="controls"><button class="active" data-filter="all">全部 30</button>']
    for key in ('problem', 'mixed', 'good'):
        out.append(f'<button data-filter="{key}">{LABELS[key]} {counts["native"].get(key,0)}</button>')
    out += [f'<button data-filter="better">DLL 参照评级更好 {len(better)}</button>',
            '<input id="search" aria-label="搜索样本" placeholder="搜索标题、问题、标签…"><button id="expand">展开全部</button><button id="collapse">收起全部</button><span id="shown" class="small muted">显示 30 / 30</span></div>']
    for case in cases:
        name, number = case['name'], case['number']
        item = review['cases'][name]
        search = ' '.join([case['title'], name, item['native']['verdict'], item['legacy']['verdict']] + item['tags'])
        out.append(f'<details class="case" id="case-{number}" data-rating="{item["native"]["rating"]}" data-better="{str(name in better).lower()}" data-search="{esc(search)}">')
        out.append(f'<summary><img alt="{esc(case["title"])}的缩略图" src="{images[(name,"input")]}"><div><div class="case-title">{number:02d} · {esc(case["title"])} {badge(item["native"]["rating"])}</div><div class="case-brief">{esc(item["native"]["verdict"])}</div></div></summary>')
        out += ['<div class="case-body">', f'<div class="case-meta"><span>{case["width"]} × {case["height"]} · {esc(name)}</span><span>{esc(" / ".join(item["tags"]))}</span>'
                f'<a href="{esc(case["source"])}" target="_blank" rel="noopener">样本来源原文</a></div>', '<div class="figures">',
                f'<figure><img alt="{esc(case["title"])}原始输入像素预览" src="{images[(name,"input")]}"><figcaption>原图预览 · 原始文件未改动</figcaption></figure>']
        for backend, label in (('native', '当前原生行框'), ('legacy', '原 DLL 行框')):
            record = measures[(name, backend)]
            out.append('<figure>')
            if data[(name, backend)] is not None:
                out.append(f'<img alt="{esc(case["title"])}{label}" src="{images[(name,backend)]}">')
                out.append(f'<figcaption>{label} · {record["lines"]} 行 · {record["seconds"]:.3f}s（含启动）</figcaption>')
            else:
                out.append('<div class="failed-figure">原 DLL 返回 -18<br>未得到 OCR 结果</div><figcaption>执行失败 · 原图结果保留为空</figcaption>')
            out.append('</figure>')
        out += ['</div><div class="grid2">']
        for backend, label in (('native', '当前原生'), ('legacy', '原 DLL 参照')):
            out.append(f'<div class="engine-note"><strong>{label} {badge(item[backend]["rating"])}</strong>{esc(item[backend]["verdict"])}</div>')
        out.append('</div><h3>具体证据</h3><div class="table-scroll"><table class="evidence"><thead><tr><th>核对位置</th><th>原图 / 核对依据</th><th>当前原生</th><th>原 DLL</th><th>判断说明</th></tr></thead><tbody>')
        for row in item['evidence']:
            out.append('<tr>' + ''.join('<td>' + esc(row[k]) + '</td>' for k in ('region','original','native','legacy','note')) + '</tr>')
        out += ['</tbody></table></div><div class="text-panes">']
        for backend, label in (('native', '当前原生'), ('legacy', '原 DLL 参照')):
            result = data[(name, backend)]
            out.append(f'<details><summary>{label} · 展开全部识别文本</summary>')
            if result is not None:
                text = '\n'.join(f'L{i:02d}  {line.get("text", "")}' for i,line in enumerate(result['lines']))
                out.append('<pre class="ocr">' + esc(text) + '</pre>')
                out.append(f'<p class="links"><a href="{backend}/{name}.json">原始 JSON</a> · <a href="{backend}/{name}.txt">纯文本</a> · 检测角度 {result.get("image_angle", 0):.3f}°</p>')
            else:
                record = measures[(name, backend)]
                out.append('<pre class="ocr error-log">' + esc(record.get('stderr') or record.get('error')) + '</pre>')
            out.append('</details>')
        out.append('</div></div></details>')
    diagnostics = read(folder / 'diagnostics/single-line-investigation.json')
    out += ['<section class="panel"><h2>极矮单行的额外诊断</h2><p>主测与这些变体分开记录，没有用变体结果“修正”原图得分。同像素 PNG 仍失败；上下各加 16 像素白边或放大两倍后，原 DLL 均恢复，原生的重复/错字也消失。该现象提示高度处理或分割是排查方向，尚未定位内部根因。</p>',
            '<div class="table-scroll"><table><thead><tr><th>变体</th><th>引擎</th><th>执行</th><th>输出</th></tr></thead><tbody>']
    for row in diagnostics:
        out.append(f'<tr><td>{esc(row["name"])}</td><td>{"原生" if row.get("backend")=="native" else "原 DLL"}</td><td>{"成功" if row["exit_code"]==0 else "失败 -18"}</td><td>{esc(row.get("text", "无结果"))}</td></tr>')
    out += ['</tbody></table></div><p class="small"><a href="diagnostics/single-line-investigation.json">诊断记录</a>；不计入 30 张独立样本的统计。</p></section>',
            '<section class="panel"><h2>下一步优先处理什么</h2><ol class="findings">',
            '<li><strong>优先检查区域去重、行链接和阅读顺序。</strong>用 05 极矮单行、14 海报重复片段、17 食谱、18 音标页、20 小票作定向回归。</li>',
            '<li><strong>单独检查关键字符和读数。</strong>13 路牌“愚”、12 检验表“58.9”、28 数码管的小数点和单位，不能仅依赖整体段落可读性。</li>',
            '<li><strong>表格与小票增加布局处理。</strong>保留坐标并重建行列关系；本次通用 OCR 接口没有承诺单元格输出，因此不能仅通过串接 lines 就得到可靠的表格。</li>',
            '<li><strong>建立局部精标再量化回归。</strong>本报告的核对点可作起点，需先确认字符与区域真值，才能计算 CER/WER 或比较改动前后指标。本次没有调整引擎实现。</li></ol></section>',
            '<section class="panel"><h2>运行版本、方法与可复现材料</h2>',
            '<p>Windows x64，CPU。主测使用插件默认路径 <code>artifacts/win32-x64/bin/ocr.exe</code>，与 <code>build/source-runtime/native/Release/ocr.exe</code> 的 SHA-256 相同。原生后端自报 <code>native-onnx-cpu / parity=incomplete</code>。对照通过原始 <code>bin/oneocr_wrapper.dll</code> 调用原版 <code>oneocr.dll</code>，不是原生兼容 wrapper。</p>',
            '<p>两版均显式设置最大 1000 行、内部缩放上限 1152×768，使用同源模型容器。输入文件原样传入，逐图、逐引擎启动独立进程，设 120 秒超时。耗时包含进程启动、Python 对照适配器及模型加载，不用于宣称引擎推理速度优劣。</p>',
            '<p>书页输入中的 EXIF 方向未在外部处理；图文报告的行框采用引擎解码后的原始像素坐标。摘要中的食谱预览仅为阅读方便转正，主测未转正。</p>',
            '<p><strong>样本标签纠正：</strong>' + esc(review['metadata_corrections'][0]['description']) + '</p>',
            '<div class="table-scroll"><table class="link-table"><tbody>']
    for key in ('native_cli', 'native_core', 'native_ort', 'model_manifest', 'legacy_oneocr.dll', 'legacy_oneocr.onemodel', 'legacy_oneocr_wrapper.dll', 'corpus_manifest'):
        out.append(f'<tr><th>{esc(key)}</th><td>{esc(run["fingerprints"][key]["sha256"])}</td></tr>')
    out += ['</tbody></table></div><p>新的独立运行目录可使用：</p>',
            '<pre class="ocr">.\\.research-python\\python.exe -X utf8 scripts/run-classics-oneocr.py --output reports/oneocr-classics-new-run</pre>',
            '<p class="small">运行中出现原 DLL 对照错误时命令会返回非零，仍保留已产生的测量与失败日志；不会伪装成全部成功。现有目录禁止覆盖。生成图文报告前，需要提供对应的审阅文件和运行快照。</p>',
            '<pre class="ocr">.\\.research-python\\python.exe -X utf8 scripts/report-classics-oneocr.py</pre>',
            '<p><a href="run.json">完整运行记录</a> · <a href="review.json">逐例审阅</a> · <a href="summary.json">机器可读汇总</a> · <a href="corpus-manifest-at-run.json">运行时样本清单</a> · <a href="runner-at-run.py">运行脚本快照</a></p>',
            '<p class="small muted">本 HTML 的图片和识别文本均内嵌，可离线单独打开。若需要下载原始 JSON、TXT 或审计文件，请保留完整报告目录。样本图片的来源与许可记录在清单中。</p></section>',
            '<p class="footer">定性审阅：Codex · 2026-11-02。图片与局部转录只用于展示本次运行证据；未将原文其他引擎输出当作精标。报告没有修改 OCR 引擎或替换失败的原始结果。</p>',
            '</main><dialog class="zoom" id="zoom"><button>关闭</button><img alt="放大图片"></dialog><script>' + JS + '</script></body></html>']
    report_path = folder / 'report.html'
    report_path.write_text('\n'.join(out), encoding='utf-8')
    summary = {'date': '2026-11-02', 'input_count': len(cases), 'review_method': review['method'],
               'execution_successes': {'native': native_success, 'legacy': legacy_success},
               'qualitative_counts': counts, 'dll_rated_better_cases': better,
               'parameters': run['parameters'], 'native_capabilities': run['native_capabilities'],
               'wall_time_seconds_including_startup': {
                   backend: {'sum': round(sum(x['seconds'] for x in run['measurements'] if x['backend']==backend), 3),
                             'median': round(statistics.median(x['seconds'] for x in run['measurements'] if x['backend']==backend), 3)}
                   for backend in ('native','legacy')},
               'reviews_sha256': sha(folder / 'review.json'), 'report_sha256': sha(report_path),
               'cases': [{'number': c['number'], 'name': c['name'], 'title': c['title'],
                          'native': review['cases'][c['name']]['native'], 'legacy': review['cases'][c['name']]['legacy']}
                         for c in cases]}
    (folder / 'summary.json').write_text(json.dumps(summary, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')
    print(json.dumps({'report': str(report_path), 'report_bytes': report_path.stat().st_size,
                      'inputs': len(cases), 'execution_successes': summary['execution_successes'],
                      'qualitative_counts': counts, 'dll_rated_better': len(better)}, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
