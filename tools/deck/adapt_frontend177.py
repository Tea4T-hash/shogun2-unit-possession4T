"""Connect native menu input only in an isolated private source snapshot."""
import json
from pathlib import Path
from adapt_runtime175 import substitute

ROOT=Path(__file__).resolve().parents[2]
EXPORTS=('prepare','supported','poll','stop','active','focused','ready','bindings','failed','glyphs','glyph_failed',
         'confirm','cancel','pointer')

def generate(snapshot):
    snapshot=Path(snapshot).resolve()
    if snapshot==ROOT or ROOT.is_relative_to(snapshot):raise ValueError('Use an isolated source snapshot')
    entry=substitute(snapshot/'tools/workshop/entry165.cpp',[
        ('extern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* state)',
         '#include "../../native/deck/frontend177.h"\nextern "C" __declspec(dllexport) int __cdecl s2up_workshop_prepare165(void* state)')])
    source=(snapshot/'native/deck/frontend177.lua').read_text(encoding='utf-8')
    if ']==]' in source:raise ValueError('Frontend Lua delimiter collision')
    binding='\n'.join('            api.'+name+'=assert(package.loadlib(path,"s2up_deck_'+name+'177"))' for name in EXPORTS)
    injection='''
    -- Private Deck frontend only. These are owned child components; all stock
    -- root callbacks/environment remain intact. Windows prepare returns false.
    do
        local api={}
'''+binding+'''
        if api.supported(true) then
            local ok,result=pcall(function()
                assert(api.prepare(true),"Native controller startup failed; existing cache preserved")
                local chunk=assert(loadstring([==[
'''+source+'''
]==],"@first-party/deck-frontend177"))
                setfenv(chunk,getfenv(1))
                return chunk(UIComponent(Address),api,path..".deck177")
            end)
            if not ok then
                api.stop(true)
                pcall(function()
                    local address=Component.CreateFromLayout("data/ui/common ui/Tooltip","s2up_deck177_error",Address,16,16)
                    local message=UIComponent(address);message:SetInteractive(false)
                    message:LuaCall("SetText",message:Address(),"Deck menu initialization failed: "..tostring(result):sub(1,180))
                end)
            end
        end
    end
'''
    bootstrap=substitute(snapshot/'tools/workshop/bootstrap165.lua',[
        ('    run_stock167(original)\n    assert(start(true)',
         '    run_stock167(original)\n'+injection+'    assert(start(true)')])
    build=substitute(snapshot/'tools/workshop/build_workshop165.py',[
        ("libs = ['bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib']",
         "libs = ['bcrypt.lib', 'shell32.lib', 'user32.lib', 'winmm.lib', 'gdiplus.lib']")])
    return dict(entry=entry,bootstrap=bootstrap,private_builder=build,
                first_party_frontend_embedded=True,root_callbacks_replaced=False)
