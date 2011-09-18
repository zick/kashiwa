#include "function.h"
#include "object.h"

#include <stdio.h>

void builtin_cons(env_t* env, cont_t* cont, lobject x, lobject y) {
  cons_t ret;
  ret.car = x;
  ret.cdr = y;
  CONTINUE1(cont, ADD_TAG(&ret, TAG_CONS));
}

void builtin_car(env_t* env, cont_t* cont, lobject x) {
  cons_t* c = (cons_t*)REM_TAG(x);
  CONTINUE1(cont, c->car);
}

void builtin_cdr(env_t* env, cont_t* cont, lobject x) {
  cons_t* c = (cons_t*)REM_TAG(x);
  CONTINUE1(cont, c->cdr);
}

static void print_lobject(lobject x) {
  FILE* fp = stdout;  /* TODO */
  switch (GET_TAG(x)) {
  case TAG_NUM:
    {
      int n = NUM2INT(x);
      fprintf(fp, "%d", n);
      break;
    }
  case TAG_CONS:
    {
      cons_t* p = (cons_t*)REM_TAG(x);
      printf("(");
      print_lobject(p->car);
      printf(" . ");
      print_lobject(p->cdr);
      printf(")");
      break;
    }
  default:
    return;  /* Error */
  }
}

void builtin_write(env_t* env, cont_t* cont, lobject x) {
  print_lobject(x);
  CONTINUE1(cont, x);
}

void builtin_newline(env_t* env, cont_t* cont) {
  puts("");
  CONTINUE1(cont, 0);  /* 0 means the undefined value */
}

void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc) {
  cont_proc_t cp;
  cp.tag = TAG_CONT_PROC;
  cp.c = cont;
  CONTINUE2(proc, cont, ADD_TAG(&cp, TAG_OTHER));
}
