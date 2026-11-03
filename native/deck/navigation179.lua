-- Native UI geometry drives directional choice; activation and highlighting
-- remain the game's own mouse hit-test. No synthetic ForceEvent on a target.
return function(root,api,prefix)
    local N={selected=nil};local limit=768
    local function gather()
        local choices={};local visits=0
        local function walk(c,depth)
            visits=visits+1;if visits>limit or depth>18 then return end
            if not c:Visible()then return end
            local id=tostring(c:Id()):lower()
            if id:sub(1,#prefix)==prefix then return end
            local state=tostring(c:CurrentState()):lower()
            if state:find("disabled",1,true)or state:find("inactive",1,true)then return end
            local child_choice=false
            local count=math.min(c:ChildCount(),256)
            for i=0,count-1 do local address=c:Find(i);if address and walk(UIComponent(address),depth+1)then child_choice=true end end
            local own_choice=false
            if c:IsInteractive()and not child_choice then
                local x,y=c:Position();local w,h=c:Width(),c:Height();local sw,sh=FrontEnd.ScreenSize()
                if w>=16 and h>=12 and w*h<sw*sh*.25 and x>=0 and y>=0 and x+w<=sw and y+h<=sh then
                    choices[#choices+1]={address=c:Address(),x=x+w/2,y=y+h/2};own_choice=true
                end
            end
            return own_choice or child_choice
        end
        walk(root,0);return choices
    end
    function N.move(direction)
        local choices=gather();local current
        local hover=Component.MouseOver();for _,c in ipairs(choices)do if c.address==hover then current=c;break end end
        if not current then for _,c in ipairs(choices)do if c.address==N.selected then current=c;break end end end
        local best,score
        for _,c in ipairs(choices)do
            if not current then
                local value=c.y*10000+c.x;if not score or value<score then best,score=c,value end
            elseif c~=current then
                local dx,dy=c.x-current.x,c.y-current.y
                local along=(direction=="up"and-dy)or(direction=="down"and dy)or(direction=="left"and-dx)or dx
                local across=(direction=="up"or direction=="down")and math.abs(dx)or math.abs(dy)
                if along>2 then
                    local value=along+across*3+across*across/math.max(along,1)
                    if not score or value<score then best,score=c,value end
                end
            end
        end
        if not best then api.focus("");return false end
        if api.focus(tostring(math.floor(best.x))..","..tostring(math.floor(best.y)))then N.selected=best.address;return true end
        return false
    end
    return N
end
