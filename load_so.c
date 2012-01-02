#include "load_so.h"
#include "st.h"

struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;

VALUE set_buf_string2(const char *str, long len) {
  buf_string.as.heap.ptr = (char*)str;
  buf_string.as.heap.len = len;
  return value_buf_string;
}

VALUE set_buf_string(const char *str) {
  buf_string.as.heap.ptr = (char*)str;
  buf_string.as.heap.len = strlen(str);
  return value_buf_string;
}

static VALUE load_so(VALUE self, VALUE file, VALUE init_name) {
  void (*init_func)();
  HMODULE hSo;

  hSo = LoadLibrary(RSTRING_PTR(file));
  if(!hSo) {
    rb_raise(rb_eLoadError, "Can't load: %s", RSTRING_PTR(file));
  }
  init_func = (void(*)())GetProcAddress(hSo, RSTRING_PTR(init_name));
  if(!init_func) {
    rb_raise(rb_eLoadError, "Can't Init: %s", RSTRING_PTR(file));
  }
  init_func();
  return Qnil;
}

void Init_ClassCore(VALUE);
void Init_VariableCore();
void Init_StringCore();
void Init_Class();
void Init_Variable();
void Init_String();
void Init_Numeric();
void Init_Array();
void Init_ObjSpace();
void Init_Eval();
void Init_Hash();

void Init_LoadSo(VALUE vmethod, VALUE cObject) {
  LoadLibrary(DLL_NAME); /* reference_count++ to keep static variables */
  rb_cObject = cObject;

  Init_ClassCore(vmethod);
  Init_VariableCore();
  Init_StringCore();

  Init_Eval();

  Init_Class();
  Init_Variable();
  Init_String();
  Init_Numeric();
  Init_Array();
  Init_ObjSpace();
  Init_Hash();

  rb_define_global_function("load_so", load_so, 2);
}
