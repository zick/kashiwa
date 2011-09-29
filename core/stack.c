#include "stack.h"

#include <stdarg.h>
#include <stdlib.h>

#include "heap.h"

void* stack_bottom;
jmp_buf* entry_point;
thunk_t* restart_thunk;

void check_stack(void* fn, env_t* env, int num, ...) {
  if (NEED_STACK_GC(stack_bottom, &num)) {
    va_list args;
    int i;
    thunk_t* thunk = malloc(sizeof(thunk_t) + sizeof(lobject) * (num - 1));
    thunk->num = num;
    thunk->fn = fn;
    thunk->env = env;
    va_start(args, num);
    for (i = 0; i < num; ++i) {
      thunk->vars[i] = va_arg(args, lobject);
    }
    va_end(args);
    stack_gc(thunk);
    if (restart_thunk) {
      free(restart_thunk);
    }
    restart_thunk = thunk;
    longjmp(*entry_point, 1);
  }
}
