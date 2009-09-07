#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <asm/param.h>

#define dprintf(...)

// jiffy = 1/HZ seconds (HZ from param.h)
// which is the time we get from reading proc.

#define PROC            "/proc"
#define PROC_STAT       "/proc/stat"
#define PROC_PID_STAT   "/proc/%d/stat"

#define   scand(fd, to)   assert(fscanf(fd, "%d ", &(to)) == 1)
#define i_scand(fd)       fscanf(fd, "%*d ")
#define   scans(fd, to)   assert(fscanf(fd, "%s ", &(to)) == 1)
#define i_scans(fd)       fscanf(fd, "%*s ")
#define   scanc(fd, to)   assert(fscanf(fd, "%c ", &(to)) == 1)
#define i_scanc(fd)       fscanf(fd, "%*c ")
#define   scanu(fd, to)   assert(fscanf(fd, "%u", &(to)) == 1)
#define i_scanu(fd)       fscanf(fd, "%*u")
#define   scanlu(fd, to)  assert(fscanf(fd, "%lu", &(to)) == 1)
#define i_scanlu(fd)      fscanf(fd, "%*u")
#define   scanllu(fd, to) assert(fscanf(fd, "%llu", &(to)) == 1)
#define i_scanllu(fd)     fscanf(fd, "%*u")
#define   scanld(fd, to)  assert(fscanf(fd, "%ld", &(to)) == 1)
#define i_scanld(fd)      fscanf(fd, "%*d")

typedef struct {
  unsigned long long user;
  unsigned long long nice;
  unsigned long long system;
  unsigned long long idle;
  unsigned long long iowait;
  unsigned long long irq;
  unsigned long long softirq;
  unsigned long long steal;
  unsigned long long guest;
} proc_cpu_stat;

typedef struct {
  char *stat_path;
  FILE *stat_fd;
  
  proc_cpu_stat *cpu;
} proc_base;

typedef struct {
  long unsigned int u_time;
  long unsigned int s_time;
  long unsigned int cu_time;
  long unsigned int cs_time;
} proc_pid_cpu_stat;

typedef struct {
  char *pid_stat_path;
  FILE *pid_stat_fd;
  
  proc_pid_cpu_stat *cpu;
} proc_pid;

proc_base *
proc_create()
{
  proc_base *build = malloc(sizeof(proc_base));
  build->stat_path = malloc(strlen(PROC_STAT) + 1);
  strcpy(build->stat_path, PROC_STAT);
  
  build->cpu = malloc(sizeof(proc_cpu_stat));;
  return build;
}

proc_pid *
proc_pid_create(int pid)
{
  int plen = 0;
  int t_pid = pid;

  while (t_pid > 0)
  {
    plen++;
    t_pid /= 10;
  }
  
  proc_pid *build = malloc(sizeof(proc_pid));
  
  build->pid_stat_path = malloc(strlen(PROC_PID_STAT) + plen);
  sprintf(build->pid_stat_path, PROC_PID_STAT, pid);
  build->cpu = malloc(sizeof(proc_pid_cpu_stat));;
  
  return build;
}

void
proc_poll(proc_base *pb)
{
  bzero(pb->cpu, sizeof(proc_cpu_stat));
  
  pb->stat_fd = fopen(pb->stat_path, "r");
  
  if (pb->stat_fd == NULL)
  {
    perror("fopen");
    return;
  }
  
  assert(fscanf(pb->stat_fd, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu",
    &(pb->cpu->user),
    &(pb->cpu->nice),
    &(pb->cpu->system),
    &(pb->cpu->idle),
    &(pb->cpu->iowait),
    &(pb->cpu->irq),
    &(pb->cpu->softirq),
    &(pb->cpu->steal),
    &(pb->cpu->guest)
  ) == 9);
  
  fclose(pb->stat_fd);
  pb->stat_fd = NULL;
  return;
}

void
proc_pid_poll(proc_pid *pp)
{
  bzero(pp->cpu, sizeof(proc_pid_cpu_stat));
  
  pp->pid_stat_fd = fopen(pp->pid_stat_path, "r");
  
  if (pp->pid_stat_fd == NULL)
  {
    perror("fopen");
    return;
  }

  FILE *fp = pp->pid_stat_fd;

  // pid
  i_scand(fp);
  // tcomm
  i_scans(fp);
  // state
  i_scanc(fp);
  // ppid
  i_scand(fp);
  // pgid
  i_scand(fp);
  // sid
  i_scand(fp);
  // tty_nr
  i_scand(fp);
  // tty_pgrp
  i_scand(fp);
  // flags
  i_scanu(fp);
  // min_flt
  i_scanlu(fp);
  // cmin_flt
  i_scanlu(fp);
  // maj_flt
  i_scanlu(fp);
  // cmaj_flt
  i_scanlu(fp);
  
  // utime
  scanlu(fp, pp->cpu->u_time);
  // stime
  scanlu(fp, pp->cpu->s_time);
  // cutime
  scanld(fp, pp->cpu->cu_time);
  // cstime
  scanld(fp, pp->cpu->cs_time);
  
  // priority
  i_scanld(fp);
  
  // nice
  i_scanld(fp);
  
  // num_threads
  i_scand(fp);
  
  // zero
  i_scand(fp);

  // start_time
  i_scanllu(fp);

  // vsize
  i_scanlu(fp);

  // rss
  i_scanld(fp);

  // rsslim
  i_scanlu(fp);

  // start_code
  i_scanlu(fp);
  
  // end_code
  i_scanlu(fp);

  // start_stack
  i_scanlu(fp);
  
  // esp
  i_scanlu(fp);

  // ... ignore the rest until we need them ...
  
  fclose(pp->pid_stat_fd);
  pp->pid_stat_fd = NULL;
  return;
}

void
proc_free(proc_base *pb)
{
  free(pb->stat_path);
  free(pb);
}

void
proc_pid_free(proc_pid *pp)
{
  free(pp->pid_stat_path);
  free(pp);
}

long unsigned int
proc_total(proc_base *pb)
{
  return pb->cpu->user +
    pb->cpu->nice +
    pb->cpu->system +
    pb->cpu->idle +
    pb->cpu->iowait + 
    pb->cpu->irq + 
    pb->cpu->softirq + 
    pb->cpu->steal + 
    pb->cpu->guest;
}

long unsigned int
proc_pid_total(proc_pid *pp)
{
  return pp->cpu->u_time + pp->cpu->s_time;
}

unsigned int
proc_pid_cpu(pid_t pid)
{
  unsigned int total_pre, part_pre, total_post, part_post;

  proc_base *test = proc_create();
  proc_pid  *testinit = proc_pid_create(pid);
  
  proc_poll(test);
  proc_pid_poll(testinit);
  
  total_pre = proc_total(test);
  part_pre  = proc_pid_total(testinit);
  
  sleep(1);
  
  proc_poll(test);
  proc_pid_poll(testinit);
  
  total_post = proc_total(test);
  part_post  = proc_pid_total(testinit);
  
  proc_free(test);
  proc_pid_free(testinit);

  dprintf("total_pre = %u\n", total_pre);
  dprintf("part_pre = %u\n", part_pre);
  dprintf("total_post = %u\n", total_post);
  dprintf("part_post = %u\n", part_post);
  
  return ((part_post - part_pre) * 10000) / (total_post - total_pre);
}
