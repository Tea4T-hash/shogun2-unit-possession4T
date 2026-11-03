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
 function methods:SetStateText(v)self.text=v;if opt.text_reflow then self.x=0;self.y=0 end end
 function methods:Resize(w,h)
  self.resize_calls=(self.resize_calls or 0)+1
  if in_frame and opt.forbid_frame_geometry then error("geometry mutation inside frame")end
  assert(w>0 and h>0)
  if opt.parent_reflow and self.id=='s2up_deck181_group'then
   for _,a in ipairs(self.children)do local c=nodes[a];c.x=c.x*w/self.w;c.y=c.y*h/self.h end
  end
  self.w=w;self.h=h
  if opt.native_frontend_layout210 then
   for _,address in ipairs(self.children)do local c=nodes[address];if c.dock210==5 then c.x=(w-c.w)/2;c.y=(h-c.h)/2 end end
  end
 end
 function methods:MoveTo(x,y)self.x=x;self.y=y end
 function methods:SetImageMetrics()self.metrics=true end
 function methods:ChildCount()return #self.children end
 function methods:Find(i)if type(i)=="string"then for a,v in pairs(nodes)do if v.id==i then return a end end;return nil end;return self.children[i+1]end
 function methods:LuaCall(name)flags.stock_callback={id=self.id,name=name};return true end
 function methods:GlobalExists(name)return self.globals[name]~=nil end
 function methods:SetGlobal(name,fn)assert(type(fn)=='function');self.globals[name]=fn end
 function methods:SetEventCallback(event,name)assert(type(name)=='string'and self.globals[name]);self.callbacks[event]=name end
 local root=node('root');root.visible=true;root.native=true;root.w=1280;root.h=800;if opt.conflict then root.globals.OnPreDraw=function()return "stock",nil,7 end end
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

 local api={prepare=function()return false end}
 if opt.menu_fit209 then
  api.menu_fit209=function(value)
   flags.menu_extent209=value;flags.menu_fit_calls209=(flags.menu_fit_calls209 or 0)+1
   flags.logical_width210=value=='off'and 1310 or 1638;flags.logical_height210=value=='off'and 819 or 1024
   root.w=flags.logical_width210;root.h=flags.logical_height210;return true
  end
  api.ui_viewport185=function()return flags.logical_width210 or 1310,flags.logical_height210 or 819 end
 end
 if opt.bindings183 then
  for _,name in ipairs({'home','arts','finance','diplomacy','clan','end_turn','quick_save','move_speed','encyclopedia'})do local key=name;api[key]=function(v)assert(v==true);local value=flags[key];flags[key]=false;return value end end
  api.click183=function(value)flags.click=value;flags.click_count=(flags.click_count or 0)+1;return true end
 end
 api.context=function(tag)
  flags.last_native_boundary=tag
  return not flags.context_invalid
 end
 for _,key in ipairs({'active','focused','ready','bindings','failed','glyphs','glyph_failed','cancel','up','down','left','right'})do local k=key;api[k]=function(v)assert(v==true);return flags[k]end end
 api.viewport=function(v)assert(v==true);if flags.viewport_absent then return end;return flags.width or 1280,flags.height or 800 end
 api.attach=function(token)flags.token=token;return true end
 api.detach=function(token)if token==flags.token then stops=stops+1 end;return true end
 for _,key in ipairs({'pause','next','previous','pan_left','pan_right','pan_up','pan_down','turn_left','turn_right'})do local k=key;api[k]=function()local v=flags[k];if type(v)=='number'then if v>0 then flags[k]=v-1;return true end;return false end;return v end end
 if opt.legend184 then api.legend184=function(v)assert(v==true);return flags.legend_ready end end
 if opt.ui_scale185 then api.ui_viewport185=function()return 1310,819 end end
 if opt.native_camera then api.camera=function(v)assert(v==true);if not flags.panel_state or flags.panel_state=='map'then flags.native_camera_calls=(flags.native_camera_calls or 0)+1 end;return true end end
 api.back188=function(v)assert(v==true);flags.native_back=(flags.native_back or 0)+1;return true end
 if opt.native_camera then
  api.panel188=function(value)assert(value=='map'or value=='open'or value=='unknown');flags.panel_state=value;return true end
 end
 local target;api.focus=function(value)target=value;return true end
 api.poll=function(v)assert(v);return flags.fresh end
 api.stop=function(v)assert(v);stops=stops+1 end
 api.error=function(why)assert(type(why)=='string');flags.last_error=why;errors=errors+1 end
 api.heartbeat=function()heartbeats=heartbeats+1 end
 local game_math={};for k,v in pairs(math)do if type(v)=="function"then game_math[k]=v end end
 if opt.no_math_functions then game_math={}end
 local env={math=game_math,UIComponent=function(a)return assert(nodes[a])end,
  Component={MouseOver=function()return nil end,CreateFromLayout=function(layout,id,parent,x,y)assert(layout=='data/ui/common ui/general_purpose_pixel');if opt.fail_create then error('fixture native layout error')end;return node(id,parent).address end,
   Adopt=function(address)static_adoptions=static_adoptions+1;adopt(current_owner,address)end},
  FrontEnd={ScreenSize=function()return 1280,800 end},
  UIImage=function(path)return {SetComponentTexture=function(self,a,i)assert(i==0 and path:find('%.tga$'));nodes[a].texture=path end}end,
  CampaignUI={CameraTarget=function()return 10,20,5,0 end,CameraPosition=function()return 10,100,-5 end,SetCameraTargetInstant=function(x,z)flags.camera={x,z};flags.camera_calls=(flags.camera_calls or 0)+1 end,SetCameraHeading=function(v)flags.heading=v end,OpenEscapeMenu=function()flags.pause_called=true end,NextAutoEntitySelection=function()flags.next_called=true end,PreviousAutoEntitySelection=function()flags.previous_called=true end},PanelManager={OpenPanels=function()return flags.panel_open or false end,CloseAllPanels=function()flags.closed=true end},
  TransitionBack=function()backs=backs+1 end,OnDestroyed=function()destroyed=destroyed+1 end,OnAdoptChild=root.globals.OnAdoptChild,OnPreDraw=opt.conflict and root.globals.OnPreDraw or nil}
 setmetatable(env,{__index=_G});local chunk=assert(loadfile(opt.legacy and legacy_source or source));setfenv(chunk,env)
 local nav=assert(loadfile(opt.old_navigation and assert(arg[4])or nav_source));setfenv(nav,env)
 local m=chunk(root,api,'owned-cache.deck177',nav(),opt.campaign and'campaign'or'menu')
 local function pulse(event)
  local owner=m.group;local fn=owner.globals[owner.callbacks[event or 'OnUpdatePulse']];assert(fn)
  local old=current_owner;current_owner=owner;in_frame=true;fn();in_frame=false;current_owner=old
 end
 return {m=m,flags=flags,nodes=nodes,root=root,stock=stock,pulse=pulse,counts=function()return backs,stops,errors,heartbeats,destroyed end,node=node,env=env,target=function()return target end,
 stats=function()return frame_mutations,static_adoptions,explicit_adoptions,stock_calls end,
 fail_stock=function(value)stock_fails=value end}
end
test('current native root reflows once using the same logical viewport',function()
 local f=fixture({native_camera=true,ui_scale185=true});assert(f.root.w==1310 and f.root.h==819)
 f.pulse();f.pulse();assert(f.root.w==1310 and f.root.h==819 and not f.flags.last_error)
end)

test('campaign native shortcuts no longer inspect or click Lua panel objects',function()
 for _,kind in ipairs({'arts','finance','diplomacy','end_turn','encyclopedia'})do
  local f=fixture({campaign=true,native_camera=true,bindings183=true});f.root.Find=function()error('geometry route retired')end
  f.flags[kind]=true;f.pulse();assert(not f.flags.last_error and not f.flags.click and not f.flags.stock_callback and f.m.live)
 end
end)

test('campaign panel keys cannot move the pointer or map from Lua',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});f.flags.arts=true;f.pulse();assert(not f.flags.click and not f.flags.camera and not f.flags.last_error)
end)

test('Clan queues the verified original toolbar button, never a direct Lua/native panel call',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true})
 local c=f.node('button_clan',f.root.address);c.visible=true;c.interactive=true;c.x=1100;c.y=700;c.w=51;c.h=51
 f.flags.clan=true;f.pulse();assert(f.flags.click=='1125,725'and not f.flags.stock_callback and not f.flags.last_error)
 local count=f.flags.click_count;f.pulse();assert(f.flags.click_count==count)
end)
test('Clan refuses an invalid or off-screen original button without poisoning the native context',function()
 for _,bad in ipairs({'width','position','state','visibility'})do
  local f=fixture({campaign=true,native_camera=true,bindings183=true});local c=f.node('button_clan',f.root.address);c.visible=true;c.interactive=true
  if bad=='width'then c.w=function()end elseif bad=='position'then c.x=-1000 elseif bad=='state'then c.state='inactive'else c.visible=false end
  f.flags.clan=true;f.pulse();assert(not f.flags.click and not f.flags.last_error and f.m.live)
 end
end)
test('Clan waits for native Escape to close another panel before queuing a toolbar click',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});local c=f.node('button_clan',f.root.address);c.visible=true;c.interactive=true
 local backs=0;f.m.live=true;f.flags.panel_open=true;f.env.PanelManager.CloseAllPanels=function()error('direct close forbidden')end
 f.flags.clan=true;f.pulse();assert(not f.flags.click and not f.flags.last_error)
 f.flags.panel_open=false;f.pulse();assert(f.flags.click and not f.flags.last_error)
end)

test('missing campaign panel geometry cannot disable controller service',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});f.root.Find=function()error('no geometry available')end
 f.flags.arts=true;f.pulse();assert(not f.flags.click and not f.flags.last_error and f.m.live)
end)

test('end turn no longer mutates native UI from a frame callback',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});f.env.CampaignUI.EndTurn=function()error('native poll owns end turn')end
 f.flags.end_turn=true;f.pulse();assert(not f.flags.stock_callback and not f.flags.click and not f.flags.last_error)
end)

test('native quick-save eligibility and home or speed APIs retain their native gates',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});local saved,home,speed=0,0,0
 f.env.CampaignUI.CanSave=function()return false end;f.env.CampaignUI.QuickSave=function()saved=saved+1 end
 f.env.CampaignUI.ZoomToCapital=function()home=home+1 end;f.env.CampaignUI.ToggleMoveSpeed=function()speed=speed+1 end
 f.flags.quick_save=true;f.flags.home=true;f.flags.move_speed=true;f.pulse();assert(saved==0 and home==1 and speed==1)
 f.env.CampaignUI.CanSave=function()return true end;f.flags.quick_save=true;f.pulse();assert(saved==1 and not f.flags.last_error)
end)
test('encyclopedia no longer needs a visible named campaign button',function()
 local f=fixture({campaign=true,native_camera=true,bindings183=true});f.flags.encyclopedia=true;f.pulse();assert(not f.flags.click and not f.flags.last_error)
end)

test('basic native font labels explicitly visible; no tooltip dependency',function()local f=fixture();assert(#f.m.panels==4 and f.m.group.visible);for _,c in ipairs(f.m.panels)do assert(c.visible and not c.interactive)end;assert(f.m.status.text:find('Menu controls'))end)
test('frame ownership uses plain owned pixel; native root callbacks chain stock',function()local f=fixture();assert(f.root.callbacks.OnUpdatePulse=='OnUpdatePulse'and f.root.callbacks.OnPreDraw=='OnPreDraw');assert(f.m.group.callbacks.OnUpdatePulse=='OnUpdatePulse'and f.m.group.callbacks.OnPreDraw=='OnPreDraw') end)
test('stock callback is preserved',function()local f=fixture();assert(f.root.globals.stock==f.stock)end)
test('owned component destruction stops only its reader',function()local f=fixture();f.pulse('OnDestroyed');local b,s,e,h,d=f.counts();assert(s==1 and d==0 and not f.m.live)end)
test('native glyph textures populate native components',function()local f=fixture();f.flags.glyphs=true;f.pulse();for _,c in ipairs(f.m.icons)do assert(c.visible and c.texture and c.metrics)end end)
test('B uses stock Back once per fresh edge',function()local f=fixture();f.flags.cancel=true;f.pulse();f.flags.fresh=false;f.pulse();assert(f.counts()==1)end)
test('focus loss prevents Back',function()local f=fixture();f.flags.cancel=true;f.flags.focused=false;f.pulse();assert(f.counts()==0)end)
test('worker handoff hides only owned prompts',function()local f=fixture();f.flags.active=false;f.pulse();assert(not f.m.live and not f.m.status.visible)end)
test('UI creation failure reports without stopping input',function()local ok=pcall(fixture,{fail_create=true});assert(not ok)end)
test('UI update error reports without killing worker',function()local f=fixture();f.m.status.SetStateText=function()error('fixture state failure')end;f.flags.failed=true;f.pulse();local b,s,e=f.counts();assert(s==0 and e==1 and f.m.live)end)
test('binding state is visible and callbacks heartbeat',function()local f=fixture();f.flags.bindings=true;f.pulse();local b,s,e,h=f.counts();assert(f.m.status.text:find('Waiting')and h==1)end)
test('foreign root frame callback is delegated',function()local f=fixture({conflict=true});assert(f.root.globals.OnPreDraw and f.root.callbacks.OnPreDraw=='OnPreDraw')end)
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

test('campaign has no legend but its native input callback stays live',function()local f=fixture({campaign=true,native_camera=true});f.flags.glyphs=true;f.pulse();assert(#f.m.panels==0 and #f.m.icons==0 and not f.m.status and not f.m.group.visible and f.m.live and f.flags.native_camera_calls==1)end)
test('campaign native camera consumes bounded step amounts once',function()local f=fixture({campaign=true});f.flags.pan_right=4;f.flags.pan_up=2;f.pulse();assert(f.flags.camera[1]==10.5 and f.flags.camera[2]==5.25 and f.flags.camera_calls==1);f.pulse();assert(f.flags.camera_calls==1)end)
test('campaign camera backlog remains bounded per native frame',function()local f=fixture({campaign=true});f.flags.pan_right=1000;f.pulse();assert(f.flags.camera[1]==14 and f.flags.pan_right==968)end)
test('campaign back queues native Escape without closing panels from our frame',function()local f=fixture({campaign=true});f.flags.cancel=true;f.pulse();assert(f.flags.native_back==1 and not f.flags.closed and f.counts()==0)end)
test('campaign uses native pause and selection functions',function()local f=fixture({campaign=true});f.flags.next=true;f.flags.previous=true;f.pulse();assert(f.flags.next_called and f.flags.previous_called);f.flags.pause=true;f.pulse();assert(f.flags.native_back==1 and not f.flags.pause_called)end)
test('root frame callback survives a missing child pulse',function()local f=fixture();f.m.group.callbacks.OnUpdatePulse=nil;f.root.globals.OnUpdatePulse();assert(select(4,f.counts())==1)end)
test('root stock callback return arity is preserved',function()local f=fixture({conflict=true});local a,b,c=f.root.globals.OnPreDraw();assert(a=='stock'and b==nil and c==7 and select(4,f.counts())==1)end)
test('native bearing remains usable without optional camera position',function()local f=fixture({campaign=true});f.env.CampaignUI.CameraPosition=nil;f.flags.turn_right=1;f.pulse();assert(f.flags.heading==.01)end)

test('old navigator reproduces photographed function-valued ChildCount error',function()local f=fixture({old_navigation=true});f.root.ChildCount=function()return function()error('never invoke returned function')end end;f.flags.down=true;f.pulse();assert(f.flags.last_error and f.flags.last_error:find("number expected, got function",1,true))end)
test('function-valued root count never converts directions into mouse movement',function()local f=fixture();f.root.ChildCount=function()return function()error('not a callback')end end;f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error and f.m.panels[3].text=='Navigate')end)
test('unreadable child does not hide a valid menu choice',function()local f=fixture();local bad=add_choice(f,'unreadable',40);bad.ChildCount=function()return function()end end;add_choice(f,'single_player',100);f.flags.down=true;f.pulse();assert(f.target()=='160,115'and not f.flags.last_error)end)
test('function-valued geometry is skipped without invoking it',function()local f=fixture();local called=false;local bad=add_choice(f,'bad_geometry',40);bad.Width=function()return function()called=true end end;add_choice(f,'options',100);f.flags.down=true;f.pulse();assert(not called and f.target()=='160,115'and not f.flags.last_error)end)
test('malformed counts refuse functions nonfinite strings and negatives',function()for _,value in ipairs({function()end,math.huge,0/0,'12',false,-1})do local f=fixture();local v=value;f.root.ChildCount=function()return v end;f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error)end end)
test('directional navigation recovers after native getters recover',function()local f=fixture();add_choice(f,'options',100);local original=f.root.ChildCount;f.root.ChildCount=function()return function()end end;f.flags.down=true;f.pulse();f.root.ChildCount=original;f.flags.down=false;for i=1,60 do f.pulse()end;assert(f.target()==''and f.m.panels[3].text=='Navigate');f.flags.down=true;f.pulse();assert(f.target()=='160,115')end)
test('function-valued child address is not passed to UIComponent',function()local f=fixture();local original=f.root.Find;f.root.Find=function(self,i)if i==0 then return function()error('unknown returned callback invoked')end end;return original(self,i)end;f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error)end)
test('adoption guard tolerates function-valued count and keeps stock behavior',function()local f=fixture();f.root.ChildCount=function()return function()end end;add_choice(f,'later_panel',100);assert(not f.flags.last_error and f.m.group.parent==f.root.address)end)

test('old finite check reproduces missing math huge error',function()local old=assert(loadstring('return function(v)return type(v)=="number"and v==v and v~=math.huge and v~=-math.huge end'));setfenv(old,setmetatable({math={}},{__index=_G}));local ok,why=pcall(old(),1);assert(not ok and tostring(why):find("huge",1,true))end)
test('directional menu works without any math table members',function()local f=fixture({no_math_functions=true});add_choice(f,'single_player',100);add_choice(f,'options',160);f.flags.down=true;f.pulse();assert(f.target()=='160,115'and not f.flags.last_error);f.pulse();assert(f.target()=='160,175'and not f.flags.last_error)end)
test('nonfinite counts are rejected without a huge constant',function()local f=fixture({no_math_functions=true});f.root.ChildCount=function()return 1/0 end;f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error)end)
test('campaign pan stays available without optional sqrt and atan2',function()local f=fixture({campaign=true,no_math_functions=true});f.flags.pan_right=4;f.flags.turn_right=1;f.pulse();assert(f.flags.camera[1]==10.5 and f.flags.camera[2]==5 and f.flags.heading==.01 and not f.flags.last_error)end)
test('native viewport isolates all menu inputs from broken scripted size',function()
 local f=fixture();local called=false;f.env.FrontEnd.ScreenSize=function()return function()called=true end,function()called=true end end
 local inputs={'cancel','up','down','left','right','pause','next','previous','pan_left','pan_right','pan_up','pan_down','turn_left','turn_right'}
 for _,key in ipairs(inputs)do f.flags[key]=true;f.pulse();f.flags[key]=false;assert(not f.flags.last_error and f.m.live)end
 assert(not called and not f.m.ui_failed and f.m.group.w==1280 and f.m.group.h==800)
end)
test('malformed viewport and scripted size defer layout without stopping controls',function()
 local f=fixture();local called=false;f.flags.viewport_absent=true;f.env.FrontEnd.ScreenSize=function()return function()called=true end,nil end
 for _,key in ipairs({'up','down','left','right','cancel'})do f.flags[key]=true;f.pulse();f.flags[key]=false end
 assert(not called and not f.flags.last_error and not f.m.ui_failed and f.m.live and f.counts()==1)
end)
test('invalid sizes never reach setters and valid resize recovers',function()
 local f=fixture();f.flags.viewport_absent=true
 for _,v in ipairs({false,'1280',function()error('never call value')end,math.huge,0/0,-1,40000})do local bad=v;f.env.FrontEnd.ScreenSize=function()return bad,bad end;f.pulse();assert(not f.flags.last_error and not f.m.ui_failed)end
 f.env.FrontEnd.ScreenSize=function()return 1920,1080 end;f.pulse();assert(f.m.group.w==1920 and f.m.group.h==1080 and not f.flags.last_error)
end)
test('size getter throwing does not swallow native back input',function()
 local f=fixture();f.flags.viewport_absent=true;f.env.FrontEnd.ScreenSize=function()error('fixture native query unavailable')end;f.flags.cancel=true;f.pulse();f.flags.cancel=false;f.pulse();assert(f.counts()==1 and not f.flags.last_error and not f.m.ui_failed)
end)
test('layout takes a single validated viewport snapshot',function()
 local f=fixture();local calls=0;f.env.FrontEnd.ScreenSize=function()error('script size must not be needed')end
 f.env.math={};f.flags.width=1920;f.flags.height=1080;f.pulse();assert(f.m.group.w==1920 and f.m.group.h==1080 and not f.flags.last_error)
end)
test('campaign camera invalid results never participate in arithmetic',function()
 local f=fixture({campaign=true});local called=false;f.env.CampaignUI.CameraTarget=function()return function()called=true end,'bad' end;f.flags.pan_right=4;f.flags.turn_right=1;f.pulse();assert(not called and not f.flags.camera and not f.flags.last_error)
 f.env.CampaignUI.CameraTarget=function()return 10,20,5,0 end;f.env.CampaignUI.CameraPosition=function()return function()called=true end,false end;f.flags.pan_right=4;f.flags.turn_right=1;f.pulse();assert(not called and f.flags.camera[1]==10.5 and f.flags.heading==.01 and not f.flags.last_error)
end)
test('whole geometry boundary fails safely and recovers during navigation',function()
 local f=fixture();add_choice(f,'options',100);local saved={};for _,name in ipairs({'ChildCount','Width','Height','Position','Visible','CurrentState','Id','IsInteractive','Find'})do saved[name]=f.root[name];f.root[name]=function()return function()error('not a callback')end end end
 f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error and not f.m.ui_failed)
 for name,fn in pairs(saved)do f.root[name]=fn end;f.flags.down=false;for i=1,60 do f.pulse()end;f.flags.down=true;f.pulse();assert(f.target()=='160,115'and not f.flags.last_error)
end)
test('campaign pan preserves horizontal z rather than height or zoom',function()
 local f=fixture({campaign=true});f.env.CampaignUI.CameraTarget=function()return 410,87,265,0,'main' end;f.flags.pan_right=4;f.pulse();assert(f.flags.camera[1]==410.5 and f.flags.camera[2]==265 and not f.flags.last_error)
end)
test('changing zoom alone never shifts the horizontal map target',function()
 local f=fixture({campaign=true});for _,height in ipairs({2,50,2000})do local v=height;f.env.CampaignUI.CameraTarget=function()return 30,v,400,0 end;f.flags.pan_right=2;f.pulse();assert(f.flags.camera[1]==30.25 and f.flags.camera[2]==400)end
end)
test('campaign right-stick turning uses the native bearing field',function()
 local f=fixture({campaign=true});f.env.CampaignUI.CameraTarget=function()return 30,77,400,1.5 end;f.flags.turn_right=3;f.pulse();assert(math.abs(f.flags.heading-1.53)<.0001 and not f.flags.camera)
end)
test('campaign orientation fallback reads third position coordinate',function()
 local f=fixture({campaign=true,no_math_functions=true});f.env.CampaignUI.CameraTarget=function()return 30,77,400,0 end;f.flags.pan_up=2;f.pulse();assert(f.flags.camera[1]==30 and f.flags.camera[2]==400.25)
end)
test('parent resizing happens before children are laid out',function()
 local f=fixture({parent_reflow=true});f.flags.width=1920;f.flags.height=1080;f.pulse();assert(f.m.panels[1].x==632 and f.m.panels[1].y==47);assert(f.m.icons[1].x==596 and f.m.icons[1].y==48)
end)
test('native text reflow cannot scatter stable prompt positions',function()
 local f=fixture({text_reflow=true});f.flags.bindings=true;f.pulse();assert(f.m.status.x==276 and f.m.status.y==14);f.pulse();assert(f.m.panels[3].x==676 and f.m.panels[3].y==47 and not f.flags.last_error)
end)
test('prompt bounds stay inside common Deck and desktop viewports',function()
 local f=fixture({parent_reflow=true,text_reflow=true});for _,size in ipairs({{1280,800},{1280,720},{1920,1080},{640,360}})do f.flags.width=size[1];f.flags.height=size[2];f.pulse();for _,c in ipairs(f.m.panels)do assert(c.x>=0 and c.y>=0 and c.x+c.w<=size[1]and c.y+c.h<=size[2])end end
end)
test('same-size menu adoption reasserts the owned prompt layout',function()
 local f=fixture({text_reflow=true});f.m.panels[1].x=-400;add_choice(f,'new_page',100);f.pulse();assert(f.m.panels[1].x==312 and f.m.group.x==0 and f.m.group.y==0)
end)
test('campaign four cardinal directions remain distinct at multiple bearings without native math bindings',function()
 for _,bearing in ipairs({0,math.pi/2,math.pi,-math.pi/2})do
  for _,pair in ipairs({{'pan_right',1,0},{'pan_left',-1,0},{'pan_up',0,1},{'pan_down',0,-1}})do
   local f=fixture({campaign=true,no_math_functions=true});f.env.CampaignUI.CameraTarget=function()return 100,20,100,bearing end
   f.flags[pair[1]]=4;f.pulse();local x,z=f.flags.camera[1]-100,f.flags.camera[2]-100
   local ex=.5*(math.cos(bearing)*pair[2]+math.sin(bearing)*pair[3]);local ez=.5*(-math.sin(bearing)*pair[2]+math.cos(bearing)*pair[3])
   assert(math.abs(x-ex)<.0003 and math.abs(z-ez)<.0003 and not f.flags.last_error)
  end
 end
end)
test('campaign heading arithmetic never calls cached native math functions',function()
 local f=fixture({campaign=true});for _,name in ipairs({'sin','cos','sqrt'})do f.env.math[name]=function()error('native math binding must not be cached or invoked')end end
 f.flags.pan_right=4;f.pulse();assert(f.flags.camera and not f.flags.last_error)
end)
test('invalid native context blocks campaign setters while pointer worker remains alive',function()
 local f=fixture({campaign=true});f.flags.context_invalid=true;f.flags.pan_right=4;f.pulse()
 assert(not f.flags.camera and f.m.live and f.flags.last_error)
end)
test('native UI parser failure identifies the offending boundary',function()
 local f=fixture();local prior=f.m.status.SetStateText
 f.m.status.SetStateText=function(self,v)prior(self,v);f.flags.context_invalid=true end
 f.flags.failed=true;f.pulse()
 assert(f.flags.last_error:find('invalidated by SetStateText',1,true)and f.m.ui_failed and f.m.live)
end)
test('native result types are recorded without invoking returned functions',function()
 local f=fixture();f.root.ChildCount=function()return function()error('not a callback')end end
 f.flags.down=true;f.pulse();assert(f.target()==''and not f.flags.last_error)
end)
test('current campaign bridge bypasses scripted coordinate arithmetic',function()
 local f=fixture({campaign=true,native_camera=true})
 f.env.CampaignUI.CameraTarget=function()error('old getter must not be called')end
 f.env.CampaignUI.SetCameraTargetInstant=function()error('old setter must not be called')end
 f.pulse();assert(f.flags.native_camera_calls==1 and not f.flags.last_error)
end)
test('current bridge menus never inspect directional component geometry',function()
 local f=fixture({native_camera=true});f.root.ChildCount=function()error('directional menus deferred')end
 for i=1,120 do f.pulse()end
 assert(not f.flags.last_error and f.m.live)
end)
test('current main menu has no visual legend but keeps its input owner',function()
 local f=fixture({native_camera=true});assert(not f.m.status and #f.m.panels==0 and not f.m.group.visible)
 f.flags.focused=false;f.pulse();f.flags.focused=true;f.pulse();assert(f.m.live and not f.m.status and not f.m.group.visible)
end)
test('campaign legend is absent even when an old texture-ready export exists',function()
 local f=fixture({native_camera=true,campaign=true,legend184=true});f.flags.legend_ready=true;f.pulse()
 assert(not f.m.strip and not f.m.status and #f.m.panels==0 and not f.m.group.visible and f.m.live)
end)

test('rapid finance/diplomacy performs no panel mutation inside a Lua frame callback',function()
 local f=fixture({native_camera=true,campaign=true,bindings183=true});for _,id in ipairs({'button_finance','button_diplomacy'})do local c=f.node(id,f.root.address);c.visible=true;c.interactive=true;c.x=100;c.y=100 end
 f.env.ShowFinancePopup=function()error('unsafe immediate panel open')end;f.env.ToggleDiplomacyPopup=function()error('unsafe immediate panel open')end
 for n=1,100 do f.flags[n%2==0 and'finance'or'diplomacy']=true;f.pulse();assert(not f.flags.last_error)end
 assert(not f.flags.click_count and not f.flags.stock_callback and f.m.live)
end)
test('quiet input owner performs no UI mutations during1000 rapid panel frames',function()
 local f=fixture({native_camera=true,campaign=true,bindings183=true,forbid_frame_geometry=true})
 f.m.group.SetVisible=function()error('visibility mutation inside frame')end
 for n=1,1000 do f.flags[n%2==0 and'finance'or'diplomacy']=true;f.pulse()end
 assert(not f.flags.last_error and f.m.live and not f.m.ui_failed)
end)
test('quiet main menu never resizes its hidden controller owner',function()
 local f=fixture({native_camera=true,forbid_frame_geometry=true})
 for n=1,1000 do f.pulse()end
 assert(not f.m.group.resize_calls and not f.flags.last_error)
end)
test('news and reviews hidden on main-menu adoption without affecting other windows',function()
 local f=fixture({native_camera=true});local news=f.node('weblinks',f.root.address);news.visible=true;news.interactive=true
 local advert=f.node('advert',f.root.address);advert.visible=true;advert.interactive=true
 local options=f.node('options',f.root.address);options.visible=true;options.interactive=true
 -- Creation/adoption precedes data-driven visibility in this mock. A following
 -- menu adoption models the completed main-screen adoption event.
 f.node('main_complete',f.root.address)
 assert(not news.visible and not news.interactive and not advert.visible and not advert.interactive and options.visible and options.interactive)
end)
test('campaign never hides its native advert-like or web objects',function()
 local f=fixture({native_camera=true,campaign=true});local news=f.node('weblinks',f.root.address);news.visible=true;news.interactive=true;f.node('next',f.root.address)
 assert(news.visible and news.interactive and not f.flags.last_error)
end)
test('original modal getter gates native camera without panel geometry',function()
 local f=fixture({campaign=true,native_camera=true});f.root.Find=function()error('no geometry scan')end
 f.flags.panel_open=true;f.pulse();assert(f.flags.panel_state=='open'and not f.flags.native_camera_calls and not f.flags.last_error)
 f.flags.panel_open=false;f.pulse();assert(f.flags.panel_state=='map'and f.flags.native_camera_calls==1)
end)
test('invalid modal getter result refuses map control',function()
 local f=fixture({campaign=true,native_camera=true});f.env.PanelManager.OpenPanels=function()return function()error('not a callback')end end
 f.pulse();assert(f.flags.panel_state=='unknown'and not f.flags.native_camera_calls)
end)
test('campaign B never calls destructive native panel API in frame',function()
 local f=fixture({campaign=true,native_camera=true});f.env.PanelManager.CloseAllPanels=function()error('unsafe close')end
 f.flags.panel_open=true;f.flags.cancel=true;f.pulse();assert(f.flags.native_back==1 and not f.flags.last_error)
end)
test('campaign Menu queues native Escape instead of immediate pause mutation',function()
 local f=fixture({campaign=true,native_camera=true});f.env.CampaignUI.OpenEscapeMenu=function()error('unsafe pause')end
 f.flags.pause=true;f.pulse();assert(f.flags.native_back==1 and not f.flags.last_error)
end)
test('modal observation remains read-only across1000 frames',function()
 local f=fixture({campaign=true,native_camera=true,forbid_frame_geometry=true});f.flags.panel_open=true
 f.m.group.SetVisible=function()error('frame visibility mutation')end
 for n=1,1000 do f.pulse()end;assert(f.flags.panel_state=='open'and not f.flags.native_camera_calls and not f.flags.last_error)
end)
test('Japan campaign setup uses its whole authored screen extent',function()
 local f=fixture({native_camera=true,menu_fit209=true});local screen=f.node('grand_campaign_japan',f.root.address);screen.visible=true;screen.w=1024;screen.h=819
 local footer=f.node('start_button_bg',screen.address);footer.y=921;footer.w=379;footer.h=56
 f.pulse();assert(f.flags.menu_extent209=='1280,1024'and not f.flags.last_error)
 assert(screen.w==1638 and screen.h==1024 and footer.y==921 and footer.h==56 and not footer.resize_calls)
 for n=1,60 do f.pulse()end;assert(f.flags.menu_fit_calls209==1)
end)
test('Boshin campaign footer fits and normal menu scale returns on exit',function()
 local f=fixture({native_camera=true,menu_fit209=true});local screen=f.node('grand_campaign_boshin',f.root.address);screen.visible=true;screen.w=1280;screen.h=1024
 f.pulse();assert(f.flags.menu_extent209=='1280,1024')
 screen.visible=false;f.pulse();assert(f.flags.menu_extent209=='off'and f.flags.menu_fit_calls209==2 and not f.flags.last_error)
end)
test('campaign map and invalid frontend extents cannot change menu scaling',function()
 local f=fixture({native_camera=true,campaign=true,menu_fit209=true});local screen=f.node('grand_campaign_japan',f.root.address);screen.visible=true;screen.w=1280;screen.h=1024
 f.pulse();assert(not f.flags.menu_extent209 and not f.flags.last_error)
 local g=fixture({native_camera=true,menu_fit209=true});local bad=g.node('grand_campaign_japan',g.root.address);bad.visible=true;bad.w=9000;bad.h=1024
 g.pulse();assert(g.flags.menu_extent209=='off'and not g.flags.last_error)
end)
test('captured Japan parent clipping is repaired without shrinking its contents',function()
 local f=fixture({native_camera=true,menu_fit209=true,native_frontend_layout210=true})
 local screen=f.node('grand_campaign_japan',f.root.address);screen.visible=true;screen.w=1310;screen.h=819;screen.x=164;screen.y=102
 local main=f.node('main_panel',screen.address);main.w=1104;main.h=912;main.x=103;main.y=-46;main.dock210=5
 local footer=f.node('start_button_bg',main.address);footer.x=363;footer.y=865;footer.w=379;footer.h=56
 local button=f.node('button_forward',footer.address);button.x=72;button.y=9;button.w=234;button.h=38;button.interactive=true
 assert(main.y+footer.y>=screen.h) -- live failure: ancestor clips the complete footer
 f.pulse();assert(not f.flags.last_error and screen.w==1638 and screen.h==1024 and screen.x==0 and screen.y==0)
 assert(main.w==1104 and main.h==912 and main.x==267 and main.y==56 and not main.resize_calls and not footer.resize_calls)
 assert(main.y+footer.y+footer.h<=screen.h and button.interactive)
 assert(main.y+footer.y+button.y+button.h<=screen.h)
 local calls=screen.resize_calls;for n=1,60 do f.pulse()end;assert(screen.resize_calls==calls and f.flags.menu_fit_calls209==1)
end)

test('Boshin footer and native forward button remain inside every ancestor',function()
 local f=fixture({native_camera=true,menu_fit209=true,native_frontend_layout210=true})
 local screen=f.node('grand_campaign_boshin',f.root.address);screen.visible=true;screen.w=1310;screen.h=819;screen.x=164;screen.y=102
 local footer=f.node('start_button_bg',screen.address);footer.x=450;footer.y=968;footer.w=379;footer.h=56
 local button=f.node('button_forward',footer.address);button.x=72;button.y=9;button.w=234;button.h=38;button.interactive=true
 f.pulse();assert(not f.flags.last_error and screen.x==0 and screen.y==0 and footer.y+footer.h<=screen.h)
 assert(button.y+button.h<=footer.h and button.interactive and not footer.resize_calls and not button.resize_calls)
 screen.visible=false;f.pulse();assert(f.flags.menu_extent209=='off'and f.root.h==819)
end)

test('campaign container reflow recovers from late native adoption with cached extent',function()
 local f=fixture({native_camera=true,menu_fit209=true});local screen=f.node('grand_campaign_japan',f.root.address);screen.visible=true;screen.w=1310;screen.h=819
 f.pulse();assert(screen.w==1638 and screen.h==1024)
 screen.w=1310;screen.h=819;screen.x=164;screen.y=102
 f.pulse();assert(screen.w==1638 and screen.h==1024 and screen.x==0 and screen.y==0 and f.flags.menu_fit_calls209==1)
 local calls=screen.resize_calls;screen.visible=false;f.pulse();assert(screen.resize_calls==calls and f.root.h==819 and not f.flags.last_error)
end)
print('PASS '..passed..' frontend181 float-Lua UI policy checks; actual native UI and Deck unverified')
