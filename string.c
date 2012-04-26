#include "load_so.h"
#include <stdio.h>

static VALUE (*rb_str_intern_)(VALUE);
static VALUE (*rb_str_plus)(VALUE, VALUE);
static VALUE (*rb_str_concat_)(VALUE, VALUE);
static VALUE (*rb_str_cmp_m)(VALUE, VALUE);
static VALUE (*rb_f_string)(VALUE, VALUE);
static VALUE (*rb_sym_to_s_)(VALUE);
static VALUE (*get_default_internal)(VALUE);
static VALUE (*get_default_external)(VALUE);
static VALUE (*enc_replicate)(VALUE, VALUE);
static VALUE (*enc_list)(VALUE);
static VALUE (*enc_find)(VALUE, VALUE);
static VALUE (*rb_obj_encoding)(VALUE obj);
static VALUE (*rb_str_force_encoding)(VALUE, VALUE);

VALUE rb_str_cat(VALUE str, const char *ptr, long len) {
  buf_string.as.heap.ptr = (char*)ptr;
  buf_string.as.heap.len = len;
  str = rb_str_concat_(str, value_buf_string);
  return str;
}

VALUE rb_str_cat2(VALUE str, const char *ptr) {
  set_buf_string((char*)ptr);
  str = rb_str_concat_(str, value_buf_string);
  return str;
}

VALUE
rb_str_buf_cat(VALUE str, const char *ptr, long len)
{
    return rb_str_cat(str, ptr, len);
}

VALUE
rb_str_buf_cat2(VALUE str, const char *ptr)
{
    return rb_str_buf_cat(str, ptr, strlen(ptr));
}

VALUE rb_str_new(char *ptr, long len) {
  VALUE val;
  if (ptr) {
    return rb_str_cat(rb_class_new_instance(0, NULL, rb_cString), ptr, len);
  }
  ptr = calloc(len, 1);
  val = rb_str_cat(rb_class_new_instance(0, NULL, rb_cString), ptr, len);
  free(ptr);
  return val;
}

VALUE rb_str_new_cstr(const char *ptr) {
  VALUE str = rb_class_new_instance(0, NULL, rb_cString);
  str = rb_str_concat_(str, set_buf_string(ptr));
  return str;
}

#define STR_BUF_MIN_SIZE 128

VALUE
rb_str_buf_new(long capa)
{
    VALUE str = rb_class_new_instance(0, NULL, rb_cString);

    if (capa < STR_BUF_MIN_SIZE) {
	capa = STR_BUF_MIN_SIZE;
    }
    FL_SET(str, RSTRING_NOEMBED);
    RSTRING(str)->as.heap.aux.capa = capa;
    RSTRING(str)->as.heap.ptr = ALLOC_N(char, capa+1);
    RSTRING(str)->as.heap.ptr[0] = '\0';

    return str;
}

int ruby_snprintf(char *str, size_t n, char const *fmt, ...) {
  va_list ap;
  int ret;

  va_start(ap, fmt);
  ret = vsnprintf(str, n, fmt, ap);
  va_end(ap);
  return ret;
}

int ruby_vsnprintf(char *str, size_t n, const char *fmt, va_list ap) {
  return vsnprintf(str, n, fmt, ap);
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

int rb_str_cmp(VALUE str1, VALUE str2) {
  return FIX2LONG(rb_str_cmp_m(str1, str2));
}

VALUE rb_str_append(VALUE str, VALUE str2) {
  return rb_str_concat_(str, str2);
}

VALUE rb_str_buf_append(VALUE str, VALUE str2) {
  return rb_str_concat_(str, str2); /* TODO: correct? */
}

VALUE rb_str_concat(VALUE str, VALUE str2) {
  return rb_str_concat_(str, str2);
}

VALUE rb_String(VALUE val) {
  return rb_f_string(rb_cString, val);
}

VALUE rb_str_intern(VALUE str) {
  return rb_str_intern_(str);
}

ID rb_intern2(const char *name, long len) {
  return SYM2ID(rb_str_intern_(set_buf_string2(name, len)));
}

ID rb_intern(const char *name) {
  return SYM2ID(rb_str_intern_(set_buf_string(name)));
}

ID rb_to_id(VALUE name) {
  switch (TYPE(name)) {
  default:
    rb_raise(rb_eNotImpError, "TODO: rb_to_id(not_str_or_sym) is not implemented yet.");
  case T_STRING:
    name = rb_str_intern_(name);
    /* fall through */
  case T_SYMBOL:
    return SYM2ID(name);
  }
  return Qnil; /* not reached */
}

VALUE rb_sym_to_s(VALUE sym) {
  return rb_sym_to_s(sym);
}

VALUE rb_id2str(ID id) {
  return rb_sym_to_s_(ID2SYM(id));
}

const char *rb_id2name(ID id) {
  VALUE str = rb_id2str(id);

  if (!str) return 0;
  return RSTRING_PTR(str);
}

static int enc_check_encoding(VALUE obj) {
  if (SPECIAL_CONST_P(obj) || CLASS_OF(obj) != rb_cEncoding) {
    return -1;
  }
  return 0;
}

rb_encoding *rb_to_encoding(VALUE enc) {
  if (enc_check_encoding(enc) >= 0) return RDATA(enc)->data;
  rb_raise(rb_eNotImpError, "TODO: rb_to_encoding(not_encoding) is not implemented yet.");
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

rb_encoding *rb_enc_get(VALUE obj) {
  return rb_to_encoding(rb_obj_encoding(obj));
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

VALUE rb_enc_str_new(const char *ptr, long len, rb_encoding *encoding) {
  VALUE str, enc, list = enc_list(rb_cEncoding);
  enc = RARRAY_PTR(list)[encoding->ruby_encoding_index];
  str = rb_str_new(ptr, len);
  rb_str_force_encoding(str, enc);
  return str;
}

void Init_StringCore() {
  buf_string.basic.klass = rb_cString;
  rb_str_intern_ = get_instance_method(rb_cString, "intern");
}

void Init_String() {
  rb_str_plus = get_instance_method(rb_cString, "+");
  rb_str_concat_ = get_instance_method(rb_cString, "concat");
  rb_str_cmp_m = get_instance_method(rb_cString, "<=>");
  rb_f_string = get_global_func("String");

  rb_sym_to_s_ = get_instance_method(rb_cSymbol, "to_s");

  get_default_internal = get_method(rb_cEncoding, "default_internal");
  get_default_external = get_method(rb_cEncoding, "default_external");
  enc_replicate = get_instance_method(rb_cEncoding, "replicate");
  enc_list = get_method(rb_cEncoding, "list");
  enc_find = get_method(rb_cEncoding, "find");
  rb_obj_encoding = get_instance_method(rb_cString, "encoding");
  rb_str_force_encoding = get_instance_method(rb_cString, "force_encoding");
}
