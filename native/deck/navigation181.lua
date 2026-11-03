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
