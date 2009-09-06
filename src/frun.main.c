#include <stdio.h>

#include "frun.h"

int test_f(void *, int argc, int argv[])
{
  printf("FRUN OK\n");
}

frun_option functions[] = {
  {.func = test_f, .name = "test", .argc = 3},
  {.func = NULL}
};

int main()
{
  frun_option *func;
  func = frun_get(functions, "test");
  
  if (func->func != NULL)
  {
    func->func(NULL, 0, NULL);
  }
  else
  {
    printf("FRUN NOT OK");
  }
  
  return 0;
}
