#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"

#define INITIAL_SYMBOL_TABLE_ENTRY 1024
#define INITIAL_SYMBOL_STRING_SIZE 8192
#define INITIAL_SYMBOL_ROOTSET_ENTRY 256

static char* symbol_string;
static symbol_t* symbol_table;
static void** symbol_rootset;

static size_t symbol_string_size;
static size_t symbol_table_size;
static size_t symbol_rootset_size;

static char* symbol_string_end;
static symbol_t* symbol_table_end;
static void** symbol_rootset_end;

static char* symbol_string_to;
static char* symbol_string_from;
static symbol_t* symbol_table_to;
static symbol_t* symbol_table_from;

static char* symbol_string_free;
static symbol_t* symbol_table_free;
static void** symbol_rootset_free;

void init_symbol() {
  symbol_string_size = INITIAL_SYMBOL_STRING_SIZE;
  symbol_table_size = sizeof(symbol_t) * INITIAL_SYMBOL_TABLE_ENTRY;
  symbol_rootset_size = sizeof(void*) * INITIAL_SYMBOL_ROOTSET_ENTRY;

  symbol_string = symbol_string_free = (char*)malloc(symbol_string_size);
  symbol_table = symbol_table_free = (symbol_t*)malloc(symbol_table_size);
  symbol_rootset = symbol_rootset_free = (void**)malloc(symbol_rootset_size);

  symbol_string_end = symbol_string + INITIAL_SYMBOL_STRING_SIZE;
  symbol_table_end = symbol_table + INITIAL_SYMBOL_TABLE_ENTRY;
  symbol_rootset_end = symbol_rootset + INITIAL_SYMBOL_ROOTSET_ENTRY;

  symbol_string_from = symbol_string;
  symbol_string_to = symbol_string + symbol_string_size / 2;
  symbol_table_from = symbol_table;
  symbol_table_to = symbol_table + symbol_table_size / sizeof(symbol_t*) / 2;
}

static char* add_symbol_name(char* name) {
  char* next = symbol_string_free + strlen(name) + 1;
  char* ret = symbol_string_free;
  if ((symbol_string_from < symbol_string_to && next > symbol_string_to) ||
      (symbol_string_to < symbol_string_from && next > symbol_string_end)) {
    fprintf(stderr, "symbol string is full\n");
    exit(1);  /* TODO: GC */
  }
  strcpy(symbol_string_free, name);
  symbol_string_free = next;
  return ret;
}

lobject intern(char* name) {
  symbol_t* p;
  for (p = symbol_table_from; p < symbol_table_free; ++p) {
    if (!strcmp(p->name, name)) {
      return ADD_PTAG(p, PTAG_OTHER);
    }
  }
  if (symbol_table_free == symbol_table_end ||
      symbol_table_free == symbol_table_to) {
    fprintf(stderr, "symbol table is full\n");
    exit(1);  /* TODO: GC */
  }
  p->tag = TAG_SYMBOL;
  p->name = add_symbol_name(name);
  ++symbol_table_free;
  return ADD_PTAG(p, PTAG_OTHER);
}

void add_symbol_rootset(void* root) {
  if (symbol_rootset_free == symbol_rootset_end) {
    symbol_rootset = (void**)realloc(symbol_rootset, symbol_rootset_size * 2);
    if (!symbol_rootset) {
      fprintf(stderr, "symbol rootset is full\n");
      exit(1);
    }
    symbol_rootset_free = symbol_rootset + symbol_rootset_size / sizeof(void*);
    symbol_rootset_size *= 2;
    symbol_rootset_end = symbol_rootset + symbol_rootset_size / sizeof(void*);
  }
  *symbol_rootset_free = root;
  ++symbol_rootset_free;
}
