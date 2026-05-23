#include <common.h>
#include "syscall.h"
// void do_syscall(Context *c) {
//   uintptr_t a[4];
//   a[0] = c->GPR1;

//   switch (a[0]) {
//     case EVENT_YIELD:
//       SYS_yield(c);
//       break;
//     case EVENT_SYSCALL:
//       SYS_yield(c);
//       break;
//     case EVENT_IRQ_IODEV:
//       SYS_write(c);
//       break;
//     default: panic("Unhandled syscall ID = %d", a[0]);
//   }
// }

void do_syscall(Context *c)
{
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;
 
	switch (a[0])
    {
    case SYS_yield:
        yield();
        c->GPRx = 0;
        break;
    case SYS_exit:
	    halt(a[1]);
	    break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
}



static int sys_yield(Context *c){
  yield();
  c->GPRx = 0;
  return c->GPRx;
}

static int sys_exit(Context *c){
  // halt(1);
  char * const argv = c->GPR3;
  char * const envp = c->GPR4;
  // int ret = SYS_execve("bin/menu", argv, envp);
  int ret = SYS_execve("/bin/nterm", argv, envp);
  return 0;
}

static int sys_write(Context *c){
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


static int sys_brk(Context *c){
  intptr_t increment = c->GPR2;
  char * heap_end = c->GPR3;
  extern char _end;
  if(heap_end + increment < _end) c->GPRx = -1;
  else c->GPRx = 0;
  return c->GPRx;
}


static int sys_open(Context *c) {
  const char *pathname = (const char *)c->GPR2;  
  int flags = c->GPR3;                            
  int mode = c->GPR4;                              
  int fd = fs_open(pathname, flags, mode);

  strace_log (c);
  c->GPRx = fd;
  return c->GPRx;
}

static int sys_read(Context *c) {
  int fd = c->GPR2;         
  void *buf = (void *)c->GPR3;
  size_t len = c->GPR4;        
  
  size_t ret = fs_read(fd, buf, len);
  
  strace_log (c);
  c->GPRx = ret; 
  return c->GPRx;
}

// static int sys_write(Context *c) {
//   int fd = c->GPR2;                   
//   const void *buf = (const void *)c->GPR3;
//   size_t len = c->GPR4;                   
  
//   size_t ret = fs_write(fd, buf, len);
//   strace_log (c);
//   c->GPRx = ret; 
//   return c->GPRx;
// }

static int sys_close(Context *c) {
  int fd = c->GPR2;
  int ret = fs_close(fd);
  strace_log (c);
  c->GPRx = ret; 
  return c->GPRx;
}

static int sys_gettimeofday(Context *c) {
  struct timeval tv = (struct timeval )c->GPR2;
  struct timezone tz = NULL;
  int ret = gettimeofday(&tv, &tz);
  c->GPRx = ret; 
  return c->GPRx;
}



// void naive_uload(PCB *pcb, const char *filename) {
//   uintptr_t entry = loader(pcb, filename);
//   Log("Jump to entry = %p", entry);
//   ((void(*)())entry) ();
// }
// static int SYS_execve(Context *c) {
//   const char * pathname = c->GPR2;
//   char * const argv = c->GPR3;
//   char * const envp = c->GPR4;
//   // naive_uload(NULL, pathname);
//   context_uload(current, filename, argv, envp);
//   switch_boot_pcb();
//   yield();
//   c->GPRx = 0;
//   return c->GPRx;
// }

static int sys_execve(Context *c, uintptr_t *a)
{
    const char *filename = (const char *)a[1];
    char *const *argv = (char *const *)a[2];
    char *const *envp = (char *const *)a[3];
#ifdef CONFIG_STRACE
    printf("%s\n", pathname);
#endif
    int fd = fs_open(filename, 0, 0);
    if (fd == -1)
        return -2;
    fs_close(fd);
    void switch_boot_pcb();
    void context_uload(PCB * pcb, const char *filename, char *const argv[],
                       char *const envp[]);
    context_uload(current, filename, argv, envp);
    switch_boot_pcb();
    yield();
    return 0;
}


static int SYS_brk() {
  
}