local dll=assert(arg[1]);local cursor=assert(package.loadlib(dll,"s2up_ui_cursor_fixture185"));local empty=assert(package.loadlib(dll,"s2up_ui_empty_fixture185"));local original=assert(package.loadlib(dll,"s2up_ui_unconverted_fixture185"))
for i=1,1000 do
 local x,y,w,h=cursor(true);assert(x==100 and y==200 and w==1310 and h==819)
 assert(select("#",empty(true))==0)
 local x,y,w,h=original(true);assert(x==100 and y==200 and w==1280 and h==1024)
end
print("PASS native UI185 float-Lua:3000 real pinned gettop/settop/pushnumber calls preserve x/y, result arity and missing-conversion behavior")
