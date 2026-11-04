#pragma once
// Included inside hud80 after Assets/panel/text/mouse_image, for both the real
// QTE runtime and full HUD previews. No gameplay/timeline state is modified.
// Result flash: after a click the bar fills with the colour landed on, spreading out from the
// click point, shows the result in bold white, and disappears 3 s after the click.
// Names: green = Perfect Strike / Perfect Guard, orange = Resisted, purple = Countered,
// red = Critical Fail. Wrong button in green = Perfect Fail (green fill, no one dies).
inline constexpr ULONGLONG ResultShowMs=3000,ResultFillMs=250;
inline Color result_colour(const qte::Snapshot& s){
    switch(s.result){
        case qte::Result::Success:
        case qte::Result::PerfectFail:return Color(255,139,153,56);   // green
        case qte::Result::Purple: return Color(255,128,72,160);       // purple
        case qte::Result::Orange: return Color(255,214,128,40);       // orange
        case qte::Result::Timeout:return qte::attacking(s.action)?Color(255,96,88,76):Color(255,151,48,37); // grey / red
        default:                  return Color(255,151,48,37);        // red: Critical Fail or wrong button
    }
}
inline const wchar_t* result_word(const qte::Snapshot& s){
    const bool attack=qte::attacking(s.action);
    switch(s.result){
        case qte::Result::Success:    return attack?L"Perfect Strike":L"Perfect Guard";
        case qte::Result::PerfectFail:return L"Perfect Fail";
        case qte::Result::Orange:     return attack?L"Attacking":L"Defending";
        case qte::Result::Purple:     return L"Attack Countered";
        case qte::Result::WrongButton:return L"Wrong Button";
        case qte::Result::Timeout:    return attack?L"Attack Missed":L"Guard Critical Fail";
        default:                      return attack?L"Attack Critical Fail":L"Guard Critical Fail";
    }
}
inline void qte_meter(Graphics& g,const Assets& a,float x,float y,const qte::Snapshot& s){
    if(!s.active)return;
    // Time since this exchange was first seen resolved (kept per prompt serial, per process).
    static uint64_t result_serial=0;static ULONGLONG result_start=0;
    const bool resolved=s.result!=qte::Result::Pending&&s.result!=qte::Result::Canceled;
    ULONGLONG result_age=0;
    if(resolved){const ULONGLONG now=GetTickCount64();
        if(result_serial!=s.serial){result_serial=s.serial;result_start=now;}
        result_age=now-result_start;if(result_age>=ResultShowMs)return;}
    panel(g,a,RectF(x-150,y-32,300,64));
    const bool guard=!qte::attacking(s.action);
    if(guard){
        // Guard: red-tinted panel and a thick pulsing red frame, so it is obvious it is time to defend.
        const float pulse=.5f+.5f*std::sin(float(GetTickCount64()%1000ULL)*6.2831853f/1000.f);
        SolidBrush tint(Color(BYTE(70+50*pulse),190,20,20));g.FillRectangle(&tint,RectF(x-150,y-32,300,64));
        Pen frame(Color(BYTE(170+85*pulse),230,30,30),4.f);g.DrawRectangle(&frame,RectF(x-152,y-34,304,68));
    }else{
        // Attack: a lighter panel and an ornate gold and ivory frame with corner diamonds (no red: red is the guard's colour).
        SolidBrush light(Color(70,255,246,214));g.FillRectangle(&light,RectF(x-150,y-32,300,64));
        Pen gold(Color(255,214,176,72),3.f),ivory(Color(230,250,240,210),1.f);
        g.DrawRectangle(&gold,RectF(x-152,y-34,304,68));g.DrawRectangle(&ivory,RectF(x-148,y-30,296,60));
        SolidBrush gem(Color(255,236,200,96));
        const PointF corners[4]={PointF(x-152,y-34),PointF(x+152,y-34),PointF(x-152,y+34),PointF(x+152,y+34)};
        for(const auto& c:corners){PointF d[4]={PointF(c.X,c.Y-6),PointF(c.X+6,c.Y),PointF(c.X,c.Y+6),PointF(c.X-6,c.Y)};g.FillPolygon(&gem,d,4);}
        Pen tick(Color(255,214,176,72),1.5f);
        for(int i=-2;i<=2;++i){const float tx=x+float(i)*60.f;g.DrawLine(&tick,tx-4,y-34,tx+4,y-34);g.DrawLine(&tick,tx-4,y+34,tx+4,y+34);}
    }
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
    // Brighter zone colours on the attack bar; the guard bar keeps its tones.
    SolidBrush red(guard?Color(255,151,48,37):Color(255,214,74,58)),orange(guard?Color(255,214,128,40):Color(255,247,170,64)),
        purple(guard?Color(255,128,72,160):Color(255,170,110,220)),green(guard?Color(255,139,153,56):Color(255,170,206,80));
    auto clipped=g.Save();g.SetClip(&track,CombineModeIntersect);
    const uint32_t sweep=qte::sweep_index(s);
    const float press=qte::pressure(s.advantage);
    float run_start=0;qte::Zone run=qte::zone_at(0.f,s.serial,sweep,!guard,press);
    const int columns=int(span);
    for(int i=1;i<=columns;++i){
        const float x=float(i)/float(columns);
        const qte::Zone z=i<columns?qte::zone_at(x,s.serial,sweep,!guard,press):qte::Zone(255);
        if(z==run)continue;
        SolidBrush* brush=run==qte::Zone::Green?&green:run==qte::Zone::Purple?&purple:run==qte::Zone::Orange?&orange:&red;
        g.FillRectangle(brush,RectF(left+run_start*span,top,(x-run_start)*span,height));
        run_start=x;run=z;
    }
    if(resolved){
        // Fill outwards from the click point; ease-out over 0.25 s, then hold.
        const float click=std::clamp(qte::reticle_position(s),0.f,1.f);
        float t=std::min(1.f,float(result_age)/float(ResultFillMs));t=1.f-(1.f-t)*(1.f-t);
        const float reach=t*std::max(click,1.f-click)*span;
        SolidBrush fill(result_colour(s));
        g.FillRectangle(&fill,RectF(left+click*span-reach,top,reach*2.f,height));
    }
    g.Restore(clipped);
    Pen rim(Color(255,85,67,40),1.2f),light(Color(255,221,208,163),1.f);
    g.DrawPath(&rim,&track);g.DrawLine(&light,left+3,top+1,left+span-3,top+1);
    if(resolved){
        // Bold white word inside the bar.
        FontFamily family(L"Arial");Font bold(&family,14.f,FontStyleBold,UnitPixel);
        StringFormat centred;centred.SetAlignment(StringAlignmentCenter);centred.SetLineAlignment(StringAlignmentCenter);
        SolidBrush outline(Color(200,20,16,12)),white(Color(255,255,255,255));
        const RectF box(left,top,span,height);
        g.DrawString(result_word(s),-1,&bold,RectF(box.X+1,box.Y+1,box.Width,box.Height),&centred,&outline);
        g.DrawString(result_word(s),-1,&bold,box,&centred,&white);
        return;
    }
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
