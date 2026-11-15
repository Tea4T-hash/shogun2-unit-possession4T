#include "../../../deck/glyphs181.h"
#include "../../../deck/help_header186.h"
#pragma once
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#ifdef S2FP_LOCAL_ASSETS
#include "../../../common/local_assets159.h"
#ifdef HUD80_QTE_ONLY
#include "../../../common/qte_sprites159.h"
#else
#include "../../../common/hud_sprites159.h"
#endif
#else
#ifdef HUD80_QTE_ONLY
#include "qte-sprites.generated.h"
#else
#include "sprites.generated.h"
#endif
#endif
#include "unit_art.generated.h"
#include "../../../common/image_fit184.h"
#include "../../../common/card_image217.h"
#ifdef HUD80_QTE_ONLY
#include "../qte-src/qte_core.h"
#else
#include "../../shogun2-melee-meter79-preview/src/qte_core.h"
#endif
#include "native_font.h"
#include "../../shogun2-possession-cleanup-v46/src/hud_layout108.h"
#include "hud_state89.h"
#include "../../../common/unit_activity215.h"
#include "../../shogun2-accuracy90-preview/src/ability_protocol141.h"
#include "../../shogun2-accuracy90-preview/src/ability_catalog141.h"
#include "accuracy_geometry90.h"
#include "../../shogun2-accuracy90-preview/src/command_protocol100.h"
#include "../src/ammunition_protocol83.h"
#include "../src/equipment_protocol81.h"
namespace hud80 {
using namespace Gdiplus;
struct View {
    const UnitArt* unit{};
    std::wstring native_unit_key,native_unit_title;
    std::shared_ptr<card217::Picture> native_crest;
    std::shared_ptr<card217::Picture> native_card217;
    const FactionArt* owner{};
    std::wstring faction=L"Faction unavailable",mon;
    unsigned men{},initial{},morale{},fatigue{};
    int veterancy=-1;
    bool ranged{},bow{},aiming{},routing{},following{},fixture{},boshin{};
    float bow_power{};
    bool melee_mode{};
    unsigned aim_pose{},weapon_phase{},kind{},equipment_error{};
    bool equipment_known{},has_ranged{},crouch_supported{},crouched{},pending{};
    bool hidden{},arrow_camera{};
    LauncherSight123 launcher_sight{};
    unsigned slot{},styles{};std::wstring ammunition;
    bool ammunition_known{};ammo83::Status ammunition_state{};
    float ammo{},reload{},speed{};unsigned ammo_capacity{};
    bool unit_activity_known{};unitactivity215::Activity unit_activity{};
    bool accuracy_known{};float cone_half_angle{},projection_y{};
    unsigned climb_mode{};
    bool controller_ui{},commands_known{};command100::Status commands{};
    bool abilities_known{};ability141::Status abilities{};
};
class Assets {
    std::vector<IStream*> streams;
    std::vector<std::unique_ptr<Bitmap>> atlases;
    mutable std::map<std::wstring,std::unique_ptr<Bitmap>> tiles;
    mutable std::map<std::wstring,std::unique_ptr<Bitmap>> activity_tiles,ink_tiles,rank_tiles;

    Bitmap* sharp_activity_tile(const wchar_t* key) const {
        constexpr int native_size = 22;
        constexpr int enlarged_size = native_size * 3;
        const auto* source = sprite(key);
        if (!source || source->logical_w != native_size || source->logical_h != native_size)
            return nullptr;
        const auto cached = activity_tiles.find(key);
        if (cached != activity_tiles.end()) return cached->second.get();

        auto enlarged = std::make_unique<Bitmap>(enlarged_size, enlarged_size, PixelFormat32bppPARGB);
        auto scale_tile = [&](const wchar_t* tile_key, int size) {
            Graphics scale(enlarged.get());
            scale.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            scale.SetPixelOffsetMode(PixelOffsetModeHalf);
            scale.SetCompositingMode(CompositingModeSourceCopy);
            ImageAttributes edges;
            edges.SetWrapMode(WrapModeTileFlipXY);
            auto* tile = logical_tile(tile_key);
            return tile && scale.DrawImage(tile, Rect(0, 0, enlarged_size, enlarged_size),
                0, 0, size, size, UnitPixel, &edges) == Ok;
        };
        if (!scale_tile(key, native_size)) return nullptr;

        constexpr int row_bytes = enlarged_size * 4;
        std::vector<BYTE> pixels(enlarged_size * row_bytes);
        const Rect area(0, 0, enlarged_size, enlarged_size);
        BitmapData locked{};
        if (enlarged->LockBits(&area, ImageLockModeRead, PixelFormat32bppPARGB, &locked) != Ok)
            return nullptr;
        auto* scan = static_cast<BYTE*>(locked.Scan0);
        for (int y = 0; y < enlarged_size; ++y)
            std::memcpy(pixels.data() + y * row_bytes, scan + std::ptrdiff_t(y) * locked.Stride, row_bytes);
        if (enlarged->UnlockBits(&locked) != Ok) return nullptr;

        // Replace the Hold button's face and retain its original bronze rim and alpha.
        constexpr int hold_size = 51;
        constexpr float center_point = enlarged_size * .5f;
        constexpr float rim_inner_edge = enlarged_size * (22.25f / hold_size);
        const auto* hold_key = unitactivity215::icon(unitactivity215::Activity::Holding);
        if (!scale_tile(hold_key, hold_size) ||
            enlarged->LockBits(&area, ImageLockModeRead | ImageLockModeWrite,
                PixelFormat32bppPARGB, &locked) != Ok) return nullptr;
        scan = static_cast<BYTE*>(locked.Scan0);
        for (int y = 1; y < enlarged_size - 1; ++y) {
            auto* row = scan + std::ptrdiff_t(y) * locked.Stride;
            for (int x = 1; x < enlarged_size - 1; ++x) {
                const float distance = std::hypot(x + .5f - center_point, y + .5f - center_point);
                const float coverage = std::clamp(rim_inner_edge - distance, 0.f, 1.f);
                const auto* center = pixels.data() + y * row_bytes + x * 4;
                // All five original faces are opaque inside the rim.
                if (coverage == 0.f || center[3] != 255) continue;
                auto* destination = row + x * 4;
                for (int channel = 0; channel < 3; ++channel) {
                    const int original = center[channel];
                    const int adjacent = (center[channel - 4] + center[channel + 4] +
                        center[channel - row_bytes] + center[channel + row_bytes] + 2) / 4;
                    const int detail = std::clamp(original - adjacent, -24, 24);
                    const int face = std::clamp(original + detail, 0, 255);
                    const float tinted = face * (destination[3] / 255.f);
                    destination[channel] = static_cast<BYTE>(std::lround(
                        destination[channel] + (tinted - destination[channel]) * coverage));
                }
            }
        }
        if (enlarged->UnlockBits(&locked) != Ok) return nullptr;
        return activity_tiles.emplace(key, std::move(enlarged)).first->second.get();
    }

public:
    bool load(HMODULE module){
        if(!native_fonts.load(module))return false;
#ifdef S2FP_LOCAL_ASSETS
        for(int i=0;i<atlas_count;++i){
            auto b=assets159::resource(module,101+i);if(!b.bytes||!b.width)return false;
            auto bitmap=std::make_unique<Bitmap>(b.width,b.height,b.width*4,PixelFormat32bppARGB,const_cast<BYTE*>(b.bytes));
            if(bitmap->GetLastStatus()!=Ok)return false;atlases.push_back(std::move(bitmap));
        }
#ifndef HUD80_QTE_ONLY
        for(unsigned id:{30u,31u}){
            auto b=assets159::resource(module,400+id);if(!b.bytes||!b.width)return false;
            auto bitmap=std::make_unique<Bitmap>(b.width,b.height,b.width*4,PixelFormat32bppARGB,const_cast<BYTE*>(b.bytes));
            if(bitmap->GetLastStatus()!=Ok)return false;tiles.emplace(L"native-ability141-"+std::to_wstring(id),std::move(bitmap));
        }
#endif
#else
        for(int i=0;i<atlas_count;++i){
            HRSRC r=FindResourceW(module,MAKEINTRESOURCEW(101+i),MAKEINTRESOURCEW(10));
            if(!r)return false;
            DWORD size=SizeofResource(module,r);void* bytes=LockResource(LoadResource(module,r));
            if(!bytes||!size)return false;
            HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,size);if(!memory)return false;
            void* copy=GlobalLock(memory);if(!copy){GlobalFree(memory);return false;}
            memcpy(copy,bytes,size);GlobalUnlock(memory);IStream* stream=nullptr;
            if(CreateStreamOnHGlobal(memory,TRUE,&stream)!=S_OK){GlobalFree(memory);return false;}
            streams.push_back(stream);auto bitmap=std::make_unique<Bitmap>(stream);
            if(bitmap->GetLastStatus()!=Ok)return false;
            atlases.push_back(std::move(bitmap));
        }
#ifndef HUD80_QTE_ONLY
        for(unsigned id:{30u,31u}){
            HRSRC r=FindResourceW(module,MAKEINTRESOURCEW(400+id),MAKEINTRESOURCEW(10));if(!r)return false;
            auto size=SizeofResource(module,r);auto bytes=LockResource(LoadResource(module,r));HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,size);if(!memory)return false;
            auto copy=GlobalLock(memory);if(!copy){GlobalFree(memory);return false;}memcpy(copy,bytes,size);GlobalUnlock(memory);
            IStream* stream=nullptr;if(CreateStreamOnHGlobal(memory,TRUE,&stream)!=S_OK){GlobalFree(memory);return false;}streams.push_back(stream);
            auto bitmap=std::make_unique<Bitmap>(stream);if(bitmap->GetLastStatus()!=Ok)return false;
            tiles.emplace(L"native-ability141-"+std::to_wstring(id),std::move(bitmap));
        }
#endif

#endif
                return true;
    }
    void clear(){rank_tiles.clear();ink_tiles.clear();activity_tiles.clear();tiles.clear();atlases.clear();for(auto stream:streams)stream->Release();streams.clear();native_fonts.clear();}
    ~Assets(){clear();}
    Bitmap* logical_tile(const wchar_t* key)const{
        const Sprite* s=sprite(key);if(!s)return nullptr;
        auto found=tiles.find(key);
        if(found==tiles.end()){
            auto bitmap=std::make_unique<Bitmap>(s->logical_w,s->logical_h,PixelFormat32bppPARGB);
            {Graphics tile_graphics(bitmap.get());tile_graphics.Clear(Color(0,0,0,0));
                tile_graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);
                tile_graphics.DrawImage(atlases[s->atlas].get(),Rect(s->offset_x,s->offset_y,s->w,s->h),s->x,s->y,s->w,s->h,UnitPixel);}
            found=tiles.emplace(key,std::move(bitmap)).first;
        }
        return found->second.get();
    }
    Bitmap* rank_mark(const wchar_t* key)const{
        const auto cached=rank_tiles.find(key);
        if(cached!=rank_tiles.end())return cached->second.get();
        auto* source=logical_tile(key);
        if(!source||source->GetWidth()!=14||source->GetHeight()!=38)return nullptr;
        Color backing;
        if(source->GetPixel(2,2,&backing)!=Ok||backing.GetA()!=153)return nullptr;
        auto mark=std::make_unique<Bitmap>(14,38,PixelFormat32bppARGB);
        {
            Graphics g(mark.get());
            g.Clear(Color(0,0,0,0));
        }
        // The stock badge composites the ink over a 60% opaque pale field.
        // Skip its one-pixel frame and undo that blend to recover the marks.
        for(int y=1;y<37;++y){
            for(int x=1;x<13;++x){
                Color pixel;
                if(source->GetPixel(x,y,&pixel)!=Ok||pixel.GetA()<=backing.GetA())continue;
                const float opacity=float(pixel.GetA()-backing.GetA())/float(255-backing.GetA());
                const float remaining=(1.f-opacity)*backing.GetA();
                const auto ink=[&](BYTE composite,BYTE paper){
                    const float channel=(pixel.GetA()*composite-remaining*paper)/(opacity*255.f);
                    return BYTE(std::clamp(std::lround(channel),0L,255L));
                };
                mark->SetPixel(x,y,Color(BYTE(std::lround(opacity*255.f)),
                    ink(pixel.GetR(),backing.GetR()),ink(pixel.GetG(),backing.GetG()),ink(pixel.GetB(),backing.GetB())));
            }
        }
        return rank_tiles.emplace(key,std::move(mark)).first->second.get();
    }
    Bitmap* ink_frame_tile(const wchar_t* key)const{
        const auto cached=ink_tiles.find(key);
        if(cached!=ink_tiles.end())return cached->second.get();
        auto* source=logical_tile(key);
        if(!source)return nullptr;
        const Rect area(0,0,source->GetWidth(),source->GetHeight());
        std::unique_ptr<Bitmap> tile(source->Clone(area,PixelFormat32bppARGB));
        BitmapData pixels{};
        if(!tile||tile->LockBits(&area,ImageLockModeRead|ImageLockModeWrite,PixelFormat32bppARGB,&pixels)!=Ok)
            return nullptr;
        auto* scan=static_cast<BYTE*>(pixels.Scan0);
        for(int y=0;y<area.Height;++y){
            auto* row=scan+std::ptrdiff_t(y)*pixels.Stride;
            for(int x=0;x<area.Width;++x){
                auto* pixel=row+x*4;
                const int peak=std::max({int(pixel[0]),int(pixel[1]),int(pixel[2])});
                if(peak<=62)continue;
                // The stock timber sprites include bright paper along their
                // inside edges. Compress those highlights for dark panels.
                constexpr int matte[]={30,36,38};
                const int blend=std::min(peak-62,58);
                for(int c=0;c<3;++c)pixel[c]=BYTE((pixel[c]*(58-blend)+matte[c]*blend)/58);
            }
        }
        if(tile->UnlockBits(&pixels)!=Ok)return nullptr;
        return ink_tiles.emplace(key,std::move(tile)).first->second.get();
    }
    RectF fitted(const wchar_t* key,RectF slot)const{
        float w=0,h=0;
        if(const auto* s=sprite(key)){w=float(s->logical_w);h=float(s->logical_h);}
        else {auto tile=tiles.find(key);if(tile!=tiles.end()){w=float(tile->second->GetWidth());h=float(tile->second->GetHeight());}}
        const auto r=imagefit184::within({slot.X,slot.Y,slot.Width,slot.Height},w,h);
        return RectF(r.x,r.y,r.w,r.h);
    }
    void image(Graphics& g,const wchar_t* key,RectF target,float opacity=1.f,bool isolated=false,bool ink_trim=false)const{
        const Sprite* s=sprite(key);if(!s){auto extra=tiles.find(key);if(extra!=tiles.end())g.DrawImage(extra->second.get(),target);return;}
        float sx=target.Width/s->logical_w,sy=target.Height/s->logical_h;
        RectF bounds(target.X+s->offset_x*sx,target.Y+s->offset_y*sy,s->w*sx,s->h*sy);
        ImageAttributes attributes;ColorMatrix alpha={1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,std::clamp(opacity,0.f,1.f),0, 0,0,0,0,1};
        attributes.SetColorMatrix(&alpha);
        if(isolated){auto bitmap=ink_trim?ink_frame_tile(key):logical_tile(key);if(!bitmap)return;
            attributes.SetWrapMode(WrapModeTileFlipXY);
            g.DrawImage(bitmap,target,0.f,0.f,float(s->logical_w),float(s->logical_h),UnitPixel,&attributes);
        }else g.DrawImage(atlases[s->atlas].get(),bounds,float(s->x),float(s->y),float(s->w),float(s->h),UnitPixel,&attributes);
    }
    void activity_image(Graphics& g, const wchar_t* key, RectF bounds) const {
        if (auto* bitmap = sharp_activity_tile(key)) {
            ImageAttributes edges;
            edges.SetWrapMode(WrapModeTileFlipXY);
            g.DrawImage(bitmap, bounds, 0.f, 0.f, float(bitmap->GetWidth()),
                float(bitmap->GetHeight()), UnitPixel, &edges);
        } else {
            image(g, key, bounds, 1.f, true);
        }
    }
    void tile(Graphics& g,const wchar_t* key,RectF bounds,float scale=1.f,WrapMode wrap=WrapModeTile,bool ink_trim=false)const{
        auto bitmap=ink_trim?ink_frame_tile(key):logical_tile(key);if(!bitmap)return;
        TextureBrush texture(bitmap,wrap);texture.ScaleTransform(scale,scale);
        texture.TranslateTransform(bounds.X,bounds.Y,MatrixOrderAppend);g.FillRectangle(&texture,bounds);
    }

    void tile_rail(Graphics& g,const wchar_t* key,RectF source,RectF target,bool horizontal)const{
        auto* bitmap=logical_tile(key);
        if(!bitmap||target.Width<=0.f||target.Height<=0.f)return;
        const float scale=horizontal?target.Height/source.Height:target.Width/source.Width;
        TextureBrush texture(bitmap,WrapModeTileFlipXY,source);
        texture.ScaleTransform(scale,scale);
        texture.TranslateTransform(target.X,target.Y,MatrixOrderAppend);
        g.FillRectangle(&texture,target);
    }

};
inline Color ink(){return Color(255,38,31,24);}
inline Color red(){return Color(255,135,36,29);}
inline Color muted(){return Color(255,91,79,61);}
inline void text(Graphics& g,const std::wstring& value,float x,float y,float w,float h,float size,Color color=ink(),bool bold=false){
    (void)bold;native_fonts.paint(g,value,RectF(x,y,w,h),size,color);
}
inline void round_rect(GraphicsPath& path,RectF r,float radius){float d=radius*2;
    path.AddArc(r.X,r.Y,d,d,180,90);path.AddArc(r.GetRight()-d,r.Y,d,d,270,90);
    path.AddArc(r.GetRight()-d,r.GetBottom()-d,d,d,0,90);path.AddArc(r.X,r.GetBottom()-d,d,d,90,90);path.CloseFigure();}
inline void key_image(Graphics& g,const wchar_t* label,RectF r){
    if(deck181::draw_key(g,label,r))return;label=deck181::key_label(label);
    GraphicsPath shape;round_rect(shape,r,3);SolidBrush paper(Color(255,231,226,205));Pen rim(Color(255,95,80,57),1.1f);
    g.FillPath(&paper,&shape);g.DrawPath(&rim,&shape);Pen highlight(Color(255,255,251,233),1.f);
    g.DrawLine(&highlight,r.X+4,r.Y+3,r.GetRight()-4,r.Y+3);g.DrawLine(&rim,r.X+4,r.GetBottom()-3,r.GetRight()-4,r.GetBottom()-3);
    text(g,label,r.X+6,r.Y+2,r.Width-11,r.Height-4,14,ink());
}
inline void mouse_image(Graphics& g,RectF r,bool left,bool active=true){
    if(deck181::controller_present()){key_image(g,left?L"RT":L"LT",r);return;}
    GraphicsPath shape;round_rect(shape,r,r.Width*.43f);SolidBrush paper(Color(255,228,223,201)),pressed(active?red():muted());
    g.FillPath(&paper,&shape);GraphicsState clip=g.Save();g.SetClip(&shape,CombineModeIntersect);
    g.FillRectangle(&pressed,left?r.X:r.X+r.Width*.5f,r.Y,r.Width*.5f,r.Height*.49f);g.Restore(clip);
    Pen line(Color(255,61,50,34),1.1f);g.DrawPath(&line,&shape);
    g.DrawLine(&line,r.X+r.Width*.5f,r.Y+2,r.X+r.Width*.5f,r.Y+r.Height*.49f);
    g.DrawLine(&line,r.X+1,r.Y+r.Height*.49f,r.GetRight()-1,r.Y+r.Height*.49f);
    SolidBrush wheel(Color(255,220,209,170));g.FillRectangle(&wheel,r.X+r.Width*.5f-1.5f,r.Y+r.Height*.14f,3.f,r.Height*.18f);
}
enum class PanelFinish {Paper,Ink};
inline constexpr float panel_rail=18.f;
inline RectF panel_bounds(RectF r,float device){
    const auto snap=[=](float value){return std::round(value*device)/device;};
    const float left=snap(r.X),top=snap(r.Y);
    return RectF(left,top,snap(r.GetRight())-left,snap(r.GetBottom())-top);
}
inline void panel(Graphics& g,const Assets& a,RectF r,PanelFinish finish=PanelFinish::Paper){
    // Keep every nine-slice join on one device-pixel grid. Corner and edge
    // thickness must share the same scale instead of rounding independently.
    Matrix transform;g.GetTransform(&transform);float elements[6];transform.GetElements(elements);
    float device=std::hypot(elements[0],elements[1]);if(!std::isfinite(device)||device<=0)return;
    auto snap=[&](float value){return std::round(value*device)/device;};
    r=panel_bounds(r,device);
    const bool dark=finish==PanelFinish::Ink;
    // Extend the ink backing beneath the inside edge of the frame.
    // This bridges the corner and rail joins at fractional HUD scales.
    const float c=std::min(snap(panel_rail),std::min(r.Width,r.Height)*.5f),edge_scale=c/36.f,inset=snap(dark?3.f:8.f);
    RectF inside(r.X+inset,r.Y+inset,r.Width-2*inset,r.Height-2*inset);
    SolidBrush paper(dark?Color(255,38,36,30):Color(248,225,219,195));
    g.FillRectangle(&paper,inside);
    if(!dark)a.tile(g,L"ui/frames/data__subpanel_paper_fill.png",inside);
    // Mirror only across the narrow axis: transparent outer padding must not
    // wrap back into the opaque inner edge and produce a detached hairline.
    a.tile(g,L"ui/frames/data__big_panel_top.png",RectF(r.X+c,r.Y,r.Width-2*c,c),edge_scale,WrapModeTileFlipY,dark);
    a.tile(g,L"ui/frames/data__big_panel_bottom.png",RectF(r.X+c,r.GetBottom()-c,r.Width-2*c,c),edge_scale,WrapModeTileFlipY,dark);
    a.tile(g,L"ui/frames/data__big_panel_left.png",RectF(r.X,r.Y+c,c,r.Height-2*c),edge_scale,WrapModeTileFlipX,dark);
    a.tile(g,L"ui/frames/data__big_panel_right.png",RectF(r.GetRight()-c,r.Y+c,c,r.Height-2*c),edge_scale,WrapModeTileFlipX,dark);
    // Isolate native logical sprites before bicubic scaling; atlas neighbours
    // cannot bleed through corner cutaways. Original offsets are preserved.
    a.image(g,L"ui/frames/data__big_panel_tl.png",RectF(r.X,r.Y,c,c),1.f,true,dark);
    a.image(g,L"ui/frames/data__big_panel_tr.png",RectF(r.GetRight()-c,r.Y,c,c),1.f,true,dark);
    a.image(g,L"ui/frames/data__big_panel_bl.png",RectF(r.X,r.GetBottom()-c,c,c),1.f,true,dark);
    a.image(g,L"ui/frames/data__big_panel_br.png",RectF(r.GetRight()-c,r.GetBottom()-c,c,c),1.f,true,dark);
}

inline const wchar_t* morale_name(unsigned n){static const wchar_t* names[]={L"Impetuous",L"Eager",L"Confident",L"Steady",L"Shaken",L"Wavering",L"Broken",L"Shattered"};return n<8?names[n]:L"Unavailable";}
inline const wchar_t* fatigue_name(unsigned n){static const wchar_t* names[]={L"Fresh",L"Active",L"Winded",L"Tired",L"Very tired",L"Exhausted"};return n<6?names[n]:L"Unavailable";}
inline std::wstring class_name(const UnitArt* u){
    if(!u)return {};
    std::wstring s=u->kind;
    std::replace(s.begin(),s.end(),L'_',L' ');if(!s.empty())s[0]=wchar_t(towupper(s[0]));return s;
}
inline std::wstring display_name169(const View& v){
    if(!v.native_unit_title.empty())return v.native_unit_title;
    if(v.unit)return v.unit->name;
    if(v.native_unit_key.empty())return L"Unit identity unavailable";
    std::wstring name=v.native_unit_key;
    std::replace(name.begin(),name.end(),L'_',L' ');
    if(name.front()>=L'a'&&name.front()<=L'z')name.front()-=L'a'-L'A';
    return name;
}
#include "../../shogun2-possession-cleanup-v46/src/qte_meter108.h"

inline const wchar_t* weapon_name89(const View& v){
 if(v.kind==1)return L"Bow";
 if(v.kind==3&&v.ammunition_known&&v.ammunition_state.count){auto icon=v.ammunition_state.items[v.ammunition_state.current].icon;
  if(icon==ammo83::Mortar)return L"Mortar";if(icon==ammo83::Rocket)return L"Rocket";return L"Thrown";}
 return L"Ranged";
}
inline hudstate89::State weapon_state89(const View& v){
    unsigned icon=v.ammunition_known&&v.ammunition_state.count&&v.ammunition_state.current<v.ammunition_state.count?
        v.ammunition_state.items[v.ammunition_state.current].icon:0;
    bool known=v.ammunition_known&&v.ammunition_state.count&&v.ammunition_state.current<v.ammunition_state.count&&
        (v.ammunition_state.items[v.ammunition_state.current].flags&ammo83::Selected)!=0;
    return hudstate89::resolve({v.equipment_known,v.ranged&&v.slot==2,known,
        v.pending||v.ammunition_state.pending!=0,v.aiming,v.kind,v.weapon_phase,v.aim_pose,icon,
        v.equipment_error|v.ammunition_state.error,v.ammo,v.reload});
}
inline void drop_reticle89(Graphics& g,float x,float y,bool ready,bool open_center=false){
    Pen shadow(Color(215,30,26,20),3.f),stroke(ready?Color(245,243,232,202):Color(180,207,198,173),1.2f);
    for(unsigned i=open_center?1:0;i<4;++i){
        const float half=hudstate89::drop_width[i]*.5f*accuracy90::fixed_sight_width_scale125;
        const float top=y+hudstate89::drop_y[i];
        g.DrawLine(&shadow,x-half,top,x+half,top);g.DrawLine(&stroke,x-half,top,x+half,top);}
}
inline void bow_draw_arc120(Graphics& g,float x,float y,float strength,bool deck){
    if(!std::isfinite(strength)||strength<.35f||strength>1.f)return;
    constexpr float left=153.f,start=27.f,sweep=left-start;
    const float width=deck?112.f:88.f,height=deck?54.f:44.f;
    // Supersample this small vector surface, then composite at native HUD size.
    // This avoids changing the resolution or redraw rate of the whole HUD.
    constexpr float padding=6.f,samples=3.f;
    const float w=width+2*padding,h=height+2*padding;
    Bitmap meter(int(w*samples),int(h*samples),PixelFormat32bppPARGB);
    if(meter.GetLastStatus()!=Ok)return;
    Graphics detail(&meter);
    detail.Clear(Color(0,0,0,0));
    detail.SetSmoothingMode(SmoothingModeAntiAlias);
    detail.SetPixelOffsetMode(PixelOffsetModeHalf);
    detail.ScaleTransform(samples,samples);
    const RectF curve(padding,padding,width,height);
    const float core=deck?4.4f:3.5f;
    Pen shadow(Color(226,30,26,20),core+4.f),rim(Color(248,221,208,163),core+2.f),
        trough(Color(250,58,44,27),core),
        power(strength>=1.f?Color(255,169,181,84):Color(255,222,175,93),core);
    for(auto pen:{&shadow,&rim,&trough,&power})pen->SetLineCap(LineCapRound,LineCapRound,DashCapFlat);
    detail.DrawArc(&shadow,curve,start,sweep);
    detail.DrawArc(&rim,curve,start,sweep);
    detail.DrawArc(&trough,curve,start,sweep);
    detail.DrawArc(&power,curve,left-sweep*strength,sweep*strength);
    detail.Flush(FlushIntentionSync);
    g.DrawImage(&meter,RectF(x-w*.5f,y-h*.5f,w,h),0.f,0.f,w*samples,h*samples,UnitPixel);
}
inline float bow_dash_sight120(Graphics& g,float x,float y,float gap,bool ready){
    constexpr float inset[]={1.f,.70f,.41f,.16f};
    constexpr float taper[]={1.f,.77f,.55f,.33f};
    const float length=std::clamp(gap*.24f,7.f,14.f);
    const float step=std::clamp(7.f+gap*.09f,8.f,17.f);
    Pen shadow(Color(215,30,26,20),3.f),stroke(ready?Color(245,243,232,202):Color(180,207,198,173),1.2f);
    for(unsigned row=0;row<4;++row){
        const float offset=gap*inset[row],extent=offset+length*taper[row],top=y+step*row;
        for(auto pen:{&shadow,&stroke}){
            g.DrawLine(pen,x-extent,top,x-offset,top);
            g.DrawLine(pen,x+offset,top,x+extent,top);
        }
    }
    return y+3.f*step;
}
inline void launcher_reticle123(Graphics& g,float x,float y,LauncherSight123 sight,bool ready){
    Pen shadow(Color(215,30,26,20),3.5f),stroke(ready?Color(245,243,232,202):Color(180,207,198,173),1.35f);
    for(auto pen:{&shadow,&stroke}){
        auto mark=[&](float x1,float y1,float x2,float y2){
            const float width=accuracy90::fixed_sight_width_scale125;
            g.DrawLine(pen,x+x1*width,y+y1,x+x2*width,y+y2);
        };
        if(sight==LauncherSight123::Rocket){
            mark(-22,8,-8,-3);mark(8,-3,22,8);
            mark(-8,27,8,27);mark(-5,42,5,42);
        }else{
            for(float side:{-1.f,1.f}){
                mark(side*24,-25,side*16,-25);mark(side*16,-25,side*16,13);
                mark(side*28,-10,side*20,-10);mark(side*28,7,side*20,7);
            }
            mark(-12,28,12,28);mark(-8,44,8,44);
        }
    }
}
inline void firearm_reticle119(Graphics& g,float x,float y,float gap,bool ready){
    const float length=std::clamp(gap*.24f,7.f,14.f);
    Pen shadow(Color(215,30,26,20),3.f),stroke(ready?Color(245,243,232,202):Color(180,207,198,173),1.2f);
    for(auto pen:{&shadow,&stroke}){
        g.DrawLine(pen,x-gap-length,y,x-gap,y);
        g.DrawLine(pen,x+gap,y,x+gap+length,y);
        g.DrawLine(pen,x,y-gap-length,x,y-gap);
        g.DrawLine(pen,x,y+gap,x,y+gap+length);
    }
}
inline void center_dot(Graphics& g,float width,float height){
    const auto saved=g.Save();
    g.ResetTransform();
    const float x=width*.5f,y=height*.5f;
    SolidBrush edge(Color(220,30,26,20)),core(Color(245,243,232,202));
    g.FillEllipse(&edge,x-2.f,y-2.f,4.f,4.f);
    g.FillEllipse(&core,x-1.f,y-1.f,2.f,2.f);
    g.Restore(saved);
}
inline void card_picture(Graphics& g,const card217::Picture& picture,RectF bounds,float opacity=1.f){
    Bitmap bitmap(picture.w,picture.h,picture.w*4,PixelFormat32bppARGB,
        const_cast<BYTE*>(picture.pixels.data()));
    ImageAttributes attributes;
    ColorMatrix alpha={1,0,0,0,0, 0,1,0,0,0, 0,0,1,0,0, 0,0,0,opacity,0, 0,0,0,0,1};
    attributes.SetColorMatrix(&alpha);
    attributes.SetWrapMode(WrapModeTileFlipXY);
    g.DrawImage(&bitmap,bounds,0.f,0.f,float(picture.w),float(picture.h),UnitPixel,&attributes);
}

inline void card_fill(Graphics& g,RectF bounds,Color color){
    SolidBrush brush(color);
    g.FillRectangle(&brush,bounds);
}

inline void card_meter(Graphics& g,const Assets& a,RectF bounds,float fraction,bool vertical){
    const auto saved=g.Save();
    if(vertical){
        g.TranslateTransform(bounds.X,bounds.GetBottom());
        g.RotateTransform(-90.f);
        bounds=RectF(0,0,bounds.Height,bounds.Width);
    }
    a.image(g,L"ui/bars/data__unit_strength_bar_red.png",bounds,1.f,true);
    if(fraction>0.f){
        g.SetClip(RectF(bounds.X,bounds.Y,bounds.Width*fraction,bounds.Height),CombineModeIntersect);
        const auto* fill=vertical?L"ui/bars/data__unit_strength_bar_green.png":L"ui/bars/data__unit_strength_bar_yellow.png";
        a.image(g,fill,bounds,1.f,true);
    }
    g.Restore(saved);
}

inline constexpr float card_rail=8.f;

inline void card_trim(Graphics& g,const Assets& a,RectF frame,float rail){
    const float side_height=frame.Height-2*rail;
    // Sample the solid timber from each stock rail, excluding both the
    // transparent outer padding and the paper printed along the inner edge.
    a.tile_rail(g,L"ui/frames/data__big_panel_top.png",RectF(0,6,150,11),
        RectF(frame.X,frame.Y,frame.Width,rail),true);
    a.tile_rail(g,L"ui/frames/data__big_panel_bottom.png",RectF(0,20,150,11),
        RectF(frame.X,frame.GetBottom()-rail,frame.Width,rail),true);
    a.tile_rail(g,L"ui/frames/data__big_panel_left.png",RectF(14,0,14,50),
        RectF(frame.X,frame.Y+rail,rail,side_height),false);
    a.tile_rail(g,L"ui/frames/data__big_panel_right.png",RectF(8,0,14,50),
        RectF(frame.GetRight()-rail,frame.Y+rail,rail,side_height),false);
}

inline void draw_unit_card(Graphics& g,const Assets& a,const View& v,const hudlayout108::Layout& layout){
    const auto& identity=layout.soldier;
    constexpr float max_portrait_width=192.f;
    const float trim=std::max(1.f,std::round(card_rail*layout.zoom))/layout.zoom;
    const float x=identity.x,y=identity.y;
    const RectF slot(x+trim,y+trim,max_portrait_width,identity.h-2*trim);
    const auto* art=portrait_for(v.unit,v.owner);
    auto fit=a.fitted(art,slot);
    if(v.native_card217){
        const auto& p=*v.native_card217;
        const auto r=imagefit184::within({slot.X,slot.Y,slot.Width,slot.Height},float(p.w),float(p.h));
        fit=RectF(r.x,r.y,r.w,r.h);
    }
    const bool has_portrait=fit.Width>0.f&&fit.Height>0.f;
    const float portrait_width=has_portrait?fit.Width:slot.Height*128.f/236.f;
    const float portrait_height=has_portrait?fit.Height:slot.Height;
    const RectF portrait(slot.X,slot.GetBottom()-portrait_height,portrait_width,portrait_height);
    const bool full_height=portrait_height>=slot.Height-.25f;
    const bool title_on_art=portrait.Width>=54.f&&portrait.Height>=50.f;
    const bool rank_on_art=portrait.Width>=74.f&&portrait.Height>=100.f;
    const bool has_rank=v.veterancy>0&&v.veterancy<=9;
    const float join=portrait.GetRight();
    const float paper_left=join+(full_height?0.f:trim);
    const float panel_right=std::max(x+identity.w,paper_left+164.f+trim);
    const RectF paper(paper_left,y+trim,panel_right-paper_left-trim,slot.Height);
    const RectF frame(x,y,panel_right-x,identity.h);
    const float left=paper.X+22,right=paper.GetRight()-9,width=right-left;
    const Color ivory(255,246,239,223),gold(255,183,145,74);
    const auto shared_frame=panel_bounds(frame,layout.zoom);
    const auto portrait_frame=panel_bounds(RectF(x,portrait.Y-trim,portrait.Width+2*trim,portrait.Height+2*trim),layout.zoom);
    const auto stats_frame=panel_bounds(RectF(join,y,panel_right-join,identity.h),layout.zoom);
    const auto* paper_art=L"ui/frames/data__subpanel_paper_fill.png";
    // The native paper runs underneath the trim and the artwork so that
    // transparent custom portraits blend into the card's actual material.
    if(full_height){
        a.tile(g,paper_art,shared_frame);
    }else{
        a.tile(g,paper_art,portrait_frame);
        a.tile(g,paper_art,stats_frame);
    }

    // Loaded game/mod crest takes precedence over the stock art lookup.
    const RectF crest(left+width*.5f-102,paper.Y+paper.Height*.5f-102,204,204);
    auto saved=g.Save();
    g.SetClip(paper,CombineModeIntersect);
    if(v.native_crest){
        const auto& p=*v.native_crest;
        const auto fit=imagefit184::within({crest.X,crest.Y,crest.Width,crest.Height},float(p.w),float(p.h));
        card_picture(g,p,RectF(fit.x,fit.y,fit.w,fit.h),.19f);
    }else if(!v.mon.empty()){
        a.image(g,v.mon.c_str(),a.fitted(v.mon.c_str(),crest),.19f,true);
    }
    g.Restore(saved);

    saved=g.Save();
    g.SetClip(portrait,CombineModeIntersect);
    if(has_portrait){
        if(v.native_card217)card_picture(g,*v.native_card217,portrait);
        else a.image(g,art,portrait,1.f,true);
    }else{
        a.image(g,L"ui/panel_backgrounds/data__unit_card_background.png",portrait,1.f,true);
        native_fonts.paint(g,L"No portrait",RectF(portrait.X,portrait.Y+95,portrait.Width,26),14,muted(),true);
    }

    const float title_inset=std::min(14.f,portrait.Width*.1f);
    const float title_x=portrait.X+title_inset,title_w=portrait.Width-2*title_inset,title_size=19.f;
    const size_t title_rows=portrait.Height>=144.f?4:portrait.Height>=92.f?2:1;
    const auto lines=native_fonts.caption_lines(display_name169(v),title_w*layout.zoom,title_size*layout.zoom,title_rows);
    if(title_on_art&&!lines.empty()){
        const float line_step=std::max(24.f,14.f/layout.zoom);
        const float line_height=std::min(portrait.Height-8.f,std::max(26.f,14.f/layout.zoom));
        const float bottom=portrait.GetBottom();
        const float line_top=std::max(portrait.Y+3.f,bottom-8.f-line_height-line_step*float(lines.size()-1));
        const float fade_top=std::max(portrait.Y,std::min(bottom-100.f,line_top-24.f));
        const RectF caption(portrait.X,fade_top,portrait.Width,bottom-fade_top);
        LinearGradientBrush fade(caption,Color(0,30,30,25),Color(249,31,31,26),LinearGradientModeVertical);
        if(lines.size()>2){
            const Color tones[]={Color(0,30,30,25),Color(158,30,30,25),Color(249,31,31,26)};
            const REAL stops[]={0.f,(line_top-fade_top)/caption.Height,1.f};
            fade.SetInterpolationColors(tones,stops,3);
        }
        g.FillRectangle(&fade,caption);
        card_fill(g,RectF(title_x-title_inset*.5f,line_top,2,bottom-7.f-line_top),gold);
        for(size_t i=0;i<lines.size();++i)
            native_fonts.paint_caption(g,lines[i],RectF(title_x,line_top+line_step*float(i),title_w,line_height),title_size,ivory);
    }
    g.Restore(saved);
    if(v.melee_mode){
        const float size=std::min({32.f,portrait.Width*.72f,portrait.Height*.72f});
        const float inset=std::min(7.f,size*.22f);
        const auto* key=L"ui/icons/data__status_melee.png";
        const auto icon=a.fitted(key,RectF(portrait.X+inset,portrait.Y+inset,size,size));
        if(icon.Width>0.f&&icon.Height>0.f)a.activity_image(g,key,icon);
    }
    if(has_rank){
        wchar_t key[96];
        swprintf_s(key,L"ui/experience/data__experience_%d.png",v.veterancy);
        const RectF badge(rank_on_art?portrait.GetRight()-34.f:right-27.f,rank_on_art?portrait.Y+7.f:y+20.f,27.f,48.f);
        const auto rank=a.fitted(key,RectF(badge.X+3.f,badge.Y+3.f,badge.Width-6.f,badge.Height-6.f));
        if(auto* mark=a.rank_mark(key)){
            ImageAttributes edges;
            edges.SetWrapMode(WrapModeTileFlipXY);
            g.DrawImage(mark,rank,0.f,0.f,float(mark->GetWidth()),float(mark->GetHeight()),UnitPixel,&edges);
        }else{
            a.image(g,key,rank,1.f,true);
        }
    }
    if(full_height)card_fill(g,RectF(join,portrait.Y,1,portrait.Height),Color(255,126,107,74));

    const float men=v.initial?float(std::min(v.men,v.initial))/v.initial:0.f;
    card_meter(g,a,RectF(paper.X+8,paper.Y+10,10,paper.Height-20),men,true);
    const auto divider=[&](float position){
        Pen rule(Color(105,94,76,52),.75f);
        g.DrawLine(&rule,left,y+position,right,y+position);
    };
    const float label_width=width-(!rank_on_art&&has_rank?34.f:0.f);
    float type_begin=trim;
    if(!title_on_art){
        type_begin=24.f;
        const auto title=native_fonts.caption_lines(display_name169(v),label_width*layout.zoom,19.f*layout.zoom,2);
        for(const auto& line:title){
            native_fonts.paint_caption(g,line,RectF(left,y+type_begin,label_width,24.f),19.f,ink());
            type_begin+=24.f;
        }
    }
    const auto type=class_name(v.unit);
    const float count_top=type.empty()&&title_on_art?88.f:std::max(108.f,type_begin+(type.empty()?34.f:70.f));
    const float ammo_top=std::max(163.f,count_top+42.f);
    const bool shows_ammo=v.has_ranged||v.ammo_capacity;
    const float status_top=shows_ammo?std::max(226.f,ammo_top+38.f):std::max(163.f,count_top+44.f);
    const float count_begin=count_top-9.f,count_end=shows_ammo?ammo_top-5.f:status_top;
    if(!type.empty()){
        const RectF type_box(left+3.f,y+type_begin+6.f,label_width-6.f,count_begin-type_begin-12.f);
        native_fonts.paint_caption_block(g,type,type_box,38.f,ink());
        divider(count_begin);
    }
    if(shows_ammo)divider(count_end);
    const auto strength=std::to_wstring(v.men)+L" / "+std::to_wstring(v.initial);
    const RectF count(left,y+count_begin,width,count_end-count_begin);
    const FontFamily preferred(L"Segoe UI");
    const FontFamily* family=preferred.IsAvailable()?&preferred:FontFamily::GenericSansSerif();
    Font font(family,25.f,FontStyleBold,UnitPixel);
    SolidBrush count_ink(Color(255,33,35,34));
    StringFormat format(StringFormat::GenericTypographic());
    format.SetFormatFlags(format.GetFormatFlags()|StringFormatFlagsNoWrap);
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);
    RectF measured;
    if(font.GetLastStatus()!=Ok||g.MeasureString(strength.c_str(),-1,&font,PointF{},&format,&measured)!=Ok||measured.Width>count.Width)
        native_fonts.paint_caption(g,strength,count,21,ink(),true);
    else
        g.DrawString(strength.c_str(),-1,&font,count,&format,&count_ink);
    // Starting ammo belongs to the formation and is independent of a
    // temporarily unavailable personal weapon or an active ability.
    if(shows_ammo){
        constexpr float label_height=14.f,label_gap=4.f,meter_height=8.f;
        const float label_top=y+(count_end+status_top-label_height-label_gap-meter_height)*.5f;
        const float meter_top=label_top+label_height+label_gap;
        native_fonts.paint_caption(g,L"Ammo",RectF(left,label_top,width,label_height),13,muted(),true);
        card_meter(g,a,RectF(left,meter_top,width,meter_height),unitactivity215::ammunition(v.ammo,v.ammo_capacity),false);
    }
    divider(status_top);
    const float status_height=paper.GetBottom()-(y+status_top);
    const float status_center=paper.GetBottom()-status_height*.5f;
    const bool activity_known=v.routing||v.unit_activity_known;
    const float icon_size=std::min(36.f,status_height*.56f);
    if(activity_known){
        const auto* icon=unitactivity215::icon(v.routing?unitactivity215::Activity::Routing:v.unit_activity);
        const auto bounds=a.fitted(icon,RectF(right-icon_size,status_center-icon_size*.5f,icon_size,icon_size));
        if(bounds.Width>0&&bounds.Height>0)a.activity_image(g,icon,bounds);
    }
    const float text_right=right-(activity_known?icon_size+10.f:0.f);
    const float row_step=std::min(27.f,(status_height-10.f)*.5f);
    const RectF text_box(left,status_center-row_step,text_right-left,row_step-2.f);
    native_fonts.paint_caption_pair(g,morale_name(v.morale),fatigue_name(v.fatigue),text_box,row_step,24.f,
        v.morale>=5?red():ink(),v.fatigue>=4?red():ink());
    Pen status_rule(Color(105,94,76,52),.75f);
    g.DrawLine(&status_rule,left,status_center-1.f,text_right,status_center-1.f);
    // Keep a common frame for tall portraits. A wide custom portrait gets
    // its own lower rail so the timber can follow the full uncropped image.
    if(full_height){
        card_trim(g,a,shared_frame,trim);
    }else{
        card_trim(g,a,portrait_frame,trim);
        card_trim(g,a,stats_frame,trim);
    }
}


inline constexpr float ink_panel_width=220.f,ink_panel_right=18.f;
inline const Color ink_ivory(255,242,232,209),ink_gold(255,212,181,113);
inline void ink_label(Graphics& g,const wchar_t* label,RectF box,float size,Color color=ink_ivory){
    native_fonts.paint(g,label,box,size,color,false,true,false,12);
}

inline void ink_rule(Graphics& g,RectF panel,float top){
    Pen rule(Color(164,158,132,83),.8f);
    g.DrawLine(&rule,panel.X+17,panel.Y+top,panel.GetRight()-17,panel.Y+top);
}

inline void ink_action(Graphics& g,RectF panel,float y,const wchar_t* key,const wchar_t* label,
                       float key_width=32.f,Color color=ink_ivory){
    const float x=panel.X+17;
    key_image(g,key,RectF(x,panel.Y+y,key_width,24));
    const float text_x=x+key_width+11;
    ink_label(g,label,RectF(text_x,panel.Y+y,panel.GetRight()-17-text_x,24),15,color);
}

inline void ink_orders(Graphics& g,const Assets& a,const View& v,const hudlayout108::Layout& layout){
    const bool desktop=!v.controller_ui;
    const float height=desktop?(v.bow?383.f:357.f):233.f;
    const RectF box(layout.width-ink_panel_right-ink_panel_width,
        (layout.height-height)*.5f,ink_panel_width,height);
    panel(g,a,box,PanelFinish::Ink);

    const auto feedback=v.commands_known&&GetTickCount64()<v.commands.feedback_until
        ?static_cast<command100::Feedback>(v.commands.feedback):command100::Feedback::None;
    const wchar_t* heading=L"Unit orders";
    if(v.commands_known&&v.commands.placing)heading=L"Unit placement";
    else{
        const wchar_t* labels[]={L"Unit orders",L"Advance ordered",L"Formation ordered",L"Unit stopped",
            L"Holding here",L"Following you",L"Follow ended",L"Hold fire",L"Fire at will",L"Fire on command",
            L"No ground target",L"Command unavailable",L"Command unavailable"};
        static_assert(sizeof labels/sizeof *labels==unsigned(command100::Feedback::Unavailable)+1);
        if(unsigned(feedback)<sizeof labels/sizeof *labels)heading=labels[unsigned(feedback)];
    }
    ink_label(g,heading,RectF(box.X+17,box.Y+12,box.Width-34,24),16,ink_gold);

    const auto* skirmish=feedback==command100::Feedback::SkirmishOn?L"Skirmish on":
        feedback==command100::Feedback::SkirmishOff?L"Skirmish off":L"Toggle skirmish";
    if(desktop){
        const auto* fire=!v.commands_known?L"Fire orders":v.commands.fire_mode==0?L"Hold fire":
            v.commands.fire_mode==1?L"Fire at will":L"Fire on command";
        const auto tap=std::wstring(L"Tap \u00b7 ")+fire;
        const auto* hold=feedback==command100::Feedback::SkirmishOn||feedback==command100::Feedback::SkirmishOff
            ?skirmish:L"Hold for skirmish";
        // Our key layout (kit): F1-F4 orders; the AI Commander is not part of this build.
        ink_action(g,box,40,L"F1",L"Move / attack");
        ink_action(g,box,65,L"F2",L"Stop");
        ink_action(g,box,90,L"F3",L"Hold here",32,v.commands_known&&v.commands.move_mode==3?ink_gold:ink_ivory);
        ink_action(g,box,115,L"F4",L"Follow me",32,v.following?ink_gold:ink_ivory);
        ink_action(g,box,140,L"R",tap.c_str(),32,v.commands_known?ink_gold:ink_ivory);
        ink_action(g,box,165,L"R",hold);
    }else{
        ink_action(g,box,40,L"Q",L"Move / attack");
        ink_action(g,box,66,L"R1",L"Hold for wheel");
        ink_action(g,box,92,L"L5",feedback==command100::Feedback::SkirmishOn||feedback==command100::Feedback::SkirmishOff
            ?skirmish:L"Hold for skirmish");
    }
    const float split=desktop?208.f:122.f;
    ink_rule(g,box,split);
    ink_label(g,L"Possession",RectF(box.X+17,box.Y+split+6,box.Width-34,21),16,ink_gold);
    ink_action(g,box,split+28,L"3",L"Pause / resume");
    ink_action(g,box,split+52,L"4",L"Slow motion");
    if(desktop){
        ink_action(g,box,split+76,L"Tab",L"Switch perspective",42);
        ink_action(g,box,split+100,L"Caps Lock",L"Back to RTS",88);
        if(v.bow)ink_action(g,box,split+124,L"MMB",L"Hold to follow arrow",48);
    }else{
        ink_action(g,box,split+76,L"F10",L"Exit possession");
    }
}

inline void ink_ability(Graphics& g,const Assets& a,const View& v,const hudlayout108::Layout& layout){
    if(!v.abilities_known||!v.abilities.count||v.abilities.selected>=v.abilities.count)return;
    const auto& item=v.abilities.items[v.abilities.selected];
    const auto* art=ability141::art(item.id);
    if(!art)return;

    const RectF box(layout.width-ink_panel_right-ink_panel_width,layout.ability.y,ink_panel_width,105.f);
    panel(g,a,box,PanelFinish::Ink);
    const float x=box.X,y=box.Y;
    const RectF icon(x+17,y+16,40,40);
    a.image(g,art->icon,a.fitted(art->icon,icon),(item.flags&ability141::Available)?1.f:.45f);
    ink_label(g,art->label,RectF(x+67,y+13,box.Width-83,26),17);
    wchar_t status[64];
    if(v.abilities.error)wcscpy_s(status,item.flags&ability141::Targeted?L"Aim at an ally":L"Unavailable");
    else if(v.abilities.pending)wcscpy_s(status,L"Ordered");
    else if(item.cooldown>.05f)swprintf_s(status,L"Ready in %.0fs",std::ceil(item.cooldown));
    else wcscpy_s(status,item.flags&ability141::Active?L"Active":item.flags&ability141::Available?
        (item.flags&ability141::Targeted?L"Aim at an ally":L"Ready"):L"Unavailable");
    const bool ready=!v.abilities.error&&!v.abilities.pending&&item.cooldown<=.05f&&(item.flags&ability141::Available);
    ink_label(g,status,RectF(x+67,y+39,box.Width-83,20),14,
        v.abilities.error?Color(255,232,160,131):ready?Color(255,190,209,145):ink_ivory);
    ink_rule(g,box,64);
    ink_action(g,RectF(x,y,box.Width*.54f,box.Height),70,L"G",L"Next"); // our key layout
    ink_action(g,RectF(x+box.Width*.48f,y,box.Width*.52f,box.Height),70,L"F",
        item.flags&ability141::Active?L"End":L"Use");
}

inline void render(Graphics& g,const Assets& a,float width,float height,const View& v,const qte::Snapshot* qte=nullptr,bool offline=false,bool draw_wheel=true){
    native_fonts.boshin=v.boshin;
    g.SetSmoothingMode(SmoothingModeAntiAlias);g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHalf);g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    const float physical_width=width,physical_height=height;
    const hudlayout108::Layout layout(width,height);
    const float scale=layout.zoom;
    GraphicsState all=g.Save();g.ScaleTransform(scale,scale);width/=scale;height/=scale;
    if(v.hidden){
        // Native QTE meter includes the required mouse-button indicator.
        // The live QTE renderer is independent and never reads this HUD flag.
        if(qte&&qte->active)qte_meter(g,a,width*.5f,hudlayout108::qte_anchor_y(height),*qte);
    if(draw_wheel)deck181::draw_command_wheel183(g,a,width,height,v.mon.c_str(),v.commands_known?int(v.commands.fire_mode):-1,v.following,[&](const wchar_t* label,Gdiplus::RectF box,float size,Gdiplus::Color color){native_fonts.paint(g,label,box,size,color,true);},v.native_crest.get());
        g.Restore(all);return;
    }
    draw_unit_card(g,a,v,layout);
    const float middle=width*.5f;
    const auto& context=layout.context;
    if(v.climb_mode&&!(qte&&qte->active)){
        const float cx=context.x,cy=context.y;
        panel(g,a,RectF(cx,cy,context.w,context.h));
        {
            if(v.climb_mode<=2){key_image(g,L"E",RectF(cx+22,cy+20,30,26));text(g,L"Climb wall",cx+65,cy+21,242,26,17,ink(),true);}
            else if(v.climb_mode==5)text(g,L"Cannot climb here",cx+22,cy+23,290,26,17,ink(),true);
            else if(v.climb_mode==6)text(g,L"Approaching wall",cx+22,cy+23,290,26,17,ink(),true);
            else if(v.climb_mode==4)text(g,L"Climbing over",cx+22,cy+23,290,26,17,ink(),true);
            else {key_image(g,L"W",RectF(cx+27,cy+17,28,25));text(g,L"Ascend",cx+65,cy+18,107,25,16);
                key_image(g,L"S",RectF(cx+174,cy+17,28,25));text(g,L"Descend",cx+211,cy+18,100,25,16);
                text(g,L"Release to hold",cx+22,cy+48,290,23,14,muted());}
        }
    }
    const bool placing=v.commands_known&&v.commands.placing;
    ink_orders(g,a,v,layout);
    ink_ability(g,a,v,layout);
    auto reticle=weapon_state89(v);
    if(!v.arrow_camera&&!placing&&!v.climb_mode&&!(qte&&qte->active)){
        const bool bow=reticle.reticle==hudstate89::Reticle::Drop&&v.bow;
        const bool firearm=reticle.reticle==hudstate89::Reticle::Firearm;
        float radius=0;
        const bool spread_known=(bow||firearm)&&v.accuracy_known&&
            accuracy90::projected_radius(v.cone_half_angle,v.projection_y,physical_height,radius)&&radius<=physical_width;
        if(firearm&&spread_known)
            firearm_reticle119(g,middle,height*.5f,radius/scale,reticle.fire_prompt);
        if(reticle.reticle==hudstate89::Reticle::Drop){
            float bottom=height*.5f+hudstate89::drop_y[3];
            if(v.launcher_sight!=LauncherSight123::None)
                launcher_reticle123(g,middle,height*.5f,v.launcher_sight,reticle.fire_prompt);
            else if(bow&&spread_known)
                bottom=bow_dash_sight120(g,middle,height*.5f,radius/scale,reticle.fire_prompt);
            else
                drop_reticle89(g,middle,height*.5f,reticle.fire_prompt,bow);
            if(bow&&v.aiming&&v.weapon_phase==0)
                bow_draw_arc120(g,middle,bottom+4.f,v.bow_power,v.controller_ui);
        }else if(reticle.reticle==hudstate89::Reticle::None&&!v.pending&&
                 (v.equipment_known?v.slot==1:!v.ranged&&!v.has_ranged&&!v.ammo_capacity)){
            center_dot(g,physical_width,physical_height);
        }
    }
    if(draw_wheel)deck181::draw_command_wheel183(g,a,width,height,v.mon.c_str(),v.commands_known?int(v.commands.fire_mode):-1,v.following,[&](const wchar_t* label,Gdiplus::RectF box,float size,Gdiplus::Color color){native_fonts.paint(g,label,box,size,color,true);},v.native_crest.get());
    (void)offline;
    if(qte&&qte->active)qte_meter(g,a,middle,hudlayout108::qte_anchor_y(height),*qte);
    if(offline)text(g,L"OFFLINE HUD PREVIEW | fixture data",30,22,700,30,18,Color(255,239,232,211));
    g.Restore(all);
    if(deck181::controller_present()){auto saved=g.Save();float physical_width=width*scale;float zoom=deck186::header_scale(physical_width);g.TranslateTransform(deck186::header_left(physical_width),float(deck186::header_top));g.ScaleTransform(zoom,zoom);deck186::draw_header(g,[](unsigned slot,deck181::Glyph& glyph){return deck181::reader().glyph(slot,glyph);},[&](const wchar_t* label,Gdiplus::RectF bounds,float size){auto shadow=bounds;shadow.X+=1;shadow.Y+=1;native_fonts.paint(g,label,shadow,size,Gdiplus::Color(220,25,21,18));native_fonts.paint(g,label,bounds,size,Gdiplus::Color(255,242,233,207));});g.Restore(saved); }
    else if(deck181::pc_commander_legend78()){auto saved=g.Save();float physical_width=width*scale;float zoom=deck186::header_scale(physical_width);g.TranslateTransform(deck186::header_left(physical_width),float(deck186::header_top));g.ScaleTransform(zoom,zoom);deck186::draw_commander_pc([&](const wchar_t* label,Gdiplus::RectF bounds,float size){auto shadow=bounds;shadow.X+=1;shadow.Y+=1;native_fonts.paint(g,label,shadow,size,Gdiplus::Color(220,25,21,18));native_fonts.paint(g,label,bounds,size,Gdiplus::Color(255,242,233,207));});g.Restore(saved); }
}
}
