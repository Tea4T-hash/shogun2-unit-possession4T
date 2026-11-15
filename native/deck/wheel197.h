#pragma once
#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include "commander_display76.h"
namespace deck197 {
// All texture/font/mon pixels come from the existing locally generated HUD
// resources. Only geometry and labels ship in the mod, never game artwork.
inline void sector(Gdiplus::GraphicsPath& path,float x,float y,float outer,float inner,float angle,float sweep){
 const float start=angle-sweep*.5f;
 path.AddArc(x-outer,y-outer,outer*2,outer*2,start,sweep);
 path.AddArc(x-inner,y-inner,inner*2,inner*2,start+sweep,-sweep);path.CloseFigure();
}
template<class Assets>void draw_mon(Gdiplus::Graphics& g,const Assets& assets,const wchar_t* mon,Gdiplus::Bitmap* crest,float x,float y,float side){
 Gdiplus::RectF slot(x-side*.5f,y-side*.5f,side,side);
 if(crest&&crest->GetWidth()&&crest->GetHeight()){
  const float w=float(crest->GetWidth()),h=float(crest->GetHeight()),scale=side/(std::max)(w,h);
  if(g.DrawImage(crest,Gdiplus::RectF(x-w*scale*.5f,y-h*scale*.5f,w*scale,h*scale))==Gdiplus::Ok)return;
 }
 if(mon&&*mon)assets.image(g,mon,assets.fitted(mon,slot));
}
inline void draw_pointer(Gdiplus::Graphics& g,float x,float y,float radius,float scale,commander76::WheelPointer direction){
 using namespace Gdiplus;
 if((!direction.x&&!direction.y)||!direction.valid())return;
 const auto saved=g.Save();
 g.TranslateTransform(x+(radius+3*scale)*direction.x,y+(radius+3*scale)*direction.y);
 g.RotateTransform(std::atan2(direction.y,direction.x)*57.295779513f);
 g.ScaleTransform(scale,scale);
 const PointF arrow[]={{-4,0},{19,-12},{29,-7},{23,0},{29,7},{19,12}};
 PointF shadow[6];
 for(unsigned i=0;i<6;++i)shadow[i]={arrow[i].X+2,arrow[i].Y+2.4f};
 SolidBrush shade(Color(190,18,13,10));g.FillPolygon(&shade,shadow,6);
 LinearGradientBrush lacquer(PointF(0,-12),PointF(0,12),Color(255,86,57,49),Color(255,36,26,23));
 g.FillPolygon(&lacquer,arrow,6);
 Pen rim(Color(255,48,32,23),2.5f),inlay(Color(255,212,170,91),1.f);
 rim.SetLineJoin(LineJoinRound);inlay.SetLineJoin(LineJoinRound);
 g.DrawPolygon(&rim,arrow,6);g.DrawPolygon(&inlay,arrow,6);
 const PointF crease[]={{1,0},{19,-7},{25,-6}};
 Pen shine(Color(255,255,223,148),1.f);
 g.DrawLines(&shine,crease,3);g.DrawLine(&shine,1.f,0.f,22.f,0.f);
 g.Restore(saved);
}
template<class Assets,class Text>
void draw_commander(Gdiplus::Graphics& g,const Assets& assets,float width,float height,
                    const wchar_t* mon,Gdiplus::Bitmap* crest,int selected,
                    const commander76::Display& status,bool pc,Text text){
 using namespace Gdiplus;
 const commander76::WheelLayout layout(status.targets);
 const float radius=commander76::wheel_radius(width,height),s=radius/240.f;
 const float x=width*.5f,y=height*.48f,inner=radius*.34f;
 const RectF disc(x-radius,y-radius,radius*2,radius*2);
 const bool available=(status.flags&commander76::Available)!=0;
 for(unsigned i=0;i<layout.size;++i){
  const int id=layout.sectors[i];
  const float angle=layout.angle(i),sweep=layout.sweep()-2.f;
  GraphicsPath shape;sector(shape,x,y,radius,inner,angle,sweep);
  const auto clip=g.Save();g.SetClip(&shape,CombineModeIntersect);
  assets.tile(g,L"ui/frames/data__subpanel_paper_fill.png",disc,s);
  if(id==selected&&available){
   SolidBrush wash(pc?Color(90,151,45,27):Color(140,162,37,28));g.FillPath(&wash,&shape);
  }
  g.Restore(clip);
  Pen rim(Color(255,74,53,31),2.f*s),line(Color(225,202,174,119),.8f*s);
  g.DrawPath(&rim,&shape);
  GraphicsPath inset;sector(inset,x,y,radius-3*s,inner+3*s,angle,sweep);g.DrawPath(&line,&inset);
  const float radians=angle*.01745329252f;
  const float cx=x+std::cos(radians)*radius*.72f,cy=y+std::sin(radians)*radius*.72f;
  wchar_t name[16]{};
  const bool all=id==commander76::army_sector;
  const unsigned bit=all?0:id==commander76::unit_sector?commander76::unit_bit:1u<<(id-commander76::first_group);
  const bool ai=all?(status.flags&(commander76::ArmyAI|commander76::ArmyMixed))!=0:(status.ai&bit)!=0;
  const wchar_t* label=L"Unit";
  if(all)label=L"All";
  else if(id>=commander76::first_group){
   swprintf_s(name,L"Group %d",id-commander76::first_group+1);label=name;
  }
  text(label,RectF(cx-44*s,cy-25*s,88*s,24*s),17*s,Color(255,45,31,21));
  const RectF badge(cx-25*s,cy+2*s,50*s,23*s);
  SolidBrush fill(!available?Color(255,85,77,65):ai?Color(255,123,44,33):Color(255,39,76,67));
  g.FillRectangle(&fill,badge);
  text(!available?L"-":ai?L"AI":L"You",badge,15*s,Color(255,255,249,231));
 }
 const float medallion=inner-6*s;
 GraphicsPath center;center.AddEllipse(x-medallion,y-medallion,medallion*2,medallion*2);
 const auto clip=g.Save();g.SetClip(&center,CombineModeIntersect);
 assets.tile(g,L"ui/panel_backgrounds/data__panel_background_fill.png",
             RectF(x-medallion,y-medallion,medallion*2,medallion*2),s);
 g.Restore(clip);
 Pen rim(Color(255,102,75,40),2*s);g.DrawPath(&rim,&center);
 draw_mon(g,assets,mon,crest,x,y,medallion*1.46f);
 draw_pointer(g,x,y,radius,s,status.pointer);
 const bool rts=(status.flags&commander76::Rts)!=0;
 text(!available?L"Commander unavailable":pc?(rts?L"Click to assign  |  Right-click to close":
      L"V: Unit orders  |  Click to assign  |  Right-click to close"):
      (rts?L"Right stick to choose  |  RT to assign  |  B to close":
      L"A: Unit orders  |  RT to assign  |  B to close"),
      RectF(x-280*s,y+radius+12*s,560*s,25*s),17*s,Color(255,244,232,203));
}
template<class Assets,class Glyph,class Text>void draw(Gdiplus::Graphics& g,const Assets& assets,float width,float height,const wchar_t* mon,int selected,int fire_mode,bool following,Glyph,Text text,const commander76::Display* commander=nullptr,bool pc=false,Gdiplus::Bitmap* crest=nullptr,const commander76::OrderFeedback* feedback=nullptr,commander76::WheelPointer pointer={}){
 using namespace Gdiplus;
 if(!std::isfinite(width)||!std::isfinite(height)||width<320||height<240)return;
 auto saved=g.Save();g.SetSmoothingMode(SmoothingModeAntiAlias);
 if(commander){
  draw_commander(g,assets,width,height,mon,crest,selected,*commander,pc,text);
  g.Restore(saved);
  return;
 }
 const float radius=commander76::wheel_radius(width,height),s=radius/240.f,x=width*.5f,y=height*.48f,inner=radius*.34f;
 const float angles[]={270,90,180,0};
 const wchar_t* orders[]={L"Follow Me",L"Stop",L"Hold Here",L"Fire Orders"};
 constexpr float sweep=84.f;
 const wchar_t* current=fire_mode==0?L"Hold fire":fire_mode==1?L"Fire at will":fire_mode==2?L"On command":L"Cycle mode";
 const RectF all(x-radius,y-radius,radius*2,radius*2);
 const auto now=GetTickCount64();
 for(unsigned i=0;i<4;++i){
  const float angle=angles[i];
  GraphicsPath shape;sector(shape,x,y,radius,inner,angle,sweep);
  auto clip=g.Save();g.SetClip(&shape,CombineModeIntersect);
  assets.tile(g,L"ui/frames/data__subpanel_paper_fill.png",all,s);
  const bool confirmed=feedback&&feedback->flash(i,now);
  if(int(i)==selected||confirmed){
   SolidBrush wash(confirmed?Color(146,223,145,43):pc?Color(78,151,45,27):Color(140,162,37,28));g.FillPath(&wash,&shape);
  }
  g.Restore(clip);
  Pen rim(Color(255,74,53,31),2.f*s),line(Color(225,202,174,119),.8f*s);
  g.DrawPath(&rim,&shape);GraphicsPath inset;sector(inset,x,y,radius-3*s,inner+3*s,angle,sweep);g.DrawPath(&line,&inset);
  const float radial=radius*.67f,rad=angle*3.14159265f/180.f;
  const float cx=x+std::cos(rad)*radial,cy=y+std::sin(rad)*radial;
  text(orders[i],RectF(cx-78*s,cy-12*s,156*s,29*s),20*s,Color(255,45,31,21));
  if(i==3)text(current,RectF(cx-66*s,cy+22*s,132*s,22*s),14*s,Color(255,99,53,37));
  else if(i==0&&following)text(L"Following",RectF(cx-64*s,cy+22*s,128*s,22*s),14*s,Color(255,99,53,37));
  else if(i==2&&feedback&&feedback->hold_pace){
   const auto* icon=feedback->hold_pace==2?L"ui/icons/data__status_run.png":L"ui/icons/data__status_walk.png";
   assets.image(g,icon,assets.fitted(icon,RectF(cx-13*s,cy+20*s,26*s,26*s)));
  }
 }
 // Circular medallion preserves the native mon's aspect ratio, including
 // transparency. Unknown/custom clan artwork is never replaced by another clan.
 const float disc=inner-7*s;
 GraphicsPath center;center.AddEllipse(x-disc,y-disc,disc*2,disc*2);
 auto clip=g.Save();g.SetClip(&center,CombineModeIntersect);assets.tile(g,L"ui/panel_backgrounds/data__panel_background_fill.png",RectF(x-disc,y-disc,disc*2,disc*2),s);g.Restore(clip);
 Pen border(Color(255,102,75,40),2*s);g.DrawPath(&border,&center);
 draw_mon(g,assets,mon,crest,x,y,disc*1.46f);
 draw_pointer(g,x,y,radius,s,pointer);
 const RectF instructions(x-radius-35*s,y+radius+44*s,radius*2+70*s,26*s);

 text(pc?L"Click to order  |  Double-click Hold Here to run":L"RT to order  |  Double RT on Hold Here to run",
      RectF(x-270*s,y+radius+14*s,540*s,26*s),17*s,Color(255,244,232,203));
 text(pc?L"V: Commander  |  Right-click to close":L"A: Commander  |  B to close",
      instructions,17*s,Color(255,244,232,203));
 g.Restore(saved);
}
}
