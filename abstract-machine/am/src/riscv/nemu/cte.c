#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

/**
 * @brief 异常中断处理函数
 * @param c 指向当前保存的上下文结构体的指针
 * @return 处理后的上下文指针（可能被上层修改）
 * 
 * 这个函数在 __am_asm_trap（汇编代码）保存完上下文后被调用。
 * 它接收从汇编传来的上下文指针，将异常信息包装成Event结构，
 * 然后调用用户注册的事件处理函数。
 */
Context* __am_irq_handle(Context *c) {
  printf("========== Context Dump ==========\n");
  printf("Context at: %p\n", c);
  
  // 打印通用寄存器
  for (int i = 0; i < NR_REGS; i++) {
    printf("  gpr[%2d] = 0x%08lx", i, c->gpr[i]);
    // 每行打印4个寄存器
    if ((i + 1) % 4 == 0) printf("\n");
  }
  printf("\n");
  
  // 打印CSR寄存器
  printf("  mcause  = 0x%08lx\n", c->mcause);
  printf("  mstatus = 0x%08lx\n", c->mstatus);
  printf("  mepc    = 0x%08lx\n", c->mepc);
  printf("  pdir    = %p\n", c->pdir);
  printf("==================================\n");
  // ----- 1. 检查是否有注册的事件处理函数 -----
  if (user_handler) {
    
    // ----- 2. 创建并初始化事件结构 -----
    Event ev = {0};  // 全部初始化为0
    
    // ----- 3. 根据异常原因（mcause）分发事件 -----
    switch (c->mcause) {
      // TODO: 需要根据不同的 mcause 值设置对应的事件类型
      case 11:
        ev.event = EVENT_YIELD; 
        break;
      case 8:
        ev.event = EVENT_SYSCALL; 
        break;
      case 9:
        ev.event = EVENT_SYSCALL; 
        break;
      case (12|13|15):
        ev.event = EVENT_PAGEFAULT; 
        break;
      // case 5:
      //   ev.event = EVENT_IRQ_TIMER; 
      //   break;
      // case 6:
      //   ev.event = EVENT_IRQ_IODEV; 
      //   break;
      default: 
        // 设置为 EVENT_ERROR，表示发生了未知错误
        ev.event = EVENT_ERROR; 
        break;
    }

    // ----- 4. 调用用户注册的事件处理函数 -----
    // 将事件 ev 和当前上下文 c 传给上层
    c = user_handler(ev, c);
    
    // ----- 5. 安全检查 -----
    // 确保上层处理函数返回了有效的上下文指针
    // 如果返回 NULL，说明上层有严重错误，直接断言失败
    assert(c != NULL);
  }

  // ----- 6. 返回处理后的上下文 -----
  // 这个返回值会传回 __am_asm_trap
  // 汇编代码会根据这个上下文恢复寄存器并返回用户程序
  return c;

}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  // 将 __am_asm_trap 函数的地址写入 RISC-V 的 mtvec 寄存器
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  // 将参数 handler 保存到全局变量 user_handler 中
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall"); 
#else
  asm volatile("li a7, -1; ecall");// # 对于标准架构：将 -1 加载到 a7 (x17)
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
