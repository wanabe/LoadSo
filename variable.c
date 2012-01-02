#include "load_so.h"

VALUE rb_cObject, rb_mKernel, rb_cModule, rb_cClass, rb_cArray, rb_cString, rb_cFloat, rb_cHash, rb_cProc, rb_cData;
VALUE rb_eRuntimeError, rb_eLoadError, rb_eTypeError, rb_eArgError, rb_eNotImpError;
VALUE rb_cFixnum, rb_cBignum, rb_cTrueClass, rb_cSymbol, rb_cNilClass, rb_cFalseClass, rb_cTime, rb_cEncoding;
VALUE rb_mGC;

static VALUE (*rb_mod_const_get)(int, VALUE*, VALUE);
static VALUE (*rb_mod_const_set)(VALUE, VALUE, VALUE);

VALUE rb_const_get(VALUE klass, ID id) {
  VALUE sym = ID2SYM(id);
  VALUE ret = rb_mod_const_get(1, &sym, klass);
  return ret;
}

static VALUE const_get_cstr(VALUE klass, const char *name) {
  set_buf_string(name);
  return rb_mod_const_get(1, &value_buf_string, klass);
}

void rb_const_set(VALUE klass, ID id, VALUE val) {
  VALUE sym = ID2SYM(id);
  rb_mod_const_set(klass, sym, val);
}

void rb_define_const(VALUE klass, const char *name, VALUE val) {
  set_buf_string(name);
  rb_mod_const_set(klass, value_buf_string, val);
}

void Init_VariableCore() {
  rb_mod_const_get = get_method(rb_cObject, "const_get");

  rb_mGC = const_get_cstr(rb_cObject, "GC");
  rb_mKernel = const_get_cstr(rb_cObject, "Kernel");

  rb_cNilClass = const_get_cstr(rb_cObject, "NilClass");
  rb_cTrueClass = const_get_cstr(rb_cObject, "TrueClass");
  rb_cFalseClass = const_get_cstr(rb_cObject, "FalseClass");
  rb_cSymbol = const_get_cstr(rb_cObject, "Symbol");
  rb_cClass = const_get_cstr(rb_cObject, "Class");
  rb_cModule = const_get_cstr(rb_cObject, "Module");
  rb_cFixnum = const_get_cstr(rb_cObject, "Fixnum");
  rb_cArray = const_get_cstr(rb_cObject, "Array");
  rb_cHash = const_get_cstr(rb_cObject, "Hash");
  rb_cString = const_get_cstr(rb_cObject, "String");
  rb_cProc = const_get_cstr(rb_cObject, "Proc");
  rb_cFloat = const_get_cstr(rb_cObject, "Float");
  rb_cBignum = const_get_cstr(rb_cObject, "Bignum");
  rb_cData = const_get_cstr(rb_cObject, "Data");
  rb_cTime = const_get_cstr(rb_cObject, "Time");
  rb_cEncoding = const_get_cstr(rb_cObject, "Encoding");

  rb_eRuntimeError = const_get_cstr(rb_cObject, "RuntimeError");
  rb_eLoadError = const_get_cstr(rb_cObject, "LoadError");
  rb_eTypeError = const_get_cstr(rb_cObject, "TypeError");
  rb_eArgError = const_get_cstr(rb_cObject, "ArgumentError");
  rb_eNotImpError = const_get_cstr(rb_cObject, "NotImplementedError");
}

void Init_Variable() {
  rb_mod_const_set = get_method(rb_cObject, "const_set");
}