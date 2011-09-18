#ifndef __KASHIWA_OBJECT__
#define __KASHIWA_OBJECT__

typedef unsigned long int lobject;

#define TAG_CONT   0  /* 00 */
#define TAG_CONS   1  /* 01 */
#define TAG_NUM    2  /* 10 */
#define TAG_OTHER  3  /* 11 */
#define GET_TAG(addr) (((lobject)(addr)) & 3)
#define ADD_TAG(addr, tag) ((lobject)(addr) | tag)
#define REM_TAG(addr) (((lobject)(addr)) & (~3))
#define NUM2INT(obj) (x >> 2)
#define INT2NUM(n) ((n << 2) | TAG_NUM)

typedef struct {
  lobject car;
  lobject cdr;
} cons_t;

#define TAG_CONT_PROC ((1 << 2) | TAG_OTHER)
typedef struct {
  unsigned char tag;
  void* c;
} cont_proc_t;

#define TAG_SYMBOL ((2 << 2) | TAG_OTHER)
typedef struct {
  unsigned char tag;
  char* name;
} symbol_t;

#endif
