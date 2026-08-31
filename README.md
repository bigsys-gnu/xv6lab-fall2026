# 운영체제 실습 — xv6 (2026 가을)

경상국립대학교 운영체제 실습 과목의 코드 저장소입니다.
[mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv) 를 기준으로 합니다.

---

## 시작하기

처음이라면 **[docs/lab0-setup.md](docs/lab0-setup.md)** 를 먼저 읽으세요.
도구 설치부터 첫 빌드까지 들어 있습니다.

**1. 이 저장소를 내 계정으로 복사합니다 (fork)**

이 페이지 오른쪽 위의 **Fork** 버튼을 누르세요. GitHub 웹에서 하는 작업이고, 터미널 명령이 아닙니다.
누르고 나면 `https://github.com/<내계정>/xv6lab-fall2026` 이 생깁니다.

**2. 내 계정에 생긴 저장소를 받습니다**

```sh
git clone https://github.com/<내계정>/xv6lab-fall2026.git xv6
cd xv6
git branch          # riscv 에 있는지 확인
```

**3. 빌드하고 실행합니다**

```sh
make qemu

# 빠져나오기 —  Ctrl-A 를 누르고 손을 뗀 다음 X
```

빌드에는 RISC-V 툴체인과 QEMU 7.2 이상이 필요합니다. 설치 방법은 [docs/lab0-setup.md](docs/lab0-setup.md) 에 있습니다.

---

## 문서

| 문서 | 내용 |
|---|---|
| [lab0-setup.md](docs/lab0-setup.md) | Lab 0 — 환경 설정 · 소스코드 받기 · 첫 빌드 |
| [lab1-userprog.md](docs/lab1-userprog.md) | Lab 1 — 소스 구조 둘러보기 · 응용 프로그램 만들기 |
| [lab2-memory.md](docs/lab2-memory.md) | Lab 2 — 메모리 관리 · freemem · vmprint · va2pa · 가드 페이지 |
| [utils.md](docs/utils.md) | 참고 — git · 소스코드 읽기 · gdb |

강의 슬라이드는 LMS 로 배포합니다.

---

## 브랜치

| 브랜치 | 용도 |
|---|---|
| `riscv` | **기본 브랜치.** 실습의 출발점. 손대지 않고 그대로 둡니다 |
| 그 외 | 수업에서 준비한 실습용 브랜치. 필요할 때 안내합니다 |

주제마다 `riscv` 에서 새 브랜치를 만들어 작업합니다.

```sh
git checkout riscv          # 원본으로 이동
git checkout -b test-branch # 새 브랜치를 만들고 그리로 이동
```

내가 고친 것만 확인하려면 **점 세 개**를 쓰세요.

```sh
git diff riscv...
```

점 두 개(`git diff riscv`)는 `riscv` 의 **현재 상태(current tip)** 와 비교합니다. 그래서 내 브랜치를 만든 뒤 `riscv` 가 갱신되면 그 차이까지 섞여 나옵니다.
점 세 개는 **분기 지점(merge base)** 과 비교하므로 내가 고친 것만 나옵니다.

---

## 자주 쓰는 명령

| 하려는 일 | 명령 |
|---|---|
| 빌드하고 실행 | `make qemu` |
| 코어 하나로 실행 (디버깅용) | `make CPUS=1 qemu` |
| gdb 와 함께 실행 | `make CPUS=1 qemu-gdb` |
| 빌드 결과 지우기 | `make clean` |
| xv6 에서 빠져나오기 | `Ctrl-A` 다음 `X` |

기본 코어 수는 3 개입니다 (`Makefile` 의 `CPUS`). `kernel/param.h` 의 `NCPU 8` 은 커널이 지원하는 상한입니다.

---

## 막혔을 때

1. `docs/lab0-setup.md` 의 **부록 A — 오류와 해결** 을 먼저 펴 보세요
2. `panic` 이나 `usertrap` 이 주소를 찍었다면 `kernel/kernel.asm` 에서 그 주소를 검색해 보세요
3. 30 분 이상 진전이 없으면 오류 메시지 전체를 캡처해 문의하세요

환경 구성은 이 과목에서 배울 내용이 아닙니다. 혼자 오래 붙들지 마세요.

---

## 원본에 대하여

xv6 는 Dennis Ritchie 와 Ken Thompson 의 Unix Version 6 (1975) 를 RISC-V 로 다시 구현한 교육용 운영체제입니다.
원본 저장소의 설명은 [README](README) 를 참고하세요.

이 저장소(repository)를 **fork 해서 사용하세요.** 실습 결과를 내 GitHub 에 그대로 올릴 수 있습니다.
이 저장소가 갱신되면 GitHub 웹의 **Sync fork** 버튼으로 받아 옵니다 — [utils.md](docs/utils.md) 참고.
