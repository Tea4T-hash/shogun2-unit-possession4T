-- Persistent native menu labels and root frame events. Tooltip lifetime,
-- hover state and a child component's draw visibility do not own input polling.
local root,api,glyph_dir=...
assert(root and api and glyph_dir,"Native menu arguments missing")
local prefix="s2up_deck178_"
local M={live=true,panels={},icons={},pending=false}
local hints={{"a","A / LT  Select"},{"b","B  Back"},{"stick","Move cursor"},{"pad","Cursor"}}
local icons_ready=false;local last_text
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
    text(M.status,"Steam Deck - Test178 - connecting controller")
end
local function layout()
    local width,height=FrontEnd.ScreenSize();local row=math.min(728,width-24);local left=(width-row)/2
    for i,c in ipairs(M.panels)do
        local x=left+(i-1)*row/4;c:MoveTo(x+36,height-53);c:Resize(row/4-40,34)
        M.icons[i]:MoveTo(x,height-52)
    end
    M.status:MoveTo(left,height-83);M.status:Resize(row,26)
end
local function labels()
    local value=api.failed(true)and"Controller initialization failed - Test178"or
        api.bindings(true)and"Waiting for Steam native bindings - Test178"or
        api.ready(true)and(not api.focused(true)and"Controller paused: game is not focused - Test178"or"Steam Deck menu controls - Test178")or
        "Connecting controller - Test178"
    if value~=last_text then text(M.status,value);last_text=value end
    if api.glyphs(true)and not icons_ready then
        for i,hint in ipairs(hints)do
            local image=assert(UIImage(glyph_dir.."/"..hint[1]..".tga"),"Steam glyph image unavailable")
            image:SetComponentTexture(M.icons[i]:Address(),0);M.icons[i]:SetImageMetrics(0,0,0,32,32);M.icons[i]:SetVisible(true)
        end
        icons_ready=true
    end
    -- Stock root re-adopts its tooltip above newly adopted children. Use that
    -- same ordinary child-order path, without changing global input priority.
    local count=root:ChildCount()
    local tail=count>0 and root:Find(count-1)or nil
    local signature=tostring(count)..":"..tostring(tail)
    if tail~=M.group:Address()and signature~=M.child_order then
        Component.Adopt(M.group:Address())
        count=root:ChildCount();tail=count>0 and root:Find(count-1)or nil
        M.child_order=tostring(count)..":"..tostring(tail)
    end
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
    if fresh and api.focused(true)and api.cancel(true)then TransitionBack();return end
    if not M.ui_failed then
        local ok,why=pcall(function()if not M.status then make_ui()end;labels()end)
        if not ok then M.ui_failed=true;api.error(tostring(why))end
    end
end
local function protected_tick()
    local ok,why=pcall(tick)
    if not ok then api.error(tostring(why))end
end
local environment=getfenv(1)
assert(not root:GlobalExists("OnUpdatePulse")and not root:GlobalExists("OnPreDraw"),"Another menu owns the root frame callbacks; preserved")
assert(not root:GlobalExists("S2UPDeckFrame178"),"Existing controller frame callback preserved")
local stock_destroy=rawget(environment,"OnDestroyed")
local function destroyed(...)
    M.live=false;api.stop(true)
    if type(stock_destroy)=="function"then return stock_destroy(...)end
end
-- Match the actual stock radar callback pattern: SetGlobal followed by a
-- string callback name. Stock root functions are retained in their environment.
root:SetGlobal("S2UPDeckFrame178",protected_tick)
root:SetGlobal("S2UPDeckDestroyed178",destroyed)
root:SetEventCallback("OnUpdatePulse","S2UPDeckFrame178")
root:SetEventCallback("OnPreDraw","S2UPDeckFrame178")
root:SetEventCallback("OnDestroyed","S2UPDeckDestroyed178")
local ui_ok,ui_why=pcall(function()make_ui();labels()end)
if not ui_ok then M.ui_failed=true;api.error(tostring(ui_why))end
M.tick=protected_tick
return M
