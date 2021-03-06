#ifndef __KASHIWA_FUNCTION__
#define __KASHIWA_FUNCTION__

#include "object.h"

#include <stdio.h>
#include <stdlib.h>

/* thunk_t is used for resuming the function call from GC.
 * It keeps the all arguments and the environment.
 */
typedef struct {
  int num;
  void* fn;
  env_t* env;
  lobject vars[1];
} thunk_t;

#define NUM_ARGUMENTS_LIMIT 4

#define CALL_THUNK(tk)                                  \
  switch ((tk)->num) {                                  \
  case 1: CALL1(tk); break;                             \
  case 2: CALL2(tk); break;                             \
  case 3: CALL3(tk); break;                             \
  case 4: CALL4(tk); break;                             \
  default: assert(0);                                   \
  }

#define CALL1(cs)                               \
  ((function1_t)((cs)->fn))                     \
  ((cs)->env, (cs)->vars[0])
#define CALL2(cs)                               \
  ((function2_t)((cs)->fn))                     \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1])
#define CALL3(cs)                                           \
  ((function3_t)((cs)->fn))                                 \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1], (cs)->vars[2])
#define CALL4(cs)                                                       \
  ((function4_t)((cs)->fn))                                             \
  ((cs)->env, (cs)->vars[0], (cs)->vars[1], (cs)->vars[2], (cs)->vars[3])

#define CHECK_NUM_ARGS(cont, num)                 \
  (((cont_t*)(cont))->num_required_args == num && \
   ((cont_t*)(cont))->optional_args == 0)

#define CHECK_NUM_ARGS_OPT(cont, num)                 \
  (((cont_t*)(cont))->num_required_args - 1 <= num && \
   ((cont_t*)(cont))->optional_args)

#define CONTINUE1(cont, val)                            \
  if (GET_PTAG(cont) == PTAG_CONT) {                    \
    if (CHECK_NUM_ARGS(cont, 1)) {                      \
      RAW_CONTINUE1(cont, val);                         \
    } else if (CHECK_NUM_ARGS_OPT(cont, 1)) {           \
      continue_with_opt((cont_t*)cont, 1,               \
                        (lobject)val);                  \
    } else {                                            \
      fprintf(stderr, "Wrong number of arguments.\n");  \
      exit(1);                                          \
    }                                                   \
  } else {                                              \
    fprintf(stderr, "Invalid application.\n");          \
    exit(1);                                            \
  }
#define CONTINUE2(cont, val1, val2)                         \
  if (GET_PTAG(cont) == PTAG_CONT) {                        \
    if (CHECK_NUM_ARGS(cont, 2)) {                          \
      RAW_CONTINUE2(cont, val1, val2);                      \
    } else if (CHECK_NUM_ARGS_OPT(cont, 2)) {               \
      continue_with_opt((cont_t*)cont, 2,                   \
                        (lobject)val1, (lobject)val2);      \
    } else {                                                \
      fprintf(stderr, "Wrong number of arguments.\n");      \
      exit(1);                                              \
    }                                                       \
  } else if (GET_PTAG(cont) == PTAG_OTHER &&                \
             OBJ_TAG(cont) == TAG_CONT_PROC) {              \
    RAW_CONTINUE1(((cont_proc_t*)REM_PTAG(cont))->c, val2); \
  } else {                                                  \
    fprintf(stderr, "Invalid application.\n");              \
    exit(1);                                                \
  }
#define CONTINUE3(cont, val1, val2, val3)               \
  if (GET_PTAG(cont) == PTAG_CONT) {                    \
    if (CHECK_NUM_ARGS(cont, 3)) {                      \
      RAW_CONTINUE3(cont, val1, val2, val3);            \
    } else if (CHECK_NUM_ARGS_OPT(cont, 3)) {           \
      continue_with_opt((cont_t*)cont, 3,               \
                        (lobject)val1,                  \
                        (lobject)val2, (lobject)val3);  \
    } else {                                            \
      fprintf(stderr, "Wrong number of arguments.\n");  \
      exit(1);                                          \
    }                                                   \
  } else {                                              \
    fprintf(stderr, "Invalid application.\n");          \
    exit(1);                                            \
  }
#define CONTINUE4(cont, val1, val2, val3, val4)         \
  if (GET_PTAG(cont) == PTAG_CONT) {                    \
    if (CHECK_NUM_ARGS(cont, 4)) {                      \
      RAW_CONTINUE4(cont, val1, val2, val3, val4);      \
    } else if (CHECK_NUM_ARGS_OPT(cont, 4)) {           \
      continue_with_opt((cont_t*)cont, 4,               \
                        (lobject)val1, (lobject)val2,   \
                        (lobject)val3, (lobject)val4);  \
    } else {                                            \
      fprintf(stderr, "Wrong number of arguments.\n");  \
      exit(1);                                          \
    }                                                   \
  } else {                                              \
    fprintf(stderr, "Invalid application.\n");          \
    exit(1);                                            \
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

void continue_with_opt(cont_t* cont, unsigned int num, ...);
void continue_with_many(cont_t* cont, unsigned int num, ...);

#endif
