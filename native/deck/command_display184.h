#pragma once
#include "../outputs/shogun2-accuracy90-preview/src/command_protocol100.h"
namespace deck184 {
enum class Read {Missing,Contended,Stable};
// Display-only continuity across a seqlock collision. A stable reset/invalid
// sample immediately wins. Freshness remains the protocol's original 1000ms.
class CommandDisplay {
 command100::Status last_{};bool known_{};
public:
 template<class Reader,class Clock>bool read(Reader reader,Clock clock,unsigned pid,unsigned lease,unsigned actor,unsigned unit,command100::Status& out){
  command100::Status sample{};const auto result=reader(sample);const uint64_t now=clock();
  return resolve(result,sample,pid,lease,actor,unit,now,out);
 }
 bool resolve(Read result,const command100::Status& sample,unsigned pid,unsigned lease,unsigned actor,unsigned unit,uint64_t now,command100::Status& out){
  if(result==Read::Missing){known_=false;return false;}
  if(result==Read::Stable){known_=command100::valid(sample,pid,lease,actor,unit,now);if(known_)last_=sample;}
  if(!known_||!command100::valid(last_,pid,lease,actor,unit,now)){known_=false;return false;}
  out=last_;return true;
 }
};
}
