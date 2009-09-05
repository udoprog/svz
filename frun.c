/*
 *
 * Keep a dynamic list of functions which can be triggered by string matching,  which will also:
 * - keep track of the amount of arguments.
 */

#include <string.h>

#include "frun.h"

void *frun_get(frun_option options[], const char *name)
{
  int i = 0;
  frun_option func = options[i];
  
  while (func.func != NULL)
  {
    if (strcmp(func.name, name) == 0)
    {
      return func.func;
    }
    
    func = options[i++];
  }
  
  return NULL;
}
