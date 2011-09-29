#include <stdarg.h>

#include "../core/function.h"
#include "builtin-util.h"
#include "builtin.h"

void builtin_eql(env_t* env, unsigned int num_args, ...) {  /* procedure = */
  va_list args;
  cont_t* cont;
  int i, val, eql = 1;
  lobject x;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
  val = FIXNUM2INT(x);
  for (i = 2; i < num_args; ++i) {
    VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
    if (val != FIXNUM2INT(x)) {
      eql = 0;
    }
  }
  va_end(args);
  if (eql) {
    CONTINUE1(cont, sharpt);
  } else {
    CONTINUE1(cont, sharpf);
  }
}

void builtin_list_eql(env_t* env, lobject cont,
                      lobject x, lobject y, lobject opt) {
  int val, eql = 1;
  PTAG_CHECK(x, PTAG_FIXNUM);
  PTAG_CHECK(y, PTAG_FIXNUM);
  val = FIXNUM2INT(x);
  if (val != FIXNUM2INT(y)) {
    eql = 0;
  }
  while (eql && GET_PTAG(opt) == PTAG_CONS) {
    lobject z;
    z = ((cons_t*)REM_PTAG(opt))->car;
    PTAG_CHECK(z, PTAG_FIXNUM);
    if (val != FIXNUM2INT(z)) {
      eql = 0;
    }
    opt = ((cons_t*)REM_PTAG(opt))->cdr;
  }
  if (eql) {
    CONTINUE1(cont, sharpt);
  } else {
    CONTINUE1(cont, sharpf);
  }
}

void builtin_plus(env_t* env, unsigned int num_args, ...) {  /* procedure + */
  va_list args;
  cont_t* cont;
  int i, sum = 0;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  for (i = 1; i < num_args; ++i) {
    lobject x;
    VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
    sum += FIXNUM2INT(x);
  }
  va_end(args);
  CONTINUE1(cont, INT2FIXNUM(sum));
}

void builtin_list_plus(env_t* env, lobject cont, lobject opt) {
  int sum = 0;
  while (GET_PTAG(opt) == PTAG_CONS) {
    lobject x;
    x = ((cons_t*)REM_PTAG(opt))->car;
    PTAG_CHECK(x, PTAG_FIXNUM);
    sum += FIXNUM2INT(x);
    opt = ((cons_t*)REM_PTAG(opt))->cdr;
  }
  CONTINUE1(cont, INT2FIXNUM(sum));
}

void builtin__(env_t* env, unsigned int num_args, ...) {  /* procedure - */
  va_list args;
  cont_t* cont;
  int i, sum;
  lobject x;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
  sum = FIXNUM2INT(x);
  if (num_args == 2) {
    va_end(args);
    CONTINUE1(cont, INT2FIXNUM(-sum));
  } else {
    for (i = 2; i < num_args; ++i) {
      VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
      sum -= FIXNUM2INT(x);
    }
    va_end(args);
    CONTINUE1(cont, INT2FIXNUM(sum));
  }
}

void builtin_list__(env_t* env, lobject cont, lobject x, lobject opt) {
  int sum;
  PTAG_CHECK(x, PTAG_FIXNUM);
  sum = FIXNUM2INT(x);
  if (opt == nil) {
    CONTINUE1(cont, INT2FIXNUM(-sum));
  } else {
    while (GET_PTAG(opt) == PTAG_CONS) {
      x = ((cons_t*)REM_PTAG(opt))->car;
      PTAG_CHECK(x, PTAG_FIXNUM);
      sum -= FIXNUM2INT(x);
      opt = ((cons_t*)REM_PTAG(opt))->cdr;
    }
    CONTINUE1(cont, INT2FIXNUM(sum));
  }
}

void builtin_star(env_t* env, unsigned int num_args, ...) {  /* procedure * */
  va_list args;
  cont_t* cont;
  int i, sum = 1;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  for (i = 1; i < num_args; ++i) {
    lobject x;
    VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
    sum *= FIXNUM2INT(x);
  }
  va_end(args);
  CONTINUE1(cont, INT2FIXNUM(sum));
}

void builtin_list_star(env_t* env, lobject cont, lobject opt) {
  int sum = 1;
  while (GET_PTAG(opt) == PTAG_CONS) {
    lobject x;
    x = ((cons_t*)REM_PTAG(opt))->car;
    PTAG_CHECK(x, PTAG_FIXNUM);
    sum *= FIXNUM2INT(x);
    opt = ((cons_t*)REM_PTAG(opt))->cdr;
  }
  CONTINUE1(cont, INT2FIXNUM(sum));
}

void builtin_sla(env_t* env, unsigned int num_args, ...) {  /* procedure / */
  va_list args;
  cont_t* cont;
  int i, sum;
  lobject x;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
  sum = FIXNUM2INT(x);
  if (num_args == 2) {
    va_end(args);
    CONTINUE1(cont, INT2FIXNUM(1 / sum));
  } else {
    for (i = 2; i < num_args; ++i) {
      VA_ARG_PTAG_CHECK(args, x, PTAG_FIXNUM);
      sum /= FIXNUM2INT(x);
    }
    va_end(args);
    CONTINUE1(cont, INT2FIXNUM(sum));
  }
}

void builtin_list_sla(env_t* env, lobject cont, lobject x, lobject opt) {
  int sum;
  PTAG_CHECK(x, PTAG_FIXNUM);
  sum = FIXNUM2INT(x);
  if (opt == nil) {
    CONTINUE1(cont, INT2FIXNUM(1 / sum));
  } else {
    while (GET_PTAG(opt) == PTAG_CONS) {
      x = ((cons_t*)REM_PTAG(opt))->car;
      PTAG_CHECK(x, PTAG_FIXNUM);
      sum /= FIXNUM2INT(x);
      opt = ((cons_t*)REM_PTAG(opt))->cdr;
    }
    CONTINUE1(cont, INT2FIXNUM(sum));
  }
}
