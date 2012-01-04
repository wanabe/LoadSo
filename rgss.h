/*
  This file is made up of the part of Matz' Ruby Implementation.
  http://svn.ruby-lang.org/cgi-bin/viewvc.cgi/branches/ruby_1_9_2/
*/

#include <stdarg.h>

#ifdef __cplusplus
#define ANYARGS ...
#else
#define ANYARGS
#endif
#define RSHIFT(x,y) ((x)>>(int)(y))
#define ALLOC(type) ((type*)malloc(sizeof(type)))

typedef unsigned long VALUE;
typedef unsigned long ID;
#define SIGNED_VALUE long

enum ruby_special_consts {
    RUBY_Qfalse = 0,
    RUBY_Qtrue  = 2,
    RUBY_Qnil   = 4,
    RUBY_Qundef = 6,

    RUBY_IMMEDIATE_MASK = 0x03,
    RUBY_FIXNUM_FLAG    = 0x01,
    RUBY_SYMBOL_FLAG    = 0x0e,
    RUBY_SPECIAL_SHIFT  = 8
};
#define Qfalse ((VALUE)RUBY_Qfalse)
#define Qtrue  ((VALUE)RUBY_Qtrue)
#define Qnil   ((VALUE)RUBY_Qnil)
#define Qundef ((VALUE)RUBY_Qundef)	/* undefined value for placeholder */
#define IMMEDIATE_MASK RUBY_IMMEDIATE_MASK
#define FIXNUM_FLAG RUBY_FIXNUM_FLAG
#define SYMBOL_FLAG RUBY_SYMBOL_FLAG

#define ID2SYM(x) (((VALUE)(x)<<RUBY_SPECIAL_SHIFT)|SYMBOL_FLAG)
#define SYM2ID(x) RSHIFT((unsigned long)(x),RUBY_SPECIAL_SHIFT)
#define ID_ALLOCATOR 1

#define INT2FIX(i) ((VALUE)(((SIGNED_VALUE)(i))<<1 | FIXNUM_FLAG))

#define FL_SINGLETON FL_USER0
#define FL_MARK      (((VALUE)1)<<5)
#define FL_RESERVED  (((VALUE)1)<<6) /* will be used in the future GC */
#define FL_FINALIZE  (((VALUE)1)<<7)
#define FL_TAINT     (((VALUE)1)<<8)
#define FL_UNTRUSTED (((VALUE)1)<<9)
#define FL_EXIVAR    (((VALUE)1)<<10)
#define FL_FREEZE    (((VALUE)1)<<11)
#define FL_USHIFT    12
#define FL_USER0     (((VALUE)1)<<(FL_USHIFT+0))
#define FL_USER1     (((VALUE)1)<<(FL_USHIFT+1))
#define FL_USER2     (((VALUE)1)<<(FL_USHIFT+2))
#define FL_USER3     (((VALUE)1)<<(FL_USHIFT+3))
#define FL_USER4     (((VALUE)1)<<(FL_USHIFT+4))
#define FL_USER5     (((VALUE)1)<<(FL_USHIFT+5))
#define FL_USER6     (((VALUE)1)<<(FL_USHIFT+6))
#define FL_USER7     (((VALUE)1)<<(FL_USHIFT+7))
#define FL_USER8     (((VALUE)1)<<(FL_USHIFT+8))
#define FL_USER9     (((VALUE)1)<<(FL_USHIFT+9))
#define FL_USER10    (((VALUE)1)<<(FL_USHIFT+10))
#define FL_USER11    (((VALUE)1)<<(FL_USHIFT+11))
#define FL_USER12    (((VALUE)1)<<(FL_USHIFT+12))
#define FL_USER13    (((VALUE)1)<<(FL_USHIFT+13))
#define FL_USER14    (((VALUE)1)<<(FL_USHIFT+14))
#define FL_USER15    (((VALUE)1)<<(FL_USHIFT+15))
#define FL_USER16    (((VALUE)1)<<(FL_USHIFT+16))
#define FL_USER17    (((VALUE)1)<<(FL_USHIFT+17))
#define FL_USER18    (((VALUE)1)<<(FL_USHIFT+18))
#define FL_USER19    (((VALUE)1)<<(FL_USHIFT+19))

enum ruby_value_type {
    RUBY_T_NONE   = 0x00,

    RUBY_T_OBJECT = 0x01,
    RUBY_T_CLASS  = 0x02,
    RUBY_T_MODULE = 0x03,
    RUBY_T_FLOAT  = 0x04,
    RUBY_T_STRING = 0x05,
    RUBY_T_REGEXP = 0x06,
    RUBY_T_ARRAY  = 0x07,
    RUBY_T_HASH   = 0x08,
    RUBY_T_STRUCT = 0x09,
    RUBY_T_BIGNUM = 0x0a,
    RUBY_T_FILE   = 0x0b,
    RUBY_T_DATA   = 0x0c,
    RUBY_T_MATCH  = 0x0d,
    RUBY_T_COMPLEX  = 0x0e,
    RUBY_T_RATIONAL = 0x0f,

    RUBY_T_NIL    = 0x11,
    RUBY_T_TRUE   = 0x12,
    RUBY_T_FALSE  = 0x13,
    RUBY_T_SYMBOL = 0x14,
    RUBY_T_FIXNUM = 0x15,

    RUBY_T_UNDEF  = 0x1b,
    RUBY_T_NODE   = 0x1c,
    RUBY_T_ICLASS = 0x1d,
    RUBY_T_ZOMBIE = 0x1e,

    RUBY_T_MASK   = 0x1f
};

#define T_NONE   RUBY_T_NONE
#define T_NIL    RUBY_T_NIL
#define T_OBJECT RUBY_T_OBJECT
#define T_CLASS  RUBY_T_CLASS
#define T_ICLASS RUBY_T_ICLASS
#define T_MODULE RUBY_T_MODULE
#define T_FLOAT  RUBY_T_FLOAT
#define T_STRING RUBY_T_STRING
#define T_REGEXP RUBY_T_REGEXP
#define T_ARRAY  RUBY_T_ARRAY
#define T_HASH   RUBY_T_HASH
#define T_STRUCT RUBY_T_STRUCT
#define T_BIGNUM RUBY_T_BIGNUM
#define T_FILE   RUBY_T_FILE
#define T_FIXNUM RUBY_T_FIXNUM
#define T_TRUE   RUBY_T_TRUE
#define T_FALSE  RUBY_T_FALSE
#define T_DATA   RUBY_T_DATA
#define T_MATCH  RUBY_T_MATCH
#define T_SYMBOL RUBY_T_SYMBOL
#define T_RATIONAL RUBY_T_RATIONAL
#define T_COMPLEX RUBY_T_COMPLEX
#define T_UNDEF  RUBY_T_UNDEF
#define T_NODE   RUBY_T_NODE
#define T_ZOMBIE RUBY_T_ZOMBIE
#define T_MASK   RUBY_T_MASK
#define BUILTIN_TYPE(x) (int)(((struct RBasic*)(x))->flags & T_MASK)

#define RTEST(v) (((VALUE)(v) & ~Qnil) != 0)
#define NIL_P(v) ((VALUE)(v) == Qnil)
#define FIXNUM_P(f) (((SIGNED_VALUE)(f))&FIXNUM_FLAG)
#define IMMEDIATE_P(x) ((VALUE)(x) & IMMEDIATE_MASK)
#define SYMBOL_P(x) (((VALUE)(x)&~(~(VALUE)0<<RUBY_SPECIAL_SHIFT))==SYMBOL_FLAG)
#define SPECIAL_CONST_P(x) (IMMEDIATE_P(x) || !RTEST(x))
#define FL_ABLE(x) (!SPECIAL_CONST_P(x) && BUILTIN_TYPE(x) != T_NODE)
#define FL_TEST(x,f) (FL_ABLE(x)?(RBASIC(x)->flags&(f)):0)
#define OBJ_TAINTED(x) (!!FL_TEST((x), FL_TAINT))

#define FIX2LONG(x) (long)RSHIFT((SIGNED_VALUE)(x),1)

struct RBasic {
    VALUE flags;
    VALUE klass;
};

struct rb_classext_struct {
    VALUE super;
    struct st_table *iv_tbl;
    struct st_table *const_tbl;
};
typedef struct rb_classext_struct rb_classext_t;
struct RClass {
    struct RBasic basic;
    rb_classext_t *ptr;
    struct st_table *m_tbl;
    struct st_table *iv_index_tbl;
};
#define RCLASS_EXT(c) (RCLASS(c)->ptr)
#define RCLASS_SUPER(c) (RCLASS_EXT(c)->super)
#define RCLASS_M_TBL(c) (RCLASS(c)->m_tbl)

#define RSTRING_EMBED_LEN_MAX ((int)((sizeof(VALUE)*3)/sizeof(char)-1))
struct RString {
    struct RBasic basic;
    union {
	struct {
	    long len;
	    char *ptr;
	    union {
		long capa;
		VALUE shared;
	    } aux;
	} heap;
	char ary[RSTRING_EMBED_LEN_MAX + 1];
    } as;
};
#define RSTRING_NOEMBED FL_USER1
#define RSTRING_EMBED_LEN_MASK (FL_USER2|FL_USER3|FL_USER4|FL_USER5|FL_USER6)
#define RSTRING_EMBED_LEN_SHIFT (FL_USHIFT+2)
#define RSTRING_EMBED_LEN(str) \
     (long)((RBASIC(str)->flags >> RSTRING_EMBED_LEN_SHIFT) & \
            (RSTRING_EMBED_LEN_MASK >> RSTRING_EMBED_LEN_SHIFT))
#define RSTRING_LEN(str) \
    (!(RBASIC(str)->flags & RSTRING_NOEMBED) ? \
     RSTRING_EMBED_LEN(str) : \
     RSTRING(str)->as.heap.len)
#define RSTRING_PTR(str) \
    (!(RBASIC(str)->flags & RSTRING_NOEMBED) ? \
     RSTRING(str)->as.ary : \
     RSTRING(str)->as.heap.ptr)

#define RARRAY_EMBED_LEN_MAX 3
struct RArray {
    struct RBasic basic;
    union {
	struct {
	    long len;
	    union {
		long capa;
		VALUE shared;
	    } aux;
	    VALUE *ptr;
	} heap;
	VALUE ary[RARRAY_EMBED_LEN_MAX];
    } as;
};
#define RARRAY_EMBED_FLAG FL_USER1
#define RARRAY_EMBED_LEN_MASK (FL_USER4|FL_USER3)
#define RARRAY_EMBED_LEN_SHIFT (FL_USHIFT+3)
#define RARRAY_LEN(a) \
    ((RBASIC(a)->flags & RARRAY_EMBED_FLAG) ? \
     (long)((RBASIC(a)->flags >> RARRAY_EMBED_LEN_SHIFT) & \
	 (RARRAY_EMBED_LEN_MASK >> RARRAY_EMBED_LEN_SHIFT)) : \
     RARRAY(a)->as.heap.len)
#define RARRAY_PTR(a) \
    ((RBASIC(a)->flags & RARRAY_EMBED_FLAG) ? \
     RARRAY(a)->as.ary : \
     RARRAY(a)->as.heap.ptr)

struct RFloat {
    struct RBasic basic;
    double float_value;
};
#define RFLOAT_VALUE(v) (RFLOAT(v)->float_value)
#define DBL2NUM(dbl)  rb_float_new(dbl)

struct RHash {
    struct RBasic basic;
    struct st_table *ntbl;      /* possibly 0 */
    int iter_lev;
    VALUE ifnone;
};

struct RData {
    struct RBasic basic;
    void (*dmark)(void*);
    void (*dfree)(void*);
    void *data;
};
#define DATA_PTR(dta) (RDATA(dta)->data)
typedef struct rb_data_type_struct rb_data_type_t;
struct rb_data_type_struct {
    const char *wrap_struct_name;
    struct {
	void (*dmark)(void*);
	void (*dfree)(void*);
	size_t (*dsize)(const void *);
	void *reserved[2]; /* For future extension.
			      This array *must* be filled with ZERO. */
    } function;
    const rb_data_type_t *parent;
    void *data;        /* This area can be used for any purpose
                          by a programmer who define the type. */
};
struct RTypedData {
    struct RBasic basic;
    const rb_data_type_t *type;
    VALUE typed_flag; /* 1 or not */
    void *data;
};
#define RTYPEDDATA_P(v)    (RTYPEDDATA(v)->typed_flag == 1)
#define RTYPEDDATA_TYPE(v) (RTYPEDDATA(v)->type)
#define RTYPEDDATA_DATA(v) (RTYPEDDATA(v)->data)
#define TypedData_Get_Struct(obj,type,data_type,sval) do {\
  (sval) = (type*)DATA_PTR(obj);\
} while(0) /* TODO: type check */
typedef void (*RUBY_DATA_FUNC)(void*);

#define R_CAST(st)   (struct st*)
#define RBASIC(obj)  (R_CAST(RBasic)(obj))
#define ROBJECT(obj) (R_CAST(RObject)(obj))
#define RCLASS(obj)  (R_CAST(RClass)(obj))
#define RMODULE(obj) RCLASS(obj)
#define RFLOAT(obj)  (R_CAST(RFloat)(obj))
#define RSTRING(obj) (R_CAST(RString)(obj))
#define RREGEXP(obj) (R_CAST(RRegexp)(obj))
#define RARRAY(obj)  (R_CAST(RArray)(obj))
#define RHASH(obj)   (R_CAST(RHash)(obj))
#define RDATA(obj)   (R_CAST(RData)(obj))
#define RTYPEDDATA(obj)   (R_CAST(RTypedData)(obj))
#define RSTRUCT(obj) (R_CAST(RStruct)(obj))
#define RBIGNUM(obj) (R_CAST(RBignum)(obj))
#define RFILE(obj)   (R_CAST(RFile)(obj))
#define RRATIONAL(obj) (R_CAST(RRational)(obj))
#define RCOMPLEX(obj) (R_CAST(RComplex)(obj))

static inline int rb_type(VALUE obj) {
  if (IMMEDIATE_P(obj)) {
    if (FIXNUM_P(obj)) return T_FIXNUM;
    if (obj == Qtrue) return T_TRUE;
    if (SYMBOL_P(obj)) return T_SYMBOL;
    if (obj == Qundef) return T_UNDEF;
  } else if (!RTEST(obj)) {
    if (obj == Qnil) return T_NIL;
    if (obj == Qfalse) return T_FALSE;
  }
  return BUILTIN_TYPE(obj);
}
#define TYPE(x) rb_type((VALUE)(x))
#define CLASS_OF(v) rb_class_of((VALUE)(v))

#define NEWOBJ(obj,type) type *(obj) = (type*)rb_newobj()
#define OBJSETUP(obj,c,t) do {\
    RBASIC(obj)->flags = (t);\
    RBASIC(obj)->klass = (c);\
} while (0)

typedef enum {
    NOEX_PUBLIC    = 0x00,
    NOEX_NOSUPER   = 0x01,
    NOEX_PRIVATE   = 0x02,
    NOEX_PROTECTED = 0x04,
    NOEX_MASK      = 0x06,
    NOEX_BASIC     = 0x08,
    NOEX_UNDEF     = NOEX_NOSUPER,
    NOEX_MODFUNC   = 0x12,
    NOEX_SUPER     = 0x20,
    NOEX_VCALL     = 0x40,
    NOEX_RESPONDS  = 0x80
} rb_method_flag_t;
typedef enum {
    VM_METHOD_TYPE_ISEQ,
    VM_METHOD_TYPE_CFUNC,
    VM_METHOD_TYPE_ATTRSET,
    VM_METHOD_TYPE_IVAR,
    VM_METHOD_TYPE_BMETHOD,
    VM_METHOD_TYPE_ZSUPER,
    VM_METHOD_TYPE_UNDEF,
    VM_METHOD_TYPE_NOTIMPLEMENTED,
    VM_METHOD_TYPE_OPTIMIZED, /* Kernel#send, Proc#call, etc */
    VM_METHOD_TYPE_MISSING   /* wrapper for method_missing(id) */
} rb_method_type_t;
typedef struct rb_method_cfunc_struct {
    VALUE (*func)(ANYARGS);
    int argc;
} rb_method_cfunc_t;
typedef struct rb_method_attr_struct {
    ID id;
    VALUE location;
} rb_method_attr_t;
typedef struct rb_iseq_struct rb_iseq_t;
typedef struct rb_method_definition_struct {
    rb_method_type_t type; /* method type */
    ID original_id;
    union {
	rb_iseq_t *iseq;            /* should be mark */
	rb_method_cfunc_t cfunc;
	rb_method_attr_t attr;
	VALUE proc;                 /* should be mark */
	enum method_optimized_type {
	    OPTIMIZED_METHOD_TYPE_SEND,
	    OPTIMIZED_METHOD_TYPE_CALL
	} optimize_type;
    } body;
    int alias_count;
} rb_method_definition_t;
typedef struct rb_method_entry_struct {
    rb_method_flag_t flag;
    char mark;
    rb_method_definition_t *def;
    ID called_id;
    VALUE klass;                    /* should be mark */
} rb_method_entry_t;
struct METHOD {
    VALUE recv;
    VALUE rclass;
    ID id;
    rb_method_entry_t me;
};
#define ISDIGIT(c) (c >= '0' && c <= '9') /*rb_isdigit((unsigned char)(c))*/
typedef struct OnigEncodingTypeST {
  void *reserved1;     /* TODO */
  const char *name;
  int max_enc_len;
  int min_enc_len;
  void *reserved2[12]; /* TODO */
  int ruby_encoding_index;
} OnigEncodingType, rb_encoding;

typedef void rb_vm_t; /* TODO */
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
typedef struct rb_thread_struct {
    VALUE self;
    rb_vm_t *vm;

    /* execution information */
    VALUE *stack;		/* must free, must mark */
    unsigned long stack_size;
    rb_control_frame_t *cfp;
    int safe_level;
  /* TODO: rb_thread_t has more members */
} rb_thread_t;

VALUE rb_newobj();
void rb_raise(VALUE, const char*,...);
VALUE rb_const_get(VALUE, ID);
ID rb_intern(const char*);
VALUE rb_define_module_under(VALUE, const char*);
VALUE rb_define_module(const char*);
void rb_const_set(VALUE, ID, VALUE);
void rb_define_singleton_method(VALUE, const char*, VALUE (*)(ANYARGS), int);
VALUE rb_ary_new();
VALUE rb_ary_new4(long, const VALUE*);
int rb_block_given_p();
VALUE rb_block_proc();
VALUE rb_eval_string(const char*);
VALUE rb_class_new_instance(int, VALUE*, VALUE);
void rb_define_global_function(const char*, VALUE (*)(ANYARGS), int);
VALUE rb_funcall3(VALUE, ID, int, const VALUE*);
void rb_gc_mark(VALUE);
VALUE rb_ary_push(VALUE, VALUE);
VALUE rb_vsprintf(const char *format, va_list ap);

extern VALUE rb_cObject, rb_mKernel, rb_cModule, rb_cClass, rb_cArray, rb_cString, rb_cFloat, rb_cHash, rb_cProc, rb_cData;
extern VALUE rb_eRuntimeError, rb_eLoadError, rb_eTypeError, rb_eArgError, rb_eNotImpError, rb_eSecurityError, rb_eNoMethodError;
extern VALUE rb_cFixnum, rb_cBignum, rb_cTrueClass, rb_cSymbol, rb_cNilClass, rb_cFalseClass, rb_cTime, rb_cEncoding, rb_cThread;
extern VALUE rb_cMethod, rb_cUnboundMethod;
extern VALUE rb_mGC;

static inline VALUE rb_class_of(VALUE obj) {
  if (IMMEDIATE_P(obj)) {
    if (FIXNUM_P(obj)) return rb_cFixnum;
    if (obj == Qtrue)  return rb_cTrueClass;
    if (SYMBOL_P(obj)) return rb_cSymbol;
  }
  else if (!RTEST(obj)) {
    if (obj == Qnil)   return rb_cNilClass;
    if (obj == Qfalse) return rb_cFalseClass;
  }
  return RBASIC(obj)->klass;
}
