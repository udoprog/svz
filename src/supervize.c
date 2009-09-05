#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#include "stack.h"
#include "frun.h"
#include "supervize.h"
#include "lexer.h"

#include "libs/core.h"

#define dprintf(...)

#define COMMAND_MAX 1024

globals g_global = {
  .f_pos = 0
};

callspace *g_cs;

char *g_program_name;
char *argument;

int g_status;
int g_mode;

char *g_command_string;
pid_t g_last_pid;
char g_last_pid_str[256];
char *g_supervize_file;
FILE *g_fp;

void
print_help(void)
{
  puts("");
  puts("svz <if-stmt> [-do <do-stmt>] [-else <else-stmt>]");
  puts("");
  puts("This is the grammar:");
  puts("");
  puts("  if-stmt   := stmt");
  puts("  do-stmt   := stmt");
  puts("  else-stmt := stmt");
  puts("");
  puts("  stmt := { stmt }");
  puts("  stmt |= <-not|!> stmt");
  puts("  stmt |= stmt -and stmt");
  puts("  stmt |= stmt -or stmt");
  puts("  stmt |= -pid <argument>");
  puts("    TRUE if <argument> is an existing pid, otherwise FALSE");
  puts("");
  puts("  stmt |= -echo argv $");
  puts("    Echoes all arguments, is always TRUE.");
  puts("");
  puts("  stmt |= -exec argv $");
  puts("    Executes <path> with argument list. Is TRUE if execution returns status 0, otherwise FALSE.");
  puts("");
  puts("  argv |= ARGUMENT [ARGUMENT [..]]");
  puts("");
}

int
sv_stmt_and(callspace* cs, int argv[])
{
  int p1_id = argv[0];
  int p2_id = argv[1];
  
  int p1, p2;
  
  p1 = proc_run(cs, p1_id);

  if (p1 == FALSE)
  {
    dprintf("AND: call(%d) -> 0 && call(%d) -> ?\n", p1_id, p2_id);
    return FALSE;
  }

  p2 = proc_run(cs, p2_id);
  
  if (p2 == FALSE)
  {
    dprintf("AND: call(%d) -> 1 && call(%d) -> 0\n", p1_id, p2_id);
    return FALSE;
  }
  
  dprintf("AND: call(%d) -> 1 && call(%d) -> 1\n", p1_id, p2_id);
  
  return TRUE;
}

int
sv_stmt_not(callspace* cs, int argv[])
{
  return (proc_run(cs, argv[0]) == 0) ? 1 : 0;
}

int
sv_stmt_or(callspace* cs, int argv[])
{
  int p1_id = argv[0];
  int p2_id = argv[1];
  
  int p1, p2;
  
  p1 = proc_run(cs, p1_id);

  if (p1 == TRUE)
  {
    dprintf(" OR: call(%d) -> 1 || call(%d) -> ?\n", p1_id, p2_id);
    return TRUE;
  }

  p2 = proc_run(cs, p2_id);

  if (p2 == TRUE)
  {
    dprintf(" OR: call(%d) -> 0 || call(%d) -> 1\n", p1_id, p2_id);
    return TRUE;
  }

  dprintf(" OR: call(%d) -> 0 || call(%d) -> 0\n", p1_id, p2_id);
  
  return FALSE;
}

void
sv_root(callspace* cs, int status, int do_proc, int else_proc)
{
  g_status = 255;

  switch (status)
  {
    case TRUE:
      if (do_proc != -1)
      {
        proc_run(g_cs, do_proc);
      }
      
      g_status = 0;
      break;
    default:
      if (else_proc != -1)
      {
        proc_run(g_cs, else_proc);
      }
      
      g_status = 255;
      break;
  }
}

/**
 * load_functions - Load a bunch of functions into a specific global.
 * @g_cs: The callspace into which the functions are loaded.
 * @func: The actual functions (where func->func is NULL when the list is terminated).
 *
 * load_functions will return non-zero if a function is attempted to be loaded twice.
 */
void
load_module(callspace *g_cs, svz_module *mod)
{
  int c = 0, i;
  globals *global = (globals *)g_cs->global;
  frun_option *to_func;
  const frun_option *from_func = mod->functions;
  
  while ((from_func + c)->func != NULL)
  {
    dprintf("new: %d\n", (from_func + c)->argc);
    c++;
  }
  
  global->f_pos += c;
  global->functions = realloc(global->functions, sizeof(frun_option *));
  to_func = global->functions + global->f_pos - c;
  
  // iterate the list backwards and load the functions.
  do
  {
    *(to_func + c) = *(from_func + c);
    dprintf("new to: %s; %d\n", (to_func + c)->name, (to_func + c)->argc);
  }
  while (c-- > 0);
}

void
read_opts(int argc, char *argv[])
{
  char c;
  
  opterr = 0;
  
  while ((c = getopt (argc, argv, "i:")) != -1)
  {
    switch (c)
    {
    case 'i':
      g_fp = fopen(optarg, "r");
      
      if (g_fp == NULL)
      {
        return;
      }
      
      dprintf("opening file: %s\n", optarg);
      g_mode = STDIN;
      break;
    case '?':
      return;
    default:
      return;
    }
  }
}

/**
 * supervize a program by checking specific arguments and executing actions if they are not true.
 */
int main(int argc, char *argv[])
{
  g_mode = CLI;
  
  int i;
  g_program_name = argv[0];
  
  g_fp = stdin;
  
  g_cs = create_callspace();
  g_cs->global = &g_global;
  
  load_module(g_cs, &core_module);
  
  g_argv = argv;

  g_command_string = malloc(COMMAND_MAX);
  g_last_pid = 0;
  strcpy(g_last_pid_str, "0");
  
  read_opts(argc, argv);
  
  g_index = optind;
  /* set mode to stdin when we don't have any arguments */
  if (argc - g_index < 1)
  {
    g_mode = STDIN;
  }
  
  yyparse();
  return g_status;
}
