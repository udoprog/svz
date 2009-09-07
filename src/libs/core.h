#ifndef _CORE_H_
#define _CORE_H_

#include "stack.h"
#include "frun.h"
#include "supervize.h"

int sv_cpu(callspace*, int argv[]);
int sv_exec(callspace*, int argv[]);
int sv_spawn(callspace*, int argv[]);
int sv_echo(callspace*, int argv[]);
int sv_pid(callspace*, int argv[]);
int sv_pidto(callspace*, int argv[]);
int sv_pidto(callspace*, int argv[]);
int sv_debug_print(callspace*, int argv[]);

extern svz_module core_module;

#endif /* _CORE_H_ */
