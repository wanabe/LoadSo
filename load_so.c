#include "load_so.h"

VALUE (*rb_obj_method)(VALUE, VALUE);
VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
VALUE (*rb_mod_const_set)(VALUE, VALUE, VALUE);
VALUE (*rb_obj_singleton_class)(VALUE);
VALUE (*rb_mod_attr_reader)(int, VALUE*, VALUE);
VALUE (*rb_mod_public)(int, VALUE*, VALUE);
VALUE (*rb_mod_private)(int, VALUE*, VALUE);
VALUE (*rb_str_intern)(VALUE);
VALUE (*rb_mod_instance_method)(VALUE, VALUE);
VALUE (*rb_class_new_instance)(int, VALUE*, VALUE);
VALUE (*rb_f_p)(int argc, VALUE *argv, VALUE self);
VALUE (*rb_str_plus)(VALUE, VALUE);
struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;
typedef VALUE (*cfunc)(ANYARGS);

VALUE rb_cObject, rb_mKernel, rb_cModule, rb_cString, rb_eRuntimeError, rb_eLoadError;

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

void rb_define_method(VALUE klass, const char *name, VALUE (*func)(ANYARGS), int argc) {
  VALUE vmethod;
  struct METHOD *method;

  set_buf_string(name);
  rb_mod_attr_reader(1, &value_buf_string, klass);
  vmethod = rb_mod_instance_method(klass, value_buf_string);
  method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);
  method->me.def->type = VM_METHOD_TYPE_CFUNC;
  method->me.def->body.cfunc.func = func;
  method->me.def->body.cfunc.argc = argc;
  rb_mod_public(1, &value_buf_string, klass);
}

void rb_define_private_method(VALUE klass, const char *name, VALUE (*func)(ANYARGS), int argc) {
  rb_define_method(klass, name, func, argc);
  rb_mod_private(1, &value_buf_string, klass);
}
void rb_define_singleton_method(VALUE obj, const char *name, VALUE (*func)(ANYARGS), int argc) {
  rb_define_method(rb_obj_singleton_class(obj), name, func, argc);
}

void rb_define_module_function(VALUE module, const char *name, VALUE (*func)(ANYARGS), int argc) {
  rb_define_private_method(module, name, func, argc);
  rb_define_singleton_method(module, name, func, argc);
}

void rb_define_global_function(const char *name, VALUE (*func)(ANYARGS), int argc) {
  rb_define_module_function(rb_mKernel, name, func, argc);
}

ID rb_intern(const char *name) {
  set_buf_string(name);
  return SYM2ID(rb_str_intern(value_buf_string));
}

VALUE rb_const_get(VALUE klass, ID id) {
  VALUE sym = ID2SYM(id);
  VALUE ret = rb_mod_const_get(1, &sym, klass);
  return ret;
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

VALUE load_so(VALUE self, VALUE file, VALUE init_name) {
  void (*init_func)();
  HMODULE hSo;

  hSo = LoadLibrary(RSTRING_PTR(file));
  if(!hSo) {
    VALUE msg;
    set_buf_string("Can't load: ");
    msg = rb_str_plus(value_buf_string, file);
    rb_raise(rb_eLoadError, RSTRING_PTR(msg));
  }
  init_func = (void(*)())GetProcAddress(hSo, RSTRING_PTR(init_name));
  if(!init_func) {
    VALUE msg;
    set_buf_string("Can't Init: ");
    msg = rb_str_plus(value_buf_string, file);
    rb_raise(rb_eLoadError, RSTRING_PTR(msg));
  }
  init_func();
  return Qnil;
}

int Init_LoadSo(VALUE vmethod, VALUE cObject) {
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
  /* rb_define_method */

  rb_mod_private = get_method(rb_cObject, "private");
  /* rb_define_private_method */

  rb_obj_singleton_class = get_global_func("singleton_class");
  /* rb_define_singleton_method */

  rb_mKernel = rb_const_get(rb_cObject, rb_intern("Kernel"));
  /* rb_define_global_function */

  rb_f_raise = get_global_func("raise");
  /* rb_raise */

  rb_eRuntimeError = rb_const_get(rb_cObject, rb_intern("RuntimeError"));
  rb_eLoadError = rb_const_get(rb_cObject, rb_intern("LoadError"));

  rb_class_new_instance = get_method(rb_cObject, "new");
  rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  /* rb_define_module */

  rb_str_plus = get_instance_method(rb_cString, "+");
  rb_define_global_function("load_so", load_so, 2);

  return 1;
}
