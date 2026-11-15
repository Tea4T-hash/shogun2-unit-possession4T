#pragma once
// Read-only card resources from this player's game and enabled local packs.
// A missing/unsupported optional card never participates in gameplay admission.
#include "card_image217.h"
#include "../deck/proton_paths174.h"
#include "../../tools/workshop/ui_compat168.h"
#include <unordered_map>
#include <cwctype>
#include <tuple>
namespace card217 {
namespace fs=std::filesystem;
inline std::string narrow(const std::wstring& s){
 if(s.empty())return {};int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0,nullptr,nullptr);need(n>0,"Card name UTF16");std::string v(n,0);need(WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,s.data(),int(s.size()),v.data(),n,nullptr,nullptr)==n,"Card name UTF8");return v;
}
inline std::wstring wide(const std::string& s){
 if(s.empty())return {};int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),nullptr,0);need(n>0,"Pack UTF8");std::wstring v(n,0);need(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),int(s.size()),v.data(),n)==n,"Pack UTF8");return v;
}
// The Steam launcher commonly writes UTF-16LE without a BOM.
inline std::wstring selection_text(const Bytes& bytes){
 if(bytes.empty())return {};
 const bool le=bytes.size()>=2&&bytes[0]==255&&bytes[1]==254;
 const bool be=bytes.size()>=2&&bytes[0]==254&&bytes[1]==255;
 if(le||be||(bytes.size()>=2&&bytes[1]==0)){
  size_t start=le||be?2:0;
  need((bytes.size()-start)%2==0,"Selection UTF16 length");
  std::wstring text;
  text.reserve((bytes.size()-start)/2);
  for(size_t at=start;at<bytes.size();at+=2)
   text+=wchar_t(be?(unsigned(bytes[at])<<8)|bytes[at+1]:u16(bytes.data()+at));
  return text;
 }
 size_t start=bytes.size()>=3&&bytes[0]==239&&bytes[1]==187&&bytes[2]==191?3:0;
 return wide(std::string(bytes.begin()+start,bytes.end()));
}
inline std::string normal(std::string s){
 need(!s.empty()&&s.size()<=1024,"Card name length");for(auto& c:s){if(c=='\\')c='/';if(c>='A'&&c<='Z')c+=32;need(static_cast<unsigned char>(c)>=32,"Card name control");}
 need(s.front()!='/'&&s.back()!='/'&&s.find(':')==s.npos&&s.find("//")==s.npos,"Card absolute path");
 size_t p=0;while(p<s.size()){auto end=s.find('/',p);auto part=s.substr(p,end-p);need(part!="."&&part!="..","Card traversal");if(end==s.npos)break;p=end+1;}return s;
}
inline Bytes file(const fs::path& p,size_t limit){
 deck174::paths::read_file(p);auto n=fs::file_size(p);need(n<=limit,"Optional card input too large");std::ifstream in(p,std::ios::binary);need(bool(in),"Card input unavailable");Bytes b(size_t(n),0);need(b.empty()||bool(in.read(reinterpret_cast<char*>(b.data()),b.size())),"Card input truncated");return b;
}
struct Entry {fs::path pack;uint64_t offset{};unsigned size{};};
inline Bytes payload(const Entry& e,size_t limit){
 deck174::paths::read_file(e.pack);need(e.size&&e.size<=limit,"Card payload too large");auto total=fs::file_size(e.pack);need(e.offset<=total&&e.size<=total-e.offset,"Card payload changed");std::ifstream in(e.pack,std::ios::binary);in.seekg(std::streamoff(e.offset));Bytes b(e.size);need(bool(in.read(reinterpret_cast<char*>(b.data()),b.size())),"Card payload missing");return b;
}
inline bool resource(const std::string& n){
 return n.rfind("ui/units/info/",0)==0||n.rfind("ui/units/icons/",0)==0||
        n.rfind("ui/units/icons_square/",0)==0||
        (n.rfind("ui/flags/",0)==0&&n.size()>11&&n.compare(n.size()-11,11,"/mon_64.tga")==0);
}
inline bool loc_file(const std::string& n){return n.rfind("text/",0)==0&&n.compare(n.size()-4,4,".loc")==0;}
inline std::map<std::string,Entry> index(const fs::path& pack){
 deck174::paths::read_file(pack);std::ifstream in(pack,std::ios::binary);unsigned char h[32]{};need(bool(in.read(reinterpret_cast<char*>(h),32)),"Pack header truncated");
 unsigned flags=u32(h+4),deps=u32(h+8),depbytes=u32(h+12),count=u32(h+16),bytes=u32(h+20);auto total=fs::file_size(pack);
 need((!memcmp(h,"PFH2",4)||!memcmp(h,"PFH3",4))&&!(flags&~0x4fu)&&(flags&15)<=4&&deps<=4096&&depbytes<=1048576&&count<=100000&&bytes<=16*1024*1024&&32ull+depbytes+bytes<=total,"Optional pack index unsupported");
 Bytes dependency(depbytes);need(dependency.empty()||bool(in.read(reinterpret_cast<char*>(dependency.data()),depbytes)),"Pack dependencies truncated");need(size_t(std::count(dependency.begin(),dependency.end(),0))==deps&&(dependency.empty()||dependency.back()==0),"Pack dependencies malformed");
 Bytes data(bytes);need(data.empty()||bool(in.read(reinterpret_cast<char*>(data.data()),bytes)),"Pack index truncated");size_t p=0;uint64_t offset=32ull+depbytes+bytes;std::set<std::string> seen;std::map<std::string,Entry> found;
 for(unsigned i=0;i<count;++i){unsigned fixed=flags&0x40?12:4;need(p+fixed<=data.size(),"Pack entry truncated");unsigned size=u32(data.data()+p);p+=fixed;auto begin=p;while(p<data.size()&&data[p])++p;need(p<data.size(),"Pack name unterminated");auto n=normal(std::string(reinterpret_cast<char*>(data.data()+begin),p-begin));++p;
  need(seen.insert(n).second&&offset<=total&&size<=total-offset,"Pack entry duplicate/outside file");if(resource(n)||loc_file(n)||n.rfind("db/uniforms_tables/",0)==0||n.rfind("db/factions_tables/",0)==0)found.emplace(n,Entry{pack,offset,size});offset+=size;
 }need(p==data.size(),"Pack trailing index bytes");return found;
}
struct Uniform {std::wstring name,faction,card,unit;};
inline std::wstring key_case(std::wstring s){
 for(auto& c:s)if(c>=L'A'&&c<=L'Z')c+=L'a'-L'A';
 return s;
}
using UniformKey=std::tuple<std::wstring,std::wstring,std::wstring>;
// Schema3 is the native SHOGUN2 uniforms table: four UTF16 fields and four
// optional role/imposter strings. Read identity fields, never infer filenames.
inline std::vector<Uniform> uniforms(const Bytes& b){
 size_t p=0;auto integer=[&](){need(p+4<=b.size(),"DB integer truncated");auto n=u32(b.data()+p);p+=4;return n;};
 auto string=[&](){need(p+2<=b.size(),"DB string truncated");unsigned n=u16(b.data()+p);p+=2;need(n<=2048&&size_t(n)*2<=b.size()-p,"DB string bound");std::wstring s(n,0);if(n)memcpy(s.data(),b.data()+p,size_t(n)*2);p+=size_t(n)*2;need(s.find(L'\0')==s.npos,"DB embedded zero");return s;};
 auto optional=[&](){need(p<b.size(),"DB optional truncated");auto present=b[p++];need(present<=1,"DB optional flag");return present?string():std::wstring{};};
 if(b.size()>=4&&u32(b.data())==0xfffcfefd){p=4;string();}
 need(integer()==0xfffefdfc&&integer()==3,"Unsupported uniforms schema");need(p<b.size()&&b[p++]==1,"DB record flag");unsigned count=integer();need(count<=20000,"DB row bound");std::vector<Uniform> out;out.reserve(count);
 for(unsigned i=0;i<count;++i){Uniform row;row.name=string();row.faction=optional();row.card=string();row.unit=string();for(unsigned j=0;j<4;++j)optional();out.push_back(std::move(row));}
 need(p==b.size(),"DB trailing data");return out;
}
class TextTable {
 const Bytes& bytes_;size_t pos_{};
public:
 explicit TextTable(const Bytes& bytes,size_t start=0):bytes_(bytes),pos_(start){}
 const unsigned char* take(size_t size){
  need(pos_<=bytes_.size()&&size<=bytes_.size()-pos_,"Card table truncated");
  const auto* result=bytes_.data()+pos_;pos_+=size;return result;
 }
 unsigned integer(){return u32(take(4));}
 unsigned flag(){unsigned value=*take(1);need(value<=1,"Card table flag");return value;}
 std::wstring string(){
  size_t count=u16(take(2));const auto* data=take(count*2);
  std::wstring value(count,L'\0');
  if(count)memcpy(value.data(),data,count*2);
  return value;
 }
 bool end()const{return pos_==bytes_.size();}
};
using TextRows=std::map<std::wstring,std::wstring>;
inline TextRows names(const Bytes& bytes){
 TextTable rows(bytes);need(!memcmp(rows.take(6),"\xff\xfeLOC\0",6)&&rows.integer()==1,"Unsupported card localization");
 const unsigned count=rows.integer();need(count<=200000,"Card localization count");
 TextRows result;
 for(unsigned i=0;i<count;++i){
  const auto key=rows.string();auto value=rows.string();rows.flag();
  if(key.rfind(L"units_on_screen_name_",0)!=0&&key.rfind(L"factions_screen_name_",0)!=0)continue;
  if(value.empty()||value.size()>180||std::any_of(value.begin(),value.end(),[](wchar_t c){return c<32;}))continue;
  result[key]=std::move(value);
 }
 need(rows.end(),"Card localization trailing data");return result;
}
// Shogun 2 factions schema v7. Read the declared flags_path so a DB key
// such as oda_hitachi can use a separately named icon directory.
inline TextRows faction_flags(const Bytes& bytes){
 TextTable rows(bytes);unsigned header=rows.integer();
 if(header==0xfffcfefd){rows.string();header=rows.integer();}
 need(header==0xfffefdfc&&rows.integer()==7&&rows.flag()==1,"Unsupported faction schema");
 const unsigned count=rows.integer();need(count<=4096,"Faction row bound");
 TextRows result;
 for(unsigned i=0;i<count;++i){
  auto key=rows.string();rows.take(8);
  for(unsigned field=2;field<=7;++field)rows.string();
  if(rows.flag())rows.string();
  for(unsigned field=9;field<=11;++field)rows.flag();
  rows.string();auto flag=rows.string();
  for(unsigned field=14;field<=15;++field)if(rows.flag())rows.string();
  rows.take(18*4);rows.string();
  if(rows.flag())rows.string();
  rows.take(3*4);
  if(rows.flag())rows.string();
  rows.string();rows.flag();rows.flag();
  for(unsigned field=43;field<=46;++field)rows.string();
  if(rows.flag())rows.string();
  rows.string();
  for(unsigned field=49;field<=50;++field)if(rows.flag())rows.string();
  rows.string();
  if(!key.empty()&&!flag.empty())result[std::move(key)]=std::move(flag);
 }
 need(rows.end(),"Faction table trailing data");return result;
}
class Catalog {
 fs::path game_;bool initialized_{};std::map<std::string,Entry> cards_;std::map<UniformKey,Uniform> uniforms_;TextRows names_,flags_;
 struct Cached {std::wstring key;std::shared_ptr<Picture> picture;};std::vector<Cached> cache_;size_t pixel_bytes_{};
 void add(const fs::path& p){try{auto entries=index(p);if(cards_.size()+entries.size()>50000)return;for(const auto& pair:entries){if(resource(pair.first))cards_[pair.first]=pair.second;
  else if(loc_file(pair.first)||pair.first.rfind("db/factions_tables/",0)==0){
  try{
   const bool loc=loc_file(pair.first);auto bytes=payload(pair.second,16*1024*1024);
   auto rows=loc?names(bytes):faction_flags(bytes);auto& target=loc?names_:flags_;
   for(auto& row:rows)if(target.size()<25000||target.count(row.first))target[row.first]=std::move(row.second);
  }catch(...){}
 }
 else try{auto rows=uniforms(payload(pair.second,8*1024*1024));for(auto& row:rows){
   row.unit=key_case(std::move(row.unit));row.faction=key_case(std::move(row.faction));
   // Several shipped overhaul tables reuse the same uniform name for
   // separate unit records. Retain each declared unit/clan mapping.
   UniformKey key{row.unit,row.faction,key_case(row.name)};
   if(uniforms_.size()<20000||uniforms_.count(key))uniforms_[std::move(key)]=std::move(row);
  }}catch(...){} }}catch(...){} }
public:
 void configure(fs::path game){if(initialized_)return;initialized_=true;game_=std::move(game);
  try{
   std::vector<fs::path> folders{game_/L"data"},active;auto selected=game_/L"used_mods.txt";
   try{
    if(fs::is_regular_file(selected)){
     auto directives=workshop168::selection(selection_text(file(selected,1048576)));
     for(const auto& d:directives)if(d.key==L"add_working_directory")try{
      fs::path folder=d.value;
      if(folder.is_absolute()&&folders.size()<128){
       deck174::paths::read_directory(folder);
       if(fs::is_directory(folder))folders.push_back(folder.lexically_normal());
      }
     }catch(...){} // One missing or invalid library cannot hide other enabled packs.
     for(const auto& d:directives)if(d.key==L"mod"){
      fs::path name=d.value;
      if(name!=name.filename()||workshop168::lower(name.extension().wstring())!=L".pack")continue;
      for(const auto& folder:folders){
       auto p=folder/name;
       std::error_code ec;
       if(fs::is_regular_file(p,ec)){active.push_back(p);break;}
      }
     }
    }
   }catch(...){} // Keep base and automatic packs when the launcher list is unreadable.
   std::vector<std::pair<unsigned,fs::path>> automatic;
   for(const auto& folder:folders)try{unsigned count=0;for(const auto& q:fs::directory_iterator(folder)){if(++count>4096)break;if(!q.is_regular_file()||workshop168::lower(q.path().extension().wstring())!=L".pack")continue;
     try{deck174::paths::read_file(q.path());std::ifstream in(q.path(),std::ios::binary);unsigned char h[8]{};if(in.read(reinterpret_cast<char*>(h),8)&&(!memcmp(h,"PFH2",4)||!memcmp(h,"PFH3",4))){unsigned type=u32(h+4)&15;if(type<=2||type==4)automatic.emplace_back(type,q.path());}}catch(...){}
   }}catch(...){}
   std::sort(automatic.begin(),automatic.end(),[](const auto& a,const auto& b){return a.first!=b.first?a.first<b.first:workshop168::lower(a.second.wstring())<workshop168::lower(b.second.wstring());});
   for(auto& p:automatic)if(p.first<4)add(p.second);for(auto& p:active)add(p);for(auto& p:automatic)if(p.first==4)add(p.second);
  }catch(...){} // Optional artwork must never stop possession or the HUD.
 }
 std::wstring title(const wchar_t* prefix,const std::wstring& key)const{
  auto found=names_.find(prefix+key);return found==names_.end()?std::wstring{}:found->second;
 }
 std::shared_ptr<Picture> crest(const std::wstring& faction){
  try{
   auto found=flags_.find(faction);
   auto dir=normal(narrow(found==flags_.end()?faction:found->second));
   if(dir.rfind("data/",0)==0)dir.erase(0,5);
   if(dir.rfind("ui/flags/",0)!=0){
    if(dir.find('/')!=dir.npos)return {};
    dir="ui/flags/"+dir;
   }
   return card(L"",faction,wide(dir+"/mon_64.tga"),L"");
  }catch(...){return {};}
 }
 std::shared_ptr<Picture> card(const std::wstring& unit,const std::wstring& faction,const std::wstring& info,const std::wstring& icon){
  if(!initialized_||game_.empty())return {};
  auto key=unit+L"\n"+faction+L"\n"+info+L"\n"+icon;for(auto i=cache_.begin();i!=cache_.end();++i)if(i->key==key){auto hit=*i;cache_.erase(i);cache_.push_back(hit);return hit.picture;}
  std::shared_ptr<Picture> result;
  try{std::vector<std::wstring> aliases;
   std::set<std::wstring> faction_cards,default_cards;
   const auto unit_key=key_case(unit),faction_key=key_case(faction);
   for(auto entry=uniforms_.lower_bound({unit_key,L"",L""});
       entry!=uniforms_.end()&&std::get<0>(entry->first)==unit_key;++entry){
    const auto& row=entry->second;
    if(row.card.empty())continue;
    if(!faction_key.empty()&&row.faction==faction_key)faction_cards.insert(key_case(row.card));
    else if(row.faction.empty())default_cards.insert(key_case(row.card));
   }
   // Use a uniform's alias only when all matching rows agree on its artwork.
   // The live record's info and icon still provide fallbacks for ambiguous rows.
   if(faction_cards.size()==1)aliases.push_back(*faction_cards.begin());
   auto fallback=faction_cards.empty()&&default_cards.size()==1?*default_cards.begin():std::wstring{};
   auto load=[&](const std::wstring& alias,const char* directory){
    if(result||alias.empty())return;
    try{
     auto name=normal(narrow(alias));
     if(name.rfind("data/ui/units/",0)==0)name.erase(0,5);
     auto path=resource(name)?name:directory+name;
     if(!resource(name)&&name.find('/')!=name.npos)return;
     auto dot=path.find_last_of('.');
     const bool extension=dot!=path.npos&&dot>path.find_last_of('/');
     for(const char* suffix:extension?std::vector<const char*>{""}:std::vector<const char*>{".tga",".dds",".png"})try{
      auto n=path+suffix;
      auto loose=game_/L"data"/fs::u8path(n);
      auto item=cards_.find(n);
      Bytes b;
      if(fs::is_regular_file(loose))b=file(loose,16*1024*1024);
      else if(item!=cards_.end())b=payload(item->second,16*1024*1024);
      else continue;
      result=std::make_shared<Picture>(decode(b));
      return;
     }catch(...){}
    }catch(...){} // An invalid alias cannot mask a valid fallback.
   };
   for(const auto& alias:aliases)load(alias,"ui/units/info/");
   load(info,"ui/units/info/");
   load(fallback,"ui/units/info/");
   for(const auto& alias:aliases){
    load(alias,"ui/units/icons/");
    load(alias,"ui/units/icons_square/");
   }
   load(icon,"ui/units/icons/");
   load(icon,"ui/units/icons_square/");
   load(unit,"ui/units/info/");
   load(unit,"ui/units/icons/");
   load(unit,"ui/units/icons_square/");
   load(fallback,"ui/units/icons/");
   load(fallback,"ui/units/icons_square/");
  }catch(...){}
  size_t incoming=result?result->pixels.size():0;while(!cache_.empty()&&(cache_.size()>=64||pixel_bytes_+incoming>32*1024*1024)){if(cache_.front().picture)pixel_bytes_-=cache_.front().picture->pixels.size();cache_.erase(cache_.begin());}pixel_bytes_+=incoming;cache_.push_back({std::move(key),result});return result;
 }
};
} // card217
