#ifndef _SUPERVIZE_H_
#define _SUPERVIZE_H_

#include "stack.h"
#include "frun.h"

#define FALSE 0
#define TRUE (!FALSE)

void output_append(int (*)(int[]));
int sv_stmt_and(callspace*, int argv[]);
int sv_stmt_or(callspace*, int argv[]);
int sv_stmt_not(callspace*, int argv[]);
void sv_root(callspace*, int status, int do_proc, int else_proc);

extern callspace* g_cs;

extern frun_option g_functions[];

#endif /* _SUPERVIZE_H_ */
