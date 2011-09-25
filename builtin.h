#ifndef __KASHIWA_BUILTIN__
#define __KASHIWA_BUILTIN__

#include "function.h"
#include "object.h"

#define BOOLTEST(x) \
  (GET_PTAG(x) != PTAG_OTHER || OBJ_TAG(x) != TAG_BOOLEAN || \
   ((boolean_t*)REM_PTAG(x))->bool)

extern lobject sharpt;
extern lobject sharpf;

void init_builtin();
void builtin_cons(env_t* env, cont_t* cont, lobject x, lobject y);
void builtin_car(env_t* env, cont_t* cont, lobject x);
void builtin_cdr(env_t* env, cont_t* cont, lobject x);
void builtin_write(env_t* env, unsigned int num_args, ...);
void builtin_newline(env_t* env, unsigned int num_args, ...);
void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc);

#endif
