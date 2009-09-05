#include <stdio.h>

#include "frun.h"

int test_f(int argc, int argv[])
{
  printf("FRUN OK\n");
}

frun_option functions[] = {
  {.func = test_f, .name = "test", .argc = 3},
  {.func = NULL}
};

int main()
{
  int (*func)(int, int*);
  func = frun_get(functions, "test");
  
  if (func != NULL)
  {
    func(0, NULL);
  }
  else
  {
    printf("FRUN NOT OK");
  }
  
  return 0;
}
