#include "load_so.h"

static VALUE (*rb_ary_push_m)(int, VALUE*, VALUE);
static VALUE (*rb_ary_join_m)(int, VALUE*, VALUE);
static VALUE (*rb_ary_clear_)(VALUE);
static VALUE (*rb_ary_concat_)(VALUE, VALUE);
static VALUE (*rb_ary_unshift_m)(int, VALUE*, VALUE);
static VALUE (*rb_ary_delete_at_m)(VALUE, VALUE);

VALUE rb_ary_new() {
  return rb_class_new_instance(0, NULL, rb_cArray);
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

VALUE rb_ary_new4(long n, const VALUE *elts) {
  VALUE ary = INT2FIX(n);

  ary = rb_class_new_instance(1, &ary, rb_cArray);
  if (n > 0 && elts) {
    memcpy(RARRAY_PTR(ary), elts, sizeof(VALUE) * n);
  }
  return ary;
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

VALUE rb_ary_push(VALUE ary, VALUE item) {
  return rb_ary_push_m(1, &item, ary);
}

VALUE rb_ary_join(VALUE ary, VALUE sep) {
  rb_ary_join_m(1, &sep, ary);
  return rb_ary_join_m(1, &sep, ary);
}

VALUE rb_ary_clear(VALUE ary) {
  return rb_ary_clear_(ary);
}

VALUE rb_ary_concat(VALUE x, VALUE y) {
  return rb_ary_concat_(x, y);
}

VALUE rb_ary_unshift(VALUE ary, VALUE item) {
  return rb_ary_unshift_m(1, &item, ary);
}

VALUE rb_ary_delete_at(VALUE ary, long pos) {
  return rb_ary_delete_at_m(ary, INT2FIX(pos));
}

void rb_ary_store(VALUE ary, long idx, VALUE val) {
  rb_raise(rb_eNotImpError, "TODO: rb_ary_store is not implemented yet.");
}

void Init_Array() {
  rb_ary_push_m = get_instance_method(rb_cArray, "push");
  rb_ary_join_m = get_instance_method(rb_cArray, "join");
  rb_ary_clear_ = get_instance_method(rb_cArray, "clear");
  rb_ary_concat_ = get_instance_method(rb_cArray, "concat");
  rb_ary_unshift_m = get_instance_method(rb_cArray, "unshift");
  rb_ary_delete_at_m = get_instance_method(rb_cArray, "delete_at");
}
