#pragma once
#include <windows.h>
namespace deck218 {
// SteamBackend clears its owned reader even if SDK Shutdown returns false.
// Callers must follow that actual state, not leave running=1 with no reader.
// Retain the lease only when a backend really remains initialized.
template<class Backend>bool close_reader(Backend& backend,volatile LONG& running){
 backend.close();
 if(backend.initialized())return false;
 InterlockedExchange(&running,0);return true;
}
}
