-- Actual compiled Workshop bootstrap/native export lookup plus explicit mock
-- Proton/UI input. Never Steam calls, game hooks or a remote Deck assertion.
local bootstrap=assert(arg[1]);local passed=0
local function run(mode)
    local nodes={};local serial=0;local creates=0;local activated=0;local went_back=0;local stops=0
    local unrequired=0
    local flags={supported=true,prepare=true,active=true,focused=true,ready=true,poll=true}
    if mode=='prepare-failure'then flags.prepare=false end
    local methods={}
    local function node(id)
        serial=serial+1;local c={id=id,address=serial,callbacks={},globals={},children={},visible=false,interactive=true}
        nodes[serial]=setmetatable(c,{__index=methods});return c
    end
    function methods:Address()return self.address end
    function methods:Adopt(address)
        for i,v in ipairs(self.children)do if v==address then table.remove(self.children,i);break end end
        table.insert(self.children,address)
    end
    function methods:Find(i)if type(i)~='number'then return 2 end;return self.children[i+1]end
    function methods:ChildCount()return #self.children end
    function methods:GlobalExists(name)return self.globals[name]~=nil end
    function methods:SetGlobal(name,fn)self.globals[name]=fn end
    function methods:SetStateText(value)self.text=value end
    function methods:Id()return self.id end
    function methods:Visible()return self.visible end
    function methods:IsInteractive()return self.interactive end
    function methods:CurrentState()return 'active'end
    function methods:SetInteractive(v)self.interactive=v end
    function methods:SetVisible(v)self.visible=v end
    function methods:SetImageColour()end
    function methods:SetImageMetrics()end
    function methods:StealShortcutKey()end
    function methods:Resize()end
    function methods:MoveTo()end
    function methods:LuaCall(name,address,text)assert(name=='SetText'and address==self.address);self.text=text end
    function methods:SetEventCallback(event,fn)assert(type(fn)=='string'and self.globals[fn]);self.callbacks[event]=self.globals[fn] end
    function methods:ForceEvent(event)assert(event=='OnMouseLClickUp');activated=activated+1 end
    function methods:InterfaceFunction(name)assert(name=='TransitionBack');went_back=went_back+1 end
    local root=node('root');node('blackout');local button=node('button_single_player')
    local lookups={};local env={Address=root.address,UIComponent=function(address)return assert(nodes[address])end,
        system={ClearRequiredFiles=function()end},require=function(name)assert(name=='Utilities');return {UnRequire=function(value)assert(value=='Utilities'or value=='CoreUtils');unrequired=unrequired+1 end}end,
        FrontEnd={ScreenSize=function()return 1280,800 end},
        Component={Adopt=function()end,MouseOver=function()return button.address end,CreateFromLayout=function(layout,id,parent,x,y)
            creates=creates+1;if mode=='UI-failure'and creates==1 then error('Fixture native layout failure')end
            assert(nodes[parent]);local c=node(id);table.insert(nodes[parent].children,c.address);return c.address
        end},
        package={path='',loadlib=function(path,name)
            local real,why=package.loadlib(path,name);assert(type(real)=='function',why)
            local key=name:match('^s2up_menu_(.+)180$')
            if not key then return real end
            lookups[key]=true
            return function(value)
                if key=='focus'then assert(type(value)=='string');return true end
                if key=='error'then assert(type(value)=='string');return end
                assert(value==true)
                if key=='stop'then stops=stops+1;return true end
                return flags[key]
            end
        end},print=function()end}
    setmetatable(env,{__index=_G});local chunk=assert(loadfile(bootstrap));setfenv(chunk,env);chunk('first',nil,'last')
    local count=0;for name,fn in pairs(env)do if type(fn)=='function'and getfenv(fn)==env then count=count+1 end end
    assert(count==37,'Stock callbacks/environment changed: '..count)
    for _,name in ipairs({'supported','prepare','poll','stop','active','focused','ready','bindings','failed','glyphs','glyph_failed','cancel','heartbeat','error','up','down','left','right','focus'})do assert(lookups[name],'Missing real native export '..name)end
    local status;for _,c in pairs(nodes)do if c.id=='s2up_deck180_status'then status=c end end
    if mode=='normal'then
        assert(creates==10 and status and status.visible and status.text:find('Test180'))
        assert(not root.callbacks.OnUpdatePulse and not root.callbacks.OnPreDraw)
        local group;for _,c in pairs(nodes)do if c.id=='s2up_deck180_group'then group=c end end;assert(group)
        flags.cancel=true;group.callbacks.OnUpdatePulse();assert(went_back==1)
        flags.poll=false;group.callbacks.OnPreDraw();assert(went_back==1)
        group.callbacks.OnDestroyed();assert(stops==1 and unrequired==0);env.OnDestroyed();assert(unrequired==2)
    else
        assert(stops==0 and activated==0,'UI failure stopped input')
    end
    passed=passed+1;print('PASS compiled frontend180 bootstrap '..mode..'; real exports/all37 callbacks; UI/backend are explicit mocks')
end
run('normal');run('UI-failure');run('prepare-failure')
print('PASS '..passed..' compiled frontend180 bootstrap integration checks; no actual Deck execution')
