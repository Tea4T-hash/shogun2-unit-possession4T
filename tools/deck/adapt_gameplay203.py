"""Shared PC/Deck gameplay corrections, applied only to isolated build copies."""
SRC='native/outputs/shogun2-accuracy90-preview/src/'
def apply(transform):
 def camera(s):
  s='#include "../../../deck/framing203.h"\n'+s
  start=s.index('        // Head is the pivot;')
  end=s.index('        reinterpret_cast<Cancel>',start)
  return s[:start]+'''        // Keep the character beside the centre aim ray throughout scrolling.
        // Query the complete shoulder-offset segment, including native roll.
        float delta203[3],length203{};
        if(!framing203::segment(forward,eye.up,view_distance152,delta203,length203))return;
        for(unsigned i=0;i<3;++i)end[i]=eye.view[i]+delta203[i];
        float fraction203=framing203::fraction(camera_clearance152(eye.view,end),length203);
        for(unsigned i=0;i<3;++i){field<float>(camera,8+i*4)=eye.view[i]+delta203[i]*fraction203;field<float>(camera,0x14+i*4)=field<float>(camera,8+i*4)+forward[i];field<float>(camera,0x58+i*4)=eye.up[i];}
'''+s[end:]
 transform(SRC+'third_person152.h',camera)
 transform(SRC+'verify_view152.cpp',lambda s:s.replace('std::abs(field<float>(cam,0x10)+1.25f)<.001f','std::abs(field<float>(cam,0x10)+3.f*(.5f-.25f/(3.f*std::sqrt(1.f+.25f*.25f+.08f*.08f))))<.001f').replace('    view_reset152();require(', '    #include "../../../../tools/deck/verify_framing203.h"\n    view_reset152();require('))

 def camera_native_test(s):
  # Scroll distance is the backward component; the real shoulder segment is longer.
  for old,new in [('std::sqrt(d)-3','std::sqrt(d)-3*std::sqrt(1.f+.25f*.25f+.08f*.08f)'),('std::sqrt(distance154)-3','std::sqrt(distance154)-3*std::sqrt(1.f+.25f*.25f+.08f*.08f)')]:
   assert s.count(old)==1,old
   s=s.replace(old,new)
  return s.replace('Normal pitched stationary/walking camera fell back to first person','Normal pitched camera lost its full shoulder segment').replace('Orbit lost soldier pivot/distance','Orbit lost soldier pivot/shoulder segment')
 transform(SRC+'verify_view153.cpp',camera_native_test)
 def camera_death_test(s):
  old='std::sqrt(d)-5';assert s.count(old)==1
  s=s.replace(old,'std::sqrt(d)-5*std::sqrt(1.f+.25f*.25f+.08f*.08f)')
  old='sample.eye.view[0]-std::sin(yaw)*std::cos(pitch)*5';assert s.count(old)==1
  return s.replace(old,'sample.eye.view[0]-std::sin(yaw)*std::cos(pitch)*5+std::cos(yaw)*1.25f-std::sin(yaw)*std::sin(pitch)*.4f')
 transform(SRC+'verify_view156.cpp',camera_death_test)


 transform(SRC+'adapter.cpp',lambda s:s.replace('static void __fastcall detour(void* actor,void*) {','#include "../../../deck/locomotion203.h"\nstatic void __fastcall detour(void* actor,void*) {').replace('        if(controlled){finish(', '        if(controlled){locomotion_mark203(reinterpret_cast<uintptr_t>(actor));finish(').replace('create_unit_detach130();create_mounted_render_hooks70();','create_unit_detach130();create_locomotion203();create_mounted_render_hooks70();').replace('        if(disable_control_hooks130()!=MH_OK)result=MH_UNKNOWN;', '        if(disable_control_hooks130()!=MH_OK)result=MH_UNKNOWN;\n        {auto stop203=MH_DisableHook(reinterpret_cast<void*>(base+0x366a10));if(stop203!=MH_OK&&stop203!=MH_ERROR_DISABLED)result=MH_UNKNOWN;}'))

 def acquisition(s):
  s=s.replace('static uintptr_t __fastcall target_squad147(void* context,void*){','#include "../../../deck/acquisition203.h"\nstatic uintptr_t __fastcall target_squad147(void* context,void*){')
  s=s.replace('    return original_target_squad147(context);','    auto native_result203=original_target_squad147(context);\n    __try{return acquisition_group203(reinterpret_cast<uintptr_t>(context),native_result203);}__except(EXCEPTION_EXECUTE_HANDLER){return native_result203;}')
  s=s.replace('static void create_proximity147(){','static void create_proximity147(){\n    create_acquisition203();')
  return s
 transform(SRC+'proximity147.h',acquisition)
 transform(SRC+'unit_detach130.h',lambda s:s.replace('0x1679c0u}', '0x1679c0u,0x174ec0u,0x16aa00u,0x1b8740u,0x48d6f3u,0x459b4du,0x175360u,0x175346u,0x1b8830u,0x48d6a0u,0x48d860u,0x1752d0u,0x47efc0u}'))
 transform(SRC+'verify_proximity147.cpp',lambda s:s.replace('static void charge_assignment147(', '#include "../../../../tools/deck/verify_acquisition207.h"\nstatic void charge_assignment147(').replace(' field<float>(h,0x48)=1;', ' verify_acquisition207(f,formation,ip,h);\n field<float>(h,0x48)=1;').replace(' set_distance(6);require(', ' #include "../../../../tools/deck/verify_acquisition203.h"\n verify_acquisition207(f,formation,ip,possessed);\n set_distance(6);require(',1))
 def movement_fixture(s):
  edits=[('int main(){','#include "../../../../tools/deck/verify_locomotion203_provider.h"\nint main(){'),
    (' // Native mounted principal proof,',' #include "../../../../tools/deck/verify_locomotion203.h"\n // Native mounted principal proof,'),
    (' require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK,"Disable presentation hook");',' #include "../../../../tools/deck/verify_locomotion206.h"\n require(MH_DisableHook(MH_ALL_HOOKS)==MH_OK,"Disable presentation hook");')]
  for old,new in edits:
   assert s.count(old)==1,old
   s=s.replace(old,new)
  return s
 transform(SRC+'verify_melee_presentation137.cpp',movement_fixture)
