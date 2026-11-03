#pragma once
#include <windows.h>
#include <cwchar>
namespace deck177 {
enum class Owner {Frontend,Battle};
inline void name(Owner role,wchar_t (&text)[96]){
    swprintf_s(text,role==Owner::Frontend?L"Local\\S2UP-DeckFrontend177-%lu":L"Local\\S2UP-DeckBattle177-%lu",GetCurrentProcessId());
}
inline bool busy(Owner role){wchar_t text[96]{};name(role,text);HANDLE h=OpenMutexW(SYNCHRONIZE,FALSE,text);if(!h)return false;CloseHandle(h);return true;}
struct BattleRequest {
 HANDLE handle{};
 bool open(){if(handle)return true;wchar_t text[96]{};swprintf_s(text,L"Local\\S2UP-DeckBattleRequest181-%lu",GetCurrentProcessId());handle=CreateEventW(nullptr,TRUE,FALSE,text);return handle!=nullptr;}
 bool set(bool value){return open()&&(value?SetEvent(handle):ResetEvent(handle));}
 bool active(){return open()&&WaitForSingleObject(handle,0)==WAIT_OBJECT_0;}
 ~BattleRequest(){if(handle)CloseHandle(handle);}
};
inline BattleRequest& battle_request(){static BattleRequest request;return request;}
struct Lease {
    HANDLE handle{};
    Lease()=default;Lease(const Lease&)=delete;Lease& operator=(const Lease&)=delete;
    bool acquire(Owner role){
        if(handle)return true;wchar_t text[96]{};name(role,text);SetLastError(0);
        HANDLE h=CreateMutexW(nullptr,FALSE,text);DWORD error=GetLastError();
        if(!h)return false;if(error==ERROR_ALREADY_EXISTS){CloseHandle(h);return false;}
        handle=h;return true;
    }
    void release(){if(handle){CloseHandle(handle);handle=nullptr;}}
    ~Lease(){release();}
};
inline bool claim_frontend(Lease& lease){
 if(battle_request().active()||busy(Owner::Battle))return false;
 if(!lease.acquire(Owner::Frontend))return false;
 if(battle_request().active()||busy(Owner::Battle)){lease.release();return false;}return true;
}
inline bool claim_battle(Lease& lease){
 if(!battle_request().active()||busy(Owner::Frontend))return false;
 if(!lease.acquire(Owner::Battle))return false;
 if(busy(Owner::Frontend)){lease.release();return false;}return true;
}

}
