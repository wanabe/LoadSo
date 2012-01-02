#include <stdio.h>
#include <stdarg.h>
#define va_init_list(a,b) va_start((a),(b))
#include "load_so.h"
#include "st.h"

VALUE (*rb_f_raise)(int, VALUE*);
VALUE (*rb_f_p)(int, VALUE*, VALUE);
VALUE (*rb_f_eval)(int, VALUE*, VALUE);
VALUE (*rb_proc_s_new)(int, VALUE*, VALUE);
VALUE (*proc_call)(int, VALUE*, VALUE);
VALUE (*rb_f_block_given_p)();
VALUE (*rb_f_float)(VALUE, VALUE);
VALUE (*rb_ary_push_m)(int, VALUE*, VALUE);
VALUE (*rb_ary_join_m)(int, VALUE*, VALUE);
VALUE (*rb_f_public_send)(int argc, VALUE *argv, VALUE recv);
VALUE (*rb_fix_lshift)(VALUE, VALUE);
VALUE (*fix_and)(VALUE, VALUE);
VALUE (*rb_f_integer)(int, VALUE*, VALUE);
VALUE (*rb_big_to_s)(int, VALUE*, VALUE);
VALUE (*rb_gc_start)();

struct RString buf_string = {{0x2005, 0}};
VALUE value_buf_string = (VALUE)&buf_string;
const struct st_hash_type *ptr_objhash;

typedef VALUE (*cfunc)(ANYARGS);

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

void rb_p(VALUE obj) {
  rb_f_p(1, &obj, Qnil);
}

void rb_raise(VALUE exc, const char *msg,...) {
  VALUE v[2] = {exc, value_buf_string};

  /* TODO: va_args */
  set_buf_string(msg);
  rb_f_raise(2, v);
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
    rb_raise(rb_eLoadError, "Can't load: %s", RSTRING_PTR(file));
  }
  init_func = (void(*)())GetProcAddress(hSo, RSTRING_PTR(init_name));
  if(!init_func) {
    rb_raise(rb_eLoadError, "Can't Init: %s", RSTRING_PTR(file));
  }
  init_func();
  return Qnil;
}

VALUE rb_eval_string(const char *str) {
  set_buf_string(str);
  return rb_f_eval(1, &value_buf_string, Qnil);
}

void rb_set_end_proc(void (*func)(VALUE), VALUE data) {
  fprintf(stderr, "TODO: LoadSo can't set rb_set_end_proc\n");
}

VALUE rb_ary_push(VALUE ary, VALUE item) {
  return rb_ary_push_m(1, &item, ary);
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

VALUE rb_ary_join(VALUE ary, VALUE sep) {
  rb_ary_join_m(1, &sep, ary);
  return rb_ary_join_m(1, &sep, ary);
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

VALUE rb_hash_new() {
  return rb_class_new_instance(0, NULL, rb_cHash);
}

struct st_table *rb_hash_tbl(VALUE hash) {
  if (!RHASH(hash)->ntbl) {
    RHASH(hash)->ntbl = st_init_table(ptr_objhash);
  }
  return RHASH(hash)->ntbl;
}

VALUE rb_funcall(VALUE recv, ID mid, int n, ...) {
  va_list ar;
  int i;
  VALUE ary = INT2FIX(n + 1), *ptr;

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  ptr = RARRAY_PTR(ary);
  ptr[0] = ID2SYM(mid);
  va_init_list(ar, n);
  for (i = 1; i <= n; i++) {
    ptr[i] = va_arg(ar, VALUE);
  }
  va_end(ar);
  return rb_f_public_send(n + 1, ptr, recv);
}

VALUE rb_funcall3(VALUE recv, ID mid, int argc, const VALUE *argv) {
  VALUE ary = INT2FIX(argc + 1), *ptr;

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  ptr = RARRAY_PTR(ary);
  ptr[0] = ID2SYM(mid);
  memcpy(ptr + 1, argv, sizeof(VALUE) * argc);
  return rb_f_public_send(argc + 1, ptr, recv);
}

static int mark_keyvalue(VALUE key, VALUE value, st_data_t data) {
  rb_gc_mark(key);
  rb_gc_mark(value);
  return ST_CONTINUE;
}

void rb_mark_hash(st_table *tbl) {
  if (!tbl) return;
  st_foreach(tbl, mark_keyvalue, 0);
}

VALUE rb_int2big(SIGNED_VALUE n) {
  VALUE ret = INT2FIX(RSHIFT(n, 2));

  ret = rb_fix_lshift(ret, INT2FIX(2));
  return fix_and(INT2FIX(n & 3), ret);
}

SIGNED_VALUE rb_num2long(VALUE val) {
  SIGNED_VALUE ret;
  char *endptr;

  val = rb_f_integer(1, &val, Qnil);
  switch(TYPE(val)) {
  case T_FIXNUM:
    return FIX2LONG(val);
  case T_BIGNUM:
    val = rb_big_to_s(0, NULL, val);
    ret = strtol(RSTRING_PTR(val), &endptr, 10);
    if(!errno) {
      return ret;
    }
    /* pass */
  default:
    rb_raise(rb_eArgError, "invalid value for NUM2LONG()");
  }
  /* not reached */
  return 0;
}

void Init_ClassCore(VALUE);
void Init_VariableCore();
void Init_StringCore();
void Init_Class();
void Init_Variable();
void Init_String();

void Init_LoadSo(VALUE vmethod, VALUE cObject) {
  LoadLibrary(DLL_NAME); /* reference_count++ to keep static variables */
  rb_cObject = cObject;

  Init_ClassCore(vmethod);
  Init_VariableCore();
  Init_StringCore();

  rb_f_eval = get_global_func("eval");

  Init_Class();
  Init_Variable();
  Init_String();

  rb_f_p = get_global_func("p");

  rb_f_raise = get_global_func("raise");

  rb_ary_push_m = get_instance_method(rb_cArray, "push");

  rb_proc_s_new = get_method(rb_cProc, "new");
  proc_call = get_instance_method(rb_cProc, "call");

  rb_f_float = get_global_func("Float");

  rb_f_block_given_p = get_global_func("block_given?");

  rb_ary_join_m = get_instance_method(rb_cArray, "join");

  ptr_objhash = RHASH(rb_class_new_instance(0, NULL, rb_cHash))->ntbl->type;

  rb_f_public_send = get_instance_method(rb_cObject, "public_send");

  rb_fix_lshift = get_instance_method(rb_cFixnum, "<<");
  fix_and = get_instance_method(rb_cFixnum, "&");
  rb_f_integer = get_global_func("Integer");

  rb_big_to_s = get_instance_method(rb_cBignum, "to_s");

  rb_gc_start = get_method(rb_mGC, "start");

  rb_define_global_function("load_so", load_so, 2);
}
