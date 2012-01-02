#include "load_so.h"

VALUE (*fix_and)(VALUE, VALUE);
VALUE (*rb_fix_lshift)(VALUE, VALUE);
VALUE (*rb_f_integer)(int, VALUE*, VALUE);
VALUE (*rb_big_to_s)(int, VALUE*, VALUE);
VALUE (*rb_f_float)(VALUE, VALUE);

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

void Init_Numeric() {
  rb_f_float = get_global_func("Float");
  rb_fix_lshift = get_instance_method(rb_cFixnum, "<<");
  fix_and = get_instance_method(rb_cFixnum, "&");
  rb_f_integer = get_global_func("Integer");
  rb_big_to_s = get_instance_method(rb_cBignum, "to_s");
}
