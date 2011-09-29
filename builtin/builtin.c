#include "builtin.h"

#include <stdlib.h>

#include "../core/function.h"
#include "builtin-util.h"

lobject sharpt;
lobject sharpf;
lobject nil;

/* 6.2.5. Numerical operations */
lobject builtin_clos_eql;
lobject builtin_clos_plus;
lobject builtin_clos__;
lobject builtin_clos_star;
lobject builtin_clos_sla;
/* 6.3.2. Pairs and lists */
lobject builtin_clos_cons;
lobject builtin_clos_car;
lobject builtin_clos_cdr;
lobject builtin_clos_list;
lobject builtin_clos_liststar;
/* 6.4. Control features */
lobject builtin_clos_call_with_current_continuation;
/* 6.6.3. Output */
lobject builtin_clos_write;
lobject builtin_clos_newline;

void init_builtin_clos() {
  MAKE_BUILTIN_CLOS(builtin_clos_cons, builtin_cons, 3, 0);
  MAKE_BUILTIN_CLOS(builtin_clos_car, builtin_car, 2, 0);
  MAKE_BUILTIN_CLOS(builtin_clos_cdr, builtin_cdr, 2, 0);
  MAKE_BUILTIN_CLOS(builtin_clos_write, builtin_list_write, 3, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_newline, builtin_list_newline, 2, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_call_with_current_continuation,
                    builtin_call_with_current_continuation, 3, 0);
  MAKE_BUILTIN_CLOS(builtin_clos_eql, builtin_list_eql, 4, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_plus, builtin_list_plus, 2, 1);
  MAKE_BUILTIN_CLOS(builtin_clos__, builtin_list__, 3, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_star, builtin_list_star, 2, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_sla, builtin_list_sla, 3, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_list, builtin_list_list, 2, 1);
  MAKE_BUILTIN_CLOS(builtin_clos_liststar, builtin_list_liststar, 2, 1);
}

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
  init_builtin_clos();
}
