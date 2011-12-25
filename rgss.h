/*
  This file is made up of the part of Matz' Ruby Implementation.
  http://svn.ruby-lang.org/cgi-bin/viewvc.cgi/branches/ruby_1_9_2/
*/
#ifdef __cplusplus
#define ANYARGS ...
#else
#define ANYARGS
#endif
#define RSHIFT(x,y) ((x)>>(int)(y))
#define ALLOC(type) ((type*)malloc(sizeof(type)))

typedef unsigned long VALUE;
typedef unsigned long ID;

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

struct RBasic {
    VALUE flags;
    VALUE klass;
};

struct RClass {
    struct RBasic basic;
    /*rb_classext_t*/void *ptr;
    struct st_table *m_tbl;
    struct st_table *iv_index_tbl;
};
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
#define RSTRING_PTR(str) \
    (!(RBASIC(str)->flags & RSTRING_NOEMBED) ? \
     RSTRING(str)->as.ary : \
     RSTRING(str)->as.heap.ptr)

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

#define R_CAST(st)      (struct st*)
#define RBASIC(obj)     (R_CAST(RBasic)(obj))
#define RCLASS(obj)     (R_CAST(RClass)(obj))
#define RSTRING(obj)    (R_CAST(RString)(obj))
#define RDATA(obj)      (R_CAST(RData)(obj))
#define RTYPEDDATA(obj) (R_CAST(RTypedData)(obj))

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

extern VALUE rb_cObject, rb_cModule, rb_cString, rb_eRuntimeError;
void rb_raise(VALUE, const char*,...);
VALUE rb_const_get(VALUE, ID);
ID rb_intern(const char*);
VALUE rb_define_module_under(VALUE, const char*);
VALUE rb_define_module(const char*);
void rb_const_set(VALUE, ID, VALUE);
void rb_define_singleton_method(VALUE, const char*, VALUE (*)(ANYARGS), int);
