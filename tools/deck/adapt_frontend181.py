"""Preserve native roots, attach contextual frontend/campaign input and prompts."""
from pathlib import Path
from adapt_runtime175 import substitute
ROOT=Path(__file__).resolve().parents[2]
EXPORTS=('prepare','supported','poll','stop','active','focused','ready','bindings','failed','glyphs','glyph_failed',
 'cancel','heartbeat','error','up','down','left','right','focus','attach','detach','campaign_prepare','pause','next','previous',
 'home','arts','finance','diplomacy','clan','end_turn','quick_save','move_speed','encyclopedia',
 'pan_left','pan_right','pan_up','pan_down','turn_left','turn_right','viewport','context','camera','legend184','ui_viewport185','panel188','back188')
def injection(snapshot,context):
    source=(snapshot/'native/deck/frontend181.lua').read_text();nav=(snapshot/'native/deck/navigation181.lua').read_text()
    assert ']==]'not in source+nav
    binding='\n'.join('        api.'+name+'=assert(package.loadlib(path,"s2up_menu_'+name+'181"))'for name in EXPORTS)+'\n        api.click183=assert(package.loadlib(path,"s2up_menu_click183"))'
    return '''
    do
        local api={}
'''+binding+'''
        if api.supported(true)then
            local ok,why=pcall(function()
                assert(api.attach("'''+context+''':"..tostring(Address)),"Native controller context unavailable")
                assert(api.prepare(true),"Native controller startup failed")
                local chunk=assert(loadstring([==[
'''+source+'''
]==],"@first-party/deck-frontend181"));setfenv(chunk,getfenv(1))
                local navigation=assert(loadstring([==[
'''+nav+'''
]==],"@first-party/deck-navigation181"));setfenv(navigation,getfenv(1))
                return chunk(UIComponent(Address),api,path..".deck177",navigation(),"'''+context+'''")
            end)
            if not ok then api.error(tostring(why))end
        end
    end
'''
def generate(snapshot):
    snapshot=Path(snapshot).resolve()
    if snapshot==ROOT or ROOT.is_relative_to(snapshot):raise ValueError('Use isolated source snapshot')
    entry=substitute(snapshot/'tools/workshop/entry165.cpp',[(
      'extern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* state)',
      '#include "../../native/deck/frontend181.h"\nextern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* state)')])
    entry_path=snapshot/'tools/workshop/entry165.cpp'
    text=entry_path.read_text()
    text=text.replace('    prepared165=false;if(root_guard165', '    \n#ifndef S2UP165_LUA_FIXTURE\n    require(device192::select(),"Device profile unavailable");\n    if(device192::enabled()){\n        require(deck205::edges.install(reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll")),true),"Native Deck cursor-edge camera suppression unavailable");\n        require(deck185::install_ui185(reinterpret_cast<uintptr_t>(GetModuleHandleW(L"Empire.Retail.dll"))),"Native Deck UI scaling unavailable");\n    }\n#endif\n    prepared165=false;if(root_guard165')
    entry_path.write_text(text)
    bootstrap=substitute(snapshot/'tools/workshop/bootstrap165.lua',[(
      '    run_stock167(original)\n    assert(start(true)',
      '    run_stock167(original)\n'+injection(snapshot,'menu')+'    assert(start(true)')])
    build=substitute(snapshot/'tools/workshop/build_workshop165.py',[(
      "libs = ['bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib']",
      "libs = ['bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib', 'gdiplus.lib', 'gdi32.lib']")])
    builder=snapshot/'tools/workshop/build_workshop165.py'
    text=builder.read_text();text=text.replace("    run(\n        'compile',", "    vendor=ROOT/'native/outputs/shogun2-accuracy90-preview/vendor'\n    hooks=[]\n    for source in ['buffer.c','hook.c','trampoline.c','hde/hde32.c']:\n        obj=out/('bridge-'+Path(source).stem+'.obj');hooks.append(obj)\n        run('bridge-'+Path(source).stem, [['cl','/nologo','/O2','/MT','/W3','/I'+str(vendor/'include'),'/c',vendor/'src'/source,'/Fo'+str(obj)]])\n    run(\n        'compile',")
    text=text.replace("                work / 'entry165.cpp',\n                res,", "                work / 'entry165.cpp',\n                res,*hooks,")
    builder.write_text(text)
    pack=substitute(snapshot/'tools/deck/pack174_generated.h' ,[(
      'bool stock,std::string* metadata=nullptr){',
      'bool stock,std::string* metadata=nullptr,const char* target_root="ui/frontend ui/layout_scripts/root.luac",unsigned expected_size=13739,const char* expected_sha=root_sha){'),
      ('if(name=="ui/frontend ui/layout_scripts/root.luac")','if(name==target_root)'),
      ('(!stock||root_size==13739)','(!stock||root_size==expected_size)'),
      ('==root_sha,"Owned root script fingerprint differs"','==expected_sha,"Owned root script fingerprint differs"')])
    compatibility=substitute(snapshot/'tools/deck/ui_compat174_generated.h',[(
      'const std::string& native_sha=""){','const std::string& native_sha="",const char* target_root="ui/frontend ui/layout_scripts/root.luac",unsigned expected_size=13739,const char* expected_sha=workshop165::root_sha){'),
      ('workshop165::ui_root168(p,false,&metadata)','workshop165::ui_root168(p,false,&metadata,target_root,expected_size,expected_sha)'),
      ('workshop165::original_root(game/L"data/data.pack")','workshop165::ui_root168(game/L"data/data.pack",true,nullptr,target_root,expected_size,expected_sha)')])
    return dict(entry=entry,bootstrap=bootstrap,private_builder=build,campaign_reader=pack,campaign_coexistence=compatibility,
      stock_callbacks_preserved=True,parenting_outside_frame_callbacks=True,root_frame_callbacks_chained=True)
def campaign_wrapper(snapshot,native_sha):
    snapshot=Path(snapshot)
    original=(ROOT/'tools/workshop/bootstrap165.lua').read_text();fallback=original[:original.index('local function initialize165()')]
    fallback=fallback.replace('ui/frontend ui/layout_scripts/root.luac','ui/campaign ui/layout.root.luac').replace('@owned-game/frontend-root','@owned-game/campaign-root')
    return fallback+'''
local function initialize_campaign181()
    local path=assert(os.getenv("LOCALAPPDATA")).."/S2UP-Workshop165-'''+native_sha+'''.dll"
    local prepare=assert(package.loadlib(path,"s2up_menu_campaign_prepare181"))
    assert(prepare(true),"Owned campaign root preparation failed")
    local f=assert(io.open(path..".campaign.luac","rbD"));local original=f:read(262145);f:close()
    assert(original and #original>0 and #original<=262144,"Prepared campaign root exceeds bound")
    run_stock167(original)
'''+injection(snapshot,'campaign')+'''
end
local ok,why=pcall(initialize_campaign181)
if not ok then
    if not stock_invoked167 then pcall(fallback_stock167)end
    pcall(function()print("Unit Possession campaign controls: "..tostring(why):sub(1,512))end)
end
'''
