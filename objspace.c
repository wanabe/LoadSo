#include "load_so.h"
#include "st.h"

static VALUE (*rb_gc_start)();

VALUE rb_newobj() {
  return rb_class_new_instance(0, NULL, rb_cObject);
}

VALUE rb_data_object_alloc(VALUE klass, void *datap, RUBY_DATA_FUNC dmark, RUBY_DATA_FUNC dfree) {
  NEWOBJ(data, struct RData);
  OBJSETUP(data, klass, T_DATA);
  data->data = datap;
  data->dfree = dfree;
  data->dmark = dmark;

  return (VALUE)data;
}

void rb_gc_mark(VALUE ptr) {
  register struct RBasic *obj;

  obj = RBASIC(ptr);
  if (SPECIAL_CONST_P(ptr)) return; /* special const not marked */
  if (obj->flags == 0) return;       /* free cell */
  if (obj->flags & FL_MARK) return;  /* already marked */
  obj->flags |= FL_MARK;

  /* TODO: objspace->heap.live_num should be incleased */
  /* TODO: check stack_overflow */

  /* TODO: mark ivar */
  switch (BUILTIN_TYPE(obj)) {
  case T_DATA:
    if (RTYPEDDATA_P(obj)) {
      RUBY_DATA_FUNC mark_func = RTYPEDDATA(obj)->type->function.dmark;
      if (mark_func) (*mark_func)(DATA_PTR(obj));
    } else {
      if (RDATA(obj)->dmark) (*RDATA(obj)->dmark)(DATA_PTR(obj));
    }
    break;
  default:
    rb_raise(rb_eNotImpError, "TODO: mark %s is not implemented yet", BUILTIN_TYPE(obj));
  }
}

static void gv_mark(void *ptr) {
  rb_gc_mark(*(VALUE*)ptr);
}

static void gv_free(void *ptr) {
}

void rb_global_variable(VALUE *var) {
  VALUE wrap, global_list = rb_eval_string("$__loadso__global_list");
  wrap = rb_data_object_alloc(0, var, gv_mark, gv_free);
  rb_ary_push(global_list, wrap);
}

void rb_gc_register_mark_object(VALUE obj) {
  VALUE global_list = rb_eval_string("$__loadso__global_list");
  rb_ary_push(global_list, obj);
}

void *ruby_xmalloc(size_t size) {
  return malloc(size);
}

void *ruby_xmalloc2(size_t n, size_t size) {
  return malloc(n * size);
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

void Init_ObjSpace() {
  rb_eval_string("$__loadso__global_list = []");
  rb_gc_start = get_method(rb_mGC, "start");
}
