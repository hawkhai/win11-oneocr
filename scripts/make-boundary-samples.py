"""Generate local curve/long-line scenes; exact branch coverage uses the DLL geometry oracles."""
import hashlib
import json
import math
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'samples/oneocr-boundaries'
def main():
    font_file=Path('C:/Windows/Fonts/arial.ttf')
    font_sha=hashlib.sha256(font_file.read_bytes()).hexdigest()
    OUT.mkdir(parents=True,exist_ok=True);cases=[]
    def save(name,image,purpose):
        path=OUT/(name+'.png');image.save(path)
        cases.append({'file':path.name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
            'width':image.width,'height':image.height,'purpose':purpose,'source':'generated locally; not accuracy ground truth'})
    image=Image.new('RGB',(420,100))
    image.putdata([((x+y)%256,(x*3+y*2)%256,(x*7+y*5)%256) for y in range(100) for x in range(420)])
    save('normalizer-grid',image,'RGB source used by the six exact clipped-normalization geometry oracles')
    image=Image.new('RGB',(2000,350),'white');font=ImageFont.truetype(str(font_file),42)
    text='OneOCR curved text geometry 0123456789 SOURCE RECOVERY '
    x=50
    for char in text:
        advance=float(font.getlength(char));center=x+advance*.5
        if center>1950:break
        glyph=Image.new('RGBA',(100,100));ImageDraw.Draw(glyph).text((50,50),char,font=font,fill='black',anchor='mm')
        slope=.00022*(center-1000);glyph=glyph.rotate(-math.degrees(math.atan(slope)),Image.Resampling.BICUBIC,expand=True)
        y=100+.00011*(center-1000)**2
        image.paste(glyph,(round(center-glyph.width/2),round(y-glyph.height/2)),glyph);x+=advance
    save('curved-line',image,'Naturally curved printed Latin line; branch reachability is reported separately from visual appearance')
    image=Image.new('RGB',(10000,140),'white');draw=ImageDraw.Draw(image);font=ImageFont.truetype(str(font_file),34)
    text='OneOCR long line 0123456789 | ';advance=float(font.getlength(text))
    for i in range(math.ceil(9900/advance)):draw.text((20+i*advance,48),text,font=font,fill='black')
    save('very-long-line',image,'10000-pixel maximum legal image width; independent same-image DLL/native/Wasm comparison')
    image=Image.new('RGB',(1600,180),'white');draw=ImageDraw.Draw(image);font=ImageFont.truetype(str(font_file),17)
    text='Near collinear baseline 0123456789 ';x=20
    for i in range(5):draw.text((x,72+i%2),text,font=font,fill='black');x+=float(font.getlength(text))
    save('near-collinear-line',image,'Nearly horizontal small text; exact rank-deficient inputs are in the polynomial oracle contract')
    (OUT/'manifest.json').write_text(json.dumps({'generator':'scripts/make-boundary-samples.py','font':font_file.name,
        'font_sha256':font_sha,'cases':cases,'coverage_note':'PNG scenes are end-to-end fixtures. Internal rank deficiency and width-limit transitions are proved by the separate synthetic geometry oracle tests.'},ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    print('Generated',len(cases),'boundary sample images')
if __name__=='__main__':main()
