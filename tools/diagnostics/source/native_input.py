"""Read-only game-focused mouse state and native combat capture. No game writes/input."""
from process_probe import Process, processes, modules
import ctypes as C
from ctypes import wintypes as W
import argparse, json, struct, time
from pathlib import Path
from process_memory import sample as memory_sample
from accuracy_read90 import decode_accuracy90
from actor_roles import human, combat_actor

args=argparse.ArgumentParser(description=__doc__)
args.add_argument('--pid',type=int,help='Pin the recording to this game process.')
args.add_argument('--duration',type=float,default=None,help='Optional bounded recording; default runs until stop file or game exit.')
args.add_argument('--qte-version',type=int,default=53)
args.add_argument('--native-log',type=Path,help='Native runtime log path when the integrated mod runs from its embedded cache.')
args.add_argument('--output-dir',type=Path,default=Path('outputs/shogun2-melee-qte-v47/private'),help='Candidate-specific recording directory.')
opts=args.parse_args()
if opts.duration is not None and opts.duration<=0:args.error('--duration must be positive')
pid=opts.pid if opts.pid is not None else processes()[0]['pid'];base=next(m['base'] for m in modules(pid) if m['name'].lower()=='empire.retail.dll')
p=Process(pid);k=C.WinDLL('kernel32',use_last_error=True);u=C.WinDLL('user32',use_last_error=True)
k.GetTickCount64.restype=C.c_uint64
k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)];k.GetExitCodeProcess.restype=W.BOOL
k.OpenFileMappingW.argtypes=[W.DWORD,W.BOOL,W.LPCWSTR];k.OpenFileMappingW.restype=W.HANDLE
k.MapViewOfFile.argtypes=[W.HANDLE,W.DWORD,W.DWORD,W.DWORD,C.c_size_t];k.MapViewOfFile.restype=C.c_void_p
k.UnmapViewOfFile.argtypes=[C.c_void_p];k.CloseHandle.argtypes=[W.HANDLE]
u.GetForegroundWindow.restype=W.HWND
u.GetWindowThreadProcessId.argtypes=[W.HWND,C.POINTER(W.DWORD)]
u.GetAsyncKeyState.argtypes=[C.c_int];u.GetAsyncKeyState.restype=C.c_short
h=k.OpenFileMappingW(4,False,f'Local\\Shogun2PossessionPOC46-{pid}')
if not h:raise C.WinError(C.get_last_error())
v=k.MapViewOfFile(h,4,0,0,84)
if not v:raise C.WinError(C.get_last_error())
equipment_version,equipment_size=106,224
eh=k.OpenFileMappingW(4,False,f'Local\\Shogun2Equipment{equipment_version}-{pid}')
if not eh:
    equipment_version,equipment_size=81,216
    eh=k.OpenFileMappingW(4,False,f'Local\\Shogun2Equipment{equipment_version}-{pid}')
if not eh:raise RuntimeError('Equipment telemetry is not loaded')
ev=k.MapViewOfFile(eh,4,0,0,equipment_size)
if not ev:raise C.WinError(C.get_last_error())
def equipment_snapshot81():
    for retry in range(4):
        seq=struct.unpack('<i',C.string_at(ev+16,4))[0]
        if seq&1:continue
        data=C.string_at(ev,equipment_size)
        if seq!=struct.unpack('<i',C.string_at(ev+16,4))[0]:continue
        magic,version,size,owner=struct.unpack_from('<4I',data)
        if (magic,version,size,owner)!=(0x31514553,equipment_version,equipment_size,pid):raise RuntimeError('Equipment81 protocol differs')
        names=['lease','actor','unit','slot','kind','styles','ability','projectile','crouch_supported','crouched','pending','error','switches','ammo_changes']
        result=dict(zip(names,struct.unpack_from('<14I',data,20)))
        result['tick_ms']=struct.unpack_from('<Q',data,80)[0]
        result['ammunition']=data[88:216].decode('utf-16-le').split('\0')[0]
        if equipment_version==106:result['bow_power']=struct.unpack_from('<f',data,216)[0]
        return result
    return {'unstable':True}

ah=k.OpenFileMappingW(4,False,fr'Local\Shogun2Ammunition83-{pid}')
if not ah:raise RuntimeError('Ammunition83 telemetry is not loaded')
av=k.MapViewOfFile(ah,4,0,0,4808)
if not av:raise C.WinError(C.get_last_error())
def ammunition_snapshot83():
    for retry in range(4):
        seq=struct.unpack('<i',C.string_at(av+16,4))[0]
        if seq&1:continue
        data=C.string_at(av,4808)
        if seq!=struct.unpack('<i',C.string_at(av+16,4))[0]:continue
        if struct.unpack_from('<4I',data)!=(0x334d4153,83,4808,pid):raise RuntimeError('Ammo83 protocol differs')
        r=dict(zip(['lease','actor','unit','count','current','pending','error'],struct.unpack_from('<7I',data,20)))
        if r['count']>33 or (r['count'] and r['current']>=r['count']):raise RuntimeError('Ammo83 count differs')
        r['tick_ms']=struct.unpack_from('<Q',data,48)[0];r['items']=[]
        for i in range(r['count']):
            off=56+i*144;x=dict(zip(['ability','projectile','icon','flags'],struct.unpack_from('<4I',data,off)))
            x['label']=data[off+16:off+144].decode('utf-16-le').split('\0')[0];r['items'].append(x)
        return r
    return {'unstable':True}

accuracy_handle90=k.OpenFileMappingW(4,False,fr'Local\Shogun2Accuracy90-{pid}')
accuracy_view90=k.MapViewOfFile(accuracy_handle90,4,0,0,112) if accuracy_handle90 else None
def accuracy_snapshot90():
    if not accuracy_view90:return {'unavailable':True}
    for _ in range(4):
        seq=struct.unpack('<i',C.string_at(accuracy_view90+16,4))[0]
        if seq&1:continue
        raw90=C.string_at(accuracy_view90,112)
        if seq!=struct.unpack('<i',C.string_at(accuracy_view90+16,4))[0]:continue
        return decode_accuracy90(raw90,pid)
    return {'unstable':True}

started=k.GetTickCount64();root=opts.output_dir;root.mkdir(parents=True,exist_ok=True)
dest=root/f'live-input{opts.qte_version}-{pid}-{started}.jsonl'
source=opts.native_log if opts.native_log is not None else root/f'qte-playable{opts.qte_version}-{pid}.jsonl'
stop_file=dest.with_suffix('.stop')
bookmark={'pid':pid,'started_tick_ms':started,'source_log':str(source),'source_offset':source.stat().st_size if source.exists() else None,'input_log':str(dest),'stop_file':str(stop_file),'kind':'read-only mouse/native/memory telemetry; no video','duration_seconds':opts.duration,'memory_interval_ms':1000,'address_space_interval_ms':5000}
last_buttons=None;next_pose=0;next_memory=0;next_map=0;rows=0;errors=0;memory_samples=0;memory_errors=0;reason='interrupted';exit_code=None
def running():
    code=W.DWORD()
    if not k.GetExitCodeProcess(p.handle,C.byref(code)):return False,None
    return code.value==259,code.value
def snapshot(raw):
    env=p.u32(p.u32(base+0x1bc8180)+0x110);camera=p.u32(p.u32(env+0x281fc)+0x8e0)
    if p.u32(camera)!=base+0x15d22e8:return {'camera_valid':False}
    view={'camera_eye':list(struct.unpack('<3f',p.read(camera+8,12))),'camera_target':list(struct.unpack('<3f',p.read(camera+0x14,12))),'camera_up':list(struct.unpack('<3f',p.read(camera+0x58,12)))}
    actor=p.u32(camera+0x188)
    if not actor:return {'actor_valid':False,**view}
    a=p.read(actor,0x700)
    word=lambda off:struct.unpack_from('<I',a,off)[0]
    flt=lambda off:struct.unpack_from('<f',a,off)[0]
    if not human(base,word(0)) or word(0x1c8)!=struct.unpack_from('<I',raw,72)[0]:return {'actor_valid':False,**view}
    result={'actor':actor,'alive':a[0xf0],'unit':word(0x214),'witness':word(0x688),'state':word(0x1d0),'planned_clip':word(0x1d4),'current_mode':word(0x1f0),'render_record':word(0x474),'position':[flt(0x48+i*4) for i in range(3)],'velocity':[flt(0x8c+i*4) for i in range(3)],'bearing':struct.unpack_from('<H',a,0x64)[0], 'native_alignment_target':list(struct.unpack_from('<3f',a,0x69c)), 'native_alignment_bearing':struct.unpack_from('<H',a,0x6a8)[0], 'native_motion_task':word(0x68c)}
    result['animation_frame']=word(0x1d8)
    result['simulation_tick']=p.u32(p.u32(env+8)+0x50)
    animation=word(0x474)
    if animation:
        asset=p.u32(animation+4)
        if asset:result['animation_asset_frames']=p.u32(asset+0x64)
    principal=actor
    mount=word(0x210)
    if mount:
        b=p.read(mount,0x700)
        bw=lambda off:struct.unpack_from('<I',b,off)[0]
        links=bw(0x6d0)
        if (bw(0)==base+0x15e8524 and bw(0x214)==word(0x214) and
            bw(0x44)==word(0x44) and bw(0x28)==word(0x28) and word(0x21c)==0 and
            bw(0x6cc)==1 and links and p.u32(links)==actor):
            principal=mount
            result['mounted_subject']={'actor':mount,'actor_id':bw(0x1c8),'kind':'horse',
                'rider':actor,'rider_id':word(0x1c8),'alive':b[0xf0],
                'unit':bw(0x214),'witness':bw(0x688),'state':bw(0x1d0),
                'planned_clip':bw(0x1d4),'current_mode':bw(0x1f0),
                'charge_flags':b[0x119],'position':list(struct.unpack_from('<3f',b,0x48)),
                'velocity':list(struct.unpack_from('<3f',b,0x8c)),
                'native_alignment_target':list(struct.unpack_from('<3f',b,0x69c)),
                'bearing':struct.unpack_from('<H',b,0x64)[0]}
    witness=result['mounted_subject']['witness'] if 'mounted_subject' in result else result['witness']
    result.update(view)
    if witness and p.u32(witness)==base+0x15e1550:
        w=p.read(witness,0x100);q=lambda off:struct.unpack_from('<I',w,off)[0]
        result['combat']={'active':w[0xc],'start_requested':w[0xd],'phase':w[0xd0],'playing':w[0xd1],'aligned':w[0xd2],'count':q(0x24),'principals':q(0x38),'outcome':q(0x80),'attacker':q(0x84),'defender':q(0x88),'matched':q(0x98),'alternate':w[0xee],'deadline':struct.unpack_from('<f',w,0xdc)[0]}
        if q(0x38)==2 and principal in (q(0x30),q(0x34)):
            other=q(0x34) if q(0x30)==principal else q(0x30)
            b=p.read(other,0x700)
            if combat_actor(base,struct.unpack_from('<I',b,0)[0]) and struct.unpack_from('<I',b,0x688)[0]==witness:
                result['opponent']={'actor':other,'actor_id':struct.unpack_from('<I',b,0x1c8)[0],'alive':b[0xf0],'position':list(struct.unpack_from('<3f',b,0x48)),'native_alignment_target':list(struct.unpack_from('<3f',b,0x69c)),'bearing':struct.unpack_from('<H',b,0x64)[0], 'native_alignment_bearing':struct.unpack_from('<H',b,0x6a8)[0], 'native_motion_task':struct.unpack_from('<I',b,0x68c)[0], 'state':struct.unpack_from('<I',b,0x1d0)[0], 'planned_clip':struct.unpack_from('<I',b,0x1d4)[0]}
    return result
try:
    with dest.open('w',buffering=1) as out:
        out.write(json.dumps({'event':'recording_started',**bookmark})+'\n')
        first_memory=memory_sample(p.handle,include_address_space=True)
        if 'private_bytes' not in first_memory or not first_memory['address_space']['regions']:
            raise RuntimeError('Memory recording could not confirm readable process counters/map: '+json.dumps(first_memory))
        now=k.GetTickCount64()
        out.write(json.dumps({'event':'memory_sample','tick_ms':now,'startup':True,**first_memory})+'\n')
        memory_samples=1;memory_errors=int(bool(first_memory.get('counter_errors')))
        rows+=1;next_memory=now+1000;next_map=now+5000
        bookmark['memory_profile_confirmed']=True
        bookmark['first_memory_tick_ms']=now
        # Publish readiness only after the first counters/map have been flushed.
        ready_file=root/f'active-live-input{opts.qte_version}.json'
        ready_temp=ready_file.with_suffix('.tmp')
        ready_temp.write_text(json.dumps(bookmark,indent=2))
        ready_temp.replace(ready_file)
        print(json.dumps({'recording':'active','pid':pid,'path':str(dest),'duration_seconds':opts.duration,'stop_file':str(stop_file),'memory_profile_confirmed':True}),flush=True)
        while True:
            live,code=running()
            if not live:reason='game_process_exited';exit_code=code;break
            if stop_file.exists():reason='requested_stop';break
            if opts.duration is not None and k.GetTickCount64()-started>=opts.duration*1000:reason='duration_elapsed';break
            now=k.GetTickCount64();foreground=W.DWORD();u.GetWindowThreadProcessId(u.GetForegroundWindow(),C.byref(foreground));focus=foreground.value==pid
            raw=C.string_at(v,84);mode=struct.unpack_from('<I',raw,24)[0];seq=struct.unpack_from('<I',raw,44)[0]
            if now>=next_memory:
                memory=memory_sample(p.handle,include_address_space=now>=next_map)
                out.write(json.dumps({'event':'memory_sample','tick_ms':now,'focused':focus,'mode':mode,**memory})+'\n')
                memory_samples+=1;rows+=1;memory_errors+=int(bool(memory.get('counter_errors')))
                next_memory=k.GetTickCount64()+1000
                if 'address_space' in memory:next_map=k.GetTickCount64()+5000
            left=focus and bool(u.GetAsyncKeyState(1)&0x8000);right=focus and bool(u.GetAsyncKeyState(2)&0x8000)
            controls={name:focus and bool(u.GetAsyncKeyState(code)&0x8000) for name,code in [('weapon1',0x31),('weapon2',0x32),('tab',0x09),('ctrl',0x11),('w',0x57),('a',0x41),('s',0x53),('d',0x44),('f9',0x78),('f10',0x79),('escape',0x1b),('insert',0x2d),('f12',0x7b)]}
            buttons=(focus,left,right,mode,*controls.values())
            if buttons!=last_buttons:
                out.write(json.dumps({'event':'input_state','tick_ms':now,'focused':focus,'left':left,'right':right,'mode':mode,**controls})+'\n');last_buttons=buttons;rows+=1
            if focus and mode==2 and not seq%2 and now>=next_pose:
                row={'event':'native_pose','tick_ms':now,'left':left,'right':right,'mode':mode,'actor_id':struct.unpack_from('<I',raw,72)[0],'unit_id':struct.unpack_from('<I',raw,64)[0],'lease':struct.unpack_from('<I',raw,76)[0]}
                try:row.update(snapshot(raw));row['equipment81']=equipment_snapshot81();row['ammunition83']=ammunition_snapshot83();row['accuracy90']=accuracy_snapshot90()
                except OSError as e:
                    live,code=running()
                    if not live:reason='game_process_exited';exit_code=code;break
                    row['read_error']=str(e);errors+=1
                out.write(json.dumps(row)+'\n');rows+=1;next_pose=now+50
            time.sleep(.01)
        out.write(json.dumps({'event':'recording_finished','tick_ms':k.GetTickCount64(),'rows':rows,'read_errors':errors,'memory_samples':memory_samples,'memory_errors':memory_errors,'reason':reason,'process_exit_code':exit_code})+'\n')
finally:
    if accuracy_view90:k.UnmapViewOfFile(accuracy_view90)
    if accuracy_handle90:k.CloseHandle(accuracy_handle90)
    k.UnmapViewOfFile(av);k.CloseHandle(ah);p.close();k.UnmapViewOfFile(ev);k.CloseHandle(eh);k.UnmapViewOfFile(v);k.CloseHandle(h)
bookmark.update(finished_tick_ms=k.GetTickCount64(),rows=rows,read_errors=errors,memory_samples=memory_samples,memory_errors=memory_errors,reason=reason,process_exit_code=exit_code)
dest.with_suffix('.session.json').write_text(json.dumps(bookmark,indent=2))
print(json.dumps({'recording':'complete','path':str(dest),'rows':rows,'read_errors':errors,'reason':reason,'process_exit_code':exit_code}),flush=True)
