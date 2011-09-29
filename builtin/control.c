#include "../core/function.h"
#include "builtin-util.h"
#include "builtin.h"

void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc) {
  cont_proc_t cp;
  cp.tag = TAG_CONT_PROC;
  cp.c = cont;
  CONTINUE2(proc, cont, ADD_PTAG(&cp, PTAG_OTHER));
}
