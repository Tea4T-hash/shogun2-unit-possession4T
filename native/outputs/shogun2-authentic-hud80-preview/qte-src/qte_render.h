#pragma once
#define HUD80_QTE_ONLY
#include "../src/hud_renderer.h"
namespace qte {
inline std::unique_ptr<hud80::Assets> renderer_assets;
inline bool renderer_failed=false;
inline void release_renderer(){renderer_assets.reset();renderer_failed=false;}
inline void render(Gdiplus::Graphics& g,float x,float y,const Snapshot& s){
    if(!s.active||renderer_failed)return;
    if(!renderer_assets){
        HMODULE owner=nullptr;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&render),&owner)){renderer_failed=true;return;}
        auto assets=std::make_unique<hud80::Assets>();
        if(!assets->load(owner)){renderer_failed=true;return;}renderer_assets=std::move(assets);
    }
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    hud80::qte_meter(g,*renderer_assets,x,y,s);
}
// Kit v10: shown for a few seconds when the possessed soldier dies.
inline void render_banner173(Gdiplus::Graphics& g,float x,float y,const wchar_t* line){
    using namespace Gdiplus;
    g.SetSmoothingMode(SmoothingModeAntiAlias);g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    const RectF box(x-140,y-24,280,48);
    SolidBrush back(Color(220,30,22,16));g.FillRectangle(&back,box);
    Pen gold(Color(255,214,176,72),3.f),ivory(Color(230,250,240,210),1.f);
    g.DrawRectangle(&gold,RectF(box.X-2,box.Y-2,box.Width+4,box.Height+4));g.DrawRectangle(&ivory,RectF(box.X+2,box.Y+2,box.Width-4,box.Height-4));
    FontFamily family(L"Arial");Font bold(&family,20.f,FontStyleBold,UnitPixel);
    StringFormat centred;centred.SetAlignment(StringAlignmentCenter);centred.SetLineAlignment(StringAlignmentCenter);
    SolidBrush outline(Color(220,20,16,12)),white(Color(255,255,246,224));
    g.DrawString(line,-1,&bold,RectF(box.X+1,box.Y+1,box.Width,box.Height),&centred,&outline);
    g.DrawString(line,-1,&bold,box,&centred,&white);
}
}
