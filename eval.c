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

VALUE push_block() {
  rb_thread_t *th = GET_THREAD();
  VALUE block = th->cfp->lfp[0];
  th->cfp->lfp[0] = 0;
  return block;
}

void pop_block(VALUE block) {
  GET_THREAD()->cfp->lfp[0] = block;
}

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

VALUE rb_funcall2(VALUE recv, ID mid, int argc, const VALUE *argv) {
  VALUE ary = rb_ary_new_with_len(argc + 2);
  VALUE *ptr = RARRAY_PTR(ary);

  ptr[0] = ID2SYM(rb_intern("__send__"));
  ptr[1] = ID2SYM(mid);
  MEMCPY(ptr + 2, argv, VALUE, argc);
  return rb_f_public_send(argc + 2, ptr, recv);
}

VALUE rb_apply(VALUE recv, ID mid, VALUE args) {
  return rb_funcall2(recv, mid, RARRAY_LEN(args), RARRAY_PTR(args));
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

void rb_backtrace() {
  fprintf(stderr, "TODO: rb_backtrace is not implemented yet.");
}

NODE *rb_node_newnode(enum node_type type, VALUE a0, VALUE a1, VALUE a2){
  NODE *n = (NODE*)rb_newobj();

  n->flags |= T_NODE;
  nd_set_type(n, type);

  n->u1.value = a0;
  n->u2.value = a1;
  n->u3.value = a2;

  return n;
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

VALUE rb_iterate(VALUE (* it_proc) (VALUE), VALUE data1, VALUE (* bl_proc) (ANYARGS), VALUE data2) {
  int state;
  volatile VALUE retval = Qnil;
  rb_thread_t *th = GET_THREAD();
  rb_control_frame_t *volatile cfp = th->cfp;

  NODE *node = NEW_IFUNC(bl_proc, data2);
  node->nd_aid = rb_frame_this_func();
  TH_PUSH_TAG(th);
  state = TH_EXEC_TAG();
  if (state == 0) {
  iter_retry:
    {
      rb_block_t *blockptr;
      if (bl_proc) {
        blockptr = RUBY_VM_GET_BLOCK_PTR_IN_CFP(th->cfp);
        blockptr->iseq = (void *)node;
        blockptr->proc = 0;
      } else {
        blockptr = GC_GUARDED_PTR_REF(th->cfp->lfp[0]);
      }
      th->passed_block = blockptr;
    }
    retval = (*it_proc) (data1);
  } else {
    VALUE err = th->errinfo;
    if (state == TAG_BREAK) {
      VALUE *escape_dfp = GET_THROWOBJ_CATCH_POINT(err);
      VALUE *cdfp = cfp->dfp;

      if (cdfp == escape_dfp) {
        state = 0;
        th->state = 0;
        th->errinfo = Qnil;

        /* check skipped frame */
        while (th->cfp != cfp) {
          if (UNLIKELY(VM_FRAME_TYPE(th->cfp) == VM_FRAME_MAGIC_CFUNC)) {
            /* TODO: event hook
            const rb_method_entry_t *me = th->cfp->me;
            EXEC_EVENT_HOOK(th, RUBY_EVENT_C_RETURN, th->cfp->self, me->called_id, me->klass);
             */
          }

          th->cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(th->cfp);
        }
      } else{
        /* SDR(); printf("%p, %p\n", cdfp, escape_dfp); */
      }
    } else if (state == TAG_RETRY) {
      VALUE *escape_dfp = GET_THROWOBJ_CATCH_POINT(err);
      VALUE *cdfp = cfp->dfp;

      if (cdfp == escape_dfp) {
        state = 0;
        th->state = 0;
        th->errinfo = Qnil;
        th->cfp = cfp;
        goto iter_retry;
      }
    }
  }
  TH_POP_TAG();
  node->u1.id = 0; /* TODO: this line avoid GC's SEGV - but why? */

  switch (state) {
  case 0:
    break;
  default:
    TH_JUMP_TAG(th, state);
  }

  return retval;
}

struct iter_method_arg {
  VALUE obj;
  ID mid;
  int argc;
  VALUE *argv;
};

static VALUE iterate_method(VALUE obj) {
  const struct iter_method_arg * arg = (struct iter_method_arg *) obj;

  return rb_funcall3(arg->obj, arg->mid, arg->argc, arg->argv);
}

VALUE rb_block_call(VALUE obj, ID mid, int argc, VALUE * argv, VALUE (*bl_proc) (ANYARGS), VALUE data2) {
  struct iter_method_arg arg;

  arg.obj = obj;
  arg.mid = mid;
  arg.argc = argc;
  arg.argv = argv;
  return rb_iterate(iterate_method, (VALUE)&arg, bl_proc, data2);
}

VALUE rb_errinfo(void) {
  rb_thread_t *th = GET_THREAD();
  return th->errinfo;
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

VALUE rb_require(const char *fname) {
  return rb_funcall(rb_mKernel, rb_intern("require"), 1, rb_str_new_cstr(fname));
}

void rb_check_frozen(VALUE obj) {
  if (OBJ_FROZEN(obj)) {
    rb_raise(rb_eRuntimeError, "can't modify frozen %s", rb_obj_classname(obj));
  }
}

void
rb_obj_call_init(VALUE obj, int argc, VALUE *argv)
{
    PASS_PASSED_BLOCK();
    rb_funcall2(obj, rb_intern("initialize"), argc, argv);
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
