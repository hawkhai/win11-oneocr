"""Generate labelled synthetic OCR fixtures; original Microsoft images are hash locked."""
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageFilter

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/'samples/oneocr-capabilities'
FONTS = Path('C:/Windows/Fonts')
SCRIPTS = {
    'latin':('arial.ttf','Invoice number A-2026 / Total EUR 129.50'),
    'cjk':('simsun.ttc','中文简体與繁體 日本語 한국어 2026'),
    'cyrillic':('arial.ttf','Привет мир! Проверка текста 2026'),
    'arabic':('arial.ttf','مرحبا بالعالم 2026'),
    'devanagari':('Nirmala.ttf','नमस्ते दुनिया 2026'),
    'hebrew':('arial.ttf','שלום עולם 2026'),
    'thai':('LeelawUI.ttf','สวัสดีชาวโลก 2026'),
    'tamil':('Nirmala.ttf','வணக்கம் உலகம் 2026'),
    'greek':('arial.ttf','Γειά σου κόσμε 2026'),
}
SOURCES = [
 ('azure-multilingual.png','MultiLingual.png','31b9bd7cc31ba3f98be9f59ed2b2bcc357c8946691192f8acd5fdb177ab07141',['multilingual','printed']),
 ('azure-printed-photo.jpg','printed_text.jpg','144ba6545b0c43115f121acf107303ed7a687150c06e60f559d9965a88e2df29',['camera-photo','printed']),
 ('azure-dog-negative.jpg','dog.jpg','cc413070133fc9625b5dc6ce2c3dcedba98aedfb531f35046d03f750577a93f6',['non-text','photo']),
]

def main():
    OUT.mkdir(exist_ok=True)
    cases=[]
    def record(name,tags,**extra):
        path=OUT/name
        with Image.open(path) as im: size=im.size
        cases.append({'name':path.stem,'file':name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                      'width':size[0],'height':size[1],'tags':tags,**extra})
    def save(name,im,tags,**extra):
        filename=name+'.png';im.save(OUT/filename)
        record(filename,tags,source='Locally generated fixture, not a Microsoft demonstration or accuracy ground truth',**extra)
    def font(name='arial.ttf',size=32):return ImageFont.truetype(str(FONTS/name),size)
    for filename,original,sha,tags in SOURCES:
        if hashlib.sha256((OUT/filename).read_bytes()).hexdigest()!=sha:raise ValueError('Original sample changed: '+filename)
        record(filename,tags,source='azure-samples',source_path='ComputerVision/Images/'+original)
    mixed=Image.new('RGB',(1100,900),'white');draw=ImageDraw.Draw(mixed)
    for i,(script,(face,text)) in enumerate(SCRIPTS.items()):
        im=Image.new('RGB',(1100,170),'white');ImageDraw.Draw(im).text((30,55),text,font=font(face,36),fill='black')
        save('script-'+script,im,['synthetic','script-'+script,'printed'],rendered_text=text,font=face)
        draw.text((30,25+i*95),text,font=font(face,34),fill='black')
    save('mixed-nine-scripts',mixed,['synthetic','multilingual','mixed-scripts','reading-order'])
    screenshot=Image.new('RGB',(1100,440),(27,30,36));d=ImageDraw.Draw(screenshot)
    for i,line in enumerate(['CONTACT DIRECTORY','Name: Alex Example','Email: alex@example.com','Phone: +1 (202) 555-0142','Website: https://example.com/help','Case ID: OCR-2026-001']):
        d.text((35,25+i*65),line,font=font(size=32),fill=(240,243,247))
    save('dark-contact-screenshot',screenshot,['synthetic','screenshot','email','phone','dark-background'])
    table=Image.new('RGB',(1100,470),'white');d=ImageDraw.Draw(table)
    rows=[['Item','Quantity','Unit price','Total'],['Notebook','3','12.50','37.50'],['Pen','10','1.20','12.00'],['Folder','2','4.00','8.00'],['TOTAL','','','57.50']]
    for row,values in enumerate(rows):
        for col,value in enumerate(values):d.text((25+col*270,25+row*85),value,font=font(size=30),fill='black')
    for y in range(0,426,85):d.line((5,y+5,1090,y+5),fill=(80,80,80),width=2)
    for x in range(0,1081,270):d.line((x+5,5,x+5,430),fill=(80,80,80),width=2)
    save('single-table',table,['synthetic','table','rows','columns','word-boundaries'])
    quality=Image.new('RGB',(1100,230),(232,228,220));d=ImageDraw.Draw(quality)
    d.text((30,40),'Low contrast receipt / EUR 125.80',font=font(size=30),fill=(142,140,136))
    d.text((30,105),'Small text 0123456789 ABC xyz',font=font(size=16),fill=(115,115,115))
    save('low-contrast-small-blur',quality.filter(ImageFilter.GaussianBlur(.45)),['synthetic','low-contrast','small-font','blur'])
    hw=Image.open(ROOT/'samples/oneocr-parity/handwriting-azure.jpg').convert('RGB')
    combined=Image.new('RGB',(max(1100,hw.width),hw.height+150),'white')
    ImageDraw.Draw(combined).text((25,30),'Printed header / handwritten notes below',font=font(size=36),fill='black')
    combined.paste(hw,(0,150))
    save('printed-and-real-handwriting',combined,['derived','printed','real-handwriting','mixed-style'],derived_from='../oneocr-parity/handwriting-azure.jpg',operation='Unscaled source pasted below a synthetic printed header')
    save('blank-unicode-空白-😀',Image.new('RGB',(640,360),'white'),['synthetic','blank','unicode-path'])
    manifest={'schema_version':1,'source_commit':'65af3b93cab5f45c76594f3265210646d4b3809e',
              'source_repository':'https://github.com/Azure-Samples/cognitive-services-sample-data-files',
              'license_file':'../oneocr-parity/LICENSE.azure.txt','font_files':[{'file':x,'sha256':hashlib.sha256((FONTS/x).read_bytes()).hexdigest()} for x in sorted({x[0] for x in SCRIPTS.values()})],
              'cases':cases}
    (OUT/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Created/verified',len(cases),'capability fixtures')

if __name__=='__main__':main()
