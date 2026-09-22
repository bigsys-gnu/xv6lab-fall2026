# Lab 3 — 페이지 테이블 들여다보기

**vmprint() · va2pa()**

*새 알고리즘은 없습니다. 강의에서 본 매크로와 함수를 조합할 뿐  ·  약 2시간*

> 환경 설정은 [lab0-setup.md](lab0-setup.md), git · gdb 사용법은 [utils.md](utils.md) 를 참고하세요.

## 목차

- [개요](#개요)
  - [강의와 이어지는 부분](#강의와-이어지는-부분)
  - [이번에 쓰는 도구](#이번에-쓰는-도구)
  - [시스템 호출 번호 배정](#시스템-호출-번호-배정)
  - [실습 브랜치 만들기](#실습-브랜치-만들기)
  - [문서 읽는 법](#문서-읽는-법)
- [3.1 vmprint() — 트리 전체를 훑기](#31-vmprint-트리-전체를-훑기)
  - [3.1.1 목표](#311-목표)
  - [3.1.2 학습 내용](#312-학습-내용)
  - [3.1.3 단계별 구현](#313-단계별-구현)
  - [3.1.4 출력 읽는 법](#314-출력-읽는-법)
  - [3.1.5 확인 질문](#315-확인-질문)
  - [3.1.6 변경 요약](#316-변경-요약)
- [3.2 va2pa() — 주소 하나를 따라 내려가기](#32-va2pa-주소-하나를-따라-내려가기)
  - [3.2.1 목표](#321-목표)
  - [3.2.2 학습 내용](#322-학습-내용)
  - [3.2.3 단계별 구현](#323-단계별-구현)
  - [3.2.4 손 계산과 대조](#324-손-계산과-대조)
  - [3.2.5 확인 질문](#325-확인-질문)
  - [3.2.6 변경 요약](#326-변경-요약)
- [3.3 커밋하고 올리기](#33-커밋하고-올리기)
- [종합 — 두 실습 비교](#종합-두-실습-비교)
- [부록 A — TODO 정답](#부록-a-todo-정답)
- [부록 B — 자주 막히는 곳](#부록-b-자주-막히는-곳)

## 개요

강의에서 페이지 테이블을 그림으로만 봤습니다. L2 · L1 · L0 세 단계, 칸마다 512개, 칸 안에는 PPN 과 플래그. 이번에는 그 그림을 **실제 커널에서 꺼내 봅니다.**

두 방향에서 접근합니다. 3.1 은 트리 **전체**를 위에서부터 훑고, 3.2 는 주소 **하나**를 따라 아래로 내려갑니다. 두 실습 모두 새로 짤 알고리즘은 없습니다. 이미 커널에 있는 매크로와 함수를 제자리에 놓기만 하면 됩니다.

| 단계  | 무엇을         | 난이도 | 시간  |
| --- | ----------- | --- | --- |
| 3.1 | `vmprint()` | 초급  | 50분 |
| 3.2 | `va2pa()`   | 중급  | 60분 |
| 3.3 | 커밋하고 올리기    | —   | 10분 |

### 강의와 이어지는 부분

실습을 하다 막히면 해당 슬라이드를 다시 펴 보세요.

| 강의에서 본 것                       | 이번에 확인하는 곳          |
| ------------------------------ | ------------------- |
| 희소 트리 — L2 에서 0번과 255번 칸만 켜진다  | 3.1 출력              |
| 중간 PTE 는 R · W · X 가 모두 0 이다   | 3.1 TODO ②          |
| `PTE2PA` 는 PPN 을 꺼내 4096 을 곱한다 | 3.1 · 3.2 모두        |
| `PX(level, va)` 로 칸 번호를 뽑는다    | 3.2 TODO ①          |
| `walk(…, 0)` 은 찾기만 하고 만들지 않는다  | 3.2 Step 3          |
| 가드 페이지는 매핑은 있고 `PTE_U` 만 꺼져 있다 | 3.1 확인 질문 · 3.2 테스트 |

### 이번에 쓰는 도구

모두 이미 커널에 있습니다. 새로 만들지 않습니다.

| 이름                                      | 위치                | 하는 일                       |
| --------------------------------------- | ----------------- | -------------------------- |
| `PTE_V · PTE_R · PTE_W · PTE_X · PTE_U` | `kernel/riscv.h`  | PTE 의 플래그 비트               |
| `PTE2PA(pte)`                           | `kernel/riscv.h`  | PTE 에서 페이지 시작 물리 주소를 얻는다   |
| `PX(level, va)`                         | `kernel/riscv.h`  | 가상 주소에서 해당 단계의 칸 번호(0~511) |
| `MAXVA`                                 | `kernel/riscv.h`  | 쓸 수 있는 가상 주소의 상한 (2³⁸)     |
| `walk(pt, va, alloc)`                   | `kernel/vm.c`     | va 가 쓸 PTE 칸의 **주소**를 돌려준다 |
| `printk(...)`                           | `kernel/printk.c` | 커널 쪽 출력 (예전 버전의 `printf`)  |

> ℹ️ **`walk` 는 이미 `kernel/defs.h` 에 선언되어 있습니다** vm.c 밖에서 바로 부를 수 있습니다. 선언을 새로 넣을 필요가 없습니다.

### 시스템 호출 번호 배정

이번 실습은 `riscv` 에서 새로 갈라진 브랜치에서 합니다. 현재 마지막 번호가 `SYS_sync 22` 이므로 **23** 을 씁니다.

```
// kernel/syscall.h
#define SYS_va2pa 23
```

> ℹ️ **Lab 2 에서도 23 을 썼는데요?** 괜찮습니다. Lab 2 의 `freepages` 는 `lab2-syscall` 브랜치에만 있고, 이번 브랜치는 그것을 모릅니다. 번호는 항상 **지금 브랜치의 코드 기준** 으로 셉니다.

3.1 의 `vmprint()` 는 시스템 호출이 아니라 커널 안에서만 부르는 함수입니다. 번호가 필요 없습니다.

### 실습 브랜치 만들기

```
git checkout riscv               # 손대지 않은 원본으로 이동
git checkout -b lab3-memory      # 브랜치를 만들고 그리로 이동
git branch                       # * lab3-memory 로 바뀌었는지 확인
```

### 문서 읽는 법

- 각 단계의 제목에 **고칠 파일 경로** 가 붙어 있습니다.
- `// TODO ①` 로 표시된 자리는 직접 채워 넣으세요. 그 줄만 비어 있고 나머지는 다 나와 있습니다.
- 막히면 부록 B 의 「자주 막히는 곳」 을 먼저 펴 보세요. 정답은 부록 A 에 있지만 최소 5분은 스스로 생각해 보세요.

---

## 3.1 vmprint() — 트리 전체를 훑기

### 3.1.1 목표

페이지 테이블 하나를 받아 **유효한 칸만** 출력하는 커널 함수 `vmprint()` 를 만듭니다. 트리가 3단계이므로 재귀로 내려갑니다.

완성하면 부팅할 때 첫 프로세스(`init`)의 페이지 테이블이 화면에 찍힙니다. 강의의 「exec 직후」 그림이 실제 숫자로 나타납니다.

### 3.1.2 학습 내용

- 페이지 테이블 한 장이 PTE 512칸짜리 배열이라는 것
- `PTE_V` 로 빈 칸을 거르고, R · W · X 로 **다음 테이블인지 최종 페이지인지** 가르는 방법
- `PTE2PA` 의 결과가 다음 단계 테이블의 **시작 주소** 라는 것
- 매핑이 없는 가지는 아예 만들어지지 않는다는 것 — 희소 트리

### 3.1.3 단계별 구현

#### Step 1 — 재귀 출력 함수를 만든다  `(kernel/vm.c)`

파일 맨 아래에 두 함수를 추가합니다. 안쪽 함수가 한 단계를 훑고 필요하면 한 단계 더 내려갑니다. 바깥 함수는 루트 주소를 찍고 재귀를 시작합니다.

`level` 은 **강의의 `PX(level, va)` 와 같은 번호** 를 씁니다. 루트가 2, 그 아래가 1, 맨 아래가 0 입니다. `walk` 의 `for (level = 2; level > 0; level--)` 와 같은 방향입니다.

```
// kernel/vm.c  — add at the bottom of the file

// Print the valid PTEs of one page-table page,
// then descend into lower-level tables.
// level: 2 = root (L2), 1 = L1, 0 = L0  — same numbering as PX(level, va)
static void
vmprint_level(pagetable_t pagetable, int level)
{
  for (int i = 0; i < 512; i++) {
    pte_t pte = pagetable[i];

    if ((pte & PTE_V) == 0)
      continue;                         // empty slot

    // TODO ①  indent: print " .." once for L2, twice for L1, three times for L0

    printk("%d: pte %p pa %p\n", i, (void *)pte, (void *)PTE2PA(pte));

    // TODO ②  if this PTE points to a lower-level table, descend into it
  }
}

void
vmprint(pagetable_t pagetable)
{
  printk("page table %p\n", (void *)pagetable);
  vmprint_level(pagetable, 2);
}
```

> ℹ️ **다음 테이블인지 어떻게 아나** 강의의 `walk` 한 줄씩 ② 를 떠올리세요. 중간 테이블을 새로 만들 때 `*pte = PA2PTE(pagetable) | PTE_V;` 였습니다. R · W · X 를 **하나도 붙이지 않았습니다.** 그러니 V 만 켜져 있고 R · W · X 가 모두 0 이면, 그 PTE 가 가리키는 곳은 데이터가 아니라 **다음 단계 테이블** 입니다.

> ℹ️ **`PTE2PA` 한 번이면 됩니다** 결과가 곧 다음 테이블의 0번 칸 주소입니다. 커널이 물리 메모리를 같은 주소로 직접 매핑해 두었으므로 `(pagetable_t)` 로 형변환해 그대로 배열처럼 씁니다 — `walk` 가 하던 일과 똑같습니다.

#### Step 2 — 다른 파일에서 부를 수 있게 선언한다  `(kernel/defs.h)`

`vm.c` 묶음에 한 줄을 더합니다.

```
// kernel/defs.h  — vm.c 묶음
pte_t *         walk(pagetable_t, uint64, int);
uint64          walkaddr(pagetable_t, uint64);
void            vmprint(pagetable_t);        // 추가
```

#### Step 3 — 첫 프로세스에서 한 번 부른다  `(kernel/exec.c)`

`kexec()` 의 맨 끝, 새 페이지 테이블로 **교체가 끝난 뒤** `return argc;` 바로 앞에 넣습니다. 모든 프로세스마다 찍으면 화면이 넘치므로 첫 프로세스에만 제한합니다.

```
// kernel/exec.c  — kexec() 의 끝부분
  p->trapframe->sp = sp;         // initial stack pointer
  proc_freepagetable(oldpagetable, oldsz);

  if (p->pid == 1)               // 추가
    vmprint(p->pagetable);       // 추가

  return argc;
```

> ⚠️ **반드시 `return argc;` 앞** 뒤에 두면 절대 실행되지 않습니다. 컴파일은 통과하므로 알아차리기 어렵습니다.

#### Step 4 — 빌드하고 확인한다

```
$ make qemu
xv6 kernel is booting
...
```

`init` 이 시작될 때 이런 **모양** 이 찍혀야 합니다. 숫자는 환경마다 다릅니다.

```
page table 0x…
 ..0: pte 0x… pa 0x…
 .. ..0: pte 0x… pa 0x…
 .. .. ..0: pte 0x… pa 0x…
 .. .. ..1: pte 0x… pa 0x…
 .. .. ..2: pte 0x… pa 0x…
 .. .. ..3: pte 0x… pa 0x…
 ..255: pte 0x… pa 0x…
 .. ..511: pte 0x… pa 0x…
 .. .. ..510: pte 0x… pa 0x…
 .. .. ..511: pte 0x… pa 0x…
```

> ⚠️ **숫자보다 구조를 보세요** 주소 값은 매번 다를 수 있습니다. 하지만 **L2 에서 0 · 255 두 칸만** 나오고, 255 아래가 **511 → 510 · 511** 이어야 합니다. 맨 아래 칸의 개수는 `init` 의 크기에 따라 조금 다를 수 있습니다.

### 3.1.4 출력 읽는 법

` ..` 하나가 한 단계입니다. 하나면 L2, 둘이면 L1, 셋이면 L0 입니다.

| 출력 줄                                | 무엇인가                                             |
| ----------------------------------- | ------------------------------------------------ |
| ` ..0` 아래 전부                        | 사용자 메모리 가지 — text · data · 가드 페이지 · 스택 (낮은 주소)   |
| ` ..255 →  .. ..511 →  .. .. ..511` | 주소 공간 맨 꼭대기 = 트램펄린 (`TRAMPOLINE = 0x3FFFFFF000`) |
| ` .. .. ..510`                      | 트램펄린 바로 아래 = 트랩프레임                               |

` ..0` 과 ` ..255` 는 **루트 한 장만 공유하고** 그 아래로는 완전히 별개의 가지입니다. 강의 「exec 직후」 그림의 두 가지가 바로 이 두 덩어리입니다.

### 3.1.5 확인 질문

- L2 테이블 512칸 중 출력에 나온 칸은 몇 개입니까? 왜 0 과 255 뿐입니까? (힌트 — `TRAMPOLINE` 을 `PX(2, …)` 로 계산해 보세요)
- 출력에 나온 **페이지 테이블 페이지** 는 모두 몇 장입니까? 줄마다 「이 줄의 pa 가 테이블인가, 데이터인가」 를 따져 세어 보세요. 강의의 「exec 직후 테이블 5장」 과 맞습니까?
- 중간 단계 줄(` ..0`, ` .. ..0`, ` ..255`, ` .. ..511`)의 pte 값을 `& 0x3FF` 해 보세요. 어떤 비트가 켜져 있습니까? 규칙과 맞습니까?
- 맨 아래 칸 중 하나는 가드 페이지입니다. 어느 줄입니까? 그 줄의 pte 에서 `PTE_U`(0x10) 는 켜져 있습니까?
- TODO ② 의 조건을 `(pte & PTE_V)` 로 바꾸면 어떻게 될까요? 데이터 페이지의 내용을 테이블로 착각하면 무슨 일이 벌어질지 **먼저 예상하고** 실제로 해 보세요.

### 3.1.6 변경 요약

| #   | 파일              | 무엇을 했나                         |
| --- | --------------- | ------------------------------ |
| 1   | `kernel/vm.c`   | vmprint_level() · vmprint() 추가 |
| 2   | `kernel/defs.h` | 선언 한 줄                         |
| 3   | `kernel/exec.c` | 첫 프로세스에서만 vmprint() 호출         |

> ℹ️ **확장 과제 — 권한을 글자로** pte 값 옆에 `rwxu` 네 글자를 찍어 보세요. 켜진 비트는 글자로, 꺼진 비트는 `-` 로. 예를 들어 text 는 `r-xu`, 트램펄린은 `r-x-` 로 나와야 합니다. 3.1.5 의 세 번째 · 네 번째 질문이 한눈에 풀립니다.

---

## 3.2 va2pa() — 주소 하나를 따라 내려가기

### 3.2.1 목표

사용자 프로그램이 넘긴 가상 주소 하나를 커널이 `walk` 로 따라가 **물리 주소로 바꿔 돌려주는** 시스템 호출 `va2pa()` 를 만듭니다. 내려가는 도중 각 단계의 칸 번호도 함께 찍습니다.

강의 「전체 추적」 슬라이드에서 손으로 했던 계산을, 이번에는 커널이 대신 해 줍니다.

### 3.2.2 학습 내용

- `PX(level, va)` 로 세 칸 번호와 오프셋을 뽑는 방법
- `walk(pt, va, 0)` — **alloc = 0** 이면 찾기만 하고 테이블을 만들지 않는다는 것
- `PTE2PA` 는 페이지 **시작** 주소만 준다 — 오프셋은 따로 더해야 한다
- 매핑이 없는 주소, 매핑은 있지만 사용자용이 아닌 주소를 구분하는 방법

> ℹ️ **이번에는 copyout 이 필요 없습니다** 사용자가 준 주소를 **읽거나 쓰지 않고**, 그 숫자를 페이지 테이블로 **번역만** 합니다. 결과는 숫자 하나라 반환값(`a0`)으로 충분합니다.

### 3.2.3 단계별 구현

#### Step 1 — 번호를 배정한다  `(kernel/syscall.h)`

```
#define SYS_sync   22
#define SYS_va2pa  23        // 추가
```

#### Step 2 — 선언하고 표에 등록한다  `(kernel/syscall.c)`

```
// 파일 위쪽, 다른 extern 들 옆에
extern uint64 sys_va2pa(void);

// syscalls[] 배열 안에
static uint64 (*syscalls[])(void) = {
  ...
  [SYS_sync]    = sys_sync,
  [SYS_va2pa]   = sys_va2pa,        // 추가
};
```

#### Step 3 — 커널 쪽 함수를 구현한다  `(kernel/sysproc.c)`

인자로 받은 가상 주소를 **현재 프로세스의** 페이지 테이블에서 찾습니다. 검사가 세 겹인 이유는 아래 상자에 있습니다.

```
// kernel/sysproc.c  — add at the bottom

uint64
sys_va2pa(void)
{
  uint64 va;
  pte_t *pte;
  struct proc *p = myproc();

  argaddr(0, &va);                    // first argument, read as an address
  if (va >= MAXVA)
    return 0;                         // walk() would panic on this

  // TODO ①  print the three indices and the in-page offset of va
  //         printk("va %p : L2=%d L1=%d L0=%d off=0x%x\n", ...);

  pte = walk(p->pagetable, va, 0);    // alloc = 0 : look only, never build
  if (pte == 0 || (*pte & PTE_V) == 0)
    return 0;                         // not mapped
  if ((*pte & PTE_U) == 0)
    return 0;                         // mapped, but not for user mode

  // TODO ②  return the physical address of va  (page start + offset)
}
```

> ⚠️ **왜 `MAXVA` 를 먼저 거르나** `walk` 의 첫 줄은 `if (va >= MAXVA) panic("walk");` 입니다. 사용자가 아무 숫자나 넘길 수 있는데 그대로 `walk` 에 주면, 죽는 것은 사용자 프로그램이 아니라 **커널** 입니다. 사용자에게서 온 값은 커널 함수에 넣기 전에 먼저 걸러야 합니다.

> ℹ️ **검사 세 겹이 각각 무엇을 거르나** `pte == 0` — 중간 테이블조차 없다. `PTE_V` 가 0 — 칸은 있지만 비어 있다. `PTE_U` 가 0 — 매핑은 있지만 사용자 모드로는 못 쓰는 페이지(트램펄린 · 트랩프레임 · 가드 페이지)다.

> ℹ️ **`argaddr` 는 채워 두었습니다** Lab 2 의 `sysinfo` 에서 본 그 함수입니다. 0번째 인자를 주소로 읽어 `va` 에 넣습니다.

#### Step 4 — 사용자 쪽에 노출한다  `(user/user.h, user/usys.pl)`

```
// user/user.h  — system calls 묶음 안에
uint64 va2pa(uint64);

// user/usys.pl  — entry 목록 끝에
entry("va2pa");
```

#### Step 5 — 테스트 프로그램을 만든다  `(user/va2patest.c)`

성격이 다른 주소 세 개를 차례로 넣어 봅니다. 데이터 영역, 스택, 그리고 스택 바로 아래의 가드 페이지입니다.

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

int global = 42;                      // data

void
show(char *name, void *va)
{
  printf("%s  va %p -> pa %p\n", name, va, (void *)va2pa((uint64)va));
}

int
main(void)
{
  int local = 7;                      // stack

  show("global", &global);
  show("local ", &local);

  // one page below the stack page = guard page
  uint64 guard = ((uint64)&local & ~(uint64)(PGSIZE - 1)) - PGSIZE;
  show("guard ", (void *)guard);

  exit(0);
}
```

#### Step 6 — 빌드 목록에 넣고 실행한다  `(Makefile)`

```
# Makefile 의 UPROGS 에 추가
    $U/_va2patest\
```

```
$ make qemu
$ va2patest
```

커널의 `printk` 한 줄과 사용자의 `printf` 한 줄이 번갈아 나옵니다. 아래는 **예시** 입니다. 주소 값은 여러분 환경에서 다르지만, 뒤에 나오는 **패턴** 은 같아야 합니다.

```
va 0x0000000000001010 : L2=0 L1=0 L0=1 off=0x10
global  va 0x0000000000001010 -> pa 0x0000000087f3e010
va 0x0000000000003fbc : L2=0 L1=0 L0=3 off=0xfbc
local   va 0x0000000000003fbc -> pa 0x0000000087f3cfbc
va 0x0000000000002000 : L2=0 L1=0 L0=2 off=0x0
guard   va 0x0000000000002000 -> pa 0x0000000000000000
```

| 줄              | 확인할 패턴                                     |
| -------------- | ------------------------------------------ |
| global · local | pa 의 **아래 12비트가 va 와 똑같다**                 |
| guard          | **0** — 칸 번호는 찍혔으니 트리 안에는 있다. 그런데 왜 0 일까요? |

### 3.2.4 손 계산과 대조

출력된 va 하나를 골라 **직접** 분해해 보세요. 위 예시의 `local` 이라면 —

```
va = 0x0000000000003fbc

L2  = (va >> 30) & 0x1FF = 0
L1  = (va >> 21) & 0x1FF = 0
L0  = (va >> 12) & 0x1FF = 3
off =  va        & 0xFFF = 0xfbc
```

커널이 찍은 `L2=0 L1=0 L0=3 off=0xfbc` 와 같다면, 여러분은 하드웨어 MMU 와 **똑같은 계산** 을 한 것입니다. 이어서 pa 의 아래 세 자리(`…fbc`)가 off 와 같은지도 확인하세요.

> ⚠️ **3.1 의 출력과는 대조하지 마세요** `vmprint` 는 **pid 1(`init`)** 의 페이지 테이블만 찍습니다. `va2patest` 는 다른 프로세스라 물리 페이지가 전혀 다릅니다. 같은 프로세스의 트리와 대조하고 싶다면 아래 확장 과제를 하세요.

> ℹ️ **확장 과제 — 같은 프로세스의 트리와 대조** `sys_va2pa` 안에서 처음 한 번만 `vmprint(p->pagetable)` 를 부르게 해 보세요(static 변수 하나면 됩니다). 그러면 `va2patest` **자신의** 트리가 찍히고, 방금 번역한 pa 가 그 트리의 어느 줄에서 왔는지 직접 짝을 맞출 수 있습니다.

### 3.2.5 확인 질문

- `global` 과 `local` 은 L2 · L1 칸 번호가 똑같이 0 입니다. 왜 그렇습니까? (힌트 — L0 테이블 한 장이 덮는 범위는 몇 바이트입니까?)
- pa 의 아래 12비트는 **항상** va 의 아래 12비트와 같습니다. 이 사실을 `PTE2PA` 의 정의와 연결해 설명해 보세요.
- `guard` 는 칸 번호까지 찍혔는데 0 이 나왔습니다. Step 3 의 검사 세 겹 중 **어느 것** 에서 걸렸습니까? 그 검사를 지우면 무엇이 나올까요?
- 테스트에 `show("huge  ", (void *)0x5000000000L);` 를 추가해 보세요. 결과는 0 입니다. 이제 Step 3 의 `MAXVA` 검사 두 줄을 지우고 다시 실행하면 무엇이 멈춥니까? 사용자 프로그램입니까, 커널입니까?
- 같은 프로그램을 두 번 실행해 보세요. va 는 그대로인데 pa 는 달라질 수 있습니다. 왜 그렇습니까?

### 3.2.6 변경 요약

| #   | 파일                 | 무엇을 했나         |
| --- | ------------------ | -------------- |
| 1   | `kernel/syscall.h` | SYS_va2pa 23   |
| 2   | `kernel/syscall.c` | extern + 배열 등록 |
| 3   | `kernel/sysproc.c` | sys_va2pa() 추가 |
| 4   | `user/user.h`      | 프로토타입          |
| 5   | `user/usys.pl`     | entry("va2pa") |
| 6   | `user/va2patest.c` | 새 파일           |
| 7   | `Makefile`         | UPROGS 에 등록    |

---

## 3.3 커밋하고 올리기

```
git status                       # 무엇이 바뀌었나 (확인용)
git diff riscv...                # 분기 지점 이후 내가 고친 것 전부 (확인용)

# 커밋할 파일을 지정한다
git add kernel/vm.c kernel/defs.h kernel/exec.c
git add kernel/syscall.h kernel/syscall.c kernel/sysproc.c
git add user/user.h user/usys.pl user/va2patest.c Makefile

git status                       # 빠진 파일이 없는지 확인 (확인용)

git commit -m "lab3: vmprint, va2pa 추가"
git push -u origin lab3-memory   # 내 GitHub 저장소에 올리기

git log --oneline                # 커밋이 잘 남았는지 (선택)
```

> ℹ️ **목록이 길다면** git add -A 로 한꺼번에 담을 수도 있습니다. (선택) 다만 색인 파일(tags · cscope.*)이 섞이지 않았는지 git status 로 확인하세요.

> ⚠️ **Permission denied 가 나온다면** 수업 저장소를 fork 하지 않고 그대로 clone 한 경우입니다. utils.md 를 참고해 내 저장소를 연결하세요.

제출물은 `lab3-memory` 브랜치 또는 `git diff riscv...` 결과입니다. 확인 질문에 대한 답도 함께 내세요.

---

## 종합 — 두 실습 비교

|           | vmprint()                          | va2pa()                              |
| --------- | ---------------------------------- | ------------------------------------ |
| 방향        | 트리 **전체** 를 위에서 훑는다                | 주소 **하나** 를 따라 내려간다                  |
| 시스템 호출    | 아니다 — 커널 안에서만 부른다                  | 그렇다 — SYS_va2pa 23                   |
| 대상 테이블    | pid 1 의 테이블                        | 부른 프로세스 **자신** 의 테이블                 |
| 핵심 도구     | `PTE_V` · R·W·X 판정 · `PTE2PA` · 재귀 | `PX` · `walk(…, 0)` · `PTE2PA` + 오프셋 |
| 강의의 어느 그림 | 「exec 직후」 두 가지 트리                  | 「walk 한 줄씩」 · 「전체 추적」                |

두 실습은 같은 동작의 **두 방향** 입니다. vmprint 는 재귀로 **모든 가지** 를 내려가고, va2pa 는 `walk` 로 **한 가지만** 내려갑니다. 칸을 고르는 방식만 다를 뿐 — vmprint 는 0부터 511까지 전부, walk 는 `PX` 가 고른 한 칸 — 한 단계 내려갈 때마다 `PTE2PA` 로 다음 테이블의 0번 칸을 얻는다는 점은 같습니다.

---

---

## 부록 A — 자주 막히는 곳

| 증상                                           | 원인과 해결                                                                          |
| -------------------------------------------- | ------------------------------------------------------------------------------- |
| 부팅은 되는데 vmprint 출력이 없다                       | exec.c 의 호출을 `return argc;` 뒤에 넣었습니다. 반드시 앞이어야 합니다.                             |
| `implicit declaration of function 'vmprint'` | kernel/defs.h 에 선언을 넣지 않았습니다.                                                   |
| vmprint 출력이 끝없이 나오거나 panic 이 난다              | TODO ② 의 판정이 틀려 데이터 페이지를 테이블로 읽고 있습니다. R · W · X **셋 모두** 를 검사했는지 보세요.          |
| 들여쓰기가 L2 에서 0칸, L0 에서 2칸으로 한 칸씩 모자라다         | TODO ① 의 반복 조건이 `d < 2` 입니다. 루트에서도 한 번은 찍어야 합니다.                                |
| `control reaches end of non-void function`   | sys_va2pa 의 TODO ② 를 채우기 전에 빌드했습니다. `-Werror` 라 경고도 오류가 됩니다.                    |
| undefined reference to `va2pa'               | user/user.h 또는 user/usys.pl 을 빠뜨렸습니다. 둘 다 확인하세요.                                |
| unknown sys call 23                          | kernel/syscall.c 의 syscalls[] 배열에 등록하지 않았습니다. extern 선언도 함께 확인하세요.              |
| exec va2patest failed                        | Makefile 의 UPROGS 에 등록하지 않았습니다.                                                 |
| va2pa 결과가 전부 `…000` 으로 끝난다                   | 오프셋을 더하지 않았습니다. `PTE2PA` 는 페이지 시작 주소만 돌려줍니다.                                    |
| `panic: walk`                                | `MAXVA` 검사를 빼고 큰 주소를 넘겼습니다. 확인 질문 다섯 번째가 이 이야기입니다 — 사용자 값 하나로 **커널** 이 멈춘 것입니다. |
| `unused variable 'p'`                        | sys_va2pa 에서 `p` 를 선언만 하고 `walk` 에 넘기지 않았습니다.                                   |
| Makefile 수정 후 빌드가 깨진다                        | 줄 끝의 역슬래시(\\)를 확인하세요. 앞뒤 줄과 모양이 같아야 합니다.                                        |
| Ctrl-C 로 qemu 가 안 꺼진다                        | Ctrl-A 를 누르고 손을 뗀 다음 X 입니다.                                                     |
