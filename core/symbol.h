#ifndef __KASHIWA_SYMBOL__
#define __KASHIWA_SYMBOL__

#include "object.h"

void init_symbol();
lobject intern(char* name);
void add_symbol_rootset(void* root);

#endif
