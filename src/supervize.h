#ifndef _SUPERVIZE_H_
#define _SUPERVIZE_H_

#include <stdio.h>
#include "stack.h"
#include "frun.h"

#define FALSE 0
#define TRUE (!FALSE)

#define CLI 1
#define STDIN 2

void output_append(int (*)(int[]));
int sv_stmt_and(callspace*, int argv[]);
int sv_stmt_or(callspace*, int argv[]);
int sv_stmt_not(callspace*, int argv[]);
void sv_root(callspace*, int status, int do_proc, int else_proc);

typedef struct {
  int last_pid;
  char last_pid_str[255];
  frun_option *functions;
  int f_pos;
  char *program_name;
} globals;

extern callspace* g_cs;

extern int g_last_pid;
extern char *g_program_name;
extern char g_last_pid_str[];
extern int g_mode;
extern FILE *g_fp;

extern frun_option g_functions[];

typedef struct {
  const char *name;
  const frun_option *functions;
} svz_module;

#endif /* _SUPERVIZE_H_ */
