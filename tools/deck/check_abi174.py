"""Check our small ABI adapter against separately obtained Valve Proton declarations."""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[2]
PROTON_COMMIT='e91ca2be0df2cef4c230cbbc0b86604d73a0bbf6'
SLOTS={0:'Init',1:'Shutdown',2:'SetInputActionManifestFilePath',3:'RunFrame',6:'GetConnectedControllers',
       9:'GetActionSetHandle',10:'ActivateActionSet',11:'GetCurrentActionSet',16:'GetDigitalActionHandle',
       17:'GetDigitalActionData',18:'GetDigitalActionOrigins',20:'GetAnalogActionHandle',
       21:'GetAnalogActionData',22:'GetAnalogActionOrigins',23:'GetGlyphPNGForActionOrigin',29:'GetMotionData',
       36:'ShowBindingPanel',37:'GetInputTypeForHandle',39:'GetGamepadIndexForController',45:'GetRemotePlaySessionID'}
NAMES=['A','B','X','Y','L1','R1','L3','R3','Up','Down','Left','Right','View','Menu',
       'L4','L5','R4','R5','LeftPadTouch','RightPadTouch','LeftPadClick','RightPadClick','LeftStickTouch','RightStickTouch']
ORIGINS=['A','B','X','Y','L1','R1','L3','R3','DPad_North','DPad_South','DPad_West','DPad_East',
         'View','Menu','L4','L5','R4','R5','LeftPad_Touch','RightPad_Touch','LeftPad_Click','RightPad_Click',
         'LeftStick_Touch','RightStick_Touch']


def inspect(reference):
    assert hashlib.sha256((reference/'isteaminput.h').read_bytes()).hexdigest()=='277e53a45bfae91967fa113f1c1ab876f24629b14b9a3eae06cfa5af19397569'
    assert hashlib.sha256((reference/'steam_api_flat.h').read_bytes()).hexdigest()=='5244d56f6fbc91df5f41a689a068b1aa3253d56ee866d7ec2422eff3b61edfb8'
    utils_path=reference/'isteamutils.h'
    assert hashlib.sha256(utils_path.read_bytes()).hexdigest()=='f9200ff54fca94435dad516db0685727d89787957ade4012ccf4a3bcea1f0be2'
    utils=utils_path.read_text()
    methods=re.findall(r'virtual\s+[^\n;]+?\b(\w+)\s*\(',utils[utils.index('class ISteamUtils'):])
    assert len(methods)==39 and methods[34]=='IsSteamRunningOnSteamDeck'
    assert '#define STEAMUTILS_INTERFACE_VERSION "SteamUtils010"' in utils
    ours_utils=(ROOT/'native/deck/device192.h').read_text()
    our_methods=re.findall(r'virtual\s+[^\n;]+?\b(\w+)\s*\(',ours_utils)
    assert len(our_methods)==35 and our_methods[34]==methods[34]
    s=(reference/'isteaminput.h').read_text()
    assert '#define STEAMINPUT_INTERFACE_VERSION "SteamInput006"' in s
    section=s[s.index('class ISteamInput'):]
    names=re.findall(r'virtual\s+[^\n;]+?\b(\w+)\s*\(',section)
    assert len(names)==48
    assert all(names[i]==name for i,name in SLOTS.items())
    assert '#pragma pack( push, 1 )' in s
    body=re.search(r'enum EInputActionOrigin\s*\{(.*?)\};',s,re.S).group(1)
    body=re.sub(r'//[^\n]*|/\*.*?\*/','',body,flags=re.S)
    values={};current=-1
    for token in body.split(','):
        token=token.strip()
        if not token:continue
        if '=' in token:
            name,value=map(str.strip,token.split('=',1));current=int(value,0)
        else:name=token;current+=1
        values[name]=current
    ours=(ROOT/'native/deck/steam_input174.h').read_text()
    declared=dict((n,int(v)) for n,v in re.findall(r'\{"(\w+)",(\d+)\}',ours))
    assert set(declared)==set(NAMES)
    assert all(declared[n]==values['k_EInputActionOrigin_SteamDeck_'+o] for n,o in zip(NAMES,ORIGINS))
    analog_values=[values['k_EInputActionOrigin_SteamDeck_'+o] for o in
        ('LeftStick_Move','RightStick_Move','LeftPad_Swipe','RightPad_Swipe','L2_SoftPull','R2_SoftPull')]
    actual_analog=re.search(r'analog_origin\[6\]=\{([^}]+)\}',ours).group(1)
    assert [int(v) for v in actual_analog.split(',')]==analog_values
    for name,origin in [('left_trigger_soft','L2_SoftPull'),('left_trigger_full','L2'),
                        ('right_trigger_soft','R2_SoftPull'),('right_trigger_full','R2')]:
        assert int(re.search(r'\b'+name+r'=(\d+)',ours).group(1))==values['k_EInputActionOrigin_SteamDeck_'+origin]
    interface=ours[ours.index('struct Interface {'):ours.index('struct Api {')]
    our_names=re.findall(r'virtual\s+[^\n;]+?\b(\w+)\s*\(',interface)
    assert len(our_names)==48
    assert all(our_names[i]==name for i,name in SLOTS.items())
    # Value-return types need both exact packing and trivial POD layout on x86.
    assert 'struct Digital { bool state,active; };' in ours
    assert 'struct Analog { int mode;float x,y;bool active; };' in ours
    assert 'std::is_trivial_v<Digital>' in ours
    flat=(reference/'steam_api_flat.h').read_text()
    assert 'SteamAPI_ISteamClient_GetISteamUtils( ISteamClient* self, HSteamPipe hSteamPipe, const char * pchVersion )' in flat
    assert 'SteamAPI_ISteamClient_GetISteamGenericInterface( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )' in flat
    return {'passed':True,'interface':'SteamInput006','methods_checked':len(SLOTS),
        'digital_physical_origins_checked':len(NAMES),'analog_origins_checked':6,
        'same_trigger_soft_full_origins_checked':4,'source_commit':PROTON_COMMIT,
        'input_header_sha256':hashlib.sha256((reference/'isteaminput.h').read_bytes()).hexdigest(),
        'hardware_or_Proton_execution':False,'device_interface':'SteamUtils010','device_query_slot':34,'device_header_sha256':hashlib.sha256(utils_path.read_bytes()).hexdigest()}


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference-dir',type=Path,required=True)
    print(json.dumps(inspect(p.parse_args().reference_dir),indent=2))
