#include "load_so.h"
#include <stdio.h>

#define LLONG_MIN_MINUS_ONE ((double)LLONG_MIN-1)
#define LLONG_MAX_PLUS_ONE (2*(double)(LLONG_MAX/2+1))
#define ULLONG_MAX_PLUS_ONE (2*(double)(ULLONG_MAX/2+1))

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

double rb_big2dbl(VALUE x) {
  x = rb_funcall(x, rb_intern("to_s"), 0);
  return atof(RSTRING_PTR(x));
}

VALUE rb_fix2str(VALUE x, int base) {
  rb_raise(rb_eNotImpError, "TODO: rb_fix2str is not implemented yet.");
  return 0;
}

#define LONG_LONG long long

LONG_LONG rb_big2ll(VALUE x) {
  x = rb_funcall(x, rb_intern("to_s"), 0);
  return atoll(RSTRING_PTR(x));
}

unsigned LONG_LONG rb_big2ull(VALUE x) {
  x = rb_funcall(x, rb_intern("to_s"), 0);
  return strtoull(RSTRING_PTR(x), NULL, 10);
}

LONG_LONG rb_num2ll(VALUE val) {
  if (FIXNUM_P(val)) return (LONG_LONG)FIX2LONG(val);
  switch (TYPE(val)) {
  case T_FLOAT:
    if (RFLOAT_VALUE(val) < LLONG_MAX_PLUS_ONE
        && RFLOAT_VALUE(val) > LLONG_MIN_MINUS_ONE) {
      return (LONG_LONG)(RFLOAT_VALUE(val));
    } else {
      rb_raise(rb_eRangeError, "out of range");
    }
  case T_BIGNUM:
    return rb_big2ll(val);
  }
  rb_raise(rb_eNotImpError, "TODO: rb_num2ll(not_int_or_float) is under construction.");
  return Qnil; /* not reached */
}

unsigned LONG_LONG rb_num2ull(VALUE val) {
  if (TYPE(val) == T_BIGNUM) {
    return rb_big2ull(val);
  }
  return (unsigned LONG_LONG)rb_num2ll(val);
}

unsigned long rb_num2ulong(VALUE val) {
  if (TYPE(val) == T_BIGNUM) {
    val = rb_funcall(val, rb_intern("to_s"), 0);
    return strtoul(RSTRING_PTR(val), NULL, 0);
  }
  if (FIXNUM_P(val)) return (unsigned long)FIX2LONG(val);
  rb_raise(rb_eNotImpError, "TODO: rb_num2ulong(not_int) is under construction.");
  return 0;
}

VALUE rb_ull2big(unsigned LONG_LONG n) {
  char buf[20 + 1]; /* Math.log10(1<<64) < 20 */
  snprintf(buf, sizeof(buf), "%llu", n);
  return rb_eval_string(buf);
}

VALUE rb_ll2big(LONG_LONG n) {
  char buf[20 + 1]; /* Math.log10(1<<64) < 20 */
  snprintf(buf, sizeof(buf), "%lld", n);
  return rb_eval_string(buf);
}

VALUE rb_ull2inum(unsigned LONG_LONG n) {
  if (POSFIXABLE(n)) return INT2FIX(n);
  return rb_ull2big(n);
}

VALUE rb_ll2inum(LONG_LONG n) {
  if (FIXABLE(n)) return INT2FIX(n);
  return rb_ll2big(n);
}

void Init_Numeric() {
  rb_f_float = get_global_func("Float");
  rb_fix_lshift = get_instance_method(rb_cFixnum, "<<");
  fix_and = get_instance_method(rb_cFixnum, "&");
  rb_f_integer = get_global_func("Integer");
  rb_big_to_s = get_instance_method(rb_cBignum, "to_s");
}
