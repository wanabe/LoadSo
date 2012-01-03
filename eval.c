#include <stdio.h>
#include "load_so.h"

static VALUE (*rb_f_eval)(int, VALUE*, VALUE);
static VALUE (*rb_f_raise)(int, VALUE*);
static VALUE (*rb_proc_s_new)(int, VALUE*, VALUE);
static VALUE (*proc_call)(int, VALUE*, VALUE);
static VALUE (*rb_f_block_given_p)();
static VALUE (*rb_f_public_send)(int argc, VALUE *argv, VALUE recv);
static VALUE (*rb_f_p)(int, VALUE*, VALUE);
static VALUE (*umethod_bind)(VALUE, VALUE);
static VALUE (*rb_method_call)(int, const VALUE*, VALUE);

void rb_p(VALUE obj) {
  rb_f_p(1, &obj, Qnil);
}

void rb_raise(VALUE exc, const char *format,...) {
  VALUE v[2] = {exc};
  va_list ap;

  va_start(ap, format);
  v[1] = rb_vsprintf(format, ap);
  va_end(ap);
  rb_f_raise(2, v);
}

void rb_check_type(VALUE x, int t) {
  int xt;

  xt = TYPE(x);
  if (xt != t || (xt == T_DATA && RTYPEDDATA_P(x))) {
    rb_raise(rb_eTypeError, "wrong argument type");
  }
}

VALUE rb_eval_string(const char *str) {
  set_buf_string(str);
  return rb_f_eval(1, &value_buf_string, Qnil);
}

void rb_set_end_proc(void (*func)(VALUE), VALUE data) {
  fprintf(stderr, "TODO: LoadSo can't set rb_set_end_proc\n");
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

VALUE rb_funcall(VALUE recv, ID mid, int n, ...) {
  va_list ar;
  int i;
  VALUE ary = INT2FIX(n + 2), *ptr;

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  ptr = RARRAY_PTR(ary);
  ptr[0] = ID2SYM(rb_intern("__send__"));
  ptr[1] = ID2SYM(mid);
  va_start(ar, n);
  for (i = 2; i < n + 2; i++) {
    ptr[i] = va_arg(ar, VALUE);
  }
  va_end(ar);
  return rb_f_public_send(n + 2, ptr, recv);
}

VALUE rb_funcall3(VALUE recv, ID mid, int argc, const VALUE *argv) {
  VALUE ary = INT2FIX(argc + 1), *ptr;

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  ptr = RARRAY_PTR(ary);
  ptr[0] = ID2SYM(mid);
  memcpy(ptr + 1, argv, sizeof(VALUE) * argc);
  return rb_f_public_send(argc + 1, ptr, recv);
}

VALUE rb_call_super(int argc, const VALUE *argv) {
  rb_thread_t *th = GET_THREAD();
  ID id = th->cfp->me->def->original_id;
  VALUE method, klass = RCLASS_SUPER(th->cfp->me->klass);

  if (klass == 0) {
    rb_raise(rb_eNotImpError, "TODO: super can't find superclass. under construction.");
  }
  method = rb_instance_method(klass, id);
  method = umethod_bind(method, th->cfp->self);
  return rb_method_call(argc, argv, method);
}

void Init_Eval() {
  rb_f_eval = get_global_func("eval");
  rb_f_raise = get_global_func("raise");
  rb_proc_s_new = get_method(rb_cProc, "new");
  proc_call = get_instance_method(rb_cProc, "call");
  rb_f_block_given_p = get_global_func("block_given?");
  rb_f_public_send = get_instance_method(rb_cObject, "public_send");
  umethod_bind = get_instance_method(rb_cUnboundMethod, "bind");
  rb_method_call = get_instance_method(rb_cMethod, "call");
  rb_f_p = get_global_func("p");
}
