#pragma once
#include "input174.h"
namespace deck182 {
// Matches the native action declarations; unused diagnostic inputs are not
// gameplay commands and do not gate the scheme. Physical-origin checks remain.
inline uint64_t buttons(deck174::Context context){switch(context){
 case deck174::Context::Menu:return 798723ull;
 case deck174::Context::Campaign:return 1048447ull;
 case deck174::Context::RTS:return 1048447ull;
 case deck174::Context::Possession:return 1048575ull;
 default:return deck174::all_buttons;}}
inline unsigned analogs(deck174::Context context){switch(context){
 case deck174::Context::Menu:return 60u;
 case deck174::Context::Campaign:return 63u;
 case deck174::Context::RTS:return 63u;
 case deck174::Context::Possession:return 63u;
 default:return 63u;}}
}
