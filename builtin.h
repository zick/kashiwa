#ifndef __KASHIWA_BUILTIN__
#define __KASHIWA_BUILTIN__

#include "function.h"
#include "object.h"

void builtin_cons(env_t* env, cont_t* cont, lobject x, lobject y);
void builtin_car(env_t* env, cont_t* cont, lobject x);
void builtin_cdr(env_t* env, cont_t* cont, lobject x);
void builtin_write(env_t* env, cont_t* cont, lobject x);
void builtin_newline(env_t* env, cont_t* cont);
void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc);

#endif
