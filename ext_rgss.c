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

VALUE (*rb_obj_method)(VALUE obj, VALUE vid);
VALUE (*rb_f_raise)(int argc, VALUE *argv);
struct RString buf_string = {0x2005, 0};
VALUE value_buf_string = (VALUE)&buf_string;
typedef VALUE (*cfunc)(ANYARGS);

static cfunc get_method(VALUE obj, char *name) {
  VALUE vmethod;
  struct METHOD *method;

  buf_string.as.heap.ptr = name;
  buf_string.as.heap.len = strlen(name);
  vmethod = rb_obj_method(obj, value_buf_string);
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  return method->me.def->body.cfunc.func;
}

static cfunc get_global_func(char *name) {
  return get_method(Qnil, name);
}

int Init_ext_rgss(VALUE vmethod, VALUE cObject) {
  struct METHOD *method;

  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  rb_obj_method = method->me.def->body.cfunc.func;
  rb_f_raise = get_global_func("raise");

  EnumWindows(EnumWindowsProc, (LPARAM)GetCurrentProcessId());
  {
    HRESULT hr;
    LPDIRECT3D9 pD3D;
    D3DPRESENT_PARAMETERS D3DPP = {0,0,D3DFMT_UNKNOWN,0,D3DMULTISAMPLE_NONE,0,
      D3DSWAPEFFECT_DISCARD,NULL,TRUE,TRUE,D3DFMT_D24S8,D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL,
      D3DPRESENT_RATE_DEFAULT,D3DPRESENT_INTERVAL_DEFAULT};

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
  return 1;
}

void Graphics_s_update() {
  pD3DDevice->lpVtbl->Clear(pD3DDevice, 0, NULL, (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER), D3DCOLOR_RGBA(255,255,255,255), 1.0f, 0);
  pD3DDevice->lpVtbl->Present(pD3DDevice, NULL,NULL,NULL,NULL);
}
