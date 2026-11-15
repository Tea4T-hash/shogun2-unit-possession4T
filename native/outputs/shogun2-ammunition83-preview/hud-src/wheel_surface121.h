#pragma once
#include <array>
#include <memory>
#include <string>
#include <vector>

// Keep the wheel body rasterized between pointer changes. No game resources or
// presentation objects are owned by this cache, only two bounded cropped DIBs.
class WheelSurface121 {
    std::array<int,16> key_{};
    std::wstring mon_;
    std::shared_ptr<card217::Picture> crest_;
    std::vector<unsigned char> body_, frame_;
    static void configure(Gdiplus::Graphics& g,float scale,int x,int y){
        using namespace Gdiplus;
        Matrix position(scale,0,0,scale,float(-x),float(-y));
        g.SetTransform(&position);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(PixelOffsetModeHalf);
    }
public:
    const unsigned char* render(const hud80::Assets& assets,RECT client,int x,int y,int w,int h,
        const wchar_t* mon,const std::shared_ptr<card217::Picture>& crest,bool boshin,
        unsigned lease,int selected,int fire_mode,bool following,bool pc,
        const commander76::Display* commander,const commander76::OrderFeedback* feedback,
        commander76::WheelPointer pointer,bool* rebuilt=nullptr){
        using namespace Gdiplus;
        if(w<=0||h<=0||uint64_t(w)*h*4>overlay106::capacity(overlay106::Wheel))return nullptr;
        const float scale=hudlayout108::scale(float(client.right),float(client.bottom));
        const float width=client.right/scale,height=client.bottom/scale;
        const auto now=GetTickCount64();
        unsigned flashes=0;
        if(feedback&&!commander)for(unsigned i=0;i<4;++i)if(feedback->flash(i,now))flashes|=1u<<i;
        const std::array<int,16> key{client.right,client.bottom,x,y,w,h,int(lease),
            selected,fire_mode,following,pc,boshin,int(commander?commander->flags:0),
            int(commander?commander->targets:0),int(commander?commander->ai:0),
            int(feedback&&!commander?(flashes<<2)|feedback->hold_pace:0)};
        const bool redraw=body_.empty()||key_!=key||mon_!=mon||crest_!=crest;
        if(rebuilt)*rebuilt=redraw;
        if(redraw){
            body_.assign(size_t(w)*h*4,0);
            Bitmap surface(w,h,w*4,PixelFormat32bppPARGB,body_.data());
            Graphics g(&surface);
            configure(g,scale,x,y);
            hud80::native_fonts.boshin=boshin;
            auto state=commander?*commander:commander76::Display{};
            state.pointer={};
            std::optional<Bitmap> bitmap;
            deck181::wheel_mon_bitmap(bitmap,crest.get());
            deck197::draw(g,assets,width,height,mon,selected,fire_mode,following,
                [](unsigned,RectF){return false;},
                [&](const wchar_t* label,RectF box,float size,Color color){
                    hud80::native_fonts.paint(g,label,box,size,color,true);
                },commander?&state:nullptr,pc,bitmap?&*bitmap:nullptr,feedback);
            key_=key;
            mon_=mon;
            crest_=crest;
        }
        if((!pointer.x&&!pointer.y)||!pointer.valid())return body_.data();
        frame_=body_;
        Bitmap surface(w,h,w*4,PixelFormat32bppPARGB,frame_.data());
        Graphics g(&surface);
        configure(g,scale,x,y);
        const float radius=commander76::wheel_radius(width,height);
        deck197::draw_pointer(g,width*.5f,height*.48f,radius,radius/240.f,pointer);
        return frame_.data();
    }
};
