#ifdef S2FP_LOCAL_ASSETS
#include "../../../common/local_assets159.h"
#endif
#pragma once
// Native Creative Assembly Unicode Font masks, metrics and pair kerning.
// Format reference: Europa Barbarorum CUFReader/FormatConstants (2010-2012).
// No system-font fallback and no synthetic bolding.
namespace hud80 {
class NativeFont {
    const BYTE* data=nullptr;size_t length{};unsigned glyph_count{},bitmap_start{},bitmap_end{},kern_count{},kern_skip{};
    mutable std::map<unsigned,std::unique_ptr<Gdiplus::Bitmap>> masks;
    unsigned u16(size_t p)const{return p+2<=length?unsigned(data[p])|(unsigned(data[p+1])<<8):65535;}
    unsigned u32(size_t p)const{return u16(p)|(u16(p+2)<<16);}
    unsigned glyph(wchar_t c)const{unsigned code=u16(32+size_t(c)*2);return code<glyph_count?code:u16(32+L'?'*2);}
    const BYTE* dimensions(unsigned code)const{return data+0x20020+code*4;}
    float advance(unsigned character,unsigned next)const{
        // Shogun's footer range starts at Unicode33 ('!'). The glyph map
        // starts at Unicode1 -> glyph0; using glyph indices shifts every pair
        // and causes obvious overlap/gaps ("mm" incorrectly advances4 vs14).
        if(character>=kern_skip&&character<kern_skip+kern_count&&next>=kern_skip&&next<kern_skip+kern_count)
            return data[bitmap_end+4+(character-kern_skip)*kern_count+next-kern_skip];
        return dimensions(glyph(wchar_t(character)))[1];
    }
    const Gdiplus::Bitmap* mask(unsigned code)const{
        auto found=masks.find(code);if(found!=masks.end())return found->second.get();
        const BYTE* d=dimensions(code);unsigned w=d[2],h=d[3];if(!w||!h)return nullptr;
        unsigned offset=u32(0x20020+glyph_count*4+code*4);if(offset+w*h>bitmap_end-bitmap_start)return nullptr;
        auto bitmap=std::make_unique<Gdiplus::Bitmap>(w,h,PixelFormat32bppARGB);
        Gdiplus::BitmapData lock{};Gdiplus::Rect bounds(0,0,w,h);
        if(bitmap->LockBits(&bounds,Gdiplus::ImageLockModeWrite,PixelFormat32bppARGB,&lock)!=Gdiplus::Ok)return nullptr;
        for(unsigned y=0;y<h;++y){auto row=reinterpret_cast<unsigned*>(static_cast<BYTE*>(lock.Scan0)+y*lock.Stride);
            for(unsigned x=0;x<w;++x)row[x]=(unsigned(data[bitmap_start+offset+y*w+x])<<24)|0xffffff;}
        bitmap->UnlockBits(&lock);auto result=bitmap.get();masks.emplace(code,std::move(bitmap));return result;
    }
public:
    unsigned nominal{};
    bool load(HMODULE module,int id,unsigned size){
#ifdef S2FP_LOCAL_ASSETS
        auto blob=assets159::resource(module,id);data=blob.bytes;length=blob.size;
#else
        HRSRC r=FindResourceW(module,MAKEINTRESOURCEW(id),MAKEINTRESOURCEW(10));if(!r)return false;
        length=SizeofResource(module,r);data=static_cast<const BYTE*>(LockResource(LoadResource(module,r)));
#endif
        return load_data(data,length,size);
    }
    bool load_data(const BYTE* bytes,size_t bytes_count,unsigned size){
        data=bytes;length=bytes_count;
        if(!data||length<0x20020||memcmp(data,"CUF0",4))return false;
        nominal=size;glyph_count=u16(26);bitmap_start=0x20020+glyph_count*8;bitmap_end=bitmap_start+u32(28);
        if(!glyph_count||bitmap_end+4>length)return false;
        kern_count=u16(bitmap_end);kern_skip=u16(bitmap_end+2);
        if(kern_count+kern_skip>glyph_count||bitmap_end+4+size_t(kern_count)*kern_count!=length)return false;
        for(unsigned c=0;c<glyph_count;++c){const BYTE* d=dimensions(c);unsigned offset=u32(0x20020+glyph_count*4+c*4);
            if(offset+unsigned(d[2])*d[3]>bitmap_end-bitmap_start)return false;}
        return true;
    }
    float width(const std::wstring& value)const{float x=0;for(size_t i=0;i<value.size();++i)x+=advance(value[i],i+1<value.size()?value[i+1]:65535);return x;}
    void paint(Gdiplus::Graphics& g,std::wstring value,Gdiplus::RectF rect,Gdiplus::Color color,bool center=false)const{
        if(value.empty())return;
        Gdiplus::Matrix world;g.GetTransform(&world);Gdiplus::PointF corners[2]={{rect.X,rect.Y},{rect.GetRight(),rect.GetBottom()}};world.TransformPoints(corners,2);
        float available=corners[1].X-corners[0].X;
        if(width(value)>available){while(!value.empty()&&width(value+L"...")>available)value.pop_back();value+=L"...";}
        int upper=9999,lower=-9999;
        for(auto c:value){const BYTE* d=dimensions(glyph(c));if(!d[2]||!d[3])continue;int top=-int(static_cast<signed char>(d[0]));upper=std::min(upper,top);lower=std::max(lower,top+d[3]);}
        if(lower<upper)return;
        int x=int(std::lround(corners[0].X+(center?std::max(0.f,(available-width(value))*.5f):0))),baseline=int(std::lround(corners[0].Y+(corners[1].Y-corners[0].Y-float(lower-upper))*.5f-upper));
        auto saved=g.Save();g.ResetTransform();g.SetClip(Gdiplus::RectF(corners[0].X,corners[0].Y,available,corners[1].Y-corners[0].Y),Gdiplus::CombineModeIntersect);
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
        Gdiplus::ColorMatrix matrix={{{color.GetR()/255.f,0,0,0,0},{0,color.GetG()/255.f,0,0,0},{0,0,color.GetB()/255.f,0,0},{0,0,0,color.GetA()/255.f,0},{0,0,0,0,1}}};
        Gdiplus::ImageAttributes tint;tint.SetColorMatrix(&matrix);
        for(size_t i=0;i<value.size();++i){unsigned code=glyph(value[i]);const BYTE* d=dimensions(code);auto bitmap=mask(code);
            if(bitmap)g.DrawImage(const_cast<Gdiplus::Bitmap*>(bitmap),Gdiplus::Rect(x,baseline-int(static_cast<signed char>(d[0])),d[2],d[3]),0,0,d[2],d[3],Gdiplus::UnitPixel,&tint);
            x+=int(advance(value[i],i+1<value.size()?value[i+1]:65535));}
        g.Restore(saved);
    }
};
class NativeFonts {
    std::vector<std::unique_ptr<NativeFont>> sizes;
public:
    bool boshin=false;
    bool load(HMODULE module){if(!sizes.empty())return true;unsigned values[]={12,14,16,18,20,22,24,28,32,38};
        for(unsigned i=0;i<20;++i){auto font=std::make_unique<NativeFont>();if(!font->load(module,201+i,values[i%10])){clear();return false;}sizes.push_back(std::move(font));}return true;}
    template<class Reader>bool load_local(Reader reader){
        if(!sizes.empty())return true;unsigned values[]={12,14,16,18,20,22,24,28,32,38};
        for(unsigned i=0;i<20;++i){auto blob=reader(201+i);auto font=std::make_unique<NativeFont>();if(!font->load_data(blob.bytes,blob.size,values[i%10])){clear();return false;}sizes.push_back(std::move(font));}return true;
    }
    void clear(){sizes.clear();}
    void paint(Gdiplus::Graphics& g,const std::wstring& value,Gdiplus::RectF rect,float size,Gdiplus::Color color,bool center=false)const{
        if(sizes.empty())return;Gdiplus::Matrix world;g.GetTransform(&world);float matrix[6];world.GetElements(matrix);float physical=size*std::hypot(matrix[0],matrix[1]);
        unsigned offset=boshin?10:0;const NativeFont* font=sizes[offset].get();for(unsigned i=offset;i<offset+10;++i){const auto& candidate=sizes[i];if(std::abs(float(candidate->nominal)-physical)<std::abs(float(font->nominal)-physical))font=candidate.get();}
        font->paint(g,value,rect,color,center);
    }
};
inline NativeFonts native_fonts;
}
