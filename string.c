#include "load_so.h"
#include <stdio.h>

static VALUE (*rb_str_intern)(VALUE);
static VALUE (*rb_str_plus)(VALUE, VALUE);
static VALUE (*rb_str_concat)(VALUE, VALUE);
static VALUE (*get_default_internal)(VALUE);
static VALUE (*get_default_external)(VALUE);
static VALUE (*enc_replicate)(VALUE, VALUE);
static VALUE (*enc_list)(VALUE);
static VALUE (*enc_find)(VALUE, VALUE);

ID rb_intern2(const char *name, long len) {
  set_buf_string2(name, len);
  return SYM2ID(rb_str_intern(value_buf_string));
}

ID rb_intern(const char *name) {
  set_buf_string(name);
  return SYM2ID(rb_str_intern(value_buf_string));
}

VALUE rb_str_cat(VALUE str, const char *ptr, long len) {
  buf_string.as.heap.ptr = (char*)ptr;
  buf_string.as.heap.len = len;
  str = rb_str_concat(str, value_buf_string);
  return str;
}

VALUE rb_str_new(const char *ptr, long len) {
  return rb_str_cat(rb_class_new_instance(0, NULL, rb_cString), ptr, len);
}

VALUE rb_str_new_cstr(const char *ptr) {
  VALUE str = rb_class_new_instance(0, NULL, rb_cString);
  set_buf_string(ptr);
  str = rb_str_concat(str, value_buf_string);
  return str;
}

VALUE rb_vsprintf(const char *format, va_list ap) {
  VALUE result;
  char *buf;
  long len;

  len = vsnprintf(NULL, 0, format, ap);
  buf = (char*)malloc(len + 1);
  vsnprintf(buf, len, format, ap);
  result = rb_str_new(buf, len);
  free(buf);

  return result;
}

VALUE rb_sprintf(const char *format, ...) {
  VALUE result;
  va_list ap;

  va_start(ap, format);
  result = rb_vsprintf(format, ap);
  va_end(ap);
  return result;
}

VALUE rb_string_value(volatile VALUE *ptr) {
  if (TYPE(*ptr) != T_STRING) {
    *ptr = rb_funcall3(*ptr, rb_intern("to_str"), 0, NULL);
  }
  return *ptr;
}

char *rb_string_value_ptr(volatile VALUE *ptr) {
  VALUE str = rb_string_value(ptr);
  return RSTRING_PTR(str);
}

char *rb_string_value_cstr(volatile VALUE *ptr) {
  VALUE str = rb_string_value(ptr);
  char *s = RSTRING_PTR(str);
  long len = RSTRING_LEN(str);

  if (!s || memchr(s, 0, len)) {
    rb_raise(rb_eArgError, "string contains null byte");
  }
  if (s[len]) {
    rb_str_cat(str, "\0", 1);
    s = RSTRING_PTR(str);
  }
  return s;
}

static int enc_check_encoding(VALUE obj) {
  if (SPECIAL_CONST_P(obj) || CLASS_OF(obj) != rb_cEncoding) {
    return -1;
  }
  return 0;
}

rb_encoding *rb_to_encoding(VALUE enc) {
  if (enc_check_encoding(enc) >= 0) return RDATA(enc)->data;
  rb_raise(rb_eNotImpError, "rb_to_encoding(not_encoding) is not implemented yet.");
  return NULL;
}

rb_encoding *rb_default_internal_encoding() {
  VALUE val = get_default_internal(rb_cEncoding);
  if (NIL_P(val)) {
    return NULL;
  }
  return rb_to_encoding(val);
}

rb_encoding *rb_default_external_encoding() {
  VALUE val = get_default_external(rb_cEncoding);
  if (NIL_P(val)) {
    return NULL;
  }
  return rb_to_encoding(val);
}

rb_encoding *rb_enc_from_index(int index) {
  VALUE list = enc_list(rb_cEncoding);
  return rb_to_encoding(RARRAY_PTR(list)[index]);
}

int rb_enc_find_index(const char *name) {
  /* TODO: STUB */
  return -1;
}

VALUE rb_ascii8bit_encoding() {
  return enc_find(rb_cEncoding, set_buf_string("ascii-8bit"));
}

int rb_define_dummy_encoding(const char *name) {
  VALUE enc, list;
  enc = rb_ascii8bit_encoding();
  enc = enc_replicate(enc, set_buf_string(name));
  list = enc_list(rb_cEncoding);
  if(RARRAY_PTR(list)[RARRAY_LEN(list) - 1] != enc) {
    rb_raise(rb_eRuntimeError, "Can't define encoding");
  }
  return RARRAY_LEN(list) - 1;
}

void Init_StringCore() {
  buf_string.basic.klass = rb_cString;
  rb_str_intern = get_instance_method(rb_cString, "intern");
}

void Init_String() {
  rb_str_plus = get_instance_method(rb_cString, "+");
  rb_str_concat = get_instance_method(rb_cString, "concat");

  get_default_internal = get_method(rb_cEncoding, "default_internal");
  get_default_external = get_method(rb_cEncoding, "default_external");
  enc_replicate = get_instance_method(rb_cEncoding, "replicate");
  enc_list = get_method(rb_cEncoding, "list");
  enc_find = get_method(rb_cEncoding, "find");
}
