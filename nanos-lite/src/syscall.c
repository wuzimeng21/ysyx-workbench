#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case EVENT_YIELD:
      SYS_yield(c);
      break;
    case EVENT_SYSCALL:
      SYS_yield(c);
      break;
    case EVENT_IRQ_IODEV:
      SYS_write(c);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

static int SYS_yield(Context *c){
  yield();
  c->GPRx = 0;
  return c->GPRx;
}

static int SYS_exit(Context *c){
  halt(1);
  return 0;
}

static int SYS_write(Context *c){
  int fd = (int)c->GPR2;
  char * buf = (char *)c->GPR3;
  size_t count = (size_t)c->GPR4;
  if(fd == 1 || fd == 2) {
    for(int i = 0; i < count ; i ++) putch(buf[i]);
    c->GPRx = count;
  }
  else c->GPRx = -1;  
  return c->GPRx;
}


static int SYS_brk(Context *c){
  intptr_t increment = c->GPR2;
  char * heap_end = c->GPR3;
  extern char _end;
  if(heap_end + increment < _end) c->GPRx = -1;
  else c->GPRx = 0;
  return c->GPRx;
}


static int SYS_open(Context *c) {
 
  const char *pathname = (const char *)c->GPR2;  
  int flags = c->GPR3;                            
  int mode = c->GPR4;                              
  int fd = fs_open(pathname, flags, mode);

  strace_log (c);
  c->GPRx = fd;
  return c->GPRx;
}

static int SYS_read(Context *c) {
  int fd = c->GPR2;         
  void *buf = (void *)c->GPR3;
  size_t len = c->GPR4;        
  
  size_t ret = fs_read(fd, buf, len);
  
  strace_log (c);
  c->GPRx = ret; 
  return c->GPRx;
}

static int SYS_write(Context *c) {
  int fd = c->GPR2;                   
  const void *buf = (const void *)c->GPR3;
  size_t len = c->GPR4;                   
  
  size_t ret = fs_write(fd, buf, len);
  strace_log (c);
  c->GPRx = ret; 
  return c->GPRx;
}

static int SYS_close(Context *c) {
  int fd = c->GPR2;
  int ret = fs_close(fd);
  strace_log (c);
  c->GPRx = ret; 
  return c->GPRx;
}

static int SYS_gettimeofday(Context *c) {
  struct timeval tv = (struct timeval )c->GPR2;
  struct timezone tz = NULL;
  int ret = gettimeofday(&tv, &tz);
  c->GPRx = ret; 
  return c->GPRx;
}


