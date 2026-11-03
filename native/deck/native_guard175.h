#pragma once
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include "bindings175_generated.h"

namespace deck175 {
inline bool native_guards(uintptr_t image){
    // Hash metadata only is generated from separately owned engine bytes and
    // verified native instruction exports. No engine bodies are embedded.
    BCRYPT_ALG_HANDLE algorithm{};
    if(BCryptOpenAlgorithmProvider(&algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0)return false;
    bool result=true;
    for(const auto& span:binding175::functions){
        if(!span.size||span.size>131072||span.rva>0x1d88000||span.size>0x1d88000-span.rva){result=false;break;}
        std::vector<unsigned char> bytes(span.size);SIZE_T got{};
        if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(image+span.rva),bytes.data(),bytes.size(),&got)||got!=bytes.size()){result=false;break;}
        for(unsigned i=0;i<span.reloc_count;++i){unsigned offset=binding175::relocations[span.reloc_start+i];
            if(offset+4>bytes.size()){result=false;break;}
            uint32_t value;memcpy(&value,bytes.data()+offset,4);value-=uint32_t(image-0x10000000u);memcpy(bytes.data()+offset,&value,4);
        }
        unsigned char digest[32]{};
        if(!result||BCryptHash(algorithm,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0){result=false;break;}
        static constexpr char digits[]="0123456789abcdef";char hex[65]{};
        for(unsigned i=0;i<32;++i){hex[i*2]=digits[digest[i]>>4];hex[i*2+1]=digits[digest[i]&15];}
        if(strcmp(hex,span.sha256)){result=false;break;}
    }
    BCryptCloseAlgorithmProvider(algorithm,0);return result;
}
}
