#include "load_so.h"

#define GetThreadPtr(obj, ptr) \
    TypedData_Get_Struct((obj), rb_thread_t, ptr_ruby_thread_data_type, (ptr))

static VALUE (*thread_s_current)(VALUE);
static const rb_data_type_t *ptr_ruby_thread_data_type;

VALUE rb_thread_current() {
  return thread_s_current(rb_cThread);
}

rb_thread_t *get_thread(VALUE thread) {
  rb_thread_t *th;
  GetThreadPtr(thread, th);
  return th;
}

int rb_safe_level() {
  return GET_THREAD()->safe_level;
}

void rb_secure(int level) {
  if (level <= rb_safe_level()) {
    rb_raise(rb_eSecurityError, "Insecure operation at level %d",
             rb_safe_level());
  }
}

void rb_insecure_operation(void) {
  rb_raise(rb_eSecurityError, "Insecure operation");
}

void rb_check_safe_obj(VALUE x) {
  if (rb_safe_level() > 0 && OBJ_TAINTED(x)) {
    rb_insecure_operation();
  }
  rb_secure(4);
}

void Init_Thread() {
  thread_s_current = get_method(rb_cThread, "current");
  ptr_ruby_thread_data_type = RTYPEDDATA_TYPE(rb_thread_current());
}
