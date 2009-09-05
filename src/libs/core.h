#ifndef _CORE_H_
#define _CORE_H_

#include "stack.h"
#include "frun.h"
#include "supervize.h"

int sv_exec(callspace*, int argv[]);
int sv_spawn(callspace*, int argv[]);
int sv_echo(callspace*, int argv[]);
int sv_pid(callspace*, int argv[]);
int sv_pidto(callspace*, int argv[]);

void output_append(int (*)(int[]));
int sv_stmt_and(callspace*, int argv[]);
int sv_stmt_or(callspace*, int argv[]);
int sv_stmt_not(callspace*, int argv[]);
void sv_root(callspace*, int status, int do_proc, int else_proc);

extern svz_module core_module;

#endif /* _CORE_H_ */
