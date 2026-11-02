"""Create deterministic local fixtures. Does not download fonts or images."""
import json
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'work/oneocr-portable/corpus'
OUT.mkdir(parents=True,exist_ok=True)
fonts=Path('C:/Windows/Fonts')
samples={
    'latin':('arial.ttf','OneOCR native source 2026\nHello World! Linux Android macOS'),
    'cjk':('simsun.ttc','中文识别与跨平台源码\n日本語テキスト 한국어 테스트'),
    'cyrillic':('arial.ttf','Привет мир! Распознавание текста 2026'),
    'arabic':('arial.ttf','مرحبا بالعالم 2026'),
    'devanagari':('Nirmala.ttf','नमस्ते दुनिया 2026'),
    'hebrew':('arial.ttf','שלום עולם 2026'),
    'thai':('LeelawUI.ttf','สวัสดีชาวโลก 2026'),
    'tamil':('Nirmala.ttf','வணக்கம் உலகம் 2026'),
    'greek':('arial.ttf','Γειά σου κόσμε 2026'),
}
cases=[];missing=[]
def save(name,image,tags):
    path=OUT/(name+'.png');image.save(path)
    cases.append({'name':name,'file':path.name,'tags':tags,'source':'generated locally by scripts/make-corpus.py; not an accuracy ground truth'})
for script,(font,text) in samples.items():
    path=fonts/font
    if not path.exists():
        missing.append({'script':script,'font':font});continue
    for size in ([18,36] if script in ('latin','cjk') else [36]):
        image=Image.new('RGB',(1000,220),'white')
        ImageDraw.Draw(image).multiline_text((30,40),text,font=ImageFont.truetype(str(path),size),fill='black',spacing=20)
        save(f'{script}-{size}',image,[script,'printed','small-font' if size==18 else 'normal'])
        if script in ('latin','cjk') and size==36:
            for angle in (.5,3,10,90,180,270):
                save(f'{script}-rotate-{angle}',image.rotate(angle,expand=True,fillcolor='white'),[script,'rotation',str(angle)])
save('blank',Image.new('RGB',(320,180),'white'),['blank'])
image=Image.new('RGBA',(1000,220),(255,255,255,0))
ImageDraw.Draw(image).text((30,40),'Alpha transparent text 2026',font=ImageFont.truetype(str(fonts/'arial.ttf'),36),fill=(0,0,0,255))
save('透明路径-emoji-😀',image,['unicode-path','alpha'])
multi=Image.new('RGB',(1000,300),'white');d=ImageDraw.Draw(multi);font=ImageFont.truetype(str(fonts/'arial.ttf'),28)
d.multiline_text((30,30),'Column A\nFirst line\nSecond line',font=font,fill='black',spacing=25)
d.multiline_text((520,30),'Column B\nThird line\nFourth line',font=font,fill='black',spacing=25)
save('multi-column',multi,['columns'])
vertical=Image.new('RGB',(220,600),'white');d=ImageDraw.Draw(vertical)
font=ImageFont.truetype(str(fonts/'simsun.ttc'),40)
for i,c in enumerate('中文竖排识别测试'):
    d.text((80,20+i*60),c,font=font,fill='black')
save('vertical-cjk',vertical,['cjk','vertical'])
manifest={'cases':cases,'missing_fonts':missing,'coverage_gaps':['Real handwriting images are not present; cursive fonts are not counted as handwriting.','Generated text supplies reproducible DLL comparisons, not independent recognition ground truth.']}
(OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf8')
print('Created',len(cases),'fixtures; missing fonts:',missing)
