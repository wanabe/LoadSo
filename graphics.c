#include "ext_rgss.h"
#include "d3d9.h"

HWND hWndRgss;
LPDIRECT3DDEVICE9 pD3DDevice;

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lp) {
  DWORD pId;
  GetWindowThreadProcessId(hWnd, &pId);
  if(pId == lp) {
    hWndRgss = hWnd;
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

  EnumWindows(EnumWindowsProc, (LPARAM)GetCurrentProcessId());
  if(FAILED(CoInitialize(NULL))) {
    return 0;
  }
  if((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
    return 0;
  }

  hr = pD3D->lpVtbl->CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWndRgss, D3DCREATE_HARDWARE_VERTEXPROCESSING, &D3DPP, &pD3DDevice);
  if(FAILED(hr)) {
    return 0;
  }
}

static VALUE Graphics_s_update(VALUE self) {
  pD3DDevice->lpVtbl->Clear(pD3DDevice, 0, NULL, (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER), D3DCOLOR_RGBA(255,255,255,255), 1.0f, 0);
  pD3DDevice->lpVtbl->Present(pD3DDevice, NULL,NULL,NULL,NULL);
  return Qnil;
}

void Init_ExtGraphics() {
  VALUE mGraphics = rb_const_get(rb_cObject, rb_intern("Graphics"));
  rb_define_singleton_method(mGraphics, "init", Graphics_s_init, 0);
  rb_define_singleton_method(mGraphics, "update", Graphics_s_update, 0);
}
