# xv6 실습 도구

**git · 소스코드 읽기 · gdb**

*필요할 때 찾아 보는 참고 문서입니다*

> 환경 설정이 아직이라면 [lab0-setup.md](lab0-setup.md) 를 먼저 보세요.

## 목차

- [1. git 사용법 (Using git)](#1-git-사용법-using-git)
  - [1.1 add · commit · push 의 차이](#11-add-commit-push-의-차이)
  - [1.2 브랜치 생성과 이동 (Branching)](#12-브랜치-생성과-이동-branching)
  - [1.3 작업 흐름 (Basic Workflow)](#13-작업-흐름-basic-workflow)
  - [1.4 원격 저장소에 올리기 (Pushing)](#14-원격-저장소에-올리기-pushing)
  - [1.5 수업 저장소가 갱신되었을 때](#15-수업-저장소가-갱신되었을-때)
  - [1.6 변경 되돌리기 (Undoing Changes)](#16-변경-되돌리기-undoing-changes)
  - [1.7 명령 요약](#17-명령-요약)
- [2. 소스코드 읽기 (Reading the Source)](#2-소스코드-읽기-reading-the-source)
  - [2.1 시작점 — kernel/defs.h](#21-시작점-kerneldefsh)
  - [2.2 도구 선택](#22-도구-선택)
  - [2.3 GUI 에디터 — VS Code](#23-gui-에디터-vs-code)
  - [2.4 터미널 도구 — ctags 와 cscope](#24-터미널-도구-ctags-와-cscope)
  - [2.5 검색 — grep](#25-검색-grep)
  - [2.6 빌드 산출물 — kernel.asm 과 kernel.sym](#26-빌드-산출물-kernelasm-과-kernelsym)
  - [2.7 읽기 요령](#27-읽기-요령)
- [3. gdb 디버깅 (Kernel Debugging)](#3-gdb-디버깅-kernel-debugging)
  - [3.1 초기 설정](#31-초기-설정)
  - [3.2 기본 사용법](#32-기본-사용법)
  - [3.3 주요 명령 (gdb Commands)](#33-주요-명령-gdb-commands)
  - [3.4 상황별 사용법](#34-상황별-사용법)
  - [3.5 화면 분할 — layout src](#35-화면-분할-layout-src)
- [부록 B — 명령 요약](#부록-b-명령-요약)

## 1. git 사용법 (Using git)

*명령을 외우지 않아도 됩니다. 필요할 때 찾아 보고 익숙해지면 외워집니다.*

### 1.1 add · commit · push 의 차이

| 명령 | 무엇을 하나 | 어디에 남나 |
|---|---|---|
| `git add` | 이번에 저장할 파일을 고른다 | 아직 저장 안 됨 (대기 상태) |
| `git commit` | 고른 것을 하나의 기록으로 저장한다 | 내 컴퓨터 |
| `git push` | 그 기록을 인터넷의 저장소로 복사한다 | GitHub 등 (선택) |

### 1.2 브랜치 생성과 이동 (Branching)

이 저장소의 기본 브랜치 이름은 `riscv` 입니다. 다른 저장소의 `main` 에 해당합니다. 이 브랜치는 손대지 않고 그대로 둡니다.

```
# 지금 어느 브랜치에 있는지 확인 (앞에 * 가 붙은 것이 현재 브랜치)
git branch

# 메모리 관리 실습을 시작할 때
git checkout riscv          # 원본 브랜치로 이동
git checkout -b memory      # (브랜치명 예시)memory 브랜치를 새로 생성하고 그리로 이동

# 다음 주제로 넘어갈 때
git checkout riscv          # 손대지 않은 원본으로 복귀
git checkout -b process     # (브랜치명 예시)process 브랜치를 새로 생성

# 예전에 만들어 둔 브랜치로 돌아가기 (-b 를 붙이지 않는다)
git checkout memory
```

> ⚠️ **-b 의 유무** -b 가 있으면 “새로 만들고 그리로 이동”, 없으면 “이미 있는 브랜치로 이동만”입니다. 이미 있는 이름에 -b 를 붙이면 오류가 납니다.

> ⚠️ **브랜치 이동 전 확인** 아직 커밋하지 않은 변경이 있으면 git 이 이동을 막습니다. 커밋하거나(1.3), 잠시 치워 두거나(1.6 상황 C), 버리세요(1.6 상황 B).

### 1.3 작업 흐름 (Basic Workflow)

랩 문서의 Step 하나를 마쳤을 때의 실제 절차입니다. `kalloc.c` 와 `defs.h` 두 파일을 고쳤다고 하겠습니다.

```
# ① 무엇이 바뀌었나 — 고친 파일 목록
$ git status
  On branch memory
  Changes not staged for commit:
    modified:   kernel/kalloc.c
    modified:   kernel/defs.h

# ② 어떻게 바뀌었나 — 실제 변경 내용
$ git diff kernel/kalloc.c        # 이 파일만
$ git diff                        # 고친 것 전부
$ git diff riscv...               # 브랜치 분기 이후 내가 고친 것 전부

# ③ 빌드가 통과하는지 확인
$ make qemu

# ④ 저장할 파일을 고르고
$ git add kernel/kalloc.c kernel/defs.h
$ git add -A                      # 또는 바뀐 것 전부

# ⑤ 하나의 기록으로 남긴다
$ git commit -m "lab1 step1: freemem() 추가"

# ⑥ 커밋이 잘 남았는지 확인 (선택)
$ git log --oneline
  a3f1c92 lab1 step1: freemem() 추가
  35b0884 test for nlink overflow
```

> ℹ️ **점이 세 개인 이유** git diff riscv (점 두 개) 는 riscv 브랜치의 현재 상태(current tip) 와 비교합니다. 그래서 내 브랜치를 만든 뒤 riscv 가 갱신되면(예: docs 문서 수정) 그 차이까지 섞여 나옵니다. 점 세 개는 분기 지점(merge base) 과 비교하므로 내가 고친 것만 정확히 보여 줍니다.

> ℹ️ **커밋 시점** 빌드가 통과한 시점마다 하나씩 남기세요. 랩 문서의 Step 하나가 커밋 하나에 해당합니다. 커널 실습은 “어제까지 되던 게 안 된다”가 흔한데, 커밋이 없으면 어디서 망가졌는지 찾을 방법이 없습니다.

### 1.4 원격 저장소에 올리기 (Pushing)

실습 결과를 GitHub 에 남겨 두면 노트북이 고장 나도 코드를 잃지 않습니다. 앞에서 어느 방법으로 코드를 받았느냐에 따라 절차가 다릅니다.

#### 방법 A 로 받은 경우 — 추가 설정 없음

`origin` 이 이미 내 fork(내계정/xv6lab-fall2026)를 가리키고 있습니다.

```
# 현재 브랜치를 처음 올릴 때
git push -u origin memory

# 그 뒤로는 이것만
git push

# 올라갔는지 확인 — 브라우저에서 내 저장소를 열어
# 브랜치 목록에 memory 가 보이면 성공
```

#### 방법 B 로 받은 경우 — 원격 저장소 연결

수업 저장소에는 쓰기 권한이 없으므로 그쪽으로는 push 할 수 없습니다. 내 계정에 빈 저장소를 만들어 원격지로 추가하세요.

```
# ① GitHub 에서 새 저장소를 하나 만든다 (예: xv6-lab, 비어 있는 상태로)

# ② 그 주소를 mine 이라는 이름으로 등록
git remote add mine https://github.com/내계정/xv6-lab.git

# ③ 확인
git remote -v
  mine    https://github.com/내계정/xv6-lab.git (fetch)
  mine    https://github.com/내계정/xv6-lab.git (push)
  origin  https://github.com/bigsys-gnu/xv6lab-fall2026.git (fetch)

# ④ 처음 올릴 때
git push -u mine memory

# ⑤ 그 뒤로는
git push
```

> ⚠️ **인증 — Personal Access Token** GitHub 는 계정 비밀번호 대신 Personal Access Token 을 요구합니다. GitHub → Settings → Developer settings → Personal access tokens 에서 만들어 비밀번호 자리에 넣으세요. 한 번 만들어 두면 계속 씁니다.

> ℹ️ **push 시점** 매번 할 필요는 없습니다. 한 랩을 마쳤을 때나 하루 작업을 끝낼 때 한 번씩이면 충분합니다. push 를 하지 않아도 커밋은 내 컴퓨터에 남아 있으므로 실습에는 지장이 없습니다.

### 1.5 수업 저장소가 갱신되었을 때

학기 중에 수업 저장소의 문서나 실습 브랜치가 갱신될 수 있습니다. **대부분은 내려받을 필요가 없습니다** — 브라우저에서 원본 저장소를 열어 보면 됩니다.

> ℹ️ **가장 간단한 방법 — 그냥 웹에서 보기** docs/ 아래의 문서는 GitHub 웹에서 항상 최신본이 보입니다. 갱신 안내를 받으면 원본 저장소 주소를 열어 확인하세요. 내 저장소에 굳이 받아 올 필요가 없습니다.

#### 그래도 내 fork 에 반영하고 싶다면

GitHub 웹에서 버튼 하나로 됩니다. 명령어도, 원격 저장소 설정도 필요 없습니다.

- ① 브라우저에서 내 fork 저장소를 엽니다.
- ② 브랜치 이름 아래에 `This branch is N commits behind …` 표시가 보입니다.
- ③ **Sync fork → Update branch** 를 누릅니다.
- ④ 내 컴퓨터로 가져옵니다 — 아래 두 줄.

```
git checkout riscv
git pull
```

> ⚠️ **실습 중인 브랜치에는 반영하지 마세요** riscv 브랜치에만 받아 두면 충분합니다. 작업 중인 브랜치에 끌어오면 여러분이 고친 부분과 엉킬 수 있고, git diff riscv... 결과도 지저분해집니다.

### 1.6 변경 되돌리기 (Undoing Changes)

#### 상황 A — 파일 하나를 원래대로

마지막 커밋 상태로 그 파일만 되돌립니다. 다른 파일은 그대로 남습니다. 가장 많이 쓰게 될 명령입니다.

```
git checkout -- kernel/kalloc.c
```

#### 상황 B — 모든 변경 취소

커밋하지 않은 변경을 전부 없앱니다. 되돌릴 수 없으니 실행 전에 한 번 더 생각하세요.

```
git status                 # 무엇이 사라질지 먼저 확인
git checkout -- .
```

#### 상황 C — 작업 임시 보관 (stash)

`git stash` 는 서랍입니다. 고치던 내용을 통째로 서랍에 넣어 두면 작업 디렉터리가 깨끗해지고, 나중에 그대로 꺼낼 수 있습니다.

```
$ git stash                # 고치던 것을 서랍에 넣는다 (작업 디렉터리가 깨끗해짐)
$ git checkout riscv       # 다른 브랜치에 다녀오고
$ git checkout memory      # 원래 브랜치로 돌아와서
$ git stash pop            # 서랍에서 꺼내 되돌린다

# 서랍에 무엇이 들어 있는지 보기
$ git stash list
  stash@{0}: WIP on memory: a3f1c92 lab1 step1: freemem() 추가
```

> ℹ️ **stash 와 commit 의 차이** commit 은 “이 상태를 기록으로 남긴다”이고 stash 는 “잠시 치워 둔다”입니다. 아직 완성되지 않아 커밋하기는 애매한데 지금 당장 치워야 할 때 씁니다.

#### 상황 D — 직전 커밋 취소

바로 앞 커밋을 통째로 지웁니다. 그 커밋에 담긴 변경 내용도 함께 사라집니다.

```
git log --oneline          # 무엇이 사라질지 먼저 확인
git reset --hard HEAD~1
```

> ⚠️ **--hard 는 되돌릴 수 없습니다** 변경 내용을 남기고 커밋만 취소하려면 --hard 대신 --soft 를 쓰세요. git reset --soft HEAD~1 은 커밋을 풀되 고친 내용은 그대로 둡니다.

### 1.7 명령 요약

| 하려는 일 | 명령 |
|---|---|
| 현재 브랜치 확인 | `git branch` |
| 브랜치 생성하고 이동 | `git checkout -b <이름>` |
| 기존 브랜치로 이동 | `git checkout <이름>` |
| 고친 파일 목록 | `git status` |
| 변경 내용 보기 | `git diff` |
| 분기 지점 이후 내가 고친 것 | `git diff riscv...` |
| 저장할 파일 고르기 | `git add -A` |
| 기록으로 남기기 | `git commit -m "메시지"` |
| 커밋 확인 (선택) | `git log --oneline` |
| 내 저장소에 올리기 | `git push -u origin <브랜치>` |
| 파일 하나 되돌리기 | `git checkout -- <파일>` |
| 잠시 치워 두기 / 꺼내기 | `git stash  /  git stash pop` |

---

## 2. 소스코드 읽기 (Reading the Source)

*xv6 는 파일 예순여덟 개, 6,000줄입니다. 손으로 뒤지기에는 많고, 도구를 쓰면 금방입니다.*

### 2.1 시작점 — kernel/defs.h

`kernel/defs.h` 를 먼저 여세요. 커널 안의 모든 함수가 **어느 파일에 있는지 파일별로 묶여** 선언되어 있습니다. 커널의 목차인 셈입니다.

```
# 예: kalloc 이 어디 있나
grep -n "kalloc" kernel/defs.h

  59: // kalloc.c
  60: void*           kalloc(void);
  61: void            kfree(void *);
```

주석으로 파일 이름이 붙어 있으므로, 그 위의 주석만 보면 어느 파일인지 바로 알 수 있습니다.

### 2.2 도구 선택

두 갈래가 있고 우열은 없습니다. 자기 작업 방식에 맞는 쪽 하나만 고르세요.

|   | GUI 에디터 (VS Code) | 터미널 (ctags · cscope) |
|---|---|---|
| 이럴 때 | 내 노트북에서 화면을 보며 작업한다 | vim · emacs 를 쓰거나 SSH 로 서버에 붙는다 |
| 강점 | 진입 장벽이 낮고, 변경 사항을 색으로 본다 | 가볍고 빠르다. “누가 이 함수를 부르나”가 강하다 |
| 한계 | SSH 전용 환경에서는 별도 설정이 필요하다 | 처음 설정과 색인 갱신을 손으로 해야 한다 |
| 현장에서는 | 응용 개발에서 널리 쓰인다 | 리눅스 커널 개발에서 여전히 표준적인 조합이다 |

> ℹ️ **두 방식의 공통 도구** 어느 쪽을 고르든 kernel/defs.h 를 목차로, grep 을 보조로 씁니다. 앞의 5.1 과 뒤의 검색 절은 두 방식 모두에 해당합니다.

### 2.3 GUI 에디터 — VS Code

- **윈도우 · WSL2** — **WSL** 확장을 설치한 뒤, WSL 터미널에서 `code .` 로 여세요.
- **공통** — **C/C++** 확장을 설치하면 정의 이동이 됩니다.
- xv6 는 헤더가 전부 저장소 안에 있어 대개 별도 설정 없이 동작합니다.

> ⚠️ **WSL2 — 소스 위치 주의** WSL2 를 쓴다면 소스는 ~ 안에 두고, VS Code 를 WSL 모드로 여세요. /mnt/c 로 옮기면 빌드가 몇 배로 느려집니다.

| 단축키 | 하는 일 |
|---|---|
| `F12` | 정의로 이동 — walk() 위에서 누르면 vm.c 의 정의로 |
| `Alt + ←` | 뒤로 — 따라간 길을 되돌아온다 |
| `Shift + F12` | 이 함수를 부르는 곳 모두 찾기 |
| `Ctrl + P` | 파일 이름으로 바로 열기 |
| `Ctrl + Shift + F` | 저장소 전체에서 문자열 검색 |

정의 이동이 잘 되지 않으면 컴파일 명령 정보를 만들어 주면 됩니다. 선택 사항이므로 잘 되고 있다면 건너뛰세요.

```
sudo apt install bear
bear -- make            # compile_commands.json 이 생긴다
```

> ℹ️ **설정 범위** F12 가 되면 그것으로 충분합니다. 도구를 완벽하게 맞추느라 한 주를 보내는 학생이 매년 나옵니다.

### 2.4 터미널 도구 — ctags 와 cscope

vim 을 쓰거나 터미널만으로 작업한다면 이 둘 중 하나면 됩니다. 리눅스 커널 개발 현장에서 여전히 널리 쓰이는 조합입니다.

#### ctags — 정의로 이동

```
sudo apt install universal-ctags
cd ~/xv6-riscv
ctags -R kernel user           # tags 파일이 생긴다

# vim 안에서
#   Ctrl-]   커서 아래 이름의 정의로 이동
#   Ctrl-t   되돌아오기
#   :tag kalloc   이름으로 바로 이동
```

#### cscope — 호출 지점 추적

```
sudo apt install cscope
cd ~/xv6-riscv
cscope -Rbq                    # 색인 생성
cscope -d                      # 대화형 화면 열기
```

화면 아래쪽 메뉴에서 원하는 항목에 커서를 두고 이름을 입력하면 됩니다.

| cscope 메뉴 | 무엇을 찾나 |
|---|---|
| Find this C symbol | 이 이름이 나오는 곳 전부 |
| Find this global definition | 정의된 곳 |
| Find functions called by this function | 이 함수가 부르는 함수들 |
| Find functions calling this function | 이 함수를 부르는 곳 — 커널 읽기에 가장 유용 |
| Find files #including this file | 이 헤더를 포함하는 파일들 |

|   | ctags | cscope |
|---|---|---|
| 찾는 방향 | 이름 → 정의 (한 방향) | 정의도, 부르는 곳도 (양방향) |
| 설정 | 간단 | 조금 더 필요 |
| 커널 읽기에는 | 충분할 때가 많다 | “누가 이걸 부르나”를 알 수 있어 더 유용 |

> ⚠️ **색인 갱신** ctags -R kernel user 또는 cscope -Rbq 를 다시 실행합니다. 색인이 낡으면 엉뚱한 줄로 점프합니다.

Makefile 에 `make tags` 도 있습니다. Emacs 용 TAGS 파일을 만듭니다.

### 2.5 검색 — grep

매크로, 구조체 필드, 문자열 상수는 정의 이동보다 검색이 빠릅니다.

```
# 이 이름이 어디에 나오나
grep -rn "kalloc" kernel/

# 몇 군데에서 쓰나
grep -rn "PTE_U" kernel/ | wc -l

# 파일 이름만
grep -rl "spinlock" kernel/

# 더 빠른 검색기 (선택)
sudo apt install ripgrep
rg -n "kalloc" kernel/
```

### 2.6 빌드 산출물 — kernel.asm 과 kernel.sym

make 를 돌릴 때마다 자동으로 생기는 파일 두 개가 디버깅에 아주 유용합니다.

| 파일 | 무엇인가 | 언제 쓰나 |
|---|---|---|
| `kernel/kernel.asm` | 커널 전체의 역어셈블. C 소스와 어셈블리가 나란히 섞여 있다 | panic 이나 usertrap 이 찍은 주소가 어느 함수의 어느 줄인지 찾을 때 |
| `kernel/kernel.sym` | 심볼 이름과 주소만 모은 표 | “이 주소가 대체 뭐지?” 싶을 때 가장 가까운 심볼 찾기 |

```
# 예: usertrap 이 sepc=0x0000000080001f2a 를 찍었다
grep -n "80001f2a" kernel/kernel.asm

# 예: 이 주소 근처의 심볼 찾기
sort kernel/kernel.sym | less
```

> ℹ️ **제출물 제외** .gitignore 에 등록되어 있어 git 이 무시합니다. make clean 으로 지워집니다.

### 2.7 읽기 요령

- 전부 이해하려 하지 마세요. 강의에서 다루는 파일만 열어도 충분합니다.
- 함수 하나를 볼 때 “누가 이걸 부르나”를 먼저 확인하면 맥락이 잡힙니다.
- 주석을 믿으세요. xv6 의 주석은 짧지만 정확합니다.
- 막히면 교재의 해당 절을 먼저 읽고 다시 코드로 돌아오세요.

---

## 3. gdb 디버깅 (Kernel Debugging)

*학기 중반부터 씁니다. 지금은 설정만 해 두고, 필요해질 때 이 장으로 돌아오세요.*

### 3.1 초기 설정

터미널 두 개가 필요합니다. 하나는 QEMU 를, 다른 하나는 gdb 를 띄웁니다.

```
# 터미널 1 — xv6 디렉터리에서
make CPUS=1 qemu-gdb
  *** Now run 'gdb' in another window.

# 터미널 2 — 같은 디렉터리에서
gdb-multiarch kernel/kernel        # Ubuntu · WSL2
riscv64-elf-gdb kernel/kernel      # macOS
```

| 왜 이렇게 | 이유 |
|---|---|
| `CPUS=1` | 코어가 여럿이면 실행이 여기저기로 튀어 따라가기가 매우 어렵습니다 |
| `qemu-gdb` | QEMU 를 멈춘 상태로 띄우고 gdb 의 접속을 기다립니다 |
| `같은 디렉터리` | gdb 가 그 디렉터리의 .gdbinit 을 읽어 접속 설정을 자동으로 가져옵니다 |

`.gdbinit` 은 빌드할 때 자동으로 만들어집니다. 접속 포트는 사용자마다 다르며 `make print-gdbport` 로 확인할 수 있습니다.

#### 첫 실행 시 나오는 오류

```
warning: File "/home/사용자/xv6-riscv/.gdbinit" auto-loading has been declined
To enable execution of this file add
        add-auto-load-safe-path /home/사용자/xv6-riscv/.gdbinit
line to your configuration file "/home/사용자/.config/gdb/gdbinit".
```

gdb 가 친절하게 해결 방법을 알려 줍니다. 그 줄을 그대로 설정 파일에 넣으면 됩니다.

```
mkdir -p ~/.config/gdb
echo "add-auto-load-safe-path $HOME/xv6-riscv/.gdbinit" >> ~/.config/gdb/gdbinit
```

> ⚠️ **경로 확인** 위 명령은 xv6-riscv 를 홈 디렉터리에 받았을 때를 가정합니다. 다른 곳에 받았다면 gdb 가 출력한 메시지의 경로를 그대로 복사해 쓰세요.

### 3.2 기본 사용법

gdb 가 붙으면 커널이 첫 명령어 직전에 멈춰 있습니다. 여기서 중단점을 걸고 `c` 로 실행을 시작합니다.

```
(gdb) b main              # main 함수에 중단점
(gdb) c                   # 실행 (Continue)

Breakpoint 1, main () at kernel/main.c:12
12      {
(gdb) n                   # 한 줄 실행
(gdb) p started           # 변수 값 출력
$1 = 0
```

### 3.3 주요 명령 (gdb Commands)

| 명령 | 줄임 | 무엇을 하나 |
|---|---|---|
| `break <위치>` | `b` | 중단점 설정 — b main · b kalloc · b vm.c:100 |
| `continue` | `c` | 다음 중단점까지 계속 실행 |
| `next` | `n` | 한 줄 실행 — 함수 안으로 들어가지 않음 |
| `step` | `s` | 한 줄 실행 — 함수 안으로 들어감 |
| `finish` | `fin` | 현재 함수가 끝날 때까지 실행 |
| `backtrace` | `bt` | 여기까지 어떤 함수를 거쳐 왔는지 (호출 스택) |
| `print <식>` | `p` | 변수나 식의 값 — p sz · p/x pte · p *p |
| `x/<형식> <주소>` | `—` | 메모리를 직접 들여다보기 |
| `info registers` | `i r` | 레지스터 전부 — i r satp 처럼 하나만도 가능 |
| `info breakpoints` | `i b` | 걸어 둔 중단점 목록 |
| `delete <번호>` | `d` | 중단점 삭제 |
| `watch <변수>` | `—` | 그 값이 바뀌는 순간 멈춤 |
| `layout src` | `—` | 소스를 보면서 디버깅 (화면 분할) |
| `quit` | `q` | 종료 |

> ℹ️ **엔터로 직전 명령 반복** n 을 한 번 친 뒤에는 엔터만 눌러도 계속 한 줄씩 진행합니다. 단계 실행할 때 편합니다.

### 3.4 상황별 사용법

#### 상황 1 — 시스템콜 호출 확인

```
(gdb) b sys_freemem
(gdb) c

# 터미널 1 의 xv6 에서 freetest 를 실행하면 여기서 멈춘다
(gdb) bt                  # 누가 나를 불렀나
#0  sys_freemem () at kernel/sysproc.c:110
#1  syscall () at kernel/syscall.c:140
#2  usertrap () at kernel/trap.c:67
```

시스템콜이 아예 불리지 않는다면 등록이 빠진 것입니다. 랩 문서의 변경 요약 표로 돌아가 확인하세요.

#### 상황 2 — 페이지 테이블 내용 확인

`x` 명령으로 메모리를 덤프합니다. `x/8xg` 는 “8개를, 16진수로, 8바이트 단위로” 라는 뜻입니다.

```
(gdb) b walk
(gdb) c
(gdb) p/x pagetable                 # 페이지 테이블의 주소
$1 = 0x87f52000
(gdb) x/8xg 0x87f52000              # 앞 8칸을 들여다본다
0x87f52000:  0x0000000021fd3801  0x0000000000000000
0x87f52010:  0x0000000000000000  0x0000000000000000

(gdb) p/x (pagetable[255] >> 10) << 12   # PTE2PA 를 손으로
```

#### 상황 3 — panic 원인 추적

```
(gdb) b panic
(gdb) c

Breakpoint 1, panic (s=0x...) at kernel/printk.c:120
(gdb) p s                 # panic 메시지 확인
$1 = 0x8000a3c8 "acquire"
(gdb) bt                  # 어디서 왔는지 거슬러 올라간다
```

> ℹ️ **gdb 없이 확인하기** panic 이나 usertrap 이 sepc=0x... 를 찍었다면, kernel/kernel.asm 에서 그 주소를 검색해 보세요. 어느 함수의 어느 줄인지 바로 나옵니다. gdb 를 띄우는 것보다 빠를 때가 많습니다.

#### 상황 4 — 값 변경 시점 감시 (watch)

```
(gdb) b kinit
(gdb) c
(gdb) watch kmem.freelist          # 이 값이 바뀌면 멈춘다
(gdb) c

Hardware watchpoint 2: kmem.freelist
Old value = (struct run *) 0x0
New value = (struct run *) 0x87f4e000
```

#### 상황 5 — 사용자 프로그램 디버깅

기본 설정은 커널의 심볼만 읽습니다. 사용자 프로그램을 따라가려면 그 프로그램의 심볼을 추가로 읽혀야 합니다.

```
(gdb) file user/_freetest      # 심볼을 사용자 프로그램 것으로 교체
(gdb) b main
(gdb) c
```

> ⚠️ **주의** file 명령은 심볼을 통째로 바꿉니다. 커널 쪽으로 돌아가려면 file kernel/kernel 로 다시 읽혀야 합니다.

### 3.5 화면 분할 — layout src

`layout src` 를 치면 위쪽에 소스, 아래쪽에 명령창이 나옵니다. 지금 어느 줄에 있는지 눈으로 보면서 진행할 수 있습니다.

```
(gdb) layout src
(gdb) refresh              # 화면이 깨졌을 때
(gdb) tui disable          # 원래 화면으로
```

## 부록 B — 명령 요약

| 하려는 일 | 명령 |
|---|---|
| 빌드하고 실행 | `make qemu` |
| 코어 수 바꿔 실행 | `make CPUS=1 qemu` |
| 빌드 결과 지우기 | `make clean` |
| xv6 에서 빠져나오기 | `Ctrl-A  다음  X` |
| 현재 브랜치 보기 | `git branch` |
| 새 브랜치 만들기 | `git checkout -b <이름>` |
| 무엇을 고쳤는지 보기 | `git diff riscv...` |
| 기준 커밋 확인 | `git log -1 --format=%H` |

> ℹ️ **문의 기준** 환경 구성은 이 과목에서 배울 내용이 아닙니다. 30분 이상 진전이 없으면 오류 메시지 전체를 캡처해 조교에게 문의하세요.

