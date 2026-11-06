// mkfs/fsview.c — xv6 의 fs.img 를 읽어 디스크 배치 · inode · 디렉터리를 출력한다
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/types.h"
#include "kernel/fs.h"

FILE *img;
struct superblock sb;   // IBLOCK 이 sb.inodestart 를 쓴다 — 이름이 꼭 sb

// 블록 b 를 buf 로 읽는다 : 블록 b = fs.img 의 (b * BSIZE) 바이트부터 BSIZE 바이트
void
read_block(uint b, void *buf)
{
  fseek(img, (long)b * BSIZE, SEEK_SET);
  if (fread(buf, BSIZE, 1, img) != 1) {
    fprintf(stderr, "fs.img 의 블록 %u 을 읽지 못했습니다\n", b);
    exit(1);
  }
}

// 디스크 배치 : 슈퍼블록에 적힌 시작 블록으로 영역을 나눈다
// (각 영역은 다음 영역의 시작 바로 앞에서 끝난다)
void
layout(void)
{
  uint datastart = sb.size - sb.nblocks;

  printf("block    0          boot\n");
  printf("block    1          superblock\n");
  printf("block %4u - %4u  log\n", sb.logstart, sb.inodestart - 1);
  printf("block %4u - %4u  inode table (%u inodes, %u per block)\n",
         sb.inodestart, sb.bmapstart - 1, sb.ninodes, (uint)IPB);
  printf("block %4u - %4u  bitmap\n", sb.bmapstart, datastart - 1);
  printf("block %4u - %4u  data\n", datastart, sb.size - 1);
}

// inode inum 을 *dip 로 읽는다
void
read_inode(uint inum, struct dinode *dip)
{
  struct dinode buf[IPB];   // 블록 하나 = inode 16개
  read_block(IBLOCK(inum, sb), buf);   // inum이 든 inode 블록을 통째로 읽음
  *dip = buf[inum % IPB];              // 그 블록 안 inum 칸을 복사

  // TODO ①  inum 이 든 블록을 buf 로 읽고, 그 안의 inum 칸을 *dip 에 복사한다.
  //          - 블록 번호  : IBLOCK(inum, sb)
  //          - 블록 안 칸 : inum % IPB
}

// inode 하나를 출력한다
void
show_inode(uint inum)
{
  struct dinode di;

  read_inode(inum, &di);
  printf("inode %u : block %u, slot %u\n", inum, (uint)IBLOCK(inum, sb), (uint)(inum % IPB));
  printf("  type %d  nlink %d  size %u\n", di.type, di.nlink, di.size);
  printf("  data blocks:");
  for (int i = 0; i < NDIRECT; i++)    // 직접 블록 addrs[0] ~ [11]
    if (di.addrs[i])
      printf(" %u", di.addrs[i]);
  if (di.addrs[NDIRECT])               // addrs[12] = 간접 블록
    printf("  (indirect %u)", di.addrs[NDIRECT]);
  printf("\n");
}

// 디렉터리 inum 의 엔트리를 출력한다
// (디렉터리 내용의 첫 블록만 본다 — 엔트리 64개까지)
void
list_dir(uint inum)
{
  struct dinode di;
  struct dirent de[BSIZE / sizeof(struct dirent)];  // 블록 하나 = 엔트리 64개
  uint n;

  read_inode(inum, &di);
  if (di.type != 1) {          // 1 = 디렉터리 (T_DIR)
    printf("inode %u 은 디렉터리가 아닙니다\n", inum);
    return;
  }
  read_block(di.addrs[0], de);         // 디렉터리 내용의 첫 블록을 엔트리 배열로 읽음
                                       //
  // TODO ②  디렉터리 내용의 첫 블록을 de 로 읽는다.
  //          - 그 블록 번호는 inode 의 addrs[0] 에 적혀 있다

  n = di.size / sizeof(struct dirent);   // 엔트리 수 = 크기 / 16
  if (n > BSIZE / sizeof(struct dirent))
    n = BSIZE / sizeof(struct dirent);
  for (uint k = 0; k < n; k++)
    if (de[k].inum != 0)       // inum 0 = 빈 칸
      printf("  %-14.14s inode %u\n", de[k].name, de[k].inum);
}

int
main(int argc, char *argv[])
{
  uchar buf[BSIZE];

  if (argc < 3 || (img = fopen(argv[1], "rb")) == 0) {
    fprintf(stderr, "usage: fsview fs.img layout | inode N | dir N\n");
    exit(1);
  }
  read_block(1, buf);                                // 슈퍼블록은 언제나 블록 1
  memmove(&sb, buf, sizeof(sb));

  if (strcmp(argv[2], "layout") == 0)
    layout();
  else if (strcmp(argv[2], "inode") == 0 && argc > 3)
    show_inode(atoi(argv[3]));
  else if (strcmp(argv[2], "dir") == 0 && argc > 3)
    list_dir(atoi(argv[3]));
  else
    fprintf(stderr, "usage: fsview fs.img layout | inode N | dir N\n");
  return 0;
}
