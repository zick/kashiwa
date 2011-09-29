#ifndef __KASHIWA_STACK__
#define __KASHIWA_STACK__

#include "function.h"
#include "object.h"

#include <setjmp.h>

#define STACK_GC_THRESHOLD 1048576

/* This macro assumes that the stack grow from higher to lower */
#define NEED_STACK_GC(bottom, top) \
  (((char*)(bottom) - (char*)(top)) > STACK_GC_THRESHOLD)

extern void* stack_bottom;
extern jmp_buf* entry_point;
extern thunk_t* restart_thunk;

void check_stack(void* fn, env_t* env, int num, ...);

#endif
