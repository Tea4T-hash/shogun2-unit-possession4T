"""Verify additional QoL native dependencies against both exact owned engines."""
from pathlib import Path
import argparse,hashlib,json,sys
sys.path.insert(0,str(Path(__file__).resolve().parent))
from review_bindings175 import pe,STOCK,BUGFIX
ROOT=Path(__file__).resolve().parents[2]
RANGES=[(0x125500,121),(0x37b480,998),(1549344, 347), (3283072, 14), (17717408, 462), (17392192, 18), (16282752, 594), (16371264, 171), (17707904, 466), (17697280, 52), (17703104, 20), (17682000, 52), (17717184, 103), (17827216, 31), (17678432, 81), (17794544, 132), (17769456, 47), (3650608, 73), (3776384, 58), (2446976, 158), (3042992, 79), (3418416, 232), (1884672, 192), (1903936, 85), (1904880, 91), (4560624, 197)]
FORMATION_PREVIEW = [(0x2647e0, 0x1aa), (0x40bd40, 0x4a), (0x42e540, 0x253),
                     (0x471950, 0x1b1), (0x1101e10, 0x66a)]
TRANSLATION_PREVIEW = [(0x264600, 0x1d5), (0x42dfd0, 0x2b5)]
MOVE_AUDIO = [(0x269240, 0x7b), (0x31e6d0, 0xa), (0x2e9593, 0x66)]
FIRE_ELIGIBILITY = [(0x15c2e0, 0x1f), (0x181c20, 0x3f), (0x15c4c0, 0x68)]
WIDTH_RELEASE = [(0x10dfb80, 0x47), (0x10f97d0, 0xde5), (0x10ff190, 0x7d),
                 (0x2644d0, 0x130), (0x42dec0, 0xa1)]
TOOLTIP_DRAW = [(0xf8a4e0, 0xc66), (0x1401080, 0xf7), (0x13e35d0, 0x7d),
                (0x1421f70, 0xd3), (0x1421ec0, 0x7f)]
TOOLTIP_DRAW_RVAS = {rva for rva, _ in TOOLTIP_DRAW}
RANGES += TOOLTIP_DRAW + FORMATION_PREVIEW + [(0x10dc0c0, 0x197)] + TRANSLATION_PREVIEW + MOVE_AUDIO + FIRE_ELIGIBILITY + WIDTH_RELEASE
WIDTH_RVAS = {rva for rva, _ in WIDTH_RELEASE}
FIRE_ELIGIBILITY_RVAS = {rva for rva, _ in FIRE_ELIGIBILITY}
TRANSLATION_RVAS = {rva for rva, _ in TRANSLATION_PREVIEW}
MOVE_AUDIO_RVAS = {rva for rva, _ in MOVE_AUDIO}
PREVIEW_RVAS = {rva for rva, _ in FORMATION_PREVIEW}

def generate(stock,bugfix):
 a=Path(stock).read_bytes();b=Path(bugfix).read_bytes()
 assert hashlib.sha256(a).hexdigest()==STOCK and hashlib.sha256(b).hexdigest()==BUGFIX
 ra,fa=pe(a);rb,fb=pe(b);spans=[];rel=[]
 for r,n in RANGES:
  feature=512 if r in TOOLTIP_DRAW_RVAS else 256 if r in WIDTH_RVAS else 128 if r in FIRE_ELIGIBILITY_RVAS else 64 if r in MOVE_AUDIO_RVAS else 32 if r in TRANSLATION_RVAS else 16 if r in PREVIEW_RVAS else 8 if r in(0x125500,0x37b480,0x10dc0c0)else 1 if r==0x1096240 else 4 if r in(0xf87480,0xf9ce40)else 8 if r in(17827216, 17678432, 17794544, 17769456, 3650608, 3776384, 2446976, 3042992, 3418416, 1884672, 1903936, 1904880, 4560624)else 2
  body=a[ra(r):ra(r)+n];assert len(body)==n and body==b[rb(r):rb(r)+n]
  rr=[x-r for x in fa if r<=x<r+n];assert rr==[x-r for x in fb if r<=x<r+n] and all(x+4<=n for x in rr)
  first=len(rel);rel.extend(rr);spans.append((r,n,first,len(rr),feature,hashlib.sha256(body).hexdigest()))
 text=['#pragma once','// Generated from original instruction ranges; see tools/deck/generate_native214.py.','namespace native214 {','struct Span{unsigned rva,size,start,count,feature;const char* sha;};','inline constexpr unsigned relocations[]={'+','.join(map(str,rel))+'};','inline constexpr Span spans[]={']
 text+=['{%d,%d,%d,%d,%d,"%s"},'%x for x in spans];text+=['};','}']
 return '\n'.join(text)+'\n'
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for k in ['stock','bugfix','output']:p.add_argument('--'+k,type=Path,required=True)
 p.add_argument('--write',action='store_true');x=p.parse_args();text=generate(x.stock,x.bugfix);target=ROOT/'native/deck/native214_generated.h'
 if x.write:target.write_bytes(text.replace('\n','\r\n').encode())
 else:assert target.read_text()==text
 x.output.mkdir(parents=True,exist_ok=False);(x.output/'verification.json').write_text(json.dumps(dict(guarded_ranges=RANGES,stock=STOCK,bugfix=BUGFIX))+ '\n')
 print('PASS native214: exact original modal, speed and text-layout instruction guards')
if __name__=='__main__':main()
