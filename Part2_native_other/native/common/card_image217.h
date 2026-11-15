#pragma once
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#define BCDEC_STATIC
#define BCDEC_IMPLEMENTATION
#include "../vendor/bcdec/bcdec.h"
#undef BCDEC_IMPLEMENTATION
#undef BCDEC_STATIC
namespace card217 {
using Bytes=std::vector<unsigned char>;
inline void need(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
inline unsigned u16(const unsigned char* p){return unsigned(p[0])|(unsigned(p[1])<<8);}
inline unsigned u32(const unsigned char* p){unsigned n;memcpy(&n,p,4);return n;}
struct Picture {unsigned w{},h{};Bytes pixels;};
inline Picture tga(const Bytes& b){
    need(b.size()>=18,"Truncated TGA");unsigned cmap=b[1],type=b[2],start=u16(b.data()+3),colors=u16(b.data()+5),palette_bits=b[7],w=u16(b.data()+12),h=u16(b.data()+14),bits=b[16],desc=b[17];
    need(w&&h&&w<=2048&&h<=2048&&(type==1||type==2||type==3||type==9||type==10||type==11),"Unsupported TGA image");size_t p=18+b[0];need(p<=b.size(),"Truncated TGA identifier");
    auto color=[&](unsigned depth){need((depth==8||depth==16||depth==24||depth==32)&&p+depth/8<=b.size(),"Invalid TGA pixel");unsigned c=0;if(depth==8){c=0xff000000u|unsigned(b[p])*0x10101u;}else if(depth==16){unsigned x=u16(b.data()+p);c=0xff000000u|((x&31)*255/31)|(((x>>5)&31)*255/31<<8)|(((x>>10)&31)*255/31<<16);}else{c=unsigned(b[p])|(unsigned(b[p+1])<<8)|(unsigned(b[p+2])<<16)|(unsigned(depth==32?b[p+3]:255)<<24);}p+=depth/8;return c;};
    std::vector<unsigned> palette;if(cmap){need(cmap==1&&colors&&colors<=65536,"Invalid TGA palette");for(unsigned i=0;i<colors;++i)palette.push_back(color(palette_bits));}
    Picture out{w,h,Bytes(size_t(w)*h*4)};size_t done=0;
    auto pixel=[&](){if(type==1||type==9){need((bits==8||bits==16)&&p+bits/8<=b.size(),"Invalid TGA index");unsigned n=bits==8?b[p]:u16(b.data()+p);p+=bits/8;need(n>=start&&n-start<palette.size(),"TGA index outside palette");return palette[n-start];}return color(bits);};
    while(done<size_t(w)*h){unsigned n=1;bool repeat=false;if(type>=9){need(p<b.size(),"Truncated TGA RLE");unsigned packet=b[p++];n=(packet&127)+1;repeat=(packet&128)!=0;}need(n<=size_t(w)*h-done,"TGA RLE exceeds image");unsigned c=repeat?pixel():0;
        for(unsigned i=0;i<n;++i){if(!repeat)c=pixel();unsigned x=unsigned(done%w),y=unsigned(done/w);if(desc&16)x=w-1-x;if(!(desc&32))y=h-1-y;memcpy(out.pixels.data()+4*(size_t(y)*w+x),&c,4);++done;}}
    return out;
}

inline Picture dds(const Bytes& b){
 need(b.size()>=128&&!memcmp(b.data(),"DDS ",4)&&u32(b.data()+4)==124&&u32(b.data()+76)==32,"Invalid DDS card");
 unsigned h=u32(b.data()+12),w=u32(b.data()+16),code=u32(b.data()+84),bits=u32(b.data()+88),mode=0,block=0,offset=128;
 need(w&&h&&w<=2048&&h<=2048&&u32(b.data()+28)<=16&&!(u32(b.data()+112)&65024u),"DDS card dimensions/type");
 if(code==827611204u){mode=1;block=8;} // DXT1
 else if(code==861165636u){mode=2;block=16;} // DXT3
 else if(code==894720068u){mode=3;block=16;} // DXT5
 else if(code==808540228u){
  need(b.size()>=148&&u32(b.data()+132)==3&&u32(b.data()+140)==1&&!(u32(b.data()+136)&4),"DDS array/cube card refused");
  unsigned format=u32(b.data()+128);offset=148;
  if(format==71||format==72){mode=1;block=8;}else if(format==74||format==75){mode=2;block=16;}else if(format==77||format==78){mode=3;block=16;}else if(format==98||format==99){mode=7;block=16;}
  else if(format==28||format==29)mode=10;else if(format==87||format==91)mode=11;else need(false,"Unsupported DDS card format");
 }
 Picture out{w,h,Bytes(size_t(w)*h*4)};
 if(block){
  const size_t stride=(w+3)/4,rows=(h+3)/4;need(stride*rows*block<=b.size()-offset,"Truncated DDS card blocks");
  for(unsigned by=0;by<rows;++by)for(unsigned bx=0;bx<stride;++bx){
   alignas(16) unsigned char encoded[16]{},rgba[64]{};memcpy(encoded,b.data()+offset+(size_t(by)*stride+bx)*block,block);
   if(mode==1)bcdec_bc1(encoded,rgba,16);else if(mode==2)bcdec_bc2(encoded,rgba,16);else if(mode==3)bcdec_bc3(encoded,rgba,16);else bcdec_bc7(encoded,rgba,16);
   for(unsigned y=0;y<4&&by*4+y<h;++y)for(unsigned x=0;x<4&&bx*4+x<w;++x){auto src=rgba+(y*4+x)*4;auto dst=out.pixels.data()+4*(size_t(by*4+y)*w+bx*4+x);dst[0]=src[2];dst[1]=src[1];dst[2]=src[0];dst[3]=src[3];}
  }return out;
 }
 if(mode==10||mode==11){need(size_t(w)*h*4<=b.size()-offset,"Truncated DDS RGBA card");memcpy(out.pixels.data(),b.data()+offset,out.pixels.size());if(mode==10)for(size_t i=0;i<out.pixels.size();i+=4)std::swap(out.pixels[i],out.pixels[i+2]);return out;}
 need(!code&&(bits==24||bits==32)&&(u32(b.data()+80)&64),"Unsupported uncompressed DDS card");
 size_t pitch=(u32(b.data()+8)&8)?u32(b.data()+20):size_t(w)*(bits/8);need(pitch>=size_t(w)*(bits/8)&&pitch<=8192&&size_t(h)*pitch<=b.size()-offset,"Truncated DDS card rows");
 unsigned masks[4]{u32(b.data()+100),u32(b.data()+96),u32(b.data()+92),u32(b.data()+104)};
 for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x){unsigned value=0;memcpy(&value,b.data()+offset+size_t(y)*pitch+x*(bits/8),bits/8);auto dst=out.pixels.data()+4*(size_t(y)*w+x);
  for(unsigned c=0;c<4;++c){auto mask=masks[c];if(!mask){dst[c]=c==3?255:0;continue;}unsigned shift=0;while(!(mask&1)){mask>>=1;++shift;}need(mask<=255&&!(mask&(mask+1)),"Invalid DDS channel mask");dst[c]=static_cast<unsigned char>(((value>>shift)&mask)*255/mask);}
 }return out;
}
inline Picture png(const Bytes& b){
 const unsigned char signature[]{137,80,78,71,13,10,26,10};need(b.size()>=33&&!memcmp(b.data(),signature,8)&&!memcmp(b.data()+12,"IHDR",4),"Invalid PNG card");
 auto be=[](const unsigned char* p){return unsigned(p[0])*16777216u+unsigned(p[1])*65536u+unsigned(p[2])*256u+p[3];};unsigned w=be(b.data()+16),h=be(b.data()+20);need(w&&h&&w<=2048&&h<=2048,"PNG card too large");
 HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,b.size());need(memory!=nullptr,"Card allocation failed");auto p=GlobalLock(memory);if(!p){GlobalFree(memory);need(false,"Card lock failed");}memcpy(p,b.data(),b.size());GlobalUnlock(memory);
 IStream* raw=nullptr;if(CreateStreamOnHGlobal(memory,TRUE,&raw)!=S_OK){GlobalFree(memory);need(false,"Card stream unavailable");}
 struct Stream{IStream* value;~Stream(){value->Release();}} stream{raw};Gdiplus::Bitmap bitmap(raw);need(bitmap.GetLastStatus()==Gdiplus::Ok&&bitmap.GetWidth()==w&&bitmap.GetHeight()==h,"PNG card decode failed");
 Picture out{w,h,Bytes(size_t(w)*h*4)};Gdiplus::BitmapData pixels{};Gdiplus::Rect rect(0,0,w,h);need(bitmap.LockBits(&rect,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&pixels)==Gdiplus::Ok,"PNG pixels unavailable");
 for(unsigned y=0;y<h;++y)memcpy(out.pixels.data()+size_t(y)*w*4,static_cast<unsigned char*>(pixels.Scan0)+ptrdiff_t(y)*pixels.Stride,w*4);bitmap.UnlockBits(&pixels);return out;
}
inline Picture decode(const Bytes& b){
 need(!b.empty()&&b.size()<=16*1024*1024,"Card file exceeds bound");
 if(b.size()>=4&&!memcmp(b.data(),"DDS ",4))return dds(b);
 if(b.size()>=8&&b[0]==137&&b[1]==80)return png(b);
 return tga(b);
}
} // card217
