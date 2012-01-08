#include <stdio.h>
#include "load_so.h"
#include "st.h"
#include "eval.h"

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
#ifdef CREATE_CONSOLE
  VALUE v = ID2SYM(rb_intern("inspect"));
  printf("%s\n", RSTRING_PTR(rb_f_public_send(1, &v, obj)));
  fflush(stdout);
#else
  rb_f_p(1, &obj, Qnil);
#endif
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
  rb_thread_t *th = GET_THREAD();

  if ((th->cfp->lfp[0] & 0x02) == 0 &&
      GC_GUARDED_PTR_REF(th->cfp->lfp[0])) {
    return TRUE;
  }
  else {
    return FALSE;
  }
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

VALUE rb_apply(VALUE recv, ID mid, VALUE args) {
  rb_raise(rb_eNotImpError, "TODO: rb_apply is not implemented yet.");
  return Qnil;
}

void rb_backtrace() {
  fprintf(stderr, "TODO: rb_backtrace is not implemented yet.");
}

VALUE rb_block_call(VALUE obj, ID mid, int argc, VALUE * argv, VALUE (*bl_proc) (ANYARGS), VALUE data2) {
  rb_raise(rb_eNotImpError, "TODO: rb_block_call is not implemented yet.");
  return Qnil;
}

VALUE rb_errinfo(void) {
  rb_thread_t *th = GET_THREAD();
  return th->errinfo;
}

static ID frame_func_id(rb_control_frame_t *cfp) {
  if (cfp->me) {
    return cfp->me->def->original_id;
  }
  rb_raise(rb_eNotImpError, "TODO: frame_func_id is under construction.");
  return 0;
}
ID rb_frame_this_func(void) {
  return frame_func_id(GET_THREAD()->cfp);
}

VALUE rb_funcall2(VALUE recv, ID mid, int argc, const VALUE *argv) {
  rb_raise(rb_eNotImpError, "TODO: rb_funcall2 is not implemented yet.");
  return Qnil;
}

VALUE rb_ensure(VALUE (*b_proc)(ANYARGS), VALUE data1, VALUE (*e_proc)(ANYARGS), VALUE data2) {
  int state;
  volatile VALUE result = Qnil;

  PUSH_TAG();
  if ((state = EXEC_TAG()) == 0) {
    result = (*b_proc) (data1);
  }
  POP_TAG();
  /* TODO: fix me */
  /* retval = prot_tag ? prot_tag->retval : Qnil; */     /* save retval */
  (*e_proc) (data2);
  fflush(stdout);
  if (state)
    JUMP_TAG(state);
  return result;
}

void rb_fiber_start(void) {
  fprintf(stderr, "TODO: rb_fiber_start is under construction\n");
}

VALUE rb_protect(VALUE (* proc) (VALUE), VALUE data, int * state) {
    volatile VALUE result = Qnil;
    int status;
    rb_thread_t *th = GET_THREAD();
    rb_control_frame_t *cfp = th->cfp;
    struct rb_vm_protect_tag protect_tag;
    rb_jmpbuf_t org_jmpbuf;

    protect_tag.prev = th->protect_tag;

    PUSH_TAG();
    th->protect_tag = &protect_tag;
    MEMCPY(&org_jmpbuf, &(th)->root_jmpbuf, rb_jmpbuf_t, 1);
    if ((status = EXEC_TAG()) == 0) {
	SAVE_ROOT_JMPBUF(th, result = (*proc) (data));
    }
    MEMCPY(&(th)->root_jmpbuf, &org_jmpbuf, rb_jmpbuf_t, 1);
    th->protect_tag = protect_tag.prev;
    POP_TAG();

    if (state) {
	*state = status;
    }
    if (status != 0) {
	th->cfp = cfp;
	return Qnil;
    }

    return result;
}

#include <windows.h>
void rb_write_error(const char *mesg) {
  fprintf(stderr, "%s", mesg);
#ifdef CREATE_CONSOLE
  MessageBox(0, mesg, "error", 0);
#endif
}

void ruby_finalize(void) {
  fprintf(stderr, "TODO: ruby_finalize is not implemented yet.");
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
