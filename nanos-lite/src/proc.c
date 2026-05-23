#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)1);
  context_kload(&pcb[1], hello_fun, (void *)2);
  // context_uload(&pcb[1], "/bin/pal");
  context_uload(&pcb[1], "/bin/pal", (char *const[]){"--skip", NULL},(char *const[]){NULL});
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // add PA3.2 testing loader()
  naive_uload(NULL, NULL);

}

// struct Context {
//   uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
//   void *pdir; // (Page Directory)
// };

// Context* schedule(Context *prev) {
//   current->cp = prev;
//   current = (current == pcb[0] ? pcb[1] : pcb[0]); // switch
//   return current->cp;
// }


Context *schedule(Context *prev)
{
    static int schedule_table[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                   1, 1, 1, 1, 1, 1, 1, 1, 1, 0};
    static int ptr = 0;
    current->cp = prev;
    PCB *switch_to = &pcb[schedule_table[ptr]];
    ptr = (ptr + 1) % (sizeof(schedule_table) / sizeof(int));
    current = switch_to;
    return current->cp;
}

// typedef union {
//   uint8_t stack[STACK_SIZE] PG_ALIGN;
//   struct {
//     Context *cp;      // 上下文指针，保存寄存器状态
//     AddrSpace as;      // 地址空间，管理进程内存映射
//     // we do not free memory, so use `max_brk' to determine when to call _map()
//     uintptr_t max_brk; // 最大堆地址，用于内存分配
//   };
// } PCB;
// Context *kcontext(Area kstack, void (*entry)(void *), void *arg) 

void context_kload(PCB * p, void (*entry)(void *), void *arg) {
  AddrSpace a = p->as;
  Context * c = kcontext(a, a->area, arg);
  p->cp = c;
}

// // Context *ucontext(AddrSpace *as, Area kstack, void *entry) 
// Context * context_uload(PCB * p, const char * path) {
//   AddrSpace a = p->as;
//   // entry from loader
//   uintptr_t loader_entry = loader(); 
//   Context * c = ucontext(a, a->area, entry);
//   p->cp = c;
//   return c->GPRx
// }


