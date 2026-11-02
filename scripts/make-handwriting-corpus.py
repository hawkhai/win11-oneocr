"""Generate deterministic rotations of the pinned, real handwritten Azure sample."""
import hashlib
import json
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
def main():
    root=ROOT/'samples/oneocr-parity';source=root/'handwriting-azure.jpg';digest=hashlib.sha256(source.read_bytes()).hexdigest()
    if digest!='cafe17111f6a496a5dc976804f33ecff2e66a2375a8e06f3c67f1ea7588a3e23':raise ValueError('Pinned handwritten source hash changed')
    source_url='https://github.com/Azure-Samples/cognitive-services-sample-data-files/blob/65af3b93cab5f45c76594f3265210646d4b3809e/ComputerVision/Images/handwritten_text.jpg'
    im=Image.open(source).convert('RGB');cases=[{'name':'handwriting-azure','file':source.name,'sha256':digest}]
    for degrees in (.5,3,10,90,180,270):
        label=format(degrees,'g');name='handwriting-rotate-'+label;destination=root/(name+'.png')
        im.rotate(degrees,Image.Resampling.BICUBIC,expand=True,fillcolor='white').save(destination)
        cases.append({'name':name,'file':destination.name,'sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'derived_from':source.name,'rotation_degrees':degrees})
    (root/'manifest.json').write_text(json.dumps({'schema_version':1,'source_url':source_url,'source_commit':'65af3b93cab5f45c76594f3265210646d4b3809e','source_sha256':digest,'license_file':'LICENSE.azure.txt','cases':cases},indent=2),encoding='utf8')
if __name__=='__main__':main()
