"""First-party native Steam Input defaults, not a keyboard/mouse community layout."""
from pathlib import Path

SET = 'DeckHardware174'
BUTTONS = ['A','B','X','Y','L1','R1','L3','R3','Up','Down','Left','Right',
           'View','Menu','L4','L5','R4','R5','LeftPadTouch','RightPadTouch',
           'LeftPadClick','RightPadClick','LeftStickTouch','RightStickTouch']
ANALOG = ['LeftStick','RightStick','LeftPad','RightPad','L2','R2']


def node(pairs, depth=0):
    text=[]
    for key,value in pairs:
        if any(c in key for c in '\n\r"\\'):
            raise ValueError('Invalid VDF key')
        if isinstance(value,list):
            text.extend(['\t'*depth+'"'+key+'"','\t'*depth+'{',node(value,depth+1),'\t'*depth+'}'])
        else:
            value=str(value)
            if any(c in value for c in '\n\r"\\'):
                raise ValueError('Invalid VDF string')
            text.append('\t'*depth+'"'+key+'" "'+value+'"')
    return '\n'.join(text)


def actions():
    return [(SET,[('title','#DeckHardware174'),
        ('StickPadGyro',[(n,[('title','#'+n),('input_mode','joystick_move')]) for n in ANALOG[:4]]),
        ('AnalogTrigger',[(n,'#'+n) for n in ANALOG[4:]]),
        ('Button',[(n,'#'+n) for n in BUTTONS])])]


def manifest():
    return node([('Action Manifest',[
        ('configurations',[('controller_neptune',[('0',[('path','deck-native174.vdf')])])]),
        ('actions',actions()),
        ('localization',[('english',[(n,n) for n in [SET,*BUTTONS,*ANALOG]])])])])+'\n'


def press(action, soft=False):
    return [('activators',[('Soft_Press' if soft else 'Full_Press',[
        ('bindings',[('binding','game_action '+SET+' '+action)])])])]


def configuration():
    groups=[];bindings=[]
    def group(identifier,mode,source,inputs,analog=None):
        values=[('id',str(identifier)),('mode',mode)]
        if inputs:values.append(('inputs',inputs))
        if analog:
            values.extend([('gameactions',[(SET,analog)]),('settings',[
                ('virtual_mode','1'),('deadzone_inner_radius','0'),
                ('deadzone_outer_radius','32767'),('haptic_intensity','0')])])
        groups.append(('group',values));bindings.append((str(identifier),source+' active'))
    group(0,'four_buttons','button_diamond',[(k,press(v)) for k,v in
        [('button_a','A'),('button_b','B'),('button_x','X'),('button_y','Y')]])
    group(1,'switches','switch',[(k,press(v)) for k,v in [
        ('left_bumper','L1'),('right_bumper','R1'),('button_menu','View'),('button_escape','Menu'),
        ('button_back_left_upper','L4'),('button_back_left','L5'),
        ('button_back_right_upper','R4'),('button_back_right','R5')]])
    group(2,'dpad','dpad',[(k,press(v)) for k,v in [
        ('dpad_north','Up'),('dpad_south','Down'),('dpad_west','Left'),('dpad_east','Right')]])
    group(3,'joystick_move','joystick',[
        ('click',press('L3')),('touch',press('LeftStickTouch'))],'LeftStick')
    group(4,'joystick_move','right_joystick',[
        ('click',press('R3')),('touch',press('RightStickTouch'))],'RightStick')
    group(5,'joystick_move','left_trackpad',[
        ('click',press('LeftPadClick',True)),('touch',press('LeftPadTouch'))],'LeftPad')
    group(6,'joystick_move','right_trackpad',[
        ('click',press('RightPadClick',True)),('touch',press('RightPadTouch'))],'RightPad')
    group(7,'trigger','left_trigger',[],'L2')
    group(8,'trigger','right_trigger',[],'R2')
    value=[('controller_mappings',[
        ('version','3'),('revision','2'),('title','Shogun 2 native Deck input proof'),
        ('description','Native API actions only; no keyboard, mouse or virtual gamepad outputs.'),
        ('controller_type','controller_neptune'),('major_revision','1'),('minor_revision','1'),
        ('actions',actions()),*groups,
        ('preset',[('id','0'),('name',SET),('group_source_bindings',bindings)]),
        ('localization',[('english',[(n,n) for n in [SET,*BUTTONS,*ANALOG]])])])]
    return node(value)+'\n'


def validate(text):
    if any(x in text.lower() for x in ('key_press ', 'mouse_button ', 'xinput_', 'steam_action ', 'radial_menu')):
        raise ValueError('Emulated/system/Steam radial output refused')
    expected={'game_action '+SET+' '+b for b in BUTTONS}
    import re
    actual=set(re.findall(r'"binding" "([^"]+)"',text))
    if actual!=expected:
        raise ValueError('Native button binding closure differs')
    if 'controller_neptune' not in text:
        raise ValueError('Deck-only type missing')


def write(folder):
    folder=Path(folder);folder.mkdir(parents=True,exist_ok=False)
    values={'deck-actions174.vdf':manifest(),'deck-native174.vdf':configuration()}
    validate(values['deck-native174.vdf'])
    for name,text in values.items():(folder/name).write_text(text,encoding='utf-8')
    return values


if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
    print('\n'.join(write(p.parse_args().output)))
