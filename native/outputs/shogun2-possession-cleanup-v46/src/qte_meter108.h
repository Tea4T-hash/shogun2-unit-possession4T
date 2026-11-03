#pragma once
// Included inside hud80 after Assets/panel/text/mouse_image, for both the real
// QTE runtime and full HUD previews. No gameplay/timeline state is modified.
inline void qte_meter(Graphics& g,const Assets& a,float x,float y,const qte::Snapshot& s){
    if(!s.active)return;
    panel(g,a,RectF(x-150,y-32,300,64));
    constexpr float span=hudlayout108::qte_span,height=20.f;
    const float left=x-span*.5f,top=y-10.f;
    // Hourglass bar: full height at the ends, pinched in the middle, smooth curves.
    const float pinch=5.f,mid=left+span*.5f;
    GraphicsPath track;
    track.AddBezier(PointF(left,top),PointF(left+span*.25f,top),PointF(mid-span*.15f,top+pinch),PointF(mid,top+pinch));
    track.AddBezier(PointF(mid,top+pinch),PointF(mid+span*.15f,top+pinch),PointF(left+span*.75f,top),PointF(left+span,top));
    track.AddLine(PointF(left+span,top),PointF(left+span,top+height));
    track.AddBezier(PointF(left+span,top+height),PointF(left+span*.75f,top+height),PointF(mid+span*.15f,top+height-pinch),PointF(mid,top+height-pinch));
    track.AddBezier(PointF(mid,top+height-pinch),PointF(mid-span*.15f,top+height-pinch),PointF(left+span*.25f,top+height),PointF(left,top+height));
    track.CloseFigure();
    SolidBrush red(Color(255,151,48,37)),blue(Color(255,58,104,170)),orange(Color(255,214,128,40)),
        purple(Color(255,128,72,160)),green(Color(255,139,153,56));
    auto clipped=g.Save();g.SetClip(&track,CombineModeIntersect);
    const uint32_t sweep=qte::sweep_index(s);
    float run_start=0;qte::Zone run=qte::zone_at(0.f,s.serial,sweep);
    const int columns=int(span);
    for(int i=1;i<=columns;++i){
        const float x=float(i)/float(columns);
        const qte::Zone z=i<columns?qte::zone_at(x,s.serial,sweep):qte::Zone(255);
        if(z==run)continue;
        SolidBrush* brush=run==qte::Zone::Green?&green:run==qte::Zone::Purple?&purple:run==qte::Zone::Orange?&orange:run==qte::Zone::Blue?&blue:&red;
        g.FillRectangle(brush,RectF(left+run_start*span,top,(x-run_start)*span,height));
        run_start=x;run=z;
    }
    g.Restore(clipped);
    Pen rim(Color(255,85,67,40),1.2f),light(Color(255,221,208,163),1.f);
    g.DrawPath(&rim,&track);g.DrawLine(&light,left+3,top+1,left+span-3,top+1);
    const float marker=left+qte::reticle_position(s)*span;
    Pen shadow(Color(255,40,31,20),5),pointer(Color(255,255,248,224),2);
    g.DrawLine(&shadow,marker,top-5,marker,top+height+5);g.DrawLine(&pointer,marker,top-5,marker,top+height+5);
    SolidBrush time_track(Color(255,127,111,82)),remaining(Color(255,71,59,40));
    g.FillRectangle(&time_track,left,y+19,span,3.f);
    g.FillRectangle(&remaining,left,y+19,span*std::clamp(qte::response_remaining(s)/float(qte::response_deadline(s)),0.f,1.f),3.f);
    mouse_image(g,RectF(x-32,y+42,64,88),qte::attacking(s.action),s.result==qte::Result::Pending);
    if(s.result==qte::Result::Canceled){
        RectF banner(x-135,y-19,270,38);GraphicsPath shape;round_rect(shape,banner,3);
        SolidBrush backing(Color(248,48,37,25));Pen trim(Color(255,193,164,108),1.5f);
        g.FillPath(&backing,&shape);g.DrawPath(&trim,&shape);
        text(g,L"INTERRUPTED",banner.X+9,banner.Y+2,banner.Width-18,34,25,Color(255,255,241,201),true);
    }
}
