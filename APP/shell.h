#ifndef __SHELL_H
#define __SHELL_H
typedef int (*pr)(const char *, ...);

struct cmd_tbl {
  char *name;
  int (*cmd) (int argc, char *argv[], pr p);
};

#endif
