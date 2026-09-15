# Lab 2 — 내 시스템 호출 만들기

**freepages() · sysinfo()**

*고칠 파일은 여섯 개, 진짜 코드는 함수 하나  ·  약 2시간*

> 환경 설정은 [lab0-setup.md](lab0-setup.md), git · gdb 사용법은 [utils.md](utils.md) 를 참고하세요.

## 목차

- [개요](#개요)
  - [시스템 호출 추가 시 수정할 파일](#시스템-호출-추가-시-수정할-파일)
  - [시스템 호출 번호 배정](#시스템-호출-번호-배정)
  - [실습 브랜치 만들기](#실습-브랜치-만들기)
  - [문서 읽는 법](#문서-읽는-법)
- [2.1 freepages() — 남은 페이지 세기](#21-freepages-남은-페이지-세기)
  - [2.1.1 목표](#211-목표)
  - [2.1.2 학습 내용](#212-학습-내용)
  - [2.1.3 단계별 구현](#213-단계별-구현)
  - [2.1.4 확인 질문](#214-확인-질문)
  - [2.1.5 변경 요약](#215-변경-요약)
- [2.2 sysinfo() — 구조체를 사용자에게 돌려주기](#22-sysinfo-구조체를-사용자에게-돌려주기)
  - [2.2.1 목표](#221-목표)
  - [2.2.2 학습 내용](#222-학습-내용)
  - [2.2.3 단계별 구현](#223-단계별-구현)
  - [2.2.4 확인 질문](#224-확인-질문)
  - [2.2.5 변경 요약](#225-변경-요약)
- [2.3 커밋하고 올리기](#23-커밋하고-올리기)
- [종합 — 두 실습 비교](#종합-두-실습-비교)
- [부록 A — TODO 정답](#부록-a-todo-정답)
  - [2.1 freepages](#21-freepages)
  - [2.2 sysinfo](#22-sysinfo)
- [부록 B — 자주 막히는 곳](#부록-b-자주-막히는-곳)

## 개요

지난 실습에서 `write` · `fork` · `open` 을 직접 불러 봤습니다. 이번에는 그 목록에 내 것을 하나 더합니다.

커널이 관리하는 정보를 사용자 프로그램에서 볼 수 있게 만드는 것이 목표입니다. 남은 페이지가 몇 개인지, 지금 프로세스가 몇 개나 살아 있는지 — 커널만 아는 것을 밖으로 꺼내 옵니다.

| 단계  | 무엇을           | 난이도 | 시간  |
| --- | ------------- | --- | --- |
| 2.1 | `freepages()` | 초급  | 50분 |
| 2.2 | `sysinfo()`   | 중급  | 50분 |
| 2.3 | 커밋하고 올리기      | —   | 10분 |

### 시스템 호출 추가 시 수정할 파일

한 군데라도 빠뜨리면 링크 또는 실행에서 걸립니다. 이 표를 옆에 두고 진행하세요.

| #   | 파일                 | 무엇을 넣나                 |
| --- | ------------------ | ---------------------- |
| ①   | `kernel/syscall.h` | 번호를 배정한다               |
| ②   | `kernel/syscall.c` | extern 선언과 배열 등록       |
| ③   | `kernel/sysproc.c` | 커널 쪽 진입 함수 — 진짜 코드는 여기 |
| ④   | `user/user.h`      | 사용자 쪽 프로토타입            |
| ⑤   | `user/usys.pl`     | 스텁을 찍어 내도록 entry 추가    |
| ⑥   | `Makefile`         | 테스트 프로그램을 UPROGS 에 등록  |

> 

> ⚠️ **증상으로 원인 찾기** undefined reference → ④ 나 ⑤ 를 빠뜨렸습니다. unknown sys call 23 → ② 의 배열 등록이 빠졌습니다. exec … failed → ⑥ 의 UPROGS 입니다. 세 가지가 각각 링크 · 커널 메시지 · 셸에서 나타나므로 어디서 걸렸는지로 구분할 수 있습니다.

### 시스템 호출 번호 배정

현재 코드의 마지막 번호는 `SYS_sync 22` 입니다. 이번 실습에서는 **23** 과 **24** 를 씁니다.

```
// kernel/syscall.h
#define SYS_freepages 23
#define SYS_sysinfo 24
```

> ℹ️ **번호를 건너뛰거나 중복하지 마세요** syscalls[] 는 번호를 배열 인덱스로 쓰는 표입니다. 번호가 어긋나면 엉뚱한 함수가 불리거나 널 포인터를 부릅니다.

### 실습 브랜치 만들기

```
git checkout riscv               # 손대지 않은 원본으로 이동
git checkout -b lab2-syscall     # 브랜치를 만들고 그리로 이동
git branch                       # * lab2-syscall 로 바뀌었는지 확인
```

### 문서 읽는 법

- 각 단계의 제목에 **고칠 파일 경로** 가 붙어 있습니다.
- `// TODO ①` 로 표시된 자리는 직접 채워 넣으세요. 그 줄만 비어 있고 나머지는 다 나와 있습니다.
- 막히면 부록 B 의 「자주 막히는 곳」 을 먼저 펴 보세요. 정답은 부록 A 에 있지만 최소 5분은 스스로 생각해 보세요.

---

## 2.1 freepages() — 남은 페이지 세기

### 2.1.1 목표

커널의 free list 에 남아 있는 페이지가 몇 개인지 세어 돌려주는 시스템 호출을 만듭니다. 사용자 프로그램에서 `freepages()` 를 부르면 지금 남은 페이지 수가 숫자로 나옵니다.

### 2.1.2 학습 내용

- 시스템 호출을 추가할 때 고치는 여섯 파일의 역할
- 커널 안에서만 보이는 자료(`kmem.freelist`)를 밖으로 꺼내는 방법
- 커널 자료를 훑을 때 락으로 감싸는 관용 (자세한 이유는 「동기화」 에서)

### 2.1.3 단계별 구현

#### Step 1 — 세는 함수를 만든다  `(kernel/kalloc.c)`

`kmem` 은 `kalloc.c` 안에서 **이름 없는 구조체**로 선언돼 있습니다 (21–24행). 타입에 이름이 없으니 다른 파일에서는 `extern` 선언조차 쓸 수 없고, 리스트의 마디인 `struct run` 도 이 파일에만 있습니다. 그래서 세는 일도 이 파일 안에서 해야 합니다. 파일 맨 아래에 함수를 추가하세요.

```
// kernel/kalloc.c  — add at the bottom of the file

// Return the number of free pages.
uint64
freepages(void)
{
  struct run *r;
  uint64 n = 0;

  acquire(&kmem.lock);

  r = kmem.freelist;
  while (r) {
    // TODO ①  count this page, then move to the next one
  }

  release(&kmem.lock);

  return n;
}
```

> ℹ️ **`acquire` · `release` 두 줄은 채워 두었습니다** 세는 동안 다른 코어가 `kalloc` 이나 `kfree` 로 이 리스트를 고치면 안 되기 때문입니다. 지우면 컴파일도 되고 대부분 잘 도는 것처럼 보입니다 — 그래서 더 위험합니다. 이 두 줄이 정확히 무엇을 막아 주는지는 「동기화」 에서 답이 납니다. 지금은 *커널 자료를 훑을 때는 락으로 감싼다* 는 관용만 눈에 익혀 두세요.

#### Step 2 — 다른 파일에서 부를 수 있게 선언한다  `(kernel/defs.h)`

`kalloc.c` 묶음에 한 줄을 더합니다.

```
// kernel/defs.h  — kalloc.c 묶음
void*           kalloc(void);
void            kfree(void *);
void            kinit(void);
uint64          freepages(void);        // 추가
```

#### Step 3 — 번호를 배정한다  `(kernel/syscall.h)`

```
#define SYS_sync   22
#define SYS_freepages 23        // 추가
```

#### Step 4 — 선언하고 표에 등록한다  `(kernel/syscall.c)`

```
// 파일 위쪽, 다른 extern 들 옆에
extern uint64 sys_freepages(void);

// syscalls[] 배열 안에
static uint64 (*syscalls[])(void) = {
  ...
  [SYS_sync]    = sys_sync,
  [SYS_freepages] = sys_freepages,        // 추가
};
```

#### Step 5 — 커널 쪽 진입 함수를 만든다  `(kernel/sysproc.c)`

인자가 없으므로 할 일이 거의 없습니다. Step 1 에서 만든 함수를 부르기만 하면 됩니다.

```
// kernel/sysproc.c  — add at the bottom

uint64
sys_freepages(void)
{
  // TODO ②  call the function you wrote in kalloc.c
}
```

#### Step 6 — 사용자 쪽에 노출한다  `(user/user.h, user/usys.pl)`

```
// user/user.h  — system calls 묶음 안에
uint64 freepages(void);

// user/usys.pl  — entry 목록 끝에
entry("freepages");
```

> ℹ️ **usys.S 는 어디에?** 저장소에 없습니다. 빌드할 때 usys.pl 이 만들어 냅니다. make 한 뒤 user/usys.S 를 열어 보면 여러분이 추가한 세 줄이 들어 있습니다.

#### Step 7 — 테스트 프로그램을 만든다  `(user/pagetest.c)`

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("free pages: %lu\n", freepages());
  exit(0);
}
```

#### Step 8 — 빌드 목록에 넣고 실행한다  `(Makefile)`

```
# Makefile 의 UPROGS 에 추가
    $U/_pagetest\
```

```
$ make qemu
$ pagetest
free pages: 32548
```

0 이 아닌 큰 수가 찍히면 성공입니다. **커널 안에만 있던 값이 여섯 파일을 거쳐 여기까지 나왔다는 뜻입니다** — 이번 실습이 증명하려던 것이 그것입니다.

> ℹ️ **값이 매번 똑같지 않아도 됩니다** 그때그때 살아 있는 프로그램이 다르기 때문입니다. 이 값이 박제된 상수가 아니라 **지금의 커널 상태** 라는 것은 2.2 에서 `fork` 로 직접 확인합니다.

### 2.1.4 확인 질문

- `make` 를 돌린 뒤 `user/usys.S` 를 열어 보세요. 방금 추가한 `freepages` 스텁 세 줄이 보입니다. `a7` 에 실리는 번호는 어디서 온 값입니까?
- 리스트를 처음부터 끝까지 훑습니다. 페이지가 3만 개라면 몇 번 반복합니까? 이 시스템 호출이 비싼 이유는 무엇입니까?
- `kernel/syscall.h` 의 번호를 23 이 아니라 이미 쓰이고 있는 12 로 바꾸면 무슨 일이 벌어질까요? 실제로 해 보기 전에 먼저 예상해 보세요.

### 2.1.5 변경 요약

| #   | 파일                 | 무엇을 했나             |
| --- | ------------------ | ------------------ |
| 1   | `kernel/kalloc.c`  | freepages() 추가     |
| 2   | `kernel/defs.h`    | 선언 한 줄             |
| 3   | `kernel/syscall.h` | SYS_freepages 23   |
| 4   | `kernel/syscall.c` | extern + 배열 등록     |
| 5   | `kernel/sysproc.c` | sys_freepages() 추가 |
| 6   | `user/user.h`      | 프로토타입              |
| 7   | `user/usys.pl`     | entry("freepages") |
| 8   | `user/pagetest.c`  | 새 파일               |
| 9   | `Makefile`         | UPROGS 에 등록        |

---

## 2.2 sysinfo() — 구조체를 사용자에게 돌려주기

### 2.2.1 목표

이번에는 **값 하나가 아니라 구조체** 를 돌려줍니다. 남은 페이지 수와 살아 있는 프로세스 수를 한 번에 알려주는 `sysinfo()` 를 만듭니다.

### 2.2.2 학습 내용

- `argaddr` 로 사용자가 준 주소를 받는 방법
- `copyout` 으로 커널의 값을 사용자 메모리에 안전하게 써 넣는 방법
- proc[] 배열을 훑어 상태를 세는 방법

> ℹ️ **왜 그냥 대입하면 안 되나** 사용자가 준 주소는 **그 프로세스의 주소 공간에서만 뜻이 있는 숫자** 입니다. 커널이 그대로 역참조하면 엉뚱한 곳을 가리킵니다. 그래서 `copyout` 을 거칩니다. 지금은 이 관용만 익히면 되고, 왜 그런지는 「메모리 관리」 에서 답이 납니다.

### 2.2.3 단계별 구현

#### Step 1 — 구조체를 정의한다  `(kernel/sysinfo.h — 새 파일)`

```
// kernel/sysinfo.h
struct sysinfo {
  uint64 freepages;   // 남은 페이지 수
  uint64 nproc;       // UNUSED 가 아닌 프로세스 수
};
```

#### Step 2 — 살아 있는 프로세스를 센다  `(kernel/proc.c)`

`proc[]` 배열을 훑어 `UNUSED` 가 아닌 칸을 셉니다. `acquire` · `release` 는 2.1 때처럼 채워 두었습니다 — 커널 자료를 훑을 때의 관용이고, 자세한 이유는 「동기화」 에서 답이 납니다.

```
// kernel/proc.c  — add at the bottom

// Count processes whose state is not UNUSED.
uint64
nproc(void)
{
  struct proc *p;
  uint64 n = 0;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    // TODO ①  count this one if its state is not UNUSED
    release(&p->lock);
  }
  return n;
}
```

`kernel/defs.h` 의 `proc.c` 묶음에 선언도 추가하세요.

```
uint64          nproc(void);
```

#### Step 3 — 번호를 배정하고 표에 등록한다  `(kernel/syscall.h, kernel/syscall.c)`

```
// kernel/syscall.h  — SYS_freepages 아래에
#define SYS_sysinfo 24        // 추가
```

```
// kernel/syscall.c  — 파일 위쪽, 다른 extern 들 옆에
extern uint64 sys_sysinfo(void);

// syscalls[] 배열 안에
  [SYS_freepages] = sys_freepages,
  [SYS_sysinfo] = sys_sysinfo,        // 추가
```

#### Step 4 — 커널 쪽 함수를 구현한다  `(kernel/sysproc.c)`

이번에는 인자가 있습니다. 사용자가 준 주소를 받아, 그 주소로 구조체를 복사해 보냅니다.

```
// kernel/sysproc.c  — 기존 #include 들 맨 아래에 한 줄 추가
#include "sysinfo.h"

uint64
sys_sysinfo(void)
{
  struct sysinfo info;
  uint64 addr;
  struct proc *p = myproc();

  argaddr(0, &addr);        // 첫 번째 인자를 주소로 받는다

  info.freepages = freepages();
  info.nproc     = nproc();

  // TODO ②  copy info out to the user address
  //        copyout(p->pagetable, p->sz, addr, ...) 를 씁니다

  return 0;
}
```

> ℹ️ **`argaddr` 은 채워 두었습니다** `argaddr(0, &addr)` 은 *0번째 인자를 주소로 읽어 `addr` 에 넣어라* 입니다. 반환값이 없고 주소가 올바른지 검사하지도 않습니다 — 검사는 뒤의 `copyout` 이 합니다. 정수를 받을 때는 `argint`, 문자열은 `argstr` 입니다 (`kernel/syscall.c` 57–81).

> ⚠️ **include 순서** `sysinfo.h` 는 `uint64` 를 쓰므로 반드시 `types.h` **뒤** 에 와야 합니다. 파일 첫 줄에 넣으면 `unknown type name 'uint64'` 로 깨집니다. 기존 `#include` 묶음의 맨 아래에 붙이세요.

> ⚠️ **copyout 의 인자** copyout(pagetable, sz, dstva, src, len) 입니다. 어느 주소 공간으로 보낼지(pagetable), 어디에(dstva), 무엇을(src), 몇 바이트(len). 실패하면 음수를 돌려주므로 검사해서 -1 을 반환하세요.

#### Step 5 — 사용자 쪽에 노출한다  `(user/user.h, user/usys.pl)`

```
// user/user.h  — 3행의 struct stat; 옆에
struct stat;
struct sysinfo;                       // 추가 — 전방 선언

// system calls 묶음 안에
int sysinfo(struct sysinfo *);

// user/usys.pl
entry("sysinfo");
```

#### Step 6 — 테스트 프로그램을 만든다  `(user/sysinfotest.c)`

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    printf("sysinfo failed\n");
    exit(1);
  }
  printf("freepages = %lu pages\n", info.freepages);
  printf("nproc     = %lu\n", info.nproc);

  // TODO ③  fork a child, call sysinfo again, and see nproc change

  exit(0);
}
```

#### Step 7 — 빌드하고 확인한다

```
# Makefile 의 UPROGS 에
    $U/_sysinfotest\
```

```
$ sysinfotest
freepages = 32533 pages
nproc     = 3
```

xv6 를 갓 띄운 상태라면 `init` · `sh` · 그리고 방금 실행한 `sysinfotest` 셋이 살아 있습니다.

### 2.2.4 확인 질문

- fork 로 자식을 만든 뒤 `nproc` 를 다시 부르면 값이 얼마나 늘어납니까? `wait` 을 부르기 전과 후가 다릅니까?
- `copyout` 대신 `*(struct sysinfo *)addr = info;` 라고 쓰면 어떻게 됩니까? 실제로 해 보고 무슨 일이 벌어지는지 적어 오세요.
- 2.1 의 `freepages()` 는 인자가 없었고 `sysinfo()` 는 하나 있습니다. 그런데 **고칠 여섯 자리는 똑같았습니다.** 무엇이 달라졌고 무엇이 그대로였습니까?
- `sysinfo()` 는 결과를 반환값이 아니라 `copyout` 으로 내보냅니다. 왜 구조체를 그냥 반환하지 않습니까? (힌트 — 반환값이 실려 오는 곳은 `a0` 레지스터 하나입니다)

### 2.2.5 변경 요약

| #   | 파일                   | 무엇을 했나           |
| --- | -------------------- | ---------------- |
| 1   | `kernel/sysinfo.h`   | 새 파일 — 구조체 정의    |
| 2   | `kernel/proc.c`      | nproc() 추가       |
| 3   | `kernel/defs.h`      | 선언 한 줄           |
| 4   | `kernel/syscall.h`   | SYS_sysinfo 24   |
| 5   | `kernel/syscall.c`   | extern + 배열 등록   |
| 6   | `kernel/sysproc.c`   | sys_sysinfo() 추가 |
| 7   | `user/user.h`        | 프로토타입            |
| 8   | `user/usys.pl`       | entry("sysinfo") |
| 9   | `user/sysinfotest.c` | 새 파일             |
| 10  | `Makefile`           | UPROGS 에 등록      |

---

## 2.3 커밋하고 올리기

```
git status                       # 무엇이 바뀌었나 (확인용)
git diff riscv...                # 분기 지점 이후 내가 고친 것 전부 (확인용)

# 커밋할 파일을 지정한다
git add kernel/kalloc.c kernel/proc.c kernel/defs.h
git add kernel/syscall.h kernel/syscall.c kernel/sysproc.c kernel/sysinfo.h
git add user/user.h user/usys.pl Makefile
git add user/pagetest.c user/sysinfotest.c

git status                       # 빠진 파일이 없는지 확인 (확인용)

git commit -m "lab2: freepages, sysinfo 시스템 호출 추가"
git push -u origin lab2-syscall  # 내 GitHub 저장소에 올리기

git log --oneline                # 커밋이 잘 남았는지 (선택)
```

> ℹ️ **목록이 길다면** git add -A 로 한꺼번에 담을 수도 있습니다. (선택) 다만 색인 파일(tags · cscope.*)이 섞이지 않았는지 git status 로 확인하세요.

> ⚠️ **Permission denied 가 나온다면** 수업 저장소를 fork 하지 않고 그대로 clone 한 경우입니다. utils.md 를 참고해 내 저장소를 연결하세요.

제출물은 `lab2-syscall` 브랜치 또는 `git diff riscv...` 결과입니다. 확인 질문에 대한 답도 함께 내세요.

---

## 종합 — 두 실습 비교

|            | freepages()     | sysinfo()                |
| ---------- | --------------- | ------------------------ |
| 인자         | 없음              | 사용자 주소 하나                |
| 반환         | 값 하나 (uint64)   | 구조체 — copyout 으로 보낸다     |
| 새로 쓰는 함수   | 없음              | `argaddr · copyout`      |
| 건드리는 커널 자료 | `kmem.freelist` | `kmem.freelist · proc[]` |
| 핵심         | 여섯 파일의 절차 익히기   | 커널과 사용자 사이로 데이터를 옮기기     |

두 번째가 어려운 이유는 **경계를 넘어 데이터를 옮기는 일** 이 들어가기 때문입니다. 커널은 사용자가 준 주소를 그대로 믿지 않고 `copyout` 을 거쳐 씁니다. 왜 그래야 하는지는 「메모리 관리」 에서 답이 납니다.

---

## ## 부록 A — 자주 막히는 곳

| 증상                                         | 원인과 해결                                                                                                    |
| ------------------------------------------ | --------------------------------------------------------------------------------------------------------- |
| undefined reference to `freepages'         | user/user.h 또는 user/usys.pl 을 빠뜨렸습니다. 둘 다 확인하세요.                                                          |
| unknown sys call 23                        | kernel/syscall.c 의 syscalls[] 배열에 등록하지 않았습니다. extern 선언도 함께 확인하세요.                                        |
| exec pagetest failed                       | Makefile 의 UPROGS 에 등록하지 않았습니다.                                                                           |
| kernel/kalloc.c: 'kmem' undeclared         | 함수를 파일 맨 아래가 아니라 kmem 선언보다 위에 넣었습니다.                                                                      |
| panic: acquire                             | 이미 잡고 있는 락을 또 잡았습니다. release 를 빠뜨렸는지 보세요.                                                                 |
| freepages 값이 0 이다                          | 리스트를 훑는 while 안에서 r 을 갱신하지 않아 무한 루프에 빠졌거나, n 을 증가시키지 않았습니다.                                               |
| sysinfo 가 항상 -1                            | copyout 이 실패하고 있습니다. argaddr 로 받은 주소가 맞는지, sizeof(info) 를 넘겼는지 확인하세요.                                     |
| `scause=0xf ...` 가 찍히고 `panic: kerneltrap` | copyout 대신 사용자 주소를 직접 역참조했습니다. 커널이 그 주소를 못 찾아 폴트가 났고, 죽는 것은 사용자 프로그램이 아니라 **커널** 입니다. 확인 질문 2번이 이 이야기입니다. |
| unused variable 'p'                        | `sys_sysinfo` 의 TODO ② 를 채우기 전에 빌드했습니다. `p` 는 `copyout` 에서만 쓰입니다. `-Werror` 라 경고도 오류가 됩니다.                |
| Makefile 수정 후 빌드가 깨진다                      | 줄 끝의 역슬래시(\)를 확인하세요. 앞뒤 줄과 모양이 같아야 합니다.                                                                   |
| Ctrl-C 로 qemu 가 안 꺼진다                      | Ctrl-A 를 누르고 손을 뗀 다음 X 입니다.                                                                               |
