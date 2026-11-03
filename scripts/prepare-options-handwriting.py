"""Create additive fixtures from retained photographs; never synthesize handwriting."""
import argparse
import hashlib
import json
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT=Path(__file__).resolve().parents[1]
SOURCES=[
    ('red-grid-jason','7SFlf90CNic','1785339544283-608810cc6058','red-chinese-characters-handwritten-on-white-paper','2026-07-29'),
    ('red-calligraphy-jason','5oLLnT9lxIU','1786271818844-452b00030f8c','handwritten-chinese-calligraphy-on-grid-paper','2026-08-09'),
]

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def write(path,value):path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquired',type=Path,required=True)
    p.add_argument('--font',type=Path,default=Path('C:/Windows/Fonts/msyh.ttc'));a=p.parse_args()
    target=ROOT/'samples/options-handwriting';target.mkdir(parents=True,exist_ok=True);sources=[];derivatives=[]
    for name,photo_id,cdn,slug,date in SOURCES:
        source=a.acquired/(name+'.jpg');destination=target/source.name;destination.write_bytes(source.read_bytes())
        with Image.open(destination) as photo:
            photo.load();image=photo.convert('RGB');size=list(photo.size)
        sources.append({'file':destination.name,'sha256':sha(destination),'dimensions':size,'kind':'photograph of actual Chinese handwriting',
            'photographer':'jason hu (@hujason)','writer':'not separately identified by photographer; no multi-writer claim',
            'source_page':f'https://unsplash.com/photos/{slug}-{photo_id}','published':date,
            'download_url':f'https://images.unsplash.com/photo-{cdn}?w=1600','delivery':'CDN width-1600 rendition; not claimed to be full-resolution camera original',
            'license':'Unsplash License','license_url':'https://unsplash.com/license','retrieved':'2026-11-03','original_retained':True})
        for angle in [90,180,270,10]:
            output=target/f'{name}-rotate-{angle}.png'
            if angle in (90,180,270):result=image.transpose({90:Image.Transpose.ROTATE_90,180:Image.Transpose.ROTATE_180,270:Image.Transpose.ROTATE_270}[angle])
            else:result=image.rotate(angle,resample=Image.Resampling.BICUBIC,expand=True,fillcolor='white')
            result.save(output)
            derivatives.append({'file':output.name,'sha256':sha(output),'source':destination.name,'source_sha256':sha(destination),
                                'operation':{'rotate_counterclockwise_degrees':angle,'expand':True,'resample':'transpose (lossless pixels)' if angle%90==0 else 'Pillow BICUBIC','fill':'white'}})
    image=Image.new('RGB',(1280,960),'white');draw=ImageDraw.Draw(image);font=ImageFont.truetype(str(a.font),32)
    for i in range(12):draw.text((32,20+74*i),f'第 {i+1:02d} 行：中文识别参数回归 OneOCR {i+1:02d}',font=font,fill='black')
    printed=target/'twelve-printed-lines.png';image.save(printed)
    blank=target/'parameter-blank.png';Image.new('RGB',(120,100),'white').save(blank)
    write(target/'provenance.json',{'schema_version':1,'sources':sources,'derivatives':derivatives,
        'generated_controls':[{'file':printed.name,'sha256':sha(printed),'kind':'printed control, NOT a handwriting sample','font_file':a.font.name,'font_sha256':sha(a.font),'font_redistributed':False},
                              {'file':blank.name,'sha256':sha(blank),'kind':'blank control'}],
        'limitations':['Two photographs from one photographer; writer identities are unspecified. This is not population-level handwriting coverage.',
                        'Parity means reproducing the original DLL, not correctness against human transcription.']})
    cases=[]
    def add(file,n,w,h,purpose):
        source=target/file;cases.append({'name':f'{source.stem}-m{n}-r{w}x{h}','file':source.relative_to(ROOT).as_posix(),'sha256':sha(source),
            'options':{'max_lines':n,'resize_width':w,'resize_height':h},'purpose':purpose})
    for n in [1,2,3,4,5,7,9,11,32,999,1000]:
        for w,h in [(1152,768),(640,384),(320,192),(768,512),(33,65)]:
            add(printed.name,n,w,h,'Cartesian coverage of line-budget batching and valid DLL resize settings')
    for source in sources:
        for n,w,h in [(1,640,384),(5,1152,768),(7,768,512)]:add(source['file'],n,w,h,'Actual Chinese handwriting with nondefault options')
    for n,w,h in [(0,640,384),(2,0,0)]:add(printed.name,n,w,h,'Portable zero aliases; DLL baseline uses explicitly recorded effective defaults')
    for w,h in [(32,32),(1152,768),(1152,767),(1151,768)]:add(blank.name,1,w,h,'Valid DLL resize boundary controls')
    write(ROOT/'research/parameter-cases.json',{'schema_version':1,'scope':'67 finite reproducible combinations, not all possible options',
        'dll_supported_resize':{'maximum_width':1152,'maximum_height':768},'cases':cases})
    print(f'{len(sources)} retained real photographs, {len(derivatives)} derivatives, 2 controls; {len(cases)} parameter cases')

if __name__=='__main__':main()
