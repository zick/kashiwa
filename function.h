#ifndef __KASHIWA_FUNCTION__
#define __KASHIWA_FUNCTION__

#include "object.h"

#include <stdio.h>
#include <stdlib.h>

/* env_t is used for lexical scope.
 * It keeps the local variables and static link.
 */
typedef struct tag_env_t {
  int num;
  struct tag_env_t* link;
  lobject vars[1];
} env_t;


typedef void (*function1_t)(env_t*, lobject);
typedef void (*function2_t)(env_t*, lobject, lobject);
typedef void (*function3_t)(env_t*, lobject, lobject, lobject);
typedef void (*function4_t)(env_t*, lobject, lobject, lobject, lobject);

/* thunkN is used for resuming the function call from GC.
 * It keeps the all arguments and the environment.
 */
#define DEFTHUNK(n)                             \
  typedef struct {                              \
    int num;                                    \
    void* fn;                                   \
    env_t* env;                                 \
    lobject vars[n];                            \
  } thunk##n

DEFTHUNK(1);
DEFTHUNK(2);
DEFTHUNK(3);
DEFTHUNK(4);

#define CALL1(cs)               \
  (function1_t((cs)->fn))       \
  ((cs)->env, (cs)->vars[0])
#define CALL2(cs)                               \
  (function2_t((cs)->fn))                       \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1])
#define CALL3(cs)                                           \
  (function3_t((cs)->fn))                                   \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1], (cs)->vars[2])
#define CALL4(cs)                                                       \
  (function4_t((cs)->fn))                                               \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1], (cs)->vars[2], (cs)->vars[3])

/* cont_t represents the continuations.
 * It corresponds to Scheme's closure.
 */
typedef struct {
  function1_t fn;
  env_t* env;
} cont_t;

#define CONTINUE1(cont, val)                                \
  if (GET_TAG(cont) == TAG_CONT) {                          \
    RAW_CONTINUE1(cont, val);                               \
  } else {                                                  \
    fprintf(stderr, "Invalid application.\n");              \
    exit(1);                                                \
  }
#define CONTINUE2(cont, val1, val2)                                 \
  if (GET_TAG(cont) == TAG_CONT) {                                  \
    RAW_CONTINUE2(cont, val1, val2);                                \
  } else if (GET_TAG(cont) == TAG_OTHER &&                          \
             *((unsigned char*)REM_TAG((cont))) == TAG_CONT_PROC) { \
    RAW_CONTINUE1(((cont_proc_t*)REM_TAG(cont))->c, val2);          \
  } else {                                                          \
    fprintf(stderr, "Invalid application.\n");                      \
    exit(1);                                                        \
  }
#define CONTINUE3(cont, val1, val2, val3)                   \
  if (GET_TAG(cont) == TAG_CONT) {                          \
    RAW_CONTINUE3(cont, val1, val2, val3);                  \
  } else {                                                  \
    fprintf(stderr, "Invalid application.\n");              \
    exit(1);                                                \
  }
#define CONTINUE4(cont, val1, val2, val3, val4)             \
  if (GET_TAG(cont) == TAG_CONT) {                          \
    RAW_CONTINUE4(cont, val1, val2, val3, val4);            \
  } else {                                                  \
    fprintf(stderr, "Invalid application.\n");              \
    exit(1);                                                \
  }

#define RAW_CONTINUE1(cont, val)                                        \
  ((function1_t)(((cont_t*)(cont))->fn))                                \
  (((cont_t*)(cont))->env, (lobject)(val))
#define RAW_CONTINUE2(cont, val1, val2)                                 \
  ((function2_t)(((cont_t*)(cont))->fn))                                \
  (((cont_t*)(cont))->env, (lobject)(val1), (lobject)(val2))
#define RAW_CONTINUE3(cont, val1, val2, val3)                           \
  ((function3_t)(((cont_t*)(cont))->fn))                                \
  (((cont_t*)(cont))->env, (lobject)(val1), (lobject)(val2), (lobject)(val3))
#define RAW_CONTINUE4(cont, val1, val2, val3, val4)                     \
  ((function4_t)(((cont_t*)(cont))->fn))                                \
  (((cont_t*)(cont))->env, (lobject)(val1), (lobject)(val2),            \
   (lobject)(val3), (lobject)(val4))

#endif
