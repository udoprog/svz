#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "parser.tab.h"
#include "stack.h"
#include "supervize.h"

#define dprintf(...)

#define YYACCEPT 0
#define YYABORT 1
#define YYERROR -1
#define COMMAND_MAX 1024

char *program_name;
char *argument;
char **g_argv;
int g_index;
char *g_shell;
int g_status;

char *g_command_string;
pid_t g_last_pid;
char g_last_pid_str[256];
char *g_supervize_file;

#define PROC_PID "/proc/%d/stat"

void
print_help(void)
{
  puts("");
  puts("svz <if-stmt> [-do <do-stmt> [-else <else-stmt>]]");
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
check_pid(int pid)
{
  struct stat pid_st;
  
  char procfile[1024];
  sprintf(procfile, PROC_PID, pid);
  
  if (stat(procfile, &pid_st) == 0)
  {
    return TRUE;
  }
  
  return FALSE;
}

int
fork_exec(int argc, char *argv[])
{
  int childpid = fork();

  switch (childpid)
  {
    case 0:
      /*
       * able to execute other program.
       */
      execvp(argv[0], argv);
      fprintf(stderr, "Unable to fork child: %s - %s\n", argv[0], strerror(errno));
      // get here when unable to exec.
      exit(1);
      break;
  }
  
  int status;
  waitpid(childpid, &status, 0);
  return status == 0 ? TRUE : FALSE;
}

#define coBEGIN     static int state = 0; switch (state) { case 0:
#define coRETURN(v) { state = __LINE__; return v; case __LINE__:; } while(0);
#define coEND       }

int
yylex(YYSTYPE *lvalp)
{
  char *argument;
  coBEGIN;

  while (1)
  {
    argument = g_argv[g_index++];
    
    if (argument == NULL)
    {
      coRETURN(YYACCEPT);
      break;
    }
    
    if (strcmp(argument, "-exec") == 0)
    {
      /* execute statement mode, everything is swallowed until $ is reached */
      coRETURN(EXEC);

      while (1)
      {
        argument = g_argv[g_index++];
        
        if (argument == NULL)
        {
          coRETURN(YYERROR);
          break;
        }
        
        if (strcmp("$", argument) == 0)
        {
          break;
        }
        
        (*lvalp).string = string_append(g_cs, argument);
        coRETURN(ARGUMENT);
      }
      
      coRETURN(ARGEND);
      continue;
    }
    
    if (strcmp(argument, "-pid") == 0)
    {
      coRETURN(PID);
      continue;
    }
    
    if (strcmp(argument, "-echo") == 0)
    {
      coRETURN(ECHO);

      while (1)
      {
        argument = g_argv[g_index++];
        
        if (argument == NULL)
        {
          coRETURN(YYERROR);
          break;
        }
        
        if (strcmp("$", argument) == 0)
        {
          break;
        }
        
        (*lvalp).string = string_append(g_cs, argument);
        coRETURN(ARGUMENT);
      }
      
      coRETURN(ARGEND);
      continue;
    }
    
    if (strcmp(argument, "-or") == 0)
    {
      coRETURN(OR);
      continue;
    }

    if (strcmp(argument, "!") == 0 || strcmp(argument, "-not") == 0)
    {
      coRETURN(NOT);
      continue;
    }
    
    if (strcmp(argument, "-and") == 0)
    {
      coRETURN(AND);
      continue;
    }
    
    if (strcmp(argument, "{") == 0)
    {
      coRETURN('{');
      continue;
    }

    if (strcmp(argument, "}") == 0)
    {
      coRETURN('}');
      continue;
    }
    
    if (strcmp(argument, "-do") == 0)
    {
      coRETURN(DO);
      continue;
    }
    
    if (strcmp(argument, "-else") == 0)
    {
      coRETURN(ELSE);
      continue;
    }
    
    (*lvalp).string = string_append(g_cs, argument);
    coRETURN(ARGUMENT);
  }
  
  coEND;
  fprintf(stderr, "Lexer called after YYACCEPT\n");
  return YYERROR;
}

void
yyerror(const char *message)
{
  printf("%s: Parser Error - %s\n", program_name, message);
  print_help();
  return;
}

int
sv_exec(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);
  
  char **fork_argv = malloc(sizeof(char*) * (array_c + 1));
  
  int i;
  char *arg;
  
  for (i = 0; i < array_c; i++)
  {
    arg = string_get(g_cs, array[i]);

    if (strcmp(arg, "%%") == 0)
    {
      fork_argv[i] = g_last_pid_str;
    }
    else
    {
      fork_argv[i] = arg;
    }
  }
  
  fork_argv[i+1] = NULL;
  
  return fork_exec(array_c, fork_argv);
}

int
sv_echo(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);

  int i;
  
  for (i = 0; i < array_c; i++)
  {
    puts(string_get(g_cs, array[i]));
  }

  return TRUE;
}

int
sv_pid(callspace *cs, int argv[])
{
  FILE* fp = NULL;

  pid_t pid = 0;
  char* f = string_get(cs, argv[0]);
  
  if ((fp = fopen(f, "r")) != NULL)
  {
    fscanf(fp, "%d", &pid);
    fclose(fp);
  }
  
  if (pid == 0)
  {
    sscanf(f, "%d", &pid);
  }

  if (check_pid(pid) == TRUE)
  {
    g_last_pid = pid;
    sprintf(g_last_pid_str, "%d", g_last_pid);
    return TRUE;
  }

  return FALSE;
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

callspace *g_cs;

/**
 * supervize a program by checking specific arguments and executing actions if they are not true.
 */
int main(int argc, char *argv[])
{
  int i;
  program_name = argv[0];

  g_cs = create_callspace();
  g_index = 1;
  g_argv = argv;

  g_command_string = malloc(COMMAND_MAX);
  g_last_pid = 0;
  strcpy(g_last_pid_str, "0");
  
  g_shell = (void *)getenv("SHELL");

  if (argc < 2)
  {
    fprintf(stderr, "%s: Bad number of arguments\n", program_name);
    return 1;
  }

  if (strcmp(argv[1], "-help") == 0)
  {
    print_help();
    return 0;
  }
  
  g_supervize_file = argv[1];
  
  if (g_shell == NULL)
  {
    fprintf(stderr, "%s: SHELL environment variable not set\n", program_name);
    return 1;
  }
  
  yyparse();
  return g_status;
}
