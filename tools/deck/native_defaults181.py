"""Separate native Steam Input sets, with distinct handles for each game context."""
import json,re
import native_defaults174 as old
SETS=('Menu','Campaign','RTS','Possession')
NAMES=set(old.BUTTONS+old.ANALOG)
def parse(text):
    tokens=re.findall(r'"(?:[^"\\]|\\.)*"|[{}]',text);at=0
    def pairs(nested=False):
        nonlocal at
        result=[]
        while at<len(tokens):
            if tokens[at]=='}':
                if not nested:raise ValueError('Unexpected VDF close')
                at+=1;return result
            key=json.loads(tokens[at]);at+=1
            if tokens[at]=='{':at+=1;value=pairs(True)
            else:value=json.loads(tokens[at]);at+=1
            result.append((key,value))
        if nested:raise ValueError('Unclosed VDF block')
        return result
    result=pairs()
    if at!=len(tokens):raise ValueError('Trailing tokens')
    return result
def replace(value,context):
    prefix=context+'_';setname=context+'181'
    if isinstance(value,list):return [(setname if k==old.SET else prefix+k if k in NAMES else k,replace(v,context))for k,v in value]
    if value==old.SET:return setname
    if value in NAMES:return prefix+value
    if value=='#'+old.SET:return '#'+setname
    if value.startswith('#') and value[1:] in NAMES:return '#'+prefix+value[1:]
    if value.startswith('game_action '+old.SET+' '):return 'game_action '+setname+' '+prefix+value.split(' ')[-1]
    return value


# These are Steam's player-visible labels. Internal action/set identifiers and
# physical bindings stay stable; unused inputs must not advertise an action.
CONFIG_MAJOR=3
CONFIG_MINOR=6

SET_TITLES = {'Menu': 'Menus', 'Campaign': 'Campaign', 'RTS': 'Battle (RTS)', 'Possession': 'Possession'}
LABELS = {
    'Menu': dict(A='Select', B='Back', L2='Right click', R2='Select / drag',
        Menu='Skip intro / back', View='Hold: controls guide',
        RightPad='Move cursor', LeftPad='Scroll',
        LeftPadTouch='Enable scrolling (touch)', RightPadTouch='Enable cursor (touch)'),
    'Campaign': dict(A='Toggle movement speed (Space)',B='Cancel / back',X='Zoom home',Y='End turn (hold)',L1='Previous selection',R1='Next selection',
        L3='Hold: faster map pan / CTRL selection',Menu='Game menu',View='Hold: controls guide',Up='Mastery of Arts',Down='Finances',Left='Diplomacy',Right='Clan Management',
        L4='SHIFT: select card range (hold)',L5='Add / remove selection (hold)',R4='Quick-save (hold)',R5='Toggle character movement speed',
        LeftStick='Pan campaign map',RightStick='Turn / zoom camera',LeftPad='Scroll / zoom',RightPad='Cursor; panel L1 + swipe: scroll',L2='Move / attack',R2='Select / interact',
        LeftPadTouch='Enable scrolling (touch)',RightPadTouch='Enable cursor (touch)'),
    'RTS': dict(A='Show formations / range (hold)',B='Cancel placement / deselect',X='Toggle run / walk',Y='Possess soldier',L1='Zoom in (hold)',R1='Zoom out (hold)',
        L3='Add / remove selection (hold)',Menu='Game menu',View='Hide / cycle HUD; hold: controls guide',Up='Group 1: tap to select / hold to assign',Down='Group 2: tap to select / hold to assign',Left='Group 3: tap to select / hold to assign',Right='Group 4: tap to select / hold to assign',
        L4='SHIFT: range / queue; + R4: select all',L5='Add selection / group modifier (hold)',R4='Melee mode; hold L4: select all',R5='Group / ungroup; L5: lock formation',
        LeftStick='Pan battle camera',RightStick='Turn / tilt camera',LeftPad='Zoom camera',RightPad='Move cursor',L2='Move / attack / formation; double: run',R2='Select / drag box; double: focus unit',
        LeftPadTouch='Enable zoom (touch)',RightPadTouch='Enable cursor (touch)'),
    'Possession': dict(A='Climb / interact',B='Mount / dismount unit',X='Toggle unit melee / ranged',Y='Return to RTS',L1='Unit order / formation; double: run',R1='Command wheel (hold)',
        L3='Run (hold)',R3='Change first / third person (hold)',Menu='Game menu (remain possessed)',View='Hide / cycle HUD; hold: controls guide',Up='Cycle ability / ammunition',Down='Use / end ability',Left='Third-person camera closer',Right='Third-person camera farther',
        L4='Orbit camera (hold)',L5='Crouch (hold)',R4='Stop unit',R5='Follow Me',
        LeftStick='Move soldier',RightStick='Look / command wheel choice',LeftPad='Third-person camera distance',RightPad='Look / touch for gyro',L2='Ready weapon / secondary melee response',R2='Fire / hold-release bow / primary melee response',
        LeftPadTouch='Enable camera zoom (touch)',RightPadTouch='Enable look / gyro (touch)'),
}

def labels(context):
    explicit = LABELS[context]
    if not set(explicit) <= NAMES:
        raise ValueError('Unknown controller input in labels')
    return {name: explicit[name] for name in old.BUTTONS + old.ANALOG if name in explicit}

def binding_semantics(text):
    """Ignore display-only metadata; retain every action, preset and binding."""
    def strip(pairs):
        return [(k, strip(v) if isinstance(v, list) else v)
                for k, v in pairs if k not in ('title', 'description', 'localization')]
    return strip(parse(text))


def values():
    base=parse(old.configuration())[0][1];actions=[];groups=[];presets=[];localization=[]
    for index,context in enumerate(SETS):
        active={context+'_'+n for n in labels(context)}
        action=replace(old.actions(),context)
        action=[(key,[(kind,[(k,v)for k,v in items if k in active]if isinstance(items,list)else items)for kind,items in value])for key,value in action]
        actions.extend(action)
        offset=index*16
        for key,value in base:
            if key=='group':
                group=replace(value,context);filtered=[];has_input=False
                for k,v in group:
                    if k=='id':v=str(int(v)+offset)
                    if k=='inputs':
                        v=[(physical,setting)for physical,setting in v if any(token in active for token in re.findall(r'game_action [^ ]+ ([^" ]+)',old.node(setting)))];has_input|=bool(v)
                    if k=='gameactions':v=[(a,b)for a,b in v if b in active];has_input|=bool(v)
                    filtered.append((k,v))
                if has_input:groups.append((key,filtered))
            if key=='preset':
                preset=replace(value,context);preset=[(k,str(index)if k=='id' else [(str(int(a)+offset),b)for a,b in v if str(int(a)+offset)in {dict(g)['id']for _,g in groups}]if k=='group_source_bindings'else v)for k,v in preset];presets.append((key,preset))
        localization.append((context+'181', SET_TITLES[context]))
        localization.extend((context+'_'+n, label) for n, label in labels(context).items())
    config=[('version','3'),('revision',str(CONFIG_MINOR)),('title','Unit Possession Layout'),('description','Automatic control schemes. Browse Menus, Campaign, Battle (RTS), and Possession action sets to see each binding.'),('controller_type','controller_neptune'),('major_revision',str(CONFIG_MAJOR)),('minor_revision',str(CONFIG_MINOR)),('actions',actions),*groups,*presets,('localization',[('english',localization)])]
    manifest=old.node([('Action Manifest',[('major_revision',str(CONFIG_MAJOR)),('minor_revision',str(CONFIG_MINOR)),('major_revision_affects_xinput','0'),('major_revision_affects_mousekb','0'),('configurations',[('controller_neptune',[('0',[('path','deck-native174.vdf')])])]),('actions',actions),('localization',[('english',localization)])])])+'\n'
    bindings=old.node([('controller_mappings',config)])+'\n'
    validate(manifest,bindings);return manifest,bindings

def validate(manifest,bindings):
    for context in SETS:
        for n in labels(context):
            if n not in old.BUTTONS:continue
            assert 'game_action '+context+'181 '+context+'_'+n in bindings
        assert '"'+context+'181"' in manifest
    assert not any(x in bindings.lower()for x in ('key_press ','mouse_button ','xinput_'))
    # Parse readback, retaining repeated VDF group/preset keys.
    for text in (manifest,bindings):assert parse(text)
