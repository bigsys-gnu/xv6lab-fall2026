// user/forkbench.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NCHILD 5
#define NSTEP  3

int
main(void)
{
  for (int i = 0; i < NCHILD; i++) {
    int pid = fork();

    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }

    if (pid == 0) {                       // 자식
      for (int j = 0; j < NSTEP; j++)
        printf("child %d (pid %d): step %d\n", i, getpid(), j);
      exit(0);
    }
    // 부모는 계속 돌아 다음 자식을 만든다
  }

  printf("parent: all children done\n");
  exit(0);
}
