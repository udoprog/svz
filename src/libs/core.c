#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "libs/core.h"
#include "frun.h"
#include "supervize.h"
#include "proc.h"

#define PROC_PID "/proc/%d/stat"

#define dprintf(...) fprintf(stderr, "STACK: " __VA_ARGS__)

#ifndef _DEBUG
#undef dprintf
#define dprintf(...)
#endif

static const frun_option core_functions[] = 
{
  {.func = sv_spawn,        .name = "spawn",  .argc = -1},
  {.func = sv_exec,         .name = "exec",   .argc = -1},
  {.func = sv_echo,         .name = "echo",   .argc = -1},
  {.func = sv_pid,          .name = "pid",    .argc = 1},
  {.func = sv_pidto,        .name = "pidto",  .argc = 1},
  {.func = sv_debug_print,  .name = "debug",  .argc = 0},
  {.func = sv_cpu,          .name = "cpu",    .argc = 3},
  {NULL, NULL, 0x0}
};

svz_module core_module = 
{
  .name = "core",
  .functions = core_functions
};

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
  
  if (childpid > 0)
  {
    g_last_pid = childpid;
    sprintf(g_last_pid_str, "%d", g_last_pid);
  }
  
  return status == 0 ? TRUE : FALSE;
}

int
fork_spawn(int argc, char *argv[])
{
  int childpid = fork();
  
  switch (childpid)
  {
    case 0:
      /*
       * able to execute other program.
       */
      if (setsid() >= 0)
      {
        execvp(argv[0], argv);
      }
      
      fprintf(stderr, "Unable to fork child: %s - %s\n", argv[0], strerror(errno));
      // get here when unable to exec.
      exit(1);
      break;
  }
  
  if (childpid > 0)
  {
    g_last_pid = childpid;
    sprintf(g_last_pid_str, "%d", g_last_pid);
  }
  
  return TRUE;
}

int
sv_spawn(callspace *cs, int argv[])
{
  dprintf("reached spawn...\n");
  
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
  
  return fork_spawn(array_c, fork_argv);
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

  
pid_t
pid_from_string(const char *f)
{
  FILE* fp = NULL;
  
  pid_t pid = 0;
  
  if ((fp = fopen(f, "r")) != NULL)
  {
    fscanf(fp, "%d", &pid);
    fclose(fp);
  }
  
  if (pid == 0)
  {
    sscanf(f, "%d", &pid);
  }
  
  return pid;
}

int
sv_pid(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);
  
  char* f = string_get(cs, array[0]);
  
  pid_t pid = pid_from_string(string_get(cs, array[0]));
  
  if (check_pid(pid) == TRUE)
  {
    g_last_pid = pid;
    sprintf(g_last_pid_str, "%d", g_last_pid);
    return TRUE;
  }
  
  return FALSE;
}

int
sv_cpu(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);

  char *f = string_get(cs, array[0]);
  char *gte = string_get(cs, array[1]);
  char *lte = string_get(cs, array[2]);

  unsigned int i_gte = 0, i_lte = 0;
  
  if (sscanf(gte, "%u", &i_gte) != 1 || i_gte <= 0 || i_gte > 100)
  {
    fprintf(stderr, "cpu: bad argument #2; expected integer in range 1-100, not %u\n", i_gte);
    return FALSE;
  }
  
  if (sscanf(lte, "%u", &i_lte) != 1 || i_lte <= 0 || i_lte > 100)
  {
    fprintf(stderr, "cpu: bad argument #3; expected integer in range 1-100, not %u\n", i_lte);
    return FALSE;
  }

  i_lte *= 100;
  i_gte *= 100;
  
  pid_t pid = pid_from_string(string_get(cs, array[0]));
  
  unsigned int cpu = proc_pid_cpu(pid);

  printf("actual cpu: %u\n", cpu);
  
  return cpu >= i_gte && cpu <= i_lte ? TRUE : FALSE;
}

int
sv_pidto(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);
  
  FILE* fp = NULL;

  pid_t pid = 0;
  char* f = string_get(cs, array[0]);
  
  if ((fp = fopen(f, "w")) != NULL)
  {
    fprintf(fp, "%d\n", g_last_pid);
    fclose(fp);
    return TRUE;
  }
  else
  {
    fprintf(stderr, "%s: Unable to open %s - %s\n", g_program_name, f, strerror(errno));
  }
  
  return FALSE;
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
sv_debug_print(callspace *cs, int argv[])
{
  globals *global = (globals *)g_cs->global;
  
  frun_option *func = global->functions;
  
  while (func->func != NULL)
  {
    if (func->argc == -1)
    {
      printf("%s variadic arguments\n", func->name);
    }
    else
    {
      printf("%s %d argument(s)\n", func->name, func->argc);
    }

    dprintf(" - mempos: %p\n", func->func);
    
    func++;
  }
  
  return FALSE;
}
