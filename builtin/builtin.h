#ifndef __KASHIWA_BUILTIN__
#define __KASHIWA_BUILTIN__

#include "../core/object.h"

#define BOOLTEST(x) \
  (GET_PTAG(x) != PTAG_OTHER || OBJ_TAG(x) != TAG_BOOLEAN || \
   ((boolean_t*)REM_PTAG(x))->bool)

extern lobject sharpt;
extern lobject sharpf;
extern lobject nil;

/* 6.2.5. Numerical operations */
extern lobject builtin_clos_eql;
extern lobject builtin_clos_plus;
extern lobject builtin_clos__;
extern lobject builtin_clos_star;
extern lobject builtin_clos_sla;
/* 6.3.2. Pairs and lists */
extern lobject builtin_clos_cons;
extern lobject builtin_clos_car;
extern lobject builtin_clos_cdr;
extern lobject builtin_clos_list;
extern lobject builtin_clos_liststar;
/* 6.4. Control features */
extern lobject builtin_clos_call_with_current_continuation;
/* 6.6.3. Output */
extern lobject builtin_clos_write;
extern lobject builtin_clos_newline;

void init_builtin();

/* 6.2.5. Numerical operations */
void builtin_eql(env_t* env, unsigned int num_args, ...);
void builtin_plus(env_t* env, unsigned int num_args, ...);
void builtin__(env_t* env, unsigned int num_args, ...);
void builtin_star(env_t* env, unsigned int num_args, ...);
void builtin_sla(env_t* env, unsigned int num_args, ...);
/* 6.3.2. Pairs and lists */
void builtin_cons(env_t* env, cont_t* cont, lobject x, lobject y);
void builtin_car(env_t* env, cont_t* cont, lobject x);
void builtin_cdr(env_t* env, cont_t* cont, lobject x);
void builtin_list(env_t* env, unsigned int num_args, ...);
void builtin_liststar(env_t* env, unsigned int num_args, ...);
/* 6.4. Control features */
void builtin_call_with_current_continuation (env_t* env, cont_t* cont,
                                             lobject proc);
/* 6.6.3. Output */
void builtin_write(env_t* env, unsigned int num_args, ...);
void builtin_newline(env_t* env, unsigned int num_args, ...);

#endif
