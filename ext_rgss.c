#include "ext_rgss.h"

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

VALUE rb_cObject, rb_eRuntimeError;

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
/* TODO: rb_raise can take more args */
void rb_raise(VALUE exc, const char *msg) {
  VALUE v[2] = {exc, value_buf_string};
  set_buf_string(msg);
  rb_f_raise(2, v);
}
int Init_ext_rgss(VALUE vmethod, VALUE cObject) {
  struct METHOD *method;
  VALUE cString, mGraphics;

  LoadLibrary(DLL_NAME); /* reference_count++ to keep static variables */
  rb_cObject = cObject;
  TypedData_Get_Struct(vmethod, struct METHOD, NULL, method);
  rb_obj_method = method->me.def->body.cfunc.func;
  rb_mod_const_get = get_method(cObject, "const_get");
  rb_obj_singleton_class = get_global_func("singleton_class");
  rb_mod_attr_reader = get_method(cObject, "attr_reader");
  rb_mod_public = get_method(cObject, "public");
  rb_mod_instance_method = get_method(cObject, "instance_method");
  set_buf_string("String");
  cString = rb_mod_const_get(1, &value_buf_string, cObject);
  rb_str_intern = get_instance_method(cString, "intern");

  rb_f_raise = get_global_func("raise");
  rb_eRuntimeError = rb_const_get(rb_cObject, rb_intern("RuntimeError"));

  Init_ExtGraphics();
  return 1;
}
