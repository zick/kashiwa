#include "heap.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

#define INITIAL_HEAP_ENTRY 131072
#define INITIAL_HEAP_ROOTSET_ENTRY 256

static lobject* heap;
static size_t heap_size;
static lobject* heap_end;
static lobject* heap_from;
static lobject* heap_to;
static lobject* heap_free;
static lobject* heap_scan_start;

static lobject** heap_rootset;
static size_t heap_rootset_size;
static lobject** heap_rootset_end;
static lobject** heap_rootset_free;

static int in_heap_gc;

void init_heap() {
  heap_size = INITIAL_HEAP_ENTRY * sizeof(lobject);
  heap = heap_free = (lobject*)malloc(heap_size);
  heap_end = heap + INITIAL_HEAP_ENTRY;
  heap_from = heap;
  heap_to = heap + heap_size / sizeof(lobject) / 2;

  heap_rootset_size = INITIAL_HEAP_ROOTSET_ENTRY * sizeof(lobject*);
  heap_rootset = heap_rootset_free = (lobject**)malloc(heap_rootset_size);
  heap_rootset_end = heap_rootset + INITIAL_HEAP_ROOTSET_ENTRY;

  in_heap_gc = 0;
}

static int in_target_space(void* p) {
  if (!in_heap_gc) {  /* stack gc */
    if (p < (void*)heap_from || (void*)heap_end <= p) {
      return 1;
    }
    return 0;
  }
  if ((heap_from < heap_to && (void*)heap_from <= p && p < (void*)heap_to) ||
      (heap_from > heap_to && (void*)heap_from <= p && p < (void*)heap_end)) {
    return 1;
  }
  return 0;
}

static lobject copy_other_object(void* p) {
  lobject ret;
  size_t size;
  if (!in_target_space(p)) {
    return (lobject)ADD_PTAG(p, PTAG_OTHER);
  }
  switch (OBJ_TAG(p)) {
  case TAG_CONT_PROC:
    size = sizeof(cont_proc_t);
    break;
  case TAG_SYMBOL:
    size = sizeof(symbol_t);
    break;
  case TAG_FORWARDING:
    return ADD_PTAG(FORWARDING_ADDRESS(p), PTAG_OTHER);
  default:
    assert(0);
  }
  /* TODO: heap available size check */
  memcpy(heap_free, p, size);
  ret = ADD_PTAG(heap_free, PTAG_OTHER);
  heap_free += size / sizeof(lobject);
  return ret;
}

static lobject copy_lobject(lobject x) {
  lobject ret;
  size_t size;
  void* p;
  switch (GET_PTAG(x)) {
  case PTAG_CONT:
    size = sizeof(cont_t);
    break;
  case PTAG_CONS:
    size = sizeof(cons_t);
    break;
  case PTAG_FIXNUM:
    return x;
  case PTAG_OTHER:
    return copy_other_object((void*)REM_PTAG(x));
  default:
    assert(0);
  }
  p = (void*)REM_PTAG(x);
  if (!in_target_space(p)) {
    return x;
  }
  if (OBJ_TAG(p) == TAG_FORWARDING) {
    return ADD_PTAG(FORWARDING_ADDRESS(p), GET_PTAG(x));
  }
  /* TODO: heap available size check */
  memcpy(heap_free, p, size);
  OBJ_TAG(p) = TAG_FORWARDING;
  FORWARDING_ADDRESS(p) = heap_free;
  ret = ADD_PTAG(heap_free, GET_PTAG(x));
  heap_free += size / sizeof(lobject);
  return ret;
}

static env_t* copy_env(env_t* env) {
  env_t* ret;
  size_t size;
  if (env == NULL || (heap <= (lobject*)env && (lobject*)env < heap_end)) {
    return env;
  }
  if (OBJ_TAG(env) == TAG_FORWARDING) {
    return FORWARDING_ADDRESS(env);
  }
  size = sizeof(env_t) + sizeof(lobject) * (env->num - 1);
  /* TODO: heap available size check */
  memcpy(heap_free, env, size);
  OBJ_TAG(env) = TAG_FORWARDING;
  FORWARDING_ADDRESS(env) = heap_free;
  ret = (env_t*)heap_free;
  heap_free += size / sizeof(lobject);
  return ret;
}

static void scan_heap_for_stack_gc() {
  lobject* p;
  int i;
  for (p = heap_scan_start; p < heap_free;) {
    size_t size;
    switch (OBJ_TAG(p)) {
    case TAG_CONT:
      size = sizeof(cont_t);
      ((cont_t*)p)->env = copy_env(((cont_t*)p)->env);
      break;
    case TAG_CONS:
      size = sizeof(cons_t);
      ((cons_t*)p)->car = copy_lobject(((cons_t*)p)->car);
      ((cons_t*)p)->cdr = copy_lobject(((cons_t*)p)->cdr);
      break;
    case TAG_ENV:
      size = sizeof(env_t) + sizeof(lobject) * (((env_t*)p)->num - 1);
      ((env_t*)p)->link = copy_env(((env_t*)p)->link);
      for (i = 0; i < ((env_t*)p)->num; ++i) {
        ((env_t*)p)->vars[i] = copy_lobject(((env_t*)p)->vars[i]);
      }
      break;
    case TAG_CONT_PROC:
      size = sizeof(cont_proc_t);
      ((cont_proc_t*)p)->c =
        (void*)copy_lobject((lobject)((cont_proc_t*)p)->c);
      break;
    case TAG_SYMBOL:
      size = sizeof(symbol_t);
      break;
    default:
      assert(0);
    }
    p += size / sizeof(lobject);
  }
}

void stack_gc(thunk_t* thunk) {
  int i;
  lobject** p;
  heap_scan_start = heap_free;
  for (p = heap_rootset; p < heap_rootset_free; ++p) {
    if (**p) {
      **p = copy_lobject(**p);
    }
  }
  thunk->env = copy_env(thunk->env);
  for (i = 0; i < thunk->num; ++i) {
    thunk->vars[i] = copy_lobject(thunk->vars[i]);
  }
  scan_heap_for_stack_gc();
}

void add_heap_rootset(lobject* root) {
  if (heap_rootset_free == heap_rootset_end) {
    heap_rootset = (lobject**)realloc(heap_rootset, heap_rootset_size * 2);
    if (!heap_rootset) {
      fprintf(stderr, "heap rootset is full\n");
      exit(1);
    }
    heap_rootset_free = heap_rootset + heap_rootset_size / sizeof(lobject*);
    heap_rootset_size *= 2;
    heap_rootset_end = heap_rootset + heap_rootset_size / sizeof(lobject*);
  }
  *heap_rootset_free = root;
  ++heap_rootset_free;
}
