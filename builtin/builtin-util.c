#include "builtin-util.h"

extern lobject nil;

int list_length(lobject x) {  /* Not detect circular lists */
  int ret;
  if (x == nil) {
    return 0;
  }
  PTAG_CHECK(x, PTAG_CONS);
  ret = 0;
  do {
    ++ret;
    x = ((cons_t*)REM_PTAG(x))->cdr;
  } while (GET_PTAG(x) == PTAG_CONS);
  return ret;
}
