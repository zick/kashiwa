#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../core/function.h"
#include "builtin-util.h"
#include "builtin.h"

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

void builtin_list_write(env_t* env, lobject cont, lobject x, lobject opt) {
  int num_oargs;
  lobject port;
  num_oargs = list_length(opt);
  if (num_oargs > 1) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(1);
  }
  if (num_oargs == 1) {
    port = ((cons_t*)REM_PTAG(opt))->car;  /* TODO */
  }
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

void builtin_list_newline(env_t* env, lobject cont, lobject opt) {
  int num_oargs;
  lobject port;
  num_oargs = list_length(opt);
  if (num_oargs > 1) {
    fprintf(stderr, "Wrong number of arguments.\n");
    exit(1);
  }
  if (num_oargs  == 1) {
    port = ((cons_t*)REM_PTAG(opt))->car;  /* TODO */
  }
  puts("");
  RAW_CONTINUE1(cont, 0);  /* 0 means the undefined value */
}
