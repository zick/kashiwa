#ifndef __KASHIWA_OBJECT__
#define __KASHIWA_OBJECT__

typedef unsigned long int lobject;

#define PTAG_CONT   0  /* 00 */
#define PTAG_CONS   1  /* 01 */
#define PTAG_FIXNUM 2  /* 10 */
#define PTAG_OTHER  3  /* 11 */
#define GET_PTAG(addr) (((lobject)(addr)) & 3)
#define ADD_PTAG(addr, tag) ((lobject)(addr) | (tag))
#define REM_PTAG(addr) (((lobject)(addr)) & (~3))
#define FIXNUM2INT(obj) ((int)(obj) >> 2)
#define INT2FIXNUM(n) (((n) << 2) | PTAG_FIXNUM)

#define OBJ_TAG(x) *((unsigned int*)(REM_PTAG(x)))
#define FORWARDING_ADDRESS(x) *((void**)(REM_PTAG(x)) + 1)

#define TAG_CONT       0
#define TAG_CONS       1
#define TAG_ENV        2
#define TAG_CONT_PROC  3
#define TAG_SYMBOL     4
#define TAG_FORWARDING ~((unsigned int)0)

/* env_t is used for lexical scope.
 * It keeps the local variables and static link.
 */
typedef struct tag_env_t {
  unsigned int tag;
  unsigned int num;
  struct tag_env_t* link;
  lobject vars[1];
} env_t;

typedef void (*function1_t)(env_t*, lobject);
typedef void (*function2_t)(env_t*, lobject, lobject);
typedef void (*function3_t)(env_t*, lobject, lobject, lobject);
typedef void (*function4_t)(env_t*, lobject, lobject, lobject, lobject);

/* cont_t represents the continuations.
 * It corresponds to Scheme's closure.
 */
typedef struct {
  unsigned int tag;
  function1_t fn;
  env_t* env;
} cont_t;

typedef struct {
  unsigned int tag;
  lobject car;
  lobject cdr;
} cons_t;

typedef struct {
  unsigned int tag;
  void* c;
} cont_proc_t;

typedef struct {
  unsigned int tag;
  char* name;
} symbol_t;

#endif
