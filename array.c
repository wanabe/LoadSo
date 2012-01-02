#include "load_so.h"

static VALUE (*rb_ary_push_m)(int, VALUE*, VALUE);
static VALUE (*rb_ary_join_m)(int, VALUE*, VALUE);

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

void Init_Array() {
  rb_ary_push_m = get_instance_method(rb_cArray, "push");
  rb_ary_join_m = get_instance_method(rb_cArray, "join");
}
