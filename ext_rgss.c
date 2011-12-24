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

VALUE Graphics_s_update(VALUE self) {
  pD3DDevice->lpVtbl->Clear(pD3DDevice, 0, NULL, (D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER), D3DCOLOR_RGBA(255,255,255,255), 1.0f, 0);
  pD3DDevice->lpVtbl->Present(pD3DDevice, NULL,NULL,NULL,NULL);
  return Qnil;
}

VALUE (*rb_obj_method)(VALUE, VALUE);
VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
VALUE (*rb_obj_singleton_class)(VALUE);
VALUE (*rb_mod_attr_reader)(int, VALUE*, VALUE);
VALUE (*rb_mod_public)(int, VALUE*, VALUE);
VALUE (*rb_str_intern)(VALUE);
VALUE (*rb_mod_instance_method)(VALUE mod, VALUE vid);
struct RString buf_string = {0x2005, 0};
VALUE value_buf_string = (VALUE)&buf_string;
typedef VALUE (*cfunc)(ANYARGS);

static void set_buf_string(const char *str) {
  buf_string.as.heap.ptr = (char*)str;
  buf_string.as.heap.len = strlen(str);
}

static cfunc get_method(VALUE obj, char *name) {
  VALUE vmethod;
  struct METHOD *method;

  set_buf_string(name);
  vmethod = rb_obj_method(obj, value_buf_string);
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  return method->me.def->body.cfunc.func;
}

static cfunc get_global_func(char *name) {
  return get_method(Qnil, name);
}
static cfunc get_instance_method(VALUE mod, char *name) {
  VALUE vmethod;
  struct METHOD *method;

  set_buf_string(name);
  vmethod = rb_mod_instance_method(mod, value_buf_string);
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  return method->me.def->body.cfunc.func;
}
void rb_define_singleton_method(VALUE obj, char *name, VALUE (*func)(ANYARGS), int argc) {
  VALUE klass, vmethod;
  struct METHOD *method;

  klass = rb_obj_singleton_class(obj);
  set_buf_string(name);
  rb_mod_attr_reader(1, &value_buf_string, klass);
  rb_mod_public(1, &value_buf_string, klass);
  vmethod = rb_obj_method(obj, value_buf_string);
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  method->me.def->type = VM_METHOD_TYPE_CFUNC;
  method->me.def->body.cfunc.func = func;
  method->me.def->body.cfunc.argc = argc;
}
ID rb_intern(const char *name) {
  set_buf_string(name);
  return SYM2ID(rb_str_intern(value_buf_string));
}
VALUE rb_const_get(VALUE klass, ID id) {
  VALUE sym = ID2SYM(id);
  return rb_mod_const_get(1, &sym, klass);
}

int Init_ext_rgss(VALUE vmethod, VALUE cObject) {
  struct METHOD *method;
  VALUE cString, mGraphics;

  LoadLibrary(DLL_NAME); /* reference_count++ to keep static variables */
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  rb_obj_method = method->me.def->body.cfunc.func;
  rb_f_raise = get_global_func("raise");
  rb_mod_const_get = get_method(cObject, "const_get");
  rb_obj_singleton_class = get_global_func("singleton_class");
  rb_mod_attr_reader = get_method(cObject, "attr_reader");
  rb_mod_public = get_method(cObject, "public");
  rb_mod_instance_method = get_method(cObject, "instance_method");
  set_buf_string("String");
  cString = rb_mod_const_get(1, &value_buf_string, cObject);
  rb_str_intern = get_instance_method(cString, "intern");
  mGraphics = rb_const_get(cObject, rb_intern("Graphics"));
  rb_define_singleton_method(mGraphics, "update", Graphics_s_update, 0);

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
