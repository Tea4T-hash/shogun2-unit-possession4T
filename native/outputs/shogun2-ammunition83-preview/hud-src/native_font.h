#ifdef S2FP_LOCAL_ASSETS
#include "../../../common/local_assets159.h"
#endif
#pragma once
#include <string_view>
#include <utility>
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
    float width(std::wstring_view value)const{float x=0;for(size_t i=0;i<value.size();++i)x+=advance(value[i],i+1<value.size()?value[i+1]:65535);return x;}
    std::pair<int,int> vertical_bounds(std::wstring_view value)const{
        int upper=9999,lower=-9999;
        for(auto c:value){
            const BYTE* d=dimensions(glyph(c));
            if(!d[2]||!d[3])continue;
            const int top=-int(static_cast<signed char>(d[0]));
            upper=std::min(upper,top);
            lower=std::max(lower,top+d[3]);
        }
        return {upper,lower};
    }
    void paint(Gdiplus::Graphics& g,std::wstring value,Gdiplus::RectF rect,Gdiplus::Color color,
        bool center=false,bool right=false,bool keep_full=false,float maximum_scale=1.f)const{
        if(value.empty())return;
        Gdiplus::Matrix world;g.GetTransform(&world);Gdiplus::PointF corners[2]={{rect.X,rect.Y},{rect.GetRight(),rect.GetBottom()}};world.TransformPoints(corners,2);
        float available=corners[1].X-corners[0].X;
        const float height=corners[1].Y-corners[0].Y;
        if(available<=0||height<=0)return;
        if(!keep_full&&width(value)>available){while(!value.empty()&&width(value+L"...")>available)value.pop_back();value+=L"...";}
        const auto [upper,lower]=vertical_bounds(value);
        if(lower<=upper)return;
        const float full_width=width(value);
        const float scale=keep_full?std::min({maximum_scale,available/std::max(1.f,full_width),height/float(lower-upper)}):1.f;
        float offset = 0.f;
        if (right || center) offset = std::max(0.f, available - full_width*scale) * (right ? 1.f : .5f);
        float x=float(std::lround(corners[0].X+offset));
        const float baseline=corners[0].Y+(height-float(lower-upper)*scale)*.5f-upper*scale;
        auto saved=g.Save();g.ResetTransform();g.SetClip(Gdiplus::RectF(corners[0].X,corners[0].Y,available,corners[1].Y-corners[0].Y),Gdiplus::CombineModeIntersect);
        g.SetInterpolationMode(scale==1.f?Gdiplus::InterpolationModeNearestNeighbor:Gdiplus::InterpolationModeHighQualityBilinear);g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
        Gdiplus::ColorMatrix matrix={{{color.GetR()/255.f,0,0,0,0},{0,color.GetG()/255.f,0,0,0},{0,0,color.GetB()/255.f,0,0},{0,0,0,color.GetA()/255.f,0},{0,0,0,0,1}}};
        Gdiplus::ImageAttributes tint;tint.SetColorMatrix(&matrix);
        for(size_t i=0;i<value.size();++i){unsigned code=glyph(value[i]);const BYTE* d=dimensions(code);auto bitmap=mask(code);
            if(bitmap){
                const int top=int(static_cast<signed char>(d[0]));
                if(scale==1.f)g.DrawImage(const_cast<Gdiplus::Bitmap*>(bitmap),Gdiplus::Rect(int(x),int(std::lround(baseline))-top,d[2],d[3]),0,0,d[2],d[3],Gdiplus::UnitPixel,&tint);
                else {
                    // GDI+ can drop the first row of a glyph drawn into a fractional destination.
                    // Round the edges independently to retain the full source bitmap.
                    const int left=int(std::lround(x));
                    const int pixel_top=int(std::lround(baseline-top*scale));
                    const int pixel_right=int(std::lround(x+d[2]*scale));
                    const int pixel_bottom=int(std::lround(baseline+(d[3]-top)*scale));
                    if(pixel_right>left&&pixel_bottom>pixel_top)
                        g.DrawImage(const_cast<Gdiplus::Bitmap*>(bitmap),
                            Gdiplus::Rect(left,pixel_top,pixel_right-left,pixel_bottom-pixel_top),0,0,d[2],d[3],Gdiplus::UnitPixel,&tint);
                }
            }
            x+=advance(value[i],i+1<value.size()?value[i+1]:65535)*scale;}
        g.Restore(saved);
    }
};
class NativeFonts {
    std::vector<std::unique_ptr<NativeFont>> sizes;
    const NativeFont* closest(float physical)const{
        const unsigned offset=boshin?10:0;
        const NativeFont* font=sizes[offset].get();
        for(unsigned i=offset;i<offset+10;++i){
            const auto* candidate=sizes[i].get();
            if(std::abs(float(candidate->nominal)-physical)<std::abs(float(font->nominal)-physical))font=candidate;
        }
        return font;
    }
public:
    bool boshin=false;
    bool load(HMODULE module){if(!sizes.empty())return true;unsigned values[]={12,14,16,18,20,22,24,28,32,38};
        for(unsigned i=0;i<20;++i){auto font=std::make_unique<NativeFont>();if(!font->load(module,201+i,values[i%10])){clear();return false;}sizes.push_back(std::move(font));}return true;}
    template<class Reader>bool load_local(Reader reader){
        if(!sizes.empty())return true;unsigned values[]={12,14,16,18,20,22,24,28,32,38};
        for(unsigned i=0;i<20;++i){auto blob=reader(201+i);auto font=std::make_unique<NativeFont>();if(!font->load_data(blob.bytes,blob.size,values[i%10])){clear();return false;}sizes.push_back(std::move(font));}return true;
    }
    // Fit at word boundaries. The card renderer can shrink a long single word
    // or the final line to fit its box while retaining the full text.
    std::vector<std::wstring> caption_lines(std::wstring_view name,float available,float physical_size,size_t max_lines=4)const{
        std::vector<std::wstring> lines;
        if(sizes.empty())return lines;
        const auto* font=closest(physical_size);
        constexpr std::wstring_view spaces=L" \t\r\n\u3000";
        while(lines.size()<max_lines){
            const auto first=name.find_first_not_of(spaces);
            if(first==std::wstring_view::npos)break;
            name.remove_prefix(first);
            if(lines.size()+1==max_lines || font->width(name)<=available){
                lines.emplace_back(name);
                break;
            }
            size_t fit=0;
            while(fit<name.size()&&font->width(name.substr(0,fit+1))<=available)++fit;
            auto split=name.find_last_of(spaces,fit);
            if(split==std::wstring_view::npos)split=name.find_first_of(spaces);
            if(split==std::wstring_view::npos){lines.emplace_back(name);break;}
            const auto end=name.substr(0,split).find_last_not_of(spaces);
            lines.emplace_back(name.substr(0,end+1));
            name.remove_prefix(split);
        }
        return lines;
    }

    void paint_caption_block(Gdiplus::Graphics& g,std::wstring_view caption,Gdiplus::RectF box,float size,
        Gdiplus::Color color)const{
        if(sizes.empty()||caption.empty()||box.Width<=0||box.Height<=0)return;
        Gdiplus::Matrix world;
        g.GetTransform(&world);
        float matrix[6];
        world.GetElements(matrix);
        const float zoom=std::hypot(matrix[0],matrix[1]);
        if(zoom<=0)return;
        const float width=box.Width*zoom;
        const float height=box.Height*zoom;
        const float gap=4.f*zoom;
        const unsigned first=boshin?10:0;
        size_t index=first+9;
        const auto* largest=closest(size*zoom);
        while(sizes[index].get()!=largest)--index;
        const NativeFont* best_font=nullptr;
        std::vector<std::wstring> best_lines;
        float best_row=0.f,best_height=0.f,best_scale=0.f;
        float best_size=0.f;
        for(int i=int(index);i>=int(first);--i){
            const auto* font=sizes[i].get();
            if(float(font->nominal)<=best_size)break;
            auto lines=caption_lines(caption,width,float(font->nominal),4);
            if(lines.empty())return;
            float widest=0.f,row=0.f;
            for(const auto& line:lines){
                const auto [top,bottom]=font->vertical_bounds(line);
                row=std::max(row,float(bottom-top));
                widest=std::max(widest,font->width(line));
            }
            if(row<=0.f)return;
            const float row_count=float(lines.size());
            const float block_height=(row+2.f)*row_count+gap*(row_count-1.f);
            const float scale=std::min({1.f,width/std::max(1.f,widest),height/block_height});
            const float effective_size=float(font->nominal)*scale;
            if(effective_size<=best_size)continue;
            best_size=effective_size;
            best_font=font;
            best_lines=std::move(lines);
            best_row=row;
            best_height=block_height;
            best_scale=scale;
        }
        if(!best_font)return;
        const float line_height=(best_row+2.f)*best_scale/zoom;
        const float line_step=line_height+gap*best_scale/zoom;
        float y=box.Y+(box.Height-best_height*best_scale/zoom)*.5f;
        for(const auto& line:best_lines){
            best_font->paint(g,line,Gdiplus::RectF(box.X,y,box.Width,line_height),color,true,false,true,best_scale);
            y+=line_step;
        }
    }

    void clear(){sizes.clear();}
    void paint_caption(Gdiplus::Graphics& g,const std::wstring& line,Gdiplus::RectF box,float size,Gdiplus::Color color,
        bool center=false,bool right=false)const{
        if(sizes.empty()||line.empty())return;
        Gdiplus::Matrix world;g.GetTransform(&world);float matrix[6];world.GetElements(matrix);
        closest(size*std::hypot(matrix[0],matrix[1]))->paint(g,line,box,color,center,right,true);
    }
    void paint_caption_pair(Gdiplus::Graphics& g,const std::wstring& top,const std::wstring& bottom,
        Gdiplus::RectF box,float row_step,float size,Gdiplus::Color top_ink,Gdiplus::Color bottom_ink)const{
        if(sizes.empty()||box.Width<=0.f||box.Height<=0.f)return;
        Gdiplus::Matrix world;
        g.GetTransform(&world);
        float matrix[6];
        world.GetElements(matrix);
        const float zoom=std::hypot(matrix[0],matrix[1]);
        const auto* font=closest(size*zoom);
        const float width=std::max(font->width(top),font->width(bottom));
        const auto [top_upper,top_lower]=font->vertical_bounds(top);
        const auto [bottom_upper,bottom_lower]=font->vertical_bounds(bottom);
        const float height=float(std::max(top_lower-top_upper,bottom_lower-bottom_upper));
        const float scale=std::min({1.f,box.Width*zoom/std::max(1.f,width),box.Height*zoom/std::max(1.f,height)});
        font->paint(g,top,box,top_ink,false,false,true,scale);
        box.Y+=row_step;
        font->paint(g,bottom,box,bottom_ink,false,false,true,scale);
    }
    void paint(Gdiplus::Graphics& g,const std::wstring& value,Gdiplus::RectF rect,float size,Gdiplus::Color color,bool center=false,bool fit=false,bool right=false,unsigned minimum=14)const{
        if(sizes.empty())return;Gdiplus::Matrix world;g.GetTransform(&world);float matrix[6];world.GetElements(matrix);float physical=size*std::hypot(matrix[0],matrix[1]);
        const unsigned offset=boshin?10:0;
        const NativeFont* font=closest(physical);
        if(fit){
            const float available=rect.Width*std::hypot(matrix[0],matrix[1]);
            const size_t chosen=static_cast<size_t>(std::find_if(sizes.begin()+offset,sizes.begin()+offset+10,
                [&](const auto& candidate){return candidate.get()==font;})-sizes.begin());
            for(size_t i=chosen;i>offset&&font->width(value)>available;){
                const auto* next=sizes[--i].get();
                if(next->nominal<minimum)break;
                font=next;
            }
        }
        font->paint(g,value,rect,color,center,right);
    }
};
inline NativeFonts native_fonts;
}
