#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define RDB_FILE_SIZE 512

#define FILE_PID    "pid"
#define FILE_ALIVE  "alive"

typedef struct
{
  char *path;
  uid_t uid;
} rdb;

void
rdb_reads(rdb *rd);

void
rdb_dumps(rdb *rd);

void
_rdb_build_structure(rdb *rd)
{
  FILE *f;
  struct stat rd_s;
  
  stat(rd->path, &rd_s);
  
  if (!S_ISDIR(rd_s.st_mode))
  {
    printf("target is not a directory\n");
    return 1;
  }

  if (rd_s.st_uid != rd->uid)
  {
    printf("Target must be owned by user\n");
    return 1;
  }
  
  if (!(S_IRUSR & rd_s.st_mode))
  {
    printf("Target is not readable\n");
    return 1;
  }
  
  if (!(S_IWUSR & rd_s.st_mode))
  {
    printf("Target is not writable\n");
    return 1;
  }
  
  return 0;
}

rdb*
rdb_open(const char *file)
{
  rdb *build = malloc(sizeof(rdb));
  
  FILE *f;
  
  f = fopen(file, "r");
  
  build->path = malloc(RDB_FILE_SIZE);
  build->uid = getuid();
  strcpy(build->path, file);
  
  if (_rdb_build_structure(build))
  {
    return NULL;
  }
  
  return build;
}

void
rdb_dumps(rdb *rd)
{
}

void
rdb_reads(rdb *rd)
{
}

int main()
{
  rdb *mydb = rdb_open("/sbin");
}
