-- Exercise the production numeric-return export with the matching float Lua
-- host and the verified pure push helper from an inert owned engine mapping.
local dll=assert(arg[1]);local function bind(n)return assert(package.loadlib(dll,n))end
local viewport=bind('s2up_menu_viewport181')
local valid=bind('s2up_viewport_valid_fixture181');local stale=bind('s2up_viewport_stale_fixture181');local invalid=bind('s2up_viewport_invalid_fixture181')
assert(valid(true));local w,h=viewport(true);assert(type(w)=='number'and type(h)=='number'and w==1280 and h==800)
assert(select('#',viewport(true))==2)
assert(select('#',viewport())==0 and select('#',viewport(true,true))==0)
assert(stale(true)and select('#',viewport(true))==0)
assert(invalid(true)and select('#',viewport(true))==0)
assert(valid(true));local a,b=viewport('unchanged incoming string');assert(type(a)=='number'and a==1280 and b==800)
print('PASS 6 actual float-Lua viewport export cases; pure owned helper only, no engine entry or worker')
