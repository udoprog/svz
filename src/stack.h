#ifndef _STACK_H_
#define _STACK_H_

#define STRING_MAX 1024
#define STRING_FACTOR 2

#define ARRAY_MAX 16
#define ARRAY_FACTOR 2

#define ARRAY_ELEMENTS 16
#define ARRAY_ELEMENTS_FACTOR 2

#define PROC_MAX 64
#define PROC_FACTOR 2

typedef struct array {
  int *elements;
  int e_pos;
  int e_alloc;
} array;

typedef struct callspace {
  char *strings;
  int s_pos;
  int s_alloc;
  
  struct procedure* procedures;
  int p_pos;
  int p_alloc;
  
  array *arrays;
  int a_pos;
  int a_alloc;
  
  /* nice place to store application specific global variables. */
  void *global;
} callspace;

typedef struct procedure {
  int (*func)(callspace*, int[]);
  int argv[16];
  int argc;
} procedure;

callspace*
create_callspace(void);

void
destroy_callspace(callspace *);

int
string_append(callspace *cs, const char *str);

char*
string_get(callspace *cs, int p);

int
array_create(callspace *cs);

int
array_append(callspace *cs, int ar_id, int str_id);

int*
array_get(callspace *cs, int ar_id);

int
array_length(callspace *cs, int ar_id);

void
array_destroy(callspace *cs, int ar_id);

int
proc_append(callspace *cs, int (*func)(callspace *, int[]), int n_args, ...);

int
proc_run(callspace *cs, int proc_n);

#endif /* _STACK_H_ */
