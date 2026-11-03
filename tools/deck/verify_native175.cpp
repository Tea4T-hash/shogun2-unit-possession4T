#include "../../native/deck/native_guard175.h"
#include <cassert>
#include <iostream>
int wmain(int argc,wchar_t** argv){
    if(argc!=3){std::cerr<<"Use separately owned stock and reviewed Bugfix images\n";return 1;}
    // Reserve the preferred image base so this exercises real relocation
    // normalization, rather than merely assuming the loader moved the image.
    void* reservation=VirtualAlloc(reinterpret_cast<void*>(0x10000000),0x1d88000,MEM_RESERVE,PAGE_NOACCESS);
    for(unsigned i=1;i<3;++i){
        // An isolated mapped image only: no game DllMain, entry, thread, hook,
        // audio, rendering, world or gameplay function is executed by this test.
        HMODULE image=LoadLibraryExW(argv[i],nullptr,DONT_RESOLVE_DLL_REFERENCES);
        if(!image)return 1;auto base=reinterpret_cast<uintptr_t>(image);assert(base!=0x10000000);
        assert(deck175::native_guards(base));
        auto target=reinterpret_cast<unsigned char*>(base+binding175::functions[0].rva);
        DWORD old{};assert(VirtualProtect(target,1,PAGE_EXECUTE_READWRITE,&old));
        auto saved=*target;*target^=1;assert(!deck175::native_guards(base));*target=saved;
        DWORD ignored{};assert(VirtualProtect(target,1,old,&ignored));
        assert(deck175::native_guards(base));FreeLibrary(image);
    }
    if(reservation)VirtualFree(reservation,0,MEM_RELEASE);
    std::cout<<"PASS RTS175 normalized native guards: stock/Bugfix, relocated mapping, corruption refusal and restoration; no game code executed\n";
}
