# Lab 1 — 처음 만나는 xv6

**소스 구조 둘러보기 · 응용 프로그램 만들어 보기**

*(약 1시간 45분)*

> 환경 설정이 아직이라면 [lab0-setup.md](lab0-setup.md) 를 먼저 마치세요.

## 목차

- [개요](#개요)
- [1.1 소스 구조 둘러보기](#11-소스-구조-둘러보기)
  - [1.1 제출할 답](#11-제출할-답)
- [1.2 첫 프로그램 — hello.c](#12-첫-프로그램-helloc)
  - [1.2 확인 질문](#12-확인-질문)
- [1.3 프로세스 만들어 보기 — forkbench.c](#13-프로세스-만들어-보기-forkbenchc)
  - [1.3 확인 질문](#13-확인-질문)
- [1.4 파일 입출력 — filetest.c](#14-파일-입출력-filetestc)
  - [1.4 관찰 네 가지](#14-관찰-네-가지)
  - [1.4 확인 질문](#14-확인-질문)
- [1.5 커밋하고 올리기](#15-커밋하고-올리기)
- [정리 — 오늘 관찰한 것](#정리-오늘-관찰한-것)
- [제출물](#제출물)
- [자주 막히는 곳](#자주-막히는-곳)

## 개요

이번 실습의 목적은 두 가지입니다. 하나는 소스 트리에서 무엇이 어디에 있는지 감을 잡는 것이고, 다른 하나는 사용자 프로그램을 직접 만들어 xv6 위에서 돌려 보는 것입니다.

커널 코드는 건드리지 않습니다. 대신 `user/` 아래에 프로그램 세 개를 만들고, 실행 결과에서 몇 가지를 관찰합니다.

| 단계  | 무엇을                   | 시간  |
| --- | --------------------- | --- |
| 1.1 | 소스 구조 둘러보기 — 여섯 가지 관찰 | 25분 |
| 1.2 | `hello.c`             | 10분 |
| 1.3 | `forkbench.c`         | 25분 |
| 1.4 | `filetest.c`          | 25분 |
| 1.5 | 커밋하고 올리기              | 10분 |

> ⚠️ **먼저 확인하세요** make qemu 로 $ 프롬프트가 뜨고 Ctrl-A 다음 X 로 빠져나올 수 있어야 합니다. 아직이라면 setup.md 를 먼저 마치세요.

---

## 1.1 소스 구조 둘러보기

프로그램을 만들기 전에, 앞으로 한 학기 동안 볼 코드가 어떻게 생겼는지 훑어봅니다. 여섯 가지를 관찰하고, 답을 적어 두었다가 제출하세요.

#### 관찰 ① — 디렉터리는 몇 개인가

```
cd ~/xv6
ls
```

디렉터리 셋과 파일 몇 개가 전부입니다. `kernel/` 은 커널, `user/` 는 사용자 프로그램, `mkfs/` 는 디스크 이미지를 만드는 도구입니다.

#### 관찰 ② — 커널은 몇 줄인가

```
wc -l kernel/*.c | tail -1
wc -l kernel/*.h | tail -1
```

> ℹ️ **직접 세어 보세요** .c 파일만 합치면 5,000줄대가 나옵니다. 리눅스 커널은 1,000만 줄이 넘습니다. 한 학기에 전부 읽을 수 있는 크기라는 것이 xv6 를 쓰는 이유입니다.

#### 관찰 ③ — 딸려 오는 프로그램은 몇 개인가

```
ls user/*.c
ls user/*.c | wc -l
```

셸(`sh.c`), 기본 명령어들(`cat · echo · ls · grep · wc`), 커널을 시험하는 프로그램들(`usertests.c` 등), 그리고 라이브러리 대용 파일들이 섞여 있습니다.

#### 관찰 ④ — 가장 짧은 프로그램 읽어 보기

`user/echo.c` 는 19줄입니다. 지금 열어서 끝까지 읽어 보세요.

```
cat user/echo.c
```

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc; i++) {
    write(1, argv[i], strlen(argv[i]));
    if (i + 1 < argc) {
      write(1, " ", 1);
    } else {
      write(1, "\n", 1);
    }
  }
  exit(0);
}
```

이 19줄 안에 xv6 프로그램의 뼈대가 전부 들어 있습니다.

| 보이는 것                          | 무엇인가                                      |
| ------------------------------ | ----------------------------------------- |
| `#include "user/user.h"`       | 시스템콜 목록. stdio.h 같은 표준 헤더는 없습니다           |
| `main(int argc, char *argv[])` | 명령줄 인자. 커널이 스택에 밀어 넣고 레지스터로 넘겨 줍니다        |
| `write(1, ...)`                | printf 가 아니라 시스템콜을 직접 부릅니다. 1 은 표준 출력     |
| `exit(0)`                      | return 이 아닙니다. 프로세스를 끝내는 시스템콜이고 돌아오지 않습니다 |

> ℹ️ **write 의 첫 인자가 1 인 이유** 0 · 1 · 2 는 각각 표준 입력 · 표준 출력 · 표준 오류로 이미 열려 있습니다. 1.4 에서 직접 확인합니다.

#### 관찰 ⑤ — 그 프로그램이 어떻게 빌드에 들어가는가

```
grep -n "echo" Makefile
```

Makefile 의 `UPROGS` 목록에 `$U/_echo` 라는 줄이 있습니다. **이 목록에 없으면 파일을 만들어도 xv6 안에서 실행할 수 없습니다.** 다음 단계에서 바로 겪게 됩니다.

#### 관찰 ⑥ — printf 는 어디서 오는가

echo.c 에는 `stdio.h` 가 없습니다. 그런데 다른 프로그램들은 `printf` 를 잘 씁니다. 어디서 오는 걸까요?

```
grep -n "printf" user/user.h      # 선언은 어디에
wc -l user/printf.c               # 구현은 몇 줄인가
grep -n "write" user/printf.c     # 결국 무엇을 부르는가
grep -n "ULIB" Makefile           # 어떻게 프로그램에 붙는가
```

선언은 `user/user.h` 에 있습니다. 이 파일 하나가 표준 C 의 `stdio.h` · `string.h` · `stdlib.h` 를 합쳐 놓은 역할을 합니다. 50줄뿐이고, 세 덩어리로 나뉘어 있습니다.

```
// system calls        ← 커널이 제공
int write(int, const void *, int);
int open(const char *, int);
...

// ulib.c              ← 여기서부터는 사용자 코드
uint strlen(const char *);
void *memset(void *, int, uint);

// printf.c
void printf(const char *, ...);

// umalloc.c
void *malloc(uint);
```

구현은 세 파일에 나뉘어 있습니다. 셋을 합쳐 387줄이 xv6 의 라이브러리 전부입니다.

| 파일               | 줄 수 | 무엇이 들어 있나                                               |
| ---------------- | --- | ------------------------------------------------------- |
| `user/ulib.c`    | 162 | strlen · strcpy · memset · memmove · atoi · gets · stat |
| `user/printf.c`  | 135 | printf · fprintf · putc                                 |
| `user/umalloc.c` | 90  | malloc · free                                           |

평소 C 프로그램이 쓰는 그 라이브러리를 **C 표준 라이브러리(C Standard Library)**, 줄여서 **libc** 라고 부릅니다. 리눅스에서 흔히 쓰는 구현은 **glibc**(GNU C Library) 입니다.

**xv6 에서 libc 에 해당하는 것이 바로 이 세 파일** — `user/ulib.c` · `user/printf.c` · `user/umalloc.c` 입니다. 여기에 시스템콜 진입 코드인 `user/usys.S` 를 더한 넷이 모든 프로그램에 붙습니다.

|         | 표준 C — libc                              | xv6                            |
| ------- | ---------------------------------------- | ------------------------------ |
| 이름      | C 표준 라이브러리 · libc (리눅스는 대개 glibc)        | 따로 이름이 없습니다. Makefile 의 `ULIB` |
| 선언 (헤더) | `stdio.h` · `string.h` · `stdlib.h` … 여럿 | `user/user.h` 하나               |
| 구현      | 한 덩어리 (glibc 는 수백만 줄)                    | 세 파일, 합쳐 387줄                  |
| 링크      | 대개 `libc.so` 를 여럿이 공유 (동적)               | 프로그램마다 통째로 복사 (정적)             |
| 담는 범위   | 파일 · 문자열 · 수학 · 시간 · 로케일 · 스레드 …         | 파일 · 문자열 · 메모리 할당만             |

> ℹ️ **printf 는 libc 안에 있습니다** 표준 C 에서 printf 는 stdio.h 로 선언되지만 구현은 libc 한 덩어리에 들어 있습니다. xv6 가 printf.c 를 따로 둔 것은 표준의 구분을 따른 게 아니라, 성격이 다른 코드를 파일로 나눈 것뿐입니다. 링크할 때는 셋이 함께 붙습니다.

Makefile 104~107행을 보면 그 네 파일이 **모든 사용자 프로그램에 통째로 붙는다** 는 것을 알 수 있습니다.

```
ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB) $U/user.ld
    $(LD) $(LDFLAGS) -T $U/user.ld -o $@ $< $(ULIB)
```

공유 라이브러리가 없습니다. `echo` 도 `cat` 도 각자 자기 몫의 `printf` 코드를 포함하고 있습니다.

> ⚠️ **그래서 printf 는 시스템콜이 아닙니다** user/printf.c 안에서 형식 문자열을 해석한 뒤 12행의 write(fd, &c, 1) 로 한 글자씩 내보냅니다. 시스템콜은 write 하나뿐이고, printf 는 그것을 감싼 사용자 코드입니다. echo.c 가 write 를 직접 쓴 것은 형식 지정이 필요 없어 한 겹을 건너뛴 것입니다.

### 1.1 제출할 답

| #   | 질문                                                       | 답   |
| --- | -------------------------------------------------------- | --- |
| 1   | kernel/*.c 는 몇 줄입니까?                                     |     |
| 2   | user/*.c 는 몇 개입니까?                                       |     |
| 3   | echo.c 에서 exit(0) 을 지우면 무슨 일이 벌어질까요? (예상만)               |     |
| 4   | echo.c 에 printf 가 없는데도 화면에 글자가 나옵니다. 어떻게 그럴까요?           |     |
| 5   | printf 는 시스템콜입니까? 아니라면 무엇입니까?                            |     |
| 6   | 모든 프로그램이 printf 코드를 각자 품고 있습니다. 무엇이 낭비되고, 대신 무엇이 단순해집니까? |     |

---

## 1.2 첫 프로그램 — hello.c

내용은 단순하지만 목적이 분명합니다. **파일을 만드는 것만으로는 부족하고 빌드 목록에 등록해야 한다**는 것을 몸으로 익힙니다. 앞으로 만들 모든 프로그램에서 반복됩니다.

#### Step 0 — 작업할 브랜치 만들기

지금까지는 읽기만 했습니다. 이제부터 파일을 만들고 `Makefile` 을 고칩니다. `riscv` 브랜치는 손대지 않고 그대로 두기 위해, 작업할 브랜치를 따로 만듭니다.

```
git branch                       # 지금 riscv 에 있는지 확인
git checkout -b lab1-userprog    # 브랜치를 만들고 그리로 이동
git branch                       # * lab1-userprog 로 바뀌었는지 확인
```

> ⚠️ **왜 브랜치를 나누나** `riscv` 는 언제든 돌아올 수 있는 깨끗한 출발점입니다. 실습에서 무엇을 망가뜨려도 `git checkout riscv` 로 원래 상태를 볼 수 있습니다. 주제마다 새 브랜치를 만드는 것이 이 과목의 규칙입니다 — [utils.md](utils.md) 참고.

#### Step 1 — 파일 만들기  `(user/hello.c)`

```
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
```

#### Step 2 — 등록하지 않고 먼저 실행해 보기

일부러 한 번 실패해 봅니다.

```
$ make qemu
$ hello
exec hello failed
```

> ⚠️ **왜 실패하나** fs.img 에 그 프로그램이 들어 있지 않기 때문입니다. 파일을 만들었다고 자동으로 들어가지 않습니다. Makefile 의 UPROGS 목록이 “디스크에 넣을 프로그램 명단” 입니다.

#### Step 3 — 빌드 목록에 등록하기  `(Makefile)`

```
# Makefile 의 UPROGS 목록에 한 줄 추가
UPROGS=\
    $U/_cat\
    # ... 기존 항목들 ...
    $U/_sync\
    $U/_hello\        # 이 줄을 추가
```

> ⚠️ **줄 끝의 역슬래시** 각 줄 끝에 \ 가 있어야 다음 줄로 이어집니다. 마지막 항목에는 빼먹기 쉬우니 앞뒤 줄과 모양을 맞추세요.

#### Step 4 — 다시 빌드하고 실행

```
$ make qemu
$ hello
hello, xv6!
argc = 1
  argv[0] = hello

$ hello a b c
hello, xv6!
argc = 4
  argv[0] = hello
  argv[1] = a
  argv[2] = b
  argv[3] = c
```

> ℹ️ **argv[0] 은 프로그램 이름입니다** 셸이 명령줄을 잘라 argv 배열을 만들고, exec 이 그것을 새 프로세스의 스택에 밀어 넣습니다. 그래서 프로그램이 자기 이름을 알 수 있습니다.

### 1.2 확인 질문

- make clean 후 make qemu 를 하면 시간이 더 걸립니다. 왜 그럴까요?
- hello.c 만 고치고 make qemu 를 하면 전체가 다시 컴파일됩니까, 그 파일만 됩니까?
- UPROGS 에서 _hello 를 지우고 다시 빌드하면 어떻게 됩니까?

---

## 1.3 프로세스 만들어 보기 — forkbench.c

`fork()` 로 자식 프로세스를 여럿 만들고, 각자 몇 줄씩 출력하게 합니다. **출력이 섞이는 모습을 관찰하는 것** 이 목적입니다.

#### Step 1 — 프로그램 작성  `(user/forkbench.c)`

```
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

  for (int i = 0; i < NCHILD; i++)        // 자식 다섯을 거둔다
    wait(0);

  printf("parent: all children done\n");
  exit(0);
}
```

Makefile 의 UPROGS 에 `$U/_forkbench` 를 추가하고 빌드합니다.

#### Step 2 — 여러 번 실행해 보기

같은 프로그램을 세 번 이상 실행하고 출력 순서를 비교하세요.

```
$ forkbench
$ forkbench
$ forkbench
```

> ⚠️ **여기가 핵심입니다** 실행할 때마다 순서가 달라집니다. 프로그램은 바뀌지 않았는데 결과가 달라집니다. 무엇이 순서를 정하고 있을까요?

#### Step 3 — 코어 수를 바꿔 보기

xv6 를 종료하고 코어 수를 달리해 다시 띄웁니다.

```
# 코어 하나로
$ make CPUS=1 qemu
$ forkbench

# 코어 여덟 개로
$ make CPUS=8 qemu
$ forkbench
```

두 경우의 출력을 나란히 놓고 비교하세요. 섞이는 양상이 다릅니다.

| 설정     | 관찰되는 경향                                            |
| ------ | -------------------------------------------------- |
| CPUS=1 | 한 자식의 세 줄이 붙어 나오는 편입니다. 진짜 동시 실행이 아니라 번갈아 도는 것이니까요 |
| CPUS=8 | 여러 자식의 줄이 더 잘게 섞입니다. 실제로 동시에 돌기 때문입니다              |

> ℹ️ **“경향”이라고 쓴 이유** 매번 그렇지는 않습니다. 확실히 정해진 것이 아니라 그때그때 다릅니다 — 그 사실 자체가 이 실습에서 볼 것입니다.

#### Step 4 — 한 가지 실험 더

`wait(0)` 를 부르는 루프를 통째로 지우고 다시 실행해 보세요.

```
  // for (int i = 0; i < NCHILD; i++)
  //   wait(0);
```

> ⚠️ **무슨 일이 벌어졌나** 부모가 먼저 끝나고 셸 프롬프트가 돌아온 뒤에도 자식의 출력이 이어서 나올 수 있습니다. 부모가 자식을 기다리지 않았기 때문입니다.

### 1.3 확인 질문

- 자식들의 출력 순서가 매번 다릅니다. 무엇이 그 순서를 정하고 있습니까?
- `fork()` 가 부모에게는 자식의 pid 를, 자식에게는 0 을 돌려줍니다. 왜 굳이 다른 값을 줄까요?
- 변수 `i` 는 자식마다 값이 다릅니다. 자식이 부모의 `i` 를 “보고 있는” 것입니까, “복사해 간” 것입니까? 어떻게 확인할 수 있을까요?
- `wait(0)` 를 지웠을 때 자식들은 어떻게 되었습니까? 누가 그들을 거두었을까요?

---

## 1.4 파일 입출력 — filetest.c

파일을 만들고, 쓰고, 다시 열어 읽습니다. 그 과정에서 파일 디스크립터가 어떤 규칙으로 배정되는지 직접 확인합니다.

#### Step 1 — 프로그램 작성  `(user/filetest.c)`

```
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
```

> ⚠️ **헤더가 하나 더 필요합니다** O_CREATE 같은 플래그는 kernel/fcntl.h 에, struct stat 은 kernel/stat.h 에 있습니다. 빠뜨리면 컴파일이 실패합니다.

#### Step 2 — 실행하고 관찰하기

```
$ filetest
write fd = 3
wrote 10 bytes
read  fd = 3
read 10 bytes: hello xv6
size = 10 bytes, inode = ...
```

### 1.4 관찰 네 가지

#### 관찰 ① — fd 가 3 이다

0 · 1 · 2 는 이미 콘솔이 쓰고 있습니다. 그래서 새로 여는 파일은 3 번을 받습니다. **echo.c 가 write(1, ...) 을 쓴 이유**가 여기서 설명됩니다.

#### 관찰 ② — 닫았다 다시 열어도 또 3 이다

③번 줄에서 다시 3 이 나옵니다. `close(fd)` 로 3 번이 비었고, `open` 이 다시 **쓰이지 않는 번호 중 가장 작은 것** 을 주었기 때문입니다.

> ℹ️ **이 규칙 하나가 리다이렉션의 전부입니다** 나중에 셸이 close(1) 을 하고 곧바로 open 을 하면, 그 파일이 1 번을 받습니다. 그러면 프로그램의 표준 출력이 파일로 갑니다. 프로그램은 한 줄도 고치지 않고요.

직접 확인해 보세요 — `close(fd)` 한 줄을 주석 처리하고 다시 실행하면 두 번째 fd 는 무엇이 나옵니까?

#### 관찰 ③ — write 가 쓴 바이트 수를 돌려준다

요청한 만큼 다 쓰지 못할 수도 있기 때문에 반환값이 있습니다. 시스템콜이 왜 대부분 정수를 돌려주는지 생각해 보세요.

#### 관찰 ④ — 프로그램이 끝나도 파일이 남는다

xv6 셸에서 직접 확인합니다.

```
$ ls
mydata.txt     2 25 10

$ cat mydata.txt
hello xv6

$ cat < mydata.txt
hello xv6
```

> ℹ️ **마지막 줄이 재미있습니다** cat 은 파일 이름을 받아도 되고 리다이렉션으로 받아도 됩니다. 그런데 cat 의 코드는 그 차이를 모릅니다. read(fd, ...) 를 부를 뿐이고, 그 fd 가 무엇에 연결되어 있는지는 커널만 압니다.

### 1.4 확인 질문

- ③번에서 왜 또 3 이 나옵니까? `close(fd)` 를 빼면 무엇이 나옵니까?
- write 의 세 번째 인자를 10 대신 5 로 바꾸면 파일에 무엇이 들어갑니까?
- `O_CREATE` 없이 존재하지 않는 파일을 열면 fd 가 몇입니까? 그 값의 의미는?
- 프로그램을 두 번 실행하면 파일 내용이 어떻게 됩니까? 이어 붙습니까, 덮어씁니까? 짧은 내용으로 바꿔 다시 실험해 보세요.

> ⚠️ **마지막 질문에 대한 힌트** kernel/fcntl.h 를 열어 보면 플래그가 다섯 개 있습니다. 우리가 쓰지 않은 것 중에 답이 있습니다.

---

## 1.5 커밋하고 올리기

프로그램 셋을 다 만들었으니 한 번 저장합니다. `utils.md` 의 작업 흐름을 그대로 한 바퀴 도는 것입니다.

```
git status                       # 무엇이 바뀌었나 (확인용)
  modified:   Makefile
  untracked:  user/hello.c
              user/forkbench.c
              user/filetest.c

git add Makefile
git add user/hello.c user/forkbench.c user/filetest.c  # 커밋 대상 지정
(참고) git add .     # 현재 디렉터로내 새 파일이나 수정된 파일 한꺼번에 지

git commit -m "lab1: hello, forkbench, filetest 추가"

git log --oneline                # 커밋이 잘 남았는지 (선택)
git push -u origin lab1-userprog # 내 github 저장소에 올리기
```

> ℹ️ **필수는 세 줄입니다** `add` → `commit` → `push` 가 한 묶음의 처리 루틴입니다. `status` · `diff` · `log` 는 확인용이라 건너뛰어도 결과는 같습니다. 다만 `commit` 전에 `diff` 로 한 번 훑어보는 습관은 들여 두세요.



브라우저에서 내 저장소를 열어 브랜치 목록에 `lab1-userprog` 이 보이면 성공입니다.

> ⚠️ **Permission denied 가 나온다면** 수업 저장소를 그대로 clone 한 경우입니다([lab0-setup.md](lab0-setup.md) 방법 B). 그 저장소에는 쓰기 권한이 없어 push 가 거부됩니다. 내 계정에 저장소를 하나 만들어 원격지로 연결하면 됩니다 — [utils.md](utils.md) 참고.

> ℹ️ **push 는 선택입니다** 하지 않아도 실습에는 지장이 없습니다. 커밋은 이미 내 컴퓨터에 남아 있습니다. 다만 노트북이 고장 나면 잃으므로, 한 랩을 마칠 때마다 한 번씩 올려 두기를 권합니다.

## 정리 — 오늘 관찰한 것

| 관찰                 | 어디서   | 언제 다시 만나나                                     |
| ------------------ | ----- | --------------------------------------------- |
| 출력 순서가 매번 다르다      | 1.3   | 동기화 — 왜 순서를 보장할 수 없는가                         |
| 코어 수를 바꾸면 양상이 달라진다 | 1.3   | 스케줄링 — 여러 코어가 배열을 훑는 방식                       |
| 자식이 부모의 변수를 복사해 간다 | 1.3   | 메모리 관리 — fork 가 주소 공간을 복사하는 방법                |
| fd 는 3 부터 시작한다     | 1.4   | 리다이렉션 — close 하고 open 하면 끝난다                  |
| cat 은 상대가 무엇인지 모른다 | 1.4   | 파일 시스템 — 하나의 인터페이스로 여러 대상을                    |
| UPROGS 에 등록해야 실행된다 | 1.2   | 시스템콜 추가 — 고쳐야 할 파일 여섯 곳 중 하나                  |
| printf 는 사용자 코드다   | 1.1 ⑥ | 메모리 관리 — malloc 도 커널이 아니라 user/umalloc.c 에 있다 |
| 커널은 이 라이브러리를 못 쓴다  | 1.1 ⑥ | 시동 절차 — 커널의 printf 는 kernel/printk.c 에 따로 있다  |

오늘 만든 프로그램 세 개는 앞으로도 그대로 씁니다. 지우지 마세요.

## 제출물

1. 1.1 의 답 네 개
2. `user/hello.c` · `user/forkbench.c` · `user/filetest.c`
3. forkbench 를 CPUS=1 과 CPUS=8 로 실행한 출력 (각 1회, 화면 복사)
4. 1.3 과 1.4 의 확인 질문에 대한 답

`lab1-userprog` 브랜치를 제출하거나 `git diff riscv...` 결과를 함께 내면 됩니다. 자세한 것은 [utils.md](utils.md) 를 보세요.

## 자주 막히는 곳

| 증상                      | 원인과 해결                                             |
| ----------------------- | -------------------------------------------------- |
| exec hello failed       | Makefile 의 UPROGS 에 등록하지 않았습니다                     |
| Makefile 수정 후 빌드가 깨진다   | 줄 끝의 역슬래시(\)를 확인하세요. 앞뒤 줄과 모양이 같아야 합니다             |
| O_CREATE 가 정의되지 않았다는 오류 | #include "kernel/fcntl.h" 를 빠뜨렸습니다                 |
| struct stat 이 없다는 오류    | #include "kernel/stat.h" 를 빠뜨렸습니다                  |
| size 출력이 이상하다           | st.size 는 uint64 입니다. %d 가 아니라 %ld 를 쓰세요           |
| forkbench 가 멈춘 것 같다     | wait 하는 자식 수가 만든 수보다 많으면 영원히 기다립니다. 두 수가 같은지 확인하세요 |
| Ctrl-C 로 qemu 가 안 꺼진다   | Ctrl-A 를 누르고 손을 뗀 다음 X 입니다                         |
