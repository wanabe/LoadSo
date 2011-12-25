#include "ext_rgss.h"
#include "graphics.h"

VALUE (*rb_obj_method)(VALUE, VALUE);
VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
VALUE (*rb_mod_const_set)(VALUE, VALUE, VALUE);
VALUE (*rb_obj_singleton_class)(VALUE);
VALUE (*rb_mod_attr_reader)(int, VALUE*, VALUE);
VALUE (*rb_mod_public)(int, VALUE*, VALUE);
VALUE (*rb_str_intern)(VALUE);
VALUE (*rb_mod_instance_method)(VALUE mod, VALUE vid);
VALUE (*rb_class_new_instance)(int, VALUE*, VALUE);
VALUE (*rb_f_p)(int argc, VALUE *argv, VALUE self);
struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;
typedef VALUE (*cfunc)(ANYARGS);

VALUE rb_cObject, rb_cModule, rb_cString, rb_eRuntimeError;
VALUE mExtRgss;

static void set_buf_string(const char *str) {
  buf_string.as.heap.ptr = (char*)str;
  buf_string.as.heap.len = strlen(str);
}

inline cfunc get_method_with_func(cfunc func, VALUE obj, char *name) {
  VALUE vmethod;
  struct METHOD *method;

  set_buf_string(name);
  vmethod = func(obj, value_buf_string);
  method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);
  return method->me.def->body.cfunc.func;
}

static cfunc get_method(VALUE obj, char *name) {
  return get_method_with_func(rb_obj_method, obj, name);
}

static cfunc get_global_func(char *name) {
  return get_method_with_func(rb_obj_method, Qnil, name);
}

static cfunc get_instance_method(VALUE mod, char *name) {
  return get_method_with_func(rb_mod_instance_method, mod, name);
}

void rb_p(VALUE obj) {
  rb_f_p(1, &obj, Qnil);
}

void rb_define_singleton_method(VALUE obj, char *name, VALUE (*func)(ANYARGS), int argc) {
  VALUE klass, vmethod;
  struct METHOD *method;

  klass = rb_obj_singleton_class(obj);
  set_buf_string(name);
  rb_mod_attr_reader(1, &value_buf_string, klass);
  rb_mod_public(1, &value_buf_string, klass);
  vmethod = rb_obj_method(obj, value_buf_string);
  method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);
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

void rb_const_set(VALUE klass, ID id, VALUE val) {
  VALUE sym = ID2SYM(id);
  rb_mod_const_set(klass, sym, val);
}

void rb_raise(VALUE exc, const char *msg,...) {
  VALUE v[2] = {exc, value_buf_string};

  /* TODO: va_args */
  set_buf_string(msg);
  rb_f_raise(2, v);
}

VALUE rb_define_module_under(VALUE outer, const char *name) {
  VALUE mod = rb_class_new_instance(0, NULL, rb_cModule);
  rb_const_set(outer, rb_intern(name), mod);
  return mod;
}

VALUE rb_define_module(const char *name) {
  return rb_define_module_under(rb_cObject, name);
}

int Init_ext_rgss(VALUE vmethod, VALUE cObject) {
  struct METHOD *method;

  LoadLibrary(DLL_NAME); /* reference_count++ to keep static variables */
  rb_cObject = cObject;

  method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);
  rb_obj_method = method->me.def->body.cfunc.func;
  /* get_method, get_global_func */

  rb_f_p = get_global_func("p");
  /* rb_p */

  rb_mod_instance_method = get_method(rb_cObject, "instance_method");
  /* get_instance_method */

  rb_mod_const_get = get_method(rb_cObject, "const_get");
  set_buf_string("String");
  rb_cString = rb_mod_const_get(1, &value_buf_string, rb_cObject);
  rb_str_intern = get_instance_method(rb_cString, "intern");
  /* rb_intern, rb_const_get */

  rb_mod_const_set = get_method(rb_cObject, "const_set");
  /* rb_const_set */

  rb_mod_attr_reader = get_method(rb_cObject, "attr_reader");
  rb_mod_public = get_method(rb_cObject, "public");
  rb_obj_singleton_class = get_global_func("singleton_class");
  /* rb_define_singleton_method */

  rb_f_raise = get_global_func("raise");
  rb_eRuntimeError = rb_const_get(rb_cObject, rb_intern("RuntimeError"));
  /* rb_raise */

  rb_class_new_instance = get_method(rb_cObject, "new");
  rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  /* rb_define_module */

  mExtRgss = rb_define_module("ExtRgss");
  Init_ExtGraphics();
  return 1;
}
