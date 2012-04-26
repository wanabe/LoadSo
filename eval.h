#include <setjmp.h>

typedef unsigned int rb_event_flag_t;
typedef void rb_event_hook_t; /* TODO */

enum ruby_tag_type {
  RUBY_TAG_RETURN	= 0x1,
  RUBY_TAG_BREAK	= 0x2,
  RUBY_TAG_NEXT	= 0x3,
  RUBY_TAG_RETRY	= 0x4,
  RUBY_TAG_REDO	= 0x5,
  RUBY_TAG_RAISE	= 0x6,
  RUBY_TAG_THROW	= 0x7,
  RUBY_TAG_FATAL	= 0x8,
  RUBY_TAG_MASK	= 0xf
};
#define TAG_RETURN	RUBY_TAG_RETURN
#define TAG_BREAK	RUBY_TAG_BREAK
#define TAG_NEXT	RUBY_TAG_NEXT
#define TAG_RETRY	RUBY_TAG_RETRY
#define TAG_REDO	RUBY_TAG_REDO
#define TAG_RAISE	RUBY_TAG_RAISE
#define TAG_THROW	RUBY_TAG_THROW
#define TAG_FATAL	RUBY_TAG_FATAL
#define TAG_MASK	RUBY_TAG_MASK

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

#define GET_THROWOBJ_VAL(obj)         ((VALUE)RNODE((obj))->u1.value)
#define GET_THROWOBJ_CATCH_POINT(obj) ((VALUE*)RNODE((obj))->u2.value)
#define GET_THROWOBJ_STATE(obj)       ((int)RNODE((obj))->u3.value)

#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))

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
typedef struct rb_block_struct {
  VALUE self;			/* share with method frame if it's only block */
  VALUE *lfp;			/* share with method frame if it's only block */
  VALUE *dfp;			/* share with method frame if it's only block */
  rb_iseq_t *iseq;
  VALUE proc;
} rb_block_t;
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

enum node_type {
    NODE_SCOPE,
#define NODE_SCOPE       NODE_SCOPE
    NODE_BLOCK,
#define NODE_BLOCK       NODE_BLOCK
    NODE_IF,
#define NODE_IF          NODE_IF
    NODE_CASE,
#define NODE_CASE        NODE_CASE
    NODE_WHEN,
#define NODE_WHEN        NODE_WHEN
    NODE_OPT_N,
#define NODE_OPT_N       NODE_OPT_N
    NODE_WHILE,
#define NODE_WHILE       NODE_WHILE
    NODE_UNTIL,
#define NODE_UNTIL       NODE_UNTIL
    NODE_ITER,
#define NODE_ITER        NODE_ITER
    NODE_FOR,
#define NODE_FOR         NODE_FOR
    NODE_BREAK,
#define NODE_BREAK       NODE_BREAK
    NODE_NEXT,
#define NODE_NEXT        NODE_NEXT
    NODE_REDO,
#define NODE_REDO        NODE_REDO
    NODE_RETRY,
#define NODE_RETRY       NODE_RETRY
    NODE_BEGIN,
#define NODE_BEGIN       NODE_BEGIN
    NODE_RESCUE,
#define NODE_RESCUE      NODE_RESCUE
    NODE_RESBODY,
#define NODE_RESBODY     NODE_RESBODY
    NODE_ENSURE,
#define NODE_ENSURE      NODE_ENSURE
    NODE_AND,
#define NODE_AND         NODE_AND
    NODE_OR,
#define NODE_OR          NODE_OR
    NODE_MASGN,
#define NODE_MASGN       NODE_MASGN
    NODE_LASGN,
#define NODE_LASGN       NODE_LASGN
    NODE_DASGN,
#define NODE_DASGN       NODE_DASGN
    NODE_DASGN_CURR,
#define NODE_DASGN_CURR  NODE_DASGN_CURR
    NODE_GASGN,
#define NODE_GASGN       NODE_GASGN
    NODE_IASGN,
#define NODE_IASGN       NODE_IASGN
    NODE_IASGN2,
#define NODE_IASGN2      NODE_IASGN2
    NODE_CDECL,
#define NODE_CDECL       NODE_CDECL
    NODE_CVASGN,
#define NODE_CVASGN      NODE_CVASGN
    NODE_CVDECL,
#define NODE_CVDECL      NODE_CVDECL
    NODE_OP_ASGN1,
#define NODE_OP_ASGN1    NODE_OP_ASGN1
    NODE_OP_ASGN2,
#define NODE_OP_ASGN2    NODE_OP_ASGN2
    NODE_OP_ASGN_AND,
#define NODE_OP_ASGN_AND NODE_OP_ASGN_AND
    NODE_OP_ASGN_OR,
#define NODE_OP_ASGN_OR  NODE_OP_ASGN_OR
    NODE_CALL,
#define NODE_CALL        NODE_CALL
    NODE_FCALL,
#define NODE_FCALL       NODE_FCALL
    NODE_VCALL,
#define NODE_VCALL       NODE_VCALL
    NODE_SUPER,
#define NODE_SUPER       NODE_SUPER
    NODE_ZSUPER,
#define NODE_ZSUPER      NODE_ZSUPER
    NODE_ARRAY,
#define NODE_ARRAY       NODE_ARRAY
    NODE_ZARRAY,
#define NODE_ZARRAY      NODE_ZARRAY
    NODE_VALUES,
#define NODE_VALUES      NODE_VALUES
    NODE_HASH,
#define NODE_HASH        NODE_HASH
    NODE_RETURN,
#define NODE_RETURN      NODE_RETURN
    NODE_YIELD,
#define NODE_YIELD       NODE_YIELD
    NODE_LVAR,
#define NODE_LVAR        NODE_LVAR
    NODE_DVAR,
#define NODE_DVAR        NODE_DVAR
    NODE_GVAR,
#define NODE_GVAR        NODE_GVAR
    NODE_IVAR,
#define NODE_IVAR        NODE_IVAR
    NODE_CONST,
#define NODE_CONST       NODE_CONST
    NODE_CVAR,
#define NODE_CVAR        NODE_CVAR
    NODE_NTH_REF,
#define NODE_NTH_REF     NODE_NTH_REF
    NODE_BACK_REF,
#define NODE_BACK_REF    NODE_BACK_REF
    NODE_MATCH,
#define NODE_MATCH       NODE_MATCH
    NODE_MATCH2,
#define NODE_MATCH2      NODE_MATCH2
    NODE_MATCH3,
#define NODE_MATCH3      NODE_MATCH3
    NODE_LIT,
#define NODE_LIT         NODE_LIT
    NODE_STR,
#define NODE_STR         NODE_STR
    NODE_DSTR,
#define NODE_DSTR        NODE_DSTR
    NODE_XSTR,
#define NODE_XSTR        NODE_XSTR
    NODE_DXSTR,
#define NODE_DXSTR       NODE_DXSTR
    NODE_EVSTR,
#define NODE_EVSTR       NODE_EVSTR
    NODE_DREGX,
#define NODE_DREGX       NODE_DREGX
    NODE_DREGX_ONCE,
#define NODE_DREGX_ONCE  NODE_DREGX_ONCE
    NODE_ARGS,
#define NODE_ARGS        NODE_ARGS
    NODE_ARGS_AUX,
#define NODE_ARGS_AUX    NODE_ARGS_AUX
    NODE_OPT_ARG,
#define NODE_OPT_ARG     NODE_OPT_ARG
    NODE_POSTARG,
#define NODE_POSTARG     NODE_POSTARG
    NODE_ARGSCAT,
#define NODE_ARGSCAT     NODE_ARGSCAT
    NODE_ARGSPUSH,
#define NODE_ARGSPUSH    NODE_ARGSPUSH
    NODE_SPLAT,
#define NODE_SPLAT       NODE_SPLAT
    NODE_TO_ARY,
#define NODE_TO_ARY      NODE_TO_ARY
    NODE_BLOCK_ARG,
#define NODE_BLOCK_ARG   NODE_BLOCK_ARG
    NODE_BLOCK_PASS,
#define NODE_BLOCK_PASS  NODE_BLOCK_PASS
    NODE_DEFN,
#define NODE_DEFN        NODE_DEFN
    NODE_DEFS,
#define NODE_DEFS        NODE_DEFS
    NODE_ALIAS,
#define NODE_ALIAS       NODE_ALIAS
    NODE_VALIAS,
#define NODE_VALIAS      NODE_VALIAS
    NODE_UNDEF,
#define NODE_UNDEF       NODE_UNDEF
    NODE_CLASS,
#define NODE_CLASS       NODE_CLASS
    NODE_MODULE,
#define NODE_MODULE      NODE_MODULE
    NODE_SCLASS,
#define NODE_SCLASS      NODE_SCLASS
    NODE_COLON2,
#define NODE_COLON2      NODE_COLON2
    NODE_COLON3,
#define NODE_COLON3      NODE_COLON3
    NODE_DOT2,
#define NODE_DOT2        NODE_DOT2
    NODE_DOT3,
#define NODE_DOT3        NODE_DOT3
    NODE_FLIP2,
#define NODE_FLIP2       NODE_FLIP2
    NODE_FLIP3,
#define NODE_FLIP3       NODE_FLIP3
    NODE_SELF,
#define NODE_SELF        NODE_SELF
    NODE_NIL,
#define NODE_NIL         NODE_NIL
    NODE_TRUE,
#define NODE_TRUE        NODE_TRUE
    NODE_FALSE,
#define NODE_FALSE       NODE_FALSE
    NODE_ERRINFO,
#define NODE_ERRINFO     NODE_ERRINFO
    NODE_DEFINED,
#define NODE_DEFINED     NODE_DEFINED
    NODE_POSTEXE,
#define NODE_POSTEXE     NODE_POSTEXE
    NODE_ALLOCA,
#define NODE_ALLOCA      NODE_ALLOCA
    NODE_BMETHOD,
#define NODE_BMETHOD     NODE_BMETHOD
    NODE_MEMO,
#define NODE_MEMO        NODE_MEMO
    NODE_IFUNC,
#define NODE_IFUNC       NODE_IFUNC
    NODE_DSYM,
#define NODE_DSYM        NODE_DSYM
    NODE_ATTRASGN,
#define NODE_ATTRASGN    NODE_ATTRASGN
    NODE_PRELUDE,
#define NODE_PRELUDE     NODE_PRELUDE
    NODE_LAMBDA,
#define NODE_LAMBDA      NODE_LAMBDA
    NODE_OPTBLOCK,
#define NODE_OPTBLOCK    NODE_OPTBLOCK
    NODE_LAST
#define NODE_LAST        NODE_LAST
};
typedef struct RNode {
  unsigned long flags;
  char *nd_file;
  union {
    struct RNode *node;
    ID id;
    VALUE value;
    VALUE (*cfunc)(ANYARGS);
    ID *tbl;
  } u1;
  union {
    struct RNode *node;
    ID id;
    long argc;
    VALUE value;
  } u2;
  union {
    struct RNode *node;
    ID id;
    long state;
    struct rb_global_entry *entry;
    long cnt;
    VALUE value;
  } u3;
} NODE;

#define nd_aid   u3.id
#define RNODE(obj)  (R_CAST(RNode)(obj))
#define NODE_TYPESHIFT 8
#define NODE_TYPEMASK  (((VALUE)0x7f)<<NODE_TYPESHIFT)
#define nd_set_type(n,t) \
    RNODE(n)->flags=((RNODE(n)->flags&~NODE_TYPEMASK)|((((unsigned long)(t))<<NODE_TYPESHIFT)&NODE_TYPEMASK))
#define NEW_NODE(t,a0,a1,a2) rb_node_newnode((t),(VALUE)(a0),(VALUE)(a1),(VALUE)(a2))
#define NEW_IFUNC(f,c) NEW_NODE(NODE_IFUNC,f,c,0)

#define VM_FRAME_MAGIC_METHOD 0x11
#define VM_FRAME_MAGIC_BLOCK  0x21
#define VM_FRAME_MAGIC_CLASS  0x31
#define VM_FRAME_MAGIC_TOP    0x41
#define VM_FRAME_MAGIC_FINISH 0x51
#define VM_FRAME_MAGIC_CFUNC  0x61
#define VM_FRAME_MAGIC_PROC   0x71
#define VM_FRAME_MAGIC_IFUNC  0x81
#define VM_FRAME_MAGIC_EVAL   0x91
#define VM_FRAME_MAGIC_LAMBDA 0xa1
#define VM_FRAME_MAGIC_MASK_BITS   8
#define VM_FRAME_MAGIC_MASK   (~(~0<<VM_FRAME_MAGIC_MASK_BITS))
#define VM_FRAME_FLAG_PASSED 0x0100

#define GC_GUARDED_PTR_REF(p) ((void *)(((VALUE)(p)) & ~0x03))
#define RUBY_VM_GET_BLOCK_PTR_IN_CFP(cfp) ((rb_block_t *)(&(cfp)->self))
#define VM_FRAME_TYPE(cfp) ((cfp)->flag & VM_FRAME_MAGIC_MASK)
#define RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp) ((cfp)+1)
#define PASS_PASSED_BLOCK_TH(th) do { \
    (th)->passed_block = GC_GUARDED_PTR_REF((rb_block_t *)(th)->cfp->lfp[0]); \
    (th)->cfp->flag |= VM_FRAME_FLAG_PASSED; \
} while (0)
#define PASS_PASSED_BLOCK() do { \
    rb_thread_t * const __th__ = GET_THREAD(); \
    PASS_PASSED_BLOCK_TH(__th__); \
} while (0)


void rb_threadptr_exec_event_hooks(rb_thread_t *th, rb_event_flag_t flag, VALUE self, ID id, VALUE klass);
#define EXEC_EVENT_HOOK(th, flag, self, id, klass) do { \
    rb_event_flag_t wait_event__ = (th)->event_flags; \
    if (UNLIKELY(wait_event__)) { \
	if (wait_event__ & ((flag) | RUBY_EVENT_VM)) { \
	    rb_threadptr_exec_event_hooks((th), (flag), (self), (id), (klass)); \
	} \
    } \
} while (0)

rb_thread_t *get_thread(VALUE);
