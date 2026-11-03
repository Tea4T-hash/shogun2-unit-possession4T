"""Recover official, hash-pinned float-Lua/Valve audit dependencies into a new folder.

Never installs, launches a game, writes Steam settings, or copies credentials.
"""
import argparse,hashlib,io,json,os,subprocess,tarfile,urllib.request
from pathlib import Path,PurePosixPath

LUA_URL='https://www.lua.org/ftp/lua-5.1.5.tar.gz'
LUA_SHA='2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333'
VALVE_COMMIT='e91ca2be0df2cef4c230cbbc0b86604d73a0bbf6'
VALVE={'isteamutils.h':'f9200ff54fca94435dad516db0685727d89787957ade4012ccf4a3bcea1f0be2','isteaminput.h':'277e53a45bfae91967fa113f1c1ab876f24629b14b9a3eae06cfa5af19397569','steam_api_flat.h':'5244d56f6fbc91df5f41a689a068b1aa3253d56ee866d7ec2422eff3b61edfb8'}
HEADER=bytes.fromhex('1b4c75615100010404040400')
SOURCES='lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c'.split()

def verified(data,expected,label):
 if len(data)>2*1024*1024 or hashlib.sha256(data).hexdigest()!=expected:raise ValueError('Dependency fingerprint mismatch: '+label)
 return data

def fetch(url,expected,label):
 with urllib.request.urlopen(url,timeout=60) as response:data=response.read(2*1024*1024+1)
 return verified(data,expected,label)

def extract_lua(data,output):
 with tarfile.open(fileobj=io.BytesIO(data),mode='r:gz') as archive:
  entries=archive.getmembers();total=0
  # Validate all members before writing anything.
  for entry in entries:
   name=PurePosixPath(entry.name)
   if name.is_absolute() or '..' in name.parts or not name.parts or name.parts[0]!='lua-5.1.5' or '\\' in entry.name:raise ValueError('Unsafe Lua source path')
   if not (entry.isfile() or entry.isdir()) or entry.issym() or entry.islnk():raise ValueError('Unsafe Lua archive member')
   total+=entry.size
   if entry.size>1024*1024 or total>8*1024*1024:raise ValueError('Lua source bounds exceeded')
  for entry in entries:
   target=output.joinpath(*PurePosixPath(entry.name).parts)
   if entry.isdir():target.mkdir(parents=True,exist_ok=True)
   else:
    target.parent.mkdir(parents=True,exist_ok=True)
    with target.open('xb') as stream:stream.write(archive.extractfile(entry).read())

def patch_float(text):
 if '#define LUA_NUMBER_DOUBLE' not in text or '#define LUA_NUMBER\tdouble' not in text:raise ValueError('Unexpected original Lua config')
 return text.replace('#define LUA_NUMBER_DOUBLE','#define LUA_NUMBER_FLOAT').replace('#define LUA_NUMBER\tdouble','#define LUA_NUMBER\tfloat').replace('"%lf"','"%f"').replace('"%.14g"','"%.7g"')

def batch(vcvars,output):
 def quote(args):
  if any(any(c in str(v) for c in '\r\n%&|<>^!') for v in args):raise ValueError('Unsupported compiler path')
  return subprocess.list2cmdline(list(map(str,args)))
 commands=[['cl','/nologo','/O2','/MT','/D_CRT_SECURE_NO_WARNINGS','/c',*SOURCES],['lib','/nologo','/out:lua51.lib',*[Path(n).with_suffix('.obj') for n in SOURCES]],['cl','/nologo','/O2','/MT','/D_CRT_SECURE_NO_WARNINGS','lua.c','lua51.lib','/Fe'+str(output/'lua51.exe')],['cl','/nologo','/O2','/MT','/D_CRT_SECURE_NO_WARNINGS','luac.c','print.c','lua51.lib','/Fe'+str(output/'luac51.exe')]]
 lines=['@echo off','call '+quote([vcvars]),'if not %errorlevel%==0 exit /b %errorlevel%']
 for command in commands:lines.extend([quote(command),'if not %errorlevel%==0 exit /b %errorlevel%'])
 return '\r\n'.join(lines)+'\r\n'

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True);p.add_argument('--vcvars',type=Path);p.add_argument('--lua-archive',type=Path,help='Optional existing official archive, still hash-checked');p.add_argument('--valve-reference-dir',type=Path,help='Optional existing pinned headers, still hash-checked');a=p.parse_args()
 output=a.output.resolve()
 if output.exists():p.error('Use a new output folder; existing files are preserved')
 if a.vcvars and not a.vcvars.is_file():p.error('MSVC x86 vcvars script not found')
 data=verified(a.lua_archive.read_bytes(),LUA_SHA,'Lua5.1.5') if a.lua_archive else fetch(LUA_URL,LUA_SHA,'Lua5.1.5')
 headers={}
 for name,expected in VALVE.items():
  headers[name]=verified((a.valve_reference_dir/name).read_bytes(),expected,name) if a.valve_reference_dir else fetch('https://raw.githubusercontent.com/ValveSoftware/Proton/'+VALVE_COMMIT+'/lsteamclient/steamworks_sdk_162/'+name,expected,name)
 output.mkdir(parents=True);(output/'lua-5.1.5.tar.gz').write_bytes(data);extract_lua(data,output)
 config=output/'lua-5.1.5/src/luaconf.h';config.write_text(patch_float(config.read_text()),newline='\n')
 (output/'valve').mkdir()
 for name,data in headers.items():(output/'valve'/name).write_bytes(data)
 report={'official_lua_url':LUA_URL,'official_lua_sha256':LUA_SHA,'number_format':'float','Valve_commit':VALVE_COMMIT,'Valve_header_hashes':VALVE,'game_files_read':False,'game_launched':False,'built':False}
 if a.vcvars:
  script=output/'build-lua.cmd';script.write_bytes(batch(a.vcvars.resolve(),output).encode())
  with (output/'build-lua.log').open('w') as log:
   result=subprocess.run([os.environ.get('COMSPEC','cmd.exe'),'/d','/c',script],cwd=output/'lua-5.1.5/src',stdout=log,stderr=subprocess.STDOUT)
  if result.returncode:raise RuntimeError('Lua build failed; inspect '+str(output/'build-lua.log'))
  probe=output/'probe.lua';probe.write_text('return 1.25\n')
  subprocess.run([output/'luac51.exe','-o',output/'probe.luac',probe],check=True)
  if (output/'probe.luac').read_bytes()[:12]!=HEADER:raise RuntimeError('Lua bytecode ABI mismatch')
  subprocess.run([output/'lua51.exe','-e','assert(1.25+2.5==3.75)'],check=True)
  report.update(built=True,bytecode_header=HEADER.hex(),binaries={n:hashlib.sha256((output/n).read_bytes()).hexdigest() for n in ['lua51.exe','luac51.exe']})
 (output/'dependencies.json').write_text(json.dumps(report,indent=2)+'\n')
 print('Verified dependencies ready at '+str(output)+'; compiler built='+str(report['built']))

if __name__=='__main__':main()
