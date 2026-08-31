# xv6 실습 환경 설정

**Ubuntu 24.04 LTS · WSL2 · macOS**

> 설정을 마친 뒤에는 [utils.md](utils.md) 에서 git · 소스 읽기 · gdb 를 참고하세요.

## 목차

- [개요](#개요)
- [1. Ubuntu 24.04 LTS](#1-ubuntu-2404-lts)
  - [1.9 설치 확인 체크리스트](#19-설치-확인-체크리스트)
- [2. Windows + WSL2](#2-windows-wsl2)
- [3. macOS](#3-macos)
- [부록 A — 오류와 해결](#부록-a-오류와-해결)

## 개요

이 문서를 따라가면 xv6 를 빌드해 실행하고, 주제별 브랜치를 만들어 실습을 시작할 수 있는 상태가 됩니다. 환경 구성만이라면 30분에서 1시간이면 충분합니다. git · 소스 읽기 · gdb 는 별도 문서 utils.md 에 있습니다 — 필요할 때 찾아 보세요.

| 환경 | 이 문서에서 | 비고 |
|---|---|---|
| Ubuntu 24.04 LTS | 1장에서 명령어 단위로 안내 | 권장 — 자료가 가장 많고 문제가 가장 적습니다 |
| Windows + WSL2 | 2장에서 요약 안내 | 그 안에 Ubuntu 24.04 를 깔면 1장과 동일합니다 |
| macOS | 3장에서 요약 안내 | Intel · Apple Silicon 모두 가능합니다 |

> ⚠️ **문서의 검증 범위** 1장(Ubuntu 24.04)은 실제로 확인한 절차입니다. 2장(WSL2)과 3장(macOS)은 방향을 잡기 위한 요약이며, 윈도우 버전 · macOS 버전 · Homebrew 상태에 따라 명령이나 패키지 이름이 달라질 수 있습니다. 이 두 환경을 쓰신다면 구체적인 사항은 각자 환경에서 직접 확인하며 진행하세요. 막히면 조교에게 문의하되, 해결이 오래 걸릴 것 같으면 1장의 Ubuntu 환경으로 옮기는 편이 빠릅니다.

> ℹ️ **공통 원칙** 각 단계가 끝날 때마다 다음으로 넘어가기 전에 결과를 확인하세요. 마지막에 몰아서 확인하면 어디서 틀렸는지 찾기가 훨씬 어렵습니다.

---

## 1. Ubuntu 24.04 LTS

*네이티브 설치, 가상 머신, WSL2 안의 Ubuntu — 어느 쪽이든 이 장의 내용은 같습니다.*

#### Step 1 — 패키지 설치

필요한 것은 네 가지입니다. RISC-V 용 컴파일러, RISC-V 용 binutils, QEMU 에뮬레이터, 그리고 디버거입니다.

```
sudo apt update
sudo apt install -y git build-essential \
    gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu \
    qemu-system-misc gdb-multiarch
```

| 패키지 | 무엇인가 |
|---|---|
| `gcc-riscv64-linux-gnu` | RISC-V 용 C 컴파일러 — 내 x86 노트북에서 RISC-V 코드를 만든다 (교차 컴파일) |
| `binutils-riscv64-linux-gnu` | 링커 · objdump 등. 커널 실행 파일을 만들어 낸다 |
| `qemu-system-misc` | RISC-V 기계를 흉내 내는 에뮬레이터가 이 안에 들어 있다 |
| `gdb-multiarch` | 여러 아키텍처를 다룰 수 있는 디버거. 학기 중반부터 씁니다 |
| `build-essential` | make 와 기본 빌드 도구 |

#### Step 2 — 설치 확인

```
riscv64-linux-gnu-gcc --version
qemu-system-riscv64 --version
```

두 명령이 모두 버전을 출력하면 성공입니다. `command not found` 가 나오면 Step 1 이 제대로 되지 않은 것입니다.

> ℹ️ **QEMU 버전 요구사항** xv6 의 Makefile 이 QEMU 7.2 이상을 요구합니다. Ubuntu 24.04 는 8.x 를 제공하므로 문제없습니다. 오래된 배포판을 쓰신다면 이 버전을 먼저 확인하세요.

#### Step 3 — 소스코드 받기  (두 방법 중 하나)

어느 쪽이든 코드는 같습니다. 다만 **실습에 git push 가 포함되므로 방법 A 를 권합니다**. 방법 B 로 받으면 나중에 내 저장소를 따로 만들어 연결해야 합니다.

> ⚠️ **Lab 1 에서 push 를 합니다** 첫 실습에서 만든 프로그램을 커밋하고 내 GitHub 에 올려 봅니다. 방법 A 로 받으면 fork 한 저장소가 이미 내 것이라 git push 한 줄로 끝납니다. 방법 B 로 받으면 저장소를 하나 더 만들고 원격지로 등록해야 합니다(utils.md 참고).

####   방법 A — 수업 저장소 fork 후 clone  `(권장)`

> ⚠️ **되도록 이 방법을 쓰세요** fork 는 클릭 한 번이면 끝나고, 그 뒤로 실습 결과를 내 GitHub 에 그대로 올릴 수 있습니다. 방법 B 로 시작했다가 나중에 push 가 필요해지면 저장소를 새로 만들고 원격지를 등록하는 과정을 다시 거쳐야 합니다. GitHub 계정이 있다면 처음부터 방법 A 로 가는 편이 훨씬 수월합니다.

fork 는 남의 저장소를 내 계정으로 복사해 오는 것입니다. 복사본의 주인이 나이므로 push 가 됩니다.

- **① **`github.com/bigsys-gnu/xv6lab-fall2026` 에 접속해 오른쪽 위 **Fork** 를 누릅니다.
- **② **내 계정 아래에 `내계정/xv6lab-fall2026` 가 생깁니다.
- **③ **그 주소를 clone 합니다.

```
cd ~
git clone https://github.com/내계정/xv6lab-fall2026.git xv6
cd xv6

# origin 이 내 저장소를 가리키는지 확인
git remote -v
  origin  https://github.com/내계정/xv6lab-fall2026.git (fetch)
  origin  https://github.com/내계정/xv6lab-fall2026.git (push)

# 어디에 있는지 확인 — riscv 여야 합니다
git branch
  * riscv
```

> ℹ️ **이 방법의 장점** origin 이 이미 내 저장소이므로 별도 설정 없이 git push 가 됩니다. 실습 결과를 GitHub 에 남길 수 있고, 노트북이 고장 나도 코드를 잃지 않습니다. 수업에서 준비한 docs 와 실습용 브랜치도 함께 받습니다. push 는 utils.md 에서 다룹니다.

####   방법 B — 수업 저장소 직접 clone

GitHub 계정 없이도 됩니다. 다만 push 하려면 나중에 내 저장소를 따로 연결해야 합니다(utils.md 참고).

```
cd ~
git clone https://github.com/bigsys-gnu/xv6lab-fall2026.git xv6
cd xv6

# 어디에 있는지 확인 — riscv 여야 합니다
git branch
  * riscv
```

> ℹ️ **실습용 브랜치가 더 있습니다** clone 하면 실습의 출발점인 riscv 브랜치에서 시작합니다. 저장소에는 수업에서 쓸 다른 브랜치도 있으며, git branch -a 로 전부 볼 수 있습니다. 필요할 때 안내하겠습니다.

#### Step 4 — 받은 코드 확인

```
ls
  kernel/  user/  mkfs/  Makefile  README  LICENSE

git branch          # 현재 브랜치 (앞에 * 표시)
git log --oneline -1
```

> ⚠️ **줄번호 차이** 강의 자료의 줄번호는 특정 시점 기준입니다. 함수 이름으로 찾으면 됩니다. 크게 어긋난다면 알려 주세요.

#### Step 5 — 첫 빌드와 실행

```
make qemu
```

처음에는 컴파일에 1분 남짓 걸립니다. 다음과 같이 나오면 성공입니다.

```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$
```

프롬프트 `$` 가 뜨면 몇 가지를 쳐 보세요.

```
$ ls
$ echo hello
$ cat README
```

#### Step 6 — 종료 방법

> ⚠️ **종료는 Ctrl-A 다음 X** Ctrl-A 를 누르고 손을 뗀 다음, 이어서 X 를 누르세요. QEMU 의 종료 단축키입니다. 매년 여기서 한 번씩 당황합니다.

#### Step 7 — 코어 수 변경

기본은 코어 3개입니다. `Makefile` 의 `CPUS := 3` 이 그 값이고, 명령줄에서 덮어쓸 수 있습니다.

```
make CPUS=1 qemu       # 코어 하나 — 디버깅할 때 편합니다
make CPUS=8 qemu       # 코어 여덟 개 — 락 경쟁을 볼 때
```

부팅할 때 `hart N starting` 이 몇 줄 나오는지 세어 보세요. 코어 0 은 이 메시지를 찍지 않으므로 CPUS 보다 한 줄 적게 나옵니다.

> ℹ️ **NCPU 와 CPUS 의 차이** kernel/param.h 의 NCPU 8 은 커널이 지원하는 상한이고, Makefile 의 CPUS 는 실제로 띄우는 개수입니다. CPUS 를 8보다 크게 주면 커널이 감당하지 못합니다.

#### Step 8 — 주제별 브랜치 생성

주제마다 깨끗한 상태에서 시작할 수 있도록 브랜치를 따로 씁니다. 앞 주제에서 고친 코드가 뒤 주제를 방해하지 않습니다.

```
# 메모리 관리 실습을 시작할 때
git checkout riscv
git checkout -b memory

# 다음 주제로 넘어갈 때
git checkout riscv
git checkout -b process
```

지금 어느 브랜치에 있는지는 `git branch` 로, 무엇을 고쳤는지는 `git diff riscv...` 로 볼 수 있습니다.

### 1.9 설치 확인 체크리스트

아래 다섯 가지가 모두 되면 준비가 끝난 것입니다.

| # | 확인할 것 | 어떻게 |
|---|---|---|
| 1 | 툴체인이 설치되었다 | riscv64-linux-gnu-gcc --version 이 버전을 출력 |
| 2 | QEMU 가 설치되었다 | qemu-system-riscv64 --version 이 7.2 이상 |
| 3 | 소스가 홈 디렉터리 안에 있다 | pwd 를 쳤을 때 /home/... 으로 시작 |
| 4 | xv6 가 부팅한다 | make qemu 로 $ 프롬프트까지 나온다 |
| 5 | 빠져나올 수 있다 | Ctrl-A 다음 X 로 종료된다 |

그리고 다음 수업에 **기준 커밋 해시** 와 **hart 메시지 줄 수** 를 적어 오세요.

---

## 2. Windows + WSL2

*WSL2 안에 Ubuntu 24.04 를 깔면, 그 안에서는 1장과 완전히 동일합니다.*

#### Step 1 — WSL2 와 Ubuntu 설치

PowerShell 이나 명령 프롬프트를 관리자 권한으로 열고 실행합니다.

```
wsl --install -d Ubuntu-24.04
```

설치가 끝나면 재부팅한 뒤, 처음 실행할 때 리눅스용 사용자 이름과 비밀번호를 정하게 됩니다. 이 비밀번호는 나중에 sudo 를 쓸 때 필요하므로 기억해 두세요.

설치 상태는 이렇게 확인합니다.

```
wsl -l -v

#   NAME            STATE      VERSION
# * Ubuntu-24.04    Running    2          <- VERSION 이 2 여야 합니다
```

> ⚠️ **VERSION 이 1 인 경우** wsl --set-version Ubuntu-24.04 2 로 바꾸세요. WSL1 에서는 성능도 동작도 보장되지 않습니다.

#### Step 2 — 소스 위치

이것이 WSL2 에서 가장 중요한 주의사항입니다.

| 경로 | 결과 |
|---|---|
| `~/xv6-riscv  (=  /home/사용자/xv6-riscv)` | 정상 — 이렇게 두세요 |
| `/mnt/c/Users/사용자/xv6-riscv` | 빌드가 몇 배로 느려집니다. 피하세요 |

`/mnt/c` 아래는 윈도우 파일 시스템을 리눅스에서 들여다보는 통로입니다. 파일을 하나 열고 닫을 때마다 두 시스템 사이를 오가야 해서, 파일을 수백 개 다루는 빌드에서는 차이가 크게 벌어집니다. xv6 는 파일이 예순 개가 넘습니다.

> ℹ️ **윈도우 탐색기에서 열기** 주소창에 \\wsl$\Ubuntu-24.04\home\사용자이름 을 입력하면 리눅스 쪽 파일을 윈도우 탐색기에서 볼 수 있습니다. 소스를 옮기지 말고 이렇게 보세요. VS Code 를 쓴다면 WSL 확장을 설치한 뒤 WSL 터미널에서 code . 로 여는 방법이 가장 편합니다.

#### Step 3 — 코어 · 메모리 배정

WSL2 는 기본적으로 호스트의 논리 프로세서를 모두 씁니다. 조절하고 싶다면 윈도우 사용자 폴더에 `.wslconfig` 파일을 만듭니다.

```
# 위치: C:\Users\<사용자이름>\.wslconfig

[wsl2]
processors=8
memory=8GB
```

고친 뒤에는 `wsl --shutdown` 으로 한 번 내렸다가 다시 켜야 반영됩니다. 배정한 코어 수는 `nproc` 으로 확인할 수 있습니다.

> ℹ️ **QEMU 코어 수와의 구분** WSL2 에 배정하는 코어는 리눅스가 쓸 수 있는 호스트 코어입니다. xv6 가 보는 코어 수는 make CPUS=N 이 정합니다. 둘은 별개이며, WSL2 코어가 적어도 CPUS=8 은 동작합니다. 다만 실제로 병렬로 도는 정도는 줄어듭니다.

#### Step 4 — 이후 절차

Ubuntu 터미널을 열고 1장의 **Step 1** 부터 그대로 따라가면 됩니다. 패키지 설치도, 빌드도, 실행도 동일합니다.

> ⚠️ **직접 확인이 필요합니다** 윈도우 버전과 빌드에 따라 wsl 명령의 동작이 조금씩 다릅니다. 특히 오래된 Windows 10 에서는 WSL2 를 쓰기 위해 별도의 기능 활성화가 필요할 수 있습니다. 위 절차가 그대로 되지 않으면 마이크로소프트의 WSL 설치 문서를 참고해 각자 환경에 맞춰 진행하세요.

---

## 3. macOS

*Intel 과 Apple Silicon 모두 됩니다. QEMU 가 RISC-V 를 전부 소프트웨어로 흉내 내므로 호스트 CPU 종류와 무관합니다.*

#### Step 1 — 준비

```
# Xcode 명령줄 도구
xcode-select --install

# Homebrew 가 없다면
/bin/bash -c "$(curl -fsSL \
  https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Apple Silicon 에서 Homebrew 를 새로 설치했다면 PATH 설정이 필요합니다. 설치 마지막에 안내되는 `eval "$(/opt/homebrew/bin/brew shellenv)"` 줄을 셸 설정 파일에 넣으세요.

#### Step 2 — 툴체인과 QEMU 설치

```
brew tap riscv-software-src/riscv
brew install riscv-tools
brew install qemu
brew install riscv64-elf-gdb
```

> ⚠️ **설치 소요 시간** riscv-tools 는 소스에서 빌드하는 경우가 있어 수십 분이 걸릴 수 있습니다. 수업 직전에 시작하지 마세요.

#### Step 3 — 설치 확인

```
riscv64-unknown-elf-gcc --version
qemu-system-riscv64 --version
```

컴파일러 이름이 리눅스와 다릅니다. apt 로 깔면 `riscv64-linux-gnu-` 이고, Homebrew 는 `riscv64-unknown-elf-` 입니다.

xv6 의 Makefile 은 이 이름을 자동으로 찾습니다. 찾는 순서가 `riscv64-unknown-elf-` → `riscv64-elf-` → `riscv64-none-elf-` → `riscv64-linux-gnu-` 이므로, Homebrew 로 깔았다면 대개 그냥 됩니다.

그래도 `Couldn't find a riscv64 version of GCC/binutils` 오류가 난다면 직접 지정하세요.

```
make TOOLPREFIX=riscv64-unknown-elf- qemu

# 매번 치기 번거로우면 Makefile 35번 줄 근처의 주석을 풀어 고정합니다
# TOOLPREFIX = riscv64-unknown-elf-
```

#### Step 4 — 이후 절차

1장의 **Step 3**(소스 받기)부터 그대로 따라가면 됩니다. 빌드와 실행, 브랜치 사용법은 모두 동일합니다.

> ⚠️ **직접 확인이 필요합니다** macOS 버전과 Homebrew 상태에 따라 탭 이름이나 패키지 구성이 바뀌는 일이 있습니다. brew install 이 실패하면 오류 메시지에 나오는 안내를 먼저 따라 보시고, 그래도 안 되면 brew update 후 다시 시도하세요. 위 절차가 그대로 되지 않을 수 있으니 각자 환경에서 확인하며 진행하세요.

---

## 부록 A — 오류와 해결

| 증상 | 원인과 해결 |
|---|---|
| Couldn't find a riscv64 version of GCC | 툴체인이 없거나 이름이 다릅니다. Step 2 의 확인 명령을 먼저 쳐 보세요. macOS 라면 TOOLPREFIX 를 지정합니다. |
| qemu-system-riscv64: command not found | qemu-system-misc 를 설치하지 않았습니다. 패키지 이름에 riscv 가 들어가지 않으니 주의하세요. |
| QEMU 버전이 낮다는 오류 | Makefile 이 7.2 이상을 요구합니다. 배포판이 오래되었다면 최신 Ubuntu 로 옮기는 편이 빠릅니다. |
| make 가 매우 느리다 (WSL2) | 소스가 /mnt/c 아래에 있습니다. ~ 안으로 옮기고 다시 빌드하세요. |
| Ctrl-C 로 qemu 가 안 꺼진다 | Ctrl-A 를 누르고 손을 뗀 다음 X 입니다. |
| 부팅은 되는데 hart 메시지가 2줄뿐 | 정상입니다. 기본 CPUS 가 3 이고 코어 0 은 이 메시지를 찍지 않습니다. |
| 빌드는 됐는데 화면이 멈춘 것 같다 | 이미 xv6 안입니다. 커서가 $ 뒤에 있으면 ls 를 쳐 보세요. |
| 이전 빌드 결과가 남아 이상하다 | make clean 후 다시 make qemu 하세요. |

