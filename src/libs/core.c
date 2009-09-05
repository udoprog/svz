#include "libs/core.h"
#include "frun.h"
#include "supervize.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define PROC_PID "/proc/%d/stat"

const frun_option core_functions[] = 
{
  {.func = sv_exec,   .name = "exec",   .argc = -1},
  {.func = sv_spawn,  .name = "spawn",  .argc = -1},
  {.func = sv_echo,   .name = "echo",   .argc = -1},
  {.func = sv_pid,    .name = "pid",    .argc = 1},
  {.func = sv_pidto,  .name = "pidto",  .argc = 1},
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
      execvp(argv[0], argv);
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

int
sv_pid(callspace *cs, int argv[])
{
  int *array = array_get(g_cs, argv[0]);
  int array_c = array_length(g_cs, argv[0]);
  
  FILE* fp = NULL;

  pid_t pid = 0;
  char* f = string_get(cs, array[0]);
  
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
