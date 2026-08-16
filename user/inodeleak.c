#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// inodeleak.c -- demonstrate that xv6 can leak an on-disk inode with NO
// crash, purely from a legal concurrent interleaving in iput().
//
// THE BUG (kernel/fs.c, iput):
//
//   The freeing thread F holds a counted reference to ip across the ENTIRE
//   free path; `ip->ref--` is the last statement, run only AFTER itrunc /
//   iupdate have already zeroed the inode on disk and after the sleeplock
//   is released:
//
//       if (ip->ref == 1 && ip->valid && ip->nlink == 0) {
//         acquiresleep(&ip->lock);
//         release(&itable.lock);
//         itrunc(ip); ip->type = 0; iupdate(ip); ip->valid = 0;   // inum X is now FREE on disk
//         releasesleep(&ip->lock);
//         acquire(&itable.lock);
//       }
//       ip->ref--;                                                // F still owned ref>0 up to here
//
//   Between F's `release(&itable.lock)` and F's `ip->ref--`, inum X reads
//   type==0 on disk (allocatable) while F's in-core table slot still has
//   ref>0 at inum X.  A concurrent ialloc()+iget() for the recycled inum
//   therefore ADOPTS F's slot -- iget matches any entry with ref>0 at
//   (dev,inum) -- and bumps ref 1->2.  The new incarnation can live an
//   entire life (create, link, use, unlink, close); its final iput sees
//   ref==2 (F's ghost still counted) and SKIPS the free, because the guard
//   is an `if` that fires only at ref==1.  F then drops ref 2->1, and later
//   ->0, but by then the guard is long past: the NEW incarnation's inode is
//   never freed.  Result on disk: type!=0, nlink==0, ref==0, no dirent --
//   a leaked inode, and not a single crash occurred.
//
//   A one-word fix closes it: make the free guard a `while` instead of an
//   `if`, so whichever thread performs the FINAL decrement re-tests the
//   guard and buries the body itself.
//
// HOW THIS TEST DETECTS IT:
//
//   A leaked inode is unrecoverable while the kernel runs -- only boot-time
//   ireclaim reclaims it -- so the count of allocatable inodes drops
//   permanently within a single run.  We (1) measure free-inode capacity by
//   creating files until allocation fails, (2) run a heavy concurrent
//   create/unlink/close workload that constantly recycles the just-freed
//   inum, (3) measure capacity again.  A drop, with no panic, is the leak.
//
//   The race window is small, so detection is PROBABILISTIC.  It needs real
//   parallelism (run qemu with CPUS>1, the default) and enough churn.  If a
//   run reports 0 leaked, raise the arguments or run it again.
//
//   usage:  inodeleak [children] [iters-per-child]
//   default: 8 children, 3000 iterations each.

static void
mkname(char *buf, char *prefix, int n)
{
  int i = 0;
  while (prefix[i]) {
    buf[i] = prefix[i];
    i++;
  }
  char tmp[12];
  int j = 0;
  if (n == 0)
    tmp[j++] = '0';
  while (n > 0) {
    tmp[j++] = '0' + (n % 10);
    n /= 10;
  }
  while (j > 0)
    buf[i++] = tmp[--j];
  buf[i] = 0;
}

// Number of inodes that can be allocated right now: create files until
// allocation fails, then free them all again.  Leaves the fs as it found it.
static int
capacity(char *prefix)
{
  char name[16];
  int n = 0;

  for (;;) {
    mkname(name, prefix, n);
    int fd = open(name, O_CREATE | O_RDWR);
    if (fd < 0)
      break;
    close(fd);
    n++;
    if (n > 4096) // safety valve; xv6 has NINODES == 200
      break;
  }
  for (int i = 0; i < n; i++) {
    mkname(name, prefix, i);
    unlink(name);
  }
  return n;
}

// One churning worker: repeatedly allocate and free a single inode.  The
// close() is the freeing iput() -- the race site above -- while the sibling
// workers' open() calls are the ialloc()+iget() that recycles the inum.
static void
churn(int id, int iters)
{
  char name[16];
  mkname(name, "c", id);

  for (int i = 0; i < iters; i++) {
    int fd = open(name, O_CREATE | O_RDWR);
    if (fd < 0)
      continue;        // pool momentarily drained by in-flight frees
    unlink(name);      // nlink -> 0, dirent removed; fd still pins a ref
    close(fd);         // ref -> 0, nlink == 0  =>  iput frees inum on disk
  }
}

int
main(int argc, char **argv)
{
  int children = 8, iters = 3000;

  if (argc > 1)
    children = atoi(argv[1]);
  if (argc > 2)
    iters = atoi(argv[2]);
  if (children < 1)
    children = 1;

  int before = capacity("f");
  printf("inodeleak: free inodes before = %d\n", before);
  printf("inodeleak: churning with %d children x %d iters...\n", children, iters);

  for (int c = 0; c < children; c++) {
    int pid = fork();
    if (pid == 0) {
      churn(c, iters);
      exit(0);
    }
    if (pid < 0) {
      printf("inodeleak: fork failed\n");
      children = c; // only wait for the ones that started
      break;
    }
  }
  for (int c = 0; c < children; c++)
    wait(0);

  // Defensive: remove any straggler worker files (there should be none --
  // every iteration is balanced -- so leftovers would themselves be a bug).
  for (int c = 0; c < children; c++) {
    char name[16];
    mkname(name, "c", c);
    unlink(name);
  }

  int after = capacity("f");
  printf("inodeleak: free inodes after  = %d\n", after);

  int leaked = before - after;
  if (leaked > 0)
    printf("inodeleak: LEAKED %d inode(s) with no crash -- bug reproduced\n",
           leaked);
  else
    printf("inodeleak: no leak observed this run "
           "(raise args, ensure CPUS>1, or re-run)\n");

  exit(0);
}
