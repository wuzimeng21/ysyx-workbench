#include <common.h>
#include <fs.h>
#include <memory.h>
#include <proc.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
  case SYS_exit:
    halt(a[1]);
    break;
  case SYS_yield:
    yield();
    c->GPRx = 0;
    break;
  case SYS_open:
    c->GPRx = fs_open((const char *)a[1], a[2], a[3]);
    break;
  case SYS_read:
    c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
    break;
  case SYS_write:
    c->GPRx = fs_write(a[1], (const void *)a[2], a[3]);
    break;
  case SYS_kill:
    c->GPRx = -1;
    break;
  case SYS_getpid:
    c->GPRx = 0;
    break;
  case SYS_close:
    c->GPRx = fs_close(a[1]);
    break;
  case SYS_lseek:
    c->GPRx = fs_lseek(a[1], a[2], a[3]);
    break;
  case SYS_brk:
    c->GPRx = mm_brk(a[1]);
    break;
  case SYS_fstat:
    c->GPRx = -1;
    break;
  case SYS_time:
    c->GPRx = 0;
    break;
  case SYS_signal:
    c->GPRx = -1;
    break;
  case SYS_execve:
    naive_uload(NULL, (const char *)a[1]);
    break;
  case SYS_fork:
    c->GPRx = -1;
    break;
  case SYS_link:
    c->GPRx = -1;
    break;
  case SYS_unlink:
    c->GPRx = -1;
    break;
  case SYS_wait:
    c->GPRx = -1;
    break;
  case SYS_times:
    c->GPRx = 0;
    break;
  case SYS_gettimeofday: {
    AM_TIMER_UPTIME_T t = io_read(AM_TIMER_UPTIME);
    if (a[1]) {
      uint32_t *tv = (uint32_t *)(uintptr_t)a[1];
      tv[0] = t.us / 1000000;
      tv[1] = t.us % 1000000;
    }
    c->GPRx = 0;
    break;
  }
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
