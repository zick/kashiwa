#ifndef __KASHIWA_BUILTIN_UTIL__
#define __KASHIWA_BUILTIN_UTIL__

#include <stdio.h>
#include <stdlib.h>

#include "../core/object.h"

#define VA_ARG_PTAG_CHECK(args, var, ptag)      \
  {                                             \
    (var) = va_arg((args), lobject);            \
    if (GET_PTAG(var) != (ptag)) {              \
      fprintf(stderr, "Invalid type.\n");       \
      exit(1);                                  \
    }                                           \
  }

#define PTAG_CHECK(var, ptag)                   \
  if (GET_PTAG(var) != (ptag)) {                \
    fprintf(stderr, "Invalid type.\n");         \
    exit(1);                                    \
  }

#define MAKE_BUILTIN_CLOS(clos, fun, req, opt)  \
  {                                             \
  cont_t* p;                                    \
  p = (cont_t*)malloc(sizeof(cont_t));          \
  p->tag = TAG_CONT;                            \
  p->fn = (function1_t)fun;                     \
  p->env = NULL;                                \
  p->num_required_args = req;                   \
  p->optional_args = opt;                       \
  clos = (lobject)ADD_PTAG(p, TAG_CONT);        \
  }

int list_length(lobject x);

/* 6.2.5. Numerical operations */
void builtin_list_eql(env_t* env, lobject cont,
                      lobject x, lobject y, lobject opt);
void builtin_list_plus(env_t* env, lobject cont, lobject opt);
void builtin_list__(env_t* env, lobject cont, lobject x, lobject opt);
void builtin_list_star(env_t* env, lobject cont, lobject opt);
void builtin_list_sla(env_t* env, lobject cont, lobject x, lobject opt);
/* 6.3.2. Pairs and lists */
void builtin_list_list(env_t* env, lobject cont, lobject opt);
void builtin_list_liststar(env_t* env, lobject cont, lobject opt);
/* 6.6.3. Output */
void builtin_list_write(env_t* env, lobject cont, lobject x, lobject opt);
void builtin_list_newline(env_t* env, lobject cont, lobject opt);

#endif
