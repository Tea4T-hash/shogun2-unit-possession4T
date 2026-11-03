-- Actual compiled Workshop bootstrap/native export lookup plus explicit mock
-- Proton/UI input. Never Steam calls, game hooks or a remote Deck assertion.
local bootstrap=assert(arg[1]);local campaign_bootstrap=assert(arg[2]);local reference_file=assert(io.open(assert(arg[3]),"rb"));local campaign_reference=reference_file:read("*a");reference_file:close();local passed=0
local function run(mode,campaign)
    local nodes={};local serial=0;local creates=0;local activated=0;local went_back=0;local stops=0
    local reports=0;local last_focus
    local unrequired=0;local stock_calls=0
    local flags={supported=true,prepare=true,active=true,focused=true,ready=true,poll=true}
    if mode=='desktop'then flags.supported=false end
    if mode=='prepare-failure'then flags.prepare=false end
    local methods={}
    local function node(id)
        serial=serial+1;local c={id=id,address=serial,callbacks={},globals={},children={},visible=false,interactive=true}
        nodes[serial]=setmetatable(c,{__index=methods});return c
    end
    function methods:Position()return 100,100 end
    function methods:Width()return self.id=='root'and 1280 or 120 end
    function methods:Height()return self.id=='root'and 800 or 30 end
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
    local root=node('root');node('blackout');local button=node('button_single_player');root.visible=true;root.interactive=false;button.visible=true;table.insert(root.children,button.address)
    local game_math={};for k,v in pairs(math)do if type(v)=="function"then game_math[k]=v end end
    local lookups={};local env={math=game_math,Address=root.address,UIComponent=function(address)return assert(nodes[address])end,
        system={ClearRequiredFiles=function()end},require=function(name)assert(name=='Utilities');return {UnRequire=function(value)assert(value=='Utilities'or value=='CoreUtils');unrequired=unrequired+1 end}end,
        FrontEnd={ScreenSize=function()return 1280,800 end},
        Component={Adopt=function()end,MouseOver=function()return nil end,CreateFromLayout=function(layout,id,parent,x,y)
            creates=creates+1;if mode=='UI-failure'and creates==1 then error('Fixture native layout failure')end
            assert(nodes[parent]);local c=node(id);table.insert(nodes[parent].children,c.address);return c.address
        end},
        package={path='',loadlib=function(path,name)
            local real,why=package.loadlib(path,name);assert(type(real)=='function',why)
            local key=name:match('^s2up_menu_(.+)181$')
            if not key then return real end
            lookups[key]=true
            if key=='menu_fit209'then return function(value)assert(value=='off'or(type(value)=='string'and value:match('^%d+,%d+$')));return true end end
            if key=='panel188'then return function(value)assert(value=='open'or value=='map'or value=='unknown');return true end end
            if key=='back188'then return function(value)assert(value==true);went_back=went_back+1;return true end end
            if key=='context'then return function(tag)assert(type(tag)=='string');return real(tag)end end
            return function(value)
                if key=='campaign_prepare'then if mode=='prepare-failure'then return false end;return real(value)end
                if key=='attach'then assert(type(value)=='string');return true end
                if key=='detach'then stops=stops+1;return true end
                if key=='focus'then assert(type(value)=='string');last_focus=value;return true end
                if key=='error'then assert(type(value)=='string');reports=reports+1;return end
                assert(value==true)
                if key=='stop'then stops=stops+1;return true end
                return flags[key]
            end
        end},print=function()end}
    if campaign then
        env.CampaignUI={};env.PanelManager={OpenPanels=function()return false end,CloseAllPanels=function()error("Direct panel close from draw callback is forbidden")end}
        env.loadstring=function(bytes,name)
            if name=='@owned-game/campaign-root'then
                assert(bytes==campaign_reference,'Native/fallback campaign handoff bytes differ')
                return function(...)stock_calls=stock_calls+1;assert(select('#',...)==3);local a,b,c=...;assert(a=='first'and b==nil and c=='last')end
            end
            return loadstring(bytes,name)
        end
    end
    setmetatable(env,{__index=_G});local chunk=assert(loadfile(campaign and campaign_bootstrap or bootstrap));setfenv(chunk,env);chunk('first',nil,'last')
    local count=0;for name,fn in pairs(env)do if type(fn)=='function'and getfenv(fn)==env then count=count+1 end end
    if campaign then assert(stock_calls==1,'Campaign original invoked zero/twice')else assert(count==37,'Stock callbacks/environment changed: '..count)end
    if not(campaign and mode=='prepare-failure')then for _,name in ipairs({'supported','prepare','poll','stop','active','focused','ready','bindings','failed','glyphs','glyph_failed','cancel','heartbeat','error','up','down','left','right','focus','attach','detach','campaign_prepare','pause','next','previous','pan_left','pan_right','pan_up','pan_down','turn_left','turn_right','panel188','back188'})do assert(lookups[name],'Missing real native export '..name)end end
    local status;for _,c in pairs(nodes)do if c.id=='s2up_deck181_status'then status=c end end
    if mode=='desktop'then
        assert(creates==0 and reports==0 and stops==0 and not root.callbacks.OnUpdatePulse,'Desktop campaign must keep stock callbacks and no Deck children')
    elseif mode=='normal'then
        assert(creates==1 and not status,'Only the pulse owner exists before the new campaign texture is ready; main-menu legend is absent')
        assert(root.callbacks.OnUpdatePulse and root.callbacks.OnPreDraw)
        local group;for _,c in pairs(nodes)do if c.id=='s2up_deck181_group'then group=c end end;assert(group)
        flags.down=true;group.callbacks.OnUpdatePulse();flags.down=false;assert(reports==0 and last_focus==nil and activated==0,'Deferred directions must not move/activate menu choices');
        flags.cancel=true;group.callbacks.OnUpdatePulse();assert(went_back==1)
        flags.poll=false;group.callbacks.OnPreDraw();assert(went_back==1)
        group.callbacks.OnDestroyed();assert(stops==1 and unrequired==0);if not campaign then env.OnDestroyed();assert(unrequired==2)end
    else
        assert(stops==0 and activated==0,'UI failure stopped input')
    end
    passed=passed+1;print('PASS compiled181 '..(campaign and'campaign'or'menu')..' '..mode..'; actual export lookup and owned handoff; UI/backend are explicit mocks')
end
for _,campaign in ipairs({false,true})do run('desktop',campaign);run('normal',campaign);run('UI-failure',campaign);run('prepare-failure',campaign)end
print('PASS '..passed..' compiled181 menu/campaign bootstrap integration checks; no actual Deck execution')
