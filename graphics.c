#include "ext_rgss.h"
#include "d3d9.h"
#include "d3dx9.h"

#define FVF_VERTEX   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)
typedef struct
{
  float x, y, z;
  float rhw;
  D3DCOLOR diffuse;
  float u, v;
} VERTEX;

LPDIRECT3DDEVICE9 pD3DDevice;
LPD3DXEFFECT pEffect;

VERTEX vtx[]=
{
  {      0,       0,     0,  1.0, 0xff00ff00,0.5f,0.0f  },
  {    544,       0,     0,  1.0, 0xff0000ff,0.0f,0.0f  },
  {      0,     416,     0,  1.0, 0xffffffff,0.5f,1.0f  },
};

struct hWndFinder {
  HWND hWnd;
  DWORD pId;
};

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lp) {
  struct hWndFinder *pFinder = (struct hWndFinder *)lp;
  DWORD pId;
  GetWindowThreadProcessId(hWnd, &pId);
  if(pId == pFinder->pId) {
    pFinder->hWnd = hWnd;
    return FALSE;
  }
  return TRUE;
}

static VALUE Graphics_s_init(VALUE self) {
  HRESULT hr;
  LPDIRECT3D9 pD3D;
  D3DPRESENT_PARAMETERS D3DPP = {0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
    D3DSWAPEFFECT_DISCARD,NULL,TRUE,TRUE,D3DFMT_D24S8,D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL,
    D3DPRESENT_RATE_DEFAULT,D3DPRESENT_INTERVAL_DEFAULT};
  struct hWndFinder finder;
  ID3DXBuffer *pErrorMsgs;

  finder.pId = GetCurrentProcessId();
  EnumWindows(EnumWindowsProc, (LPARAM)&finder);
  if((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
    rb_raise(rb_eRuntimeError, "Direct3DCreate9 failed");
  }

  hr = pD3D->lpVtbl->CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, finder.hWnd,
                                  D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DPP, &pD3DDevice);
  if(FAILED(hr)) {
    hr = pD3D->lpVtbl->CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, finder.hWnd,
                                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &D3DPP, &pD3DDevice);
    if(FAILED(hr)) {
      rb_raise(rb_eRuntimeError, "IDirect3D9::CreateDevice failed");
    }
  }

  hr = D3DXCreateEffectFromFile(
    pD3DDevice, "ext_rgss.fx", NULL, NULL, 0, NULL, &pEffect, &pErrorMsgs);
  if(FAILED(hr)) {
    rb_raise(rb_eRuntimeError, "Can't load effect file");
  }
}

static VALUE Graphics_s_update(VALUE self) {
  pD3DDevice->lpVtbl->Clear(pD3DDevice, 0, NULL, (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER), D3DCOLOR_RGBA(0,0,0,0), 1.0f, 0);
  pD3DDevice->lpVtbl->SetFVF(pD3DDevice, FVF_VERTEX);
  if(SUCCEEDED(pD3DDevice->lpVtbl->BeginScene(pD3DDevice))) {
    pD3DDevice->lpVtbl->DrawPrimitiveUP(pD3DDevice, D3DPT_TRIANGLESTRIP,1,vtx,sizeof(VERTEX));
    pD3DDevice->lpVtbl->EndScene(pD3DDevice);
  }
  pD3DDevice->lpVtbl->Present(pD3DDevice, NULL,NULL,NULL,NULL);
  return Qnil;
}

void Init_ExtGraphics() {
  VALUE mGraphics = rb_const_get(rb_cObject, rb_intern("Graphics"));
  rb_define_singleton_method(mGraphics, "init", Graphics_s_init, 0);
  rb_define_singleton_method(mGraphics, "update", Graphics_s_update, 0);
}
