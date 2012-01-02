#include "load_so.h"
#include "st.h"

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

void Init_Hash() {
  ptr_objhash = RHASH(rb_class_new_instance(0, NULL, rb_cHash))->ntbl->type;
}
