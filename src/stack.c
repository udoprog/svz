#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "stack.h"

#ifdef _DEBUG
  #define dprintf(...) fprintf(stderr, __VA_ARGS__);
#else
  #define dprintf(...)
#endif

callspace*
create_callspace()
{
  callspace *build = malloc(sizeof(callspace));
  
  build->s_pos = 0;
  build->s_alloc = STRING_MAX;
  build->strings = malloc(sizeof(char) * build->s_alloc);
  
  build->p_pos = 0;
  build->p_alloc = PROC_MAX;
  build->procedures = malloc(sizeof(procedure) * build->p_alloc);
  
  build->a_pos = 0;
  build->a_alloc = ARRAY_MAX;
  build->arrays = malloc(sizeof(array) * build->a_alloc);
  
  return build;
}

void
destroy_callspace(callspace *cs)
{
  free(cs->strings);
  free(cs->procedures);

  int i = 0;
  for (i = 0; i < cs->a_pos; i++)
  {
    array_destroy(cs, i);
  }
  
  free(cs->arrays);

  // just in case someone wants to use the same memory space again.
  
  // clear procedures
  cs->p_pos = 0;
  cs->p_alloc = 0;

  // clear strings
  cs->s_pos = 0;
  cs->s_alloc = 0;
  
  // clear arrays
  cs->a_pos = 0;
  cs->a_alloc = 0;
  
  free(cs);
  return;
}

int
array_create(callspace *cs)
{
  if (cs->a_pos >= cs->a_alloc)
  {
    assert(cs->a_alloc * ARRAY_FACTOR > cs->a_alloc);
    cs->a_alloc *= ARRAY_FACTOR;
    dprintf("reallocating 'arrays' to %d bytes\n", cs->a_alloc);
    cs->arrays = realloc(cs->arrays, sizeof(char) * cs->a_alloc);
    assert(cs->arrays != NULL);
  }
  
  array *ary = &cs->arrays[cs->a_pos];
  
  ary->e_pos = 0;
  ary->e_alloc = ARRAY_ELEMENTS;
  ary->elements = malloc(sizeof(int) * ary->e_alloc);
  
  assert(ary->elements != NULL);
  assert(ary->e_pos < ary->e_alloc);
  
  dprintf("ary created\n");
  dprintf("ary->e_pos   = %d\n", ary->e_pos);
  dprintf("ary->e_alloc = %d\n", ary->e_alloc);
  dprintf("ary->elements = (%d * %d) = %d bytes\n", sizeof(int), ary->e_alloc, sizeof(int) * ary->e_alloc);
  
  return cs->a_pos++;
}

int
array_append(callspace *cs, int ar_id, int str_id)
{
  array *ary = &cs->arrays[ar_id];
  
  dprintf("ary->e_pos   = %d\n", ary->e_pos);
  dprintf("ary->e_alloc = %d\n", ary->e_alloc);
  
  if (ary->e_pos >= ary->e_alloc)
  {
    assert(ary->e_alloc * ARRAY_ELEMENTS_FACTOR > ary->e_alloc);
    ary->e_alloc *= ARRAY_ELEMENTS_FACTOR;
    ary->elements = realloc(ary->elements, sizeof(int) * ary->e_alloc);
    assert(ary->elements != NULL);
  }
  
  ary->elements[ary->e_pos] = str_id;
  return ary->e_pos++;
}

int*
array_get(callspace *cs, int ar_id)
{
  assert(ar_id < cs->a_pos && ar_id >= 0);
  return cs->arrays[ar_id].elements;
}

int
array_length(callspace *cs, int ar_id)
{
  assert(ar_id < cs->a_pos && ar_id >= 0);
  return cs->arrays[ar_id].e_pos;
}

void
array_destroy(callspace *cs, int ar_id)
{
  assert(ar_id < cs->a_pos && ar_id >= 0);
  array *ary = &cs->arrays[ar_id];
  free(ary->elements);
  ary->e_pos = 0;
  ary->e_alloc = 0;
}

int
string_append(callspace *cs, const char *str)
{
  int sl = strlen(str) + 1;
  int sp = cs->s_pos;
  
  while ((cs->s_pos + sl) >= cs->s_alloc)
  {
    assert(cs->s_alloc * STRING_FACTOR > cs->s_alloc);
    cs->s_alloc *= STRING_FACTOR;
    dprintf("reallocating strings to %d bytes\n", cs->s_alloc);
    cs->strings = realloc(cs->strings, sizeof(char) * cs->s_alloc);
    assert(cs->strings != NULL);
  }
  
  dprintf("writing to %d, %d bytes\n", cs->s_pos, sl);
  
  memcpy (&(cs->strings[cs->s_pos]), str, sl);
  cs->s_pos += sl;
  
  //printf("appending %d bytes, stopping at %d, returning %d\n", sl, cs->s_pos, sp);
  
  return sp;
}

char*
string_get(callspace *cs, int p)
{
  assert(p < cs->s_pos && p >= 0);
  return &cs->strings[p];
}

int
proc_append(callspace *cs, int (*func)(callspace*, int[]), int n_args, ...)
{
  if (cs->p_pos >= cs->p_alloc)
  {
    assert(cs->p_alloc * PROC_FACTOR > cs->p_alloc);
    cs->p_alloc *= PROC_FACTOR;
    dprintf("reallocating 'procedures' to %d bytes\n", cs->p_alloc);
    cs->procedures = realloc(cs->procedures, sizeof(char) * cs->p_alloc);
    assert(cs->procedures != NULL);
  }
  
  int pp = cs->p_pos;
  
  cs->procedures[pp].func = func;
  cs->procedures[pp].argc = n_args;
  
  int i;
  
  va_list ap;
  
  va_start(ap, n_args);
  for (i = 0; i < n_args; i++)
  {
    cs->procedures[pp].argv[i] = va_arg(ap, int);
  }
  va_end(ap);
  
  cs->p_pos += 1;
  
  return pp;
}

int
proc_run(callspace *cs, int proc_n)
{
  assert(proc_n < cs->p_pos && proc_n >= 0);
  return cs->procedures[proc_n].func(cs, cs->procedures[proc_n].argv);
}
