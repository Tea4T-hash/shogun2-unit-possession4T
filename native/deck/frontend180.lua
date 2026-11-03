-- Persistent native menu labels and root frame events. Tooltip lifetime,
-- hover state and a child component's draw visibility do not own input polling.
local root,api,glyph_dir,navigation_factory=...
assert(root and api and glyph_dir,"Native menu arguments missing")
local prefix="s2up_deck180_"
local M={live=true,panels={},icons={},pending=false}
local hints={{"a","A / LT  Select"},{"b","B  Back"},{"stick","Navigate"},{"pad","Cursor"}}
local icons_ready=false;local last_text;local layout_width,layout_height
local navigation=assert(navigation_factory)(root,api,prefix)
local function component(id,parent)
    local address=assert(Component.CreateFromLayout("data/ui/common ui/general_purpose_pixel",prefix..id,(parent or root):Address(),0,0),"Native basic UI creation failed")
    local c=assert(UIComponent(address));c:SetInteractive(false);c:SetVisible(true)
    return c
end
local function text(c,value)c:SetStateText(value)end
local function make_ui()
    if not M.group then M.group=component("group");M.group:SetImageColour(0,0,0,0,0)end
    local w,h=FrontEnd.ScreenSize();M.group:Resize(w,h)
    for i,hint in ipairs(hints)do
        local c=component("label"..i,M.group);c:SetImageColour(0,35,31,24,220);text(c,hint[2]);M.panels[i]=c
        local icon=component("icon"..i,M.group);icon:Resize(32,32);icon:SetVisible(false);M.icons[i]=icon
    end
    M.status=component("status",M.group);M.status:SetImageColour(0,35,31,24,220)
    text(M.status,"Steam Deck - Test180 - connecting controller")
end
local function layout()
    local width,height=FrontEnd.ScreenSize()
    if width==layout_width and height==layout_height then return end
    layout_width,layout_height=width,height
    local row=math.min(728,width-24);local left=(width-row)/2
    for i,c in ipairs(M.panels)do
        local x=left+(i-1)*row/4;c:MoveTo(x+36,height-53);c:Resize(row/4-40,34)
        M.icons[i]:MoveTo(x,height-52)
    end
    M.status:MoveTo(left,height-83);M.status:Resize(row,26)
end
local function labels()
    local value=api.failed(true)and"Controller initialization failed - Test180"or
        api.bindings(true)and"Waiting for Steam native bindings - Test180"or
        api.ready(true)and(not api.focused(true)and"Controller paused: game is not focused - Test180"or"Steam Deck menu controls - Test180")or
        "Connecting controller - Test180"
    if value~=last_text then text(M.status,value);last_text=value end
    if api.glyphs(true)and not icons_ready then
        for i,hint in ipairs(hints)do
            local image=assert(UIImage(glyph_dir.."/"..hint[1]..".tga"),"Steam glyph image unavailable")
            image:SetComponentTexture(M.icons[i]:Address(),0);M.icons[i]:SetImageMetrics(0,0,0,32,32);M.icons[i]:SetVisible(true)
        end
        icons_ready=true
    end
    -- Never change parents/child order from a child's update/draw callback.
    -- Static Component.Adopt resolves the CURRENT Lua component, not root.
    layout()
end
local function hide()
    for _,c in ipairs(M.panels)do c:SetVisible(false)end
    for _,c in ipairs(M.icons)do c:SetVisible(false)end
    if M.status then M.status:SetVisible(false)end
end
local function tick()
    if not M.live then return end
    api.heartbeat(true)
    local fresh=api.poll(true)
    if not api.active(true)then M.live=false;M.pending=false;pcall(hide);return end
    -- Input is acquired/normalized by the native worker, not by a UI draw
    -- callback. Missing artwork or labels cannot turn off cursor/selection.
    if fresh and api.focused(true)then
        if api.cancel(true)then TransitionBack();return end
        local direction=api.up(true)and"up"or api.down(true)and"down"or api.left(true)and"left"or api.right(true)and"right"
        if direction then navigation.move(direction)end
    end
    if not M.ui_failed then
        local ok,why=pcall(function()if not M.status then make_ui()end;labels()end)
        if not ok then M.ui_failed=true;api.error(tostring(why))end
    end
end
local function protected_tick()
    local ok,why=pcall(tick)
    if not ok then api.focus("");api.error(tostring(why))end
end
-- Root FrontEndRoot and Tooltip have native controller pulse handlers.
-- Register only AFTER creation of our plain pixel component has completed.
M.group=component("group");M.group:SetImageColour(0,0,0,0,0)
local function destroyed()M.live=false;api.stop(true)end
M.group:SetGlobal("OnUpdatePulse",protected_tick)
M.group:SetGlobal("OnPreDraw",protected_tick)
M.group:SetGlobal("OnDestroyed",destroyed)
M.group:SetEventCallback("OnUpdatePulse","OnUpdatePulse")
M.group:SetEventCallback("OnPreDraw","OnPreDraw")
M.group:SetEventCallback("OnDestroyed","OnDestroyed")
local ui_ok,ui_why=pcall(function()make_ui();labels()end)
if not ui_ok then M.ui_failed=true;api.error(tostring(ui_why))end
-- Keep prompts above newly adopted menu screens only from the root's native
-- adoption event, like its existing tooltip. Explicit receiver is essential:
-- moving the group from its own pulse can otherwise make it its own parent.
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
    if M.live and not M.ui_failed then
        local own_ok,own_why=pcall(function()
            local count=root:ChildCount()
            if count>0 and root:Find(count-1)~=M.group:Address()then
                root:Adopt(M.group:Address())
            end
        end)
        if not own_ok then M.ui_failed=true;api.error(tostring(own_why))end
    end
    adopting=false
end
root:SetGlobal("OnAdoptChild",adopted)
M.tick=protected_tick
return M
