#include <setjmp.h>

typedef unsigned int rb_event_flag_t;
typedef void rb_event_hook_t; /* TODO */

#define SAVE_ROOT_JMPBUF(th, stmt) do \
  if (ruby_setjmp((th)->root_jmpbuf) == 0) { \
      stmt; \
  } \
  else { \
      rb_fiber_start(); \
  } while (0)
#define RUBY_SETJMP(env) setjmp(env)
#define RUBY_LONGJMP(env,val) longjmp(env,val)
#define ruby_setjmp(env) RUBY_SETJMP(env)
#define ruby_longjmp(env,val) RUBY_LONGJMP((env),(val))
#define RUBY_JMP_BUF jmp_buf
typedef RUBY_JMP_BUF rb_jmpbuf_t;
struct rb_vm_tag {
    rb_jmpbuf_t buf;
    VALUE tag;
    VALUE retval;
    struct rb_vm_tag *prev;
};
#define TH_PUSH_TAG(th) do { \
  rb_thread_t * const _th = (th); \
  struct rb_vm_tag _tag; \
  _tag.tag = 0; \
  _tag.prev = _th->tag; \
  _th->tag = &_tag;
#define TH_POP_TAG() \
  _th->tag = _tag.prev; \
} while (0)
#define TH_POP_TAG2() \
  _th->tag = _tag.prev
#define PUSH_TAG() TH_PUSH_TAG(GET_THREAD())
#define POP_TAG()      TH_POP_TAG()
#define TH_EXEC_TAG() ruby_setjmp(_th->tag->buf)
#define EXEC_TAG() \
  TH_EXEC_TAG()
#define TH_JUMP_TAG(th, st) do { \
  ruby_longjmp((th)->tag->buf,(st)); \
} while (0)
#define JUMP_TAG(st) TH_JUMP_TAG(GET_THREAD(), (st))

enum rb_thread_status {
  THREAD_TO_KILL,
  THREAD_RUNNABLE,
  THREAD_STOPPED,
  THREAD_STOPPED_FOREVER,
  THREAD_KILLED
};
typedef void rb_vm_t; /* TODO */
typedef HANDLE rb_thread_id_t;
typedef struct native_thread_data_struct {
  HANDLE interrupt_event;
} native_thread_data_t;
typedef void rb_block_t; /* TODO */
typedef struct {
  VALUE *pc;			/* cfp[0] */
  VALUE *sp;			/* cfp[1] */
  VALUE *bp;			/* cfp[2] */
  rb_iseq_t *iseq;		/* cfp[3] */
  VALUE flag;			/* cfp[4] */
  VALUE self;			/* cfp[5] / block[0] */
  VALUE *lfp;			/* cfp[6] / block[1] */
  VALUE *dfp;			/* cfp[7] / block[2] */
  rb_iseq_t *block_iseq;	/* cfp[8] / block[3] */
  VALUE proc;			/* cfp[9] / block[4] */
  const rb_method_entry_t *me;/* cfp[10] */
} rb_control_frame_t;
typedef void *rb_atomic_t; /* TODO */
#include <windows.h>
typedef CRITICAL_SECTION rb_thread_lock_t;
typedef void rb_unblock_function_t; /* TODO */
struct rb_unblock_callback {
    rb_unblock_function_t *func;
    void *arg;
};
struct rb_vm_protect_tag {
    struct rb_vm_protect_tag *prev;
};
typedef struct rb_thread_struct {
  VALUE self;
  rb_vm_t *vm;

  /* execution information */
  VALUE *stack; /* must free, must mark */
  unsigned long stack_size;
  rb_control_frame_t *cfp;
  int safe_level;
  int raised_flag;
  VALUE last_status; /* $? */

  /* passing state */
  int state;

  /* for rb_iterate */
  const rb_block_t *passed_block;

  /* for bmethod */
  const rb_method_entry_t *passed_me;

  /* for load(true) */
  VALUE top_self;
  VALUE top_wrapper;

  /* eval env */
  rb_block_t *base_block;

  VALUE *local_lfp;
  VALUE local_svar;

  /* thread control */
  rb_thread_id_t thread_id;
  enum rb_thread_status status;
  int priority;
  int slice;

  native_thread_data_t native_thread_data;
  void *blocking_region_buffer;

  VALUE thgroup;
  VALUE value;

  VALUE errinfo;
  VALUE thrown_errinfo;
  int exec_signal;

  int interrupt_flag;
  rb_thread_lock_t interrupt_lock;
  struct rb_unblock_callback unblock;
  VALUE locking_mutex;
  struct rb_mutex_struct *keeping_mutexes;
  int transition_for_lock;

  struct rb_vm_tag *tag;
  struct rb_vm_protect_tag *protect_tag;
  int parse_in_eval;
  int mild_compile_error;
  
  /* storage */
  st_table *local_storage;
  #if USE_VALUE_CACHE
  VALUE value_cache[RUBY_VM_VALUE_CACHE_SIZE + 1];
  VALUE *value_cache_ptr;
  #endif
  
  struct rb_thread_struct *join_list_next;
  struct rb_thread_struct *join_list_head;
  
  VALUE first_proc;
  VALUE first_args;
  VALUE (*first_func)(ANYARGS);
  
  /* for GC */
  VALUE *machine_stack_start;
  VALUE *machine_stack_end;
  size_t machine_stack_maxsize;
  #ifdef __ia64
  VALUE *machine_register_stack_start;
  VALUE *machine_register_stack_end;
  size_t machine_register_stack_maxsize;
  #endif
  jmp_buf machine_regs;
  int mark_stack_len;
  
  /* statistics data for profiler */
  VALUE stat_insn_usage;
  
  /* tracer */
  rb_event_hook_t *event_hooks;
  rb_event_flag_t event_flags;
  int tracing;
  
  /* fiber */
  VALUE fiber;
  VALUE root_fiber;
  rb_jmpbuf_t root_jmpbuf;
  /* TODO: rb_thread_t has more members */
} rb_thread_t;

#define GC_GUARDED_PTR_REF(p) ((void *)(((VALUE)(p)) & ~0x03))

rb_thread_t *get_thread(VALUE);
