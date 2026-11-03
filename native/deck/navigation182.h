#pragma once
#include <algorithm>
#include <cmath>
namespace deck182 {
inline float accumulate(float queued,float axis,float amount,float limit){
 if(!std::isfinite(queued)||!std::isfinite(axis)||!std::isfinite(amount)||!std::isfinite(limit)||amount<0||limit<=0||axis==0)return 0;
 if(queued*axis<0)queued=0;return std::clamp(queued+axis*amount,-limit,limit);
}
}
