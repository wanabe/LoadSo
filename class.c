#include <stdarg.h>
#define va_init_list(a,b) va_start((a),(b))
#include "load_so.h"
#include "st.h"

static VALUE dummy_proc;

static VALUE (*rb_obj_method)(VALUE, VALUE);
static VALUE (*rb_mod_define_method)(int, VALUE*, VALUE);
static VALUE (*rb_mod_instance_method)(VALUE, VALUE);
static VALUE (*rb_mod_public)(int, VALUE*, VALUE);
static VALUE (*rb_mod_private)(int, VALUE*, VALUE);
static VALUE (*rb_mod_append_features)(VALUE, VALUE);
static VALUE (*rb_mod_name)(VALUE);
static VALUE (*rb_mod_alias_method)(VALUE, VALUE, VALUE);
static VALUE (*rb_class_new_instance_)(int, VALUE*, VALUE);
static VALUE (*rb_obj_singleton_class)(VALUE);
static VALUE (*rb_obj_is_kind_of_)(VALUE, VALUE);
static VALUE (*obj_respond_to)(int, VALUE*, VALUE);
static VALUE (*rb_obj_class_)(VALUE);
static VALUE (*rb_obj_clone_)(VALUE);

VALUE rb_class_new_instance(int argc, VALUE *argv, VALUE klass) {
  return rb_class_new_instance_(argc, argv, klass);
}

static inline cfunc get_method_with_func(cfunc func, VALUE obj, char *name) {
  VALUE vmethod;
  struct METHOD *method;

  vmethod = func(obj, set_buf_string(name));
  method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);
  return method->me.def->body.cfunc.func;
}

cfunc get_method(VALUE obj, char *name) {
  return get_method_with_func(rb_obj_method, obj, name);
}

cfunc get_global_func(char *name) {
  return get_method_with_func(rb_obj_method, Qnil, name);
}

cfunc get_instance_method(VALUE mod, char *name) {
  return get_method_with_func(rb_mod_instance_method, mod, name);
}

VALUE rb_instance_method(VALUE mod, ID id) {
  return rb_mod_instance_method(mod, ID2SYM(id));
}

int rb_scan_args(int argc, const VALUE *argv, const char *fmt, ...) {
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

VALUE rb_define_class_under(VALUE outer, const char *name, VALUE super) {
  VALUE mod = rb_class_new_instance(1, &super, rb_cClass);
  rb_const_set(outer, rb_intern(name), mod);
  return mod;
}

VALUE rb_define_class(const char *name, VALUE super) {
  return rb_define_class_under(rb_cObject, name, super);
}

VALUE rb_define_module_under(VALUE outer, const char *name) {
  VALUE mod = rb_class_new_instance(0, NULL, rb_cModule);
  rb_const_set(outer, rb_intern(name), mod);
  return mod;
}

VALUE rb_define_module(const char *name) {
  return rb_define_module_under(rb_cObject, name);
}

void rb_include_module(VALUE klass, VALUE module) {
  rb_mod_append_features(module, klass);
}

const char *rb_obj_classname(VALUE obj) {
  /* TODO: rb_obj_classname is not equal rb_mod_name exactly. */
  VALUE name = rb_mod_name(CLASS_OF(obj));
  /* TODO: Is GC safe? */
  return RSTRING_PTR(name);
}

void rb_define_alias(VALUE klass, const char *name1, const char *name2) {
  rb_mod_alias_method(klass, ID2SYM(rb_intern(name1)), ID2SYM(rb_intern(name2)));
}

VALUE rb_singleton_class(VALUE obj) {
  return rb_obj_singleton_class(obj);
}

void rb_define_alloc_func(VALUE klass, VALUE (*func)(VALUE)) {
  rb_method_definition_t *def = ALLOC(rb_method_definition_t);
  st_table *mtbl;

  klass = rb_singleton_class(klass);
  mtbl = RCLASS_M_TBL(klass);
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

VALUE rb_obj_is_kind_of(VALUE obj, VALUE c) {
  VALUE ret = rb_obj_is_kind_of_(obj, c);
  return ret;
}

int rb_respond_to(VALUE obj, ID id) {
  VALUE sym = ID2SYM(id);
  return obj_respond_to(1, &sym, obj);
}

VALUE rb_obj_class(VALUE obj) {
  return rb_obj_class_(obj);
}

VALUE rb_obj_clone(VALUE obj) {
  return rb_obj_clone_(obj);
}

VALUE rb_obj_alloc(VALUE klass) {
  return rb_funcall(klass, ID_ALLOCATOR, 0, 0); /* TODO: check obj/klass */
}

void Init_ClassCore(VALUE vmethod) {
  struct METHOD *method = (struct METHOD*)RTYPEDDATA_DATA(vmethod);

  rb_obj_method = method->me.def->body.cfunc.func;
  rb_mod_instance_method = get_method(rb_cObject, "instance_method");
}

void Init_Class() {
  dummy_proc = rb_eval_string("$__loadso__dummy_proc = proc{}");
  rb_mod_define_method = get_method(rb_cObject, "define_method");
  rb_mod_public = get_method(rb_cObject, "public");
  rb_mod_private = get_method(rb_cObject, "private");
  rb_obj_singleton_class = get_global_func("singleton_class");
  rb_obj_is_kind_of_ = get_global_func("kind_of?");
  rb_mod_append_features = get_instance_method(rb_cModule, "append_features");
  rb_mod_name = get_instance_method(rb_cModule, "name");
  rb_mod_alias_method = get_instance_method(rb_cModule, "alias_method");
  rb_class_new_instance_ = get_method(rb_cObject, "new");
  obj_respond_to = get_global_func("respond_to?");
  rb_obj_class_ = get_instance_method(rb_mKernel, "class");
  rb_obj_clone_ = get_instance_method(rb_mKernel, "clone");
}
