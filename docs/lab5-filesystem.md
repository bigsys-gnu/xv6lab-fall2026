# Lab 5 — 파일 시스템 들여다보기

**fsdump (코드 제공) · get_fragmentation() · locality-aware balloc**

*파일의 블록 배치를 보고, 재고, 할당을 고쳐 바꾸기  ·  약 2시간 40분*

> 환경 설정은 [lab0-setup.md](lab0-setup.md), git · gdb 사용법은 [utils.md](utils.md) 를 참고하세요.

## 목차

- [개요](#개요)
  - [강의와 이어지는 부분](#강의와-이어지는-부분)
  - [이번 실습에 사용하는 구조체와 매크로](#이번-실습에-사용하는-구조체와-매크로)
  - [실습 브랜치 만들기](#실습-브랜치-만들기)
  - [문서 읽는 법](#문서-읽는-법)
- [5.0 fsdump — 파일의 블록 번호 보기 (코드 제공)](#50-fsdump-파일의-블록-번호-보기-코드-제공)
  - [5.0.1 목표](#501-목표)
  - [5.0.2 학습 내용](#502-학습-내용)
  - [5.0.3 단계별 구현](#503-단계별-구현)
  - [5.0.4 출력 읽는 법](#504-출력-읽는-법)
  - [5.0.5 확인 질문](#505-확인-질문)
  - [5.0.6 변경 요약](#506-변경-요약)
- [5.1 get_fragmentation() — 블록이 몇 조각인지 재기](#51-get_fragmentation-블록이-몇-조각인지-재기)
  - [5.1.1 목표](#511-목표)
  - [5.1.2 학습 내용](#512-학습-내용)
  - [5.1.3 단계별 구현](#513-단계별-구현)
  - [5.1.4 출력 읽는 법](#514-출력-읽는-법)
  - [5.1.5 확인 질문](#515-확인-질문)
  - [5.1.6 변경 요약](#516-변경-요약)
- [5.2 locality-aware balloc — 블록을 이어서 놓기](#52-locality-aware-balloc-블록을-이어서-놓기)
  - [5.2.1 목표](#521-목표)
  - [5.2.2 학습 내용](#522-학습-내용)
  - [5.2.3 단계별 구현](#523-단계별-구현)
  - [5.2.4 출력 읽는 법](#524-출력-읽는-법)
  - [5.2.5 확인 질문](#525-확인-질문)
  - [5.2.6 변경 요약](#526-변경-요약)
- [커밋하고 올리기](#커밋하고-올리기)
- [부록 A — 자주 막히는 곳](#부록-a-자주-막히는-곳)

## 개요

강의에서 파일의 데이터 블록 번호가 inode 의 `addrs[]` 에 적힌다는 것과, 새 블록은 `balloc` 이 bitmap 에서 골라 준다는 것을 봤습니다. 지금 xv6 의 `balloc` 은 언제나 bitmap 의 첫 빈 블록을 주기 때문에, 두 파일에 번갈아 쓰면 블록이 섞인다는 것도 봤습니다. 이번 실습에서는 그 현상을 **눈으로 보고, 숫자로 재고, 할당을 고쳐 숫자를 바꿉니다.**

실습은 세 개입니다. **실습 0 (5.0)** 은 파일의 inode 와 블록 번호를 보여 주는 `fsdump` 를 넣습니다. 코드를 모두 제공하므로 그대로 넣고 써 보며, 이후 실습의 결과를 확인하는 도구로 씁니다. **실습 1 (5.1)** 은 시스템 호출을 하나 더해 xv6 **안에서** 파일의 블록이 몇 조각인지 잽니다. **실습 2 (5.2)** 는 `kernel/fs.c` 의 블록 할당을 고쳐 배치를 바꿉니다.

| 실습      | 무엇을                                                      | 난이도 | 시간  |
| ------- | -------------------------------------------------------- | --- | --- |
| 0 (5.0) | `fsdump` — 파일의 inode 와 블록 번호를 보는 시스템 호출 (코드 제공, TODO 없음) | 입문  | 30분 |
| 1 (5.1) | `get_fragmentation()` — 블록이 몇 조각인지 재는 시스템 호출 (TODO 1칸)   | 초급  | 40분 |
| 2 (5.2) | locality-aware `balloc` — 같은 파일의 블록을 이어서 놓기 (TODO 2칸)    | 중급  | 80분 |
| —       | 커밋하고 올리기                                                 | —   | 10분 |

> ℹ️ **실습이 이어집니다** 실습 1 은 실습 0 의 `fill_fsinfo` 를, 실습 2 는 실습 0 · 1 의 `fsdump` · `fragtest` 를 그대로 써서 결과를 비교합니다. 한 브랜치(`lab5-filesystem`)에서 차례로 진행하세요. 다음 실습 Lab 6 (learned-fs) 은 Lab 5 를 마친 이 브랜치에서 새 브랜치 `lab6-learned-fs` 를 만들어 이어갑니다.

### 강의와 이어지는 부분

실습을 하다 막히면 해당 슬라이드를 다시 펴 보세요.

| 강의에서 본 것                                          | 이번에 확인하는 곳              |
| ------------------------------------------------- | ----------------------- |
| inode 에 종류 · 링크 수 · 크기 · 데이터 블록 번호가 있다            | 5.0 `fsdump` 출력         |
| inode 의 `addrs[13]` — 직접 12칸 + 간접 블록 1칸 (다단계 인덱스) | 5.0 제공 코드 `fill_fsinfo` |
| balloc — bitmap 에서 처음 만난 0 비트를 1 로                | 5.1 결과, 5.2 Step 1      |
| 빈 블록 고르기 — 번갈아 쓰면 f1 · f2 가 8조각으로 흩어진다            | 5.1 fragtest            |
| bmap — 파일의 bn번째 블록 번호, 없으면 balloc 으로 받기           | 5.2 Step 2              |
| 블록을 고칠 때의 규칙 — bread · log_write · brelse         | 5.0 제공 코드, 5.2 Step 1   |

### 이번 실습에 사용하는 구조체와 매크로

모두 `kernel/fs.h` 에 있습니다. 제공 코드와 직접 고치는 코드에 그대로 나옵니다.

| 이름                  | 값 · 정의                       | 하는 일                                                              |
| ------------------- | ---------------------------- | ----------------------------------------------------------------- |
| `BSIZE`             | 1024                         | 블록 크기 (바이트)                                                       |
| `struct superblock` | 블록 1 의 내용 (커널에서는 전역 변수 `sb`) | 전체 블록 수 `size` · 각 영역의 시작 블록 (`inodestart` · `bmapstart`)         |
| `struct dinode`     | 64바이트                        | 디스크의 inode — `type` · `nlink` · `size` · `addrs[13]`              |
| `NDIRECT`           | 12                           | inode 의 직접 블록 칸 수 (`addrs[0]` ~ `addrs[11]`, `addrs[12]` 는 간접 블록) |
| `NINDIRECT`         | `BSIZE / sizeof(uint)` = 256 | 간접 블록 하나에 든 블록 번호 수                                               |
| `BPB`               | `BSIZE * 8` = 8192           | bitmap 블록 하나가 맡는 블록 수 (비트 하나 = 블록 하나)                             |
| `BBLOCK(b, sb)`     | `b / BPB + sb.bmapstart`     | 블록 `b` 의 비트가 들어 있는 **bitmap 블록 번호** (지금은 늘 46)                    |

### 실습 브랜치 만들기

```
git checkout riscv                 # 손대지 않은 원본으로 이동
git checkout -b lab5-filesystem    # 브랜치를 만들고 그리로 이동
git branch                         # * lab5-filesystem 로 바뀌었는지 확인
```

### 문서 읽는 법

- 각 단계의 제목에 **만들 파일 경로** 가 붙어 있습니다.
- **코드는 `// TODO` 로 표시한 곳만 비어 있습니다.** 번호(①, ②)는 실습마다 새로 매깁니다. 실습 0 에는 TODO 가 없습니다.
- 문서의 출력은 `riscv` 브랜치에 각 실습의 코드를 더해 빌드한 **실제 값** 입니다.
- 명령 앞의 프롬프트로 어디서 치는지 구분합니다. **`linux$`** 는 호스트(리눅스) 터미널, **`$`** 는 QEMU 안의 xv6 셸입니다. `make` 는 호스트에서, `fsdump` · `fragtest` 는 xv6 안에서 실행합니다.

> ⚠️ **번호는 실습마다, 그리고 강의 슬라이드와 다릅니다** inode 번호와 블록 번호는 fs.img 에 들어가는 프로그램 수와 크기로 정해집니다. a.txt 는 실습 0 빌드에서 **inode 25 · 블록 988**, `fragtest` 가 더해진 실습 1 부터 **inode 26 · 블록 1027 (실습 2 뒤에는 1040)** 입니다. 강의 슬라이드는 `user/stat.c` 를 더한 다른 빌드의 값입니다. 디스크 배치(슈퍼블록 1, inode 표 33 ~ 45, bitmap 46, 데이터 47 ~)는 모두 같습니다.

---

## 5.0 fsdump — 파일의 블록 번호 보기 (코드 제공)

### 5.0.1 목표

열린 파일의 inode 정보와 **데이터 블록 번호 목록** 을 돌려주는 시스템 호출 `fsdump(fd, &info)` 와, 그 결과를 출력하는 사용자 프로그램 `fsdump` 를 넣습니다. **코드는 모두 제공합니다.** 그대로 넣고 실행해 보며, 강의에서 그림으로 본 inode · `addrs[]` · 간접 블록을 실제 숫자로 확인합니다.

```
$ fsdump a.txt
a.txt: inode 25, type 2, nlink 1, size 6, 1 blocks
  blocks: 988
```

이후 실습 1 · 2 와 Lab 6 에서 블록 배치를 확인하는 도구로 계속 씁니다.

### 5.0.2 학습 내용

- 시스템 호출을 더하는 길 — 번호 · 표 등록 · 사용자 쪽 선언 · 커널 함수 (Lab 2 와 같은 길)
- 커널이 열린 파일의 블록 번호를 모으는 길 — fd → `struct file` → inode 의 `addrs[]` → 간접 블록
- 커널이 블록을 빌리고 돌려주는 짝 — `bread` · `brelse`, 그리고 inode 를 읽는 동안의 `ilock` · `iunlock`
- 커널이 만든 구조체를 사용자에게 넘기는 방법 — `copyout`

### 5.0.3 단계별 구현

**fd → 블록 번호** — 커널 안에서 따라가는 길입니다. 코드를 읽을 때 이 순서를 떠올리세요.

```
fd  ──(argfd)──▶  struct file  ──(f->ip)──▶  struct inode
                                              addrs[0] ~ addrs[11]  ──▶ 데이터 블록 번호
                                              addrs[12]             ──▶ 간접 블록
                                                                       a[0] ~ a[255] ──▶ 데이터 블록 번호
```

#### Step 1 — 커널과 사용자가 함께 쓸 구조체  `(kernel/fsinfo.h, 새 파일)`

```c
// fsdump이 사용자에게 돌려주는 파일 정보 (커널과 사용자가 함께 씀)
struct fsinfo {
  uint inum;            // inode 번호
  short type;           // T_FILE, T_DIR, T_DEVICE
  short nlink;
  uint size;            // 바이트
  uint nblocks;         // 데이터 블록 수 (간접 블록 자체는 뺌)
  uint indirect;        // 간접 블록 번호 (없으면 0)
  uint blocks[NDIRECT + NINDIRECT]; // 파일의 0번째, 1번째, ... 블록 번호
};
```

#### Step 2 — 커널 쪽 함수  `(kernel/sysfile.c)`

파일 위쪽 `#include` 목록 끝에 두 줄, 파일 맨 아래에 두 함수를 넣습니다.

```c
#include "fcntl.h"
#include "buf.h"        // 추가 — bread 가 돌려주는 struct buf
#include "fsinfo.h"     // 추가
```

```c
// kernel/sysfile.c  — 파일 맨 아래에 추가

// ip 의 정보와 데이터 블록 번호 목록을 *fi 에 채운다. ip 는 잠겨 있어야 한다.
static void
fill_fsinfo(struct inode *ip, struct fsinfo *fi)
{
  memset(fi, 0, sizeof(*fi));
  fi->inum = ip->inum;
  fi->type = ip->type;
  fi->nlink = ip->nlink;
  fi->size = ip->size;

  // 직접 블록 addrs[0] ~ addrs[NDIRECT - 1] — 0 이 아닌 번호만 차례로 모은다
  for (int i = 0; i < NDIRECT; i++)
    if (ip->addrs[i])
      fi->blocks[fi->nblocks++] = ip->addrs[i];

  // 간접 블록 addrs[NDIRECT] — 블록 번호 256개가 든 블록
  fi->indirect = ip->addrs[NDIRECT];
  if (fi->indirect) {
    struct buf *bp = bread(ip->dev, fi->indirect);
    uint *a = (uint *)bp->data;          // 간접 블록 = uint 256개짜리 배열
    for (int j = 0; j < NINDIRECT; j++)
      if (a[j])
        fi->blocks[fi->nblocks++] = a[j];
    brelse(bp);
  }
}

// int fsdump(int fd, struct fsinfo *info)
uint64
sys_fsdump(void)
{
  struct file *f;
  uint64 addr;
  int r = -1;

  argaddr(1, &addr);                     // 1번째 인자 = 사용자 쪽 struct fsinfo 의 주소
  if (argfd(0, 0, &f) < 0 || f->type != FD_INODE)
    return -1;                           // 0번째 인자 = 열린 파일의 fd
  struct fsinfo *fi = kalloc();          // 1 KB 가 넘어 커널 스택 대신 페이지 하나를 빌린다
  if (fi == 0)
    return -1;
  ilock(f->ip);                          // inode 를 읽는 동안 잠근다
  fill_fsinfo(f->ip, fi);
  iunlock(f->ip);
  struct proc *p = myproc();
  if (copyout(p->pagetable, p->sz, addr, (char *)fi, sizeof(*fi)) >= 0)
    r = 0;                               // 사용자 메모리로 복사
  kfree(fi);                             // 빌린 페이지를 돌려준다
  return r;
}
```

> ℹ️ **제공 코드에서 볼 곳** `ilock` · `iunlock` — inode 를 읽는 동안 잠근다. `bread` · `brelse` — 간접 블록을 빌리고 꼭 돌려준다(사본은 30개뿐). `kalloc` · `kfree` — 1092바이트 구조체를 커널 스택(4 KB) 대신 페이지에 둔다. `copyout` — 커널이 만든 구조체를 사용자 메모리로 복사한다. 실습 2 에서 `fs.c` 를 고칠 때도 같은 짝(`bread` · `brelse`)을 씁니다.

#### Step 3 — 시스템 호출을 등록한다  `(kernel/syscall.h, kernel/syscall.c, user/user.h, user/usys.pl)`

`fsdump` 는 **23** 번입니다.

```c
// kernel/syscall.h
#define SYS_sync   22
#define SYS_fsdump 23                         // 추가

// kernel/syscall.c — 다른 extern 옆에
extern uint64 sys_fsdump(void);               // 추가

// kernel/syscall.c — syscalls[] 배열 안에
  [SYS_sync]    = sys_sync,
  [SYS_fsdump]  = sys_fsdump,                 // 추가

// user/user.h — system calls 묶음 안에
struct fsinfo;
int fsdump(int, struct fsinfo *);

// user/usys.pl — entry 목록 끝에
entry("fsdump");
```

> ℹ️ **`struct fsinfo;` 한 줄** user.h 는 fsinfo.h 를 include 하지 않으므로, 포인터만 쓰는 선언을 위해 "이런 구조체가 있다" 고만 알려 줍니다.

#### Step 4 — 사용자 프로그램  `(user/fsdump.c, 새 파일)`

```c
// user/fsdump.c — 파일의 inode 번호, 크기, 블록 번호를 출력
#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/fsinfo.h"
#include "user/user.h"

struct fsinfo fi;

int
main(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0 || fsdump(fd, &fi) < 0) {
      printf("fsdump: %s failed\n", argv[i]);
      exit(1);
    }
    close(fd);
    printf("%s: inode %d, type %d, nlink %d, size %d, %d blocks\n",
           argv[i], fi.inum, fi.type, fi.nlink, fi.size, fi.nblocks);
    printf("  blocks:");
    for (int j = 0; j < fi.nblocks; j++)
      printf(" %d", fi.blocks[j]);
    if (fi.indirect)
      printf("  (indirect %d)", fi.indirect);
    printf("\n");
  }
  exit(0);
}
```

#### Step 5 — 빌드 목록에 넣고 실행한다  `(Makefile)`

```
# Makefile 의 UPROGS 에서 $U/_sync\ 아래에 추가
    $U/_fsdump\
```

프로그램이 늘어 fs.img 를 다시 만들어야 하므로 **`make clean` 부터** 합니다.

```
linux$ make clean
linux$ make qemu
$ echo hello > a.txt
$ fsdump a.txt README
a.txt: inode 25, type 2, nlink 1, size 6, 1 blocks
  blocks: 988
README: inode 2, type 2, nlink 1, size 2441, 3 blocks
  blocks: 48 49 50
$ fsdump fsdump
fsdump: inode 23, type 2, nlink 1, size 35352, 35 blocks
  blocks: 952 953 954 955 956 957 958 959 960 961 962 963 965 966 … 987  (indirect 964)
```

`fsdump fsdump` 의 실제 출력은 한 줄이 길어 가운데를 `…` 로 줄였습니다(965 ~ 987 이 이어짐).

### 5.0.4 출력 읽는 법

| 출력                          | 무엇인가                                                                      |
| --------------------------- | ------------------------------------------------------------------------- |
| `a.txt: inode 25, type 2`   | 방금 만든 파일의 inode 번호. type 2 = `T_FILE`                                     |
| `a.txt … blocks: 988`       | 6바이트라 데이터 블록 하나. inode 의 `addrs[0]` 에 적힌 번호. mkfs 가 쓴 마지막 블록 바로 뒤의 첫 빈 블록 |
| `README … blocks: 48 49 50` | mkfs 가 루트 디렉터리(블록 47) 다음에 맨 먼저 넣은 파일이라 데이터 영역 맨 앞                         |
| `fsdump: … 35 blocks`       | 35352바이트 ÷ 1024 → 35블록. 직접 12블록 + 간접 블록 안 23블록                            |
| `(indirect 964)`            | 간접 블록 자체의 번호. 데이터 블록 963 과 965 사이에 끼어 있음                                  |

```
블록    952 … 963   964        965 … 987
        addrs[0~11] addrs[12]  간접 블록 안 a[0] ~ a[22]
        직접 12블록  간접 블록   데이터 23블록
```

### 5.0.5 확인 질문

**(1)**&nbsp;&nbsp; `fsdump fsdump` 에서 간접 블록 964 가 데이터 블록 963 과 965 **사이** 에 놓인 이유는 무엇입니까? (힌트 — mkfs 가 파일을 블록 단위로 덧붙이다가 13번째 블록에서 무엇을 먼저 할당하나요?)

**(2)**&nbsp;&nbsp; `fsdump .` 을 실행하면 type 과 블록 번호는 무엇이 나옵니까? 강의에서 본 디스크 배치의 어느 블록입니까? 이어서 `fsdump console` 을 실행하면 `failed` 가 나옵니다. 제공 코드의 어느 조건 때문입니까?

**(3)**&nbsp;&nbsp; `echo hello > b.txt` 로 파일을 하나 더 만들고 `fsdump b.txt` 를 실행하면 inode 번호와 블록 번호는 몇일까요? 예측하고 확인하세요.

**(4)**&nbsp;&nbsp; 제공 코드 `fill_fsinfo` 에서 간접 블록을 읽은 뒤 `brelse(bp)` 를 빠뜨리면 어떤 일이 생길까요? `fsdump fsdump` 를 여러 번 실행한다고 생각해 보세요.

### 5.0.6 변경 요약

| #   | 파일                 | 무엇을 했나                                    |
| --- | ------------------ | ----------------------------------------- |
| 1   | `kernel/fsinfo.h`  | 새 파일 — struct fsinfo                      |
| 2   | `kernel/sysfile.c` | include 두 줄, fill_fsinfo() · sys_fsdump() |
| 3   | `kernel/syscall.h` | SYS_fsdump 23                             |
| 4   | `kernel/syscall.c` | extern + 배열 등록                            |
| 5   | `user/user.h`      | 선언 두 줄                                    |
| 6   | `user/usys.pl`     | entry 한 줄                                 |
| 7   | `user/fsdump.c`    | 새 파일                                      |
| 8   | `Makefile`         | UPROGS 에 _fsdump                          |

---

## 5.1 get_fragmentation() — 블록이 몇 조각인지 재기

### 5.1.1 목표

파일의 데이터 블록이 디스크에서 **몇 개의 연속 구간(조각)** 으로 나뉘어 있는지 숫자 하나로 돌려주는 시스템 호출 `get_fragmentation(fd)` 를 만들고, 두 파일에 번갈아 쓰면 블록이 섞인다는 것을 숫자로 확인합니다.

```
블록 48 49 50          → 1조각     (README)
블록 1028 1030 1032 …  → 8조각     (번갈아 쓴 f1)
```

블록 번호 목록은 실습 0 의 `fill_fsinfo` 로 받습니다. 이 실습에서 직접 쓰는 것은 조각을 세는 부분입니다.

### 5.1.2 학습 내용

- "이어져 있다" = 다음 블록 번호가 **앞 번호 + 1**
- 실습 0 의 함수(`fill_fsinfo`)를 다른 시스템 호출에서 다시 쓰기
- 지금 xv6 의 `balloc` 은 언제나 **bitmap 의 첫 빈 블록** 을 준다 — 그래서 두 파일에 번갈아 쓰면 블록을 하나씩 나눠 가진다

### 5.1.3 단계별 구현

#### Step 1 — 시스템 호출을 등록한다  `(kernel/syscall.h, kernel/syscall.c, user/user.h, user/usys.pl)`

`get_fragmentation` 은 **24** 번입니다. 실습 0 에서 넣은 `fsdump` 줄 아래에 하나씩 더합니다.

```c
// kernel/syscall.h
#define SYS_fsdump 23
#define SYS_get_fragmentation 24              // 추가

// kernel/syscall.c — extern sys_fsdump 아래에
extern uint64 sys_get_fragmentation(void);    // 추가

// kernel/syscall.c — syscalls[] 배열 안, [SYS_fsdump] 아래에
  [SYS_get_fragmentation] = sys_get_fragmentation,   // 추가

// user/user.h — int fsdump(...) 아래에
int get_fragmentation(int);

// user/usys.pl — entry("fsdump"); 아래에
entry("get_fragmentation");
```

#### Step 2 — get_fragmentation 을 구현한다  `(kernel/sysfile.c)`

`sys_fsdump` 아래에 넣고 **TODO ①** 을 채웁니다.

```c
// kernel/sysfile.c  — sys_fsdump 아래에 추가

// int get_fragmentation(int fd)
// 파일의 블록이 몇 조각(연속 구간)으로 나뉘어 있는지 돌려준다.
// 블록이 없으면 0, 모두 이어져 있으면 1.
uint64
sys_get_fragmentation(void)
{
  struct file *f;

  if (argfd(0, 0, &f) < 0 || f->type != FD_INODE)
    return -1;
  struct fsinfo *fi = kalloc();
  if (fi == 0)
    return -1;
  ilock(f->ip);
  fill_fsinfo(f->ip, fi);                // 실습 0 의 함수로 블록 번호 목록을 받는다
  iunlock(f->ip);

  int frags = 0;
  for (int i = 0; i < fi->nblocks; i++) {
    // TODO ①  새 조각이 시작되는 블록이면 frags 를 하나 늘린다.
    //          - 첫 블록(i == 0)은 언제나 새 조각
    //          - 그 밖에는 앞 블록 번호 + 1 이 아니면 새 조각
  }
  kfree(fi);
  return frags;
}
```

> ℹ️ **세는 규칙 하나** 블록 번호를 차례로 보면서 "앞 번호 + 1 이 아닌 곳" 마다 새 조각이 시작됩니다. `1028 1030 1032` 는 매번 2씩 건너뛰므로 블록마다 새 조각, `48 49 50` 은 한 번도 끊기지 않아 1조각입니다. 간접 블록 자체는 `fill_fsinfo` 가 목록에서 빼 두었으므로 세지 않습니다.

#### Step 3 — 테스트 프로그램  `(user/fragtest.c, 새 파일)`

두 파일 f1, f2 에 1 KB(블록 하나)씩 여덟 번 **번갈아** 쓰고, 각 파일의 블록 번호와 조각 수를 출력합니다.

```c
// user/fragtest.c — 두 파일에 1 KB씩 번갈아 써서 블록이 섞이는지 본다
#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/fsinfo.h"
#include "user/user.h"

#define NBLK 8      // 파일마다 쓸 블록 수

char buf[BSIZE];
struct fsinfo fi;

void
show(char *name, int fd)
{
  fsdump(fd, &fi);
  printf("%s: inode %d, %d blocks:", name, fi.inum, fi.nblocks);
  for (int i = 0; i < fi.nblocks; i++)
    printf(" %d", fi.blocks[i]);
  printf("\n  fragments = %d\n", get_fragmentation(fd));
}

int
main(void)
{
  unlink("/f1");
  unlink("/f2");
  int a = open("/f1", O_CREATE | O_RDWR);
  int b = open("/f2", O_CREATE | O_RDWR);
  for (int i = 0; i < NBLK; i++) {   // f1, f2, f1, f2, ... 한 블록씩
    memset(buf, 'A' + i, BSIZE);
    write(a, buf, BSIZE);
    write(b, buf, BSIZE);
  }
  show("f1", a);
  show("f2", b);
  close(a);
  close(b);
  exit(0);
}
```

#### Step 4 — 빌드 목록에 넣고 실행한다  `(Makefile)`

```
# Makefile 의 UPROGS 에서 $U/_fsdump\ 아래에 추가
    $U/_fragtest\
```

이번에도 프로그램이 늘었으므로 **`make clean` 부터** 합니다. `fragtest` 가 fs.img 에 더해져 a.txt 의 번호가 실습 0 과 달라집니다.

```
linux$ make clean
linux$ make qemu
$ echo hello > a.txt
$ fsdump a.txt
a.txt: inode 26, type 2, nlink 1, size 6, 1 blocks
  blocks: 1027
$ fragtest
f1: inode 27, 8 blocks: 1028 1030 1032 1034 1036 1038 1040 1042
  fragments = 8
f2: inode 28, 8 blocks: 1029 1031 1033 1035 1037 1039 1041 1043
  fragments = 8
$ wc f1
0 1 8192 f1
```

**fragtest 의 출력이 실습 2 의 기준선입니다.** 실습 2 에서 커널을 고친 뒤 같은 명령을 다시 실행해 비교합니다.

### 5.1.4 출력 읽는 법

| 출력                       | 무엇인가                                                                        |
| ------------------------ | --------------------------------------------------------------------------- |
| `a.txt: inode 26 … 1027` | 실습 0 의 25 · 988 에서 바뀜. fragtest(inode 24)가 fs.img 에 더해져 mkfs 가 쓴 블록이 늘었기 때문 |
| `f1: … 1028 1030 1032 …` | a.txt 바로 뒤부터 f1 · f2 가 한 블록씩 번갈아 받음                                         |
| `fragments = 8`          | 블록마다 끊김 → 8블록이 8조각                                                          |
| `wc f1` 의 8192           | 흩어져 있어도 내용은 그대로 — 다단계 인덱스가 순서를 기억함                                          |

```
블록   1028 1029 1030 1031 1032 1033 …
파일    f1   f2   f1   f2   f1   f2  …
```

`write(f1)` → `write(f2)` → … 순서로 `balloc` 이 불리고, 매번 그때의 **첫 빈 블록** 을 줍니다. 그래서 두 파일이 블록을 하나씩 번갈아 받아 각각 8조각이 됩니다. 다단계 인덱스(addrs)는 흩어진 블록도 문제없이 적지만, 파일을 처음부터 읽을 때 디스크 위치가 매번 건너뜁니다.

### 5.1.5 확인 질문

**(1)**&nbsp;&nbsp; `fragtest` 의 for 문을 "f1 에 여덟 번 다 쓰고, 그다음 f2 에 여덟 번" 으로 바꾸면 fragments 는 각각 얼마가 될까요? 예측하고 실행해 확인하세요(확인 뒤 원래대로).

**(2)**&nbsp;&nbsp; 이 빌드에서 `fsdump fsdump` 를 실행하면 실습 0 처럼 간접 블록이 데이터 블록 사이에 끼어 있습니다. 이 파일의 `get_fragmentation` 값은 얼마일까요? 간접 블록을 조각 수에 넣지 않는 이유와 함께 답하세요.

**(3)**&nbsp;&nbsp; 블록이 없는 빈 파일에서 `get_fragmentation` 은 0 을 돌려줍니다. 코드의 어느 부분 때문입니까?

### 5.1.6 변경 요약

| #   | 파일                 | 무엇을 했나                   |
| --- | ------------------ | ------------------------ |
| 1   | `kernel/syscall.h` | SYS_get_fragmentation 24 |
| 2   | `kernel/syscall.c` | extern + 배열 등록           |
| 3   | `user/user.h`      | 선언 한 줄                   |
| 4   | `user/usys.pl`     | entry 한 줄                |
| 5   | `kernel/sysfile.c` | sys_get_fragmentation()  |
| 6   | `user/fragtest.c`  | 새 파일                     |
| 7   | `Makefile`         | UPROGS 에 _fragtest       |

---

## 5.2 locality-aware balloc — 블록을 이어서 놓기

### 5.2.1 목표

`balloc` 이 빈 블록을 **찾기 시작하는 위치(hint)** 를 받게 고쳐서, 같은 파일의 블록이 이어서 놓이게 만듭니다. 5.1 의 `fragtest` 결과를 **fragments 8 → 1** 로 바꾸는 것이 목표입니다.

```
지금:   f1 = 1028 1030 1032 … (8조각)   f2 = 1029 1031 1033 … (8조각)
목표:   f1 = 1056 1057 1058 … (1조각)   f2 = 1072 1073 1074 … (1조각)
```

### 5.2.2 학습 내용

- 블록 번호를 **적는 방식**(다단계 인덱스)은 그대로 두고, 빈 블록을 **고르는 정책** 만 바꿔도 배치가 달라진다는 것
- `balloc` 은 bitmap 을 훑어 0 비트를 찾고, `bmap` 은 파일의 몇 번째 블록인지 알고 있다 — 그래서 hint 는 `bmap` 이 정한다
- 정책 세 가지
  1. **같은 파일은 이어서** — bn번째 블록은 bn − 1번째 블록 + 1 부터 찾는다
  2. **새 파일은 떨어져서** — 첫 블록은 16칸이 모두 빈 묶음의 시작에서 찾는다. 다른 파일의 "바로 뒤" 를 빼앗지 않는다
  3. **간접 블록은 데이터 밖에** — 간접 블록은 hint 없이(0 부터) 앞쪽 빈 곳에 둔다
- 블록을 고칠 때의 규칙 — `bread` → 고치기 → `log_write` → `brelse`. 읽기만 하는 검사(`blockfree`)는 `log_write` 하지 않는다

### 5.2.3 단계별 구현

**이번 실습은 `kernel/fs.c` 한 파일만 고칩니다.** 5.0 · 5.1 의 코드와 테스트 프로그램은 그대로 씁니다.

#### Step 1 — balloc 에 hint 를 더한다  `(kernel/fs.c)`

원래 `balloc()` 함수(주석 `// Allocate a zeroed disk block.` 부터 함수 끝까지)를 지우고, 그 자리에 아래 세 함수를 넣습니다. `blockfree` 와 `freegroup` 은 다 나와 있고, `balloc` 의 **TODO ①** 을 채웁니다.

```c
// kernel/fs.c  — 원래 balloc() 을 지우고 이 세 함수로 바꾼다

// 블록 b 가 비어 있나 (bitmap 을 읽기만 한다)
static int
blockfree(uint dev, uint b)
{
  struct buf *bp = bread(dev, BBLOCK(b, sb));
  int bi = b % BPB;
  int free = (bp->data[bi / 8] & (1 << (bi % 8))) == 0;
  brelse(bp);
  return free;
}

// 데이터 영역에서 GROUP 칸이 모두 비어 있고, GROUP 의 배수 번호로 시작하는
// 묶음을 찾아 그 첫 블록을 돌려준다. 없으면 0.
#define GROUP 16
static uint
freegroup(uint dev)
{
  uint first = sb.bmapstart + sb.size / BPB + 1;   // 첫 데이터 블록 (47)
  for (uint g = (first + GROUP - 1) / GROUP * GROUP; g + GROUP <= sb.size; g += GROUP) {
    int ok = 1;
    for (uint b = g; b < g + GROUP && ok; b++)
      ok = blockfree(dev, b);
    if (ok)
      return g;
  }
  return 0;
}

// hint 부터 찾기 시작해, 끝에 닿으면 0 으로 돌아와 한 바퀴 훑는다.
// 처음 만난 빈 블록을 1 로 표시하고 0 으로 채운 뒤 번호를 돌려준다. 없으면 0.
static uint
balloc(uint dev, uint hint)
{
  int bi, m;
  struct buf *bp;

  if (hint >= sb.size)
    hint = 0;
  for (uint n = 0; n < sb.size; n++) {
    uint b = 0;
    // TODO ①  이번에 볼 블록 번호 b 를 정한다.
    //          - n == 0 이면 hint, 그다음 hint + 1, …
    //          - sb.size 에 닿으면 0 부터 다시 (나머지 연산 %)
    bp = bread(dev, BBLOCK(b, sb));
    bi = b % BPB;
    m = 1 << (bi % 8);
    if ((bp->data[bi / 8] & m) == 0) {   // 빈 블록인가?
      bp->data[bi / 8] |= m;             // 사용 중으로 표시
      log_write(bp);
      brelse(bp);
      bzero(dev, b);
      return b;
    }
    brelse(bp);
  }
  printk("balloc: out of blocks\n");
  return 0;
}
```

> ℹ️ **원래 balloc 과 무엇이 다른가** 원래 코드는 bitmap 블록마다 비트를 0번부터 훑었습니다(언제나 블록 0 부터). 새 코드는 블록 번호 b 를 하나씩 정해 그 비트를 봅니다. 비트를 보는 네 줄(`bread` · `bi` · `m` · 검사)은 원래 코드와 같고, **어디서부터 볼지** 만 달라졌습니다.

> ℹ️ **한 바퀴 도는 이유** hint 뒤쪽이 다 차 있어도 앞쪽에 빈 블록이 있을 수 있습니다. 끝에 닿으면 0 으로 돌아와 hint 앞까지 보면, 빈 블록이 하나라도 있는 한 반드시 찾습니다.

#### Step 2 — bmap 이 hint 를 정한다  `(kernel/fs.c)`

`balloc` 의 인자가 둘이 되었으므로 `bmap` 의 세 호출을 모두 바꿔야 합니다. 원래 `bmap()` 함수 전체를 아래 코드로 바꾸고 **TODO ②** 를 채웁니다. 간접 블록 부분은 다 나와 있습니다.

```c
// kernel/fs.c  — bmap() 전체를 이것으로 바꾼다 (balloc 을 부르는 세 곳에 hint 를 넘긴다)
static uint
bmap(struct inode *ip, uint bn)
{
  uint addr, *a, prev;
  struct buf *bp;

  if (bn < NDIRECT) {
    if ((addr = ip->addrs[bn]) == 0) {
      uint hint = 0;
      // TODO ②  찾기 시작할 곳 hint 를 정한다.
      //          - 파일의 첫 블록(bn == 0)   : 빈 묶음의 시작 → freegroup(ip->dev)
      //          - 그다음 블록               : 바로 앞 블록 뒤 → ip->addrs[bn - 1] + 1
      addr = balloc(ip->dev, hint);
      if (addr == 0)
        return 0;
      ip->addrs[bn] = addr;
    }
    return addr;
  }
  bn -= NDIRECT;

  if (bn < NINDIRECT) {
    // 간접 블록 읽기 (없으면 만들기) — 데이터 사이에 끼지 않게 앞쪽 빈 곳에
    if ((addr = ip->addrs[NDIRECT]) == 0) {
      addr = balloc(ip->dev, 0);
      if (addr == 0)
        return 0;
      ip->addrs[NDIRECT] = addr;
    }
    bp = bread(ip->dev, addr);
    a = (uint *)bp->data;
    if ((addr = a[bn]) == 0) {
      // 간접 블록 안의 데이터 블록도 "바로 앞 블록 뒤"에서 찾는다
      prev = (bn == 0) ? ip->addrs[NDIRECT - 1] : a[bn - 1];
      addr = balloc(ip->dev, prev ? prev + 1 : 0);
      if (addr) {
        a[bn] = addr;
        log_write(bp);
      }
    }
    brelse(bp);
    return addr;
  }

  panic("bmap: out of range");
}
```

> ℹ️ **hint 가 0 이어도 괜찮나** `freegroup` 이 빈 묶음을 못 찾으면 0 을 돌려줍니다. 그러면 `balloc` 은 블록 0 부터 찾으므로 원래 xv6 처럼 "첫 빈 블록" 이 됩니다. 디스크가 거의 찼을 때도 동작이 깨지지 않습니다.

> ⚠️ **TODO ② 를 채우기 전에는 빌드되지 않습니다** `freegroup` 을 부르는 곳이 TODO ② 뿐이라, 비워 두면 `'freegroup' defined but not used` 경고가 `-Werror` 로 오류가 됩니다.

#### Step 3 — 빌드하고 기준선과 비교한다

커널이 바뀌었으므로 **반드시 `make clean`** 으로 fs.img 를 새로 만들고 5.1 과 같은 순서로 실행합니다.

```
linux$ make clean
linux$ make qemu
$ echo hello > a.txt
$ fsdump a.txt
a.txt: inode 26, type 2, nlink 1, size 6, 1 blocks
  blocks: 1040
$ fragtest
f1: inode 27, 8 blocks: 1056 1057 1058 1059 1060 1061 1062 1063
  fragments = 1
f2: inode 28, 8 blocks: 1072 1073 1074 1075 1076 1077 1078 1079
  fragments = 1
$ wc f1
0 1 8192 f1
```

| 확인할 것             | 5.1 (원래 balloc)  | 5.2 (고친 balloc) |
| ----------------- | ---------------- | --------------- |
| f1 · f2 fragments | 8 · 8            | **1 · 1**       |
| f1 블록             | 1028 1030 1032 … | 1056 ~ 1063     |
| `wc f1` (내용이 맞는지) | 8192             | 8192            |

마지막으로 xv6 의 전체 테스트가 통과하는지 확인합니다. 몇 분 걸립니다.

```
$ usertests -q
…
ALL TESTS PASSED
```

### 5.2.4 출력 읽는 법

```
1026        1040            1056 ─────── 1063         1072 ─────── 1079
(mkfs 끝)   a.txt           f1 (8블록)                 f2 (8블록)
            = 16칸 묶음      = 다음 빈 16칸 묶음의 시작   = 그다음 묶음의 시작
```

- **a.txt 가 1027 이 아니라 1040 인 이유** — 첫 블록은 "16칸이 모두 빈, 16의 배수 번호로 시작하는 묶음" 에서 찾습니다. 1027 이 든 묶음 1024 ~ 1039 는 mkfs 가 1026 까지 썼으므로 다 비어 있지 않고, 다음 묶음의 시작이 1040 입니다.
- **f1 이 1056 부터인 이유** — 1040 묶음은 a.txt 가 쓰고 있으므로 그다음 빈 묶음 1056.
- **f1 의 둘째 블록부터** — "바로 앞 블록 + 1" 에서 찾으므로 1057, 1058, … 로 이어집니다. f2 가 중간에 끼어들지 못하는 것은 f2 의 첫 블록이 다른 묶음(1072)에서 시작했기 때문입니다.
- **대가** — 1027 ~ 1039, 1041 ~ 1055 는 당분간 비어 있는 채로 남습니다. 빈 묶음이 없어지면 `freegroup` 이 0 을 돌려 원래처럼 첫 빈 블록을 씁니다.

### 5.2.5 확인 질문

**(1)**&nbsp;&nbsp; TODO ② 에서 파일의 첫 블록도 `freegroup` 대신 0 을 hint 로 주면(나머지는 그대로) fragtest 결과는 어떻게 될까요? 예측하고 실행해 보세요(확인 뒤 원래대로). 정책 2 가 왜 필요한지 설명하세요.

**(2)**&nbsp;&nbsp; `blockfree` 는 bitmap 을 읽기만 하고 `log_write` 를 부르지 않습니다. 만약 `log_write` 를 불렀다면 무엇이 낭비됩니까? (힌트 — 시스템 호출 하나가 고칠 수 있는 블록 수에는 한도가 있습니다)

**(3)**&nbsp;&nbsp; 간접 블록이 생길 만큼 큰 파일을 xv6 안에서 만들어 봅시다. 5.1 커널과 5.2 커널에서 각각 새 fs.img 로 아래를 실행하고, 간접 블록 번호와 fragments 를 비교하세요. 간접 블록에 `balloc(ip->dev, 0)` 을 쓰는 이유를 이 결과로 설명하세요.

```
$ cat fsdump > big
$ fsdump big
```

(`fsdump fsdump` 로는 비교되지 않습니다. 프로그램 파일은 커널이 아니라 호스트의 mkfs 가 fs.img 에 넣기 때문입니다.)

**(4)**&nbsp;&nbsp; 이 정책이 HDD 에서는 왜 빨라지고, SSD 에서는 효과가 작을까요?

### 5.2.6 변경 요약

| #   | 파일            | 무엇을 했나                                               |
| --- | ------------- | ---------------------------------------------------- |
| 1   | `kernel/fs.c` | blockfree() · freegroup() 추가, balloc(dev, hint) 로 변경 |
| 2   | `kernel/fs.c` | bmap() 이 hint 를 정해 balloc 에 넘김 (세 곳)                 |

---

## 커밋하고 올리기

실습 하나를 끝낼 때마다 커밋하면, 실습 2 에서 문제가 생겨도 실습 1 상태로 돌아가 비교할 수 있습니다.

```
git status                         # 무엇이 바뀌었나 (확인용)

# 실습 0 (5.0)
git add kernel/fsinfo.h kernel/syscall.h kernel/syscall.c kernel/sysfile.c
git add user/user.h user/usys.pl user/fsdump.c Makefile
git commit -m "lab5: fsdump"

# 실습 1 (5.1)
git add kernel/syscall.h kernel/syscall.c kernel/sysfile.c
git add user/user.h user/usys.pl user/fragtest.c Makefile
git commit -m "lab5: get_fragmentation, fragtest"

# 실습 2 (5.2)
git add kernel/fs.c
git commit -m "lab5: locality-aware balloc"

git push -u origin lab5-filesystem # 내 GitHub 저장소에 올리기
git log --oneline                  # 커밋 세 개가 남았는지 (선택)
```

> ℹ️ **실습 1 상태로 돌아가 비교하려면** `git stash` 로 실습 2 의 수정을 잠시 치우거나, `git checkout HEAD~1 -- kernel/fs.c` 로 실습 1 커밋의 fs.c 를 꺼냅니다. 어느 쪽이든 `make clean` 뒤에 실행하고, 비교가 끝나면 `git stash pop` 또는 `git checkout HEAD -- kernel/fs.c` 로 실습 2 의 코드를 되돌려 놓습니다.

> ⚠️ **Permission denied 가 나온다면** 수업 저장소를 fork 하지 않고 그대로 clone 한 경우입니다. utils.md 를 참고해 내 저장소를 연결하세요.

제출물은 `lab5-filesystem` 브랜치 또는 `git diff riscv...` 결과입니다. 확인 질문에 대한 답도 함께 내세요.

---

## 부록 A — 자주 막히는 곳

| 증상                                                           | 원인과 해결                                                                                                                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 숫자가 문서와 다르다 (a.txt 가 실습 0 에서 25, 실습 1 부터 26 이 아님)            | `make clean` 없이 이전 `fs.img` 를 썼거나, UPROGS 에 다른 프로그램(예: `user/stat.c`)을 더한 브랜치입니다.                                                                      |
| Ctrl-C 로 qemu 가 안 꺼진다                                        | Ctrl-A 를 누르고 손을 뗀 다음 X 입니다.                                                                                                                            |
| `implicit declaration of function 'bread'` · `struct buf` 오류 | sysfile.c 위쪽에 `#include "buf.h"` 를 넣지 않았습니다.                                                                                                           |
| `unknown type name 'struct fsinfo'` · `NINDIRECT` 오류         | `#include "fsinfo.h"` 를 빠뜨렸거나, user 프로그램에서 `kernel/fs.h` 보다 먼저 include 했습니다.                                                                           |
| undefined reference to `fsdump` / `get_fragmentation`        | user/user.h 또는 user/usys.pl 을 빠뜨렸습니다.                                                                                                                  |
| `unknown sys call 23` (또는 24)                                | kernel/syscall.c 의 syscalls[] 배열에 등록하지 않았습니다.                                                                                                          |
| `exec fsdump failed` · `exec fragtest failed`                | Makefile 의 UPROGS 에 넣지 않았습니다.                                                                                                                          |
| fsdump 의 `blocks:` 가 비어 있거나 이상하다                             | 제공 코드 `fill_fsinfo` 를 옮겨 적다 빠뜨린 줄이 있습니다. 문서의 코드를 다시 복사하세요.                                                                                             |
| `panic: bget: no buffers`                                    | `bread` 한 블록을 `brelse` 하지 않았습니다. 실습 2 에서 고친 코드와 제공 코드의 `brelse` 짝을 확인하세요.                                                                              |
| fragments 가 언제나 0 이다                                         | 실습 1 TODO ① 이 비어 있습니다.                                                                                                                                 |
| 실습 1 에서 fragments 가 8 이 아니다 (0 또는 7 등)                       | 조건을 거꾸로 썼거나(`== 앞 + 1` 일 때 세기), `i == 0` 을 세지 않았습니다. 첫 블록은 언제나 새 조각입니다.                                                                                |
| `'freegroup' defined but not used`                           | 실습 2 TODO ② 를 채우지 않았습니다.                                                                                                                               |
| `too few arguments to function 'balloc'`                     | bmap 의 balloc 호출 세 곳 중 하나를 그대로 두었습니다. 5.2 Step 2 의 bmap 전체로 바꾸세요.                                                                                      |
| `balloc: out of blocks` 가 나오고 write 가 실패한다                   | 실습 2 TODO ① 에서 b 가 늘 같은 값입니다(예: 0). `(hint + n)` 을 `sb.size` 로 나눈 나머지여야 합니다.                                                                           |
| 실습 2 뒤에도 fragments 가 8 이다                                    | `make clean` 없이 예전 fs.img 를 썼거나, TODO ② 에서 첫 블록 hint 를 0 으로 두었습니다. 첫 블록이 0 부터 찾으면 f2 의 첫 블록이 f1 바로 뒤를 차지해 다시 번갈아 놓입니다. 첫 블록은 `freegroup(ip->dev)` 입니다. |
