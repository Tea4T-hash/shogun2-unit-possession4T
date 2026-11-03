#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
// Engine-local import bridge. No OS keyboard mutation or global API detour.
namespace deck188 {
using AsyncKey=SHORT(WINAPI*)(int);
struct Modifiers {
 struct Status {unsigned magic{0x38444f4d},version{1},bytes{sizeof(Status)};volatile LONG sequence{};unsigned installed{},refused{},control{},shift{},reads{},last_key{},last_value{};uint64_t tick{};};
 Status status;void** slot{};void* original{};HWND window{};
 SRWLOCK lock=SRWLOCK_INIT;
 template<class T>static bool read(uintptr_t at,T& out){SIZE_T n{};return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),&out,sizeof out,&n)&&n==sizeof out;}
 static bool text(uintptr_t at,const char* wanted){char value[64]{};SIZE_T n{};size_t length=strlen(wanted)+1;if(length>sizeof value)return false;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(at),value,length,&n)&&n==length&&!_stricmp(value,wanted);}
 static void** find(uintptr_t base){
  IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS32 nt{};if(!read(base,dos)||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||dos.e_lfanew>0x100000||!read(base+dos.e_lfanew,nt)||nt.Signature!=IMAGE_NT_SIGNATURE||nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)return nullptr;
  unsigned size=nt.OptionalHeader.SizeOfImage;auto directory=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];if(size<0x1000||directory.VirtualAddress>=size||directory.Size>size-directory.VirtualAddress||directory.Size<sizeof(IMAGE_IMPORT_DESCRIPTOR))return nullptr;
  for(unsigned i=0;i<directory.Size/sizeof(IMAGE_IMPORT_DESCRIPTOR)&&i<4096;++i){IMAGE_IMPORT_DESCRIPTOR entry{};if(!read(base+directory.VirtualAddress+i*sizeof entry,entry))return nullptr;if(!entry.Name&&!entry.FirstThunk)break;if(entry.Name>=size||!entry.OriginalFirstThunk||entry.OriginalFirstThunk>=size||entry.FirstThunk>=size)return nullptr;if(!text(base+entry.Name,"user32.dll"))continue;
   for(unsigned j=0;j<4096;++j){unsigned offset=j*4;if(offset+4>size-entry.OriginalFirstThunk||offset+4>size-entry.FirstThunk)return nullptr;DWORD name{};if(!read(base+entry.OriginalFirstThunk+offset,name))return nullptr;if(!name)break;if(IMAGE_SNAP_BY_ORDINAL32(name))continue;if(name>=size-2)return nullptr;if(text(base+name+2,"GetAsyncKeyState"))return reinterpret_cast<void**>(base+entry.FirstThunk+offset);}
  }return nullptr;
 }
 bool install(uintptr_t base,AsyncKey replacement,void* expected){
  if(slot)return status.installed!=0;auto target=find(base);void* current{};if(!target||!read(reinterpret_cast<uintptr_t>(target),current)||current!=expected){status.refused=1;return false;}
  DWORD protection{};if(!VirtualProtect(target,sizeof(void*),PAGE_READWRITE,&protection)){status.refused=1;return false;}
  original=expected;auto before=InterlockedCompareExchangePointer(target,reinterpret_cast<void*>(replacement),current);DWORD ignored{};bool protected_again=VirtualProtect(target,sizeof(void*),protection,&ignored)!=FALSE;
  if(before!=current){original=nullptr;status.refused=1;return false;}
  slot=target;status.installed=protected_again?1u:2u;return true;
 }
 void publish(bool control,bool shift,HWND focused,uint64_t now){AcquireSRWLockExclusive(&lock);InterlockedIncrement(&status.sequence);status.control=control;status.shift=shift;status.tick=now;window=focused;MemoryBarrier();InterlockedIncrement(&status.sequence);ReleaseSRWLockExclusive(&lock);}
 void clear(){publish(false,false,nullptr,0);}
 SHORT merge(int key,SHORT physical,uint64_t now,HWND foreground){
  if(key!=VK_CONTROL&&key!=VK_LCONTROL&&key!=VK_SHIFT&&key!=VK_LSHIFT)return physical;
  AcquireSRWLockExclusive(&lock);bool fresh=window&&foreground==window&&now>=status.tick&&now-status.tick<=100;bool held=fresh&&((status.control&&(key==VK_CONTROL||key==VK_LCONTROL))||(status.shift&&(key==VK_SHIFT||key==VK_LSHIFT)));SHORT result=held?SHORT(physical|0x8000):physical;
  if(key==VK_CONTROL||key==VK_LCONTROL||key==VK_SHIFT||key==VK_LSHIFT){InterlockedIncrement(&status.sequence);++status.reads;status.last_key=unsigned(key);status.last_value=unsigned(USHORT(result));InterlockedIncrement(&status.sequence);}ReleaseSRWLockExclusive(&lock);return result;
 }
 bool remove(AsyncKey replacement){clear();if(!slot)return true;DWORD protection{};if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&protection))return false;void* before=InterlockedCompareExchangePointer(slot,original,reinterpret_cast<void*>(replacement));DWORD ignored{};bool restored=VirtualProtect(slot,sizeof(void*),protection,&ignored)!=FALSE;if(before!=reinterpret_cast<void*>(replacement))return false;if(restored){slot=nullptr;status.installed=0;}return restored;}
};
}
