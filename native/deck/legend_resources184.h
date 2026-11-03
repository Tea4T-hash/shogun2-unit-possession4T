#pragma once
#include <windows.h>
#include <string>
namespace deck184 {
inline SRWLOCK resource_lock=SRWLOCK_INIT;
inline std::wstring resource_folder;
inline void publish_resources(const std::wstring& path){AcquireSRWLockExclusive(&resource_lock);resource_folder=path;ReleaseSRWLockExclusive(&resource_lock);}
inline std::wstring resources(){AcquireSRWLockShared(&resource_lock);auto path=resource_folder;ReleaseSRWLockShared(&resource_lock);return path;}
}
