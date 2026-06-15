#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void context_kload(PCB *p, void (*entry)(void *), void *arg);

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");

  // PA4.2: Create multiple processes
  context_kload(&pcb[0], hello_fun, (void *)0);
  context_uload(&pcb[1], "/bin/menu", NULL, NULL);

  Log("Processes initialized. Enabling interrupts...");
  iset(true);  // Enable machine timer interrupts for preemptive scheduling
  yield();
  panic("init_proc: should not reach here");
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
    static int schedule_table[] = {1, 0};  // alternate: menu ↔ hello_fun
    static int ptr = 0;
    current->cp = prev;
    int idx = schedule_table[ptr];
    PCB *switch_to = &pcb[idx];
    ptr = (ptr + 1) % (sizeof(schedule_table) / sizeof(int));
    if (switch_to->cp == NULL) {
        return prev;
    }
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

void context_kload(PCB *p, void (*entry)(void *), void *arg) {
  Area kstack = (Area){&p->stack[0], &p->stack[sizeof(p->stack)]};
  Context *c = kcontext(kstack, entry, arg);
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


