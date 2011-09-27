#include "function.h"

#include <alloca.h>
#include <assert.h>
#include <stdarg.h>

extern lobject nil;

static void do_continue_with_opt(cont_t* cont, unsigned int num,
                                 va_list args) {
  lobject *rargs, lastarg;
  cons_t* oargs;
  int i, num_rargs, num_oargs;
  num_rargs = cont->num_required_args - 1;
  num_oargs = num - num_rargs;
  rargs = (lobject*)alloca(sizeof(lobject) * num_rargs);
  for (i = 0; i < num_rargs; ++i) {
    rargs[i] = va_arg(args, lobject);
  }
  if (num_oargs > 0) {
    oargs = (cons_t*)alloca(sizeof(cons_t) * num_oargs);
    lastarg = ADD_PTAG(oargs, PTAG_CONS);
  } else {
    lastarg = nil;
  }
  for (i = 0; i < num_oargs; ++i) {
    oargs[i].tag = TAG_CONS;
    oargs[i].car = va_arg(args, lobject);
    if (i == num_oargs - 1) {
      oargs[i].cdr = nil;
    } else {
      oargs[i].cdr = ADD_PTAG(oargs + i + 1, PTAG_CONS);
    }
  }
  va_end(args);
  switch (num_rargs + 1) {
  case 1:
    RAW_CONTINUE1(cont, lastarg);
    break;
  case 2:
    RAW_CONTINUE2(cont, rargs[0], lastarg);
    break;
  case 3:
    RAW_CONTINUE3(cont, rargs[0], rargs[1], lastarg);
    break;
  case 4:
    RAW_CONTINUE4(cont, rargs[0], rargs[1], rargs[2], lastarg);
    break;
  default:
    assert(0);
  }
}

void continue_with_opt(cont_t* cont, unsigned int num, ...) {
  va_list args;
  va_start(args, num);
  do_continue_with_opt(cont, num, args);
}

void continue_with_many(cont_t* cont, unsigned int num, ...) {
  va_list args;
  env_t* args_env;
  int i, num_rargs, num_oargs;
  va_start(args, num);
  if (cont->num_required_args <= NUM_ARGUMENTS_LIMIT) {
    return do_continue_with_opt(cont, num, args);
  }
  num_rargs = cont->num_required_args - cont->optional_args;
  num_oargs = num - num_rargs;
  if (num < num_rargs || (num > num_rargs && !cont->optional_args)) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(1);
  }
  args_env = (env_t*)
    alloca(sizeof(env_t) + sizeof(lobject) * (cont->num_required_args - 1));
  args_env->tag = TAG_ENV;
  args_env->num = cont->num_required_args;
  args_env->link = NULL;
  for (i = 0; i < num_rargs; ++i) {
    args_env->vars[i] = va_arg(args, lobject);
  }
  if (cont->optional_args && num_oargs == 0) {
    args_env->vars[num_rargs] = nil;
  } else if (cont->optional_args) {
    cons_t* oargs;
    oargs = (cons_t*)alloca(sizeof(cons_t) * num_oargs);
    for (i = 0; i < num_oargs; ++i) {
      oargs[i].tag = TAG_CONS;
      oargs[i].car = va_arg(args, lobject);
      if (i == num_oargs - 1) {
        oargs[i].cdr = nil;
      } else {
        oargs[i].cdr = ADD_PTAG(oargs + i + 1, PTAG_CONS);
      }
    }
    args_env->vars[num_rargs] = (lobject)ADD_PTAG(oargs, PTAG_CONS);
  }
  va_end(args);
  RAW_CONTINUE1(cont, args_env);
}
