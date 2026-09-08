// user/hello.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("hello, xv6!\n");

  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; i++)
    printf("  argv[%d] = %s\n", i, argv[i]);

  exit(0);
}
