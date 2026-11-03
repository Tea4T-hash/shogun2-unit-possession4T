-- Native UI is an explicit mock; no actual Shogun renderer/Deck execution.
local source=assert(arg[1]);local nav_source=assert(arg[2]);local legacy_source=assert(arg[3]);local passed=0
local function test(name,fn)fn();passed=passed+1;print('PASS '..name)end
local function fixture(opt)
 opt=opt or {};local nodes={};local next_id=0;local backs,stops,errors,heartbeats=0,0,0,0
 local flags={active=true,focused=true,ready=true,fresh=true};local methods={}
 local current_owner;local dispatch_adoption;local in_frame=false;local frame_mutations=0;local static_adoptions=0;local explicit_adoptions=0

 local function node(id,parent)
  next_id=next_id+1;local c={id=id,address=next_id,children={},globals={},callbacks={},visible=false,state='active',x=0,y=0,w=120,h=30,interactive=false}
  nodes[next_id]=setmetatable(c,{__index=methods});c.parent=parent
  if parent then table.insert(nodes[parent].children,c.address);if dispatch_adoption then dispatch_adoption(nodes[parent],c.address)end end
  return c
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
 local function adopt(owner,address)
  if in_frame then frame_mutations=frame_mutations+1 end
  local child=assert(nodes[address]);if child.parent then
   local old=nodes[child.parent].children;for i,v in ipairs(old)do if v==address then table.remove(old,i);break end end
  end
  child.parent=owner.address;table.insert(owner.children,address)
  if child~=owner and dispatch_adoption then dispatch_adoption(owner,address)end
 end
 function methods:Adopt(address)explicit_adoptions=explicit_adoptions+1;if opt.fail_adopt then error('fixture owned adopt failure')end;adopt(self,address)end
 current_owner=root
 dispatch_adoption=function(owner,address)
  local fn=owner.globals.OnAdoptChild
  if fn then local old=current_owner;current_owner=owner;local ok,why=pcall(fn,address);current_owner=old;if not ok then error(why,0)end end
 end
 local stock_calls=0;local stock_fails=false;local tooltip
 if opt.tooltip then tooltip=node('stock_tooltip',root.address)end
 root.globals.OnAdoptChild=function(address)
  stock_calls=stock_calls+1;if stock_fails then error('stock adoption failure')end
  if tooltip and address~=tooltip.address then
   static_adoptions=static_adoptions+1;adopt(current_owner,tooltip.address)
  end
 end

 local api={}
 for _,key in ipairs({'active','focused','ready','bindings','failed','glyphs','glyph_failed','cancel','up','down','left','right'})do local k=key;api[k]=function(v)assert(v==true);return flags[k]end end
 local target;api.focus=function(value)target=value;return true end
 api.poll=function(v)assert(v);return flags.fresh end
 api.stop=function(v)assert(v);stops=stops+1 end
 api.error=function(why)assert(type(why)=='string');errors=errors+1 end
 api.heartbeat=function()heartbeats=heartbeats+1 end
 local env={UIComponent=function(a)return assert(nodes[a])end,
  Component={MouseOver=function()return nil end,CreateFromLayout=function(layout,id,parent,x,y)assert(layout=='data/ui/common ui/general_purpose_pixel');if opt.fail_create then error('fixture native layout error')end;return node(id,parent).address end,
   Adopt=function(address)static_adoptions=static_adoptions+1;adopt(current_owner,address)end},
  FrontEnd={ScreenSize=function()return 1280,800 end},
  UIImage=function(path)return {SetComponentTexture=function(self,a,i)assert(i==0 and path:find('%.tga$'));nodes[a].texture=path end}end,
  TransitionBack=function()backs=backs+1 end,OnDestroyed=function()destroyed=destroyed+1 end,OnAdoptChild=root.globals.OnAdoptChild}
 setmetatable(env,{__index=_G});local chunk=assert(loadfile(opt.legacy and legacy_source or source));setfenv(chunk,env)
 local nav=assert(loadfile(nav_source));setfenv(nav,env)
 local m=chunk(root,api,'owned-cache.deck177',nav())
 local function pulse(event)
  local owner=m.group;local fn=owner.globals[owner.callbacks[event or 'OnUpdatePulse']];assert(fn)
  local old=current_owner;current_owner=owner;in_frame=true;fn();in_frame=false;current_owner=old
 end
 return {m=m,flags=flags,nodes=nodes,root=root,stock=stock,pulse=pulse,counts=function()return backs,stops,errors,heartbeats,destroyed end,node=node,env=env,target=function()return target end,
 stats=function()return frame_mutations,static_adoptions,explicit_adoptions,stock_calls end,
 fail_stock=function(value)stock_fails=value end}
end
test('basic native font labels explicitly visible; no tooltip dependency',function()local f=fixture();assert(#f.m.panels==4 and f.m.group.visible);for _,c in ipairs(f.m.panels)do assert(c.visible and not c.interactive)end;assert(f.m.status.text:find('Test180'))end)
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
test('legacy179 control reproduces self-parenting after a new menu is adopted',function()
 local f=fixture({legacy=true});add_choice(f,'new_menu',200);f.pulse()
 assert(f.m.group.parent==f.m.group.address,'Legacy control did not reproduce native owner semantics')
end)
test('new root child keeps owned group parented to root with explicit receiver',function()
 local f=fixture();add_choice(f,'new_menu',200);assert(f.m.group.parent==f.root.address)
 local a,b,c=f.stats();assert(a==0 and b==0 and c==1)
end)
test('repeated update/draw callbacks never change hierarchy',function()
 local f=fixture();add_choice(f,'new_menu',200)
 for i=1,128 do f.pulse(i%2==0 and'OnPreDraw'or'OnUpdatePulse')end
 local a,b=f.stats();assert(a==0 and b==0 and f.m.group.parent==f.root.address)
end)
test('nested stock tooltip adoption stays bounded and above footer',function()
 local f=fixture({tooltip=true});local before=select(4,f.stats());add_choice(f,'new_menu',200)
 local a,b,c,n=f.stats();assert(a==0 and c==1 and n-before<=4 and f.m.group.parent==f.root.address)
 assert(f.nodes[f.root.children[#f.root.children]].id=='stock_tooltip')
end)
test('stock adoption errors propagate and guard resets for later calls',function()
 local f=fixture();f.fail_stock(true);local ok,why=pcall(add_choice,f,'new_menu',200)
 assert(not ok and tostring(why):find('stock adoption failure'))
 f.fail_stock(false);add_choice(f,'later_menu',300);local a,b,c=f.stats();assert(c==1 and f.m.group.parent==f.root.address)
end)
test('owned layering failure preserves stock callback and native input',function()
 local f=fixture({fail_adopt=true});add_choice(f,'new_menu',200);local b,s,e=f.counts()
 assert(s==0 and e==1 and f.m.live and f.m.ui_failed);f.flags.cancel=true;f.pulse();assert(f.counts()==1)
end)
test('destroyed overlay does not re-adopt after the reader stops',function()
 local f=fixture();f.pulse('OnDestroyed');add_choice(f,'later_menu',300);local a,b,c=f.stats();assert(c==0)
end)
print('PASS '..passed..' frontend180 float-Lua UI policy checks; actual native UI and Deck unverified')
