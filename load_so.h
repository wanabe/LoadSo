#include <windows.h>
#include "rgss.h"

#ifndef DLL_NAME
#define DLL_NAME "msvcrt-ruby191.dll"
#endif

typedef VALUE (*cfunc)(ANYARGS);

extern struct RString buf_string;
extern VALUE value_buf_string;

VALUE set_buf_string(const char*);
VALUE set_buf_string2(const char*, long);
cfunc get_method(VALUE, char*);
cfunc get_global_func(char*);
cfunc get_instance_method(VALUE, char*);
rb_thread_t *get_thread(VALUE);
VALUE rb_instance_method(VALUE, ID);

VALUE rb_thread_current();
#define GET_THREAD() get_thread(rb_thread_current())
