-- Native UI is an explicit mock; no actual Shogun renderer/Deck execution.
local source=assert(arg[1]);local nav_source=assert(arg[2]);local passed=0
local function test(name,fn)fn();passed=passed+1;print('PASS '..name)end
local function fixture(opt)
 opt=opt or {};local nodes={};local next_id=0;local backs,stops,errors,heartbeats=0,0,0,0
 local flags={active=true,focused=true,ready=true,fresh=true};local methods={}
 local function node(id,parent)
  next_id=next_id+1;local c={id=id,address=next_id,children={},globals={},callbacks={},visible=false,state='active',x=0,y=0,w=120,h=30,interactive=false}
  nodes[next_id]=setmetatable(c,{__index=methods});if parent then table.insert(nodes[parent].children,c.address)end;return c
 end
 function methods:Id()return self.id end
 function methods:Visible()return self.visible end
 function methods:CurrentState()return self.state end
 function methods:IsInteractive()return self.interactive end
 function methods:Position()return self.x,self.y end
 function methods:Width()return self.w end
 function methods:Height()return self.h end
 function methods:Address()return self.address end
 function methods:SetInteractive(v)self.interactive=v end
 function methods:SetVisible(v)self.visible=v end
 function methods:SetImageColour()end
 function methods:SetStateText(v)self.text=v end
 function methods:Resize(w,h)assert(w>0 and h>0);self.w=w;self.h=h end
 function methods:MoveTo(x,y)self.x=x;self.y=y end
 function methods:SetImageMetrics()self.metrics=true end
 function methods:ChildCount()return #self.children end
 function methods:Find(i)return self.children[i+1]end
 function methods:GlobalExists(name)return self.globals[name]~=nil end
 function methods:SetGlobal(name,fn)assert(type(fn)=='function');self.globals[name]=fn end
 function methods:SetEventCallback(event,name)assert(type(name)=='string'and self.globals[name]);self.callbacks[event]=name end
 local root=node('root');root.visible=true;root.native=true;if opt.conflict then root.globals.OnPreDraw=function()end end
 local stock=function()end;root.globals.stock=stock;local destroyed=0
 local api={}
 for _,key in ipairs({'active','focused','ready','bindings','failed','glyphs','glyph_failed','cancel','up','down','left','right'})do local k=key;api[k]=function(v)assert(v==true);return flags[k]end end
 local target;api.focus=function(value)target=value;return true end
 api.poll=function(v)assert(v);return flags.fresh end
 api.stop=function(v)assert(v);stops=stops+1 end
 api.error=function(why)assert(type(why)=='string');errors=errors+1 end
 api.heartbeat=function()heartbeats=heartbeats+1 end
 local env={UIComponent=function(a)return assert(nodes[a])end,
  Component={MouseOver=function()return nil end,CreateFromLayout=function(layout,id,parent,x,y)assert(layout=='data/ui/common ui/general_purpose_pixel');if opt.fail_create then error('fixture native layout error')end;return node(id,parent).address end,
   Adopt=function(address)local list=root.children;for i,v in ipairs(list)do if v==address then table.remove(list,i);break end end;table.insert(list,address)end},
  FrontEnd={ScreenSize=function()return 1280,800 end},
  UIImage=function(path)return {SetComponentTexture=function(self,a,i)assert(i==0 and path:find('%.tga$'));nodes[a].texture=path end}end,
  TransitionBack=function()backs=backs+1 end,OnDestroyed=function()destroyed=destroyed+1 end}
 setmetatable(env,{__index=_G});local chunk=assert(loadfile(source));setfenv(chunk,env)
 local nav=assert(loadfile(nav_source));setfenv(nav,env)
 local m=chunk(root,api,'owned-cache.deck177',nav())
 local function pulse(event)local owner=m.group;local fn=owner.globals[owner.callbacks[event or 'OnUpdatePulse']];assert(fn);fn()end
 return {m=m,flags=flags,nodes=nodes,root=root,stock=stock,pulse=pulse,counts=function()return backs,stops,errors,heartbeats,destroyed end,node=node,env=env,target=function()return target end}
end
test('basic native font labels explicitly visible; no tooltip dependency',function()local f=fixture();assert(#f.m.panels==4 and f.m.group.visible);for _,c in ipairs(f.m.panels)do assert(c.visible and not c.interactive)end;assert(f.m.status.text:find('Test179'))end)
test('frame ownership uses plain owned pixel; native root callbacks untouched',function()local f=fixture();assert(not f.root.callbacks.OnUpdatePulse and not f.root.callbacks.OnPreDraw);assert(f.m.group.callbacks.OnUpdatePulse=='OnUpdatePulse'and f.m.group.callbacks.OnPreDraw=='OnPreDraw') end)
test('stock callback is preserved',function()local f=fixture();assert(f.root.globals.stock==f.stock)end)
test('owned component destruction stops only its reader',function()local f=fixture();f.pulse('OnDestroyed');local b,s,e,h,d=f.counts();assert(s==1 and d==0 and not f.m.live)end)
test('native glyph textures populate native components',function()local f=fixture();f.flags.glyphs=true;f.pulse();for _,c in ipairs(f.m.icons)do assert(c.visible and c.texture and c.metrics)end end)
test('B uses stock Back once per fresh edge',function()local f=fixture();f.flags.cancel=true;f.pulse();f.flags.fresh=false;f.pulse();assert(f.counts()==1)end)
test('focus loss prevents Back',function()local f=fixture();f.flags.cancel=true;f.flags.focused=false;f.pulse();assert(f.counts()==0)end)
test('worker handoff hides only owned prompts',function()local f=fixture();f.flags.active=false;f.pulse();assert(not f.m.live and not f.m.status.visible)end)
test('UI creation failure reports without stopping input',function()local ok=pcall(fixture,{fail_create=true});assert(not ok)end)
test('UI update error reports without killing worker',function()local f=fixture();f.m.status.SetStateText=function()error('fixture state failure')end;f.flags.failed=true;f.pulse();local b,s,e=f.counts();assert(s==0 and e==1 and f.m.live)end)
test('binding state is visible and callbacks heartbeat',function()local f=fixture();f.flags.bindings=true;f.pulse();local b,s,e,h=f.counts();assert(f.m.status.text:find('Waiting')and h==1)end)
test('foreign root frame callback remains unchanged',function()local f=fixture({conflict=true});assert(f.root.globals.OnPreDraw and not f.root.callbacks.OnPreDraw)end)
-- Model the exact f948e0 controller branch: update children, then native
-- controller OR Lua; never assume a native root also forwards its Lua pulse.
local function engine_pulse(c,nodes)
 for _,address in ipairs(c.children)do engine_pulse(nodes[address],nodes)end
 if not c.native and c.callbacks.OnUpdatePulse then c.globals[c.callbacks.OnUpdatePulse]()end
end
test('native-controller model bypasses root Lua but updates owned plain child',function()local f=fixture();engine_pulse(f.root,f.nodes);local b,s,e,h=f.counts();assert(h==1)end)
test('old root-only callback control reproduces missing heartbeat in native model',function()local f=fixture();f.m.group.callbacks.OnUpdatePulse=nil;f.root.globals.old=f.m.tick;f.root.callbacks.OnUpdatePulse='old';engine_pulse(f.root,f.nodes);local b,s,e,h=f.counts();assert(h==0)end)
local function add_choice(f,id,y,state)
 local c=f.node(id,f.root.address);c.visible=true;c.interactive=true;c.x=100;c.y=y;c.state=state or'active';return c
end
test('directional focus uses live native geometry and non-button-named choices',function()local f=fixture();add_choice(f,'continue_campaign',100);add_choice(f,'load_game',160);f.flags.down=true;f.pulse();assert(f.target()=='160,115');f.pulse();assert(f.target()=='160,175')end)
test('directional focus skips hidden/disabled choices and safe edge acknowledges',function()local f=fixture();local hidden=add_choice(f,'hidden',30);hidden.visible=false;add_choice(f,'inactive_choice',70,'inactive');add_choice(f,'options',100);f.flags.down=true;f.pulse();assert(f.target()=='160,115');f.pulse();assert(f.target()=='')end)
print('PASS '..passed..' frontend179 float-Lua UI policy checks; actual native UI and Deck unverified')
