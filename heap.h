#ifndef __KASHIWA_HEAP__
#define __KASHIWA_HEAP__

#include "function.h"

#define SET_WITH_BARRIER(obj, field, val)                               \
  if (heap < (unsigned char*)(obj) && (unsigned char*)(obj) < heap_end) { \
    write_barrier((void*)REM_PTAG(obj), (field), (val));                \
    field = (lobject)(val);                                            \
  } else {                                                              \
    field = (lobject)(val);                                             \
  }

extern unsigned char* heap;
extern unsigned char* heap_end;

void init_heap();
void stack_gc(thunk_t* thunk);
void add_heap_rootset(lobject* root);
void write_barrier(void* obj, lobject oldval, lobject nextval);

#endif
