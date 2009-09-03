#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "stack.h"

void
test_array()
{
  callspace* myspace = create_callspace();

  int ar = array_create(myspace);
  int i;
  
  char buildstr[100];
  
  for (i=0; i < 100000; i++)
  {
    sprintf(buildstr, "test %d", i);
    array_append(myspace, ar, string_append(myspace, buildstr));
  }
  
  int *s_ary = array_get(myspace, ar);
  int s_arc  = array_length(myspace, ar);
  
  for (i=0; i < s_arc; i++)
  {
    sprintf(buildstr, "test %d", i);
    assert(strcmp(string_get(myspace, s_ary[i]), buildstr) == 0);
  }
  
  destroy_callspace(myspace);
  
  printf("test_array: OK\n");
}

int
main()
{
  test_array();
  return 0;
}
