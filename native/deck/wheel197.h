#pragma once
#include <gdiplus.h>
#include <algorithm>
#include <cmath>
namespace deck197 {
// All texture/font/mon pixels come from the existing locally generated HUD
// resources. Only geometry and labels ship in the mod, never game artwork.
inline void sector(Gdiplus::GraphicsPath& path,float x,float y,float outer,float inner,float angle){
 const float start=angle-42.f,sweep=84.f;
 path.AddArc(x-outer,y-outer,outer*2,outer*2,start,sweep);
 path.AddArc(x-inner,y-inner,inner*2,inner*2,start+sweep,-sweep);path.CloseFigure();
}
template<class Assets,class Glyph,class Text>void draw(Gdiplus::Graphics& g,const Assets& assets,float width,float height,const wchar_t* mon,int selected,int fire_mode,bool following,Glyph glyph,Text text){
 using namespace Gdiplus;
 if(!std::isfinite(width)||!std::isfinite(height)||width<320||height<240)return;
 const float radius=(std::min)({230.f,width*.24f,height*.28f}),s=radius/210.f,x=width*.5f,y=height*.48f,inner=radius*.34f;
 auto saved=g.Save();g.SetSmoothingMode(SmoothingModeAntiAlias);
 const float angles[]={270,90,180,0};
 const wchar_t* names[]={L"Follow Me",L"Stop",L"Hold Here",L"Fire Orders"};
 const wchar_t* directions[]={L"UP",L"DOWN",L"LEFT",L"RIGHT"};
 const wchar_t* current=fire_mode==0?L"Hold fire":fire_mode==1?L"Fire at will":fire_mode==2?L"On command":L"Cycle mode";
 const RectF all(x-radius,y-radius,radius*2,radius*2);
 for(unsigned i=0;i<4;++i){
  GraphicsPath shape;sector(shape,x,y,radius,inner,angles[i]);
  auto clip=g.Save();g.SetClip(&shape,CombineModeIntersect);
  assets.tile(g,L"ui/frames/data__subpanel_paper_fill.png",all,s);
  if(int(i)==selected){SolidBrush wash(Color(78,151,45,27));g.FillPath(&wash,&shape);}
  g.Restore(clip);
  Pen rim(Color(255,74,53,31),2.f*s),line(Color(225,202,174,119),.8f*s);
  g.DrawPath(&rim,&shape);GraphicsPath inset;sector(inset,x,y,radius-3*s,inner+3*s,angles[i]);g.DrawPath(&line,&inset);
  const float radial=radius*.67f,rad=angles[i]*3.14159265f/180.f;
  const float cx=x+std::cos(rad)*radial,cy=y+std::sin(rad)*radial;
  if(!glyph(i,RectF(cx-14*s,cy-39*s,28*s,28*s)))text(directions[i],RectF(cx-46*s,cy-38*s,92*s,22*s),14*s,Color(255,86,66,44));
  text(names[i],RectF(cx-65*s,cy-5*s,130*s,29*s),20*s,Color(255,45,31,21));
  if(i==3)text(current,RectF(cx-66*s,cy+22*s,132*s,22*s),14*s,Color(255,99,53,37));
  else if(i==0&&following)text(L"Following",RectF(cx-64*s,cy+22*s,128*s,22*s),14*s,Color(255,99,53,37));
 }
 // Circular medallion preserves the native mon's aspect ratio, including
 // transparency. Unknown/custom clan artwork is never replaced by another clan.
 const float disc=inner-7*s;
 GraphicsPath center;center.AddEllipse(x-disc,y-disc,disc*2,disc*2);
 auto clip=g.Save();g.SetClip(&center,CombineModeIntersect);assets.tile(g,L"ui/panel_backgrounds/data__panel_background_fill.png",RectF(x-disc,y-disc,disc*2,disc*2),s);g.Restore(clip);
 Pen border(Color(255,102,75,40),2*s);g.DrawPath(&border,&center);
 if(mon&&*mon){RectF slot(x-disc*.73f,y-disc*.73f,disc*1.46f,disc*1.46f);assets.image(g,mon,assets.fitted(mon,slot));}
 text(L"Release RB to order  |  B to cancel",RectF(x-radius-35*s,y+radius+14*s,radius*2+70*s,26*s),17*s,Color(255,244,232,203));
 g.Restore(saved);
}
}
