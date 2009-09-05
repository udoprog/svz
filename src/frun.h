#ifndef _FRUN_H_
#define _FRUN_H_

typedef struct {
  void *func;
  char *name;
  int argc;
} frun_option;

frun_option *frun_get(frun_option *options, const char *name);

#endif /* _FRUN_H_ */
