#include <stdio.h>
#include <stdarg.h>
#include "load_so.h"
#include "st.h"

VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_f_p)(int, VALUE*, VALUE);
VALUE (*rb_f_eval)(int, VALUE*, VALUE);
VALUE (*rb_proc_s_new)(int, VALUE*, VALUE);
VALUE (*proc_call)(int, VALUE*, VALUE);
VALUE (*rb_f_block_given_p)();
VALUE (*rb_obj_method)(VALUE, VALUE);
VALUE (*rb_obj_singleton_class)(VALUE);
VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
VALUE (*rb_mod_const_set)(VALUE, VALUE, VALUE);
VALUE (*rb_mod_public)(int, VALUE*, VALUE);
VALUE (*rb_mod_private)(int, VALUE*, VALUE);
VALUE (*rb_mod_instance_method)(VALUE, VALUE);
VALUE (*rb_mod_define_method)(int, VALUE*, VALUE);
VALUE (*rb_mod_append_features)(VALUE, VALUE);
VALUE (*rb_class_new_instance)(int, VALUE*, VALUE);
VALUE (*rb_f_float)(VALUE, VALUE);
VALUE (*rb_str_intern)(VALUE);
VALUE (*rb_str_plus)(VALUE, VALUE);
VALUE (*rb_str_concat)(VALUE, VALUE);
VALUE (*rb_ary_push_m)(int, VALUE*, VALUE);
VALUE (*rb_ary_join_m)(int, VALUE*, VALUE);
struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;
VALUE dummy_proc;

typedef VALUE (*cfunc)(ANYARGS);

VALUE rb_cObject, rb_mKernel, rb_cModule, rb_cClass, rb_cArray, rb_cString, rb_cFloat, rb_cProc, rb_eRuntimeError, rb_eLoadError, rb_eTypeError, rb_eArgError;

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

VALUE rb_str_cat(VALUE str, const char *ptr, long len) {
  buf_string.as.heap.ptr = (char*)str;
  buf_string.as.heap.len = len;
  str = rb_str_concat(str, value_buf_string);
  return str;
}

VALUE rb_str_new(const char *ptr, long len) {
  return rb_str_cat(rb_class_new_instance(0, NULL, rb_cString), ptr, len);
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
  VALUE str = rb_class_new_instance(0, NULL, rb_cString);
  set_buf_string(ptr);
  str = rb_str_concat(str, value_buf_string);
  return str;
}

void rb_include_module(VALUE klass, VALUE module) {
  rb_mod_append_features(module, klass);
}

void rb_set_end_proc(void (*func)(VALUE), VALUE data) {
  fprintf(stderr, "TODO: LoadSo can't set rb_set_end_proc\n");
}

VALUE rb_ary_push(VALUE ary, VALUE item) {
  return rb_ary_push_m(1, &item, ary);
}

void rb_global_variable(VALUE *var) {
  VALUE global_list = rb_eval_string("$__loadso__global_list ||= []");
  rb_ary_push(global_list, *var);
}

void rb_check_type(VALUE x, int t) {
  int xt;

  xt = TYPE(x);
  if (xt != t || (xt == T_DATA && RTYPEDDATA_P(x))) {
    rb_raise(rb_eTypeError, "wrong argument type");
  }
}

VALUE rb_ary_entry(VALUE ary, long offset) {
  if(offset < 0) {
    offset += RARRAY_LEN(ary);
  }
  if(offset < 0 || offset >= RARRAY_LEN(ary)) {
    return Qnil;
  }

  return RARRAY_PTR(ary)[offset];
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

VALUE rb_ary_new() {
  return rb_class_new_instance(0, NULL, rb_cArray);
}

VALUE rb_ary_new4(long n, const VALUE *elts) {
  VALUE ary = INT2FIX(n);

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  if (n > 0 && elts) {
    memcpy(RARRAY_PTR(ary), elts, sizeof(VALUE) * n);
  }
  return ary;
}

int
rb_scan_args(int argc, const VALUE *argv, const char *fmt, ...)
{
    int i;
    const char *p = fmt;
    VALUE *var;
    va_list vargs;
    int f_var = 0, f_hash = 0, f_block = 0;
    int n_lead = 0, n_opt = 0, n_trail = 0, n_mand;
    int argi = 0;
    VALUE hash = Qnil;

    if (ISDIGIT(*p)) {
	n_lead = *p - '0';
	p++;
	if (ISDIGIT(*p)) {
	    n_opt = *p - '0';
	    p++;
	    if (ISDIGIT(*p)) {
		n_trail = *p - '0';
		p++;
		goto block_arg;
	    }
	}
    }
    if (*p == '*') {
	f_var = 1;
	p++;
	if (ISDIGIT(*p)) {
	    n_trail = *p - '0';
	    p++;
	}
    }
  block_arg:
    if (*p == ':') {
	f_hash = 1;
	p++;
    }
    if (*p == '&') {
	f_block = 1;
	p++;
    }
    if (*p != '\0') {
	rb_raise(rb_eRuntimeError, "bad scan arg format");//rb_fatal("bad scan arg format: %s", fmt);
    }
    n_mand = n_lead + n_trail;

    if (argc < n_mand)
	goto argc_error;

    va_start(vargs, fmt);

    /* capture an option hash - phase 1: pop */
    if (f_hash && n_mand < argc) {
	VALUE last = argv[argc - 1];

	if (NIL_P(last)) {
	    /* nil is taken as an empty option hash only if it is not
	       ambiguous; i.e. '*' is not specified and arguments are
	       given more than sufficient */
	    if (!f_var && n_mand + n_opt < argc)
		argc--;
	}
	else {
	    hash = Qnil; /*rb_check_convert_type(last, T_HASH, "Hash", "to_hash");
	    if (!NIL_P(hash))
		argc--;*/
	}
    }
    /* capture leading mandatory arguments */
    for (i = n_lead; i-- > 0; ) {
	var = va_arg(vargs, VALUE *);
	if (var) *var = argv[argi];
	argi++;
    }
    /* capture optional arguments */
    for (i = n_opt; i-- > 0; ) {
	var = va_arg(vargs, VALUE *);
	if (argi < argc - n_trail) {
	    if (var) *var = argv[argi];
	    argi++;
	}
	else {
	    if (var) *var = Qnil;
	}
    }
    /* capture variable length arguments */
    if (f_var) {
	int n_var = argc - argi - n_trail;

	var = va_arg(vargs, VALUE *);
	if (0 < n_var) {
	    if (var) *var = rb_ary_new4(n_var, &argv[argi]);
	    argi += n_var;
	}
	else {
	    if (var) *var = rb_ary_new();
	}
    }
    /* capture trailing mandatory arguments */
    for (i = n_trail; i-- > 0; ) {
	var = va_arg(vargs, VALUE *);
	if (var) *var = argv[argi];
	argi++;
    }
    /* capture an option hash - phase 2: assignment */
    if (f_hash) {
	var = va_arg(vargs, VALUE *);
	if (var) *var = hash;
    }
    /* capture iterator block */
    if (f_block) {
	var = va_arg(vargs, VALUE *);
	if (rb_block_given_p()) {
	    *var = rb_block_proc();
	}
	else {
	    *var = Qnil;
	}
    }
    va_end(vargs);

    if (argi < argc)
	goto argc_error;

    return argc;

  argc_error:
    if (0 < n_opt)
	rb_raise(rb_eArgError, "wrong number of arguments (%d for %d..%d%s)",
		 argc, n_mand, n_mand + n_opt, f_var ? "+" : "");
    else
	rb_raise(rb_eArgError, "wrong number of arguments (%d for %d%s)",
		 argc, n_mand, f_var ? "+" : "");
  return 0;
}

VALUE rb_ary_join(VALUE ary, VALUE sep) {
  rb_ary_join_m(1, &sep, ary);
  return rb_ary_join_m(1, &sep, ary);
}

VALUE rb_newobj() {
  return rb_class_new_instance(0, NULL, rb_cObject);
}

VALUE rb_float_new(double d) {
  NEWOBJ(flt, struct RFloat);
  OBJSETUP(flt, rb_cFloat, T_FLOAT);

  flt->float_value = d;
  return (VALUE)flt;
}

VALUE rb_Float(VALUE val) {
  return rb_f_float(Qnil, val);
}

double rb_num2dbl(VALUE num) {
  /* TODO: String shouldn't be OK */
  return RFLOAT_VALUE(rb_f_float(Qnil, num));
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

  rb_cFloat = rb_const_get(rb_cObject, rb_intern("Float"));
  rb_f_float = get_global_func("Float");

  rb_f_block_given_p = get_global_func("block_given?");

  rb_ary_join_m = get_instance_method(rb_cArray, "join");

  rb_str_plus = get_instance_method(rb_cString, "+");
  rb_str_concat = get_instance_method(rb_cString, "concat");

  rb_define_global_function("load_so", load_so, 2);

  return 1;
}
