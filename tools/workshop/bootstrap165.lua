-- First-party Workshop startup only. No original game script/artwork is embedded.
-- File offsets and owned-pack decoding belong to the native uint64 reader.
local args={...}
local arg_count=select("#",...)
local stock_invoked167=false
local error_reporter167
local function run_stock167(original)
    local chunk,why=loadstring(original,"@owned-game/frontend-root");assert(chunk,why)
    setfenv(chunk,getfenv(1))
    -- Never run the original twice after partially registering callbacks.
    stock_invoked167=true
    chunk(unpack(args,1,arg_count))
end
local function fallback_stock167()
    -- Emergency menu recovery only; normal startup retains native uint64 I/O.
    -- Split offsets into base-65536 limbs so float Lua cannot round away bytes.
    -- Restrict seeks to signed-long range for the game's Windows Lua CRT.
    local f=assert(io.open("data/data.pack","rb"),"Cannot open owned data.pack for menu recovery")
    local ok,result=pcall(function()
        local header=assert(f:read(32));assert(#header==32 and header:sub(1,4)=="PFH3","Unsupported owned pack")
        local function pair(s,p)
            local a,b,c,d=s:byte(p,p+3);assert(d,"Truncated pack integer")
            return a+b*256,c+d*256
        end
        local function small(s,p)local lo,hi=pair(s,p);return lo+hi*65536 end
        local flags=small(header,5);local count=small(header,17);local size=small(header,21)
        assert((flags==0 or flags==1 or flags==3) and small(header,9)==0 and small(header,13)==0,"Unsupported owned pack variant")
        assert(count<=100000 and size<=8388608,"Owned pack index exceeds safe Lua recovery bound")
        local index=assert(f:read(size));assert(#index==size,"Truncated owned pack index")
        local base=32+size;local hi=math.floor(base/65536);local lo=base-hi*65536
        local cursor=1;local root_hi,root_lo,root_size
        for i=1,count do
            local nlo,nhi=pair(index,cursor);cursor=cursor+4
            local last=assert(index:find("\0",cursor,true),"Unterminated owned pack path")
            local name=index:sub(cursor,last-1):gsub("\\","/"):lower();cursor=last+1
            if name=="ui/frontend ui/layout_scripts/root.luac" then
                assert(not root_size and nhi<4,"Duplicate or excessive original UI script")
                root_hi,root_lo,root_size=hi,lo,nlo+nhi*65536
            end
            local sum=lo+nlo;local carry=math.floor(sum/65536);lo=sum-carry*65536;hi=hi+nhi+carry
            assert(hi<=262144,"Owned pack offsets exceed recovery bound")
        end
        assert(cursor==#index+1 and root_size and root_size>0,"Original UI entry unavailable")
        assert(root_hi<32768,"Original UI requires native 64-bit recovery")
        assert(f:seek("set",root_hi*65536),"Cannot seek owned UI block")
        assert(f:seek("cur",root_lo),"Cannot seek exact owned UI byte")
        local original=assert(f:read(root_size));assert(#original==root_size,"Truncated original UI script")
        assert(original:sub(1,12)==string.char(27,76,117,97,81,0,1,4,4,4,4,0),"Original UI bytecode differs")
        return original
    end)
    f:close();assert(ok,result);run_stock167(result)
end
local function initialize165()
    assert(type(io)=="table" and type(io.open)=="function" and type(os)=="table" and type(os.getenv)=="function","Local cache I/O unavailable")
    assert(type(package)=="table" and type(package.loadlib)=="function","Native Lua loading unavailable")
    local folder=assert(os.getenv("LOCALAPPDATA"),"User cache unavailable")
    local path=folder.."/S2UP-Workshop165-__NATIVE_SHA165__.dll"
    local payload=table.concat({
-- __NATIVE_PAYLOAD165__
    })
    local function matches(candidate)
        local f=io.open(candidate,"rb");if not f then return false end
        local actual=f:read(#payload+1);f:close();return actual==payload
    end
    local existing=io.open(path,"rb")
    if existing then existing:close();assert(matches(path),"Existing native cache differs; preserved")
    else
        local temporary,file
        for i=1,32 do
            local candidate=path..".new."..i
            -- Reuse a fully written stage left by an interrupted launch.
            if matches(candidate) then
                if not os.rename(candidate,path) then
                    assert(matches(path),"Native cache commit failed; existing target preserved")
                    os.remove(candidate)
                end
                assert(matches(path),"Native cache readback differs")
                temporary=nil;break
            end
            -- The pinned game CRT implements x (exclusive creation). Never use wb.
            local f=io.open(candidate,"wbx")
            if f then temporary,file=candidate,f;break end
        end
        if not matches(path) then
            assert(file,"Cannot create exclusive native staging file")
            local ok,written=pcall(function()return file:write(payload)end)
            local closed=file:close()
            if not (ok and written and closed and matches(temporary)) then
                -- This invocation exclusively created the staging file.
                os.remove(temporary)
                error("Native staging write failed")
            end
            local renamed=os.rename(temporary,path)
            if not renamed then
                if matches(temporary) then os.remove(temporary) end
                assert(matches(path),"Native cache commit failed; existing target preserved")
            end
        elseif file then
            file:close();os.remove(temporary) -- Another invocation committed first.
        end
        assert(matches(path),"Native cache readback differs")
    end
    payload=nil
    local prepare,why=package.loadlib(path,"s2up_workshop_prepare165");assert(type(prepare)=="function",why)
    local start,reason=package.loadlib(path,"s2up_workshop_start165");assert(type(start)=="function",reason)
    error_reporter167=package.loadlib(path,"s2up_workshop_error167")
    -- Each entry checks exactly one argument, then returns that existing value
    -- on success; no Lua stack writes or engine object calls are needed.
    assert(prepare(true),"Owned root preparation failed")
    -- Native guard owns this temporary file. D shares delete access; this mode
    -- is verified in the supported engine CRT and the matching float Lua host.
    local f=assert(io.open(path..".root.luac","rbD"),"Prepared UI root unavailable")
    local original=f:read(262145);f:close();assert(original and #original>0 and #original<=262144,"Prepared UI root exceeds bound")
    run_stock167(original)

    do
        local api={}
        api.prepare=assert(package.loadlib(path,"s2up_menu_prepare181"))
        api.supported=assert(package.loadlib(path,"s2up_menu_supported181"))
        api.poll=assert(package.loadlib(path,"s2up_menu_poll181"))
        api.stop=assert(package.loadlib(path,"s2up_menu_stop181"))
        api.active=assert(package.loadlib(path,"s2up_menu_active181"))
        api.focused=assert(package.loadlib(path,"s2up_menu_focused181"))
        api.ready=assert(package.loadlib(path,"s2up_menu_ready181"))
        api.bindings=assert(package.loadlib(path,"s2up_menu_bindings181"))
        api.failed=assert(package.loadlib(path,"s2up_menu_failed181"))
        api.glyphs=assert(package.loadlib(path,"s2up_menu_glyphs181"))
        api.glyph_failed=assert(package.loadlib(path,"s2up_menu_glyph_failed181"))
        api.cancel=assert(package.loadlib(path,"s2up_menu_cancel181"))
        api.heartbeat=assert(package.loadlib(path,"s2up_menu_heartbeat181"))
        api.error=assert(package.loadlib(path,"s2up_menu_error181"))
        api.up=assert(package.loadlib(path,"s2up_menu_up181"))
        api.down=assert(package.loadlib(path,"s2up_menu_down181"))
        api.left=assert(package.loadlib(path,"s2up_menu_left181"))
        api.right=assert(package.loadlib(path,"s2up_menu_right181"))
        api.focus=assert(package.loadlib(path,"s2up_menu_focus181"))
        api.attach=assert(package.loadlib(path,"s2up_menu_attach181"))
        api.detach=assert(package.loadlib(path,"s2up_menu_detach181"))
        api.campaign_prepare=assert(package.loadlib(path,"s2up_menu_campaign_prepare181"))
        api.pause=assert(package.loadlib(path,"s2up_menu_pause181"))
        api.next=assert(package.loadlib(path,"s2up_menu_next181"))
        api.previous=assert(package.loadlib(path,"s2up_menu_previous181"))
        api.home=assert(package.loadlib(path,"s2up_menu_home181"))
        api.arts=assert(package.loadlib(path,"s2up_menu_arts181"))
        api.finance=assert(package.loadlib(path,"s2up_menu_finance181"))
        api.diplomacy=assert(package.loadlib(path,"s2up_menu_diplomacy181"))
        api.clan=assert(package.loadlib(path,"s2up_menu_clan181"))
        api.end_turn=assert(package.loadlib(path,"s2up_menu_end_turn181"))
        api.quick_save=assert(package.loadlib(path,"s2up_menu_quick_save181"))
        api.move_speed=assert(package.loadlib(path,"s2up_menu_move_speed181"))
        api.encyclopedia=assert(package.loadlib(path,"s2up_menu_encyclopedia181"))
        api.pan_left=assert(package.loadlib(path,"s2up_menu_pan_left181"))
        api.pan_right=assert(package.loadlib(path,"s2up_menu_pan_right181"))
        api.pan_up=assert(package.loadlib(path,"s2up_menu_pan_up181"))
        api.pan_down=assert(package.loadlib(path,"s2up_menu_pan_down181"))
        api.turn_left=assert(package.loadlib(path,"s2up_menu_turn_left181"))
        api.turn_right=assert(package.loadlib(path,"s2up_menu_turn_right181"))
        api.viewport=assert(package.loadlib(path,"s2up_menu_viewport181"))
        api.context=assert(package.loadlib(path,"s2up_menu_context181"))
        api.camera=assert(package.loadlib(path,"s2up_menu_camera181"))
        api.legend184=assert(package.loadlib(path,"s2up_menu_legend184181"))
        api.ui_viewport185=assert(package.loadlib(path,"s2up_menu_ui_viewport185181"))
        api.panel188=assert(package.loadlib(path,"s2up_menu_panel188181"))
        api.back188=assert(package.loadlib(path,"s2up_menu_back188181"))
        api.menu_fit209=assert(package.loadlib(path,"s2up_menu_menu_fit209181"))
        api.decorate214=assert(package.loadlib(path,"s2up_menu_decorate214181"))
        api.click183=assert(package.loadlib(path,"s2up_menu_click183"))
        if api.supported(true)then
            local ok,why=pcall(function()
                assert(api.attach("menu:"..tostring(Address)),"Native controller context unavailable")
                assert(api.prepare(true),"Native controller startup failed")
                local chunk=assert(loadstring([==[
-- Persistent native menu labels and root frame events. Tooltip lifetime,
-- hover state and a child component's draw visibility do not own input polling.
local root,api,glyph_dir,navigation_factory,context=...
local campaign=context=="campaign"
local quiet_menu=type(api.camera)=="function"
local native_legend=false -- owner removed all live legends
local function packed(...)return {n=select("#",...),...}end
local function native(tag,call,...)
 -- Refuse unsafe setters after a parser failure; do not reset native validity.
 -- The fixed-size native record identifies the FIRST boundary that failed.
 if not api.context("before:"..tag)then error("Native UI context unavailable before "..tag,0)end
 local result=packed(pcall(call,...))
 local healthy=api.context("after:"..tag)
 if not result[1]then error(result[2],0)end
 if not healthy then error("Native UI context invalidated by "..tag,0)end
 return unpack(result,2,result.n)
end
local function ui(c,name,...)
 return native(name,function(...)return c[name](c,...)end,...)
end
local function visible_component(id)
    local address=ui(root,"Find",id)
    if address and ui(UIComponent(address),"Visible")then return address end
end
local function observe_tooltip(id)
    local address=visible_component(id)
    if address then api.decorate214("tooltip:"..tostring(address))end
end
local function finite(v)return type(v)=="number"and v==v and(v-v)==0 end
local function absolute(v)return v<0 and-v or v end
-- The game's math namespace is a native binding, not the ordinary complete
-- Lua math library. Use Lua arithmetic for this bounded rotation; no cached
-- engine function references or assumptions about optional math globals.
local function basis(bearing)
 if not finite(bearing)then return nil end
 local pi=3.141592653589793;local a=(bearing+pi)%(2*pi)-pi
 local function sin(x)
  local xx=x*x
  return x*(1-xx*(1/6-xx*(1/120-xx*(1/5040-xx*(1/362880-xx/39916800)))))
 end
 local b=(a+pi/2+pi)%(2*pi)-pi
 return sin(a),sin(b)
end
local root_identity={}
local token=(campaign and"campaign:"or"menu:")..tostring(root_identity)
assert(api.attach(token),"Controller root attachment failed")
-- Native renderer, maximum bounds and pointer conversion share one UI scale.
-- Reflow an already-created root once, before adding our invisible input owner.
if type(api.ui_viewport185)=="function"then
 local ok,w,h=pcall(api.ui_viewport185,true)
 if ok and finite(w)and finite(h)and w>=640 and h>=360 then ui(root,"Resize",w,h,false,false)end
end

local function dimensions(w,h)
 return finite(w)and finite(h)and w>=640 and h>=360 and w<=32767 and h<=32767
end
local function screen_size()
 -- The focused native game window owns viewport size. Scripted getters are
 -- an initialization fallback only; validate their complete return contract.
 if type(api.viewport)=="function"then
  local ok,w,h=pcall(api.viewport,true)
  if ok and dimensions(w,h)then return w,h end
 end
 local ok,w,h=pcall(function()
  if campaign then return ui(root,"Width"),ui(root,"Height")end
  return FrontEnd.ScreenSize()
 end)
 if ok and dimensions(w,h)then return w,h end
end
assert(root and api and glyph_dir,"Native menu arguments missing")
local prefix="s2up_deck181_"
local M={live=true,panels={},icons={},pending=false}
local hints=campaign and{{"l2","Select"},{"r2","Move / order"},{"stick","Move map"},{"pad","Cursor"}}or{{"a","Select"},{"b","Back"},{"stick","Navigate"},{"pad","Cursor"}}
local icons_ready=false;local last_text,last_mode;local layout_width,layout_height;local layout_dirty=true
-- Current native bridge uses touch cursor while directional menus are deferred.
-- Do not scan native component geometry or wait for a highlight acknowledgement.
local navigation=api.camera and{pointer_mode=false,refresh=function()end,move=function()end}or assert(navigation_factory)(root,api,prefix,screen_size)
if api.camera and not campaign then hints={{"a","Select"},{"b","Back"},{"pad","Cursor"},{"l2","Select / drag"}}end
local refresh_frames=0
local function component(id,parent)
    local owner=native("Address",function()return(parent or root):Address()end)
    local address=assert(native("CreateFromLayout",function()return Component.CreateFromLayout("data/ui/common ui/general_purpose_pixel",prefix..id,owner,0,0)end),"Native basic UI creation failed")
    local c=assert(native("UIComponent",function()return UIComponent(address)end))
    ui(c,"SetInteractive",false)
    ui(c,"SetVisible",true)
    return c
end
local function text(c,value)ui(c,"SetStateText",value)end
local function make_ui()
    if quiet_menu then ui(M.group,"SetVisible",false);return true end
    if native_legend then
        if not api.legend184(true)then return true end
        if not M.strip then
            M.strip=component("campaign_legend",M.group);ui(M.strip,"SetImageColour",0,255,255,255,255)
            local image=assert(UIImage(glyph_dir.."/campaign-legend184.tga"),"Campaign legend unavailable")
            image:SetComponentTexture(ui(M.strip,"Address"),0);ui(M.strip,"SetImageMetrics",0,0,0,1184,92)
        end
        return true
    end
    if campaign then return true end
    local initial_w,initial_h=screen_size();if not finite(initial_w)or not finite(initial_h)or initial_w<640 or initial_h<360 then return false end
    if not M.group then M.group=component("group");ui(M.group,"SetImageColour",0,0,0,0,0)end
    local w,h=initial_w,initial_h;ui(M.group,"Resize",w,h)
    for i,hint in ipairs(hints)do
        local c=component("label"..i,M.group);ui(c,"SetImageColour",0,35,31,24,220);text(c,hint[2]);M.panels[i]=c
        local icon=component("icon"..i,M.group);ui(icon,"Resize",32,32);ui(icon,"SetVisible",false);M.icons[i]=icon
    end
    M.status=component("status",M.group);ui(M.status,"SetImageColour",0,35,31,24,220)
    text(M.status,"Connecting controller")
    return true
end
local function layout()
    if quiet_menu then return true end -- invisible owner needs no layout or draw mutation
    if native_legend then
        if not M.strip then return true end
        local width,height=screen_size();if not dimensions(width,height)then return false end
        if width==layout_width and height==layout_height and not layout_dirty then return true end
        local scale=(width-24)/1184;if scale>1 then scale=1 end
        ui(M.group,"Resize",width,height);ui(M.group,"MoveTo",0,0)
        ui(M.strip,"Resize",1184*scale,92*scale);ui(M.strip,"MoveTo",(width-1184*scale)/2,6);ui(M.strip,"SetVisible",true)
        layout_width,layout_height=width,height;layout_dirty=false;return true
    end
    if campaign then return true end
    local width,height=screen_size()
    if not dimensions(width,height)then return false end
    if not layout_dirty and width==layout_width and height==layout_height then return true end
    -- Native resizing/docking can move children. Set the parent geometry first,
    -- then size and place each child; text/adoption can invalidate the layout.
    ui(M.group,"Resize",width,height);ui(M.group,"MoveTo",0,0)
    layout_width,layout_height=width,height
    local row=width-24;if row>728 then row=728 end;local left=(width-row)/2
    for i,c in ipairs(M.panels)do
        local x=left+(i-1)*row/4;ui(c,"Resize",row/4-40,34);ui(c,"MoveTo",x+36,47)
        ui(M.icons[i],"Resize",32,32);ui(M.icons[i],"MoveTo",x,48)
    end
    ui(M.status,"Resize",row,26);ui(M.status,"MoveTo",left,14)
    layout_dirty=false
    return true
end
local function labels()
    if quiet_menu or native_legend then layout();return end
    if campaign then return end
    local mode_label=navigation.pointer_mode and(campaign and"Move map"or"Move cursor")or hints[3][2]
    if M.panels[3]and mode_label~=last_mode then text(M.panels[3],mode_label);last_mode=mode_label;layout_dirty=true end
    local value=api.failed(true)and"Controller unavailable"or
        api.bindings(true)and"Waiting for controller"or
        api.ready(true)and(not api.focused(true)and"Controller paused"or(campaign and"Campaign controls"or"Menu controls"))or
        "Connecting controller"
    if value~=last_text then text(M.status,value);last_text=value;layout_dirty=true end
    if api.glyphs(true)and not icons_ready then
        for i,hint in ipairs(hints)do
            local image=assert(UIImage(glyph_dir.."/"..hint[1]..".tga"),"Steam glyph image unavailable")
            image:SetComponentTexture(ui(M.icons[i],"Address"),0);ui(M.icons[i],"SetImageMetrics",0,0,0,32,32);ui(M.icons[i],"SetVisible",true)
        end
        icons_ready=true;layout_dirty=true
    end
    -- Never change parents/child order from a child's update/draw callback.
    -- Static Component.Adopt resolves the CURRENT Lua component, not root.
    layout()
end
local function hide()
    if M.strip then ui(M.strip,"SetVisible",false)end
    for _,c in ipairs(M.panels)do ui(c,"SetVisible",false)end
    for _,c in ipairs(M.icons)do ui(c,"SetVisible",false)end
    if M.status then ui(M.status,"SetVisible",false)end
end
local clan_request189=false
local clan_wait189=0
local clan_back189=false
local function click_campaign_button(button)
    if ui(button,"Visible")~=true or ui(button,"IsInteractive")~=true then return false end
    local state=ui(button,"CurrentState")
    if type(state)~="string"or state:lower():find("inactive")or state:lower():find("disabled")then return false end
    local x,y=ui(button,"Position");local w,h=ui(button,"Width"),ui(button,"Height")
    if not(finite(x)and finite(y)and finite(w)and finite(h))or w<1 or h<1 or w>256 or h>256 then return false end
    local sw,sh=screen_size();local lw,lh=ui(root,"Width"),ui(root,"Height")
    if not(dimensions(sw,sh)and dimensions(lw,lh))then return false end
    x=(x+w/2)*sw/lw;y=(y+h/2)*sh/lh
    if x<0 or y<0 or x>=sw or y>=sh then return false end
    return type(api.click183)=="function"and api.click183(tostring(x-x%1)..","..tostring(y-y%1))
end
local function campaign_component(owner,id)
    local address=ui(owner,"Find",id)
    if type(address)~="userdata"and not(finite(address)and address>0)then return end
    local component=native("UIComponent",UIComponent,address)
    if component and ui(component,"Id")==id then return component end
end
local function campaign_start()
    -- ToggleCinematicBorders creates this stock root child and hides the map HUD.
    -- Escape can dismiss the advisor without the campaign's click-up cleanup.
    local ok,cinematic=pcall(function()
        local bars=campaign_component(root,"cinematic_bars")
        return bars~=nil and ui(bars,"Visible")~=false
    end)
    if not ok then return end
    if not cinematic then
        if type(api.back188)=="function"then api.back188(true)end
        return
    end
    -- If the advisor is absent or the click is busy, leave the intro alone.
    pcall(function()
        local advisor=campaign_component(root,"advice_interface")
        if not advisor or ui(advisor,"Visible")~=true then return end
        local close=campaign_component(advisor,"minimise_button")
        if close and ui(close,"Parent","advice_interface")==ui(advisor,"Address")then
            click_campaign_button(close)
        end
    end)
end
local function clan_button189()
    if not clan_request189 then return end
    clan_wait189=clan_wait189+1
    if clan_wait189>120 then clan_request189=false;return end
    local ok,open=pcall(function()return native("PanelManager.OpenPanels",function()return PanelManager.OpenPanels()end)end)
    if not ok or type(open)~="boolean"then return end
    if open then
        if not clan_back189 and type(api.back188)=="function"then api.back188(true);clan_back189=true end
        return
    end
    local good=pcall(function()
        local ptr=ui(root,"Find","button_clan")
        if not ptr then return end
        local c=native("UIComponent",function()return UIComponent(ptr)end)
        if ui(c,"Id")=="button_clan"and click_campaign_button(c)then clan_request189=false end
    end)
    -- An unavailable/transitioning button cannot poison other native controls.
    if not good then clan_request189=false end
end
local function campaign_buttons183()
    if not api.home then return end -- older inert fixture compatibility
    if api.home(true)then native("ZoomToCapital",function()return CampaignUI.ZoomToCapital()end)end
    if api.move_speed(true)then native("ToggleMoveSpeed",function()return CampaignUI.ToggleMoveSpeed()end)end
    if api.quick_save(true)then native("QuickSave",function()if CampaignUI.CanSave()then CampaignUI.QuickSave()end end)end
    if api.clan(true)and not clan_request189 then clan_request189=true;clan_wait189=0;clan_back189=false end
    clan_button189()
    -- Other panels/end turn retain native keyboard shortcuts. Clan uses the
    -- stock button's current geometry; no panel creation or script call here.

end
-- Original region_details arrows call their region_info ancestor's native
-- PreviousRegion/NextRegion callbacks. General map selection cannot refresh an
-- already-open region-details panel. Resolve the current owner, never cache it.
local function city_owner214()
    for _,id in ipairs({"region_info","region_details"})do
        local ptr=ui(root,"Find",id)
        if ptr then
            local details=native("UIComponent",function()return UIComponent(ptr)end)
            if ui(details,"Visible")==true then
                for _,arrow_id in ipairs({"arrow R","arrow L"})do
                    local arrow=ui(details,"Find",arrow_id)
                    if arrow then
                        local button=native("UIComponent",function()return UIComponent(arrow)end)
                        local parent=ui(button,"Parent","region_info")
                        if parent then
                            local owner=native("UIComponent",function()return UIComponent(parent)end)
                            if ui(owner,"Id")=="region_info"and ui(owner,"Visible")==true then return owner end
                        end
                    end
                end
            end
        end
    end
end
local function campaign_cycle214(forward)
    local open=native("PanelManager.OpenPanels",function()return PanelManager.OpenPanels()end)
    if type(open)~="boolean"then return end
    if open then
        local owner=city_owner214()
        if owner then
            local callback=forward and"NextRegion"or"PreviousRegion"
            native("region_info."..callback,function()return owner:LuaCall(callback)end)
        end
        return -- No map selection can leak into another native modal.
    end
    if forward then native("NextAutoEntitySelection",function()return CampaignUI.NextAutoEntitySelection()end)
    else native("PreviousAutoEntitySelection",function()return CampaignUI.PreviousAutoEntitySelection()end)end
end
local menu_extent209
local function campaign_setup_fit209()
    if campaign or not quiet_menu or type(api.menu_fit209)~="function"then return end
    local value="off";local screen
    for _,id in ipairs({"grand_campaign_japan","grand_campaign_boshin"})do
        local address=ui(root,"Find",id)
        if address then
            local c=native("UIComponent",function()return UIComponent(address)end)
            if ui(c,"Id")==id and ui(c,"Visible")==true then
                local w,h=ui(c,"Width"),ui(c,"Height")
                if dimensions(w,h)and w<=8192 and h<=8192 then
                    -- Both owned layouts use the same1280x1024 authored minimum.
                    -- Do not feed our already-expanded viewport back as a new minimum.
                    value="1280,1024";screen=c;break
                end
            end
        end
    end
    if value~=menu_extent209 then
        if not api.menu_fit209(value)then return end
        menu_extent209=value
    end
    if not screen or type(api.ui_viewport185)~="function"then return end
    local ok,w,h=pcall(api.ui_viewport185,true)
    if not ok or not dimensions(w,h)or w<1280 or h<1024 or w>8192 or h>8192 then return end
    -- Revision35 changed the outer viewport but left this native screen at
    -- 1310x819. Its clipping rectangle cut off the entire Start Game footer.
    -- Resize this resizable fullscreen container too, without scaling children
    -- or images. Original docking places its fixed-size main panel and footer.
    if ui(screen,"Width")~=w or ui(screen,"Height")~=h then ui(screen,"Resize",w,h,false,false)end
    local rx,ry=ui(root,"Position");local x,y=ui(screen,"Position")
    if finite(rx)and finite(ry)and finite(x)and finite(y)and(x~=rx or y~=ry)then ui(screen,"MoveTo",rx,ry)end
end
local function tick()
    if not M.live then return end
    api.heartbeat(true)
    local fresh=api.poll(true)
    if not api.active(true)then
        if api.prepare(true)then return end
        M.live=false;M.pending=false;pcall(hide);return
    end
    if not api.focused(true)then
        if not M.hidden then pcall(hide);M.hidden=true end
        return
    elseif M.hidden then
        M.hidden=false
        if M.strip then ui(M.strip,"SetVisible",true)end
        for _,c in ipairs(M.panels)do ui(c,"SetVisible",true)end
        for _,c in ipairs(M.icons)do ui(c,"SetVisible",icons_ready)end
        if M.status then ui(M.status,"SetVisible",true)end
    end
    -- Input is acquired/normalized by the native worker, not by a UI draw
    -- callback. Missing artwork or labels cannot turn off cursor/selection.
    if fresh and api.focused(true)then
        campaign_setup_fit209()
        if campaign and type(api.panel188)=="function"then
            -- Original native OpenPanels returns the manager's modal-open bool.
            -- Observe only: the stock input phase owns all panel opens/closes.
            local ok,open=pcall(function()return native("PanelManager.OpenPanels",function()return PanelManager.OpenPanels()end)end)
            if type(api.decorate214)=="function"then
                local corner=ui(root,"Find","hud_right")
                if corner then api.decorate214("corner:"..tostring(corner))end
                -- Identify current campaign hover templates for the native
                -- draw pass. Their geometry and font state stay native.
                for _,id in ipairs({"Tooltip","STW_tech_tooltip","TechTreeItem_Tooltip",
                    "campaign_map_tooltip","Diplomacy_region_tooltip","Minister_tooltip",
                    "recruitment_effects_tooltip","tax_breakdown_tooltip",
                    "trade_route_tooltip","unitcard_tooltip","clan_level_tooltip"})do
                    observe_tooltip(id)
                end
                -- These native popups are created as root siblings under
                -- their runtime IDs, independent of the common Tooltip.
                if ok and open==true then
                    if visible_component("technology")then observe_tooltip("TechTooltipPopup")end
                    if visible_component("clan")then observe_tooltip("EffectBundleInfoPopup")end
                end
            end
            local city=ok and open and city_owner214()
            api.panel188(ok and type(open)=="boolean"and(open and(city and"city"or"open")or"map")or"unknown")
        end
        refresh_frames=refresh_frames+1;if refresh_frames>=60 then refresh_frames=0;if navigation.refresh then navigation.refresh()end end
        if campaign and api.pause(true)then campaign_start();return end
        if api.cancel(true)then
            if campaign then
                if type(api.back188)=="function"then api.back188(true)end
            else TransitionBack()end
            return
        end
        if campaign then
            campaign_buttons183()
            if api.next(true)then campaign_cycle214(true)end
            if api.previous(true)then campaign_cycle214(false)end
            if api.camera then api.camera(true)else
            local dx,dy,turn=0,0,0
            for i=1,32 do
                local any=false
                if api.pan_left(true)then dx=dx-.125;any=true end
                if api.pan_right(true)then dx=dx+.125;any=true end
                if api.pan_up(true)then dy=dy+.125;any=true end
                if api.pan_down(true)then dy=dy-.125;any=true end
                if api.turn_left(true)then turn=turn-.01;any=true end
                if api.turn_right(true)then turn=turn+.01;any=true end
                if not any then break end
            end
            if dx~=0 or dy~=0 or turn~=0 then
                -- Native documentation/assembly: x,height/zoom,z,bearing[,theatre].
                -- SetCameraTargetInstant takes the horizontal map pair x,z.
                local x,zoom_height,z,bearing=native("CameraTarget",function()return CampaignUI.CameraTarget()end)
                if finite(x)and finite(z)and absolute(x)<100000 and absolute(z)<100000 then
                    local fx,fz=basis(bearing)
                    if fx and(dx~=0 or dy~=0)then native("SetCameraTargetInstant",function()CampaignUI.SetCameraTargetInstant(x+fz*dx+fx*dy,z-fx*dx+fz*dy)end)end
                    if finite(bearing)and turn~=0 then native("SetCameraHeading",function()CampaignUI.SetCameraHeading(bearing+turn)end)end
                end
            end
        end
        -- The legacy script arithmetic is retained only for older inert hosts;
        -- the current bridge always exports camera and uses native methods.
        end
        local direction=api.up(true)and"up"or api.down(true)and"down"or api.left(true)and"left"or api.right(true)and"right"
        if direction then navigation.move(direction)end
    end
    if not quiet_menu and not M.ui_failed then
        local ok,why=pcall(function()if not M.status and not M.strip and not make_ui()then return end;labels()end)
        if not ok then M.ui_failed=true;api.error(tostring(why))end
    end
end
local in_tick=false
local function protected_tick()
    if in_tick then return end;in_tick=true
    local ok,why=pcall(tick);in_tick=false
    if not ok then api.focus("");api.error(tostring(why))end
end
-- Root FrontEndRoot and Tooltip have native controller pulse handlers.
-- Register only AFTER creation of our plain pixel component has completed.
M.group=component("group");ui(M.group,"SetImageColour",0,0,0,0,0)
if quiet_menu or(campaign and not native_legend)then ui(M.group,"SetVisible",false)end
local function destroyed()if root_identity then M.live=false;api.detach(token)end end
ui(M.group,"SetGlobal","OnUpdatePulse",protected_tick)
ui(M.group,"SetGlobal","OnPreDraw",protected_tick)
ui(M.group,"SetGlobal","OnDestroyed",destroyed)
ui(M.group,"SetEventCallback","OnUpdatePulse","OnUpdatePulse")
ui(M.group,"SetEventCallback","OnPreDraw","OnPreDraw")
ui(M.group,"SetEventCallback","OnDestroyed","OnDestroyed")
local ui_ok,ui_why=pcall(function()if make_ui()then labels()end end)
if not ui_ok then M.ui_failed=true;api.error(tostring(ui_why))end
-- Keep prompts above newly adopted menu screens only from the root's native
-- adoption event, like its existing tooltip. Explicit receiver is essential:
-- moving the group from its own pulse can otherwise make it its own parent.
-- Original main layout contains weblinks (news/reviews) and advert. Hide only
-- those stock widgets on creation/adoption, never scan or mutate UI per frame.
local function hide_news187()
    if campaign or not quiet_menu then return end
    for _,id in ipairs({"weblinks","advert"})do
        local address=ui(root,"Find",id)
        if address then
            local c=native("UIComponent",function()return UIComponent(address)end)
            if c and ui(c,"Id")==id then
                ui(c,"SetVisible",false);ui(c,"SetInteractive",false)
            end
        end
    end
end
local news_ok,news_why=pcall(hide_news187)
if not news_ok then api.error(tostring(news_why))end
local stock_adopt=rawget(getfenv(1),"OnAdoptChild")
local adopting=false
local function adopted(...)
    if adopting then
        if type(stock_adopt)=="function"then return stock_adopt(...)end
        return
    end
    adopting=true
    local ok,why=true,nil
    if type(stock_adopt)=="function"then ok,why=pcall(stock_adopt,...)end
    if not ok then adopting=false;error(why,0)end
    local news_ok,news_why=pcall(hide_news187)
    if not news_ok then api.error(tostring(news_why))end
    if M.live and not M.ui_failed and not quiet_menu and(not campaign or native_legend)then
        local own_ok,own_why=pcall(function()
            local count=ui(root,"ChildCount")
            if type(count)=="number"and count==count and count>0 and count<100000 and ui(root,"Find",count-1)~=ui(M.group,"Address")then
                ui(root,"Adopt",ui(M.group,"Address"));layout_dirty=true
            end
        end)
        if not own_ok then M.ui_failed=true;api.error(tostring(own_why))end
    end
    adopting=false
end
ui(root,"SetGlobal","OnAdoptChild",adopted)
-- Chain native root frame events; plain pixel children need not be pulsed.
-- Stock callbacks run first, and no parenting changes occur during a frame.
local environment=getfenv(1)
for _,name in ipairs({"OnUpdatePulse","OnPreDraw"})do
    local stock=rawget(environment,name)
    ui(root,"SetGlobal",name,function(...)
        local results=type(stock)=="function"and packed(stock(...))or nil
        protected_tick()
        if results then return unpack(results,1,results.n)end
    end)
    ui(root,"SetEventCallback",name,name)
end
M.tick=protected_tick
return M

]==],"@first-party/deck-frontend181"));setfenv(chunk,getfenv(1))
                local navigation=assert(loadstring([==[
-- Read native UI geometry defensively. A native getter can return an unusable
-- value while its component is unavailable; never call a returned function.
return function(root,api,prefix,screen_size)
 screen_size=screen_size or FrontEnd.ScreenSize
 local N={selected=nil,pointer_mode=false};local limit=768
 -- The reported game environment lacks math.huge. IEEE infinities fail (v-v)==0;
 -- this uses Lua arithmetic only, with no optional library constants.
 local function number(v)return type(v)=="number"and v==v and(v-v)==0 end
 local function absolute(v)return v<0 and-v or v end
 local function floor(v)return v-v%1 end
 -- Resolve and invoke each engine-bound member together, exactly as stock Lua
 -- does. Never retain an engine method across a second native binding lookup.
 local readers={
  Visible=function(c)return c:Visible()end,Id=function(c)return c:Id()end,
  CurrentState=function(c)return c:CurrentState()end,ChildCount=function(c)return c:ChildCount()end,
  Find=function(c,i)return c:Find(i)end,IsInteractive=function(c)return c:IsInteractive()end,
  Position=function(c)return c:Position()end,Width=function(c)return c:Width()end,
  Height=function(c)return c:Height()end,Address=function(c)return c:Address()end}
 local function read(c,name,...)
  if not api.context("nav-before:"..name)then return end
  local good,a,b=pcall(readers[name],c,...)
  if not api.context("nav-after:"..name)then return end
  if good then
   local expected=(name=="Visible"or name=="IsInteractive")and"boolean"or(name=="Id"or name=="CurrentState")and"string"or(name=="Find"or name=="Address")and nil or"number"
   if expected and type(a)~=expected then api.context("bad:"..name..":"..type(a))end
   return a,b
  end
 end
 local function address(v)local t=type(v);return(t=="userdata")or(t=="number"and number(v)and v>0)end
 local function gather()
  local choices={};local visits=0;local unusable=false
  local good,sw,sh=pcall(screen_size)
  if not good or not number(sw)or not number(sh)or sw<1 or sh<1 then return choices,true end
  local function walk(c,depth)
   visits=visits+1;if not c or visits>limit or depth>18 then return false end
   local visible=read(c,"Visible");if visible==false then return false end
   if type(visible)~="boolean"then unusable=true;return false end
   local id=read(c,"Id");if type(id)~="string"then unusable=true;return false end
   if id:lower():sub(1,#prefix)==prefix then return false end
   local state=read(c,"CurrentState")
   if type(state)~="string"then unusable=true;return false end
   state=state:lower();if state:find("disabled")or state:find("inactive")then return false end
   local count=read(c,"ChildCount")
   if not number(count)or count<0 then unusable=true;return false end
   count=count>256 and 256 or floor(count)
   local child_choice=false
   for i=0,count-1 do
    local ptr=read(c,"Find",i)
    if address(ptr)then local ok,child=pcall(UIComponent,ptr);if ok and walk(child,depth+1)then child_choice=true end
    elseif ptr~=nil then unusable=true end
   end
   local own_choice=false;local interactive=read(c,"IsInteractive")
   if type(interactive)~="boolean"then unusable=true;return child_choice end
   if interactive and not child_choice then
    local x,y=read(c,"Position");local w,h=read(c,"Width"),read(c,"Height");local ptr=read(c,"Address")
    if number(x)and number(y)and number(w)and number(h)and address(ptr)then
     if w>=16 and h>=12 and w*h<sw*sh*.25 and x>=0 and y>=0 and x+w<=sw and y+h<=sh then
      choices[#choices+1]={address=ptr,x=x+w/2,y=y+h/2};own_choice=true
     end
    else unusable=true end
   end
   return own_choice or child_choice
  end
  walk(root,0);return choices,unusable
 end
 local function pointer_mode(value)
  if N.pointer_mode~=value then
   -- Bad geometry must never turn the directional controls into a mouse.
   api.focus("directional");N.pointer_mode=false;N.selected=nil
  end
 end
 function N.refresh()
  if N.pointer_mode then local choices=gather();if #choices>0 then pointer_mode(false)end end
 end
 function N.move(direction)
  local choices,unusable=gather()
  if #choices==0 then api.focus("");return false end
  if N.pointer_mode then pointer_mode(false)end
  local current;local ok,hover=pcall(Component.MouseOver)
  if ok then for _,c in ipairs(choices)do if c.address==hover then current=c;break end end end
  if not current then for _,c in ipairs(choices)do if c.address==N.selected then current=c;break end end end
  local best,score
  for _,c in ipairs(choices)do
   if not current then
    local value=c.y*10000+c.x;if not score or value<score then best,score=c,value end
   elseif c~=current then
    local dx,dy=c.x-current.x,c.y-current.y
    local along=(direction=="up"and-dy)or(direction=="down"and dy)or(direction=="left"and-dx)or dx
    local across=(direction=="up"or direction=="down")and absolute(dx)or absolute(dy)
    if along>2 then local value=along+across*3+across*across/along;if not score or value<score then best,score=c,value end end
   end
  end
  if not best then api.focus("");return false end
  if api.focus(tostring(floor(best.x))..","..tostring(floor(best.y)))then N.selected=best.address;return true end
  return false
 end
 return N
end

]==],"@first-party/deck-navigation181"));setfenv(navigation,getfenv(1))
                return chunk(UIComponent(Address),api,path..".deck177",navigation(),"menu")
            end)
            if not ok then api.error(tostring(why))end
        end
    end
    assert(start(true),"Native worker startup failed")
end
local ok,why=pcall(initialize165)
if not ok then
    if not stock_invoked167 then
        local recovered,recovery=pcall(fallback_stock167)
        if not recovered then pcall(function()print("Unit Possession menu recovery: "..string.sub(tostring(recovery),1,512))end) end
    end
    pcall(function()print("Shogun 2 Unit Possession: "..string.sub(tostring(why),1,512))end)
    -- A single bounded failure note also works when the DLL was blocked.
    -- Exclusive creation preserves any existing file; no per-session history.
    pcall(function()
        local folder=os.getenv("LOCALAPPDATA");if not folder then return end
        local file=io.open(folder.."/S2UP-Workshop-startup-error.txt","wbx")
        if file then
            local note="Shogun 2 Unit Possession bootstrap error\nNative __NATIVE_SHA165__\n"..tostring(why)
            file:write(note:sub(1,4096));file:close()
        end
    end)
    if type(error_reporter167)=="function" then pcall(error_reporter167) end
end
