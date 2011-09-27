#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "function.h"
#include "object.h"

#define VA_ARG_PTAG_CHECK(args, var, ptag)      \
  {                                             \
    (var) = va_arg((args), lobject);            \
    if (GET_PTAG(var) != (ptag)) {              \
      fprintf(stderr, "Invalid type.\n");       \
      exit(1);                                  \
    }                                           \
  }

lobject sharpt;
lobject sharpf;
lobject nil;

void init_builtin() {
  boolean_t *t, *f;
  nil_t* n;
  t = (boolean_t*)malloc(sizeof(boolean_t));
  f = (boolean_t*)malloc(sizeof(boolean_t));
  t->tag = f->tag = TAG_BOOLEAN;
  t->bool = 1;
  f->bool = 0;
  sharpt = ADD_PTAG(t, PTAG_OTHER);
  sharpf = ADD_PTAG(f, PTAG_OTHER);
  n = (nil_t*)malloc(sizeof(nil_t));
  n->tag = TAG_NIL;
  nil = ADD_PTAG(n, PTAG_OTHER);
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
  case TAG_NIL:
    fprintf(fp, "()");
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

void builtin_write(env_t* env, unsigned int num_args, ...) {
  va_list args;
  cont_t* cont;
  lobject x, port;
  if (num_args > 3) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(1);
  }
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  x = va_arg(args, lobject);
  if (num_args == 3) {
    port = va_arg(args, lobject);  /* TODO */
  }
  va_end(args);
  print_lobject(x);
  RAW_CONTINUE1(cont, x);
}

void builtin_newline(env_t* env, unsigned int num_args, ...) {
  va_list args;
  cont_t* cont;
  lobject port;
  if (num_args > 2) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(1);
  }
  va_start(args, num_args);
  cont = va_arg(args, cont_t*);
  if (num_args  == 2) {
    port = va_arg(args, lobject);  /* TODO */
  }
  va_end(args);
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
