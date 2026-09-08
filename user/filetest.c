// user/filetest.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(void)
{
  char buf[64];
  struct stat st;

  // ① 파일을 만들어 연다
  int fd = open("mydata.txt", O_CREATE | O_WRONLY);
  if (fd < 0) {
    printf("open for write failed\n");
    exit(1);
  }
  printf("write fd = %d\n", fd);

  // ② 쓴다
  int n = write(fd, "hello xv6\n", 10);
  printf("wrote %d bytes\n", n);
  close(fd);

  // ③ 다시 연다 — fd 번호를 눈여겨보세요
  fd = open("mydata.txt", O_RDONLY);
  printf("read  fd = %d\n", fd);

  // ④ 읽는다
  n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0) {
    printf("read failed\n");
    exit(1);
  }
  buf[n] = 0;
  printf("read %d bytes: %s", n, buf);

  // ⑤ 파일 정보를 확인한다
  fstat(fd, &st);
  printf("size = %ld bytes, inode = %d\n", st.size, st.ino);

  close(fd);
  exit(0);
}
