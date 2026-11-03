#pragma once
#include <windows.h>
#include "input174.h"
namespace deck181 {
struct InputTrace {unsigned magic{0x38504944},version{1},bytes{sizeof(InputTrace)};volatile LONG sequence{};unsigned role{},backend{},context{},window{},pending{},mode{},aim{};uint64_t tick{},sample{},epoch{},buttons{};float left_x{},left_y{},right_x{},right_y{},l2{},r2{};};
static InputTrace input_trace;
inline void trace_input(unsigned role,unsigned backend,unsigned context,HWND window,unsigned pending,unsigned mode,unsigned aim,const deck174::Frame* f=nullptr){
 InterlockedIncrement(&input_trace.sequence);input_trace.role=role;input_trace.backend=backend;input_trace.context=context;input_trace.window=unsigned(reinterpret_cast<uintptr_t>(window));input_trace.pending=pending;input_trace.mode=mode;input_trace.aim=aim;input_trace.tick=GetTickCount64();
 if(f){input_trace.sample=f->sample_us;input_trace.epoch=f->epoch;input_trace.buttons=f->buttons;input_trace.left_x=f->left_stick.x;input_trace.left_y=f->left_stick.y;input_trace.right_x=f->right_stick.x;input_trace.right_y=f->right_stick.y;input_trace.l2=f->left_trigger;input_trace.r2=f->right_trigger;}else{input_trace.sample=input_trace.epoch=input_trace.buttons=0;input_trace.left_x=input_trace.left_y=input_trace.right_x=input_trace.right_y=input_trace.l2=input_trace.r2=0;}
 MemoryBarrier();InterlockedIncrement(&input_trace.sequence);
}
}
