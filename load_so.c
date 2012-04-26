#include "load_so.h"
#include <stdio.h>

static VALUE (*obj_to_enum)(int, VALUE *, VALUE);
struct RString buf_string = {{RSTRING_NOEMBED | T_STRING, 0}};
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
  return Qtrue;
}

VALUE rb_enumeratorize(VALUE obj, VALUE meth, int argc, VALUE *argv) {
  VALUE args = rb_ary_new_with_len(argc + 1);
  RARRAY_PTR(args)[0] = meth;
  MEMCPY(RARRAY_PTR(args), argv, VALUE, argc);
  return obj_to_enum(RARRAY_LEN(args), RARRAY_PTR(args), obj);
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
void Init_Thread();

void Init_LoadSo(VALUE vmethod, VALUE cObject) {
#ifdef CREATE_CONSOLE
  AllocConsole();
  freopen("CON", "w", stdout);
  freopen("CON", "w", stderr);
#endif
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
  Init_Thread();

  rb_define_global_function("load_so", load_so, 2);
  obj_to_enum = get_global_func("to_enum");
}
