#include "load_so.h"
#include "st.h"

static VALUE (*rb_hash_aref_)(VALUE, VALUE);
static VALUE (*rb_hash_aset_)(VALUE, VALUE, VALUE);

const struct st_hash_type *ptr_objhash;

VALUE rb_hash_new() {
  return rb_class_new_instance(0, NULL, rb_cHash);
}

struct st_table *rb_hash_tbl(VALUE hash) {
  if (!RHASH(hash)->ntbl) {
    RHASH(hash)->ntbl = st_init_table(ptr_objhash);
  }
  return RHASH(hash)->ntbl;
}

VALUE rb_hash_aref(VALUE hash, VALUE key) {
  return rb_hash_aref_(hash, key);
}

VALUE rb_hash_aset(VALUE hash, VALUE key, VALUE val) {
  return rb_hash_aset_(hash, key, val);
}

void Init_Hash() {
  ptr_objhash = RHASH(rb_class_new_instance(0, NULL, rb_cHash))->ntbl->type;
  rb_hash_aref_ = get_instance_method(rb_cHash, "[]");
  rb_hash_aset_ = get_instance_method(rb_cHash, "[]=");
}
