# xv6 메모리 관리 실습

**물리 페이지에서 주소 변환까지 — 네 단계**

*Lab 1 초급 · Lab 2 중급 · Lab 3 고급 · Lab 4 관찰*

> 환경 설정은 [lab0-setup.md](lab0-setup.md), git · gdb 사용법은 [utils.md](utils.md) 를 참고하세요.

## 목차

- [개요](#개요)
  - [시스템콜 추가 시 수정할 파일](#시스템콜-추가-시-수정할-파일)
  - [시스템콜 번호 배정](#시스템콜-번호-배정)
  - [문서 읽는 법](#문서-읽는-법)
  - [실습 브랜치 만들기](#실습-브랜치-만들기)
- [Lab 1 (초급) — freemem()](#lab-1-초급-freemem)
  - [1.1 목표](#11-목표)
  - [1.2 학습 내용](#12-학습-내용)
  - [1.3 단계별 구현](#13-단계별-구현)
  - [1.4 확인 질문](#14-확인-질문)
  - [1.5 변경 요약](#15-변경-요약)
- [Lab 2 (중급) — vmprint()](#lab-2-중급-vmprint)
  - [2.1 목표](#21-목표)
  - [2.2 학습 내용](#22-학습-내용)
  - [2.3 단계별 구현](#23-단계별-구현)
  - [2.4 출력 읽는 법](#24-출력-읽는-법)
  - [2.5 확인 질문](#25-확인-질문)
  - [2.6 변경 요약](#26-변경-요약)
- [Lab 3 (고급) — va2pa()](#lab-3-고급-va2pa)
  - [3.1 목표](#31-목표)
  - [3.2 학습 내용](#32-학습-내용)
  - [3.3 단계별 구현](#33-단계별-구현)
  - [3.4 검증 — 수동 계산과 대조](#34-검증-수동-계산과-대조)
  - [3.5 확인 질문](#35-확인-질문)
  - [3.6 변경 요약](#36-변경-요약)
- [Lab 4 (관찰) — 가드 페이지 확인](#lab-4-관찰-가드-페이지-확인)
  - [4.1 목표](#41-목표)
  - [4.2 실험 1 — 주소 공간의 구멍 찾기](#42-실험-1-주소-공간의-구멍-찾기)
  - [4.3 실험 2 — 스택 오버플로 유발](#43-실험-2-스택-오버플로-유발)
  - [4.4 두 실험 결과 대조](#44-두-실험-결과-대조)
  - [4.5 확인 질문](#45-확인-질문)
  - [4.6 변경 요약](#46-변경-요약)
- [종합 — 네 랩이 무엇을 이었나](#종합-네-랩이-무엇을-이었나)
  - [개념 카드 ① — PTE 는 주소와 권한을 함께 담는다](#개념-카드-①-pte-는-주소와-권한을-함께-담는다)
  - [개념 카드 ② — 커널이 사용자 주소를 다루는 세 가지 방법](#개념-카드-②-커널이-사용자-주소를-다루는-세-가지-방법)
- [제출](#제출)
- [부록 A — TODO 정답](#부록-a-todo-정답)
  - [Lab 1](#lab-1)
  - [Lab 2](#lab-2)
  - [Lab 3](#lab-3)
- [부록 B — 자주 막히는 곳](#부록-b-자주-막히는-곳)

## 개요

이 실습은 강의에서 그림으로 본 메모리 관리를 코드로 확인하는 것이 목적입니다. 네 개의 랩이 물리 메모리에서 시작해 주소 변환까지 한 줄기로 이어집니다.

- **Lab 1** — 물리 메모리에 빈 페이지가 몇 장 남았는지 세어 봅니다.
- **Lab 2** — 페이지 테이블을 통째로 출력해 3단계 구조를 눈으로 확인합니다.
- **Lab 3** — 가상 주소를 물리 주소로 직접 번역하는 시스템콜을 만듭니다.
- **Lab 4** — 앞의 셋을 이용해 가드 페이지의 존재를 스스로 증명합니다.

> ⚠️ **브랜치** mit-pdos/xv6-riscv 의 riscv 브랜치에서 memory 브랜치를 따서 진행합니다. 주제마다 브랜치를 따로 쓰므로 시스템콜 번호는 항상 현재 코드 기준으로 셉니다.

> ℹ️ **빌드** xv6 루트 디렉터리에서 make qemu 로 컴파일과 실행을 함께 합니다. 종료는 Ctrl-A 를 누른 뒤 X 입니다. 각 단계마다 반드시 빌드가 통과하는지 확인하고 다음으로 넘어가세요.

### 시스템콜 추가 시 수정할 파일

| 파일 | 할 일 |
|---|---|
| `kernel/syscall.h` | #define SYS_xxx 로 번호를 배정한다 |
| `kernel/syscall.c` | extern 선언을 추가하고 syscalls[] 배열에 등록한다 |
| `kernel/sysproc.c` | 실제 커널 함수 sys_xxx() 를 구현한다 |
| `user/user.h` | 사용자 프로그램이 부를 수 있게 프로토타입을 넣는다 |
| `user/usys.pl` | entry("xxx") 를 추가한다 — usys.S 는 빌드할 때 자동 생성된다 |
| `Makefile` | UPROGS 목록에 테스트 프로그램을 등록한다 |

> ⚠️ **누락 시 증상** 링크 단계에서 undefined reference 가 납니다. 컴파일은 통과하는데 링크에서 실패한다면 usys.pl 이나 user.h 를 먼저 의심하세요.

### 시스템콜 번호 배정

`kernel/syscall.h` 의 마지막 항목은 `#define SYS_sync 22` 입니다. 따라서 새로 추가할 번호는 23부터입니다.

| 랩 | 선행 | 새 시스템콜 | 번호 |
|---|---|---|---|
| Lab 1 — freemem() | 메모리 관리 ① 강의 | `freemem()` | 23 |
| Lab 2 — vmprint() | 메모리 관리 ② 강의 | 없음 (커널 내부 함수) | — |
| Lab 3 — va2pa() | Lab 2 | `va2pa()` | 24 |
| Lab 4 — 가드 페이지 | Lab 2 · Lab 3 | 없음 (관찰 실습) | — |

### 문서 읽는 법

코드 블록 안의 `// TODO ①` 표시는 직접 채워 넣을 자리입니다. 그 줄만 비워 두고 나머지는 그대로 옮겨 적으면 됩니다. 막히면 문서 끝의 **부록 A** 에 정답이 있습니다. 먼저 5분은 스스로 생각해 보세요.

### 실습 브랜치 만들기

커널 코드를 고치기 전에 작업할 브랜치를 만듭니다. `riscv` 브랜치는 손대지 않고 그대로 둡니다.

```
git checkout riscv               # 손대지 않은 원본으로 이동
git checkout -b lab2-memory      # 브랜치를 만들고 그리로 이동
git branch                       # * lab2-memory 로 바뀌었는지 확인
```

> ℹ️ **앞 실습의 코드는 그대로 둡니다** lab1-userprog 브랜치에 만든 프로그램들은 그 브랜치에 남아 있습니다. 이번 브랜치는 riscv 에서 새로 갈라져 나오므로 깨끗한 상태에서 시작합니다.

---

## Lab 1 (초급) — freemem()

*남은 물리 메모리가 몇 바이트인지 세어 돌려주는 시스템콜*

### 1.1 목표

kalloc 의 free list 를 처음부터 끝까지 따라가며 빈 페이지 개수를 세고, 그것을 바이트 단위로 돌려주는 시스템콜 `freemem()` 를 만듭니다.

강의에서 본 free list 그림이 실제로 연결 리스트라는 것, 그리고 그 리스트를 여럿이 함께 쓰기 때문에 락이 필요하다는 것을 코드로 확인합니다.

### 1.2 학습 내용

- 연결 리스트로 만들어진 free list 를 순회하는 방법
- 커널 자료구조를 읽을 때 락을 어디서 잡고 어디서 놓아야 하는지
- 시스템콜 하나가 여섯 개 파일을 거쳐 사용자 프로그램에 도달하는 전체 경로
- kalloc.c 안의 static 자료를 다른 파일에서 쓰려면 무엇이 필요한지

### 1.3 단계별 구현

#### Step 1 — 개수를 세는 함수 추가  `(kernel/kalloc.c)`

`kernel/kalloc.c` 의 맨 아래에 함수를 하나 추가합니다. `kmem` 은 이 파일 안에서만 보이므로, 세는 일도 이 파일 안에서 해야 합니다.

```
// kernel/kalloc.c  — add at the bottom of the file

// Return the amount of free memory, in bytes.
uint64
freemem(void)
{
  struct run *r;
  uint64 n = 0;

  // TODO ①  acquire the lock that protects kmem.freelist

  r = kmem.freelist;
  while (r) {
    // TODO ②  count this page, then move to the next one
  }

  // TODO ①  don't forget to release it

  return n * PGSIZE;
}
```

> ℹ️ **생각해 볼 점** TODO ① 을 아예 빼면 어떻게 될까요? 컴파일도 되고 대부분 잘 동작합니다. 그런데 여러 코어가 동시에 kalloc 을 부르는 중이라면 무슨 일이 벌어질까요?

#### Step 2 — 함수 선언 추가  `(kernel/defs.h)`

`kernel/defs.h` 의 kalloc.c 섹션에 한 줄을 추가합니다.

```
// kernel/defs.h  — in the kalloc.c section
void*           kalloc(void);
void            kfree(void *);
uint64          freemem(void);        // ADD THIS LINE
```

#### Step 3 — 시스템콜 번호 배정  `(kernel/syscall.h)`

```
// kernel/syscall.h  — add at the bottom
#define SYS_sync    22
#define SYS_freemem 23        // ADD THIS LINE
```

#### Step 4 — 핸들러 등록  `(kernel/syscall.c)`

두 군데를 고칩니다. 먼저 파일 위쪽의 extern 선언들 사이에 한 줄을 넣습니다.

```
// kernel/syscall.c  — with the other extern declarations
extern uint64 sys_freemem(void);
```

그리고 `syscalls[]` 배열에 항목을 추가합니다.

```
static uint64 (*syscalls[])(void) = {
  [SYS_fork]    sys_fork,
  [SYS_exit]    sys_exit,
  // ... existing entries ...
  [SYS_sync]    sys_sync,
  [SYS_freemem] sys_freemem,     // ADD THIS LINE
};
```

#### Step 5 — 커널 진입 함수 구현  `(kernel/sysproc.c)`

시스템콜 진입점은 인자를 받아 실제 일꾼 함수를 부르는 얇은 껍데기입니다. 이번에는 인자가 없어 더 간단합니다.

```
// kernel/sysproc.c  — add at the bottom
uint64
sys_freemem(void)
{
  return freemem();
}
```

#### Step 6 — 사용자 쪽 노출  `(user/user.h, user/usys.pl)`

```
// user/user.h  — add with the other system call prototypes
uint64 freemem(void);
```

```
# user/usys.pl  — add with the other entry() lines
entry("freemem");
```

> ℹ️ **usys.S 의 위치** user/usys.S 는 소스에 없습니다. 빌드할 때 usys.pl 이 만들어 냅니다. 그래서 usys.pl 만 고치면 됩니다.

#### Step 7 — 테스트 프로그램 작성  `(user/freetest.c)`

```
// user/freetest.c  — new file
#include "kernel/types.h"
#include "user/user.h"

#define PAGE 4096

int
main(void)
{
  uint64 before = freemem();
  printf("before : %ld bytes = %ld pages\n", before, before / PAGE);

  char *p = sbrk(10 * PAGE);
  if (p == (char *)-1) {
    printf("sbrk failed\n");
    exit(1);
  }

  // TODO ③  touch one byte in each of the 10 pages

  uint64 after = freemem();
  printf("after  : %ld bytes = %ld pages\n", after, after / PAGE);
  printf("used   : %ld pages\n", (before - after) / PAGE);
  exit(0);
}
```

#### Step 8 — 빌드 등록과 실행  `(Makefile)`

```
# Makefile  — add to the UPROGS list
UPROGS=\
	$U/_cat\
	# ... existing entries ...
	$U/_sync\
	$U/_freetest\        # ADD THIS LINE
```

빌드하고 실행합니다.

```
$ make qemu
...
$ freetest
```

### 1.4 확인 질문

- 부팅 직후 페이지 수가 32,768(=128MB ÷ 4KB)보다 작습니다. 그 차이는 무엇입니까?
- sbrk 로 10페이지를 요청했는데 used 가 정확히 10이 아닐 수 있습니다. 왜 그럴까요?
- freemem() 을 두 번 연달아 불러도 값이 다를 수 있습니다. 어떤 경우입니까?
- free list 를 세는 데 걸리는 시간은 페이지 수에 비례합니다. 이 값을 자주 부르는 코드가 있다면 무엇이 문제가 될까요?

### 1.5 변경 요약

| 파일 | 변경 내용 |
|---|---|
| `kernel/kalloc.c` | freemem() 함수 추가 |
| `kernel/defs.h` | freemem() 선언 추가 |
| `kernel/syscall.h` | SYS_freemem 23 추가 |
| `kernel/syscall.c` | extern 선언 + syscalls[] 등록 |
| `kernel/sysproc.c` | sys_freemem() 추가 |
| `user/user.h` | 프로토타입 추가 |
| `user/usys.pl` | entry("freemem") 추가 |
| `user/freetest.c` | 새 파일 |
| `Makefile` | UPROGS 에 _freetest 추가 |

> ⚠️ **확장 과제** freemem() 이 도는 동안 다른 코어가 kalloc 을 부르면 어떻게 될지 생각해 보고, 실제로 두 프로세스를 동시에 돌려 값이 흔들리는지 확인해 보세요.

---

## Lab 2 (중급) — vmprint()

*페이지 테이블을 재귀로 훑어 통째로 출력한다*

### 2.1 목표

페이지 테이블 하나를 받아 그 안의 유효한 항목을 전부 출력하는 커널 함수 `vmprint()` 를 만듭니다. 3단계 구조이므로 재귀로 내려갑니다.

완성하면 부팅할 때 첫 프로세스의 페이지 테이블이 화면에 찍힙니다. 강의에서 본 그림이 실제 출력으로 나타납니다.

### 2.2 학습 내용

- 페이지 테이블이 512개 항목짜리 4KB 페이지라는 것
- PTE 의 플래그 비트로 단말과 비단말을 구분하는 방법
- PTE 에서 물리 주소를 뽑아내는 매크로 PTE2PA 의 쓰임
- 재귀로 트리를 훑는 코드가 커널에서 어떻게 생겼는지

### 2.3 단계별 구현

#### Step 1 — 재귀 출력 함수 작성  `(kernel/vm.c)`

`kernel/vm.c` 맨 아래에 두 함수를 추가합니다. 안쪽 함수가 재귀를 돌고, 바깥 함수는 첫 줄을 찍고 재귀를 시작합니다.

```
// kernel/vm.c  — add at the bottom

static void
vmprint_level(pagetable_t pagetable, int level)
{
  for (int i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];

    if ((pte & PTE_V) == 0)
      continue;               // this slot is empty

    printk("..");
    // TODO ①  print " .." once more for each extra level
    printk("%d: pte %p pa %p\n", i, (void *)pte, (void *)PTE2PA(pte));

    // TODO ②  if this PTE is NOT a leaf, go one level deeper
  }
}

void
vmprint(pagetable_t pagetable)
{
  printk("page table %p\n", (void *)pagetable);
  vmprint_level(pagetable, 0);
}
```

> ℹ️ **핵심** 비단말 PTE 를 어떻게 알아볼까요? 단말 PTE 에는 R · W · X 중 최소 하나가 켜져 있습니다. 셋이 모두 0 이면서 V 만 켜져 있으면, 그 pa 는 데이터가 아니라 다음 표의 주소입니다.

#### Step 2 — 함수 선언 추가  `(kernel/defs.h)`

```
// kernel/defs.h  — in the vm.c section
void            vmprint(pagetable_t);     // ADD THIS LINE
```

#### Step 3 — 첫 프로세스에서 호출  `(kernel/exec.c)`

`kexec()` 의 마지막, `return argc;` 바로 앞에 넣습니다. 모든 프로세스마다 찍으면 화면이 넘치므로 첫 프로세스에만 제한합니다.

```
// kernel/exec.c  — just before "return argc;"
  if (p->pid == 1)
    vmprint(p->pagetable);

  return argc;
```

#### Step 4 — 빌드와 확인

```
$ make qemu
xv6 kernel is booting
hart 2 starting
...
page table 0x0000000087f52000
..0: pte 0x0000000021fd3801 pa 0x0000000087f4e000
.. ..0: pte 0x0000000021fd3401 pa 0x0000000087f4d000
.. .. ..0: pte 0x0000000021fd3c1b pa 0x0000000087f4f000
...
..255: pte 0x0000000021fd4401 pa 0x0000000087f51000
.. ..511: pte 0x0000000021fd4001 pa 0x0000000087f50000
.. .. ..510: pte 0x0000000021fd5807 pa 0x0000000087f56000
.. .. ..511: pte 0x000000002000180b pa 0x0000000080006000
```

> ⚠️ **주소가 다른 경우** 위 숫자는 예시입니다. 여러분 환경에서는 다른 값이 나올 수 있지만, 구조와 인덱스(0 · 255 · 511 · 510)는 같아야 합니다.

### 2.4 출력 읽는 법

점 두 개(`..`)가 한 층입니다. 점이 없으면 L2, `..` 면 L1, `.. ..` 면 L0 입니다.

| 출력 줄 | 무엇인가 |
|---|---|
| `..0: 이하 서브트리` | VPN[2]=0 아래 — text · data · 가드 페이지 · 스택 (낮은 주소) |
| `..255 → ..511 → ..511` | 주소 공간 맨 끝 페이지 = TRAMPOLINE (VA 0x3F_FFFF_F000) |
| `.. .. ..510` | TRAMPOLINE 바로 아래 = TRAPFRAME |

마지막 줄의 PTE 플래그를 손으로 풀어 보세요. `0x2000180b` 의 아래 10비트는 `0b0000001011` 이고 이는 V · R · X 입니다. 실행은 되지만 쓸 수 없고, U 가 없으니 사용자 모드에서는 보이지 않습니다 — trampoline 의 성질 그대로입니다.

### 2.5 확인 질문

- 512개 항목 중 실제로 출력되는 줄은 몇 개입니까? 왜 그렇게 적을까요?
- PTE_V 검사를 빼면 무슨 일이 벌어질까요? 실제로 빼고 돌려 보세요.
- 단말 판정을 (pte & PTE_V) 로 바꾸면 어떤 출력이 나올까요?
- 가드 페이지는 출력에 나타납니까? 나타난다면 어느 줄이고, 플래그는 무엇입니까?

### 2.6 변경 요약

| 파일 | 변경 내용 |
|---|---|
| `kernel/vm.c` | vmprint_level() · vmprint() 추가 |
| `kernel/defs.h` | vmprint() 선언 추가 |
| `kernel/exec.c` | 첫 프로세스에 한해 vmprint() 호출 |

> ⚠️ **확장 과제** vmprint 를 시스템콜로도 만들어 보세요(SYS_vmprint 25). 그러면 부팅 때뿐 아니라 원하는 시점에 자기 프로세스의 페이지 테이블을 찍을 수 있습니다. Lab 4 의 관찰이 훨씬 수월해집니다.

---

## Lab 3 (고급) — va2pa()

*가상 주소를 받아 물리 주소로 번역해 돌려준다*

### 3.1 목표

사용자 프로그램이 준 가상 주소를 커널이 페이지 테이블을 따라가 번역하고, 그 물리 주소를 돌려주는 시스템콜 `va2pa()` 를 만듭니다.

강의에서 손으로 계산한 주소 변환을 커널이 대신 해 주는 셈입니다. Lab 2 의 출력과 대조하면 계산이 맞는지 스스로 검증할 수 있습니다.

### 3.2 학습 내용

- walkaddr() 가 하는 일과 그 반환값의 성질
- 페이지 경계와 오프셋을 분리해 다시 합치는 방법
- 매핑되지 않은 주소, 사용자 모드에서 볼 수 없는 주소를 커널이 어떻게 구분하는지
- 사용자가 준 주소를 커널이 그대로 믿으면 안 되는 이유

### 3.3 단계별 구현

#### Step 1 — 시스템콜 번호와 등록  `(kernel/syscall.h, kernel/syscall.c)`

```
// kernel/syscall.h
#define SYS_va2pa 24        // ADD THIS LINE
```

```
// kernel/syscall.c
extern uint64 sys_va2pa(void);            // with the other externs
  [SYS_va2pa]   sys_va2pa,                // in the syscalls[] array
```

#### Step 2 — 커널 함수 구현  `(kernel/sysproc.c)`

`walk()` 와 `walkaddr()` 는 이미 `kernel/defs.h` 에 선언되어 있습니다. vm.c 를 고칠 필요가 없습니다.

```
// kernel/sysproc.c  — add at the bottom
uint64
sys_va2pa(void)
{
  uint64 va;
  struct proc *p = myproc();

  // TODO ①  read the first argument into va

  uint64 base = walkaddr(p->pagetable, va);
  if (base == 0)
    return 0;              // not mapped, or not user-accessible

  // TODO ②  walkaddr returns a page-aligned address.
  // TODO ②  add the offset within the page back on.
}
```

> ℹ️ **핵심** walkaddr() 는 페이지의 시작 주소만 돌려줍니다. 아래 12비트는 잘려 있습니다. 강의에서 본 대로 오프셋은 변환하지 않고 그대로 붙이는 부분이므로, 원래 va 에서 그 12비트를 꺼내 더해야 정확한 물리 주소가 됩니다.

> ⚠️ **생각해 볼 점** walkaddr() 는 PTE_U 가 없는 페이지에 대해 0 을 돌려줍니다. 그래서 trampoline 주소를 넣으면 0 이 나옵니다. 커널이 이렇게 막아 두는 이유는 무엇일까요?

#### Step 3 — 사용자 쪽 노출  `(user/user.h, user/usys.pl)`

```
// user/user.h
uint64 va2pa(uint64);
```

```
# user/usys.pl
entry("va2pa");
```

#### Step 4 — 테스트 프로그램 작성  `(user/vatest.c)`

```
// user/vatest.c  — new file
#include "kernel/types.h"
#include "user/user.h"

int global = 42;

int
main(void)
{
  int local = 7;
  char *heap = sbrk(4096);

  printf("&global %p -> pa %p\n", &global, (void *)va2pa((uint64)&global));
  printf("&local  %p -> pa %p\n", &local,  (void *)va2pa((uint64)&local));
  printf("heap    %p -> pa %p\n", heap,    (void *)va2pa((uint64)heap));

  // the very top page of the address space = TRAMPOLINE
  printf("tramp   %p -> pa %p\n", (void *)0x3ffffff000L,
         (void *)va2pa(0x3ffffff000L));

  exit(0);
}
```

Makefile 의 UPROGS 에 _vatest 를 추가하고 실행합니다.

### 3.4 검증 — 수동 계산과 대조

출력된 가상 주소 하나를 골라 직접 분해해 보세요. 예를 들어 `&global` 이 `0x0000000000002018` 이라면 —

```
VPN[2] = (va >> 30) & 0x1FF = 0
VPN[1] = (va >> 21) & 0x1FF = 0
VPN[0] = (va >> 12) & 0x1FF = 2
offset =  va        & 0xFFF = 0x018
```

Lab 2 의 출력에서 `..0 → .. ..0 → .. .. ..2` 줄을 찾아 그 pa 를 확인하고, 거기에 0x018 을 더한 값이 va2pa 의 결과와 같은지 보세요. 같다면 여러분이 하드웨어와 똑같이 계산한 것입니다.

### 3.5 확인 질문

- 아직 sbrk 로 늘리지 않은 주소를 넣으면 0 이 나옵니다. 커널 입장에서 이 주소는 무엇이 다릅니까?
- trampoline 주소가 0 을 돌려주는 이유는 무엇입니까? 커널 자신은 그 페이지를 볼 수 있는데도 말이죠.
- 같은 프로그램을 두 번 실행하면 가상 주소는 같지만 물리 주소는 다를 수 있습니다. 왜 그렇습니까?
- fork() 직후 부모와 자식에서 같은 변수의 va2pa 를 찍으면 어떻게 나올까요? 직접 확인해 보세요.

### 3.6 변경 요약

| 파일 | 변경 내용 |
|---|---|
| `kernel/syscall.h` | SYS_va2pa 24 추가 |
| `kernel/syscall.c` | extern 선언 + syscalls[] 등록 |
| `kernel/sysproc.c` | sys_va2pa() 추가 |
| `user/user.h` | 프로토타입 추가 |
| `user/usys.pl` | entry("va2pa") 추가 |
| `user/vatest.c` | 새 파일 |
| `Makefile` | UPROGS 에 _vatest 추가 |

---

## Lab 4 (관찰) — 가드 페이지 확인

*앞의 세 랩을 이용해 “거기에 구멍이 있다”를 스스로 증명한다*

### 4.1 목표

강의에서 스택 아래에 가드 페이지가 있다고 배웠습니다. 이번에는 그것을 말이 아니라 실행 결과로 확인합니다. 새로 만들 커널 코드는 없고, 앞서 만든 va2pa 를 도구로 씁니다.

두 방향에서 접근합니다. 하나는 주소 공간을 아래로 훑어 구멍을 찾는 것, 다른 하나는 실제로 스택을 넘치게 해서 죽는 주소를 확인하는 것입니다. 두 결과가 같은 자리를 가리키면 증명이 끝납니다.

### 4.2 실험 1 — 주소 공간의 구멍 찾기

지역 변수의 주소에서 출발해 4KB 씩 아래로 내려가며 `va2pa` 를 찍습니다. 매핑된 페이지는 물리 주소가 나오고, 매핑되지 않았거나 사용자 모드에서 볼 수 없는 페이지는 0 이 나옵니다.

```
// user/holetest.c  — new file
#include "kernel/types.h"
#include "user/user.h"

#define PAGE 4096

int
main(void)
{
  int local = 0;
  uint64 a = (uint64)&local & ~(uint64)(PAGE - 1);   // page of the stack

  for (int i = 0; i < 8; i++) {
    uint64 pa = va2pa(a);
    printf("va %p -> %s %p\n", (void *)a,
           pa ? "pa" : "-- HOLE --", (void *)pa);
    a -= PAGE;
  }
  exit(0);
}
```

실행하면 이런 모양이 나옵니다.

```
$ holetest
va 0x0000000000005000 -> pa 0x0000000087f4a000     <- stack
va 0x0000000000004000 -> -- HOLE -- 0x0000000000000000     <- guard page
va 0x0000000000003000 -> pa 0x0000000087f4b000     <- data
va 0x0000000000002000 -> pa 0x0000000087f4c000
...
```

> ℹ️ **핵심** 스택 바로 아래 한 페이지에서만 0 이 나옵니다. 위에도 아래에도 정상적인 물리 주소가 있는데 그 한 장만 비어 있습니다. 이것이 가드 페이지입니다.

### 4.3 실험 2 — 스택 오버플로 유발

재귀를 깊게 돌려 스택 포인터를 아래로 밀어냅니다. 스택은 한 페이지뿐이므로 곧 가드 페이지를 밟습니다.

```
// user/stackboom.c  — new file
#include "kernel/types.h"
#include "user/user.h"

int
deep(int n)
{
  char pad[256];        // eat 256 bytes of stack per call
  pad[0] = (char)n;
  if (n == 0)
    return 0;
  return deep(n - 1) + pad[0];
}

int
main(void)
{
  printf("going deep...\n");
  deep(1000);
  printf("survived (you should NOT see this)\n");
  exit(0);
}
```

실행하면 커널이 이렇게 찍고 프로세스를 죽입니다.

```
$ stackboom
going deep...
usertrap(): unexpected scause 0xf pid=4
            sepc=0x0000000000000f2a stval=0x0000000000004ff8
```

| 출력 | 뜻 |
|---|---|
| `scause 0xf` | 15 = store page fault. 쓰기를 시도하다 폴트가 났다 |
| `scause 0xd` | 13 = load page fault. 읽기를 시도하다 폴트가 났다 |
| `sepc` | 폴트를 일으킨 명령어의 주소 |
| `stval` | 접근하려 했던 그 주소 — 바로 이것이 우리가 찾는 값 |

### 4.4 두 실험 결과 대조

실험 2 의 `stval` 값을 4KB 경계로 내림하면(아래 12비트를 0 으로) 실험 1 에서 구멍이 났던 주소와 같아야 합니다.

```
stval  = 0x0000000000004ff8
       & ~0xFFF
       = 0x0000000000004000        <- 실험 1 의 HOLE 과 같은 주소
```

같은 주소가 나왔다면 증명이 끝난 것입니다. 커널이 그 자리를 일부러 비워 두었고, 스택이 넘치는 순간 정확히 그 자리를 밟아 프로세스가 죽었습니다.

### 4.5 확인 질문

- 가드 페이지는 사실 매핑되어 있습니다. R 과 W 는 켜져 있고 U 만 지워져 있습니다. 그런데 왜 va2pa 는 0 을 돌려줄까요?
- 아예 매핑하지 않아도 폴트는 납니다. 그런데 커널은 왜 굳이 자리를 잡아 두고 U 비트만 지웠을까요?
- pad 의 크기를 256 에서 8 로 줄이면 몇 번째 호출에서 죽을까요? 예측하고 확인해 보세요.
- 스택을 두 페이지로 늘리려면 어디를 고쳐야 합니까? 고치고 나면 실험 1 의 구멍 위치가 어떻게 바뀔까요?

### 4.6 변경 요약

| 파일 | 변경 내용 |
|---|---|
| `user/holetest.c` | 새 파일 — 주소 공간을 아래로 훑는다 |
| `user/stackboom.c` | 새 파일 — 재귀로 스택을 넘친다 |
| `Makefile` | UPROGS 에 _holetest, _stackboom 추가 |

> ⚠️ **확장 과제** kernel/param.h 의 USERSTACK 을 1 에서 4 로 바꾸고 커널을 다시 빌드해 보세요. 실험 1 의 구멍 위치와 실험 2 가 죽는 깊이가 어떻게 달라지는지 기록해 오세요.

---

## 종합 — 네 랩이 무엇을 이었나

|   | Lab 1 freemem | Lab 2 vmprint | Lab 3 va2pa | Lab 4 가드 페이지 |
|---|---|---|---|---|
| 보는 것 | 물리 메모리 | 페이지 테이블 | 둘 사이의 변환 | 변환의 빈자리 |
| 건드리는 파일 | kalloc.c | vm.c · exec.c | sysproc.c | 없음 (관찰) |
| 새 시스템콜 | freemem (23) | 없음 | va2pa (24) | 없음 |
| 핵심 도구 | free list 순회 | 재귀 · PTE 플래그 | walkaddr · 오프셋 | 앞의 셋 |
| 확인 방법 | 숫자가 32,768 미만 | 부팅 출력 | 손 계산과 일치 | 두 실험이 같은 주소 |

### 개념 카드 ① — PTE 는 주소와 권한을 함께 담는다

PTE 는 64비트입니다. 위쪽 44비트가 물리 페이지 번호(PPN), 아래쪽 10비트가 플래그입니다.

```
pa    = (pte >> 10) << 12        // PTE2PA(pte)
flags =  pte & 0x3FF             // PTE_FLAGS(pte)

// flag bits:  V=1  R=2  W=4  X=8  U=16
```

| 플래그 | 값의 예 | 무엇인가 |
|---|---|---|
| `V 만` | `0x...401` | 비단말 — 이 pa 는 다음 표의 주소다 |
| `V R X` | `0x...80b` | 단말, 실행 가능, 쓰기 불가 — trampoline |
| `V R W` | `0x...807` | 단말, 쓰기 가능, 실행 불가 — trapframe |
| `U 없음` | — | 사용자 모드에서는 접근 불가 — 가드 페이지도 여기 |

### 개념 카드 ② — 커널이 사용자 주소를 다루는 세 가지 방법

| 함수 | 쓰임 | 이번 실습에서 |
|---|---|---|
| `walkaddr(pt, va)` | 가상 주소 → 물리 페이지 주소. 사용자 페이지만. | Lab 3 에서 사용 |
| `walk(pt, va, alloc)` | PTE 자체를 얻는다. alloc=1 이면 없는 층을 만든다. | Lab 2 확장에서 |
| `copyin / copyout` | 사용자 메모리와 커널 메모리 사이의 안전한 복사 | 다음 주제에서 |

> ℹ️ **왜 커널은 사용자 포인터를 그냥 못 쓰나** 사용자가 준 주소가 진짜 그 프로세스의 것인지 커널은 알 수 없습니다. 게다가 커널의 페이지 테이블에서 그 주소는 전혀 다른 곳을 가리킵니다. 그래서 반드시 그 프로세스의 페이지 테이블을 따라 번역해야 합니다 — 이번 실습에서 만든 va2pa 가 바로 그 일을 축소해 놓은 것입니다.

---

## 제출

네 개의 랩을 마쳤으면 커밋하고 올립니다.

```
git status                       # 무엇이 바뀌었나 (확인용)
git diff riscv...                # 분기 지점 이후 내가 고친 것 전부 (확인용)

git add -A
git commit -m "lab2: freemem, vmprint, va2pa, 가드 페이지 실험"
git push -u origin lab2-memory

git log --oneline                # 커밋이 잘 남았는지 (선택)
```

> ℹ️ **필수는 세 줄입니다** add → commit → push 가 한 묶음입니다. status · diff · log 는 확인용이라 건너뛰어도 결과는 같습니다. 다만 commit 전에 diff 로 한 번 훑어보는 습관은 들여 두세요.

> ⚠️ **Permission denied 가 나온다면** 수업 저장소를 fork 하지 않고 그대로 clone 한 경우입니다. utils.md 를 참고해 내 저장소를 연결하세요.

제출물은 `lab2-memory` 브랜치 또는 `git diff riscv...` 결과입니다. 확인 질문에 대한 답도 함께 내세요.

---

## 부록 A — TODO 정답

먼저 스스로 5분은 생각해 본 뒤에 보세요.

### Lab 1

```
// TODO ①  — 락으로 감싼다
acquire(&kmem.lock);
  ...
release(&kmem.lock);

// TODO ②  — 세고 다음으로 넘어간다
n++;
r = r->next;

// TODO ③  — 10페이지를 한 바이트씩 건드린다
for (int i = 0; i < 10; i++)
  p[i * PAGE] = 1;
```

### Lab 2

```
// TODO ①  — 층수만큼 " .." 를 더 찍는다
for (int j = 0; j < level; j++)
  printk(" ..");

// TODO ②  — 비단말이면 한 층 내려간다
if ((pte & (PTE_R | PTE_W | PTE_X)) == 0)
  vmprint_level((pagetable_t)PTE2PA(pte), level + 1);
```

### Lab 3

```
// TODO ①  — 첫 번째 인자를 주소로 받는다
argaddr(0, &va);

// TODO ②  — 페이지 안 오프셋을 되붙인다
return base + (va & 0xFFF);
```

## 부록 B — 자주 막히는 곳

| 증상 | 원인과 해결 |
|---|---|
| undefined reference to `freemem' | user/usys.pl 에 entry 를 넣지 않았거나, user.h 프로토타입이 없습니다. |
| 알 수 없는 시스템콜 번호 오류 | syscalls[] 배열에 등록하지 않았습니다. extern 선언만으로는 부족합니다. |
| freetest 를 쳤는데 없다고 나옴 | Makefile 의 UPROGS 에 추가하지 않았습니다. 추가 후 make clean 없이 make qemu 하면 됩니다. |
| vmprint 가 아무것도 안 찍음 | exec.c 의 호출을 return argc 뒤에 넣었을 수 있습니다. 반드시 앞이어야 합니다. |
| vmprint 출력이 끝없이 나옴 | 단말 판정이 틀려 데이터 페이지까지 내려가고 있습니다. R·W·X 를 모두 검사했는지 보세요. |
| va2pa 결과가 4096 의 배수만 나옴 | 오프셋을 더하지 않았습니다. walkaddr 은 페이지 시작 주소만 돌려줍니다. |
| 커널이 panic 하며 멈춤 | 락을 잡고 놓지 않았거나 두 번 잡았을 수 있습니다. panic 메시지의 락 이름을 확인하세요. |
| Ctrl-C 로 qemu 가 안 꺼짐 | Ctrl-A 를 누른 뒤 X 입니다. |

