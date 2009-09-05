/*
 *
 * Keep a dynamic list of functions which can be triggered by string matching,  which will also:
 * - keep track of the amount of arguments.
 */

#include <string.h>
#include <stdio.h>

#include "frun.h"

frun_option *frun_get(frun_option *options, const char *name)
{
  int i = 0;
  frun_option *func = options;
  
  while (func->func != NULL)
  {
    if (strcmp(func->name, name) == 0)
    {
      return func;
    }
    
    func = ++options;
  }
  
  return NULL;
}
