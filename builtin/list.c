#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/function.h"
#include "builtin-util.h"
#include "builtin.h"

void builtin_cons(env_t* env, cont_t* cont, lobject x, lobject y) {
  cons_t ret;
  ret.tag = TAG_CONS;
  ret.car = x;
  ret.cdr = y;
  RAW_CONTINUE1(cont, ADD_PTAG(&ret, PTAG_CONS));
}

void builtin_car(env_t* env, cont_t* cont, lobject x) {
  if (GET_PTAG(x) != PTAG_CONS) {
    fprintf(stderr, "pair required\n");
    exit(1);
  }
  RAW_CONTINUE1(cont, ((cons_t*)REM_PTAG(x))->car);
}

void builtin_cdr(env_t* env, cont_t* cont, lobject x) {
  if (GET_PTAG(x) != PTAG_CONS) {
    fprintf(stderr, "pair required\n");
    exit(1);
  }
  RAW_CONTINUE1(cont, ((cons_t*)REM_PTAG(x))->cdr);
}

void builtin_list(env_t* env, unsigned int num_args, ...) {
  va_list args;
  cont_t* cont;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  if (num_args == 1) {
    va_end(args);
    CONTINUE1(cont, nil);
  } else {
    int i;
    cons_t* p;
    p = alloca(sizeof(cons_t) * (num_args - 1));
    for (i = 1; i < num_args; ++i) {
      p[i - 1].tag = TAG_CONS;
      p[i - 1].car = va_arg(args, lobject);
      if (i == num_args - 1) {
        p[i - 1].cdr = nil;
      } else {
        p[i - 1].cdr = ADD_PTAG(p + i, PTAG_CONS);
      }
    }
    va_end(args);
    CONTINUE1(cont, ADD_PTAG(p, PTAG_CONS));
  }
}

void builtin_list_list(env_t* env, lobject cont, lobject opt) {
  if (opt == nil) {
    CONTINUE1(cont, nil);
  } else {
    cons_t* p;
    int num_oargs, i;
    num_oargs = list_length(opt);
    p = alloca(sizeof(cons_t) * (num_oargs));
    for (i = 0; i < num_oargs; ++i) {
      p[i].tag = TAG_CONS;
      p[i].car = ((cons_t*)REM_PTAG(opt))->car;
      if (i == num_oargs - 1) {
        p[i].cdr = nil;
      } else {
        p[i].cdr = ADD_PTAG(p + i + 1, PTAG_CONS);
      }
      opt = ((cons_t*)REM_PTAG(opt))->cdr;
    }
    CONTINUE1(cont, ADD_PTAG(p, PTAG_CONS));
  }
}

void builtin_liststar(env_t* env, unsigned int num_args, ...) {
  va_list args;
  cont_t* cont;
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  if (num_args == 1) {
    va_end(args);
    CONTINUE1(cont, nil);
  } else if (num_args == 2) {
    lobject ret;
    ret = va_arg(args, lobject);
    va_end(args);
    CONTINUE1(cont, ret);
  } else {
    int i;
    cons_t* p;
    p = alloca(sizeof(cons_t) * (num_args - 2));
    for (i = 1; i < num_args - 1; ++i) {
      p[i - 1].tag = TAG_CONS;
      p[i - 1].car = va_arg(args, lobject);
      if (i < num_args - 2) {
        p[i - 1].cdr = ADD_PTAG(p + i, PTAG_CONS);
      }
    }
    p[num_args - 3].cdr = va_arg(args, lobject);
    va_end(args);
    CONTINUE1(cont, ADD_PTAG(p, PTAG_CONS));
  }
}

void builtin_list_liststar(env_t* env, lobject cont, lobject opt) {
  int num_oargs;
  num_oargs = list_length(opt);
  if (opt == nil) {
    CONTINUE1(cont, nil);
  } else if (num_oargs == 1) {
    CONTINUE1(cont, ((cons_t*)REM_PTAG(opt))->car);
  } else {
    int i;
    cons_t* p;
    p = alloca(sizeof(cons_t) * (num_oargs - 1));
    for (i = 0; i < num_oargs - 1; ++i) {
      p[i].tag = TAG_CONS;
      p[i].car = ((cons_t*)REM_PTAG(opt))->car;
      if (i < num_oargs - 2) {
        p[i].cdr = ADD_PTAG(p + i + 1, PTAG_CONS);
      }
      opt = ((cons_t*)REM_PTAG(opt))->cdr;
    }
    p[num_oargs - 2].cdr = ((cons_t*)REM_PTAG(opt))->car;
    CONTINUE1(cont, ADD_PTAG(p, PTAG_CONS));
  }
}
