#include "identity.h"
#include "../vendor/include/MinHook.h"
static uintptr_t base;
static volatile LONG enabled;
enum {POSSESSED=2};
struct TestStatus {LONG mode{},stop{},release{};unsigned lease_generation{1};};
static bool charge_input_locked146(){return false;}
static TestStatus status;
static TestStatus* state=&status;
#include "native_keyboard84.h"
#include "equipment_policy81.h"
static std::array<std::uint8_t,256> physical{};
static bool valid=true;
static unsigned reads;
static bool __cdecl device_read(){++reads;if(valid)memcpy(reinterpret_cast<void*>(base+0x1b30f58),physical.data(),256);return valid;}
int main(){void* allocation=nullptr;try{
    allocation=VirtualAlloc(nullptr,0x1bd0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    require(allocation!=nullptr,"Keyboard fixture allocate");base=reinterpret_cast<uintptr_t>(allocation);
    std::array<unsigned char,0xe00> root151{};
    *reinterpret_cast<uintptr_t*>(base+0x1bc8180)=reinterpret_cast<uintptr_t>(root151.data());
    auto code=reinterpret_cast<unsigned char*>(base+0xbd75f0);
    // Exact native stolen prologue, state-copy instructions and relocated
    // operands. Only its device COM read/reacquire tail is a fixture substitute.
    const unsigned char prefix[]={0x56,0x57,0xb9,0x40,0,0,0,0xbe,0,0,0,0,0xbf,0,0,0,0,0xf3,0xa5};
    memcpy(code,prefix,sizeof prefix);
    *reinterpret_cast<uintptr_t*>(code+8)=base+0x1b30f58;
    *reinterpret_cast<uintptr_t*>(code+13)=base+0x1b31058;
    const unsigned char tail[]={0xb8,0,0,0,0,0xff,0xd0,0x5f,0x5e,0xc3};
    memcpy(code+sizeof prefix,tail,sizeof tail);
    *reinterpret_cast<uintptr_t*>(code+sizeof prefix+1)=reinterpret_cast<uintptr_t>(&device_read);
    std::array<unsigned char,sizeof prefix> original_prefix{};memcpy(original_prefix.data(),code,sizeof prefix);
    DWORD prior=0;require(VirtualProtect(code,4096,PAGE_EXECUTE_READ,&prior)!=0,"Keyboard fixture protect");
    FlushInstructionCache(GetCurrentProcess(),code,4096);
    require(MH_Initialize()==MH_OK,"Keyboard fixture MinHook initialize");create_keyboard_hook84();
    require(MH_EnableHook(keyboard_poll_target84)==MH_OK,"Keyboard fixture enable");
    auto poll=reinterpret_cast<KeyboardPoll84>(code);
    auto current=reinterpret_cast<std::uint8_t*>(base+0x1b30f58);
    auto previous=reinterpret_cast<std::uint8_t*>(base+0x1b31058);
    physical[2]=0x80;physical[0x1d]=0x80;physical[0x21]=0x80;
    require(poll()&&current[2]==0x80&&current[0x1d]==0x80&&current[0x21]==0x80&&keyboard_blocked84==0,"RTS control-group and Ctrl remain native");
    enabled=1;status.mode=POSSESSED;
    #include "../../../../tools/deck/verify_keyboard_hud198.h"
    physical[0x19]=0x80;require(poll()&&!current[0x19]&&!previous[0x19]&&view_input152.toggles==1,"P edge not captured by actual poll hook");
    for(unsigned repeat152=0;repeat152<100;++repeat152)require(poll()&&view_input152.toggles==1,"Held P repeated in native poll");
    status.mode=0;require(poll()&&!current[0x19],"Held P leaked into RTS");physical[0x19]=0;require(poll(),"P release");physical[0x19]=0x80;require(poll()&&current[0x19]==0x80,"Fresh RTS P was consumed");physical[0x19]=0;require(poll(),"RTS P release");status.mode=POSSESSED;
    for(unsigned i=0;i<11;++i)physical[keyboard84::scan(i)]=0x80;
    physical[0x11]=0x80;physical[0x2a]=0x80;physical[0x44]=0x80; // W, Shift, F10
    equipment81::Edges edges;
    require(edges.sample(3,true)==3,"Equipment physical edges available");
    for(unsigned n=0;n<50000;++n){
        require(poll(),"Native successful poll forwarded");
        require(current[0x21]==0&&previous[0x21]==0&&climb_key_held97,"Wall action key masked only in owned keyboard stream");
        for(unsigned i=0;i<11;++i)require(!current[keyboard84::scan(i)]&&!previous[keyboard84::scan(i)],"Owned key current and previous masked");
        require(current[0x11]==0&&previous[0x11]==0&&current[0x2a]==0x80&&current[0x44]==0x80&&current[0x1d]==0x80,"Owned WASD must not leak into native camera/selection actions; exit/Shift/Ctrl preserved");
        require(physical[2]==0x80&&keyboard_window_owned84('1')&&keyboard_window_owned84('0')&&keyboard_window_owned84(VK_TAB)&&!keyboard_window_owned84(VK_F10),"Physical input and exit control preserved");
    }
    require(edges.sample(3,true)==0,"Held equipment input does not repeat");
    const unsigned wall_scans[]={0x11,0x1e,0x1f,0x20,0x10,0x12,0x2a,0x36};
    keyboard_wall98=1;for(auto scan:wall_scans)physical[scan]=0x80;
    for(unsigned n=0;n<5000;++n){require(poll(),"Wall poll forwarded");for(auto scan:wall_scans)require(!current[scan]&&!previous[scan],"Owned native wall intent isolates ground keys");require(current[0x44]==0x80&&current[0x1d]==0x80,"Wall filter retains F10/Ctrl");}
    require(keyboard_wall_held98==255,"Physical wall key latches preserved");keyboard_wall98=0;
    status.release=1;status.mode=0;
    require(poll()&&current[2]==0&&current[0x21]==0&&previous[0x21]==0&&climb_key_held97&&keyboard_window_owned84('1'),"Held FPS key cannot trigger a delayed RTS group selection");
    for(auto scan:wall_scans)require(!current[scan]&&!previous[scan],"Held wall keys cannot become delayed RTS intent");
    valid=false;auto held_before=keyboard_filter84.held;
    current[2]=0x80;
    require(!poll()&&current[2]==0x80&&keyboard_filter84.held==held_before&&keyboard_failures84==1,"Failed native acquire result, buffers and latch preserved");
    valid=true;physical.fill(0);
    require(poll()&&current[2]==0&&previous[2]==0&&!keyboard_window_owned84('1')&&!climb_key_held97,"Release drains FPS latch without phantom native edge");
    require(!keyboard_wall_held98,"Wall release drains latch");physical[0x11]=0x80;require(poll()&&current[0x11]==0x80,"Fresh RTS movement key works");
    physical[2]=0x80;require(poll()&&current[2]==0x80&&previous[2]==0,"Fresh RTS number press works");
    status.release=0;status.mode=POSSESSED;enabled=0;
    require(poll()&&current[2]==0x80,"Disabled adapter leaves keyboard native");
    require(keyboard84::window_bit('1')==1&&keyboard84::window_bit('9')==256&&keyboard84::window_bit('0')==512&&keyboard84::window_bit(VK_TAB)==1024&&keyboard84::window_bit('W')==0,"Window and native scan ownership agree");
    require(MH_DisableHook(keyboard_poll_target84)==MH_OK&&!memcmp(code,original_prefix.data(),original_prefix.size()),"Exact native keyboard bytes restore");
    unsigned hooked_reads=unsigned(keyboard_polls84);
    require(poll()&&current[2]==0x80&&unsigned(keyboard_polls84)==hooked_reads&&reads==hooked_reads+1,"Unhooked native poll executes exactly once");
    MH_Uninitialize();hud_visibility_close148();VirtualFree(allocation,0,MEM_RELEASE);allocation=nullptr;
    std::cout<<"PASS: 50000 original isolation plus 5000 wall isolation calls through production keyboard hook; exact native prologue/state-copy operands, cdecl/AL return, native failures, FPS number/Tab/F isolation, WASD isolation and unchanged exit/Ctrl keys, held-key RTS handoff and exact byte restoration. Full native input lifecycle still requires live validation.\n";
    return 0;
}catch(const std::exception& ex){MH_Uninitialize();if(allocation)VirtualFree(allocation,0,MEM_RELEASE);std::cerr<<ex.what()<<'\n';return 1;}}
