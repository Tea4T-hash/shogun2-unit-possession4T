local path=assert(arg[1]);local function bind(name)return assert(package.loadlib(path,name..'181'))end
local seed,health,invalid,restore,count=bind('seed'),bind('health'),bind('invalid'),bind('restore'),bind('child_count')
local key,value=seed(true);assert(type(key)=='userdata'and type(value)=='userdata');lookup={[key]=value}
assert(health('valid'));assert(count(function()end)==3)
assert(invalid(true));local f=function()error('returned function must never be invoked')end
assert(count(f)==f and type(count(f))=='function'and not health('invalid'))
assert(restore(true));assert(health('restored')and count(f)==3)
for i=1,100 do assert(health('balanced')and count(f)==3)end
print('PASS 5 native context181 lookup/numeric-result/invalid-context-function/read-only-health/balanced-stack cases; inert image and owned state facade only')

local reject=assert(package.loadlib(path,'reject_plain_find183'))
assert(reject(true))
print('PASS native string183: actual argument-reader rejects the old plain-search boolean and clears context validity; diagnostic report provider only; inert owned image')
