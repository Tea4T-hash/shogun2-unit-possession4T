"""Private native-evidence check; emit only first-party hashes/offsets, no game code."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

ROOT=Path(__file__).resolve().parents[2]
STOCK='022fae2e2da92b59b8b1b8ee52887003c624e3c8eca060b209f5d792a7f874f5'
BUGFIX='c4d91d74ce9ff8338cf61d5b021b03a2776830177a590fcac205509a42239545'
SLOTS=[0x94,0x98,0x9c,0xa0,0xa4,0xa8,0x12c,0x130]
FUNCTIONS=[0x1362030,0x1361f80,0x1362140,0x13620e0,0x1362230,0x1361fd0,
           0x3248f0,0x321820,0x2f76e0,0x2f7a50,0x2e5810,0xbd4a10,0x2f7350,
           0x10fde30,0x335260,0x1101da0,0x254f00,0x1cc200,0x1d0d40,0x1d10f0,0x2e6b70,
           0x10dfb80,0x10f97d0,0x10fdf10,0x10d1090,0x342930,0x285a00,0x285c80,0x285bf0,0xb903f0,0x3428c0,0xf92ae0,0xbd12a0,0xf90330]

def sha(data):return hashlib.sha256(data).hexdigest()

def pe(data):
    at=struct.unpack_from('<I',data,0x3c)[0];opt=at+24
    assert data[:2]==b'MZ' and data[at:at+4]==b'PE\0\0'
    assert struct.unpack_from('<H',data,at+4)[0]==0x14c
    assert struct.unpack_from('<I',data,opt+28)[0]==0x10000000
    table=opt+struct.unpack_from('<H',data,at+20)[0]
    sections=[struct.unpack_from('<4I',data,table+i*40+8) for i in range(struct.unpack_from('<H',data,at+6)[0])]
    def raw(rva):
        for virtual,start,size,offset in sections:
            if start<=rva<start+size:return offset+rva-start
        raise ValueError('Unbacked RVA '+hex(rva))
    reloc_rva,reloc_size=struct.unpack_from('<II',data,opt+96+5*8);cursor=raw(reloc_rva);end=cursor+reloc_size;fixups=[]
    while cursor<end:
        page,size=struct.unpack_from('<II',data,cursor);assert size>=8 and cursor+size<=end
        for i in range(cursor+8,cursor+size,2):
            value=struct.unpack_from('<H',data,i)[0]
            if value>>12==3:fixups.append(page+(value&4095))
            else:assert value>>12==0
        cursor+=size
    return raw,fixups

def header(value):
    rows=[];relocations=[]
    for row in value['functions']:
        first=len(relocations);relocations.extend(row['relocations'])
        rows.append('{%d,%d,%d,%d,"%s"},'%(row['rva'],row['size'],first,len(row['relocations']),row['sha256']))
    return '\n'.join(['#pragma once','namespace binding175 {',
        'struct Function {unsigned rva,size,reloc_start,reloc_count;const char* sha256;};',
        'inline constexpr unsigned camera_slots[]={'+','.join(map(str,SLOTS))+'};',
        'inline constexpr unsigned camera_functions[]={'+','.join(str(x['function_rva']) for x in value['camera_slots'])+'};',
        'inline constexpr unsigned relocations[]={'+','.join(map(str,relocations))+'};',
        'inline constexpr Function functions[]={',*rows,'};','}'])+'\n'

def review(stock,bugfix,index,focused):
    a=stock.read_bytes();b=bugfix.read_bytes();assert sha(a)==STOCK and sha(b)==BUGFIX
    raw,fixups=pe(a);raw_b,_=pe(b);rows=[];evidence=[]
    for rva in FUNCTIONS:
        original=index/'functions'/f'{rva:x}.asm'
        path=next((f/f'function_{rva:x}.asm' for f in focused if (f/f'function_{rva:x}.asm').is_file()),original)
        instructions=[]
        for line in path.read_text(encoding='utf-8').splitlines():
            if not line.startswith('0x'):continue
            address,encoded,_=line.split('\t',2);address=int(address,16);body=bytes.fromhex(encoded)
            assert a[raw(address):raw(address)+len(body)]==body,(path,hex(address))
            instructions.append((address,len(body)))
        assert instructions and min(x[0] for x in instructions)==rva
        size=max(x+n for x,n in instructions)-rva
        body=a[raw(rva):raw(rva)+size];other=b[raw_b(rva):raw_b(rva)+size]
        assert body==other,'Reviewed Bugfix changes newly used body '+hex(rva)
        rel=[x-rva for x in fixups if rva<=x<rva+size]
        assert all(x+4<=size for x in rel)
        rows.append(dict(rva=rva,size=size,sha256=sha(body),relocations=rel))
        evidence.append(dict(rva=hex(rva),instructions=len(instructions),assembly=str(path),assembly_sha256=sha(path.read_bytes())))
    cameras=[]
    for slot,expected in zip(SLOTS,FUNCTIONS[:8]):
        actual=struct.unpack_from('<I',a,raw(0x15d1a74+slot))[0]-0x10000000
        assert actual==expected
        assert a[raw(0x15d1a74+slot):raw(0x15d1a74+slot)+4]==b[raw_b(0x15d1a74+slot):raw_b(0x15d1a74+slot)+4]
        cameras.append(dict(slot=slot,function_rva=actual))
    value=dict(schema=175,stock_sha256=STOCK,reviewed_bugfix_sha256=BUGFIX,
        camera_slots=cameras,functions=rows,scope='Exact native body hashes and relocation offsets; recovered C is analysis, not an ABI proof. No game bytes included.')
    return value,evidence

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('stock','bugfix','index','output'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--focused',type=Path,action='append',required=True)
    p.add_argument('--write-reviewed-metadata',action='store_true');a=p.parse_args()
    value,evidence=review(a.stock,a.bugfix,a.index,a.focused);a.output.mkdir(exist_ok=False,parents=True)
    (a.output/'native-body-evidence.json').write_text(json.dumps(evidence,indent=2)+'\n')
    (a.output/'bindings.json').write_text(json.dumps(value,indent=2)+'\n')
    (a.output/'bindings175_generated.h').write_text(header(value))
    if a.write_reviewed_metadata:(ROOT/'tools/data/deck175-native-bindings.json').write_text(json.dumps(value,indent=2)+'\n')
    print('PASS '+str(len(value['functions']))+' exact native bodies; stock/Bugfix match; no code or assets emitted')

if __name__=='__main__':main()
