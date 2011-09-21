#include <stdio.h>
#include <stdlib.h>

#include "function.h"
#include "object.h"

lobject sharpt;
lobject sharpf;

void init_builtin() {
  boolean_t *t, *f;
  t = (boolean_t*)malloc(sizeof(boolean_t));
  f = (boolean_t*)malloc(sizeof(boolean_t));
  t->tag = f->tag = TAG_BOOLEAN;
  t->bool = 1;
  f->bool = 0;
  sharpt = ADD_PTAG(t, PTAG_OTHER);
  sharpf = ADD_PTAG(f, PTAG_OTHER);
}

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

static void print_other_object(void* obj) {
  FILE* fp = stdout;  /* TODO */
  switch (OBJ_TAG(obj)) {
  case TAG_SYMBOL:
    fprintf(fp, "%s", ((symbol_t*)obj)->name);
    break;
  case TAG_BOOLEAN:
    if (obj == (void*)REM_PTAG(sharpf)) {
      fprintf(fp, "#f");
    } else {
      fprintf(fp, "#t");
    }
    break;
  default:
    fprintf(stderr, "Not printable object\n");
    exit(1);
  }
}

static void print_lobject(lobject x) {
  FILE* fp = stdout;  /* TODO */
  switch (GET_PTAG(x)) {
  case PTAG_FIXNUM:
    fprintf(fp, "%d", FIXNUM2INT(x));
    break;
  case PTAG_CONS:
    printf("(");
    print_lobject(((cons_t*)REM_PTAG(x))->car);
    printf(" . ");
    print_lobject(((cons_t*)REM_PTAG(x))->cdr);
    printf(")");
    break;
  case PTAG_OTHER:
    print_other_object((void*)REM_PTAG(x));
    break;
  default:
    fprintf(stderr, "Not printable object\n");
    exit(1);
  }
}

void builtin_write(env_t* env, cont_t* cont, lobject x) {
  print_lobject(x);
  RAW_CONTINUE1(cont, x);
}

void builtin_newline(env_t* env, cont_t* cont) {
  puts("");
  RAW_CONTINUE1(cont, 0);  /* 0 means the undefined value */
}

void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc) {
  cont_proc_t cp;
  cp.tag = TAG_CONT_PROC;
  cp.c = cont;
  CONTINUE2(proc, cont, ADD_PTAG(&cp, PTAG_OTHER));
}
