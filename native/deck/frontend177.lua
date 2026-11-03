-- First-party Deck menu behavior. All components, focus and activation belong
-- to the owned game's UI. No keyboard events, replacement menu or Steam overlay.
local root,api,glyph_dir=...
assert(root and api and glyph_dir,"Deck frontend arguments unavailable")
local M={panels={},icons={},live=true,revision="177",pending_confirm=false}
local prefix="s2up_deck177_"
local function wrap(address)
    if not address then return nil end
    local ok,component=pcall(UIComponent,address)
    if ok then return component end
end
local function activate_hovered()
    -- MouseOver is the engine's current hit-test result, including native
    -- modal/input priority. Do not search all buttons and bypass a dialog.
    local component=wrap(Component.MouseOver())
    if not component or not component:Visible() or not component:IsInteractive() then return end
    local id=component:Id()
    if type(id)~="string" or id:sub(1,#prefix)==prefix then return end
    local state=component:CurrentState()
    if state=="inactive" or state=="disabled" then return end
    -- Stock diplomacy uses this native activation event. No second access:
    -- the callback may destroy the button or replace the entire menu.
    component:ForceEvent("OnMouseLClickUp")
end
local function back()
    -- Existing native frontend page/modal rules, not a replacement stack.
    if type(TransitionBack)=="function" then TransitionBack() end
end
local hints={{"a","Select"},{"b","Back"},{"stick","Move cursor"},{"pad","Cursor"}}
local function make_prompt(index,hint)
    local address=Component.CreateFromLayout("data/ui/common ui/Tooltip",prefix.."hint"..index,root:Address(),0,0)
    local panel=assert(wrap(address),"Native Deck prompt panel unavailable")
    panel:SetInteractive(false)
    panel:LuaCall("SetText",panel:Address(),hint[2])
    M.panels[index]=panel
    local icon=assert(wrap(Component.CreateFromLayout("data/ui/common ui/general_purpose_pixel",prefix.."icon"..index,root:Address(),0,0)),"Native Deck icon component unavailable")
    icon:SetInteractive(false);icon:Resize(32,32);icon:SetVisible(false)
    M.icons[index]=icon
end
for i,hint in ipairs(hints) do make_prompt(i,hint) end
local status=assert(wrap(Component.CreateFromLayout("data/ui/common ui/Tooltip",prefix.."status",root:Address(),0,0)),"Native Deck status unavailable")
status:SetInteractive(false);M.status=status
local icon_ready=false;local last_status
local function arrange()
    local w,h=FrontEnd.ScreenSize();local width=math.min(680,w-32);local start=(w-width)/2
    for i,panel in ipairs(M.panels) do
        local x=start+(i-1)*width/4
        panel:MoveTo(x+38,h-55);panel:Resize(width/4-42,39)
        M.icons[i]:MoveTo(x,h-52)
    end
    status:MoveTo(start,h-89);status:Resize(width,27)
end
local function update_prompts()
    if api.glyphs(true) and not icon_ready then
        for i,hint in ipairs(hints) do
            local image=UIImage(glyph_dir.."/"..hint[1]..".tga")
            assert(image,"Steam Deck glyph unavailable")
            image:SetComponentTexture(M.icons[i]:Address(),0)
            M.icons[i]:SetImageMetrics(0,0,0,32,32)
            M.icons[i]:SetVisible(true)
        end
        icon_ready=true
    end
    local text=api.failed(true) and "Controller support could not initialize" or
        api.glyph_failed(true) and "Steam Deck controls - glyphs unavailable" or
        api.ready(true) and "Steam Deck menu controls" or
        api.bindings(true) and "Waiting for Steam native bindings..." or "Connecting controller..."
    if text~=last_status then
        status:LuaCall("SetText",status:Address(),text)
        last_status=text
    end
    arrange()
end
local function tick()
    if not M.live then return end
    local fresh=api.poll(true)
    if not api.active(true) then
        M.live=false;M.pending_confirm=false
        for _,panel in ipairs(M.panels)do panel:SetVisible(false)end
        for _,icon in ipairs(M.icons)do icon:SetVisible(false)end
        status:SetVisible(false);return
    end
    update_prompts()
    if not api.focused(true) then M.pending_confirm=false;return end
    if not fresh then return end
    local confirm,cancel=api.confirm(true),api.cancel(true)
    if M.pending_confirm then
        M.pending_confirm=false;activate_hovered();return
    end
    if confirm then
        -- SetCursorPos queues the real game hover update. Allow that message
        -- through before confirming a simultaneous pad-motion/button edge.
        if api.pointer(true) then M.pending_confirm=true else activate_hovered()end
    elseif cancel then back()end
end
local function protected_tick(...)
    local ok,why=pcall(tick)
    if not ok then
        M.live=false;api.stop(true)
        pcall(function()status:LuaCall("SetText",status:Address(),"Controller menu stopped: "..tostring(why):sub(1,160))end)
    end
end
local function destroyed()
    M.live=false;api.stop(true)
end
-- Both are actual stock component events. Polling coalesces to at most125Hz
-- and emits one set of edges, preventing double activation if both fire.
-- Bind only the component we created; leave every stock/other-root callback
-- unchanged, and close input when this owned menu child is destroyed.
status:SetEventCallback("OnUpdatePulse",protected_tick)
status:SetEventCallback("OnPreDraw",protected_tick)
status:SetEventCallback("OnDestroyed",destroyed)
update_prompts()
M.tick=protected_tick
return M
