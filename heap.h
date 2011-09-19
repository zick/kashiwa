#ifndef __KASHIWA_HEAP__
#define __KASHIWA_HEAP__

#include "function.h"

void init_heap();
void stack_gc(thunk_t* thunk);
void add_heap_rootset(lobject* root);

#endif
