"""Validate campaign entry against owned scripts and an inert native fixture."""
import argparse,hashlib,json,os,struct,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'tools'),str(ROOT/'tools/deck')]
from game_packs import Pack,digest
from build174 import batch_script
import adapt_frontend181

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('build','game','output','vcvars','lua','luac'):p.add_argument('--'+n,type=Path,required=True)
 a=p.parse_args();build=a.build.resolve();out=a.output.resolve();game=a.game.resolve()
 if out.exists()or out.is_relative_to(game):p.error('Use a new isolated output outside game')
 result=json.loads((build/'verification.json').read_text());assert result['candidate']==181 and not result.get('quick_only')
 out.mkdir(parents=True);fixture=build/'workshop/bin/s2up-workshop165-fixture.dll'
 if not fixture.is_file():raise ValueError('Missing inert Workshop fixture')
 source=out/'campaign-fixture.lua';compiled=out/'campaign-fixture.luac'
 source.write_text(adapt_frontend181.campaign_wrapper(build/'source',digest(fixture)),encoding='utf-8')
 subprocess.run([str(a.luac.resolve()),'-s','-o',str(compiled),str(source)],check=True)
 pack=Pack(game/'data/data.pack');off,size=pack.entries['ui/campaign ui/layout.root.luac']
 with (game/'data/data.pack').open('rb')as f:f.seek(off);owned=f.read(size)
 assert size==60729 and hashlib.sha256(owned).hexdigest()=='d14bea292474e62bb6e1e7a5f4938cdfa121b5bcca3003d693c9a31d5d703b50'
 reference=out/'private-owned-campaign.luac';reference.write_bytes(owned)
 cache=out/'cache';cache.mkdir();env=dict(os.environ,LOCALAPPDATA=str(cache),S2UP165_FIXTURE_GAME=str(game))
 with (out/'compiled-entry.log').open('w')as log:
  subprocess.run([str(a.lua.resolve()),str(ROOT/'tools/deck/verify_campaign_entry181.lua'),str(build/'workshop/bootstrap-fixture.luac'),str(compiled),str(reference)],check=True,cwd=game,env=env,stdout=log,stderr=subprocess.STDOUT)
 assert 'PASS 8 compiled181 menu/campaign' in (out/'compiled-entry.log').read_text()
 cases=out/'reader-fixtures';cases.mkdir()
 def write(name,entries,magic=b'PFH3'):
  index=b''.join(struct.pack('<I',len(value))+key.replace('/','\\').encode()+b'\0'for key,value in entries.items())
  (cases/name).write_bytes(struct.pack('<4s5IQ',magic,3,0,0,len(entries),len(index),1)+index+b''.join(entries.values()))
 target='ui/campaign ui/layout.root.luac'
 write('database.pack',{'db/unit_tables/fixture':b'not a UI root'},b'PFH2')
 write('campaign-addon.pack',{target:b'owned test addon A'})
 write('campaign-other.pack',{target:b'owned test addon B'})
 write('oversized.pack',{target:b'x'*262145})
 identity='a'*64
 write('self.pack',{target:('/S2UP-Workshop165-'+identity+'.dll').encode(),'possession/workshop165.json':json.dumps(dict(first_party_bootstrap=True,native_sha256=identity)).encode()})
 exe=out/'verify-campaign181.exe';batch=out/'native.cmd'
 commands=[['cl','/nologo','/std:c++17','/EHsc','/O2','/MT','/W4','/I'+str(build/'source/tools/deck'),ROOT/'tools/deck/verify_campaign181.cpp','/Fo'+str(out/'native.obj'),'/Fe'+str(exe),'/link','bcrypt.lib','user32.lib'],[exe,game,cases]]
 backend_exe=out/'verify-context181.exe'
 commands.extend([['cl','/nologo','/std:c++17','/EHsc','/O2','/MT',ROOT/'tools/deck/verify_context_backend181.cpp','/Fo'+str(out/'context.obj'),'/Fe'+str(backend_exe),'/link','user32.lib'],[backend_exe]])
 batch.write_text(batch_script(a.vcvars.resolve(),commands))
 with (out/'native-reader.log').open('w')as log:subprocess.run([os.environ.get('COMSPEC','cmd.exe'),'/d','/c',str(batch)],check=True,cwd=out,stdout=log,stderr=subprocess.STDOUT)
 assert 'PASS 5 context181' in (out/'native-reader.log').read_text()
 assert 'PASS 8 campaign181' in (out/'native-reader.log').read_text()
 assert owned not in compiled.read_bytes()
 receipt=dict(candidate=181,compiled_menu_campaign_cases=8,owned_reader_cases=8,context_backend_cases=5,actual_stock_campaign_handoff_sha256=hashlib.sha256(owned).hexdigest(),stock_execution='mocked only; exact handed-off bytes compared',native_entry='inert fixture; real export lookup and campaign preparation',game_or_Steam_started=False,Deck_validation=False)
 (out/'verification.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt,indent=2))
if __name__=='__main__':main()
