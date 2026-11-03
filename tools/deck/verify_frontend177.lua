-- Run on the game's matching float-Lua host. Native UI is an explicit mock,
-- never claimed to prove rendering or actual controller input on Steam Deck.
local source=assert(arg[1]);local passed=0
local function test(name,fn)fn();passed=passed+1;print('PASS '..name)end
local function fixture(options)
    options=options or {};local nodes={};local next_id=0
    local clicks,backs,stops=0,0,0;local hovered;local flags={fresh=true,active=true,focused=true,ready=true}
    local methods={}
    local function node(id)
        next_id=next_id+1
        local c={id=id,address=next_id,visible=true,interactive=true,state='active',events={},callbacks={}}
        nodes[c.address]=c;return setmetatable(c,{__index=methods})
    end
    function methods:Address()assert(not self.dead);return self.address end
    function methods:Id()assert(not self.dead);return self.id end
    function methods:Visible()assert(not self.dead);return self.visible end
    function methods:IsInteractive()assert(not self.dead);return self.interactive end
    function methods:CurrentState()assert(not self.dead);return self.state end
    function methods:SetInteractive(v)assert(not self.dead);self.interactive=v end
    function methods:SetVisible(v)assert(not self.dead);self.visible=v end
    function methods:LuaCall(name,address,text)assert(not self.dead and name=='SetText' and address==self.address);self.text=text end
    function methods:Resize(w,h)assert(w>0 and h>0);self.w=w;self.h=h end
    function methods:MoveTo(x,y)self.x=x;self.y=y end
    function methods:SetImageMetrics(i,x,y,w,h)assert(i==0 and x==0 and y==0 and w==32 and h==32);self.metrics=true end
    function methods:SetEventCallback(event,callback)assert(type(callback)=='function');self.callbacks[event]=callback end
    function methods:ForceEvent(event)
        assert(not self.dead and event=='OnMouseLClickUp');self.events[#self.events+1]=event;clicks=clicks+1
        if self.destroy_on_click then self.dead=true end
    end
    local root=node('root');root.callbacks.stock=function()end;local stock=root.callbacks.stock
    local target=node('button_single_player');hovered=target.address
    local api={}
    for _,name in ipairs({'active','focused','ready','bindings','failed','glyphs','glyph_failed','confirm','cancel','pointer'})do
        local key=name;api[name]=function(v)assert(v==true);return flags[key]end
    end
    api.poll=function(v)assert(v==true);return flags.fresh end
    api.stop=function(v)assert(v==true);stops=stops+1;flags.active=false;return true end
    local images={}
    local env={UIComponent=function(address)return assert(nodes[address],'Unknown component')end,
        Component={MouseOver=function()return hovered end,CreateFromLayout=function(layout,id,parent,x,y)
            assert(parent==root.address and (layout=='data/ui/common ui/Tooltip' or layout=='data/ui/common ui/general_purpose_pixel'))
            if options.fail_create then error('Fixture missing native layout')end
            return node(id).address
        end},
        FrontEnd={ScreenSize=function()return 1280,800 end},
        UIImage=function(path)
            images[#images+1]=path;return {SetComponentTexture=function(self,address,index)
                assert(nodes[address] and index==0);nodes[address].texture=path
            end}
        end,
        TransitionBack=function()backs=backs+1 end}
    setmetatable(env,{__index=_G});local chunk=assert(loadfile(source));setfenv(chunk,env)
    local m=chunk(root,api,'owned-cache.deck177')
    return {m=m,flags=flags,nodes=nodes,target=target,root=root,stock=stock,images=images,
        counts=function()return clicks,backs,stops end,
        hover=function(c)hovered=c and c.address end,node=node,
        pulse=function()m.status.callbacks.OnUpdatePulse()end,
        draw=function()m.status.callbacks.OnPreDraw()end}
end
test('native prompt layouts, text and bottom anchoring',function()
    local f=fixture();assert(#f.m.panels==4 and #f.m.icons==4)
    assert(f.m.panels[1].text=='Select' and f.m.panels[3].text=='Move cursor')
    assert(f.m.status.text=='Steam Deck menu controls' and f.m.panels[1].y==745)
    for _,c in ipairs(f.m.panels)do assert(not c.interactive)end
    for _,c in ipairs(f.m.icons)do assert(not c.interactive and not c.visible)end
end)
test('all existing root callbacks untouched',function()local f=fixture();assert(f.root.callbacks.stock==f.stock and not f.root.callbacks.OnPreDraw)end)
test('Steam glyph files populate native UI textures',function()
    local f=fixture();f.flags.glyphs=true;f.pulse();f.draw();assert(#f.images==4)
    for _,c in ipairs(f.m.icons)do assert(c.visible and c.metrics and c.texture:match('%.tga$'))end
end)
test('A activates engine hovered component without navigation first',function()local f=fixture();f.flags.confirm=true;f.pulse();assert(f.counts()==1)end)
test('native modal hit-test chooses modal instead of background',function()
    local f=fixture();local modal=f.node('button_ok');f.hover(modal);f.flags.confirm=true;f.pulse()
    assert(#modal.events==1 and #f.target.events==0)
end)
test('hidden disabled and noninteractive targets ignored',function()
    for _,kind in ipairs({'hidden','disabled','inactive','noninteractive'})do
        local f=fixture();if kind=='hidden'then f.target.visible=false elseif kind=='noninteractive'then f.target.interactive=false else f.target.state=kind end
        f.flags.confirm=true;f.pulse();assert(f.counts()==0)
    end
end)
test('empty native hover does nothing',function()local f=fixture();f.hover(nil);f.flags.confirm=true;f.pulse();assert(f.counts()==0)end)
test('owned prompt cannot intercept activation',function()local f=fixture();f.hover(f.m.panels[1]);f.flags.confirm=true;f.pulse();assert(f.counts()==0)end)
test('activation never touches a button after it destroys itself',function()
    local f=fixture();f.target.destroy_on_click=true;f.flags.confirm=true;f.pulse();assert(f.m.live and f.target.dead and f.counts()==1)
end)
test('B uses native frontend Back once',function()local f=fixture();f.flags.cancel=true;f.pulse();local a,b=f.counts();assert(a==0 and b==1)end)
test('confirm and cancel do not activate two actions',function()local f=fixture();f.flags.confirm=true;f.flags.cancel=true;f.pulse();local a,b=f.counts();assert(a==1 and b==0)end)
test('simultaneous pointer motion waits for native hover message',function()
    local f=fixture();f.flags.confirm=true;f.flags.pointer=true;f.pulse();assert(f.counts()==0)
    local next_button=f.node('button_next');f.hover(next_button);f.flags.confirm=false;f.flags.pointer=false;f.pulse()
    assert(#next_button.events==1 and #f.target.events==0)
end)
test('coalesced duplicate UI events cannot duplicate an edge',function()
    local f=fixture();f.flags.confirm=true;f.pulse();f.flags.fresh=false;f.draw();assert(f.counts()==1)
end)
test('focus loss cancels delayed activation',function()
    local f=fixture();f.flags.pointer=true;f.flags.confirm=true;f.pulse()
    f.flags.focused=false;f.flags.fresh=false;f.draw()
    f.flags.focused=true;f.flags.fresh=true;f.flags.pointer=false;f.flags.confirm=false;f.pulse();assert(f.counts()==0)
end)
test('battle handoff hides owned UI without changing game components',function()
    local f=fixture();f.flags.active=false;f.pulse();assert(not f.m.live and not f.m.status.visible and f.target.visible)
    for _,c in ipairs(f.m.panels)do assert(not c.visible)end
end)
test('destroyed child shuts down only its owned reader',function()
    local f=fixture();f.m.status.callbacks.OnDestroyed();local a,b,c=f.counts();assert(c==1 and not f.m.live)
end)
test('bindings wait is visible instead of silently inert',function()
    local f=fixture();f.flags.ready=false;f.flags.bindings=true;f.pulse();assert(f.m.status.text=='Waiting for Steam native bindings...')
end)
test('native failure status is visible and polling exception stops safely',function()
    local f=fixture();f.flags.failed=true;f.pulse();assert(f.m.status.text=='Controller support could not initialize')
    f.flags.failed=false;f.target.dead=true;f.flags.confirm=true;f.pulse()
    assert(not f.m.live and f.m.status.text:find('stopped'))
end)
print('PASS '..passed..' frontend177 float-Lua UI policy checks; native game UI/Deck execution unverified')
