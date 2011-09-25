#include "heap.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

#define INITIAL_HEAP_SIZE 1048576
#define INITIAL_HEAP_ROOTSET_ENTRY 256
#define INITIAL_HEAP_REMSET_ENTRY 256
#define FREELIST_ELM(x) ((lobject)(x) & 1)
#define FREELIST_NEXT(x) ((lobject)(x) & ~1)
#define MAKE_FREELIST_ELM(x, next) *((lobject*)(x)) = ((lobject)(next) | 1)

unsigned char* heap;
unsigned char* heap_end;
static unsigned char* heap_from;
static unsigned char* heap_to;
static unsigned char* heap_free;
static unsigned char* heap_scan_start;
static size_t heap_size;

static lobject** heap_rootset;
static lobject** heap_rootset_end;
static lobject** heap_rootset_free;
static size_t heap_rootset_size;

static void** heap_remset;
static void** heap_remset_end;
static void** heap_remset_free;
static void** heap_remset_tail;
static size_t heap_remset_size;

static int in_heap_gc;

static jmp_buf gc_entry_point;

void init_heap() {
  heap_size = INITIAL_HEAP_SIZE;
  heap = heap_free = (unsigned char*)malloc(heap_size);
  heap_end = heap + heap_size;
  heap_from = heap;
  heap_to = heap + heap_size / 2;

  heap_rootset_size = INITIAL_HEAP_ROOTSET_ENTRY * sizeof(lobject*);
  heap_rootset = heap_rootset_free = (lobject**)malloc(heap_rootset_size);
  heap_rootset_end = heap_rootset + INITIAL_HEAP_ROOTSET_ENTRY;

  heap_remset_size = INITIAL_HEAP_REMSET_ENTRY * sizeof(void*);
  heap_remset = heap_remset_free = (void**)malloc(heap_remset_size);
  heap_remset_end = heap_remset + INITIAL_HEAP_REMSET_ENTRY;
  heap_remset_tail = heap_remset_free;

  in_heap_gc = 0;
}

static int in_target_space(void* p) {
  int dummy;
  /* assume that the stack is allocated at the end of the memory. */
  if ((void*)&dummy < p) {  /* stack and heap gc */
    return 1;
  }
  if (!in_heap_gc) {  /* stack gc only */
    return 0;
  }
  /* heap gc only */
  if ((heap_from < heap_to && (void*)heap_from <= p && p < (void*)heap_to) ||
      (heap_from > heap_to && (void*)heap_from <= p && p < (void*)heap_end)) {
    return 1;
  }
  return 0;
}

static lobject copy_lobject(lobject x);

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
    if (in_heap_gc && in_target_space(FORWARDING_ADDRESS(p))) {
      FORWARDING_ADDRESS(p) = (void*)
        REM_PTAG(copy_lobject(ADD_PTAG(FORWARDING_ADDRESS(p), PTAG_OTHER)));
    }
    return ADD_PTAG(FORWARDING_ADDRESS(p), PTAG_OTHER);
  default:
    assert(0);
  }
#ifdef GC_VERBOSE
  fprintf(stderr, "copy %p -> %p (tag:%x)\n", p, heap_free, (int)OBJ_TAG(p));
#endif
  if (!in_heap_gc && heap_free + size >= heap_from + heap_size / 2) {
    longjmp(gc_entry_point, 1);
  } else if (in_heap_gc && heap_free + size >= heap_to + heap_size / 2) {
    /* TODO: realloc */
    fprintf(stderr, "heap is full.\n");
    exit(1);
  }
  memcpy(heap_free, p, size);
  OBJ_TAG(p) = TAG_FORWARDING;
  FORWARDING_ADDRESS(p) = heap_free;
  ret = ADD_PTAG(heap_free, PTAG_OTHER);
  heap_free += size;
  return ret;
}

static env_t* copy_env(env_t* env);

static lobject copy_lobject(lobject x) {
  lobject ret;
  size_t size;
  void* p;
  switch (GET_PTAG(x)) {
  case PTAG_CONT:
    if (OBJ_TAG(x) == TAG_ENV) {  /* Not tagged environment can be come. */
      return (lobject)copy_env((env_t*)x);
    }
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
    if (in_heap_gc && in_target_space(FORWARDING_ADDRESS(p))) {
      FORWARDING_ADDRESS(p) = (void*)
        REM_PTAG(copy_lobject(ADD_PTAG(FORWARDING_ADDRESS(p), GET_PTAG(x))));
    }
    return ADD_PTAG(FORWARDING_ADDRESS(p), GET_PTAG(x));
  }
#ifdef GC_VERBOSE
  fprintf(stderr, "copy %p -> %p (tag:%x)\n", p, heap_free, (int)OBJ_TAG(x));
#endif
  if (!in_heap_gc && heap_free + size >= heap_from + heap_size / 2) {
    longjmp(gc_entry_point, 1);
  } else if (in_heap_gc && heap_free + size >= heap_to + heap_size / 2) {
    /* TODO: realloc */
    fprintf(stderr, "heap is full.\n");
    exit(1);
  }
  memcpy(heap_free, p, size);
  OBJ_TAG(p) = TAG_FORWARDING;
  FORWARDING_ADDRESS(p) = heap_free;
  ret = ADD_PTAG(heap_free, GET_PTAG(x));
  heap_free += size;
  return ret;
}

static env_t* copy_env(env_t* env) {
  env_t* ret;
  size_t size;
  if (env == NULL || !in_target_space(env)) {
    return env;
  }
  if (OBJ_TAG(env) == TAG_FORWARDING) {
    if (in_heap_gc && in_target_space(FORWARDING_ADDRESS(env))) {
      FORWARDING_ADDRESS(env) = (void*)copy_env(FORWARDING_ADDRESS(env));
    }
    return FORWARDING_ADDRESS(env);
  }
  size = sizeof(env_t) + sizeof(lobject) * (env->num - 1);
#ifdef GC_VERBOSE
  fprintf(stderr, "copy %p -> %p (env)\n", env, heap_free);
#endif
  if (!in_heap_gc && heap_free + size >= heap_from + heap_size / 2) {
    longjmp(gc_entry_point, 1);
  } else if (in_heap_gc && heap_free + size >= heap_to + heap_size / 2) {
    /* TODO: realloc */
    fprintf(stderr, "heap is full.\n");
    exit(1);
  }
  memcpy(heap_free, env, size);
  OBJ_TAG(env) = TAG_FORWARDING;
  FORWARDING_ADDRESS(env) = heap_free;
  ret = (env_t*)heap_free;
  heap_free += size;
  return ret;
}

static void scan_lobject(void* p, size_t* size) {
  int i;
  switch (OBJ_TAG(p)) {
  case TAG_CONT:
    *size = sizeof(cont_t);
    ((cont_t*)p)->env = copy_env(((cont_t*)p)->env);
    break;
  case TAG_CONS:
    *size = sizeof(cons_t);
    ((cons_t*)p)->car = copy_lobject(((cons_t*)p)->car);
    ((cons_t*)p)->cdr = copy_lobject(((cons_t*)p)->cdr);
    break;
  case TAG_ENV:
    *size = sizeof(env_t) + sizeof(lobject) * (((env_t*)p)->num - 1);
    ((env_t*)p)->link = copy_env(((env_t*)p)->link);
    for (i = 0; i < ((env_t*)p)->num; ++i) {
      ((env_t*)p)->vars[i] = copy_lobject(((env_t*)p)->vars[i]);
    }
    break;
  case TAG_CONT_PROC:
    *size = sizeof(cont_proc_t);
    ((cont_proc_t*)p)->c =
      (void*)copy_lobject((lobject)((cont_proc_t*)p)->c);
    break;
  case TAG_SYMBOL:
    *size = sizeof(symbol_t);
    break;
  default:
    assert(0);
  }
}

static void scan_heap_for_stack_gc() {
  unsigned char* p;
  size_t size;
  for (p = heap_scan_start; p < heap_free;) {
#ifdef GC_VERBOSE
    fprintf(stderr, "scan %p (tag:%x)\n", p, OBJ_TAG(p));
#endif
    scan_lobject((void*)p, &size);
    p += size;
  }
}

static void reset_remset() {
  heap_remset_free = heap_remset_tail = heap_remset;
}

static void do_stack_gc(thunk_t* thunk) {
  int i;
  lobject** p;
  void** vp;
  heap_scan_start = heap_free;
  for (p = heap_rootset; p < heap_rootset_free; ++p) {
    if (**p) {
      **p = copy_lobject(**p);
    }
  }
  for (vp = heap_remset; vp < heap_remset_free; ++vp) {
    size_t dummy;
    scan_lobject(*vp, &dummy);
  }
  thunk->env = copy_env(thunk->env);
  for (i = 0; i < thunk->num; ++i) {
    thunk->vars[i] = copy_lobject(thunk->vars[i]);
  }
  scan_heap_for_stack_gc();
  reset_remset();
}

void stack_gc(thunk_t* thunk) {
  assert(!in_heap_gc);
#ifdef GC_VERBOSE
  fprintf(stderr, "*** stack gc begin\n");
#endif
  if (!setjmp(gc_entry_point)) {
    do_stack_gc(thunk);
  } else {
    heap_gc(thunk);
  }
#ifdef GC_VERBOSE
  fprintf(stderr, "*** stack gc end\n");
#endif
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

void add_heap_remset(void* root) {
  *heap_remset_free = root;
  if (heap_remset_free == heap_remset_tail) {
    ++heap_remset_free;
    ++heap_remset_tail;
  } else {
    heap_remset_free = (void**)FREELIST_NEXT(*heap_remset_free);
  }
  if (heap_remset_tail == heap_remset_end) {
    heap_remset = (void**)realloc(heap_remset, heap_remset_size * 2);
    if (!heap_remset) {
      fprintf(stderr, "heap remset is full\n");
      exit(1);
    }
    heap_remset_free = heap_remset + heap_remset_size / sizeof(void*);
    heap_remset_size *= 2;
    heap_remset_end = heap_remset + heap_remset_size / sizeof(void*);
    heap_remset_tail = heap_remset_free;
  }
}

void write_barrier(void* root, lobject oldval, lobject nextval) {
  int dummy, old_in_stack = 0, next_in_stack = 0;
  void **p, **position = NULL;

  if (GET_PTAG(oldval) != PTAG_FIXNUM && (int*)oldval > &dummy) {
    old_in_stack = 1;
  }
  if (GET_PTAG(nextval) != PTAG_FIXNUM && (int*)nextval > &dummy) {
    next_in_stack = 1;
  }
  if (old_in_stack || next_in_stack) {
    for (p = heap_remset; p < heap_remset_tail; ++p) {
      if (FREELIST_ELM(*p)) {
        continue;
      } else if (*p == root) {
        position = p;
        break;
      }
    }
  }
  if (position && !next_in_stack) {  /* remove from remember set */
    MAKE_FREELIST_ELM(position, heap_remset_free);
    heap_remset_free = position;
  }
  if (!position && next_in_stack) {  /* add to remember set */
    add_heap_remset(root);
  }
}

static void scan_heap_for_heap_gc() {
  unsigned char* p;
  size_t size;
  for (p = heap_to; p < heap_free;) {
#ifdef GC_VERBOSE
    fprintf(stderr, "scan %p (tag:%x)\n", p, OBJ_TAG(p));
#endif
    scan_lobject((void*)p, &size);
    p += size;
  }
}

void heap_gc(thunk_t* thunk) {
  int i;
  lobject** p;
  unsigned char* tmp;
  assert(!in_heap_gc);
  in_heap_gc = 1;
#ifdef GC_VERBOSE
  fprintf(stderr, "*** heap gc begin\n");
#endif
  heap_free = heap_to;
  for (p = heap_rootset; p < heap_rootset_free; ++p) {
    if (**p) {
      **p = copy_lobject(**p);
    }
  }
  if (thunk) {
    thunk->env = copy_env(thunk->env);
    for (i = 0; i < thunk->num; ++i) {
      thunk->vars[i] = copy_lobject(thunk->vars[i]);
    }
  }
  scan_heap_for_heap_gc();
  reset_remset();
  tmp = heap_from;
  heap_from = heap_to;
  heap_to = tmp;
#ifdef GC_VERBOSE
  fprintf(stderr, "*** heap gc end\n");
#endif
  in_heap_gc = 0;
}
