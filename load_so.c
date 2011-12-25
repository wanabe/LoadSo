#include <stdio.h>
#include <stdarg.h>
#include "load_so.h"
#include "st.h"

VALUE (*rb_obj_method)(VALUE, VALUE);
VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
VALUE (*rb_mod_const_set)(VALUE, VALUE, VALUE);
VALUE (*rb_obj_singleton_class)(VALUE);
VALUE (*rb_mod_public)(int, VALUE*, VALUE);
VALUE (*rb_mod_private)(int, VALUE*, VALUE);
VALUE (*rb_str_intern)(VALUE);
VALUE (*rb_mod_instance_method)(VALUE, VALUE);
VALUE (*rb_class_new_instance)(int, VALUE*, VALUE);
VALUE (*rb_f_p)(int, VALUE*, VALUE);
VALUE (*rb_str_plus)(VALUE, VALUE);
VALUE (*rb_f_eval)(int, VALUE*, VALUE);
VALUE (*rb_mod_define_method)(int, VALUE*, VALUE);
VALUE (*rb_mod_append_features)(VALUE, VALUE);
VALUE (*rb_ary_push_m)(int, VALUE*, VALUE);
VALUE (*rb_proc_s_new)(int, VALUE*, VALUE);
VALUE (*proc_call)(int, VALUE*, VALUE);
VALUE (*rb_f_block_given_p)();
struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;
VALUE dummy_proc;

typedef VALUE (*cfunc)(ANYARGS);

VALUE rb_cObject, rb_mKernel, rb_cModule, rb_cClass, rb_cArray, rb_cString, rb_cProc, rb_eRuntimeError, rb_eLoadError, rb_eTypeError, rb_eArgError;

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

void rb_define_alloc_func(VALUE klass, VALUE (*func)(VALUE)) {
  rb_method_definition_t *def = ALLOC(rb_method_definition_t);
  st_table *mtbl = RCLASS_M_TBL(klass);
  rb_method_entry_t *me = ALLOC(rb_method_entry_t);
  me->flag = NOEX_PRIVATE;
  me->mark = 0;
  me->called_id = ID_ALLOCATOR;
  me->klass = klass;
  me->def = def;
  def->type = VM_METHOD_TYPE_CFUNC;
  def->original_id = ID_ALLOCATOR;
  def->alias_count = 0;
  def->body.cfunc.func = func;
  def->body.cfunc.argc = 0;
  st_insert(mtbl, ID_ALLOCATOR, (st_data_t) me);

  set_buf_string("object_id");
  rb_mod_public(1, &value_buf_string, klass);
}

void rb_define_method(VALUE klass, const char *name, VALUE (*func)(ANYARGS), int argc) {
  VALUE vmethod, v[2] = {value_buf_string, dummy_proc};
  struct METHOD *method;

  set_buf_string(name);
  rb_mod_define_method(2, v, klass);
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

void rb_define_const(VALUE klass, const char *name, VALUE val) {
  set_buf_string(name);
  rb_mod_const_set(klass, value_buf_string, val);
}

void rb_raise(VALUE exc, const char *msg,...) {
  VALUE v[2] = {exc, value_buf_string};

  /* TODO: va_args */
  set_buf_string(msg);
  rb_f_raise(2, v);
}

VALUE rb_define_class_under(VALUE outer, const char *name) {
  VALUE mod = rb_class_new_instance(0, NULL, rb_cClass);
  rb_const_set(outer, rb_intern(name), mod);
  return mod;
}

VALUE rb_define_module_under(VALUE outer, const char *name) {
  VALUE mod = rb_class_new_instance(0, NULL, rb_cModule);
  rb_const_set(outer, rb_intern(name), mod);
  return mod;
}

VALUE rb_define_module(const char *name) {
  return rb_define_module_under(rb_cObject, name);
}

VALUE rb_data_object_alloc(VALUE klass, void *datap, RUBY_DATA_FUNC dmark, RUBY_DATA_FUNC dfree) {
  struct RData *data = (struct RData*)rb_class_new_instance(0, NULL, rb_cObject);
  data->basic.klass = klass;
  data->basic.flags = T_DATA;
  data->data = datap;
  data->dfree = dfree;
  data->dmark = dmark;

  return (VALUE)data;
}

VALUE rb_ary_new3(long n, ...) {
  va_list ar;
  VALUE ary = INT2FIX(n);
  long i;

  ary = rb_class_new_instance(1, &ary, rb_cArray);

  va_start(ar, n);
  for (i=0; i<n; i++) {
    RARRAY_PTR(ary)[i] = va_arg(ar, VALUE);
  }
  va_end(ar);

  return ary;
}

static VALUE load_so(VALUE self, VALUE file, VALUE init_name) {
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

VALUE rb_eval_string(const char *str) {
  set_buf_string(str);
  return rb_f_eval(1, &value_buf_string, Qnil);
}

VALUE rb_str_new_cstr(const char *ptr) {
  VALUE str = rb_class_new_instance(0, NULL, rb_cModule);
  set_buf_string(ptr);
  str = rb_str_plus(str, value_buf_string);
  return str;
}

void rb_include_module(VALUE klass, VALUE module) {
  rb_mod_append_features(module, klass);
}

void rb_set_end_proc(void (*func)(VALUE), VALUE data) {
  fprintf(stderr, "TODO: rb_set_end_proc is unable yet\n");
}

VALUE rb_ary_push(VALUE ary, VALUE item) {
  return rb_ary_push_m(1, &item, ary);
}

void rb_global_variable(VALUE *var) {
  VALUE global_list = rb_eval_string("$__loadso__global_list ||= []");
  rb_ary_push(global_list, *var);
}

VALUE rb_yield(VALUE val) {
  VALUE proc = rb_proc_s_new(0, NULL, rb_cProc);
  return proc_call(1, &val, proc);
}

int WINAPI rb_w32_Sleep(unsigned long msec) {
  Sleep(msec);
  return msec;
}

VALUE rb_block_proc() {
  return rb_proc_s_new(0, NULL, rb_cProc);
}

int rb_block_given_p() {
  return RTEST(rb_f_block_given_p());
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
  buf_string.basic.klass = rb_cString;
  rb_str_intern = get_instance_method(rb_cString, "intern");
  /* rb_intern, rb_const_get */

  rb_mod_const_set = get_method(rb_cObject, "const_set");
  /* rb_const_set */

  rb_f_eval = get_global_func("eval");
  /* rb_eval_string */

  dummy_proc = rb_eval_string("$__loadso__dummy_proc = proc{}");
  rb_mod_define_method = get_method(rb_cObject, "define_method");
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
  rb_eTypeError = rb_const_get(rb_cObject, rb_intern("TypeError"));
  rb_eArgError = rb_const_get(rb_cObject, rb_intern("ArgumentError"));

  rb_class_new_instance = get_method(rb_cObject, "new");
  rb_cModule = rb_const_get(rb_cObject, rb_intern("Module"));
  /* rb_define_module */

  rb_cClass = rb_const_get(rb_cObject, rb_intern("Class"));
  /* rb_define_class */

  rb_mod_append_features = get_instance_method(rb_cModule, "append_features");

  rb_cArray = rb_const_get(rb_cObject, rb_intern("Array"));
  rb_ary_push_m = get_instance_method(rb_cArray, "push");

  rb_cProc = rb_const_get(rb_cObject, rb_intern("Proc"));
  rb_proc_s_new = get_method(rb_cProc, "new");
  proc_call = get_instance_method(rb_cProc, "call");

  rb_f_block_given_p = get_global_func("block_given?");

  rb_str_plus = get_instance_method(rb_cString, "+");
  rb_define_global_function("load_so", load_so, 2);

  return 1;
}
