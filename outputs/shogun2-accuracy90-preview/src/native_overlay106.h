#pragma once
#include <d3d11_1.h>
#include <d3d9.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <ostream>
#include <utility>
#include "../../shogun2-possession-cleanup-v46/src/overlay_surface106.h"
#include "../../shogun2-possession-cleanup-v46/src/game_window105.h"
#include "../vendor/include/MinHook.h"
namespace nativeoverlay106 {
using Microsoft::WRL::ComPtr;
using overlay106::Snapshot;using overlay106::Frame;using overlay106::Key;
struct Stats {volatile LONG initialized{},error{},presents11{},presents9{},draws{},uploads{},resizes{},width{},height{},fullscreen{};};
static Stats stats;
inline bool ok(HRESULT h){if(FAILED(h)){InterlockedExchange(&stats.error,LONG(h));return false;}return true;}
struct Vertex11 {float x,y,u,v;};
class Renderer11 {
 IDXGISwapChain* chain_{};ComPtr<ID3D11Device> device_;ComPtr<ID3D11DeviceContext1> context_;
 ComPtr<ID3DDeviceContextState> own_;ComPtr<ID3D11RenderTargetView> target_;
 ComPtr<ID3D11VertexShader> vs_;ComPtr<ID3D11PixelShader> ps_;ComPtr<ID3D11InputLayout> layout_;
 ComPtr<ID3D11Buffer> vertices_;ComPtr<ID3D11SamplerState> sampler_;ComPtr<ID3D11BlendState> blend_;
 ComPtr<ID3D11RasterizerState> raster_;ComPtr<ID3D11DepthStencilState> depth_;
 struct Texture {ComPtr<ID3D11Texture2D> texture;ComPtr<ID3D11ShaderResourceView> view;unsigned w{},h{};uint64_t number{};Key key{};} textures_[overlay106::Wheel];
 unsigned width_{},height_{};
public:
 bool owns(IDXGISwapChain* chain)const{return chain_==chain;}
 unsigned width()const{return width_;}unsigned height()const{return height_;}
 void clear(){for(auto& t:textures_)t={};own_.Reset();target_.Reset();vs_.Reset();ps_.Reset();layout_.Reset();vertices_.Reset();sampler_.Reset();blend_.Reset();raster_.Reset();depth_.Reset();context_.Reset();device_.Reset();chain_=nullptr;width_=height_=0;}
 bool begin(IDXGISwapChain* chain){
  if(chain_==chain&&target_)return true;clear();
  if(!ok(chain->GetDevice(__uuidof(ID3D11Device),reinterpret_cast<void**>(device_.GetAddressOf()))))return false;
  ComPtr<ID3D11Device1> one;ComPtr<ID3D11DeviceContext> c;device_->GetImmediateContext(&c);
  if(!ok(device_.As(&one))||!ok(c.As(&context_)))return false;
  auto level=device_->GetFeatureLevel();unsigned flags=(device_->GetCreationFlags()&D3D11_CREATE_DEVICE_SINGLETHREADED)?D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED:0;
  if(!ok(one->CreateDeviceContextState(flags,&level,1,D3D11_SDK_VERSION,__uuidof(ID3D11Device),nullptr,&own_)))return false;
  ComPtr<ID3D11Texture2D> back;if(!ok(chain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(back.GetAddressOf()))))return false;
  D3D11_TEXTURE2D_DESC desc{};back->GetDesc(&desc);width_=desc.Width;height_=desc.Height;
  if(!ok(device_->CreateRenderTargetView(back.Get(),nullptr,&target_)))return false;
  static const char code[]= "Texture2D tex:register(t0);SamplerState samp:register(s0);struct V{float4 p:SV_POSITION;float2 uv:TEXCOORD0;};V vs(float2 p:POSITION,float2 uv:TEXCOORD0){V o;o.p=float4(p,0,1);o.uv=uv;return o;}float4 ps(V v):SV_TARGET{return tex.Sample(samp,v.uv);}";
  ComPtr<ID3DBlob> v,p,e;
  if(!ok(D3DCompile(code,sizeof(code)-1,nullptr,nullptr,nullptr,"vs","vs_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&v,&e))||
     !ok(D3DCompile(code,sizeof(code)-1,nullptr,nullptr,nullptr,"ps","ps_4_0",D3DCOMPILE_OPTIMIZATION_LEVEL3,0,&p,&e)))return false;
  if(!ok(device_->CreateVertexShader(v->GetBufferPointer(),v->GetBufferSize(),nullptr,&vs_))||!ok(device_->CreatePixelShader(p->GetBufferPointer(),p->GetBufferSize(),nullptr,&ps_)))return false;
  D3D11_INPUT_ELEMENT_DESC elements[]={{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D11_INPUT_PER_VERTEX_DATA,0}};
  if(!ok(device_->CreateInputLayout(elements,2,v->GetBufferPointer(),v->GetBufferSize(),&layout_)))return false;
  D3D11_BUFFER_DESC vb{};vb.ByteWidth=sizeof(Vertex11)*6;vb.Usage=D3D11_USAGE_DYNAMIC;vb.BindFlags=D3D11_BIND_VERTEX_BUFFER;vb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
  D3D11_SAMPLER_DESC sd{};sd.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sd.AddressU=sd.AddressV=sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sd.MaxLOD=D3D11_FLOAT32_MAX;
  D3D11_BLEND_DESC bd{};auto& rt=bd.RenderTarget[0];rt.BlendEnable=TRUE;rt.SrcBlend=rt.SrcBlendAlpha=D3D11_BLEND_ONE;rt.DestBlend=rt.DestBlendAlpha=D3D11_BLEND_INV_SRC_ALPHA;rt.BlendOp=rt.BlendOpAlpha=D3D11_BLEND_OP_ADD;rt.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
  D3D11_RASTERIZER_DESC rd{};rd.FillMode=D3D11_FILL_SOLID;rd.CullMode=D3D11_CULL_NONE;rd.DepthClipEnable=TRUE;rd.MultisampleEnable=TRUE;
  D3D11_DEPTH_STENCIL_DESC dd{};dd.DepthFunc=D3D11_COMPARISON_ALWAYS;
  if(!ok(device_->CreateBuffer(&vb,nullptr,&vertices_))||!ok(device_->CreateSamplerState(&sd,&sampler_))||!ok(device_->CreateBlendState(&bd,&blend_))||!ok(device_->CreateRasterizerState(&rd,&raster_))||!ok(device_->CreateDepthStencilState(&dd,&depth_)))return false;
  chain_=chain;return true;
 }
 bool draw(const Snapshot& s,unsigned layer){
  if(!target_||layer<1||layer>overlay106::Wheel)return false;auto& t=textures_[layer-1];auto& f=s.frame;
  ComPtr<ID3DDeviceContextState> previous;context_->SwapDeviceContextState(own_.Get(),&previous);
  struct Restore {ID3D11DeviceContext1* c;ID3DDeviceContextState* s;~Restore(){c->SwapDeviceContextState(s,nullptr);}} restore{context_.Get(),previous.Get()};
  if(t.w!=f.width||t.h!=f.height){t={};D3D11_TEXTURE2D_DESC d{};d.Width=f.width;d.Height=f.height;d.MipLevels=d.ArraySize=1;d.Format=DXGI_FORMAT_B8G8R8A8_UNORM;d.SampleDesc.Count=1;d.Usage=D3D11_USAGE_DEFAULT;d.BindFlags=D3D11_BIND_SHADER_RESOURCE;
   if(!ok(device_->CreateTexture2D(&d,nullptr,&t.texture))||!ok(device_->CreateShaderResourceView(t.texture.Get(),nullptr,&t.view)))return false;t.w=f.width;t.h=f.height;}
  if(t.number!=f.number||!overlay106::same(t.key,f.key)){context_->UpdateSubresource(t.texture.Get(),0,nullptr,s.pixels.data(),f.width*4,0);t.number=f.number;t.key=f.key;InterlockedIncrement(&stats.uploads);}
  D3D11_VIEWPORT viewport{0,0,float(width_),float(height_),0,1};context_->RSSetViewports(1,&viewport);context_->RSSetState(raster_.Get());
  auto target=target_.Get();context_->OMSetRenderTargets(1,&target,nullptr);context_->OMSetDepthStencilState(depth_.Get(),0);context_->OMSetBlendState(blend_.Get(),nullptr,0xffffffff);
  context_->SetPredication(nullptr,FALSE);context_->IASetInputLayout(layout_.Get());context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  UINT stride=sizeof(Vertex11),offset=0;auto buffer=vertices_.Get();context_->IASetVertexBuffers(0,1,&buffer,&stride,&offset);
  context_->VSSetShader(vs_.Get(),nullptr,0);context_->PSSetShader(ps_.Get(),nullptr,0);auto sampler=sampler_.Get();auto view=t.view.Get();context_->PSSetSamplers(0,1,&sampler);context_->PSSetShaderResources(0,1,&view);
  for(unsigned i=0;i<f.rect_count;++i){auto q=f.rect[i];float l=2.f*q.x/f.view_width-1,r=2.f*(q.x+q.width)/f.view_width-1,top=1-2.f*q.y/f.view_height,b=1-2.f*(q.y+q.height)/f.view_height;
   Vertex11 v[6]={{l,top,0,0},{r,top,1,0},{l,b,0,1},{l,b,0,1},{r,top,1,0},{r,b,1,1}};D3D11_MAPPED_SUBRESOURCE mapped{};
   if(!ok(context_->Map(vertices_.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped)))return false;memcpy(mapped.pData,v,sizeof v);context_->Unmap(vertices_.Get(),0);context_->Draw(6,0);}
  InterlockedIncrement(&stats.draws);return true;
 }
};
class Renderer9 {
 IDirect3DDevice9* device_{};struct Texture {ComPtr<IDirect3DTexture9> texture;unsigned w{},h{};uint64_t number{};Key key{};} textures_[overlay106::Wheel];
public:
 bool owns(IDirect3DDevice9* device)const{return device_==device;}
 void clear(){for(auto& t:textures_)t={};device_=nullptr;}
 bool draw(IDirect3DDevice9* device,const Snapshot& s,unsigned layer){
  if(layer<1||layer>overlay106::Wheel)return false;if(device_!=device){clear();device_=device;}auto& t=textures_[layer-1];const auto& f=s.frame;
  ComPtr<IDirect3DStateBlock9> state;ComPtr<IDirect3DSurface9> targets[4],depth,back;D3DVIEWPORT9 viewport{};
  if(!ok(device->CreateStateBlock(D3DSBT_ALL,&state))||!ok(state->Capture())||!ok(device->GetViewport(&viewport)))return false;
  for(unsigned i=0;i<4;++i)device->GetRenderTarget(i,&targets[i]);device->GetDepthStencilSurface(&depth);
  struct Restore {IDirect3DDevice9* d;IDirect3DStateBlock9* s;ComPtr<IDirect3DSurface9>* t;IDirect3DSurface9* z;D3DVIEWPORT9 v;~Restore(){s->Apply();for(unsigned i=0;i<4;++i)d->SetRenderTarget(i,t[i].Get());d->SetDepthStencilSurface(z);d->SetViewport(&v);}} restore{device,state.Get(),targets,depth.Get(),viewport};
  if(t.w!=f.width||t.h!=f.height){t={};if(!ok(device->CreateTexture(f.width,f.height,1,D3DUSAGE_DYNAMIC,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&t.texture,nullptr)))return false;t.w=f.width;t.h=f.height;}
  if(t.number!=f.number||!overlay106::same(t.key,f.key)){D3DLOCKED_RECT mapped{};if(!ok(t.texture->LockRect(0,&mapped,nullptr,D3DLOCK_DISCARD)))return false;
   for(unsigned row=0;row<f.height;++row)memcpy(static_cast<unsigned char*>(mapped.pBits)+row*mapped.Pitch,s.pixels.data()+size_t(row)*f.width*4,f.width*4);t.texture->UnlockRect(0);t.number=f.number;t.key=f.key;InterlockedIncrement(&stats.uploads);}
  if(!ok(device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&back)))return false;D3DSURFACE_DESC bd{};back->GetDesc(&bd);
  if(!ok(device->SetRenderTarget(0,back.Get())))return false;for(unsigned i=1;i<4;++i)device->SetRenderTarget(i,nullptr);device->SetDepthStencilSurface(nullptr);
  D3DVIEWPORT9 v{0,0,bd.Width,bd.Height,0,1};device->SetViewport(&v);device->SetVertexShader(nullptr);device->SetPixelShader(nullptr);device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
  for(auto pair:{std::pair<D3DRENDERSTATETYPE,DWORD>{D3DRS_ZENABLE,FALSE},{D3DRS_FILLMODE,D3DFILL_SOLID},{D3DRS_ZWRITEENABLE,FALSE},{D3DRS_ALPHABLENDENABLE,TRUE},{D3DRS_SRCBLEND,D3DBLEND_ONE},{D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA},{D3DRS_BLENDOP,D3DBLENDOP_ADD},{D3DRS_SEPARATEALPHABLENDENABLE,FALSE},{D3DRS_CULLMODE,D3DCULL_NONE},{D3DRS_LIGHTING,FALSE},{D3DRS_FOGENABLE,FALSE},{D3DRS_ALPHATESTENABLE,FALSE},{D3DRS_STENCILENABLE,FALSE},{D3DRS_SCISSORTESTENABLE,FALSE},{D3DRS_COLORWRITEENABLE,15},{D3DRS_SRGBWRITEENABLE,FALSE}})device->SetRenderState(pair.first,pair.second);
  device->SetTexture(0,t.texture.Get());device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);device->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);device->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);device->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
  device->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);device->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);device->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);
  device->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);device->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);device->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);device->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);device->SetSamplerState(0,D3DSAMP_SRGBTEXTURE,0);
  if(!ok(device->BeginScene()))return false;bool result=true;
  struct V {float x,y,z,w,u,v;};
  for(unsigned i=0;i<f.rect_count;++i){auto q=f.rect[i];float l=float(q.x)*bd.Width/f.view_width-.5f,r=float(q.x+q.width)*bd.Width/f.view_width-.5f,top=float(q.y)*bd.Height/f.view_height-.5f,b=float(q.y+q.height)*bd.Height/f.view_height-.5f;
   V vertices[]={{l,top,0,1,0,0},{r,top,0,1,1,0},{l,b,0,1,0,1},{l,b,0,1,0,1},{r,top,0,1,1,0},{r,b,0,1,1,1}};result=ok(device->DrawPrimitiveUP(D3DPT_TRIANGLELIST,2,vertices,sizeof(V)))&&result;}
  device->EndScene();if(result)InterlockedIncrement(&stats.draws);return result;
 }
};
using Gate=bool(*)(const Frame&,unsigned);static Gate gate;
// Explicit stop releases GPU resources. The adapter/trampolines stay resident;
// process termination must not call a graphics driver from CRT loader-lock
// destructors after its render threads have already been terminated.
static Renderer11& renderer11=*new Renderer11;static Renderer9& renderer9=*new Renderer9;
static overlay106::Reader readers[overlay106::Wheel];static uint64_t retry[overlay106::Wheel];
static SRWLOCK lock=SRWLOCK_INIT;static volatile LONG stopping;
using Present11=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT);using Resize11=HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*,UINT,UINT,UINT,DXGI_FORMAT,UINT);
using Present9=HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*,const RECT*,const RECT*,HWND,const RGNDATA*);using Reset9=HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice9*,D3DPRESENT_PARAMETERS*);
static Present11 original11;static Resize11 resize11;static Present9 original9;static Reset9 reset9;static void* hooks[4]{};
// Acknowledgement follows successful native Present. Record the call time,
// never claim physical scanout; TEST/occluded/lost presents cannot hide fallback.
struct Submission {unsigned backend[overlay106::Wheel]{};Frame frame[overlay106::Wheel]{};bool has[overlay106::Wheel]{};double wall{};};
template<class Draw>inline Submission surfaces(unsigned backend,Draw draw){
 Submission submitted;auto now=GetTickCount64();for(unsigned layer:{overlay106::HUD,overlay106::Wheel,overlay106::QTE,overlay106::Death,overlay106::Help}){const unsigned i=layer-1;if(now<retry[i])continue;if(!readers[i].open(GetCurrentProcessId(),i+1)){retry[i]=now+250;continue;}
  submitted.has[i]=true;auto image=readers[i].read();
  if(image&&gate&&gate(image->frame,i+1)){if(draw(*image,i+1)){submitted.backend[i]=backend;submitted.frame[i]=image->frame;}}
  else submitted.backend[i]=backend;
 }submitted.wall=overlay106::wall();return submitted;
}
inline void acknowledge(const Submission& s,HRESULT result){
 AcquireSRWLockExclusive(&lock);if(!stopping)for(unsigned i=0;i<overlay106::Wheel;++i)if(s.has[i])readers[i].acknowledge(result==S_OK?s.backend[i]:0,s.frame[i].number?&s.frame[i]:nullptr,s.wall);ReleaseSRWLockExclusive(&lock);
}
static __declspec(thread) bool presenting;
struct PresentScope {PresentScope(){presenting=true;}~PresentScope(){presenting=false;}};
static HRESULT STDMETHODCALLTYPE present11(IDXGISwapChain* chain,UINT sync,UINT flags){
 if(presenting||stopping||(flags&DXGI_PRESENT_TEST))return original11(chain,sync,flags);
 PresentScope scope;Submission submitted;DXGI_SWAP_CHAIN_DESC d{};
 if(SUCCEEDED(chain->GetDesc(&d))&&gamewindow105::belongs(d.OutputWindow,GetCurrentProcessId())){
  AcquireSRWLockExclusive(&lock);try{if(!stopping&&renderer11.begin(chain)){InterlockedIncrement(&stats.presents11);stats.width=LONG(renderer11.width());stats.height=LONG(renderer11.height());stats.fullscreen=!d.Windowed;submitted=surfaces(11,[](const Snapshot& s,unsigned layer){return renderer11.draw(s,layer);});}}catch(...){stats.error=E_OUTOFMEMORY;}ReleaseSRWLockExclusive(&lock);
 }HRESULT result=original11(chain,sync,flags);acknowledge(submitted,result);return result;
}
static HRESULT STDMETHODCALLTYPE resized11(IDXGISwapChain* chain,UINT count,UINT width,UINT height,DXGI_FORMAT format,UINT flags){
 AcquireSRWLockExclusive(&lock);if(renderer11.owns(chain)){renderer11.clear();InterlockedIncrement(&stats.resizes);}ReleaseSRWLockExclusive(&lock);return resize11(chain,count,width,height,format,flags);
}
static HRESULT STDMETHODCALLTYPE present9(IDirect3DDevice9* device,const RECT* source,const RECT* dest,HWND window,const RGNDATA* dirty){
 if(presenting||stopping)return original9(device,source,dest,window,dirty);
 PresentScope scope;Submission submitted;D3DDEVICE_CREATION_PARAMETERS p{};
 if(SUCCEEDED(device->GetCreationParameters(&p))&&gamewindow105::belongs(window?window:p.hFocusWindow,GetCurrentProcessId())){
  AcquireSRWLockExclusive(&lock);try{if(!stopping){InterlockedIncrement(&stats.presents9);ComPtr<IDirect3DSwapChain9> swap;D3DPRESENT_PARAMETERS d{};if(SUCCEEDED(device->GetSwapChain(0,&swap))&&SUCCEEDED(swap->GetPresentParameters(&d))){stats.width=LONG(d.BackBufferWidth);stats.height=LONG(d.BackBufferHeight);stats.fullscreen=!d.Windowed;}submitted=surfaces(9,[&](const Snapshot& s,unsigned layer){return renderer9.draw(device,s,layer);});}}catch(...){stats.error=E_OUTOFMEMORY;}ReleaseSRWLockExclusive(&lock);
 }HRESULT result=original9(device,source,dest,window,dirty);acknowledge(submitted,result);return result;
}
static HRESULT STDMETHODCALLTYPE reset_device9(IDirect3DDevice9* device,D3DPRESENT_PARAMETERS* params){AcquireSRWLockExclusive(&lock);if(renderer9.owns(device)){renderer9.clear();InterlockedIncrement(&stats.resizes);}ReleaseSRWLockExclusive(&lock);return reset9(device,params);}
inline bool hook(void* target,void* function,void** original,unsigned slot){if(!target||MH_CreateHook(target,function,original)!=MH_OK)return false;hooks[slot]=target;return true;}
inline void remove_pair(unsigned first){for(unsigned i=first;i<first+2;++i)if(hooks[i]){MH_RemoveHook(hooks[i]);hooks[i]=nullptr;}}
inline void install(Gate accept){
 gate=accept;stopping=0;HWND w=CreateWindowExW(0,L"STATIC",L"",WS_POPUP,0,0,2,2,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
 if(!w){stats.error=HRESULT_FROM_WIN32(GetLastError());return;}
 DXGI_SWAP_CHAIN_DESC sd{};sd.BufferCount=1;sd.BufferDesc.Width=sd.BufferDesc.Height=2;sd.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;sd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;sd.OutputWindow=w;sd.SampleDesc.Count=1;sd.Windowed=TRUE;
 ComPtr<IDXGISwapChain> sc;ComPtr<ID3D11Device> d;ComPtr<ID3D11DeviceContext> c;HRESULT h=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&sd,&sc,&d,nullptr,&c);
 if(FAILED(h))h=D3D11CreateDeviceAndSwapChain(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&sd,&sc,&d,nullptr,&c);
 bool a=false,b=false;if(SUCCEEDED(h)){auto table=*reinterpret_cast<void***>(sc.Get());a=hook(table[8],reinterpret_cast<void*>(&present11),reinterpret_cast<void**>(&original11),0)&&hook(table[13],reinterpret_cast<void*>(&resized11),reinterpret_cast<void**>(&resize11),1);}
 ComPtr<IDirect3D9> api;api.Attach(Direct3DCreate9(D3D_SDK_VERSION));if(api){D3DPRESENT_PARAMETERS p{};p.Windowed=TRUE;p.SwapEffect=D3DSWAPEFFECT_DISCARD;p.hDeviceWindow=w;p.BackBufferWidth=p.BackBufferHeight=2;p.BackBufferFormat=D3DFMT_UNKNOWN;ComPtr<IDirect3DDevice9> nine;
  if(SUCCEEDED(api->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,w,D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED|D3DCREATE_FPU_PRESERVE,&p,&nine))){auto table=*reinterpret_cast<void***>(nine.Get());b=hook(table[17],reinterpret_cast<void*>(&present9),reinterpret_cast<void**>(&original9),2)&&hook(table[16],reinterpret_cast<void*>(&reset_device9),reinterpret_cast<void**>(&reset9),3);}}
 if(!a)remove_pair(0);if(!b)remove_pair(2);c.Reset();sc.Reset();d.Reset();api.Reset();
 DestroyWindow(w);stats.initialized=(a?1:0)|(b?2:0);if(!a&&!b)stats.error=E_FAIL;
}
inline void stop(){InterlockedExchange(&stopping,1);for(auto p:hooks)if(p)MH_DisableHook(p);AcquireSRWLockExclusive(&lock);renderer11.clear();renderer9.clear();for(auto& r:readers)r.close();ReleaseSRWLockExclusive(&lock);}
inline void log(std::ostream& out){out<<"{\"event\":\"presentation106\",\"tick_ms\":"<<GetTickCount64()<<",\"initialized\":"<<stats.initialized<<",\"error\":"<<stats.error<<",\"presents11\":"<<stats.presents11<<",\"presents9\":"<<stats.presents9<<",\"draws\":"<<stats.draws<<",\"uploads\":"<<stats.uploads<<",\"resizes\":"<<stats.resizes<<",\"width\":"<<stats.width<<",\"height\":"<<stats.height<<",\"fullscreen\":"<<stats.fullscreen<<"}\n";}
}
